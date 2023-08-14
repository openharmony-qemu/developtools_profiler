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

#ifndef BPF_LOG_WRITER_H
#define BPF_LOG_WRITER_H

#include "bpf_log.h"
#include "fstrace_types.h"

#define BPFLOG(level, expression, format, ...) {                                          \
    u32 bpf_log_level_index = BPF_LOG_LEVEL_INDEX;                                          \
    u32* bpf_log_level_ptr = bpf_map_lookup_elem(&config_var_map, &bpf_log_level_index);    \
    if (bpf_log_level_ptr) {                                                                \
        u32 bpf_log_level = BPF_LOG_NONE;                                                   \
        bpf_probe_read_kernel(&bpf_log_level, sizeof(u32), bpf_log_level_ptr);       \
        if ((expression) && (bpf_log_level <= level)) {                                     \
            char mesg[] = format;                                                           \
            bpf_trace_printk(mesg, sizeof(mesg), ##__VA_ARGS__);                            \
        }                                                                                   \
    } else {                                                                                \
        char mesg[] = "failed to get bpf log level";                                        \
        bpf_trace_printk(mesg, sizeof(mesg));                                               \
    }                                                                                       \
}

#ifdef BPF_LOGGER_DEBUG
#define BPFLOGD(expression, format, ...) BPFLOG(BPF_LOG_DEBUG, expression, format, ##__VA_ARGS__)
#define BPFLOGI(expression, format, ...) BPFLOG(BPF_LOG_INFO, expression, format, ##__VA_ARGS__)
#define BPFLOGW(expression, format, ...) BPFLOG(BPF_LOG_WARN, expression, format, ##__VA_ARGS__)
#define BPFLOGE(expression, format, ...) BPFLOG(BPF_LOG_ERROR, expression, format, ##__VA_ARGS__)
#define BPFLOGF(expression, format, ...) BPFLOG(BPF_LOG_FATAL, expression, format, ##__VA_ARGS__)

#elif defined(BPF_LOGGER_INFO)
#define BPFLOGD(expression, format, ...) {}
#define BPFLOGI(expression, format, ...) BPFLOG(BPF_LOG_INFO, expression, format, ##__VA_ARGS__)
#define BPFLOGW(expression, format, ...) BPFLOG(BPF_LOG_WARN, expression, format, ##__VA_ARGS__)
#define BPFLOGE(expression, format, ...) BPFLOG(BPF_LOG_ERROR, expression, format, ##__VA_ARGS__)
#define BPFLOGF(expression, format, ...) BPFLOG(BPF_LOG_FATAL, expression, format, ##__VA_ARGS__)

#elif defined(BPF_LOGGER_WARN)
#define BPFLOGD(expression, format, ...) {}
#define BPFLOGI(expression, format, ...) {}
#define BPFLOGW(expression, format, ...) BPFLOG(BPF_LOG_WARN, expression, format, ##__VA_ARGS__)
#define BPFLOGE(expression, format, ...) BPFLOG(BPF_LOG_ERROR, expression, format, ##__VA_ARGS__)
#define BPFLOGF(expression, format, ...) BPFLOG(BPF_LOG_FATAL, expression, format, ##__VA_ARGS__)

#elif defined(BPF_LOGGER_ERROR)
#define BPFLOGD(expression, format, ...) {}
#define BPFLOGI(expression, format, ...) {}
#define BPFLOGW(expression, format, ...) {}
#define BPFLOGE(expression, format, ...) BPFLOG(BPF_LOG_ERROR, expression, format, ##__VA_ARGS__)
#define BPFLOGF(expression, format, ...) BPFLOG(BPF_LOG_FATAL, expression, format, ##__VA_ARGS__)

#elif defined(BPF_LOGGER_FATAL)
#define BPFLOGD(expression, format, ...) {}
#define BPFLOGI(expression, format, ...) {}
#define BPFLOGW(expression, format, ...) {}
#define BPFLOGE(expression, format, ...) {}
#define BPFLOGF(expression, format, ...) BPFLOG(BPF_LOG_FATAL, expression, format, ##__VA_ARGS__)

#else
#define BPFLOGD(expression, format, ...) {}
#define BPFLOGI(expression, format, ...) {}
#define BPFLOGW(expression, format, ...) {}
#define BPFLOGE(expression, format, ...) {}
#define BPFLOGF(expression, format, ...) {}

#endif // BPF_LOGGER_LEVEL

#endif  // BPF_LOG_WRITER_H