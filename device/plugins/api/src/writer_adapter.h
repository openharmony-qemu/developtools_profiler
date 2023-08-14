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

#ifndef WRITER_ADAPTER_H
#define WRITER_ADAPTER_H

#include <memory>
#include "logging.h"
#include "plugin_module_api.h"
#include "writer.h"

using WriterPtr = STD_PTR(shared, Writer);
class WriterAdapter {
public:
    WriterAdapter();
    ~WriterAdapter();
    const WriterStruct* GetStruct();
    void SetWriter(const WriterPtr& writer);
    WriterPtr GetWriter();

private:
    static long WriteFunc(WriterStruct* writer, const void* data, size_t size);
    static bool FlushFunc(WriterStruct* writer);

private:
    WriterStruct writerStruct_;
    WriterPtr writer_;
};

#endif // !WRITER_ADAPTER_H
