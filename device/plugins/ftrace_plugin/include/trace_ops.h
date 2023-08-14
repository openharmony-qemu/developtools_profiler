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
 * Description: TraceOps defines
 */
#ifndef COMMON_TRACE_OPS_H
#define COMMON_TRACE_OPS_H
#include <string>
#include <vector>
#include "ftrace_namespace.h"

FTRACE_NS_BEGIN
class TraceOps {
public:
    DISALLOW_COPY_AND_MOVE(TraceOps);

    enum TraceType {
        UNKNOW = 0,
        HITRACE,
    };

    TraceOps(const std::string& path, const std::string& arg0, TraceType type);
    virtual ~TraceOps();

    bool IsSupported();

    bool HasCategory(const std::string& name);
    bool EnableCategories(const std::vector<std::string>& categories, int traceTime = 0);
    bool DisableCategories();
    void SetTraceClock(const std::string& traceClock);

    std::string GetCommand() const;
    TraceType GetTraceType() const;

protected:
    virtual std::vector<std::string> ListCategories();
    virtual bool PrepareListCategoriesCmd();
    virtual bool PrepareEnableCategoriesCmd(int traceTime = 0);
    virtual bool PrepareDisableCategoriesCmd();

    int ExecuteCommand(bool out2pipe = true, bool err2pipe = true);

protected:
    std::string arg0_;
    std::string bin_;
    TraceType type_ = UNKNOW;
    std::vector<std::string> targetCategories_;
    std::vector<std::string> supportedCategories_;
    std::vector<std::string> args_;
    std::string output_;
    std::string traceClock_;
};
FTRACE_NS_END

#endif // COMMON_TRACE_OPS_H
