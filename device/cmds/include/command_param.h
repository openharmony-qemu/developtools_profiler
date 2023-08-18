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

#ifndef COMMAND_PARAM_H
#define COMMAND_PARAM_H

#include <list>
#include <iosfwd>
#include <string>


enum PARAM_TYPE { PARAM_TYPE_SWITCH, PARAM_TYPE_TEXT };

class CommandParam {
public:
    enum PARAM_TYPE paramType_;
    std::string sDescriptor_;

    std::list<std::string> paramFilter_;

    void AddFilter(const std::string& filterName);
    bool IsInFilter(const std::string& filterName);
};

#endif