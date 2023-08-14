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

#ifndef HIEBPF_MAPS_INFO_H_
#define HIEBPF_MAPS_INFO_H_

#include <set>
#include <string>
#include <vector>

namespace OHOS {
namespace Developtools {
namespace Hiebpf {
class MapsInfo {
public:
    struct MapsItem {
        uint64_t start_ = 0;
        uint64_t end_ = 0;
        uint32_t offset_ = 0;
        uint32_t pid_ = 0;
        std::string fileName_;
    };

    MapsInfo() {};
    ~MapsInfo() {};

    bool IsCached(uint32_t pid)
    {
        return (pids_.find(pid) != pids_.end());
    }

    void CachePid(uint32_t pid)
    {
        pids_.insert(pid);
    }

    void RemovePid(uint32_t pid)
    {
        if (auto item = pids_.find(pid); item != pids_.end()) {
            pids_.erase(item);
        }
    }

    void GetMaps(uint32_t pid, std::vector<MapsItem> &mapsItems);
    uint32_t GetBinary(const MapsItem &map, std::vector<uint8_t> &buf);

private:
    void ParseMapsLine(uint32_t pid, std::string &line, std::vector<MapsItem> &maps);
    bool IsLegalFileName(const std::string &fileName);

    std::set<uint32_t> pids_;
};
} // namespace Hiebpf
} // namespace Developtools
} // namespace OHOS
#endif // HIEBPF_MAPS_INFO_H_