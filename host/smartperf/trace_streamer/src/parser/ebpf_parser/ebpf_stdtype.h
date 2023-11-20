/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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
#ifndef EBPF_DATA_STD_TYPE_H
#define EBPF_DATA_STD_TYPE_H
#if is_mingw
#include "dfx_nonlinux_define.h"
#else
#include <elf.h>
#endif
#include <map>
#include "ts_common.h"
namespace SysTuning {
namespace EbpfStdtype {
struct EbpfDataHeader {
    static constexpr uint32_t EBPF_DATA_HEADER_SIZE = 1024;
    static constexpr uint64_t HEADER_MAGIC = 0x454C494646504245uLL;
    struct HeaderData {
        uint64_t magic = HEADER_MAGIC;
        uint32_t headSize = EBPF_DATA_HEADER_SIZE;
        uint32_t version;
        uint32_t clock;
        uint32_t cmdLineLen;
    } __attribute__((packed));
    static constexpr uint32_t EBPF_COMMAND_MAX_SIZE = EBPF_DATA_HEADER_SIZE - sizeof(HeaderData);
    HeaderData header;
    char cmdline[EBPF_COMMAND_MAX_SIZE] = {'\0'};
};

enum EBPF_DATA_TYPE {
    ITEM_EVENT_MAPS = 0,
    ITEM_SYMBOL_INFO,
    ITEM_EVENT_FS,
    ITEM_EVENT_VM,
    ITEM_EVENT_BIO,
    ITEM_EVENT_STR,
    ITEM_EVENT_KENEL_SYMBOL_INFO = 0x10001,
};

struct EbpfTypeAndLength {
    uint32_t type = 0;
    uint32_t length = 0;
} __attribute__((packed));
const uint32_t EBPF_TITLE_SIZE = sizeof(EbpfTypeAndLength);

struct MapsFixedHeader {
    uint64_t start = 0;
    uint64_t end = 0;
    uint32_t offset = 0;
    uint32_t pid = 0;
    uint32_t fileNameLen = 0;
} __attribute__((packed));

enum SYSTEM_ENTRY_VALUE { ELF32_SYM = 16, ELF64_SYM = 24 };

struct ElfEventFixedHeader {
    uint64_t textVaddr = 0;
    uint32_t textOffset = 0;
    uint32_t strTabLen = 0;
    uint32_t symTabLen = 0;
    uint32_t fileNameLen = 0;
    // value range: SYSTEM_ENTRY_LENGTH
    uint32_t symEntLen = 0;
} __attribute__((packed));
// the data following are
// uint8_t aa[strTabLen] // xx, bb, cc, dd
// uint8_t bb[symTabLen] //symEntLen elf64_sym, lef32_sym

const int32_t ARGS_MAX = 4;
const int32_t MAX_TRACER_SIZE = 8;
const int32_t SINGLE_IP_SIZE = 8;
const int32_t MAX_FS_EVENT_SZIE = 16;
const int32_t MAX_PROCESS_NAME_SZIE = 16;
struct FsFixedHeader {
    uint32_t pid = 0;
    uint32_t tid = 0;
    char tracerName[MAX_TRACER_SIZE] = {0};
    uint64_t startTime = 0;
    uint64_t endTime = 0;
    char eventType[MAX_FS_EVENT_SZIE] = {0};
    int32_t ret = 0;
    uint16_t nrUserIPs = 0;
    uint16_t type = 0;
    uint64_t args[ARGS_MAX] = {0};
    char processName[MAX_PROCESS_NAME_SZIE] = {0};
} __attribute__((packed));
// the data following are ips ...

const int32_t MAX_TRACER_NAME_LEN = 8;
const int32_t MAX_TYPE_NAME_LEN = 16;
const int32_t MAX_COMM_LEN = 16;
struct PagedMemoryFixedHeader {
    uint32_t pid = 0;
    uint32_t tid = 0;
    char tagName[MAX_TRACER_NAME_LEN] = {0};
    uint64_t startTime = 0;
    uint64_t endTime = 0;
    char typeName[MAX_TYPE_NAME_LEN] = {0};
    uint64_t addr = 0;
    uint32_t size = 0;
    uint16_t nips = 0;
    uint16_t type = 0;
    char comm[MAX_COMM_LEN] = {0};
} __attribute__((packed));

struct BIOFixedHeader {
    uint32_t pid = 0;
    uint32_t tid = 0;
    char processName[MAX_PROCESS_NAME_SZIE] = {0};
    uint64_t startTime = 0;
    uint64_t endTime = 0;
    uint32_t prio = 0;
    uint32_t size = 0;
    uint64_t blkcnt = 0;
    uint32_t nips = 0;
    uint32_t type = 0;
    char typeName[MAX_PROCESS_NAME_SZIE] = {0};
} __attribute__((packed));

struct StrEventFixedHeader {
    uint32_t pid = 0;
    uint32_t tid = 0;
    uint64_t startTime = 0;
    uint32_t srcTracer = 0;
    uint32_t srcType = 0;
    uint32_t strLen = 0;
    uint32_t padding = 0;
} __attribute__((packed));

struct KernelSymbolInfoHeader {
    uint64_t vaddrStart = 0;
    uint64_t vaddrEnd = 0;
    uint32_t symTabLen = 0;
    uint32_t strTabLen = 0;
    // char* symTab = 0; // this is following the struct
    // char* strTab = 0; // the symbols will follow the symTab
} __attribute__((packed));

struct KernelSymItem {
    uint64_t value = 0;
    uint32_t size = 0;
    uint32_t nameOffset = 0;
} __attribute__((packed));

// EBPF clock relation
enum EbpfClockType {
    EBPF_CLOCK_REALTIME = 0,
    EBPF_CLOCK_BOOTTIME,
    EBPF_CLOCK_MONOTONIC,
    EBPF_CLOCK_MONOTONIC_RAW,
};

const std::map<uint32_t, uint32_t> ebpfToTSClockType_ = {
    {EBPF_CLOCK_REALTIME, TS_CLOCK_REALTIME},
    {EBPF_CLOCK_BOOTTIME, TS_CLOCK_BOOTTIME},
    {EBPF_CLOCK_MONOTONIC, TS_MONOTONIC},
    {EBPF_CLOCK_MONOTONIC_RAW, TS_MONOTONIC_RAW},
};

struct SymbolAndFilePathIndex {
    SymbolAndFilePathIndex() {}
    explicit SymbolAndFilePathIndex(bool invalidValue) : flag(invalidValue) {}
    bool flag = false;
    DataIndex symbolIndex = INVALID_UINT64;
    DataIndex filePathIndex = INVALID_UINT64;
};
// file system event type
enum FUC_SUB_TYPE {
    SYS_OPENAT2 = 1,
    SYS_READ = 2,
    SYS_WRITE = 3,
    SYS_PREAD64 = 4,
    SYS_PWRITE64 = 5,
    SYS_READV = 6,
    SYS_WRITEV = 7,
    SYS_PREADV = 8,
    SYS_PWRITEV = 9,
    SYS_CLOSE = 10,
};

const std::map<FUC_SUB_TYPE, const std::string> subTypeToName = {
    {SYS_OPENAT2, "openat2"},   {SYS_READ, "read"},   {SYS_WRITE, "write"},   {SYS_PREAD64, "pread64"},
    {SYS_PWRITE64, "pwrite64"}, {SYS_READV, "readv"}, {SYS_WRITEV, "writev"}, {SYS_PREADV, "preadv"},
    {SYS_PWRITEV, "pwritev"},   {SYS_CLOSE, "close"}};

enum FUC_SUMMARY_TYPE {
    OPEN = 0,
    CLOSE = 1,
    READ = 2,
    WRITE = 3,
};
const std::map<uint32_t, uint32_t> fucSubToSummaryType = {
    {SYS_OPENAT2, OPEN}, {SYS_READ, READ},    {SYS_WRITE, WRITE}, {SYS_PREAD64, READ},  {SYS_PWRITE64, WRITE},
    {SYS_READV, READ},   {SYS_WRITEV, WRITE}, {SYS_PREADV, READ}, {SYS_PWRITEV, WRITE}, {SYS_CLOSE, CLOSE},
};
const uint32_t MIN_USER_IP = 0xffffffff;
} // namespace EbpfStdtype
} // namespace SysTuning
#endif // EBPF_DATA_STD_TYPE_H
