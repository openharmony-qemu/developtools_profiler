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
#ifndef HIPERF_PERF_EVENT_RECORD_H
#define HIPERF_PERF_EVENT_RECORD_H

#include <atomic>
#include <chrono>
#include <map>
#include <memory>
#include <stdint.h>
#include <string>
#include <variant>
#include <vector>

#include <sys/types.h>
#include <unique_fd.h>
#include <linux/perf_event.h>
#include <linux/types.h>

#include "debug_logger.h"
#include "utilities.h"

namespace OHOS {
namespace Developtools {
namespace NativeDaemon {
namespace {
constexpr uint8_t CALL_FRAME_REPORT = 0x01; // 0001
constexpr uint8_t SYMBOL_NAME_ID_REPORT = 0x02; // 0010
constexpr uint8_t FILE_PATH_ID_REPORT = 0x04; // 0100
}
struct CallFrame {
    uint64_t ip_ = 0;
    uint64_t sp_ = 0;

    uint64_t vaddrInFile_ = 0; // in symbol file vaddr
    int32_t symbolIndex_ = -1; // symbols index , should update after sort
    std::string_view symbolName_;
    std::string_view filePath_; // lib path , elf path
    uint64_t offset_ = 0;
    uint64_t symbolOffset_ = 0;
    uint32_t filePathId_ = 0; // for memMpaItem filePathId_
    uint32_t symbolNameId_ = 0; // for symbolName_ id
    uint32_t callFrameId_ = 0; // for frame map id
    uint8_t needReport_ = 0; // 0X01 for CALL_FRAME_REPORT,0x02 for SYMBOL_NAME_ID_REPORT,0x04 for FILE_PATH_ID_REPORT

    CallFrame(uint64_t ip, uint64_t sp = 0) : ip_(ip), sp_(sp) {}

    // this is for ut test
    CallFrame(uint64_t ip, uint64_t vaddrInFile, const char *name, const char *filePath)
        : ip_(ip), vaddrInFile_(vaddrInFile), symbolName_(name), filePath_(filePath)
    {
    }
    bool operator==(const CallFrame &b) const
    {
        return (ip_ == b.ip_) && (sp_ == b.sp_);
    }
    bool operator!=(const CallFrame &b) const
    {
        return (ip_ != b.ip_) || (sp_ != b.sp_);
    }
    std::string ToString() const
    {
        return StringPrintf("ip: 0x%016llx sp: 0x%016llx", ip_, sp_);
    }
    std::string ToSymbolString() const
    {
        std::string output;
        if (vaddrInFile_ != 0) {
            output = StringPrintf("va: 0x%016llx(%llx) ", vaddrInFile_, ip_);
        } else {
            output = StringPrintf("ip: 0x%016llx ", ip_);
        }
        output.append(": ");
        output.append(symbolName_);

        output.append("@");
        output.append(filePath_);
        if (symbolIndex_ != -1) {
            output.append(":");
            output.append(std::to_string(symbolIndex_));
        }
        return output;
    }
};

struct AttrWithId {
    perf_event_attr attr;
    std::vector<uint64_t> ids;
    std::string name; // will be empty in GetAttrSection
};
} // namespace NativeDaemon
} // namespace Developtools
} // namespace OHOS
#endif