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

#include "maps_info.h"

#include <algorithm>
#include <cstring>
#include <sstream>

#include "hhlog.h"
#include "utilities.h"

namespace OHOS {
namespace Developtools {
namespace Hiebpf {
void MapsInfo::GetMaps(uint32_t pid, std::vector<MapsItem> &mapsItems)
{
    mapsItems.clear();
    std::string mapPath = "/proc/" + std::to_string(pid) + "/maps";
    std::string mapContent = ReadFileToString(mapPath);
    if (mapContent.empty()) {
        HHLOGE(true, "%s no content", mapPath.c_str());
        return;
    }

    std::istringstream s(mapContent);
    std::string line;
    while (std::getline(s, line)) {
        HHLOGD(true, "mmap line: %s", line.c_str());
        ParseMapsLine(pid, line, mapsItems);
    }

    return;
}

uint32_t MapsInfo::GetBinary(const MapsItem &map, std::vector<uint8_t> &buf)
{
    uint32_t fixLen = sizeof(map.start_) + sizeof(map.end_) + sizeof(map.offset_) + sizeof(map.pid_);
    uint32_t len = fixLen + sizeof(uint32_t) + map.fileName_.size() + 1;
    buf.resize(len);

    const uint8_t *rp = reinterpret_cast<const uint8_t *>(&map);
    uint8_t *wp = buf.data();
    std::copy(rp, rp + fixLen, wp); // start + end + offset
    wp += fixLen;
    *(reinterpret_cast<uint32_t *>(wp)) = map.fileName_.size() + 1; // fileNameLen
    wp += sizeof(uint32_t);
    std::copy(map.fileName_.c_str(), map.fileName_.c_str() + map.fileName_.size() + 1, wp); // fileName

    return buf.size();
}

constexpr const int MMAP_LINE_TOKEN_INDEX_ADDR = 0;
constexpr const int MMAP_LINE_TOKEN_INDEX_FLAG = 1;
constexpr const int MMAP_LINE_TOKEN_INDEX_OFFSET = 2;
constexpr const int MMAP_LINE_TOKEN_INDEX_DEV = 3;
constexpr const int MMAP_LINE_TOKEN_INDEX_INODE = 4;
constexpr const int MMAP_LINE_TOKEN_INDEX_NAME = 5;
constexpr const int MMAP_LINE_TOKEN_NUMBER = 6;
void MapsInfo::ParseMapsLine(uint32_t pid, std::string &line, std::vector<MapsItem> &maps)
{
    // b0023000-b0024000 r-xp 00000000 b3:05 959        /system/lib/libdl.so
    // 0                 1    2        3     4          5
    std::vector<std::string> mapTokens = StringSplit(line, " ");
    if (mapTokens.size() < MMAP_LINE_TOKEN_NUMBER) {
        // maybe file name is empty
        return;
    }
    // check device
    std::vector<std::string> dev = StringSplit(mapTokens[MMAP_LINE_TOKEN_INDEX_DEV], ":");
    if (0 == std::stoi(dev[0], nullptr, NUMBER_FORMAT_HEX_BASE)) {
        return;
    }

    // check file name
    if (!IsLegalFileName(mapTokens[MMAP_LINE_TOKEN_INDEX_NAME])) {
        return;
    }

    MapsItem &mapItem = maps.emplace_back();

    std::vector<std::string> addr = StringSplit(mapTokens[MMAP_LINE_TOKEN_INDEX_ADDR], "-");
    mapItem.start_ = std::stoull(addr[0], nullptr, NUMBER_FORMAT_HEX_BASE);
    mapItem.end_ = std::stoull(addr[1], nullptr, NUMBER_FORMAT_HEX_BASE);
    mapItem.offset_ = std::stoull(mapTokens[MMAP_LINE_TOKEN_INDEX_OFFSET],
                                  nullptr, NUMBER_FORMAT_HEX_BASE);;
    mapItem.pid_ = pid;
    mapItem.fileName_ = mapTokens[MMAP_LINE_TOKEN_INDEX_NAME];
}

bool MapsInfo::IsLegalFileName(const std::string &fileName)
{
    if (fileName.find(':') != std::string::npos or
        fileName.front() == '[' or fileName.back() == ']' or
        std::strncmp(fileName.c_str(), "/dev/", sizeof("/dev/")) == 0 or
        std::strncmp(fileName.c_str(), "/memfd:", sizeof("/memfd:")) == 0 or
        std::strncmp(fileName.c_str(), "//anon", sizeof("//anon")) == 0 or
        StringEndsWith(fileName, ".ttf")) {
        return false;
    }
    return true;
}
} // namespace Hiebpf
} // namespace Developtools
} // namespace OHOS