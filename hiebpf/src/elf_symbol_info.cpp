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
#include "elf_symbol_info.h"

#include <limits>

#include "elf_file.h"
#include "hhlog.h"

namespace OHOS {
namespace Developtools {
namespace Hiebpf {
bool ElfSymbolInfo::GetSymbolTable(const std::string &fileName, ElfSymbolTable &symbolTable)
{
    std::unique_ptr<ElfFile> elfFile = ElfFile::MakeUnique(fileName);
    CHECK_NOTNULL(elfFile, false, "elf '%s' load failed\n", fileName.c_str());

    // get .text
    symbolTable.textVaddr_ = (std::numeric_limits<uint64_t>::max)();
    for (auto &phdr : elfFile->phdrs_) {
        if ((phdr->type_ == PT_LOAD) && (phdr->flags_ & PF_X)) {
            // find the min addr
            if (symbolTable.textVaddr_ != (std::min)(symbolTable.textVaddr_, phdr->vaddr_)) {
                symbolTable.textVaddr_ = (std::min)(symbolTable.textVaddr_, phdr->vaddr_);
                symbolTable.textOffset_ = phdr->offset_;
            }
        }
    }
    CHECK_TRUE(symbolTable.textVaddr_ != (std::numeric_limits<uint64_t>::max)(), false, "get text vaddr failed");

    const std::string symTab {".symtab"};
    if (elfFile->shdrs_.find(symTab) != elfFile->shdrs_.end()) {
        // get .symtab
        const auto &shdr = elfFile->shdrs_[symTab];
        const uint8_t *data = elfFile->GetSectionData(shdr->secIndex_);
        CHECK_NOTNULL(data, false, "get section data failed");
        symbolTable.symTable_.resize(shdr->secSize_);
        std::copy(data, data + shdr->secSize_, symbolTable.symTable_.data());
        symbolTable.symEntSize_ = shdr->secEntrySize_;

        // get .strtab
        const std::string strTab {".strtab"};
        CHECK_TRUE(elfFile->shdrs_.find(strTab) != elfFile->shdrs_.end(), false, "get symbol tab failed");
        const auto &strshdr = elfFile->shdrs_[strTab];
        data = elfFile->GetSectionData(strshdr->secIndex_);
        CHECK_NOTNULL(data, false, "get section data failed");
        symbolTable.strTable_.resize(strshdr->secSize_);
        std::copy(data, data + strshdr->secSize_, symbolTable.strTable_.data());
    } else {
        // get .dynsym
        const std::string dynSym {".dynsym"};
        CHECK_TRUE(elfFile->shdrs_.find(dynSym) != elfFile->shdrs_.end(), false, "get symbol tab failed");
        const auto &shdr = elfFile->shdrs_[dynSym];
        const uint8_t *data = elfFile->GetSectionData(shdr->secIndex_);
        CHECK_NOTNULL(data, false, "get section data failed");
        symbolTable.symTable_.resize(shdr->secSize_);
        std::copy(data, data + shdr->secSize_, symbolTable.symTable_.data());
        symbolTable.symEntSize_ = shdr->secEntrySize_;

        // get .dynstr
        const std::string dynStr {".dynstr"};
        CHECK_TRUE(elfFile->shdrs_.find(dynStr) != elfFile->shdrs_.end(), false, "get symbol tab failed");
        const auto &strshdr = elfFile->shdrs_[dynStr];
        data = elfFile->GetSectionData(strshdr->secIndex_);
        CHECK_NOTNULL(data, false, "get section data failed");
        symbolTable.strTable_.resize(strshdr->secSize_);
        std::copy(data, data + strshdr->secSize_, symbolTable.strTable_.data());
    }
    CHECK_TRUE(symbolTable.strTable_.size() != 0 && symbolTable.symTable_.size() != 0, false,
               "get strTable_ or symTable failed");
    symbolTable.fileName_ = fileName;

    return true;
}

uint32_t ElfSymbolInfo::GetBinary(const ElfSymbolTable &symbolTable, std::vector<uint8_t> &buf)
{
    // strTabLen+symTabLen+fileNameLen
    uint32_t fixLen = sizeof(symbolTable.textVaddr_) + sizeof(symbolTable.textOffset_) +
                      sizeof(uint32_t) + sizeof(uint32_t) + sizeof(uint32_t) + sizeof(uint32_t);
    uint32_t len = fixLen + symbolTable.strTable_.size() + symbolTable.symTable_.size() +
                   symbolTable.fileName_.size() + 1;
    buf.resize(len);

    const uint8_t *rp = reinterpret_cast<const uint8_t *>(&symbolTable);
    uint8_t *wp = buf.data();
    std::copy(rp, rp + sizeof(symbolTable.textVaddr_) + sizeof(symbolTable.textOffset_), wp);
    wp += sizeof(symbolTable.textVaddr_) + sizeof(symbolTable.textOffset_);
    *(reinterpret_cast<uint32_t *>(wp)) = symbolTable.strTable_.size(); // strTabLen
    wp += sizeof(uint32_t);
    *(reinterpret_cast<uint32_t *>(wp)) = symbolTable.symTable_.size(); // symTabLen
    wp += sizeof(uint32_t);
    *(reinterpret_cast<uint32_t *>(wp)) = symbolTable.fileName_.size() + 1; // fileNameLen
    wp += sizeof(uint32_t);
    *(reinterpret_cast<uint32_t *>(wp)) = symbolTable.symEntSize_; // symEntLen
    wp += sizeof(uint32_t);
    std::copy(symbolTable.strTable_.data(),
              symbolTable.strTable_.data() + symbolTable.strTable_.size(), wp);
    wp += symbolTable.strTable_.size();
    std::copy(symbolTable.symTable_.data(),
              symbolTable.symTable_.data() + symbolTable.symTable_.size(), wp);
    wp += symbolTable.symTable_.size();
    std::copy(symbolTable.fileName_.c_str(),
              symbolTable.fileName_.c_str() + symbolTable.fileName_.size() + 1, wp); // fileName

    return buf.size();
}
} // namespace Hiebpf
} // namespace Developtools
} // namespace OHOS