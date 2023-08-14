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
 * Description: HitraceOps defines
 */
#ifndef HITRACE_OPS_H
#define HITRACE_OPS_H

#include "trace_ops.h"

FTRACE_NS_BEGIN
class HitraceOps : public TraceOps {
public:
    HitraceOps();
    ~HitraceOps();

private:
    bool PrepareListCategoriesCmd() override;
    bool PrepareEnableCategoriesCmd(int traceTime = 0) override;
    bool PrepareDisableCategoriesCmd() override;
};
FTRACE_NS_END

#endif // HITRACE_OPS_H
