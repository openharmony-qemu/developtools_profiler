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
#include <elf_file.h>

#include <cinttypes>

namespace OHOS {
namespace Developtools {
namespace Hiebpf {
ElfFile::ElfFile(const std::string &filename)
{
    fd_ = open(filename.c_str(), O_RDONLY);
    if (fd_ != -1) {
        struct stat sb;
        if (fstat(fd_, &sb) == -1) {
            HHLOGE(true, "unable to check the file size");
        } else {
            HHLOGD(true, "file stat size %" PRIu64 " ", sb.st_size);
            mmap_ = mmap(0, sb.st_size, PROT_READ, MAP_PRIVATE, fd_, 0);
            if (mmap_ == MMAP_FAILED) {
                HHLOGE(true, "unable to map the file size %" PRIu64 " ", sb.st_size);
                mmapSize_ = 0;
            } else {
                mmapSize_ = sb.st_size;
                HHLOGD(true, "mmap build with size %" PRIu64 " ", mmapSize_);
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
    CHECK_NOTNULL(file, nullptr, "Error in ElfFile::MakeUnique(): ElfFile::ElfFile() failed");
    CHECK_TRUE(file->IsOpened(), nullptr, "Error in ElfFile::MakeUnique(): elf file not opended");
    CHECK_TRUE(file->ParseFile(), nullptr, "parse elf file failed");
    return file;
}

bool ElfFile::ParseFile()
{
    CHECK_TRUE(ParseElfHeader(), false, "Error in ElfFile::MakeUnique(): ElfFile::ParseElfHeader() failed");
    CHECK_TRUE(ParsePrgHeaders(), false, "Error in ElfFile::MakeUnique(): ElfFile::ParsePrgHeaders() failed");
    CHECK_TRUE(ParseSecNamesStr(), false, "Error in ElfFile::MakeUnique(): ElfFile::ParseSecNamesStr() failed");
    CHECK_TRUE(ParseSecHeaders(), false, "Error in ElfFile::MakeUnique(): ElfFile::ParseSecHeaders() failed");
    return true;
}

bool ElfFile::ParseElfHeader()
{
    ssize_t ret = lseek(fd_, 0, SEEK_SET);
    CHECK_TRUE(ret == 0, false, "lseek ret %zu", ret);

    unsigned char ehdrBuf[ehdr64Size] {0};
    size_t readsize = ReadFile(ehdrBuf, ehdr64Size);
    CHECK_TRUE(readsize >= ehdr64Size, false,
               "file size not enough, try read %zu, only have %zu", ehdr64Size, readsize);
    ehdr_ = ElfHeader::MakeUnique(ehdrBuf, readsize);
    return !(ehdr_ == nullptr);
}

bool ElfFile::ParsePrgHeaders()
{
    size_t phdrSize = ehdr_->phdrEntSize_;
    size_t numPhdrs = ehdr_->phdrNumEnts_;
    uint64_t phdrOffset = ehdr_->phdrOffset_;
    int64_t ret = lseek(fd_, phdrOffset, SEEK_SET);
    if (ret != static_cast<int64_t>(phdrOffset)) {
        return false;
    }
    char *phdrsBuf = new (std::nothrow) char[phdrSize * numPhdrs];
    CHECK_NOTNULL(phdrsBuf, false, "Error in ELF::ElfFile::ParsePrgHeaders(): new failed");

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
            HHLOGE(true, "Error in Elf::ParsePrgHeaders(): ProgramHeader::MakeUnique() failed");
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
    uint64_t shdrOffset = ehdr_->shdrOffset_ + ((uint64_t)shdrIndex) * shdrSize;
    int64_t ret = lseek(fd_, shdrOffset, SEEK_SET);
    if (ret != static_cast<int64_t>(shdrOffset)) {
        return false;
    }

    char *shdrBuf = new (std::nothrow) char[shdrSize];
    CHECK_NOTNULL(shdrBuf, false, "Error in ElfFile::ParseSecNamesStr(): new failed");

    ret = ReadFile(shdrBuf, shdrSize);
    if (ret != static_cast<int64_t>(shdrSize)) {
        delete[] shdrBuf;
        shdrBuf = nullptr;
        return false;
    }
    const std::string secName {".shstrtab"};
    shdrs_[secName] = SectionHeader::MakeUnique(shdrBuf, shdrSize, shdrIndex);
    if (shdrs_[secName] == nullptr) {
        HHLOGE(true, "Error in ElfFile::ParseSecNamesStr(): SectionHeader::MakeUnique() failed");
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
    if (ret != static_cast<int64_t>(secOffset)) {
        return false;
    }
    char *secNamesBuf = new (std::nothrow) char[secSize];
    CHECK_NOTNULL(secNamesBuf, false, "Error in ElfFile::ParseSecNamesStr(): new secNamesBuf failed");
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
    if (ret != static_cast<int64_t>(shdrOffset)) {
        return false;
    }
    char *shdrsBuf = new (std::nothrow) char[shdrSize * numShdrs];
    CHECK_NOTNULL(shdrsBuf, false, "Error in ELF::ElfFile::ParseSecHeaders(): new failed");

    ret = ReadFile(shdrsBuf, shdrSize * numShdrs);
    if (ret != static_cast<int64_t>(shdrSize * numShdrs)) {
        delete[] shdrsBuf;
        shdrsBuf = nullptr;
        return false;
    }

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

std::string ElfFile::GetSectionName(const uint32_t startIndex)
{
    if (startIndex >= secNamesStr_.size()) {
        HHLOGF(true, "out_of_range %s ,endIndex %d ", secNamesStr_.c_str(), startIndex);
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

// ElfHeader
std::unique_ptr<ElfHeader> ElfHeader::MakeUnique(unsigned char * const ehdrBuf,
                                                 const std::size_t bufSize)
{
    std::unique_ptr<ElfHeader> ehdr {new (std::nothrow) ElfHeader()};
    CHECK_NOTNULL(ehdr, nullptr, "ElfHeader() failed");
    CHECK_TRUE(ehdr->Init(ehdrBuf, bufSize), nullptr, "ElfHeader::Init(ehdrBuf, bufSize) failed\n");
    return ehdr;
}

bool ElfHeader::Init(unsigned char * const ehdrBuf, const std::size_t bufSize)
{
    std::string magicStr {ehdrBuf, ehdrBuf + SELFMAG};
    std::string elfMagic {ELFMAG};
    CHECK_TRUE(magicStr.compare(elfMagic) == 0, false, "elf magic not found");
    std::copy(ehdrBuf, ehdrBuf + EI_NIDENT, ehdrIdent_);

    if (ehdrBuf[EI_CLASS] == ELFCLASS32 and ParseElf32Header(ehdrBuf, bufSize)) {
        return true;
    }
    if (ehdrBuf[EI_CLASS] == ELFCLASS64 and ParseElf64Header(ehdrBuf, bufSize)) {
        return true;
    }
    HHLOGE(true, "init elf header failed, elf header buffer dumped");
    return false;
}

bool ElfHeader::ParseElf32Header(unsigned char * const ehdrBuf, const std::size_t bufSize)
{
    if (bufSize < ehdr32Size) {
        HHLOGE(true, "bad elf32 header buffer");
        return false;
    }
    size_t curIndex {EI_NIDENT};
    uint16_t *u2Buf = reinterpret_cast<uint16_t *>(ehdrBuf + curIndex);
    type_ = u2Buf[0];
    curIndex += sizeof(uint16_t);

    u2Buf = reinterpret_cast<uint16_t *>(ehdrBuf + curIndex);
    machine_ = u2Buf[0];
    curIndex += sizeof(uint16_t);

    uint32_t *u4Buf = reinterpret_cast<uint32_t *>(ehdrBuf + curIndex);
    elfVersion_ = u4Buf[0];
    curIndex += sizeof(uint32_t);

    u4Buf = reinterpret_cast<uint32_t *>(ehdrBuf + curIndex);
    prgEntryVaddr_ = u4Buf[0];
    curIndex += sizeof(uint32_t);

    u4Buf = reinterpret_cast<uint32_t *>(ehdrBuf + curIndex);
    phdrOffset_ = u4Buf[0];
    curIndex += sizeof(uint32_t);

    u4Buf = reinterpret_cast<uint32_t *>(ehdrBuf + curIndex);
    shdrOffset_ = u4Buf[0];
    curIndex += sizeof(uint32_t);

    u4Buf = reinterpret_cast<uint32_t *>(ehdrBuf + curIndex);
    ehdrFlags_ = u4Buf[0];
    curIndex += sizeof(uint32_t);

    u2Buf = reinterpret_cast<uint16_t *>(ehdrBuf + curIndex);
    ehdrSize_ = u2Buf[0];
    curIndex += sizeof(uint16_t);

    u2Buf = reinterpret_cast<uint16_t *>(ehdrBuf + curIndex);
    phdrEntSize_ = u2Buf[0];
    curIndex += sizeof(uint16_t);

    u2Buf = reinterpret_cast<uint16_t *>(ehdrBuf + curIndex);
    phdrNumEnts_ = u2Buf[0];
    curIndex += sizeof(uint16_t);

    u2Buf = reinterpret_cast<uint16_t *>(ehdrBuf + curIndex);
    shdrEntSize_ = u2Buf[0];
    curIndex += sizeof(uint16_t);

    u2Buf = reinterpret_cast<uint16_t *>(ehdrBuf + curIndex);
    shdrNumEnts_ = u2Buf[0];
    curIndex += sizeof(uint16_t);

    u2Buf = reinterpret_cast<uint16_t *>(ehdrBuf + curIndex);
    shdrStrTabIdx_ = u2Buf[0];

    return true;
}

bool ElfHeader::ParseElf64Header(unsigned char * const ehdrBuf, const std::size_t bufSize)
{
    CHECK_TRUE(bufSize >= ehdr64Size, false, "bad elf64 header buffer");
    size_t curIndex {EI_NIDENT};
    uint16_t *u2Buf = reinterpret_cast<uint16_t *>(ehdrBuf + curIndex);
    type_ = u2Buf[0];
    curIndex += sizeof(uint16_t);

    u2Buf = reinterpret_cast<uint16_t *>(ehdrBuf + curIndex);
    machine_ = u2Buf[0];
    curIndex += sizeof(uint16_t);

    uint32_t *u4Buf = reinterpret_cast<uint32_t *>(ehdrBuf + curIndex);
    elfVersion_ = u4Buf[0];
    curIndex += sizeof(uint32_t);

    uint64_t *u8Buf = reinterpret_cast<uint64_t *>(ehdrBuf + curIndex);
    prgEntryVaddr_ = u8Buf[0];
    curIndex += sizeof(uint64_t);

    u8Buf = reinterpret_cast<uint64_t *>(ehdrBuf + curIndex);
    phdrOffset_ = u8Buf[0];
    curIndex += sizeof(uint64_t);

    u8Buf = reinterpret_cast<uint64_t *>(ehdrBuf + curIndex);
    shdrOffset_ = u8Buf[0];
    curIndex += sizeof(uint64_t);

    u4Buf = reinterpret_cast<uint32_t *>(ehdrBuf + curIndex);
    ehdrFlags_ = u4Buf[0];
    curIndex += sizeof(uint32_t);

    u2Buf = reinterpret_cast<uint16_t *>(ehdrBuf + curIndex);
    ehdrSize_ = u2Buf[0];
    curIndex += sizeof(uint16_t);

    u2Buf = reinterpret_cast<uint16_t *>(ehdrBuf + curIndex);
    phdrEntSize_ = u2Buf[0];
    curIndex += sizeof(uint16_t);

    u2Buf = reinterpret_cast<uint16_t *>(ehdrBuf + curIndex);
    phdrNumEnts_ = u2Buf[0];
    curIndex += sizeof(uint16_t);

    u2Buf = reinterpret_cast<uint16_t *>(ehdrBuf + curIndex);
    shdrEntSize_ = u2Buf[0];
    curIndex += sizeof(uint16_t);

    u2Buf = reinterpret_cast<uint16_t *>(ehdrBuf + curIndex);
    shdrNumEnts_ = static_cast<long long>(*u2Buf);
    curIndex += sizeof(uint16_t);

    u2Buf = reinterpret_cast<uint16_t *>(ehdrBuf + curIndex);
    shdrStrTabIdx_ = u2Buf[0];

    return true;
}

// SectionHeader
enum class NUMBER : int {
    ZERO = 0,
    ONE = 1,
    TWO = 2,
    THREE = 3,
    FOUR = 4,
    FIVE = 5,
    SIX = 6,
    SEVEN = 7,
    EIGHT = 8,
    NINE = 9,
    TEN = 10,
    ELEVEN = 11,
    TWELVE = 12,
};

std::unique_ptr<SectionHeader> SectionHeader::MakeUnique(char * const shdrBuf, const size_t bufSize,
                                                         const size_t index)
{
    std::unique_ptr<SectionHeader> shdr {new (std::nothrow) SectionHeader()};
    if (shdr == nullptr) {
        return nullptr;
    }
    CHECK_TRUE(shdr->Init(shdrBuf, bufSize, index), nullptr, "SectionHeader::Init(shdrBuf, bufSize, index) failed");
    return shdr;
}

bool SectionHeader::ParseSecHeader32(char * const shdrBuf)
{
    uint32_t *u4Buf = reinterpret_cast<uint32_t *>(shdrBuf);
    int index {0};
    nameIndex_ = u4Buf[index];
    index = static_cast<int>(NUMBER::ONE);
    secType_ = u4Buf[index];
    index = static_cast<int>(NUMBER::TWO);
    secFlags_ = u4Buf[index];
    index = static_cast<int>(NUMBER::SIX);
    link_ = u4Buf[index];
    index = static_cast<int>(NUMBER::SEVEN);
    info_ = u4Buf[index];
    index = static_cast<int>(NUMBER::THREE);
    secVaddr_ = u4Buf[index];
    index = static_cast<int>(NUMBER::FOUR);
    fileOffset_ = u4Buf[index];
    index = static_cast<int>(NUMBER::FIVE);
    secSize_ = u4Buf[index];
    index = static_cast<int>(NUMBER::EIGHT);
    secAddrAlign_ = u4Buf[index];
    index = static_cast<int>(NUMBER::NINE);
    secEntrySize_ = u4Buf[index];
    return true;
}

bool SectionHeader::ParseSecHeader64(char * const shdrBuf)
{
    uint64_t *u8Buf = reinterpret_cast<uint64_t *>(shdrBuf);
    uint32_t *u4Buf = reinterpret_cast<uint32_t *>(shdrBuf);
    size_t index {0};
    nameIndex_ = u4Buf[index];
    index = static_cast<size_t>(NUMBER::ONE);
    secType_ = u4Buf[index];
    secFlags_ = u8Buf[index];
    index = static_cast<size_t>(NUMBER::TEN);
    link_ = u4Buf[index];
    index = static_cast<size_t>(NUMBER::ELEVEN);
    info_ = u4Buf[index];
    index = static_cast<size_t>(NUMBER::TWO);
    secVaddr_ = u8Buf[index];
    index = static_cast<size_t>(NUMBER::THREE);
    fileOffset_ = u8Buf[index];
    index = static_cast<size_t>(NUMBER::FOUR);
    secSize_ = u8Buf[index];
    index = static_cast<size_t>(NUMBER::SIX);
    secAddrAlign_ = u8Buf[index];
    index = static_cast<size_t>(NUMBER::SEVEN);
    secEntrySize_ = u8Buf[index];
    return true;
}

// ProgramHeader
std::unique_ptr<ProgramHeader> ProgramHeader::MakeUnique(char * const phdrBuf, const size_t bufSize)
{
    std::unique_ptr<ProgramHeader> phdr {new (std::nothrow) ProgramHeader()};
    CHECK_NOTNULL(phdr, nullptr, "ProgramHeader() failed");
    CHECK_TRUE(phdr->Init(phdrBuf, bufSize), nullptr, "ProgramHeader::Init(phdrBuf, bufSize) failed");
    return phdr;
}

bool ProgramHeader::ParsePrgHeader32(char * const phdrBuf)
{
    uint32_t *u4Buf = reinterpret_cast<uint32_t *>(phdrBuf);
    size_t index {0};
    type_ = u4Buf[index];
    ++index;
    offset_ = u4Buf[index];
    ++index;
    vaddr_ = u4Buf[index];
    ++index;
    paddr_ = u4Buf[index];
    ++index;
    fileSize_ = u4Buf[index];
    ++index;
    memSize_ = u4Buf[index];
    ++index;
    flags_ = u4Buf[index];
    ++index;
    secAlign_ = u4Buf[index];
    return true;
}

bool ProgramHeader::ParsePrgHeader64(char * const phdrBuf)
{
    uint32_t *u4Buf = reinterpret_cast<uint32_t *>(phdrBuf);
    size_t index {0};
    type_ = u4Buf[index];
    ++index;
    flags_ = u4Buf[index];

    uint64_t *u8Buf = reinterpret_cast<uint64_t *>(phdrBuf);
    offset_ = u8Buf[index];
    ++index;
    vaddr_ = u8Buf[index];
    ++index;
    paddr_ = u8Buf[index];
    ++index;
    fileSize_ = u8Buf[index];
    ++index;
    memSize_ = u8Buf[index];
    ++index;
    secAlign_ = u8Buf[index];
    return true;
}
} // namespace Hiebpf
} // namespace Developtools
} // namespace OHOS
