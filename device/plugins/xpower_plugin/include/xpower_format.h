/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef POWER_FORMAT_H
#define POWER_FORMAT_H

#include <stdlib.h>

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "logging.h"


class ArgBase {
public:
    ArgBase() {}
    virtual ~ArgBase() {}
    virtual void Format(std::ostringstream& ss, const std::string& fmt) = 0;
};

template <class T>
class Arg : public ArgBase {
public:
    Arg(T arg) : m_arg(arg) {}
    virtual ~Arg() {}
    virtual void Format(std::ostringstream& ss, const std::string& fmt)
    {
        ss << m_arg;
    }

private:
    T m_arg;
};

class ArgArray : public std::vector<ArgBase*> {
public:
    ArgArray() {}
    ~ArgArray()
    {
        std::for_each(begin(), end(), [](ArgBase* p) { delete p; });
    }
};

static void FormatItem(std::ostringstream& ss, const std::string& item, const ArgArray& args)
{
    int index = 0;
    int alignment = 0;
    std::string fmt;

    char* endptr = nullptr;
    index = strtol(&item[0], &endptr, 10);
    if (index < 0 || index >= args.size()) {
        return;
    }

    if (*endptr == ',') {
        alignment = strtol(endptr + 1, &endptr, 10);
        if (alignment > 0) {
            ss << std::right << std::setw(alignment);
        } else if (alignment < 0) {
            ss << std::left << std::setw(-alignment);
        }
    }

    if (*endptr == ':') {
        fmt = endptr + 1;
    }

    args[index]->Format(ss, fmt);

    return;
}

template <class T>
static void Transfer(ArgArray& argArray, T t)
{
    argArray.push_back(new Arg<T>(t));
}

template <class T, typename... Args>
static void Transfer(ArgArray& argArray, T t, Args&&... args)
{
    Transfer(argArray, t);
    Transfer(argArray, args...);
}

template <typename... Args>
std::string Format(const std::string& format, Args&&... args)
{
    if (sizeof...(args) == 0) {
        return format;
    }

    ArgArray argArray;
    Transfer(argArray, args...);
    size_t start = 0;
    size_t pos = 0;
    std::ostringstream ss;
    while (true) {
        pos = format.find('{', start);
        if (pos == std::string::npos) {
            ss << format.substr(start);
            break;
        }

        ss << format.substr(start, pos - start);
        if (format[pos + 1] == '{') {
            ss << '{';
            start = pos + 2;
            continue;
        }

        start = pos + 1;
        pos = format.find('}', start);
        if (pos == std::string::npos) {
            ss << format.substr(start - 1);
            break;
        }

        FormatItem(ss, format.substr(start, pos - start), argArray);
        start = pos + 1;
    }

    return ss.str();
}

template <typename... Args>
void PrintHilog(const std::string& format, Args&&... args)
{
    HILOG_INFO(LOG_CORE, "%s", Format(format, args...).c_str());
}

#endif