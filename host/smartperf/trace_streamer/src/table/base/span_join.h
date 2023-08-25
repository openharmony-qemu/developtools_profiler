
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
#ifndef SAPN_JOIN_H
#define SAPN_JOIN_H
#include "table_base.h"
#include "trace_data_cache.h"
namespace SysTuning {
namespace TraceStreamer {
struct TableParse {
    std::string name;
    std::string partitionCol;
};

struct TableDesc {
    std::string name;
    std::string partition;
    std::vector<TableBase::ColumnInfo> cols;
    int32_t tsIdx;
    int32_t durIdx;
    int32_t partitionIdx;
};

struct TableColumnInfo {
    TableDesc* tableDesc;
    int32_t colIdx;
};

enum PartitionState {
    TS_REAL,
    TS_PARTITION,
    TS_MISSING,
    TS_EOF,
};

class SpanJoin : public TableBase {
public:
    SpanJoin(const TraceDataCache*);
    ~SpanJoin() override{};
    void Parse(const std::string& tablePartition, TableParse& tableParse);
    void GetTableField(const TableParse& tableParse, TableDesc& tableDesc);
    void GetColumns(const TraceDataCache* dataCache,
                    const std::string& tableName,
                    std::vector<TableBase::ColumnInfo>& columns);
    void CreateCols(TableDesc& tableDesc, std::vector<ColumnInfo>& cols);
    bool IsTsOrDurCol(const std::string& name);
    bool DeduplicationForColumn(const std::string& name, std::vector<ColumnInfo>& cols);
    void Init(int32_t argc, const char* const* argv) override;
    std::unique_ptr<TableBase::Cursor> CreateCursor() override;

    class CaclSpan {
    public:
        CaclSpan(TableBase* tableBase, const TableDesc* tableDesc, sqlite3* db);
        virtual ~CaclSpan();
        static std::string GetMergeColumns(std::vector<std::string>& columns);
        int32_t InitQuerySql(sqlite3_value** argv);
        bool IsQueryNext();
        bool GetCursorNext();
        bool GetNextState();
        bool SearchNextslice();
        void Next();
        std::string GetSqlQuery();
        void setResult(sqlite3_context* context, size_t index) const;
        int64_t GetPatitonForMiss();

    public:
        bool isEof_ = false;
        int64_t ts_ = 0;
        int64_t endTs_ = 0;
        PartitionState partitionState_ = PartitionState::TS_MISSING;
        int32_t partition_ = 0;
        int32_t missPartitionStart_ = 0;
        int32_t missPartitionEnd_ = 0;
        std::string sqlQuery_;
        sqlite3_stmt* stmt_ = nullptr;
        const TableDesc* desc_ = nullptr;
        sqlite3* db_ = nullptr;
        SpanJoin* table_ = nullptr;
    };

    class Cursor : public TableBase::Cursor {
    public:
        explicit Cursor(const TraceDataCache* dataCache, SpanJoin* table);
        int32_t Filter(const FilterConstraints& fc, sqlite3_value** argv) override;
        int32_t Column(int32_t column) const override;
        int32_t Next() override
        {
            queryNext_->Next();
            auto status = IsFindSpan();
            if (!status) {
                return SQLITE_ERROR;
            }
            return SQLITE_OK;
        }
        int32_t Eof() override
        {
            return tableFirst_.isEof_ || tableSecond_.isEof_;
        }

    private:
        bool IsFindSpan();
        bool CaclOverLap();
        CaclSpan* FindQueryResult();
        CaclSpan tableFirst_;
        CaclSpan tableSecond_;
        CaclSpan* queryNext_ = nullptr;
        SpanJoin* spanTable_;
    };

public:
    bool isSamepartitioning_ = false;

private:
    TableDesc tableFirstDesc_;
    TableDesc tableSecondDesc_;
    std::unordered_map<int32_t, TableColumnInfo> mTableColumnInfo_;
};

} // namespace TraceStreamer
} // namespace SysTuning
#endif // SAPN_JOIN_H
