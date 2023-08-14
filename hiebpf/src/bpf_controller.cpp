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

#include <cerrno>
#include <chrono>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <memory>
#include <mutex>

#include <fcntl.h>
#include <unistd.h>
#include <linux/bpf.h>
#include <linux/perf_event.h>
#include <sys/resource.h>

#include "bpf.h"
#include "elf_file.h"
#include "libbpf_logger.h"
#include "bpf_controller.h"

namespace {
std::unique_ptr<LIBBPFLogger> libbpfLogger {nullptr};
const std::string THIRD_PARTY_MUSL_ADDR = "/system/lib/ld-musl-aarch64.so.1";
constexpr int32_t SYM_32_VALUE_OFFSET = 4;
constexpr int32_t SYM_64_VALUE_OFFSET = 8;
constexpr int32_t WIDE_SIXTEEN = 16;
} // namespace

int BPFController::LIBBPFPrintFunc(enum libbpf_print_level level, const char *format, va_list args)
{
    if (libbpfLogger) {
        return libbpfLogger->Printf(level, format, args);
    }
    return 0;
}

BPFController::~BPFController()
{
    Stop();
    if (rb_) {
        // release bpf ringbuffer
        ring_buffer__free(rb_);
        rb_ = nullptr;
    }
    if (ips_) {
        delete[] ips_;
        ips_ = nullptr;
    }
    if (skel_) {
        hiebpf_bpf__destroy(skel_);
        skel_ = nullptr;
    }

    for (size_t k = 0; k < receivers_.size(); ++k) {
        receivers_[k]->Stop();
    }
    if (bpfLogReader_) {
        bpfLogReader_->Stop();
    }
}

std::unique_ptr<BPFController> BPFController::MakeUnique(const BPFConfig& config)
{
    std::unique_ptr<BPFController> bpfctlr {new(std::nothrow) BPFController {config}};
    CHECK_NOTNULL(bpfctlr, nullptr, "failed to instantiate BPFController");
    HHLOGI(true, "BPFController instantiated");

    CHECK_TRUE(bpfctlr->VerifyConfigurations() == 0, nullptr, "failed to verify config");
    HHLOGI(true, "BPFConfig verified");

    CHECK_TRUE(bpfctlr->SetUpBPF() == 0, nullptr, "failed to set up BPF");
    HHLOGI(true, "BPF setup done");

    return bpfctlr;
}

static inline int VerifyDumpEvents(const __u32 nr)
{
    CHECK_TRUE(nr <= BPFController::DUMP_EVENTS_LIMIT, -1, "dump events exceeds limit");
    return 0;
}

static inline int VerifyTraceDuration(const __u32 duration)
{
    CHECK_TRUE(duration <= BPFController::TRACE_DURATION_LIMIT, -1, "trace duration exceeds limit");
    return 0;
}

static inline int VerifyMaxStackDepth(const __u32 depth)
{
    CHECK_TRUE(depth <= MAX_STACK_LIMIT, -1, "max stack depth exceeds limit");
    return 0;
}

int BPFController::VerifySelectEventGroups(const std::set<HiebpfEventGroup> &selectEventGroups)
{
    CHECK_TRUE(!selectEventGroups.empty(), -1, "VerifySelectEventGroups() failed: event group list is empty");
    selectEventGroups_ = selectEventGroups;
    return 0;
}

int BPFController::VerifyConfigurations()
{
    CHECK_TRUE(VerifySelectEventGroups(config_.selectEventGroups_) == 0, -1, "VerifySelectEventGroups fail");
    HHLOGI(true, "VerifySelectEventGroups() done");
    CHECK_TRUE(VerifyDumpEvents(config_.dumpEvents_) == 0, -1,
               "VerifyDumpEvents() failed: dump events = %u", config_.dumpEvents_);
    HHLOGI(true, "VerifyDumpEents() done");
    CHECK_TRUE(VerifyTraceDuration(config_.traceDuration_) == 0, -1,
               "VerifyTraceDuration() failed: duration = %u", config_.traceDuration_);
    HHLOGI(true, "VerifyTraceDuration() done");
    CHECK_TRUE(VerifyMaxStackDepth(config_.maxStackDepth_) == 0, -1,
               "VerifyMaxStackDepth() failed: max stack depth = %u", config_.maxStackDepth_);
    HHLOGI(true, "VerifyMaxStackDepth() done");
    return 0;
}

int BPFController::SetUpBPF()
{
    CHECK_TRUE(ConfigLIBBPFLogger() == 0, -1, "failed to configure LIBBPF logger");
    HHLOGI(true, "ConfigLIBBPFLogger() done");

    // set up libbpf deubug level
    libbpf_set_strict_mode(LIBBPF_STRICT_ALL);
    // set RLIMIT_MEMLOCK
    struct rlimit r = {RLIM_INFINITY, RLIM_INFINITY};
    setrlimit(RLIMIT_MEMLOCK, &r);

    skel_ = hiebpf_bpf__open();
    int err = libbpf_get_error(skel_);
    CHECK_TRUE(!err, err, "failed to open BPF skeleton: %s", strerror(-err));
    HHLOGI(true, "BPF skeleton opened");
    if (config_.unwindStack_) {
        ips_ = new(std::nothrow) __u64[config_.maxStackDepth_];
        CHECK_NOTNULL(ips_, -1, "failed to allocate memory for ips");
    }
    HHLOGI(true, "allocate ips buffer done");
    dataFile_ = HiebpfDataFile::MakeShared(config_.cmd_, config_.outputFile_);
    CHECK_NOTNULL(dataFile_, -1, "failed to make hiebpf data file");
    CHECK_TRUE(FilterProgByEvents() == 0, -1, "failed to load BPF objects");
    HHLOGI(true, "make HiebpfDataFile done");
    skel_->rodata->g_stack_limit = config_.maxStackDepth_;
    err = hiebpf_bpf__load(skel_);
    CHECK_TRUE(!err, err, "failed to load BPF skeleton: %s", strerror(-err));
    CHECK_TRUE(ConfigureBPF() == 0, -1, "failed to configure BPF");
    HHLOGI(true, "BPF configuration done");

    return 0;
}

int BPFController::FilterProgByEvents()
{
    // check each one hiebpf_bpf.progs in hiebpf.skel.h
    // hiebpf_bpf.progs is autoload by default
    FilterFsGroup();

    if (selectEventGroups_.find(MEM_GROUP_ALL) == selectEventGroups_.end()) {
        bpf_program__set_autoload(skel_->progs.__do_fault_entry, false);
        bpf_program__set_autoload(skel_->progs.__do_fault_exit, false);

        bpf_program__set_autoload(skel_->progs.do_swap_page_entry, false);
        bpf_program__set_autoload(skel_->progs.do_swap_page_exit, false);

        bpf_program__set_autoload(skel_->progs.do_wp_page_entry, false);
        bpf_program__set_autoload(skel_->progs.do_wp_page_exit, false);
    }
    if (selectEventGroups_.find(BIO_GROUP_ALL) == selectEventGroups_.end()) {
        bpf_program__set_autoload(skel_->progs.block_issue, false);
        bpf_program__set_autoload(skel_->progs.blk_update_request, false);
    } else {
        dataFile_->WriteKernelSymbol();
    }
    return 0;
}

void BPFController::FilterFsGroup()
{
    if (selectEventGroups_.find(FS_GROUP_ALL) != selectEventGroups_.end()) {
        return;
    }

    if (selectEventGroups_.find(FS_GROUP_OPEN) == selectEventGroups_.end()) {
        bpf_program__set_autoload(skel_->progs.do_sys_openat2_entry, false);
        bpf_program__set_autoload(skel_->progs.do_sys_openat2_exit, false);
    }
    if (selectEventGroups_.find(FS_GROUP_READ) == selectEventGroups_.end()) {
        bpf_program__set_autoload(skel_->progs.do_readv_entry, false);
        bpf_program__set_autoload(skel_->progs.do_readv_exit, false);

        bpf_program__set_autoload(skel_->progs.do_preadv_entry, false);
        bpf_program__set_autoload(skel_->progs.do_preadv_exit, false);

        bpf_program__set_autoload(skel_->progs.ksys_read_entry, false);
        bpf_program__set_autoload(skel_->progs.ksys_read_exit, false);

        bpf_program__set_autoload(skel_->progs.ksys_pread64_entry, false);
        bpf_program__set_autoload(skel_->progs.ksys_pread64_exit, false);
    }
    if (selectEventGroups_.find(FS_GROUP_WRITE) == selectEventGroups_.end()) {
        bpf_program__set_autoload(skel_->progs.do_writev_entry, false);
        bpf_program__set_autoload(skel_->progs.do_writev_exit, false);

        bpf_program__set_autoload(skel_->progs.__arm64_sys_pwritev_entry, false);
        bpf_program__set_autoload(skel_->progs.__arm64_sys_pwritev_exit, false);
        bpf_program__set_autoload(skel_->progs.__arm64_sys_pwritev2_entry, false);
        bpf_program__set_autoload(skel_->progs.__arm64_sys_pwritev2_exit, false);
        bpf_program__set_autoload(skel_->progs.__arm64_compat_sys_pwritev_entry, false);
        bpf_program__set_autoload(skel_->progs.__arm64_compat_sys_pwritev_exit, false);
        bpf_program__set_autoload(skel_->progs.__arm64_compat_sys_pwritev2_entry, false);
        bpf_program__set_autoload(skel_->progs.__arm64_compat_sys_pwritev2_exit, false);

        bpf_program__set_autoload(skel_->progs.ksys_write_entry, false);
        bpf_program__set_autoload(skel_->progs.ksys_write_exit, false);

        bpf_program__set_autoload(skel_->progs.ksys_pwrite64_entry, false);
        bpf_program__set_autoload(skel_->progs.ksys_pwrite64_exit, false);
    }
    if (selectEventGroups_.find(FS_GROUP_CLOSE) == selectEventGroups_.end()) {
        bpf_program__set_autoload(skel_->progs.__close_fd_entry, false);
        bpf_program__set_autoload(skel_->progs.__close_fd_exit, false);
    }
}

static int InitTracerPid(const int fd, bool excludeTracer)
{
    int32_t pid = -1;
    if (excludeTracer) {
        /* we write the tracer pid into BPF map to notify BPF progs
         * to exclude the tracer itself
        */
        pid = static_cast<int32_t>(getpid());
        CHECK_TRUE(pid >= 0, -1, "failed to get current pid");
    }
    constexpr __u32 pididx {TRACER_PID_INDEX};
    int err = bpf_map_update_elem(fd, &pididx, &pid, BPF_ANY);
    CHECK_TRUE(!err, -1, "failed to update tracer pid %d in config_var_map", pid);
    return 0;
}

static inline int InitBPFLogLevel(const int fd, const __u32 level)
{
    if (level == BPF_LOG_NONE) {
        HHLOGD(true, "bpf log level is NONE!");
        return 0;
    }
    constexpr __u32 levelidx {BPF_LOG_LEVEL_INDEX};
    int err = bpf_map_update_elem(fd, &levelidx, &level, BPF_ANY);
    CHECK_TRUE(!err, -1, "failed to set bpf log level in config_var_map");
    return 0;
}

static inline int InitUnwindFlag(const int fd, bool unwind)
{
    constexpr __u32 uflagidx {UNWIND_FLAG_INDEX};
    __u32 uflag {0};
    if (unwind) {
        uflag = 1;
    }
    int err = bpf_map_update_elem(fd, &uflagidx, &uflag, BPF_ANY);
    CHECK_TRUE(!err, -1, "failed to set unwind stack flag in config_var_map");
    return 0;
}

int BPFController::InitBPFVariables() const
{
    int fd = bpf_map__fd(skel_->maps.config_var_map);
    CHECK_TRUE(fd >= 0, -1, "failed to get fd of config_var_map");
    HHLOGI(true, "InitBPFVariables() done");
    CHECK_TRUE(InitTracerPid(fd, config_.excludeTracer_) == 0, -1,
               "failed to init tracer pid in config_var_map");
    HHLOGI(true, "InitTracerPid() done");
    CHECK_TRUE(InitBPFLogLevel(fd, config_.BPFLogLevel_) == 0, -1,
               "failed to init BPF log level in config_var_map");
    HHLOGI(true, "InitBPFLogLevel() done");
    CHECK_TRUE(InitUnwindFlag(fd, config_.unwindStack_) == 0, -1,
               "failed to init unwind stack flag in config_var_map");
    HHLOGI(true, "InitUnwindFlag() done");
    return 0;
}

int BPFController::FillTargetPidMap() const
{
    int fd = bpf_map__fd(skel_->maps.target_pid_map);
    CHECK_TRUE(fd >= 0, -1, "failed to get fd of target_pid_map");
    int index {0};
    uint32_t val {1}; // target_pid_Map[0] = 1 means tracing all processes
    int err {0};
    int numPids {config_.targetPids_.size()};
    HHLOGD(true, "target pid num = %d", numPids);
    if (numPids == 0) {
        // no target pid specified, trace all processes
        err = bpf_map_update_elem(fd, &index, &val, BPF_ANY);
        CHECK_TRUE(!err, -1, "failed to set target pid = %u", val);
        return 0;
    }
    if (numPids > MAX_TARGET_PIDS) {
        HHLOGW(true, "BPFController WARN: number of target pids exceeds the maximum limit");
        numPids = MAX_TARGET_PIDS;
    }
    for (index = 1; index <= numPids; ++index) {
        val = static_cast<uint32_t>(config_.targetPids_[index - 1]);
        HHLOGD(true, "target pid = %u", val);
        std::cout << "target pid = " << val << std::endl;
        err = bpf_map_update_elem(fd, &index, &val, BPF_ANY);
        CHECK_TRUE(!err, -1, "failed to set target pid = %d", val);
    }
    return 0;
}

inline int BPFController::ConfigBPFLogger()
{
    if (config_.BPFLogLevel_ == BPF_LOG_NONE) {
        HHLOGD(true, "bpf log level is NONE!");
        return 0;
    }
#if defined(BPF_LOGGER_DEBUG) || defined(BPF_LOGGER_INFO) || defined(BPF_LOGGER_WARN) ||    \
    defined(BPF_LOGGER_ERROR) || defined(BPF_LOGGER_FATAL)
    bpfLogReader_ = BPFLogReader::MakeUnique(config_.BPFLogFile_);
    CHECK_NOTNULL(bpfLogReader_, -1, "failed to initialize BPFLogReader");
#endif
    return 0;
}

inline int BPFController::ConfigLIBBPFLogger() const
{
    // set up libbpf print callback
    HHLOGI(true, "libbpf logger: file = %s, level = %d", config_.LIBBPFLogFile_.c_str(), config_.LIBBPFLogLevel_);
    libbpf_set_print(BPFController::LIBBPFPrintFunc);
    if (config_.LIBBPFLogLevel_ == LIBBPF_NONE) {
        HHLOGD(true, "libbpf log level is NONE!");
        return 0;
    }
    libbpfLogger = LIBBPFLogger::MakeUnique(config_.LIBBPFLogFile_, config_.LIBBPFLogLevel_);
    CHECK_NOTNULL(libbpfLogger, -1, "libbpfLogger is nullptr");
    return 0;
}

int BPFController::ConfigReceivers()
{
    if (config_.dumpEvents_ == 0) {
        rb_ = ring_buffer__new(
            bpf_map__fd(skel_->maps.bpf_ringbuf_map),
            BPFController::HandleEvent,
            this, nullptr);
        int err = libbpf_get_error(rb_);
        CHECK_TRUE(!err, err, "failed to make BPF ring buffer: %s", strerror(-err));
        if (config_.pipelines_ == 0) {
            config_.pipelines_ = MIN_PIPELINES_LIMIT;
        }
        for (__u32 cnt = config_.pipelines_; cnt != 0; --cnt) {
            receivers_.push_back(BPFEventReceiver::MakeShared(dataFile_));
        }
        CHECK_TRUE(receivers_.size() == config_.pipelines_, -1, "failed to make BPF event receivers");
        last_ = 0;
    } else {
        rb_ = ring_buffer__new(
            bpf_map__fd(skel_->maps.bpf_ringbuf_map),
            BPFController::DumpEvent,
            this, nullptr);
        int err = libbpf_get_error(rb_);
        CHECK_TRUE(!err, err, "failed to make BPF ring buffer: %s", strerror(-err));
    }
    return 0;
}

uint64_t BPFController::GetSymOffset(const std::string &path, const std::string &symbol)
{
    CHECK_TRUE(access(path.c_str(), F_OK) == 0, 0, "the file does not exist");
    using namespace OHOS::Developtools::Hiebpf;
    std::unique_ptr<ElfFile> elfFile = ElfFile::MakeUnique(path);
    CHECK_NOTNULL(elfFile, 0, "ELF file open failed");
    const std::string dynsym {".dynsym"};
    CHECK_TRUE(elfFile->shdrs_.find(dynsym) != elfFile->shdrs_.end(), 0, "section dynsym failed to obtain data");
    const auto &sym = elfFile->shdrs_[dynsym];
    const uint8_t *symData = elfFile->GetSectionData(sym->secIndex_);

    const std::string dynstr {".dynstr"};
    CHECK_TRUE(elfFile->shdrs_.find(dynstr) != elfFile->shdrs_.end(), 0, "section dynstr failed to obtain data");
    const auto &str = elfFile->shdrs_[dynstr];
    const uint8_t *strData = elfFile->GetSectionData(str->secIndex_);

    uint32_t st_name = 0;
    uint64_t stepLength = 0;
    uint64_t vaddr = 0;
    while (stepLength < sym->secSize_) {
        int ret = memcpy_s(&st_name, sizeof(uint32_t), symData + stepLength, sizeof(uint32_t));
        CHECK_TRUE(ret == EOK, 0, "failed to memcpy symData");
        auto name = const_cast<uint8_t*>(strData + st_name);
        if (name != nullptr && std::string(reinterpret_cast<char*>(name)).compare(symbol) == 0) {
            int32_t valueOffset = sym->secEntrySize_ == sizeof(Elf64_Sym) ? SYM_64_VALUE_OFFSET : SYM_32_VALUE_OFFSET;
            int32_t valueSize = valueOffset == SYM_64_VALUE_OFFSET ? sizeof(uint64_t) : sizeof(uint32_t);
            ret = memcpy_s(&vaddr, sizeof(uint64_t), symData + stepLength + valueOffset, valueSize);
            CHECK_TRUE(ret == EOK, 0, "failed to memcpy symData");
            break;
        }
        stepLength += sym->secEntrySize_;
    }
    CHECK_TRUE(vaddr != 0, 0, "get vaddr failed");

    const std::string text {".text"};
    CHECK_TRUE(elfFile->shdrs_.find(text) != elfFile->shdrs_.end(), 0, "section text failed to obtain data");
    const auto &textPtr = elfFile->shdrs_[text];
    return vaddr - textPtr->secVaddr_ + textPtr->fileOffset_;
}

int32_t BPFController::ConfigDlopenBPFProg()
{
    uint64_t symOffset = GetSymOffset(THIRD_PARTY_MUSL_ADDR, "dlopen");
    CHECK_TRUE(symOffset != 0, -1, "get symOffset failed");
    skel_->links.uretprobe_dlopen = bpf_program__attach_uprobe(skel_->progs.uretprobe_dlopen,
                                                               true,
                                                               -1,
                                                               THIRD_PARTY_MUSL_ADDR.c_str(),
                                                               symOffset);
    CHECK_TRUE(skel_->links.uretprobe_dlopen, -1, "failed to attach uretprobe_dlopen");
    return 0;
}

int BPFController::ConfigureBPF()
{
    CHECK_TRUE(InitBPFVariables() == 0, -1, "failed to fill config_var_map");
    HHLOGI(true, "InitBPFVariables() done");
    CHECK_TRUE(FillTargetPidMap() == 0, -1, "failed to fill target_pid_map");
    HHLOGI(true, "FillTargetPidMap() done");
    CHECK_TRUE(ConfigBPFLogger() == 0, -1, "failed to configure BPF logger");
    HHLOGI(true, "ConfigBPFLogger() done");
    CHECK_TRUE(ConfigReceivers() == 0, -1, "failed to configure BPF ringbuffer");
    HHLOGI(true, "ConfigReceivers() done");
    CHECK_TRUE(ConfigDlopenBPFProg() == 0, -1, "failed to configure user BPF prog");
    return 0;
}

int BPFController::Start()
{
#if defined(BPF_LOGGER_DEBUG) || defined(BPF_LOGGER_INFO) || defined(BPF_LOGGER_WARN) ||    \
    defined(BPF_LOGGER_ERROR) || defined(BPF_LOGGER_FATAL)
    CHECK_TRUE(StartBPFLogReader() == 0, -1, "failed to start BPF log reader");
#endif
    HHLOGI(true, "BPF log reader started");
    CHECK_TRUE(StartReceivers() == 0, -1, "failed to start receivers");
    HHLOGI(true, "receivers started");
    // activate events
    int err = hiebpf_bpf__attach(skel_);
    CHECK_TRUE(!err, -1, "failed to attach bpf object: %s", strerror(-err));
    HHLOGI(true, "BPF events activated");

    const auto endTime = std::chrono::steady_clock::now() + std::chrono::seconds(config_.traceDuration_);
    while (!loopStop_) {
        if (BPFEventLoopOnce() != 0) {
            printf("libbpf error occured, hiebpf exit\n");
            err = -1;
            break;
        }
        if (std::chrono::steady_clock::now() >= endTime) {
            printf("timeout(%us), hiebpf exit\n", config_.traceDuration_);
            break;
        }
    }
    // receivers_ must stop after BPFEventLoopOnce();
    for (size_t k = 0; k < receivers_.size(); ++k) {
        receivers_[k]->Stop();
    }
    if (bpfLogReader_) {
        bpfLogReader_->Stop();
    }

    HHLOGI(true, "hiebpf stopped");

    return err;
}

void BPFController::Stop()
{
    loopStop_ = true;
}

int BPFController::HandleEvent(void *ctx, void *data, size_t dataSize)
{
    // get the next running receiver
    BPFController *bpfctlr = static_cast<BPFController *>(ctx);
    auto wrecv = bpfctlr->NextActiveReceiver();
    auto receiver = wrecv.lock();
    if (receiver == nullptr) {
        HHLOGF(true, "all receivers have stopped, will stop BPF event loop");
        bpfctlr->Stop();
        return -1;
    }

    // move data and notify receiver
    int ret = receiver->Put(data, dataSize);
    HHLOGE((ret < 0), "event lost: failed to move data to receiver"); // try other receivers ?
    HHLOGF(
        (0 <= ret and ret < static_cast<int>(dataSize)),
        "incomplete data movement: this should never happen");
    return ret;
}

static int DumpOpenat2Args(const struct fstrace_cmplt_event_t &cmpltEvent)
{
    std::cout << "\nArgs:";
    std::cout << "\n    dfd = " << cmpltEvent.start_event.openat2_args.dfd;
    std::cout << "\n    filename = " << cmpltEvent.start_event.openat2_args.filename;
    std::cout << "\n    how = " << cmpltEvent.start_event.openat2_args.how;
    return 0;
}

static int DumpReadvArgs(const struct fstrace_cmplt_event_t &cmpltEvent)
{
    std::cout << "\nArgs:";
    std::cout << "\n    fd = " << cmpltEvent.start_event.readv_args.fd;
    std::cout << "\n    vec = " << cmpltEvent.start_event.readv_args.vec;
    std::cout << "\n    vlen = " << cmpltEvent.start_event.readv_args.vlen;
    std::cout << "\n    flags = " << cmpltEvent.start_event.readv_args.flags;
    return 0;
}

static int DumpPreadvArgs(const struct fstrace_cmplt_event_t &cmpltEvent)
{
    std::cout << "\nArgs:";
    std::cout << "\n    fd = " << cmpltEvent.start_event.preadv_args.fd;
    std::cout << "\n    vec = " << cmpltEvent.start_event.preadv_args.vec;
    std::cout << "\n    vlen = " << cmpltEvent.start_event.preadv_args.vlen;
    std::cout << "\n    pos = " << cmpltEvent.start_event.preadv_args.pos;
    std::cout << "\n    flags = " << cmpltEvent.start_event.preadv_args.flags;
    return 0;
}

static int DumpReadArgs(const struct fstrace_cmplt_event_t &cmpltEvent)
{
    std::cout << "\nArgs:";
    std::cout << "\n    fd = " << cmpltEvent.start_event.read_args.fd;
    std::cout << "\n    buf = " << cmpltEvent.start_event.read_args.buf;
    std::cout << "\n    count = " << cmpltEvent.start_event.read_args.count;
    return 0;
}

static int DumpPread64Args(const struct fstrace_cmplt_event_t &cmpltEvent)
{
    std::cout << "\nArgs:";
    std::cout << "\n    fd = " << cmpltEvent.start_event.pread64_args.fd;
    std::cout << "\n    buf = " << cmpltEvent.start_event.pread64_args.buf;
    std::cout << "\n    count = " << cmpltEvent.start_event.pread64_args.count;
    std::cout << "\n    pos = " << cmpltEvent.start_event.pread64_args.pos;
    return 0;
}

static int DumpWritevArgs(const struct fstrace_cmplt_event_t &cmpltEvent)
{
    std::cout << "\nArgs:";
    std::cout << "\n    fd = " << cmpltEvent.start_event.writev_args.fd;
    std::cout << "\n    vec = " << cmpltEvent.start_event.writev_args.vec;
    std::cout << "\n    vlen = " << cmpltEvent.start_event.writev_args.vlen;
    std::cout << "\n    flags = " << cmpltEvent.start_event.writev_args.flags;
    return 0;
}

static int DumpPwritevArgs(const struct fstrace_cmplt_event_t &cmpltEvent)
{
    std::cout << "\nArgs:";
    std::cout << "\n    fd = " << cmpltEvent.start_event.pwritev_args.fd;
    std::cout << "\n    vec = " << cmpltEvent.start_event.pwritev_args.vec;
    std::cout << "\n    vlen = " << cmpltEvent.start_event.pwritev_args.vlen;
    std::cout << "\n    pos = " << cmpltEvent.start_event.pwritev_args.pos;
    std::cout << "\n    flags = " << cmpltEvent.start_event.pwritev_args.flags;
    return 0;
}

static int DumpWriteArgs(const struct fstrace_cmplt_event_t &cmpltEvent)
{
    std::cout << "\nArgs:";
    std::cout << "\n    fd = " << cmpltEvent.start_event.write_args.fd;
    std::cout << "\n    buf = " << cmpltEvent.start_event.write_args.buf;
    std::cout << "\n    count = " << cmpltEvent.start_event.write_args.count;
    return 0;
}

static int DumpPwrite64Args(const struct fstrace_cmplt_event_t &cmpltEvent)
{
    std::cout << "\nArgs:";
    std::cout << "\n    fd = " << cmpltEvent.start_event.pwrite64_args.fd;
    std::cout << "\n    buf = " << cmpltEvent.start_event.pwrite64_args.buf;
    std::cout << "\n    count = " << cmpltEvent.start_event.pwrite64_args.count;
    std::cout << "\n    pos = " << cmpltEvent.start_event.pwrite64_args.pos;
    return 0;
}

static int DumpCloseArgs(const struct fstrace_cmplt_event_t &cmpltEvent)
{
    std::cout << "\nArgs:";
    std::cout << "\n    files = " << cmpltEvent.start_event.close_args.files;
    std::cout << "\n    fd = " << cmpltEvent.start_event.close_args.fd;
    return 0;
}

static int DumpTypeAndArgs(const struct fstrace_cmplt_event_t &cmpltEvent)
{
    std::cout << "\nevent type:     ";
    switch (cmpltEvent.start_event.type) {
        case SYS_OPENAT2: std::cout << "openat2"; return DumpOpenat2Args(cmpltEvent);

        case SYS_READV: std::cout << "readv"; return DumpReadvArgs(cmpltEvent);
        case SYS_PREADV: std::cout << "preadv"; return DumpPreadvArgs(cmpltEvent);
        case SYS_READ: std::cout << "read"; return DumpReadArgs(cmpltEvent);
        case SYS_PREAD64: std::cout << "pread64"; return DumpPread64Args(cmpltEvent);

        case SYS_WRITEV: std::cout << "writev"; return DumpWritevArgs(cmpltEvent);
        case SYS_PWRITEV: std::cout << "pwritev"; return DumpPwritevArgs(cmpltEvent);
        case SYS_WRITE: std::cout << "write"; return DumpWriteArgs(cmpltEvent);
        case SYS_PWRITE64: std::cout << "pwrite64"; return DumpPwrite64Args(cmpltEvent);

        case SYS_CLOSE: std::cout << "close"; return DumpCloseArgs(cmpltEvent);
    }
    HHLOGE(true, "unreognized fstrace event type = %d", cmpltEvent.start_event.type);
    return -1;
}

int BPFController::DumpFSTraceEvent(BPFController *bpfctlr, void *data, size_t dataSize)
{
    if (dataSize != sizeof(fstrace_cmplt_event_t)) {
        std::cout << "DumpFSTraceEvent ERROR: size dismatch:"
                  << " data size = " << dataSize
                  << " fstrace event size = " << sizeof(fstrace_cmplt_event_t)
                  << std::endl;
        return -1;
    }
    struct fstrace_cmplt_event_t cmpltEvent {};
    if (memcpy_s(&cmpltEvent, sizeof(fstrace_cmplt_event_t), data, dataSize) != EOK) {
        std::cout << "failed to copy data to fstrace_cmplt_event_t" << std::endl;
        return -1;
    }
    std::cout << "\nFSTrace Event:"
              << "\ndata size:      " << dataSize;
    DumpTypeAndArgs(cmpltEvent);
    std::cout << "\nretval:         " << cmpltEvent.retval
              << "\nstart time:     " << cmpltEvent.start_event.stime
              << "\nexit time:      " << cmpltEvent.ctime
              << "\npid:            " << cmpltEvent.pid
              << "\ntgid:           " << cmpltEvent.tgid
              << "\ncomm:           " << cmpltEvent.comm
              << "\nips:            " << cmpltEvent.nips
              << "\nips:"
              << std::setw(WIDE_SIXTEEN) << std::hex;
    for (uint32_t i = 0; i < cmpltEvent.nips; ++i) {
        std::cout << "\n    " << cmpltEvent.ips[i];
    }
    std::cout << std::dec << std::endl;
    return 0;
}

int BPFController::DumpPFTraceEvent(BPFController *bpfctlr, void *data, size_t dataSize)
{
    if (dataSize != sizeof(pftrace_cmplt_event_t)) {
        std::cout << "DumpPFTraceEvent ERROR: size dismatch:"
                  << " data size = " << dataSize
                  << " pftrace event size = " << sizeof(pftrace_cmplt_event_t)
                  << std::endl;
        return -1;
    }
    struct pftrace_cmplt_event_t cmpltEvent {};
    if (memcpy_s(&cmpltEvent, sizeof(pftrace_cmplt_event_t), data, dataSize) != EOK) {
        std::cout << "failed to copy data to pftrace_cmplt_event_t" << std::endl;
        return -1;
    }
    std::cout << "PFTrace Event:"
              << "\ndata size:      " << dataSize
              << "\nevent type:     ";
    switch (cmpltEvent.start_event.type) {
        case PF_COPY_ON_WRITE:  std::cout << "Copy On  Write"; break;
        case PF_FAKE_ZERO_PAGE: std::cout << "Zero FAKE Page"; break;
        case PF_FILE_BACKED_IN: std::cout << "File Backed In"; break;
        case PF_PAGE_CACHE_HIT: std::cout << "Page Cache Hit"; break;
        case PF_SWAP_FROM_DISK: std::cout << "Swap From Disk"; break;
        case PF_SWAP_FROM_ZRAM: std::cout << "Swap From Zram"; break;
        case PF_ZERO_FILL_PAGE: std::cout << "Zero Fill Page"; break;
        default: std::cout << cmpltEvent.start_event.type;
    }
    std::cout << "\naddress:        " << cmpltEvent.start_event.addr
              << "\nsize:           " << cmpltEvent.size
              << "\nstart time:     " << cmpltEvent.start_event.stime
              << "\nexit time:      " << cmpltEvent.ctime
              << "\npid:            " << cmpltEvent.pid
              << "\ntgid:           " << cmpltEvent.tgid
              << "\ncomm:           " << cmpltEvent.comm
              << "\nips:            " << cmpltEvent.nips
              << std::setw(WIDE_SIXTEEN) << std::hex;
    for (uint32_t i = 0; i < cmpltEvent.nips; ++i) {
        std::cout << "\n    " << cmpltEvent.ips[i];
    }
    std::cout << std::dec << std::endl;
    return 0;
}

int BPFController::DumpBIOTraceEvent(BPFController *bpfctlr, void *data, size_t dataSize)
{
    if (dataSize != sizeof(biotrace_cmplt_event_t)) {
        std::cout << "DumpBIOTraceEvent ERROR: size dismatch:"
                  << " data size = " << dataSize
                  << " biotrace event size = " << sizeof(biotrace_cmplt_event_t)
                  << std::endl;
        return -1;
    }
    struct biotrace_cmplt_event_t cmpltEvent {};
    if (memcpy_s(&cmpltEvent, sizeof(biotrace_cmplt_event_t), data, dataSize) != EOK) {
        std::cout << "failed to copy data to biotrace_cmplt_event_t" << std::endl;
        return -1;
    }
    std::cout << "BIOTrace Event:"
              << "\ndata size:      " << dataSize
              << "\nevent type:     ";
    switch (cmpltEvent.start_event.type) {
        case BIO_DATA_READ: std::cout << "DATA_READ"; break;
        case BIO_DATA_WRITE: std::cout << "DATA_WRITE"; break;
        case BIO_METADATA_READ: std::cout << "METADATA_READ"; break;
        case BIO_METADATA_WRITE: std::cout << "METADATA_WRITE"; break;
        case BIO_PAGE_IN: std::cout << "PAGE_IN"; break;
        case BIO_PAGE_OUT: std::cout << "PAGE_OUT"; break;
        default: std::cout << cmpltEvent.start_event.type;
    }

    std::cout << "\nstart time:     " << cmpltEvent.start_event.stime
              << "\nexit time:      " << cmpltEvent.ctime
              << "\npid:            " << cmpltEvent.start_event.pid
              << "\ntgid:           " << cmpltEvent.start_event.tgid
              << "\ncomm:           " << cmpltEvent.start_event.comm
              << "\nprio:           " << cmpltEvent.prio
              << "\nsize:           " << cmpltEvent.start_event.size
              << "\nblkcnt:         " << cmpltEvent.blkcnt
              << "\nips:            " << cmpltEvent.nips
              << std::setw(WIDE_SIXTEEN) << std::hex;
    for (uint32_t i = 0; i < cmpltEvent.nips; ++i) {
        std::cout << "\n    " << cmpltEvent.ips[i];
    }
    std::cout << std::dec << std::endl;
    return 0;
}

int BPFController::DumpSTRTraceEvent(void *data, size_t dataSize)
{
    if (dataSize != sizeof(strtrace_cmplt_event_t)) {
        std::cout << "DumpSTRTraceEvent ERROR: size dismatch:"
                  << " data size = " << dataSize
                  << " strtrace event size = " << sizeof(strtrace_cmplt_event_t)
                  << std::endl;
        return -1;
    }
    struct strtrace_cmplt_event_t cmpltEvent {};
    if (memcpy_s(&cmpltEvent, sizeof(strtrace_cmplt_event_t), data, dataSize) != EOK) {
        std::cout << "failed to copy data to strtrace_cmplt_event_t" << std::endl;
        return -1;
    }
    std::cout << "STRTrace Event:"
              << "\ndata size:      " << dataSize
              << "\ntracer:         " << cmpltEvent.start_event.stracer
              << "\ntype:           " << cmpltEvent.start_event.type
              << "\naddress:        " << cmpltEvent.start_event.addr
              << "\nstart time:     " << cmpltEvent.start_event.stime
              << "\npid:            " << cmpltEvent.pid
              << "\ntgid:           " << cmpltEvent.tgid
              << "\nfilename len:   " << cmpltEvent.len
              << "\nfilename:       " << cmpltEvent.filename
              << std::endl;
    return 0;
}

int BPFController::DumpEvent(void *ctx, void *data, size_t dataSize)
{
    const __u32 *tracer = static_cast<const __u32 *>(data);
    BPFController *bpfctlr = static_cast<BPFController *>(ctx);
    if (bpfctlr->config_.dumpEvents_) {
        --bpfctlr->config_.dumpEvents_;
        static __u32 counter {0};
        std::cout << "\ncounter = " << ++counter;
        switch (*tracer) {
            case FSTRACE: return DumpFSTraceEvent(bpfctlr, data, dataSize);
            case PFTRACE: return DumpPFTraceEvent(bpfctlr, data, dataSize);
            case BIOTRACE: return DumpBIOTraceEvent(bpfctlr, data, dataSize);
            case STRTRACE: return DumpSTRTraceEvent(data, dataSize);
        }
        std::cout << "DumpEvent ERROR: bad tracer type = " << (*tracer) << std::endl;
    }
    return 0;
}

std::weak_ptr<BPFEventReceiver> BPFController::NextActiveReceiver()
{
    __u32 next = last_ + 1;
    __u32 total = receivers_.size();
    for (;;) {
        if (next >= total) {
            next -= total;
        }
        if (receivers_[next]->Running() or next == last_) {
            break;
        }
        ++next;
    }
    if (receivers_[next]->Running()) {
        last_ = next;
        return receivers_[last_];
    }
    return std::weak_ptr<BPFEventReceiver>();
}