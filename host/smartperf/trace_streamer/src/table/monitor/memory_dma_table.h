/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef MEMORY_DMA_TABLE_H
#define MEMORY_DMA_TABLE_H

#include "table_base.h"
#include "trace_data_cache.h"

namespace SysTuning {
namespace TraceStreamer {
class MemoryDmaTable : public TableBase {
public:
    explicit MemoryDmaTable(const TraceDataCache* dataCache);
    ~MemoryDmaTable() override;
    std::unique_ptr<TableBase::Cursor> CreateCursor() override;

private:
    class Cursor : public TableBase::Cursor {
    public:
        explicit Cursor(const TraceDataCache* dataCache, TableBase* table);
        ~Cursor() override;
        int32_t Column(int32_t column) const override;

    private:
        const DmaMemData& DmaMemDataObj_;
    };
};
} // namespace TraceStreamer
} // namespace SysTuning
#endif // MEMORY_DMA_TABLE_H
