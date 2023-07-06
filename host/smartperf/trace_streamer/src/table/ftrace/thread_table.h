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

#ifndef SRC_THREAD_TABLE_H
#define SRC_THREAD_TABLE_H

#include "table_base.h"
#include "trace_data_cache.h"

namespace SysTuning {
namespace TraceStreamer {
class ThreadTable : public TableBase {
public:
    explicit ThreadTable(const TraceDataCache* dataCache);
    ~ThreadTable() override;
    std::unique_ptr<TableBase::Cursor> CreateCursor() override;

private:
    void EstimateFilterCost(FilterConstraints& fc, EstimatedIndexInfo& ei) override;
    int32_t Update(int32_t argc, sqlite3_value** argv, sqlite3_int64* pRowid) override;
    void FilterByConstraint(FilterConstraints& fc, double& filterCost, size_t rowCount);

    class Cursor : public TableBase::Cursor {
    public:
        explicit Cursor(const TraceDataCache* dataCache, TableBase* table);
        ~Cursor() override;
        void FilterIpid(unsigned char op, uint64_t value);
        void FilterTid(unsigned char op, uint64_t value);
        void FilterSwitchCount(unsigned char op, uint64_t value);
        void FilterIndex(int32_t col, unsigned char op, sqlite3_value* argv);
        int32_t Filter(const FilterConstraints& fc, sqlite3_value** argv) override;
        int32_t Column(int32_t col) const override;

        void FilterId(unsigned char op, sqlite3_value* argv) override;

    private:
        std::vector<TableRowId> rowIndexBak_;
        IndexMap* indexMapBack_ = nullptr;
    };
};
} // namespace TraceStreamer
} // namespace SysTuning

#endif // SRC_THREAD_TABLE_H
