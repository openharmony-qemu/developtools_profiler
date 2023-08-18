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

#include "buffer_splitter.h"

BufferSplitter::BufferSplitter(const char* buf, int size)
{
    next_ = const_cast<char*>(buf);
    nextLine_ = const_cast<char*>(buf);
    end_ = next_ + size;
    if (size) {
        next_[size - 1] = '\0';
        NextLine();
    }
}

bool BufferSplitter::NextLine()
{
    char delimiter = '\n';
    curLine_ = nullptr;
    curLineSize_ = 0;
    curWord_ = nullptr;
    curWordSize_ = 0;

    if (next_ < end_) {
        next_ = nextLine_;
    }
    for (; next_ < end_; next_++) {
        if (*next_ == delimiter) {
            continue;
        }
        curLine_ = next_;
        while (true) {
            if (++next_ >= end_) {
                curLineSize_ = static_cast<size_t>(end_ - curLine_ - 1);
                next_ = curLine_;
                nextLine_ = end_;
                break;
            }
            if (*next_ == delimiter) {
                nextLine_ = ++next_;
                next_ = curLine_;
                curLineSize_ = static_cast<size_t>(nextLine_ - curLine_ - 1);
                break;
            }
        }
        if (curLineSize_ > 0) {
            return true;
        }
        curLine_ = nullptr;
        break;
    }
    return false;
}

bool BufferSplitter::NextWord(char delimiter)
{
    char* nextBak = next_;
    curWord_ = nullptr;
    curWordSize_ = 0;

    for (; next_ < nextLine_; next_++) {
        if (isspace(*next_) || *next_ == delimiter) {
            continue;
        }
        curWord_ = next_;
        while (true) {
            if (++next_ >= nextLine_) {
                curWordSize_ = 0;
                curWord_ = nullptr;
                next_ = nextBak;
                break;
            }

            if (*next_ == delimiter) {
                curWordSize_ = static_cast<size_t>(next_ - curWord_);
                ++next_;
                break;
            }
        }
        if (curWordSize_ > 0) {
            return true;
        }
        break;
    }
    return false;
}