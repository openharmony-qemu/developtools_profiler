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

#include "span_join.h"
#include <vector>
#include "string_help.h"

namespace SysTuning {
namespace TraceStreamer {

const std::string TS_COLUMN_NAME = "ts";
const std::string DUR_COLUMN_NAME = "dur";
constexpr int32_t MINSIZE = 5;
constexpr int32_t MAXSIZE = 1024;
constexpr int32_t NEXT_NUMBER = 1;
constexpr int32_t TSANDDUR_COLUMN = 2;
constexpr int32_t PARTITIONED_COUNT = 3;

enum Index { TS, DUR, PARTITION };

SpanJoin::SpanJoin(const TraceDataCache* dataCache) : TableBase(dataCache)
{
    tableColumn_ = {};
    tablePriKey_ = {};
    tableFirstDesc_ = {};
    tableSecondDesc_ = {};
}

void SpanJoin::Init(int32_t argc, const char* const* argv)
{
    if (argc < MINSIZE) {
        return;
    }
    // Parse the fields of the two tables separately
    TableParse tableFirstParse;
    Parse(std::string(reinterpret_cast<const char*>(argv[3])), tableFirstParse);
    TableParse tableSecondParse;
    Parse(std::string(reinterpret_cast<const char*>(argv[4])), tableSecondParse);

    // you must ensure that the two partitions exist and are the same when using
    if (tableFirstDesc_.partition != tableSecondDesc_.partition) {
        return;
    }
    isSamepartitioning_ = true;
    GetTableField(tableFirstParse, tableFirstDesc_);
    GetTableField(tableSecondParse, tableSecondDesc_);
    tableColumn_.emplace_back(TS_COLUMN_NAME, "INTEGER");
    tableColumn_.emplace_back(DUR_COLUMN_NAME, "INTEGER");
    tableColumn_.emplace_back(tableFirstDesc_.partition, "INTEGER");
    CreateCols(tableFirstDesc_, tableColumn_);
    CreateCols(tableSecondDesc_, tableColumn_);
    std::vector<std::string> primaryKeys = {"ts"};
    primaryKeys.push_back(tableFirstDesc_.partition);
    tablePriKey_ = primaryKeys;
    return;
}

void SpanJoin::CreateCols(TableDesc& tableDesc, std::vector<TableBase::ColumnInfo>& cols)
{
    for (int32_t i = 0; i < tableDesc.cols.size(); i++) {
        auto& n = tableDesc.cols.at(i).name_;
        if (IsTsOrDurCol(n)) {
            continue;
        }
        auto columnInfo = &mTableColumnInfo_[cols.size()];
        columnInfo->tableDesc = &tableDesc;
        columnInfo->colIdx = i;
        if (!DeduplicationForColumn(n, cols)) {
            continue;
        }
        cols.emplace_back(n, tableDesc.cols.at(i).type_);
    }
}

bool SpanJoin::DeduplicationForColumn(const std::string& name, std::vector<ColumnInfo>& cols)
{
    for (size_t i = 0; i < cols.size(); i++) {
        if (name == cols.at(i).name_) {
            return false;
        }
    }
    return true;
}

void SpanJoin::Parse(const std::string& tablePartition, TableParse& tableParse)
{
    std::vector<std::string> result = SplitStringToVec(tablePartition, " ");
    if (result.size() < PARTITIONED_COUNT) {
        TS_LOGW("span_join sql is invalid!");
    }
    tableParse.name = result.at(0);
    if (0 != strcasecmp(result.at(1).c_str(), "PARTITIONED")) {
        TS_LOGW("sql has not PARTITIONED");
        return;
    }
    tableParse.partitionCol = result.at(2);
    return;
}

bool SpanJoin::IsTsOrDurCol(const std::string& name)
{
    if (name == TS_COLUMN_NAME || name == DUR_COLUMN_NAME) {
        return true;
    }
    return false;
}

void SpanJoin::GetTableField(const TableParse& tableParse, TableDesc& tableDesc)
{
    std::vector<TableBase::ColumnInfo> cols;
    GetColumns(dataCache_, tableParse.name, cols);
    int32_t tsDurCount = 0;
    for (int32_t i = 0; i < cols.size(); i++) {
        auto col = cols.at(i);
        if (IsTsOrDurCol(col.name_)) {
            tsDurCount++;
        }
        if (col.name_ == TS_COLUMN_NAME) {
            tableDesc.tsIdx = i;
        } else if (col.name_ == DUR_COLUMN_NAME) {
            tableDesc.durIdx = i;
        } else if (col.name_ == tableParse.partitionCol) {
            tableDesc.partitionIdx = i;
        }
    }
    if (tsDurCount != TSANDDUR_COLUMN) {
        return;
    }
    tableDesc.name = tableParse.name;
    tableDesc.partition = tableParse.partitionCol;
    tableDesc.cols = std::move(cols);
    return;
}

void SpanJoin::GetColumns(const TraceDataCache* dataCache,
                          const std::string& tableName,
                          std::vector<TableBase::ColumnInfo>& columns)
{
    char sql[MAXSIZE];
    std::string querySql = "SELECT name, type from PRAGMA_table_info(\"%s\")";
    int32_t n = snprintf_s(sql, sizeof(sql), 1, querySql.c_str(), tableName.c_str());
    sqlite3_stmt* stmt = nullptr;
    int32_t ret = sqlite3_prepare_v2(dataCache->db_, sql, n, &stmt, nullptr);
    while (!ret) {
        int32_t err = sqlite3_step(stmt);
        if (err == SQLITE_ROW) {
            columns.emplace_back((reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0))),
                                 reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)));
            continue;
        }
        if (err == SQLITE_DONE) {
            break;
        }
        ret = err;
    }
    return;
}

SpanJoin::CaclSpan::CaclSpan(TableBase* tableBase, const TableDesc* tableDesc, sqlite3* db)
    : desc_(tableDesc), db_(db), table_(reinterpret_cast<SpanJoin*>(tableBase))
{
}

SpanJoin::CaclSpan::~CaclSpan() = default;

int32_t SpanJoin::CaclSpan::InitQuerySql(sqlite3_value** argv)
{
    sqlQuery_ = GetSqlQuery();
    bool status = IsQueryNext();
    if (!status) {
        return SQLITE_ERROR;
    }
    return SQLITE_OK;
}

std::string SpanJoin::CaclSpan::GetSqlQuery()
{
    std::vector<std::string> columnNames;
    for (int32_t i = 0; i < desc_->cols.size(); i++) {
        columnNames.push_back(desc_->cols.at(i).name_);
    }
    std::string str;
    str = GetMergeColumns(columnNames);
    std::string sql = "SELECT " + str + " FROM " + desc_->name + " ORDER BY " + desc_->partition + ", " + "ts;";
    return sql;
}

void SpanJoin::CaclSpan::setResult(sqlite3_context* context, size_t index) const
{
    if (partitionState_ != PartitionState::TS_REAL) {
        sqlite3_result_null(context);
        return;
    }
    int32_t sqliteType = sqlite3_column_type(stmt_, index);
    if (sqliteType == SQLITE_TEXT) {
        sqlite3_result_text(context, reinterpret_cast<const char*>(sqlite3_column_int64(stmt_, index)), -1,
                            reinterpret_cast<sqlite3_destructor_type>(-1));
    } else if (sqliteType == SQLITE_INTEGER) {
        sqlite3_result_int64(context, sqlite3_column_int64(stmt_, index));
    } else if (sqliteType == SQLITE_FLOAT) {
        sqlite3_result_double(context, sqlite3_column_double(stmt_, index));
    }
}

bool SpanJoin::CaclSpan::GetCursorNext()
{
    int32_t res;
    int32_t rowType;
    do {
        res = sqlite3_step(stmt_);
        rowType = sqlite3_column_type(stmt_, desc_->partitionIdx);
    } while (res == SQLITE_ROW && rowType == SQLITE_NULL);
    if (res != SQLITE_ROW) {
        isEof_ = true;
    } else {
        isEof_ = false;
    }

    return res == SQLITE_ROW || res == SQLITE_DONE;
}

void SpanJoin::CaclSpan::Next()
{
    GetNextState();
    SearchNextslice();
}

bool SpanJoin::CaclSpan::IsQueryNext()
{
    int32_t res = sqlite3_prepare_v2(db_, sqlQuery_.c_str(), static_cast<int32_t>(sqlQuery_.size()), &stmt_, nullptr);
    isEof_ = res != SQLITE_OK;
    if (res != SQLITE_OK) {
        return true;
    }
    auto status = GetCursorNext();
    if (!status) {
        return false;
    }
    missPartitionEnd_ = sqlite3_column_int64(stmt_, static_cast<int32_t>(desc_->partitionIdx));
    status = SearchNextslice();
    return status;
}

bool SpanJoin::CaclSpan::SearchNextslice()
{
    while (partitionState_ != TS_REAL) {
        bool status = GetNextState();
        if (!status) {
            return false;
        }
    }
    return true;
}

bool SpanJoin::CaclSpan::GetNextState()
{
    switch (partitionState_) {
        case PartitionState::TS_REAL: {
            GetCursorNext();
            partitionState_ = PartitionState::TS_PARTITION;
            ts_ = endTs_;
            if (isEof_ || partition_ != sqlite3_column_int64(stmt_, static_cast<int32_t>(desc_->partitionIdx))) {
                endTs_ = std::numeric_limits<int64_t>::max();
            } else {
                endTs_ = sqlite3_column_int64(stmt_, static_cast<int32_t>(desc_->tsIdx));
            }
            return true;
        }
        case PartitionState::TS_PARTITION: {
            if (endTs_ == std::numeric_limits<int64_t>::max()) {
                partitionState_ = PartitionState::TS_MISSING;
                if (isEof_) {
                    missPartitionEnd_ = std::numeric_limits<int32_t>::max();
                } else {
                    missPartitionEnd_ = sqlite3_column_int64(stmt_, static_cast<int32_t>(desc_->partitionIdx));
                }
                missPartitionStart_ = partition_ + NEXT_NUMBER;
                ts_ = 0;
            } else {
                partitionState_ = PartitionState::TS_REAL;
                ts_ = sqlite3_column_int64(stmt_, static_cast<int32_t>(desc_->tsIdx));
                endTs_ = ts_ + sqlite3_column_int64(stmt_, static_cast<int32_t>(desc_->durIdx));
            }
            return true;
        }
        case PartitionState::TS_MISSING: {
            if (missPartitionEnd_ == std::numeric_limits<int32_t>::max()) {
                partitionState_ = PartitionState::TS_EOF;
            } else {
                partitionState_ = PartitionState::TS_PARTITION;
                ts_ = 0;
                endTs_ = sqlite3_column_int64(stmt_, static_cast<int32_t>(desc_->tsIdx));
                partition_ = missPartitionEnd_;
            }
            return true;
        }
        default:
            return false;
    }
}

std::string SpanJoin::CaclSpan::GetMergeColumns(std::vector<std::string>& columns)
{
    std::string str;
    int32_t size = columns.size();
    for (int32_t i = 0; i < size; i++) {
        if (i == size - 1) {
            str += columns.at(i);
        } else {
            str += columns.at(i) + ", ";
        }
    }
    return str;
}

int64_t SpanJoin::CaclSpan::GetPatitonForMiss()
{
    return partitionState_ == TS_MISSING ? missPartitionEnd_ - NEXT_NUMBER : partition_;
}

std::unique_ptr<TableBase::Cursor> SpanJoin::CreateCursor()
{
    return std::make_unique<Cursor>(dataCache_, this);
}

SpanJoin::Cursor::Cursor(const TraceDataCache* dataCache, SpanJoin* table)
    : TableBase::Cursor(dataCache, table, 0),
      tableFirst_(table, &table->tableFirstDesc_, dataCache_->db_),
      tableSecond_(table, &table->tableSecondDesc_, dataCache_->db_),
      spanTable_(table)
{
}

int32_t SpanJoin::Cursor::Filter(const FilterConstraints& fc, sqlite3_value** argv)
{
    tableFirst_.InitQuerySql(argv);
    tableSecond_.InitQuerySql(argv);
    auto status = IsFindSpan();
    if (!status) {
        return SQLITE_ERROR;
    }
    return SQLITE_OK;
}

bool SpanJoin::Cursor::CaclOverLap()
{
    if (tableFirst_.ts_ >= tableSecond_.ts_) {
        if ((tableFirst_.partitionState_ == PartitionState::TS_REAL &&
             tableSecond_.partitionState_ == PartitionState::TS_REAL) ||
            tableFirst_.ts_ < tableSecond_.endTs_) {
            return true;
        }
    } else if (tableFirst_.ts_ <= tableSecond_.ts_ && tableSecond_.ts_ < tableFirst_.endTs_) {
        return true;
    }
    return false;
}

bool SpanJoin::Cursor::IsFindSpan()
{
    for (;;) {
        if (tableFirst_.isEof_ || tableSecond_.isEof_) {
            break;
        }
        queryNext_ = FindQueryResult();
        if (CaclOverLap()) {
            break;
        }
        queryNext_->Next();
    }
    return true;
}

SpanJoin::CaclSpan* SpanJoin::Cursor::FindQueryResult()
{
    if (!spanTable_->isSamepartitioning_) {
        return nullptr;
    }

    auto tableFirstResult = std::make_tuple(tableFirst_.GetPatitonForMiss(), tableFirst_.endTs_,
                                            tableFirst_.partitionState_ == PartitionState::TS_REAL ? true : false);
    auto tableSecondResult = std::make_tuple(tableSecond_.GetPatitonForMiss(), tableSecond_.endTs_,
                                             tableSecond_.partitionState_ == PartitionState::TS_REAL ? true : false);
    if (tableFirstResult < tableSecondResult) {
        return &tableFirst_;
    }
    return &tableSecond_;
}

int32_t SpanJoin::Cursor::Column(int32_t column) const
{
    switch (column) {
        case TS: {
            sqlite3_result_int64(context_, static_cast<sqlite3_int64>(std::max(tableFirst_.ts_, tableSecond_.ts_)));
            break;
        }
        case DUR: {
            sqlite3_result_int64(context_,
                                 static_cast<sqlite3_int64>(std::min(tableFirst_.endTs_, tableSecond_.endTs_) -
                                                            std::max(tableFirst_.ts_, tableSecond_.ts_)));
            break;
        }
        case PARTITION: {
            auto partResult = tableFirst_.partitionState_ == PartitionState::TS_REAL ? tableFirst_.partition_
                                                                                     : tableSecond_.partition_;
            sqlite3_result_int64(context_, static_cast<sqlite3_int64>(partResult));
            break;
        }
        default: {
            const auto ColumnInfo = spanTable_->mTableColumnInfo_[column];
            if (ColumnInfo.tableDesc == tableFirst_.desc_) {
                tableFirst_.setResult(context_, ColumnInfo.colIdx);
            } else {
                tableSecond_.setResult(context_, ColumnInfo.colIdx);
            }
        }
    }
    return SQLITE_OK;
}

} // namespace TraceStreamer
} // namespace SysTuning
