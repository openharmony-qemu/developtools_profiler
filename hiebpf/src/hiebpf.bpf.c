/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <linux/version.h>
#include "vmlinux.h"
#include "bpf_helpers.h"
#include "bpf_tracing.h"
#include "bpf_core_read.h"
#include "hiebpf_types.h"
#include "bpf_log_writer.h"

#ifndef VM_FAULT_ERROR
#define VM_FAULT_ERROR (VM_FAULT_OOM | VM_FAULT_SIGBUS |    \
            VM_FAULT_SIGSEGV | VM_FAULT_HWPOISON |    \
            VM_FAULT_HWPOISON_LARGE | VM_FAULT_FALLBACK)
#endif

extern int LINUX_KERNEL_VERSION __kconfig;
const volatile unsigned int g_stack_limit = 0;
static const int SHIFT_32 = 32;

// global configuration data
// const volatile int tracer_pid = -1;
// const volatile int bpf_log_level = BPF_LOG_DEBUG;
// const volatile int max_stack_depth = MAX_STACK_LIMIT;
// const volatile int target_pids[MAX_TARGET_PIDS];

/*********************************************************************************
 * use BPF map to store global configuration data instead if global variables
 * are not available
 ********************/
struct {
    /*
     * config_var_map[0] = tracer_pid, tracer_pid != -1 means not tracing the tracer itself
     * config_var_map[1] = bpf_log_level
     * config_var_map[2] = max_stack_limit
     * config_var_map[3] = max_stack_depth,
     * config_var_map[4] = unwind_stack, none-zero means unwinding stack, other wise not unwinding
    */
    __uint(type, BPF_MAP_TYPE_ARRAY);
    __uint(key_size, sizeof(u32));
    __uint(value_size, sizeof(u32));
    __uint(max_entries, NR_CONFIG_VARIABLES);
} config_var_map SEC(".maps");

struct {
    /*
     * target_pid_map[0] != 0 means tracing all processes
     */
    __uint(type, BPF_MAP_TYPE_ARRAY);
    __uint(key_size, sizeof(u32));
    __uint(value_size, sizeof(u32));
    __uint(max_entries, MAX_TARGET_PIDS + 1);
} target_pid_map SEC(".maps");
/**********************
*************************************************************************************/

/******************************** BPF maps BEGIN*************************************/
/*start event map*/
struct {
    /* Since execution of syscalls of the same process never cross over,
     * we can simply use pid as the identifier of the start of a syscall
     */
    __uint(type, BPF_MAP_TYPE_HASH);
    __uint(key_size, sizeof(u64));
    __uint(value_size, sizeof(struct start_event_t));
    __uint(max_entries, MAX_START_EVENTS_NUM);
} start_event_map SEC(".maps");

/*pftrace stats map*/
struct {
    __uint(type, BPF_MAP_TYPE_ARRAY);
    __uint(key_size, sizeof(u32));
    __uint(value_size, sizeof(struct pf_stat_t));
    __uint(max_entries, PF_MAX_EVENT_TYPE);
} pftrace_stats_map SEC(".maps");

/*bpf ringbuffers*/
struct {
    __uint(type, BPF_MAP_TYPE_RINGBUF);
    __uint(max_entries, BPF_RINGBUF_SIZE);
} bpf_ringbuf_map SEC(".maps");

/********************************* BPF maps END *************************************/

/******************************** inline funcs BEGIN ********************************/
static __always_inline
int unwind_stack()
{
    u32 index = UNWIND_FLAG_INDEX;
    const u32 *unwind_ptr = bpf_map_lookup_elem(&config_var_map, &index);
    u32 unwind = 0;
    int err = bpf_probe_read_kernel(&unwind, sizeof(u32), unwind_ptr);
    if (err) {
        BPFLOGW(BPF_TRUE, "failed to read unwind configuration");
    }
    return unwind;
}

static __always_inline
int emit_fstrace_event(void* ctx, int64_t retval)
{
    // get exit timestamp as soon as possible
    u64 ctime = bpf_ktime_get_ns();
    u64 pid_tgid = bpf_get_current_pid_tgid();

    const u64 event_size = sizeof(struct fstrace_cmplt_event_t);
    struct fstrace_cmplt_event_t *cmplt_event = bpf_ringbuf_reserve(&bpf_ringbuf_map, event_size, 0);
    if (cmplt_event == NULL) {
        BPFLOGD(BPF_TRUE, "failed to reserve space for fstrace event from BPF ringbuffer");
        return -1;
    }
    __builtin_memset(cmplt_event, 0, event_size);

    const struct fstrace_start_event_t* start_event = bpf_map_lookup_elem(&start_event_map, &pid_tgid);
    int err = bpf_probe_read_kernel(&cmplt_event->start_event, sizeof(struct fstrace_start_event_t), start_event);
    if (err) {
        BPFLOGD(BPF_TRUE, "fstrace event discarded: failed to read fstrace_start_event");
        bpf_ringbuf_discard(cmplt_event, BPF_RB_NO_WAKEUP);
        return -1;
    }
    if (cmplt_event->start_event.type == 0) {
        BPFLOGI(BPF_TRUE, "fstrace event discarded: invalide fstrace start event");
        bpf_ringbuf_discard(cmplt_event, BPF_RB_NO_WAKEUP);
        return -1;
    }

    cmplt_event->ctime = ctime;
    cmplt_event->tracer = FSTRACE;
    cmplt_event->pid = (u32) pid_tgid;
    cmplt_event->tgid = (u32) (pid_tgid >> SHIFT_32);
    cmplt_event->uid = bpf_get_current_uid_gid();
    cmplt_event->gid = (bpf_get_current_uid_gid() >> SHIFT_32);
    cmplt_event->retval = retval;
    err = bpf_get_current_comm(cmplt_event->comm, MAX_COMM_LEN);
    if (err) {
        BPFLOGD(BPF_TRUE, "fstrace event discarded: failed to get process command");
        bpf_ringbuf_discard(cmplt_event, BPF_RB_NO_WAKEUP);
        return -1;
    }

    // get user callchain
    if (unwind_stack()) {
        cmplt_event->nips =
            bpf_get_stack(ctx, cmplt_event->ips, g_stack_limit * sizeof(u64), BPF_F_USER_STACK) / sizeof(u64);
    }

    // send out the complete event data to perf event buffer
    bpf_ringbuf_submit(cmplt_event, BPF_RB_FORCE_WAKEUP);
    return 0;
}

static __always_inline
int handle_pftrace_start_event(struct vm_fault *vmf, u32 type)
{
    struct start_event_t start_event = {};
    __builtin_memset(&start_event, 0, sizeof(start_event));
    struct pftrace_start_event_t *pf_se = &start_event.pf_se;
    pf_se->stime = bpf_ktime_get_ns();
    pf_se->type = type;
    int err = bpf_probe_read_kernel(&pf_se->addr, sizeof(vmf->address), &vmf->address);
    if (err) {
        BPFLOGW(BPF_TRUE, "failed read page fault address for pftrace start event");
        // clear the start event to indicate error
        __builtin_memset(&start_event, 0, sizeof(start_event));
    }
    u64 pid_tgid = bpf_get_current_pid_tgid();
    err = (int) bpf_map_update_elem(&start_event_map, &pid_tgid, &start_event, BPF_ANY);
    if (err < 0) {
        // this should never happens
        BPFLOGF(BPF_TRUE, "failed to update pftrace_start_event for pid_tgid = %lld", pid_tgid);
        return -1;
    }
    return 0;
}

static __always_inline
int read_modify_update_page_fault_stats(u32 type, u32 duration)
{
    const struct pf_stat_t *stat_ptr = bpf_map_lookup_elem(&pftrace_stats_map, &type);
    if (stat_ptr == NULL) {
        BPFLOGD(BPF_TRUE, "failed to lookup pftrace stat");
        return -1;
    }
    struct pf_stat_t stat;
    int err = (int) bpf_probe_read_kernel(&stat, sizeof(stat), stat_ptr);
    if (err < 0) {
        BPFLOGD(BPF_TRUE, "failed to read pftrace stat");
        return -1;
    }
    u32 tot_duration = stat.tot_duration + duration;
    u32 avg_duration = tot_duration / (stat.count + 1);
    stat.dev_duration = (duration * duration + stat.count * stat.dev_duration
                        + stat.tot_duration * stat.avg_duration - tot_duration * avg_duration)
                        / (stat.count + 1);
    ++stat.count;
    stat.tot_duration = tot_duration;
    stat.avg_duration = avg_duration;
    if (duration < stat.min_duration) {
        stat.min_duration = duration;
    } else if (duration > stat.max_duration) {
        stat.max_duration = duration;
    }
    err = (int) bpf_map_update_elem(&pftrace_stats_map, &type, &stat, BPF_ANY);
    if (err < 0) {
        BPFLOGD(BPF_TRUE, "failed to update pftrace stat");
        return -1;
    }

    return 0;
}

static __always_inline
int read_modify_update_current_type(u32 type)
{
    u64 pid_tgid = bpf_get_current_pid_tgid();
    const struct pftrace_start_event_t *se_ptr = bpf_map_lookup_elem(&start_event_map, &pid_tgid);
    struct pftrace_start_event_t start_event;
    int err = bpf_probe_read_kernel(&start_event, sizeof(start_event), se_ptr);
    if (err) {
        BPFLOGW(BPF_TRUE, "failed to read pftrace start event to update event type");
        return 0;
    }
    u32 old_type = start_event.type;
    if (type) {
        start_event.type = type;
    }
    return old_type;
}

static __always_inline
int handle_return_value(struct pftrace_cmplt_event_t* cmplt_event, long long retval)
{
    switch (read_modify_update_current_type(0)) {
        case PF_PAGE_CACHE_HIT: {
            struct file *fpin = (struct file *) retval;
            if (fpin == NULL) {
                cmplt_event->size = 0;
            } else {
                cmplt_event->size = 1;
            }
            break;
        }
        case PF_FILE_BACKED_IN:
        case PF_SWAP_FROM_ZRAM:
        case PF_SWAP_FROM_DISK:
        case PF_ZERO_FILL_PAGE:
        case PF_FAKE_ZERO_PAGE:
        case PF_COPY_ON_WRITE: {
            vm_fault_t vmf_flags = (vm_fault_t) retval;
            if (vmf_flags & VM_FAULT_ERROR) {
                cmplt_event->size = 0;
            } else {
                cmplt_event->size = 1;
            }
            break;
        }
        default: return -1;
    }
    return 0;
}

static __always_inline
int emit_pftrace_event(void* ctx, int64_t retval)
{
    // get timestamp as soon as possible
    u64 ctime =  bpf_ktime_get_ns();
    u64 pid_tgid = bpf_get_current_pid_tgid();

    const u64 event_size = sizeof(struct pftrace_cmplt_event_t);
    struct pftrace_cmplt_event_t* cmplt_event = bpf_ringbuf_reserve(&bpf_ringbuf_map, event_size, 0);
    if (cmplt_event == NULL) {
        BPFLOGD(BPF_TRUE, "failed to reserve space for pftrace event from BPF ringbuffer");
        return -1;
    }
    __builtin_memset(cmplt_event, 0, event_size);

    const struct pftrace_start_event_t *se_ptr = bpf_map_lookup_elem(&start_event_map, &pid_tgid);
    int err = bpf_probe_read_kernel(&cmplt_event->start_event, sizeof(cmplt_event->start_event), se_ptr);
    if (err) {
        BPFLOGI(BPF_TRUE, "pftrace event discarded: failed to read pftrace start event");
        bpf_ringbuf_discard(cmplt_event, BPF_RB_NO_WAKEUP);
        return -1;
    }
    if (cmplt_event->start_event.type == 0) {
        BPFLOGI(BPF_TRUE, "pftrace event discarded: invalide pftrace start event");
        bpf_ringbuf_discard(cmplt_event, BPF_RB_NO_WAKEUP);
        return -1;
    }
    cmplt_event->tracer = PFTRACE;
    cmplt_event->ctime = ctime;
    cmplt_event->pid = (u32) pid_tgid;
    cmplt_event->tgid = (pid_tgid >> SHIFT_32);
    cmplt_event->uid = bpf_get_current_uid_gid();
    cmplt_event->gid = (bpf_get_current_uid_gid() >> SHIFT_32);
    err = bpf_get_current_comm(cmplt_event->comm, MAX_COMM_LEN);
    if (err < 0) {
        BPFLOGD(BPF_TRUE, "pftrace event discarded: failed to get process command");
        bpf_ringbuf_discard(cmplt_event, BPF_RB_NO_WAKEUP);
        return -1;
    }
    err = handle_return_value(cmplt_event, retval);
    if (err) {
        BPFLOGW(BPF_TRUE, "pftrace event discarded: failed to handle pftrace return value");
        bpf_ringbuf_discard(cmplt_event, BPF_RB_NO_WAKEUP);
        return -1;
    }

    // get user callchain
    if (unwind_stack()) {
        cmplt_event->nips =
            bpf_get_stack(ctx, cmplt_event->ips, g_stack_limit * sizeof(u64), BPF_F_USER_STACK) / sizeof(u64);
    }

    if (read_modify_update_page_fault_stats(cmplt_event->start_event.type,
                                            cmplt_event->ctime - cmplt_event->start_event.stime) != 0) {
        BPFLOGD(BPF_TRUE, "pftrace event discarded: failed to update pftrace stats");
        bpf_ringbuf_discard(cmplt_event, BPF_RB_NO_WAKEUP);
        return -1;
    }

    bpf_ringbuf_submit(cmplt_event, BPF_RB_FORCE_WAKEUP);
    return 0;
}

static __always_inline
int emit_event(void* ctx, int64_t retval, u32 tracer)
{
    switch (tracer) {
        case FSTRACE:   return emit_fstrace_event(ctx, retval);
        case PFTRACE:   return emit_pftrace_event(ctx, retval);
        default:
            BPFLOGD(BPF_TRUE, "unkonwn event source with id = %d", tracer);
            return -1;
    }
}

static __always_inline
int is_target_process(const char* target_comm, const size_t comm_size)
{
    char curr_comm[MAX_COMM_LEN] = "\0";
    long retval = bpf_get_current_comm(curr_comm, sizeof(curr_comm));
    if (retval == 0) {
        size_t min_comm = comm_size < sizeof(curr_comm) ? comm_size : sizeof(curr_comm);
        for (size_t j = 0; j <= min_comm; ++j) {
            if (j == min_comm) {
                char fmt[] = "current comm is %s\n";
                bpf_trace_printk(fmt, sizeof(fmt), curr_comm);
                return 0;
            }
            if (target_comm[j] != curr_comm[j]) {
                break;
            }
        }
    }
    return -1;
}

// if both the pid and tgid values are - 1, check whether the event occurs in the current process
static __always_inline
int check_current_pid(const int32_t pid, const int32_t tgid)
{
    u32 index = 0;
    const int32_t* tracer_pid_ptr = bpf_map_lookup_elem(&config_var_map, &index);
    int32_t tracer_pid = -1;
    bpf_probe_read_kernel(&tracer_pid, sizeof(int32_t), tracer_pid_ptr);
    int32_t curr_pid = pid;
    if (curr_pid < 0) {
        curr_pid = (int32_t)bpf_get_current_pid_tgid();
    }
    int32_t curr_tgid = tgid;
    if (curr_tgid < 0) {
        curr_tgid = (int32_t)(bpf_get_current_pid_tgid() >> SHIFT_32);
    }
    if (curr_pid == tracer_pid || curr_tgid == tracer_pid) {
        // currrent process is not a target process
        return -1;
    }
    const u32 trace_all_index = 0;
    int32_t target_pids[MAX_TARGET_PIDS + 1];
    __builtin_memset(target_pids, 0, sizeof(target_pids));
    const int32_t *target_pids_ptr = bpf_map_lookup_elem(&target_pid_map, &trace_all_index);
    if (target_pids_ptr == NULL) {
        BPFLOGW(BPF_TRUE, "failed to lookup target pid map, will trace all processes");
        return 0;
    }
    int err = bpf_probe_read_kernel(target_pids, sizeof(int32_t) * (MAX_TARGET_PIDS + 1), target_pids_ptr);
    if (err) {
        BPFLOGW(BPF_TRUE, "failed to read target pids, will trace all processes");
        return 0;
    }
    if (target_pids[trace_all_index] != 0) {
        return 0;
    }
    for (u32 index = 1; index != MAX_TARGET_PIDS; ++index) {
        if (target_pids[index] < 0) {
            break;
        }
        if (target_pids[index] == curr_pid || target_pids[index] == curr_tgid) {
            return 0;
        }
    }
    return -1;
}

static __always_inline
size_t get_mountpoint_by_inode(char *filename, int len, const struct inode *host)
{
    struct list_head *mountsHead = BPF_CORE_READ(host, i_sb, s_mounts.next);
    struct mount *mnt = NULL;
    mnt = list_entry(mountsHead, struct mount, mnt_instance);
    const u32 MAX_MOUNT_POINT = 5;
    size_t pos = 0;
    for (u32 cnt = MAX_MOUNT_POINT; cnt != 0; --cnt) {
        long name_len = 0;
        const u8 *name = BPF_CORE_READ(mnt, mnt_mountpoint, d_name.name);
        if (BPF_CORE_READ(mnt, mnt_mountpoint, d_name.len) <= 1) {
            break;
        }
        name_len = bpf_probe_read_kernel_str(filename + pos, MAX_DENTRY_NAME_LEN, name);
        if (name_len <= 1) {
            BPFLOGD(BPF_TRUE, "failed to read dentry name from kernel stack buffer");
            break;
        }
        pos += (size_t)name_len;
        filename[pos - 1] = '/';
        mnt = BPF_CORE_READ(mnt, mnt_parent);
    }
    return pos;
}

static __always_inline
int get_filename_by_inode(char *filename, const size_t len, const struct inode *host)
{
    int err = 0;
    struct hlist_node *curr_hlist_node = BPF_CORE_READ(host, i_dentry.first);
    if (curr_hlist_node == NULL) {
        BPFLOGD(BPF_TRUE, "failed to get alias dentries of the inode");
        return -1;
    }

    const struct dentry *curr_dentry = NULL;
    const u32 MAX_ALIAS_DENTRY = 100;
    for (u32 cnt = MAX_ALIAS_DENTRY; cnt != 0; --cnt) {
        curr_dentry = container_of(curr_hlist_node, struct dentry, d_u);
        struct inode *curr_inode = BPF_CORE_READ(curr_dentry, d_inode);
        if (curr_inode == NULL) {
            BPFLOGD(BPF_TRUE, "failed to get the current inode");
            return -1;
        }
        if (curr_inode == host) {
            break;
        }
        curr_hlist_node = BPF_CORE_READ(curr_hlist_node, next);
        if (curr_hlist_node == NULL) {
            BPFLOGD(BPF_TRUE, "failed to get the next hlist_node");
            break;
        }
    }

    unsigned int flags = BPF_CORE_READ(curr_dentry, d_flags);
    flags = (flags & 0xFFFFFF) >> FILE_TYPE_BITS;
    if (flags != DCACHE_DIRECTORY_TYPE && flags != DCACHE_REGULAR_TYPE) {
        return 0;
    }

    size_t pos = 0;
    const u32 MAX_BACKTRACE_DEPTH = 20;
    for (u32 cnt = MAX_BACKTRACE_DEPTH; cnt != 0; --cnt) {
        if (err || curr_dentry == NULL) {
            break;
        }
        unsigned int name_len = BPF_CORE_READ(curr_dentry, d_name.len);
        const u8 *name = BPF_CORE_READ(curr_dentry, d_name.name);
        if (name_len <= 1) {
            break;
        }
        long dentry_name_len = bpf_probe_read_kernel_str(filename + pos, MAX_DENTRY_NAME_LEN, name);
        if (dentry_name_len <= 1) {
            BPFLOGD(BPF_TRUE, "failed to read dentry name from kernel stack buffer");
            break;
        }
        pos += (size_t)dentry_name_len;
        filename[pos - 1] = '/';
        struct dentry *temp_dentry = BPF_CORE_READ(curr_dentry, d_parent);
        if (temp_dentry == curr_dentry || temp_dentry == NULL) {
            break;
        }
        curr_dentry = temp_dentry;
    }
    pos += get_mountpoint_by_inode(filename + pos, len - pos, host);
    return pos + 1;
}

static __always_inline
int get_filename_by_bio(char *filename, const size_t len, const struct bio *bio)
{
    if (filename == NULL || len == 0 || bio == NULL) {
        BPFLOGD(BPF_TRUE, "get_filename_by_bio() error: invalid argument");
        return -1;
    }
    struct inode *host = BPF_CORE_READ(bio, bi_io_vec, bv_page, mapping, host);
    if (host == NULL) {
        BPFLOGD(BPF_TRUE, "failed to get the bio associated inode");
        return -1;
    }

    return get_filename_by_inode(filename, len, host);
}

static __always_inline
struct file* get_file_by_fd(const struct files_struct *files, const unsigned int fd)
{
    if (files == NULL) {
        BPFLOGD(BPF_TRUE, "get_file_by_fd() error: invalid argument");
        return NULL;
    }
    if (fd >= BPF_CORE_READ(files, fdt, max_fds)) {
        BPFLOGD(BPF_TRUE, "get_file_by_fd() error: invalid argument");
        return NULL;
    }
    struct file **fd_array = BPF_CORE_READ(files, fdt, fd);
    if (fd_array == NULL) {
        BPFLOGD(BPF_TRUE, "failed to get fd array");
        return NULL;
    }
    struct file *filp = NULL;
    int err = bpf_probe_read_kernel(&filp, sizeof(filp), &fd_array[fd]);
    if (err || filp == NULL) {
        BPFLOGD(BPF_TRUE, "failed to get file");
        return NULL;
    }
    return filp;
}

static __always_inline
int get_filename_by_file(char *filename, const size_t len, const struct file *filp)
{
    if (filename == NULL || filp == NULL || len == 0) {
        BPFLOGD(BPF_TRUE, "get_filename_by_file() error: invalid argument");
        return -1;
    }
    struct inode *f_inode = BPF_CORE_READ(filp, f_inode);
    if (f_inode == NULL) {
        BPFLOGD(BPF_TRUE, "failed to get inode");
        return -1;
    }
    return get_filename_by_inode(filename, len, f_inode);
}

static __always_inline
int emit_strtrace_event(u64 stime, u32 type, const void *addr, u32 stracer)
{
    if (addr == NULL) {
        BPFLOGD(BPF_TRUE, "strtrace event discarded: invalid argument");
        return -1;
    }
    const u64 event_size = sizeof(struct strtrace_cmplt_event_t);
    struct strtrace_cmplt_event_t *cmplt_event = bpf_ringbuf_reserve(&bpf_ringbuf_map, event_size, 0);
    if (cmplt_event == NULL) {
        BPFLOGD(BPF_TRUE, "failed to reserve space for strtrace event from BPF ringbuffer");
        return -1;
    }
    __builtin_memset(cmplt_event, 0, event_size);
    cmplt_event->tracer = STRTRACE;
    cmplt_event->start_event.type = type;
    cmplt_event->start_event.stracer = stracer;
    cmplt_event->start_event.stime = stime;
    cmplt_event->start_event.addr = addr;
    cmplt_event->pid = bpf_get_current_pid_tgid();
    cmplt_event->tgid = (bpf_get_current_pid_tgid() >> SHIFT_32);
    int err = 0;
    switch (stracer) {
        case BIOTRACE: {
            err = get_filename_by_bio(cmplt_event->filename, MAX_FILENAME_LEN, addr);
            break;
        }
        case FSTRACE: {
            if (type == SYS_OPENAT2) {
                err = bpf_probe_read_user_str(cmplt_event->filename, MAX_FILENAME_LEN, addr);
                break;
            }
            if (type == SYS_CLOSE) {
                const struct sys_close_args_t *args = (const struct sys_close_args_t *) addr;
                const struct file *filp = get_file_by_fd(args->files, args->fd);
                err = get_filename_by_file(cmplt_event->filename, MAX_FILENAME_LEN, filp);
                break;
            }
            BPFLOGD(BPF_TRUE, "strtrace event discarded: bad source event type = %d of fstrace", type);
            bpf_ringbuf_discard(cmplt_event, BPF_RB_NO_WAKEUP);
            return -1;
        }
        default: {
            BPFLOGD(BPF_TRUE, "strtrace event discarded: bad strtrace source tracer = %d", stracer);
            bpf_ringbuf_discard(cmplt_event, BPF_RB_NO_WAKEUP);
            return -1;
        }
    }

    if (err <= 0) {
        BPFLOGD(BPF_TRUE, "strtrace event discarded: failed to read path for tracer = %d", stracer);
        bpf_ringbuf_discard(cmplt_event, BPF_RB_NO_WAKEUP);
        return -1;
    }
    cmplt_event->len = (__u32)err;
    bpf_ringbuf_submit(cmplt_event, BPF_RB_FORCE_WAKEUP);
    return 0;
}

static __always_inline
u32 get_biotrace_event_type_by_flags(unsigned int cmd_flags)
{
    if (cmd_flags & REQ_META) {
        if ((cmd_flags & REQ_OP_MASK) == REQ_OP_READ) {
            return BIO_METADATA_READ;
        }
        if ((cmd_flags & REQ_OP_MASK) == REQ_OP_WRITE) {
            return BIO_METADATA_WRITE;
        }
        return 0;
    }
    if (cmd_flags & REQ_SWAP) {
        if ((cmd_flags & REQ_OP_MASK) == REQ_OP_READ) {
            return BIO_PAGE_IN;
        }
        if ((cmd_flags & REQ_OP_MASK) == REQ_OP_WRITE) {
            return BIO_PAGE_OUT;
        }
        return 0;
    }
    if ((cmd_flags & REQ_OP_MASK) == REQ_OP_READ) {
        return BIO_DATA_READ;
    }
    if ((cmd_flags & REQ_OP_MASK) == REQ_OP_WRITE) {
        return BIO_DATA_WRITE;
    }
    return 0;
}
/***************************** inline funcs END **********************************/

/***************************** pftrace BPF progs BEGING *****************************/
SEC("kprobe/__do_fault")
int BPF_KPROBE(__do_fault_entry, struct vm_fault *vmf)
{
    if (check_current_pid(-1, -1) != 0) {
        // not any one of target processes, skip it
        return 0;
    }
    return handle_pftrace_start_event(vmf, (u32) PF_FILE_BACKED_IN);
}

SEC("kretprobe/__do_fault")
int BPF_KRETPROBE(__do_fault_exit, int64_t vmf_flags)
{
    if (check_current_pid(-1, -1) != 0) {
        // not any one of target processes, skip it
        return 0;
    }
    return emit_event(ctx, vmf_flags, PFTRACE);
}

SEC("kprobe/do_swap_page")
int BPF_KPROBE(do_swap_page_entry, struct vm_fault *vmf)
{
    if (check_current_pid(-1, -1) != 0) {
        // not any one of target processes, skip it
        return 0;
    }
    return handle_pftrace_start_event(vmf, (u32) PF_SWAP_FROM_DISK);
}

SEC("kretprobe/do_swap_page")
int BPF_KRETPROBE(do_swap_page_exit, int64_t vmf_flags)
{
    if (check_current_pid(-1, -1) != 0) {
        // not any one of target processes, skip it
        return 0;
    }
    return emit_event(ctx, vmf_flags, PFTRACE);
}

SEC("kprobe/do_wp_page")
int BPF_KPROBE(do_wp_page_entry, struct vm_fault *vmf)
{
    if (check_current_pid(-1, -1) != 0) {
        // not any one of target processes, skip it
        return 0;
    }
    return handle_pftrace_start_event(vmf, (u32) PF_COPY_ON_WRITE);
}

SEC("kretprobe/do_wp_page")
int BPF_KRETPROBE(do_wp_page_exit, int64_t vmf_flags)
{
    if (check_current_pid(-1, -1) != 0) {
        // not any one of target processes, skip it
        return 0;
    }
    return emit_event(ctx, vmf_flags, PFTRACE);
}
/*************************** pftrace BPF progs END *******************************/

/**************************** bio BPF progs BEGING *******************************/
static __always_inline
int handle_blk_issue(struct request *rq)
{
    if (rq == NULL) {
        BPFLOGD(BPF_TRUE, "request is NULL");
        return 0;
    }
    const u64 start_event_map_key = (const u64)rq;
    struct start_event_t start_event = {};
    __builtin_memset(&start_event, 0, sizeof(start_event));
    struct biotrace_start_event_t *bio_se = &start_event.bio_se;
    bio_se->stime = bpf_ktime_get_ns();
    bio_se->type = get_biotrace_event_type_by_flags(BPF_CORE_READ(rq, cmd_flags));
    if (bio_se->type == 0) {
        BPFLOGD(BPF_TRUE, "failed to get biotrace event type");
        return 0;
    }
    emit_strtrace_event(bio_se->stime, bio_se->type, BPF_CORE_READ(rq, bio), BIOTRACE);
    u64 tgid_pid = bpf_get_current_pid_tgid();
    bio_se->pid = (u32) tgid_pid;
    bio_se->tgid = (u32) (tgid_pid >> SHIFT_32);
    bpf_get_current_comm(bio_se->comm, MAX_COMM_LEN);
    bio_se->size = BPF_CORE_READ(rq, bio, bi_iter.bi_size);
    bpf_map_update_elem(&start_event_map, &start_event_map_key, &start_event, BPF_ANY);
    return 0;
}

SEC("tp_btf/block_rq_issue")
int block_issue(u64 *ctx)
{
    if (check_current_pid(-1, -1) != 0) {
        return 0;
    }
    if (LINUX_KERNEL_VERSION < KERNEL_VERSION(5, 10, 137)) {
        return handle_blk_issue((void *)ctx[1]);
    } else {
        return handle_blk_issue((void *)ctx[0]);
    }
}

SEC("kprobe/blk_update_request")
int BPF_PROG(blk_update_request, struct request *rq)
{
    if (check_current_pid(-1, -1) != 0) {
        return 0;
    }
    u64 ctime = bpf_ktime_get_ns();
    const u64 event_size = sizeof(struct biotrace_cmplt_event_t);
    struct biotrace_cmplt_event_t *cmplt_event= bpf_ringbuf_reserve(&bpf_ringbuf_map, event_size, 0);
    if (cmplt_event == NULL) {
        BPFLOGD(BPF_TRUE, "failed to reserve space for biotrace event from BPF ringbuffer");
        return 0;
    }
    __builtin_memset(cmplt_event, 0, event_size);
    const u64 start_event_map_key = (const u64)rq;
    const struct biotrace_start_event_t *start_event = bpf_map_lookup_elem(&start_event_map, &start_event_map_key);
    if (start_event == NULL) {
        bpf_ringbuf_discard(cmplt_event, BPF_RB_NO_WAKEUP);
        return 0;
    }
    cmplt_event->start_event = *start_event;
    if (cmplt_event->start_event.type == 0) {
        BPFLOGI(BPF_TRUE, "biotrace event discarded: invalide biotrace start event");
        bpf_ringbuf_discard(cmplt_event, BPF_RB_NO_WAKEUP);
        return 0;
    }
    cmplt_event->tracer = BIOTRACE;
    cmplt_event->ctime = ctime;
    cmplt_event->prio = BPF_CORE_READ(rq, bio, bi_ioprio);
    cmplt_event->blkcnt = BPF_CORE_READ(rq, bio, bi_iter.bi_sector);
    if (unwind_stack()) {
        cmplt_event->nips = bpf_get_stack(ctx, cmplt_event->ips, g_stack_limit * sizeof(u64), 0) / sizeof(u64);
    }
    bpf_ringbuf_submit(cmplt_event, BPF_RB_FORCE_WAKEUP);
    return 0;
}
/*************************** bio BPF progs END *******************************/

/*************************** user BPF progs START ****************************/
SEC("uretprobe//system/lib/ld-musl-aarch64.so.1:dlopen")
int BPF_KRETPROBE(uretprobe_dlopen, void *ret)
{
    if (check_current_pid(-1, -1) != 0) {
        return 0;
    }
    if (ret == NULL) {
        return 0;
    }

    const u64 event_size = sizeof(struct dlopen_trace_start_event_t);
    struct dlopen_trace_start_event_t *start_event = bpf_ringbuf_reserve(&bpf_ringbuf_map, event_size, 0);
    if (start_event == NULL) {
        BPFLOGD(BPF_TRUE, "failed to reserve space for biotrace event from BPF ringbuffer");
        return -1;
    }
    start_event->type = DLOPEN_TRACE;
    u64 tgid_pid = bpf_get_current_pid_tgid();
    start_event->tgid = (u32)(tgid_pid >> SHIFT_32);
    bpf_ringbuf_submit(start_event, BPF_RB_FORCE_WAKEUP);
    return 0;
}
/*************************** user BPF progs END ****************************/
#include "fstrace_progs.h"

char _license[] SEC("license") = "GPL";
u32 _version SEC("version") = LINUX_VERSION_CODE;
