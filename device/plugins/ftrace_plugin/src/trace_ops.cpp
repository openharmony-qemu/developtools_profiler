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
 *
 * Description: TraceOps implements
 */
#include "trace_ops.h"

#include <algorithm>
#include <cerrno>
#include <csignal>
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <functional>
#include <memory>
#include <sstream>
#include <unistd.h>

#include "logging.h"
#include "process_utils.h"
#include "string_utils.h"

FTRACE_NS_BEGIN
TraceOps::TraceOps(const std::string& path, const std::string& arg0, TraceType type)
    : arg0_(arg0), bin_(path), type_(type)
{
}

TraceOps::~TraceOps() {}

std::string TraceOps::GetCommand() const
{
    return bin_;
}

TraceOps::TraceType TraceOps::GetTraceType() const
{
    return type_;
}

bool TraceOps::IsSupported()
{
    return access(bin_.c_str(), F_OK | X_OK) == 0;
}

bool TraceOps::HasCategory(const std::string& name)
{
    if (supportedCategories_.empty()) {
        supportedCategories_ = ListCategories();
        std::sort(supportedCategories_.begin(), supportedCategories_.end());
    }
    return std::binary_search(supportedCategories_.begin(), supportedCategories_.end(), name);
}

std::vector<std::string> TraceOps::ListCategories()
{
    std::vector<std::string> result;
    args_ = {arg0_};
    CHECK_TRUE(PrepareListCategoriesCmd(), result, "prepare list categories command failed!");

    int retval = ExecuteCommand(true, true);
    HILOG_INFO(LOG_CORE, "exec %s exit %d!", bin_.c_str(), retval);

    std::string line;
    std::stringstream stream(output_);
    while (std::getline(stream, line)) {
        line = StringUtils::Strip(line);

        // format likes: freq - CPU Frequency
        auto pos = line.find('-');
        if (pos == std::string::npos) {
            continue;
        }

        auto category = line.substr(0, pos - 1);
        result.push_back(category);
    }
    return result;
}

bool TraceOps::PrepareListCategoriesCmd()
{
    return false;
}

bool TraceOps::PrepareEnableCategoriesCmd(int traceTime)
{
    return false;
}

bool TraceOps::PrepareDisableCategoriesCmd()
{
    return false;
}

void TraceOps::SetTraceClock(const std::string& traceClock)
{
    traceClock_ = traceClock;
}

bool TraceOps::EnableCategories(const std::vector<std::string>& categories, int traceTime)
{
    CHECK_TRUE(categories.size() > 0, false, "categories empty!");
    for (auto& category : categories) {
        if (HasCategory(category)) {
            targetCategories_.push_back(category);
        } else {
            HILOG_ERROR(LOG_CORE, "\"%s\" is not support category on this device", category.c_str());
        }
    }

    args_ = {arg0_};
    if (!traceClock_.empty()) {
        args_.push_back("--trace_clock");
        args_.push_back(traceClock_);
    }
    CHECK_TRUE(PrepareEnableCategoriesCmd(traceTime), false, "prepare enable categories failed!");

    int retval = ExecuteCommand();
    HILOG_INFO(LOG_CORE, "exec %s exit %d!", bin_.c_str(), retval);
    CHECK_TRUE(retval == 0, false, "exec %s failed with %d", bin_.c_str(), retval);
    return true;
}

bool TraceOps::DisableCategories()
{
    args_ = {arg0_};
    CHECK_TRUE(PrepareDisableCategoriesCmd(), false, "prepare enable categories failed!");

    int retval = ExecuteCommand();
    HILOG_INFO(LOG_CORE, "exec %s exit %d!", bin_.c_str(), retval);

    targetCategories_.clear();
    CHECK_TRUE(retval == 0, false, "exec %s failed with %d", bin_.c_str(), retval);
    return true;
}

int TraceOps::ExecuteCommand(bool out2pipe, bool err2pipe)
{
    CHECK_TRUE(bin_.size() > 0, -1, "bin_ empty");
    CHECK_TRUE(args_.size() > 0, -1, "args_ empty");

    ExecuteArgs execArgs = {};
    execArgs.bin_ = bin_;
    execArgs.argv_ = args_;
    execArgs.out2pipe_ = out2pipe;
    execArgs.err2pipe_ = err2pipe;

    std::string cmdline = StringUtils::Join(args_, " ");
    int retval = ProcessUtils::Execute(execArgs, output_);
    return retval;
}
FTRACE_NS_END
