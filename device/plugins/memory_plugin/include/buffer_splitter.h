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

#ifndef BUFFER_SPLITTER_H
#define BUFFER_SPLITTER_H

#include <string>

/**
 * Description: Buffer Splitter Class
 * The BufferSplitter class object splits the buffer content by moving the pointer
 * Except for the last character, other buffer contents are not modified during the segmentation
 */

class BufferSplitter {
public:
    /* During construction, buf[size-1] will be forcibly modified to '\ 0'
       The constructor will automatically call the NextLine to initialize the first line of data */
    BufferSplitter(const char* buf, int size);

    ~BufferSplitter() {}
    /* Find and update the next line header pointer, and line length. Length does not include '\ n' and '\ 0' */
    bool NextLine();

    /* Find and update the NextWord's head pointer and Word length according to the delimiter within the
       current line range. The length calculation does not include the 'delimiter' */
    /* When the current line cannot find a specific 'delimiter', it can
        be split from the current position according to the new 'delimiter' */
    bool NextWord(char delimiter);

    const char* CurWord()
    {
        return curWord_;
    }
    size_t CurWordSize() const
    {
        return curWordSize_;
    }
    char* CurLine()
    {
        return curLine_;
    }
    size_t CurLineSize() const
    {
        return curLineSize_;
    }

private:
    char* curWord_ = nullptr;
    size_t curWordSize_ = 0;
    char* next_;
    char* curLine_ = nullptr;
    size_t curLineSize_ = 0;
    char* nextLine_;
    char* end_;
};
#endif
