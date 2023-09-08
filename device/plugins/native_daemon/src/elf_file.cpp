/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021. All rights reserved.
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

#include <elf_parser.h>

using namespace OHOS::Developtools::NativeDaemon::ELF;
namespace OHOS {
namespace Developtools {
namespace NativeDaemon {
ElfFile::ElfFile(const std::string &filename)
{
#if is_mingw
    fd_ = open(filename.c_str(), O_RDONLY | O_BINARY);
#else
    fd_ = open(filename.c_str(), O_RDONLY);
#endif
    if (fd_ != -1) {
        struct stat sb;
        if (fstat(fd_, &sb) == -1) {
            HLOGE("unable to check the file size");
        } else {
            HLOGD("file stat size %" PRIu64 "", sb.st_size);

            mmap_ = mmap(0, sb.st_size, PROT_READ, MAP_PRIVATE, fd_, 0);
            if (mmap_ == MMAP_FAILED) {
                HLOGE("unable to map the file size %" PRIu64 " ", sb.st_size);
                mmapSize_ = 0;
            } else {
                mmapSize_ = sb.st_size;
                HLOGD("mmap build with size %" PRIu64 " ", mmapSize_);
            }
        }
    }
}

ElfFile::~ElfFile()
{
    if (mmap_ != MMAP_FAILED) {
        munmap(mmap_, mmapSize_);
    }

    if (fd_ != -1) {
        close(fd_);
        fd_ = -1;
    }
}

std::unique_ptr<ElfFile> ElfFile::MakeUnique(const std::string &filename)
{
    std::unique_ptr<ElfFile> file {new (std::nothrow) ElfFile(filename)};
    UNWIND_CHECK_NOTNULL(file, nullptr, "Error in ElfFile::MakeUnique(): ElfFile::ElfFile() failed");
    UNWIND_CHECK_TRUE(file->IsOpened(), nullptr, "Error in ElfFile::MakeUnique(): elf file not opended: %s", filename.c_str());
    UNWIND_CHECK_TRUE(file->ParseFile(), nullptr, "parse elf file failed: %s", filename.c_str());
    return file;
}

bool ElfFile::ParseFile()
{
    UNWIND_CHECK_TRUE(ParseElfHeader(), false, "Error in ElfFile::MakeUnique(): ElfFile::ParseElfHeader() failed");
    UNWIND_CHECK_TRUE(ParsePrgHeaders(), false, "Error in ElfFile::MakeUnique(): ElfFile::ParsePrgHeaders() failed");
    UNWIND_CHECK_TRUE(ParseSecNamesStr(), false, "Error in ElfFile::MakeUnique(): ElfFile::ParseSecNamesStr() failed");
    UNWIND_CHECK_TRUE(ParseSecHeaders(), false, "Error in ElfFile::MakeUnique(): ElfFile::ParseSecHeaders() failed");
    return true;
}

bool ElfFile::ParseElfHeader()
{
    size_t ret = lseek(fd_, 0, SEEK_SET);
    UNWIND_CHECK_TRUE(ret == 0, false, "lseek ret %zu", ret);
    HLOG_ASSERT(ret == 0);
    unsigned char ehdrBuf[ehdr64Size] {0};
    ret = ReadFile(ehdrBuf, ehdr64Size);
    if (ret < ehdr64Size) {
        HLOGW("file size not enough, try read %zu, only have %zu", ehdr64Size, ret);
    }
    HLOG_ASSERT(ret > 0);
    ehdr_ = ElfHeader::MakeUnique(ehdrBuf, ret);
    return !(ehdr_ == nullptr);
}

bool ElfFile::ParsePrgHeaders()
{
    size_t phdrSize = ehdr_->phdrEntSize_;
    size_t numPhdrs = ehdr_->phdrNumEnts_;
    uint64_t phdrOffset = ehdr_->phdrOffset_;
    int64_t ret = lseek(fd_, phdrOffset, SEEK_SET);
    HLOG_ASSERT(ret == static_cast<int64_t>(phdrOffset));
    char *phdrsBuf = new (std::nothrow) char[phdrSize * numPhdrs];
    UNWIND_CHECK_NOTNULL(phdrsBuf, false, "Error in ELF::ElfFile::ParsePrgHeaders(): new failed");
    if (memset_s(phdrsBuf, phdrSize * numPhdrs, 0, phdrSize * numPhdrs) != EOK) {
        HLOGE("memset_s failed");
    }
    ret = ReadFile(phdrsBuf, phdrSize * numPhdrs);
    if (ret != static_cast<int64_t>(phdrSize * numPhdrs)) {
        delete[] phdrsBuf;
        phdrsBuf = nullptr;
        return false;
    }
    char *phdrBuf = phdrsBuf;
    for (size_t count = 0; count < numPhdrs; ++count) {
        std::unique_ptr<ProgramHeader> phdr = ProgramHeader::MakeUnique(phdrBuf, phdrSize);
        if (phdr == nullptr) {
            delete[] phdrsBuf;
            phdrsBuf = nullptr;
            HLOGE("Error in Elf::ParsePrgHeaders(): ProgramHeader::MakeUnique() failed");
            return false;
        }
        phdrs_.push_back(std::move(phdr));
        phdrBuf += phdrSize;
    }
    delete[] phdrsBuf;
    phdrsBuf = nullptr;
    return true;
}

bool ElfFile::ParseSecNamesStr()
{
    // get string table section header
    size_t shdrSize = ehdr_->shdrEntSize_;
    size_t shdrIndex = ehdr_->shdrStrTabIdx_;
    uint64_t shdrOffset = ehdr_->shdrOffset_ + shdrIndex * shdrSize;
    int64_t ret = lseek(fd_, shdrOffset, SEEK_SET);
    HLOG_ASSERT(ret == static_cast<int64_t>(shdrOffset));
    char *shdrBuf = new (std::nothrow) char[shdrSize];
    UNWIND_CHECK_NOTNULL(shdrBuf, false, "Error in ElfFile::ParseSecNamesStr(): new failed");
    if (memset_s(shdrBuf, shdrSize, 0, shdrSize) != EOK) {
        HLOGE("memset_s failed");
    }
    ret = ReadFile(shdrBuf, shdrSize);
    HLOG_ASSERT(ret == static_cast<int64_t>(shdrSize));
    const std::string secName {".shstrtab"};
    shdrs_[secName] = SectionHeader::MakeUnique(shdrBuf, shdrSize, shdrIndex);
    if (shdrs_[secName] == nullptr) {
        HLOGE("Error in ElfFile::ParseSecNamesStr(): SectionHeader::MakeUnique() failed");
        delete[] shdrBuf;
        shdrBuf = nullptr;
        return false;
    }
    delete[] shdrBuf;
    shdrBuf = nullptr;

    // get content of string section table
    uint64_t secOffset = shdrs_[secName]->fileOffset_;
    size_t secSize = shdrs_[secName]->secSize_;
    ret = lseek(fd_, secOffset, SEEK_SET);
    HLOG_ASSERT(ret == static_cast<int64_t>(secOffset));
    char *secNamesBuf = new (std::nothrow) char[secSize];
    if (secNamesBuf == nullptr) {
        return false;
    }
    if (memset_s(secNamesBuf, secSize, '\0', secSize) != EOK) {
        delete[] secNamesBuf;
        secNamesBuf = nullptr;
        HLOGE("memset_s failed");
        return false;
    }
    ret = ReadFile(secNamesBuf, secSize);
    if (ret != static_cast<int64_t>(secSize)) {
        delete[] secNamesBuf;
        secNamesBuf = nullptr;
        return false;
    }
    secNamesStr_ = std::string(secNamesBuf, secNamesBuf + secSize);
    delete[] secNamesBuf;
    secNamesBuf = nullptr;
    return true;
}

bool ElfFile::ParseSecHeaders()
{
    size_t shdrSize = ehdr_->shdrEntSize_;
    size_t numShdrs = ehdr_->shdrNumEnts_;
    uint64_t shdrOffset = ehdr_->shdrOffset_;
    int64_t ret = lseek(fd_, shdrOffset, SEEK_SET);
    HLOG_ASSERT(ret == static_cast<int64_t>(shdrOffset));
    char *shdrsBuf = new (std::nothrow) char[shdrSize * numShdrs];
    UNWIND_CHECK_NOTNULL(shdrsBuf, false, "Error in ELF::ElfFile::ParseSecHeaders(): new failed");
    if (memset_s(shdrsBuf, shdrSize * numShdrs, '\0', shdrSize * numShdrs) != EOK) {
        HLOGE("memset_s failed");
    }
    ret = ReadFile(shdrsBuf, shdrSize * numShdrs);
    HLOG_ASSERT(ret == static_cast<int64_t>(shdrSize * numShdrs));
    char *shdrBuf = shdrsBuf;
    for (size_t count = 0; count < numShdrs; ++count) {
        if (count == ehdr_->shdrStrTabIdx_) {
            shdrBuf += shdrSize;
            continue;
        }
        std::unique_ptr<SectionHeader> shdr = SectionHeader::MakeUnique(shdrBuf, shdrSize, count);
        if (shdr == nullptr) {
            delete[] shdrsBuf;
            shdrsBuf = nullptr;
            return false;
        }
        std::string secName = GetSectionName(shdr->nameIndex_);
        shdrs_[secName] = std::move(shdr);
        shdr.reset(nullptr);
        shdrBuf += shdrSize;
    }
    delete[] shdrsBuf;
    shdrsBuf = nullptr;
    return true;
}

bool ElfFile::ParseSymTable(const std::string secName)
{
    if (shdrs_.find(secName) == shdrs_.end()) {
        HLOGE("Error in ELF::ElfFile::ParseSymTable(): section %s does not exist", secName.c_str());
        return false;
    } else {
        return ParseSymTable(shdrs_[secName].get());
    }
}

bool ElfFile::ParseSymTable(const SectionHeader *shdr)
{
    if (shdr == nullptr) {
        return false;
    }
    uint64_t secOffset = shdr->fileOffset_;
    int64_t ret = lseek(fd_, secOffset, SEEK_SET);
    HLOG_ASSERT(ret == static_cast<int64_t>(secOffset));
    uint64_t secSize = shdr->secSize_;
    uint64_t entrySize = shdr->secEntrySize_;
    char *secBuf = new (std::nothrow) char[secSize];
    UNWIND_CHECK_NOTNULL(secBuf, false, "Error in EFL::ElfFile::ParseSymTable(): new failed");
    ret = ReadFile(secBuf, secSize);
    HLOG_ASSERT(ret == static_cast<int64_t>(secSize));
    symTable_ = SymbolTable::MakeUnique(symNamesStr_, secBuf, secSize, entrySize);
    if (symTable_ == nullptr) {
        delete[] secBuf;
        secBuf = nullptr;
        return false;
    }
    delete[] secBuf;
    secBuf = nullptr;
    return true;
}

std::string ElfFile::GetSectionName(const uint32_t startIndex)
{
    if (startIndex >= secNamesStr_.size()) {
        HLOGF("out_of_range %s ,endIndex %d ", secNamesStr_.c_str(), startIndex);
        return "";
    }
    size_t endIndex {startIndex};
    for (; endIndex < secNamesStr_.size(); ++endIndex) {
        if (secNamesStr_[endIndex] == '\0') {
            break;
        }
    }
    return secNamesStr_.substr(startIndex, endIndex - startIndex);
}
} // namespace NativeDaemon
} // namespace Developtools
} // namespace OHOS