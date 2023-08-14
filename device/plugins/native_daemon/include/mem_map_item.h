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

#ifndef MEMMAPITEM_H
#define MEMMAPITEM_H

#include <sstream>
#include <string>
#include <utilities.h>

class MemMaps {
public:
    MemMaps() {}
    MemMaps(uint64_t begin, uint64_t end, uint64_t offset, uint32_t type, uint32_t filePathId, const std::string& name)
        : soBegin_(begin), soEnd_(end), filePathId_(filePathId), name_(name)
    {
        maps_.emplace_back(begin, end, static_cast<uint16_t>(type), offset, name_);
    }
    uint64_t soBegin_;
    uint64_t soEnd_;
    uint32_t filePathId_ {0}; // for maps item filePath id
    std::string name_;
    bool isReported_ {false}; // indicates whether information about item has been reported

    class MemMapItem {
    public:
        MemMapItem() {}
        MemMapItem(uint64_t begin, uint64_t end, uint16_t type, uint64_t offset, std::string_view nameHold)
            : begin_(begin), end_(end), type_(type), pageoffset_(offset), nameHold_(nameHold) {}
        uint64_t begin_ {0};
        uint64_t end_ {0};
        uint16_t type_ {0}; // rwx : PROT_READ | PROT_WRITE | PROT_EXEC
        uint16_t flags_ {0}; // ps : MAP_PRIVATE | MAP_SHARED
        uint64_t pageoffset_ {0};
        std::string_view nameHold_;

        uint64_t FileOffsetFromAddr(uint64_t addr) const
        {
            // real vaddr - real map begin = addr offset in section
            // section offset + page off set = file offset
            return addr - begin_ + pageoffset_;
        }
    };

    std::vector<MemMapItem> maps_;
};

class MemMapItem {
public:
    uint64_t begin_ = 0;
    uint64_t end_ = 0;
    uint16_t type_ = 0; // rwx : PROT_READ | PROT_WRITE | PROT_EXEC
    uint16_t flags = 0; // ps  : MAP_PRIVATE | MAP_SHARED
    uint64_t pageoffset_ = 0;
    uint64_t major_ = 0;
    uint64_t minor_ = 0;
    uint64_t inode = 0;
    uint32_t filePathId_ = 0; // for maps item filePath id
    std::string name_;
    std::string_view nameHold_;
    bool isReported = false; // indicates whether information about memMapItem has been reported

    MemMapItem() {}
    MemMapItem(uint64_t begin, uint64_t end, uint64_t offset, const std::string &name)
        : begin_(begin), end_(end), pageoffset_(offset), name_(name),
        nameHold_(OHOS::Developtools::NativeDaemon::memHolder.HoldStringView(name))
    {
    }

    // use for find
    inline bool operator==(const std::string &name) const
    {
        return name_ == name;
    }

    inline bool operator<(const MemMapItem &other) const
    {
        return end_ < other.end_;
    }

    uint64_t FileOffsetFromAddr(uint64_t addr) const
    {
        // real vaddr - real map begin = addr offset in section
        // section offset + page off set = file offset
        return addr - begin_ + pageoffset_;
    }
    // debug only
    const std::string ToString() const
    {
        std::stringstream sstream;
        sstream << "0x" << std::hex << begin_;
        sstream << "-0x" << std::hex << end_;
        sstream << " type 0x" << std::hex << type_;
        sstream << " flags 0x" << std::hex << flags;
        sstream << " pageoffset 0x" << std::hex << pageoffset_;
        sstream << " " << name_;
        return sstream.str();
    }
    static bool GreaterSort(const MemMapItem &a, const MemMapItem &b)
    {
        return (a.begin_ > b.begin_);
    }
    static bool LessSort(const MemMapItem &a, const MemMapItem &b)
    {
        return (a.begin_ < b.begin_);
    }
    // The range [first, last) must be partitioned with respect to the expression !(value < element)
    // or !comp(value, element)
    static bool ValueLessThan(uint64_t vaddr, const MemMapItem &a)
    {
        return vaddr <= a.begin_;
    }
    bool Contain(uint64_t addr) const
    {
        return addr >= begin_ and addr < end_;
    }
};
#endif
