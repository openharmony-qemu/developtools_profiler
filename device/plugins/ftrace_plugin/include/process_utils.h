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
#ifndef PROCESS_UTILS_H
#define PROCESS_UTILS_H

#include <string>
#include <vector>

struct ExecuteArgs {
    std::string bin_;
    std::vector<std::string> argv_;
    bool out2pipe_ = true;
    bool err2pipe_ = false;
};

class ProcessUtils {
public:
    static int Execute(const ExecuteArgs& args, std::string& output);
};

#endif // PROCESS_UTILS_H