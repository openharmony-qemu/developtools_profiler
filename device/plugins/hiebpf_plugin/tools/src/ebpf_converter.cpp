/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "ebpf_converter.h"

#include <algorithm>
#include <cstring>
#include <cxxabi.h>
#include <fcntl.h>
#include <getopt.h>
#include <iostream>
#include <unistd.h>
#include <memory>
#include <securec.h>

#define CHK(expr) \
    do { \
        if (!(expr)) { \
            std::cout << __FUNCTION__ << ": "; \
            std::cout <<  __LINE__ << ", "; \
            std::cout << "read data failed" << std::endl; \
            return; \
        } \
    } while (0)

namespace OHOS {
namespace EBPF_TOOLS {
namespace {
constexpr uint8_t HEADER_MAGIC = 8;
constexpr uint8_t ARGS_NUM = 4;
constexpr uint8_t SYM_ENT_LEN_64 = 24;
constexpr uint8_t SYM_ENT_LEN_32 = 16;
constexpr uint8_t STT_FUNC = 2;
constexpr uint8_t EVENT_FS_FIXED_SIZE = 64;
constexpr uint8_t EVENT_MAPS_FIXED_SIZE = 36;
constexpr uint8_t EVENT_SYM_FIXED_SIZE = 36;
constexpr uint8_t EVENT_MEM_FIXED_SIZE = 88;
constexpr uint8_t EVENT_STR_FIXED_SIZE = 40;
constexpr uint8_t EVENT_BIO_FIXED_SIZE = 88;
} // namespace
EbpfConverter::EbpfConverter(const std::string& inputPath, const std::string& outPath)
    : inputPath_(inputPath), outputPath_(outPath) {}

void EbpfConverter::StartParsing()
{
    if (access(inputPath_.c_str(), R_OK) != 0) {
        std::cout << "the input file path is invalid" << std::endl;
        return;
    }
    fd_ = open(inputPath_.c_str(), O_RDONLY);
    if (fd_ == -1) {
        std::cout << "open " << inputPath_ << " failed" << std::endl;
        return;
    }
    char magic[HEADER_MAGIC + 1];
    (void)memset_s(magic, sizeof(magic), 0, sizeof(magic));
    CHK(Read(reinterpret_cast<void*>(&magic), sizeof(magic) - 1));
    outData_ << "magic: " << magic << '\n';
    HeaderDataItem header = {};
    CHK(Read(reinterpret_cast<void*>(&header), sizeof(header)));
    outData_  << "headSize: " << header.headSize << '\n' << "version: " << header.version << '\n'
        << "clock: " << header.clock << '\n' << "cmdLineLen: " << header.cmdLineLen;

    char cmdline[header.cmdLineLen + 1];
    if (header.cmdLineLen > 0) {
        CHK(Read(reinterpret_cast<void*>(cmdline), sizeof(cmdline) - 1));
        outData_ << "\ncmdline: " << cmdline << '\n';
    }
    fileSize_ = lseek(fd_, header.headSize, SEEK_SET);

    FILE *file = fopen(outputPath_.c_str(), "w");
    if (file == nullptr) {
        std::cout << "create " << outputPath_ << " failed" << std::endl;
        return;
    }

    while (true) {
        uint32_t type = 0;
        if (read(fd_, reinterpret_cast<void*>(&type), sizeof(type)) <= 0) {
            break;
        }
        switch (type) {
            case MAPSTRACE: {
                EventMapsParsing();
                break;
            }
            case SYMBOLTRACE: {
                SymbolInfoParsing();
                break;
            }
            case FSTRACE: {
                EventFsParsing();
                break;
            }
            case PFTRACE: {
                EventMemParsing();
                break;
            }
            case BIOTRACE: {
                EventBIOParsing();
                break;
            }
            case STRTRACE: {
                EventStrParsing();
                break;
            }
            default: {
                std::cout << "Invalid type: " << type << std::endl;
                close(fd_);
                fd_ = -1;
                fileSize_ = 0;
                fclose(file);
                return;
            }
        }
    }
    close(fd_);
    fd_ = -1;
    fileSize_ = 0;
    std::cout << "Data read successfully, output..." << std::endl;
    size_t len = outData_.str().size();
    if (fwrite(outData_.str().c_str(), 1, len, file) != len) {
        std::cout << "write data failed" << std::endl;
        fclose(file);
        return;
    }
    fclose(file);
    std::cout << "Data parsing success..." << std::endl;
}

bool EbpfConverter::Read(void* buffer, size_t size)
{
    ssize_t ret = read(fd_, buffer, size);
    if (ret <= 0) {
        close(fd_);
        fd_ = -1;
        fileSize_ = 0;
        return false;
    }
    return true;
}

void EbpfConverter::EventFsParsing()
{
    lseek(fd_, fileSize_, SEEK_SET);
    EventFs efs = {};
    CHK(Read(reinterpret_cast<void*>(&efs), EVENT_FS_FIXED_SIZE));
    uint64_t args[ARGS_NUM] = { 0 };
    CHK(Read(reinterpret_cast<void*>(args), sizeof(args)));
    efs.args = std::vector<uint64_t>(args, args + ARGS_NUM);
    CHK(Read(reinterpret_cast<void*>(&efs.comm), sizeof(efs.comm)));
    uint64_t userIPs[efs.nrUserIPs];
    if (efs.nrUserIPs > 0) {
        CHK(Read(reinterpret_cast<void*>(userIPs), sizeof(userIPs)));
        efs.userIPs = std::vector<uint64_t>(userIPs, userIPs + efs.nrUserIPs);
    }
    fileSize_ = lseek(fd_, 0, SEEK_CUR);

    outData_ << "\nEventFs:\n"
        << "  type: " << efs.tag << '\n'
        << "  len: " << efs.len << '\n'
        << "  pid: " << efs.pid << '\n'
        << "  tid: " << efs.tid << '\n'
        << "  tracerName: " << efs.tracerName << '\n'
        << "  start: " << efs.start << '\n'
        << "  end: " << efs.end << '\n'
        << "  typeName: " << efs.typeName << '\n'
        << "  ret: " << efs.ret << '\n'
        << "  nrUserIPs: " << efs.nrUserIPs << '\n'
        << "  type: " << efs.type << '\n'
        << "  args: ";
    for (const auto &arg : efs.args) {
        outData_ << arg << ", ";
    }

    outData_ << "\n  userIPs:\n";
    for (const auto &ip : efs.userIPs) {
        if (ip == 0) {
            continue;
        }
        outData_ << "    0x" << std::hex << ip << std::dec << ":";
        auto itm = GetSymbolInfo(efs.pid, ip);
        outData_ << "\n      vAddr_: 0x" << std::hex << vAddr_[ip] << std::dec << '\n';
        for (const auto &sym : itm.second) {
            outData_ << "      symbol: " << sym << '\n';
        }
        outData_ << "      fileName: " << itm.first << '\n';
    }
    outData_ << "  comm: " << std::string(efs.comm) << '\n';
}

void EbpfConverter::EventMapsParsing()
{
    lseek(fd_, fileSize_, SEEK_SET);
    EventMaps maps = {};
    CHK(Read(reinterpret_cast<void*>(&maps), EVENT_MAPS_FIXED_SIZE));
    char fileName[maps.fileNameLen + 1];
    (void)memset_s(fileName, sizeof(fileName), 0, sizeof(fileName));
    if (maps.fileNameLen > 0) {
        CHK(Read(reinterpret_cast<void*>(fileName), sizeof(fileName) - 1));
        maps.fileName = fileName;
    }
    auto iter = maps_.find(maps.pid);
    if (iter == maps_.end()) {
        maps_[maps.pid] = std::vector<EventMaps>({ maps });
    } else {
        iter->second.push_back(maps);
    }
    fileSize_ = lseek(fd_, 0, SEEK_CUR);

    // put output
    outData_ << "\nEventMaps:\n"
        << "  type: " << maps.type << '\n'
        << "  len: " << maps.len << '\n'
        << "  start: 0x" << std::hex << maps.start << std::dec << '\n'
        << "  end: 0x" << std::hex << maps.end << std::dec << '\n'
        << "  offset: 0x" << std::hex << maps.offset << std::dec << '\n'
        << "  pid: " << maps.pid << '\n'
        << "  fileNameLen: " << maps.fileNameLen << '\n'
        << "  fileName: " << maps.fileName << '\n';
}

void EbpfConverter::SymbolInfoParsing()
{
    lseek(fd_, fileSize_, SEEK_SET);
    uint64_t strAddr = 0;
    uint64_t symAddr = 0;
    SymbolInfo info = {};
    CHK(Read(reinterpret_cast<void*>(&info), EVENT_SYM_FIXED_SIZE));

    info.strTab = new char[info.strTabLen + 1];
    if (memset_s(info.strTab, info.strTabLen + 1, 0, info.strTabLen + 1) != EOK) {
        std::cout << "memset string table failed" << std::endl;
        return;
    }
    if (info.strTabLen > 0) {
        auto ret = lseek(fd_, 0, SEEK_CUR);
        if (ret >= 0) {
            strAddr = (uint64_t)ret;
        }
        CHK(Read(reinterpret_cast<void*>(info.strTab), info.strTabLen));
    }
    info.symTab = new char[info.symTabLen + 1];
    if (memset_s(info.symTab, info.symTabLen + 1, 0, info.symTabLen + 1) != EOK) {
        std::cout << "memset symbol table failed" << std::endl;
        return;
    }
    if (info.symTabLen > 0) {
        auto ret = lseek(fd_, 0, SEEK_CUR);
        if (ret >= 0) {
            symAddr = (uint64_t)ret;
        }
        CHK(Read(reinterpret_cast<void*>(info.symTab), info.symTabLen));
    }
    char fileName[info.fileNameSize + 1];
    (void)memset_s(fileName, sizeof(fileName), 0, sizeof(fileName));
    if (info.fileNameSize > 0) {
        CHK(Read(reinterpret_cast<void*>(&fileName), sizeof(fileName) - 1));
        info.fileName = fileName;
    }
    record_[std::string(fileName)] = Record(fileName, symAddr, strAddr);
    symbolInfo_[fileName] = info;
    fileSize_ = lseek(fd_, 0, SEEK_CUR);

    // put output
    outData_ << "\nSymbolInfo:\n"
            << "  tag: " << info.type << '\n'
            << "  len: " << info.len << '\n'
            << "  textVaddr: 0x" << std::hex << info.textVaddr << std::dec << '\n'
            << "  textOffset: 0x" << std::hex << info.textOffset << std::dec << '\n'
            << "  strTabLen: 0x" << std::hex << info.strTabLen << std::dec <<'\n'
            << "  symTabLen: 0x" << std::hex << info.symTabLen << std::dec << '\n'
            << "  fileNameSize: " << info.fileNameSize << '\n'
            << "  fileName: " << info.fileName << '\n'
            << "  strTabAddr: 0x" << std::hex << record_[info.fileName].strTabAddr << std::dec << '\n'
            << "  symTabAddr: 0x" << std::hex << record_[info.fileName].symTabAddr << std::dec << '\n';
}

void EbpfConverter::EventMemParsing()
{
    lseek(fd_, fileSize_, SEEK_SET);
    EventMem mem = {};
    CHK(Read(reinterpret_cast<void*>(&mem), EVENT_MEM_FIXED_SIZE));
    uint64_t userIPs[mem.nips];
    if (mem.nips > 0) {
        CHK(Read(reinterpret_cast<void*>(&userIPs), sizeof(userIPs)));
        mem.userIPs = std::vector<uint64_t>(userIPs, userIPs + mem.nips);
    }
    fileSize_ = lseek(fd_, 0, SEEK_CUR);

    outData_ << "\nEventMem:\n"
        << "  tag: " << mem.tag << '\n'
        << "  len: " << mem.len << '\n'
        << "  pid: " << mem.pid << '\n'
        << "  tid: " << mem.tid << '\n'
        << "  tagName: " << mem.tagName << '\n'
        << "  start: " << mem.start << '\n'
        << "  end: " << mem.end << '\n'
        << "  typeName: " << mem.typeName << '\n'
        << "  addr: " << mem.addr << '\n'
        << "  size: " << mem.size << '\n'
        << "  nips: " << mem.nips << '\n'
        << "  type: " << mem.type << '\n'
        << "  comm: " << mem.comm << '\n'
        << "  userIPs:\n";
    for (const auto &ip : mem.userIPs) {
        if (ip == 0) {
            continue;
        }
        outData_ << "    0x" << std::hex << ip << std::dec << ":";
        auto itm = GetSymbolInfo(mem.pid, ip);
        outData_ << "\n      vAddr_: 0x" << std::hex << vAddr_[ip] << std::dec << '\n';
        for (const auto &sym : itm.second) {
            outData_ << "      symbol: " << sym << '\n';
        }
        outData_ << "      fileName: " << itm.first << '\n';
    }
}

void EbpfConverter::EventStrParsing()
{
    lseek(fd_, fileSize_, SEEK_SET);
    EventStr str = {};
    CHK(Read(reinterpret_cast<void*>(&str), EVENT_STR_FIXED_SIZE));
    char fileName[str.strLen + 1];
    if (str.strLen > 0) {
        CHK(Read(reinterpret_cast<void*>(&fileName), sizeof(fileName) - 1));
        str.fileName = fileName;
    }
    str_.push_back(str);
    int32_t newPos = static_cast<int32_t>(str.len - 32 - str.strLen);
    fileSize_ = lseek(fd_, lseek(fd_, 0, SEEK_CUR) + newPos, SEEK_SET);

    outData_ << "\nEventStr:\n"
        << "  tag: " << str.tag << '\n'
        << "  len: " << str.len << '\n'
        << "  pid: " << str.pid << '\n'
        << "  tid: " << str.tid << '\n'
        << "  start: " << str.start << '\n'
        << "  srcTracer: " << str.srcTracer << '\n'
        << "  srcType: " << str.srcType << '\n'
        << "  strLen: " << str.strLen << '\n'
        << "  fileName: " << str.fileName << '\n';
}

void EbpfConverter::EventBIOParsing()
{
    lseek(fd_, fileSize_, SEEK_SET);
    EventBIO bio = {};
    CHK(Read(reinterpret_cast<void*>(&bio), EVENT_BIO_FIXED_SIZE));
    uint64_t userIPs[bio.nips];
    if (bio.nips > 0) {
        CHK(Read(reinterpret_cast<void*>(userIPs), sizeof(userIPs)));
        bio.userIPs = std::vector<uint64_t>(userIPs, userIPs + bio.nips);
    }
    fileSize_ = lseek(fd_, 0, SEEK_CUR);

    outData_ << "\nEventBIO:\n"
        << "  tag: " << bio.tag << '\n'
        << "  len: " << bio.len << '\n'
        << "  pid: " << bio.pid << '\n'
        << "  tid: " << bio.tid << '\n'
        << "  comm: " << bio.comm << '\n'
        << "  start: " << bio.start << '\n'
        << "  end: " << bio.end << '\n'
        << "  prio: " << bio.prio << '\n'
        << "  size: " << bio.size << '\n'
        << "  type: " << bio.type << '\n'
        << "  typeName: " << bio.typeName << '\n';

    for (const auto &ip : bio.userIPs) {
        if (ip == 0) {
            continue;
        }
        outData_ << "    0x" << std::hex << ip << std::dec << ":";
        auto itm = GetSymbolInfo(bio.pid, ip);
        outData_ << "\n      vAddr_: 0x" << std::hex << vAddr_[ip] << std::dec << '\n';
        for (const auto &sym : itm.second) {
            outData_ << "      symbol: " << sym << '\n';
        }
        outData_ << "      fileName: " << itm.first << '\n';
    }
}

std::pair<std::string, std::vector<std::string>> EbpfConverter::GetSymbolInfo(uint64_t pid, uint64_t ip)
{
    auto mapIter = maps_.find(pid);
    if (mapIter == maps_.end()) {
        return std::pair<std::string, std::vector<std::string>>();
    }

    std::string fileName;
    uint64_t start = 0;
    uint32_t offset = 0;
    std::any_of(mapIter->second.begin(), mapIter->second.end(), [&](const EventMaps &maps) {
        if (ip >= maps.start && ip <= maps.end) {
            fileName = maps.fileName;
            start = maps.start;
            offset = maps.offset;
            return true;
        }
        return false;
    });
    if (fileName.empty()) {
        return std::pair<std::string, std::vector<std::string>>();
    }

    auto symItem = symbolInfo_.find(fileName);
    if (symItem == symbolInfo_.end()) {
        return std::pair<std::string, std::vector<std::string>>();
    }
    uint64_t vaddr = ip - start + offset - symItem->second.textOffset + symItem->second.textVaddr;
    vAddr_[ip] = vaddr;
    std::pair<std::string, std::vector<std::string>> symbolInfos;
    symbolInfos.first = fileName;
    if (symItem->second.symEntlen == SYM_ENT_LEN_64) {
        uint32_t count = 0;
        while (count < symItem->second.symTabLen) {
            Elf64_Sym sym;
            if (memcpy_s(&sym, sizeof(sym), symItem->second.symTab + count, SYM_ENT_LEN_64) != EOK) {
                std::cout << "copy symTab failed" << std::endl;
                return std::pair<std::string, std::vector<std::string>>();
            }
            if (vaddr >= sym.st_value &&
                vaddr <= sym.st_value + sym.st_size &&
                (sym.st_info & STT_FUNC) &&
                sym.st_value != 0) {
                char *ret = abi::__cxa_demangle(static_cast<char *>(symItem->second.strTab + sym.st_name),
                                                nullptr, nullptr, nullptr);
                ret == nullptr ? symbolInfos.second.push_back(std::string(symItem->second.strTab + sym.st_name))
                    : symbolInfos.second.push_back(std::string(ret));
            }
            count += SYM_ENT_LEN_64;
        }
    } else if (symItem->second.symEntlen == SYM_ENT_LEN_32) {
        uint32_t count = 0;
        while (count < symItem->second.symTabLen) {
            Elf32_Sym sym;
            if (memcpy_s(&sym, sizeof(sym), symItem->second.strTab + count, SYM_ENT_LEN_32) != EOK) {
                std::cout << "copy symTab failed" << std::endl;
                return std::pair<std::string, std::vector<std::string>>();
            }
            if (vaddr >= sym.st_value &&
                vaddr <= sym.st_value + sym.st_size &&
                (sym.st_info & STT_FUNC) &&
                sym.st_value != 0) {
                char *ret = abi::__cxa_demangle(symItem->second.strTab + sym.st_name, nullptr, nullptr, nullptr);
                ret == nullptr ? symbolInfos.second.push_back(std::string(symItem->second.strTab + sym.st_name))
                    : symbolInfos.second.push_back(std::string(ret));
            }
            count += SYM_ENT_LEN_32;
        }
    } else {
        std::cout << "Invalid symEntlen" << std::endl;
    }
    return symbolInfos;
}
} // EBPF_TOOLS
} // OHOS

int32_t main(int32_t argc, char* argv[])
{
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);
    struct option long_options[] = {
        {"input", required_argument, nullptr, 'i'},
        {"out", required_argument, nullptr, 'o'},
        {"help", no_argument, nullptr, 'h'},
        {0, 0, 0, 0}
    };
    int32_t opt;
    std::string inputPath, outputPath;
    while ((opt = getopt_long(argc, argv, "i:o:h", long_options, nullptr)) != -1) {
        switch (opt) {
            case 'i': {
                inputPath = optarg;
                continue;
            }
            case 'o': {
                outputPath = optarg;
                continue;
            }
            case 'h': {
                std::cout << "    --input" << "  " << "-i" << "  :" << "input file name" << std::endl;
                std::cout << "    --out" << "    " << "-o" << "  :" << "out file name" << std::endl;
                break;
            }
            default: {
                std::cout << "Parameter error" << std::endl;
                break;
            }
        }
    }
    OHOS::EBPF_TOOLS::EbpfConverter ec(inputPath, outputPath);
    ec.StartParsing();
    return 0;
}