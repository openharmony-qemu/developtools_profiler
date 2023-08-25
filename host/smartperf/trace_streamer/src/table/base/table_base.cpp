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

#include "table_base.h"

#include <cctype>
#include <cstring>

#include "log.h"

#define UNUSED(expr)             \
    do {                         \
        static_cast<void>(expr); \
    } while (0)

namespace SysTuning {
namespace TraceStreamer {
namespace {
struct TableContext {
    TabTemplate tmplate;
    TraceDataCache* dataCache;
    sqlite3_module module;
    std::string tableName;
};
} // namespace

TableBase::~TableBase()
{
    dataCache_ = nullptr;
    cursor_ = nullptr;
}

void TableBase::TableRegister(sqlite3& db, TraceDataCache* cache, const std::string& tableName, TabTemplate tmplate)
{
    std::unique_ptr<TableContext> context(std::make_unique<TableContext>());
    context->dataCache = cache;
    context->tmplate = tmplate;
    context->tableName = tableName;
    sqlite3_module& module = context->module;
    module = {0};

    auto createFn = [](sqlite3* xdb, void* pAux, int32_t argc, const char* const* argv, sqlite3_vtab** ppVTab,
                       char** pzErr) {
        UNUSED(argc);
        UNUSED(argv);
        UNUSED(pzErr);
        auto xdesc = static_cast<const TableContext*>(pAux);
        auto table = xdesc->tmplate(xdesc->dataCache);
        table->name_ = xdesc->tableName;
        if (table->name_ == "process" || table->name_ == "thread") {
            table->wdataCache_ = xdesc->dataCache;
        }

        table->Init(argc, argv);
        std::string createStmt = table->CreateTableSql();
        TS_LOGD("xCreate table %s, statement: %s", table->name_.c_str(), createStmt.c_str());
        int32_t ret = sqlite3_declare_vtab(xdb, createStmt.c_str());
        if (ret != SQLITE_OK) {
            if ((table->name_ == "span_join") || (table->name_ == "_span_join")) {
                return ret;
            }
            TS_LOGE("sqlite3_declare_vtab %s faild: %s", table->name_.c_str(), createStmt.c_str());
            return ret;
        }
        *ppVTab = table.release();
        return SQLITE_OK;
    };

    auto destroyFn = [](sqlite3_vtab* t) {
        TS_LOGD("xDestroy table %s", static_cast<TableBase*>(t)->name_.c_str());
        delete static_cast<TableBase*>(t);
        return SQLITE_OK;
    };
    module.xCreate = createFn;
    module.xConnect = createFn;
    module.xDisconnect = destroyFn;
    module.xDestroy = destroyFn;

    module.xOpen = [](sqlite3_vtab* pVTab, sqlite3_vtab_cursor** ppCursor) {
        TS_LOGD("xOpen: %s", static_cast<TableBase*>(pVTab)->name_.c_str());
        return static_cast<TableBase*>(pVTab)->Open(ppCursor);
    };

    module.xClose = [](sqlite3_vtab_cursor* vc) {
        TS_LOGD("xClose: %s", static_cast<Cursor*>(vc)->table_->name_.c_str());
        delete static_cast<Cursor*>(vc);
        return SQLITE_OK;
    };

    module.xBestIndex = [](sqlite3_vtab* pVTab, sqlite3_index_info* idxInfo) {
        TS_LOGD("xBestIndex: %s %d", static_cast<TableBase*>(pVTab)->name_.c_str(), idxInfo->nConstraint);
        return static_cast<TableBase*>(pVTab)->BestIndex(idxInfo);
    };

    module.xFilter = [](sqlite3_vtab_cursor* vc, int32_t idxNum, const char* idxStr, int32_t argc,
                        sqlite3_value** argv) {
        auto* c = static_cast<Cursor*>(vc);
        c->Reset();
        TS_LOGD("xFilter %s: [%d]%s", static_cast<Cursor*>(vc)->table_->name_.c_str(), idxNum, idxStr);
        if (c->table_->cacheIdxNum_ != idxNum) {
            c->table_->cacheConstraint_.Clear();
            c->table_->cacheConstraint_.FromString(idxStr);
            c->table_->cacheIdxNum_ = idxNum;
        }
        return c->Filter(c->table_->cacheConstraint_, argv);
    };

    module.xNext = [](sqlite3_vtab_cursor* vc) { return static_cast<TableBase::Cursor*>(vc)->Next(); };
    module.xEof = [](sqlite3_vtab_cursor* vc) { return static_cast<TableBase::Cursor*>(vc)->Eof(); };
    module.xColumn = [](sqlite3_vtab_cursor* vc, sqlite3_context* ctx, int32_t col) {
        static_cast<TableBase::Cursor*>(vc)->context_ = ctx;
        return static_cast<TableBase::Cursor*>(vc)->Column(col);
    };
    if (tableName == "process" || tableName == "thread") {
        module.xUpdate = [](sqlite3_vtab* pVTab, int32_t argc, sqlite3_value** argv, sqlite3_int64* pRowid) {
            TS_LOGD("xUpdate: %s", static_cast<TableBase*>(pVTab)->name_.c_str());
            return static_cast<TableBase*>(pVTab)->Update(argc, argv, pRowid);
        };
    }

    sqlite3_create_module_v2(&db, tableName.c_str(), &module, context.release(),
                             [](void* arg) { delete static_cast<TableContext*>(arg); });
}

std::string TableBase::CreateTableSql() const
{
    std::string stmt = "CREATE TABLE x(";
    for (const auto& col : tableColumn_) {
        stmt += " " + col.name_ + " " + col.type_;
        stmt += ",";
    }
    stmt += " PRIMARY KEY(";
    for (size_t i = 0; i < tablePriKey_.size(); i++) {
        if (i != 0)
            stmt += ", ";
        stmt += tablePriKey_.at(i);
    }
    stmt += ")) WITHOUT ROWID;";
    return stmt;
}
int32_t TableBase::Cursor::Next()
{
    indexMap_->Next();
    return SQLITE_OK;
}

int32_t TableBase::Cursor::Eof()
{
    return dataCache_->Cancel() || indexMap_->Eof();
}
uint32_t TableBase::Cursor::CurrentRow() const
{
    return indexMap_->CurrentRow();
}
void TableBase::Cursor::FilterEnd()
{
    indexMap_->Sort();
}
int32_t TableBase::BestIndex(sqlite3_index_info* idxInfo)
{
    FilterConstraints filterConstraints;
    for (int32_t i = 0; i < idxInfo->nConstraint; i++) {
        const auto& constraint = idxInfo->aConstraint[i];
        if (constraint.usable) {
            filterConstraints.AddConstraint(i, constraint.iColumn, constraint.op);
        }
    }
    for (int32_t i = 0; i < idxInfo->nOrderBy; i++) {
        filterConstraints.AddOrderBy(idxInfo->aOrderBy[i].iColumn, idxInfo->aOrderBy[i].desc);
    }

    EstimatedIndexInfo estimate = {idxInfo->estimatedRows, idxInfo->estimatedCost, false};
    EstimateFilterCost(filterConstraints, estimate);
    idxInfo->orderByConsumed = estimate.isOrdered;
    idxInfo->estimatedCost = estimate.estimatedCost;
    idxInfo->estimatedRows = estimate.estimatedRows;

    auto cs = filterConstraints.GetConstraints();
    for (size_t i = 0; i < cs.size(); i++) {
        auto& c = cs[i];
        idxInfo->aConstraintUsage[c.idxInaConstraint].argvIndex = static_cast<int32_t>(i + 1);
        idxInfo->aConstraintUsage[c.idxInaConstraint].omit = c.isSupport;
    }

    std::string str;
    filterConstraints.ToString(str);
    char* pIdxStr = static_cast<char*>(sqlite3_malloc(str.size() + 1));
    std::copy(str.begin(), str.end(), pIdxStr);
    pIdxStr[str.size()] = '\0';
    idxInfo->idxStr = pIdxStr;
    idxInfo->needToFreeIdxStr = true;
    idxInfo->idxNum = ++bestIndexNum_;

    TS_LOGD("%s BestIndex return: %d: %s", name_.c_str(), idxInfo->idxNum, str.c_str());
    TS_LOGD("%s, aConstraintUsage[%d]", idxInfo->idxStr, idxInfo->nConstraint);
    for (int32_t i = 0; i < idxInfo->nConstraint; i++) {
        TS_LOGD("col: %d op: %d, argvindex: %d omit: %d", idxInfo->aConstraint[i].iColumn, idxInfo->aConstraint[i].op,
                idxInfo->aConstraintUsage[i].argvIndex, idxInfo->aConstraintUsage[i].omit);
    }
    TS_LOGD("estimated: %lld cost:%.3f", idxInfo->estimatedRows, idxInfo->estimatedCost);

    return SQLITE_OK;
}

int32_t TableBase::Open(sqlite3_vtab_cursor** ppCursor)
{
    *ppCursor = static_cast<sqlite3_vtab_cursor*>(CreateCursor().release());
    return SQLITE_OK;
}

TableBase::Cursor::Cursor(const TraceDataCache* dataCache, TableBase* table, uint32_t rowCount)
    : context_(nullptr),
      table_(table),
      dataCache_(dataCache),
      indexMap_(std::make_unique<IndexMap>(0, rowCount)),
      rowCount_(rowCount)
{
}

bool TableBase::CanFilterId(const char op, size_t& rowCount)
{
    switch (op) {
        case SQLITE_INDEX_CONSTRAINT_EQ:
            rowCount = 1;
            break;
        case SQLITE_INDEX_CONSTRAINT_GT:
        case SQLITE_INDEX_CONSTRAINT_GE:
        case SQLITE_INDEX_CONSTRAINT_LE:
        case SQLITE_INDEX_CONSTRAINT_LT:
            // assume filter out a half of rows
            rowCount = (rowCount >> 1);
            break;
        default:
            return false;
    }
    return true;
}

TableBase::Cursor::~Cursor()
{
    context_ = nullptr;
    dataCache_ = nullptr;
}
void TableBase::Cursor::FilterTS(unsigned char op, sqlite3_value* argv, const std::deque<InternalTime>& times)
{
    auto v = static_cast<uint64_t>(sqlite3_value_int64(argv));
    auto getValue = [](const uint64_t& row) { return row; };
    switch (op) {
        case SQLITE_INDEX_CONSTRAINT_EQ:
            indexMap_->IntersectabcEqual(times, v, getValue);
            break;
        case SQLITE_INDEX_CONSTRAINT_GT:
            v++;
        case SQLITE_INDEX_CONSTRAINT_GE: {
            indexMap_->IntersectGreaterEqual(times, v, getValue);
            break;
        }
        case SQLITE_INDEX_CONSTRAINT_LE:
            v++;
        case SQLITE_INDEX_CONSTRAINT_LT: {
            indexMap_->IntersectLessEqual(times, v, getValue);
            break;
        }
        case SQLITE_INDEX_CONSTRAINT_ISNOTNULL: {
            indexMap_->RemoveNullElements(times, v);
            break;
        }
        default:
            break;
    } // end of switch (op)
}

int32_t TableBase::Cursor::RowId(sqlite3_int64* id)
{
    if (dataCache_->Cancel() || indexMap_->Eof()) {
        return SQLITE_ERROR;
    }
    *id = static_cast<sqlite3_int64>(indexMap_->CurrentRow());
    return SQLITE_OK;
}
void TableBase::Cursor::FilterId(unsigned char op, sqlite3_value* argv)
{
    auto type = sqlite3_value_type(argv);
    if (type != SQLITE_INTEGER) {
        // other type consider it NULL
        indexMap_->Intersect(0, 0);
        return;
    }
    if (indexMap_->HasData()) {
        indexMap_->CovertToIndexMap();
    }

    auto v = static_cast<TableRowId>(sqlite3_value_int64(argv));
    switch (op) {
        case SQLITE_INDEX_CONSTRAINT_EQ:
            indexMap_->Intersect(v, v + 1);
            break;
        case SQLITE_INDEX_CONSTRAINT_GE:
            indexMap_->Intersect(v, rowCount_);
            break;
        case SQLITE_INDEX_CONSTRAINT_GT:
            v++;
            indexMap_->Intersect(v, rowCount_);
            break;
        case SQLITE_INDEX_CONSTRAINT_LE:
            v++;
            indexMap_->Intersect(0, v);
            break;
        case SQLITE_INDEX_CONSTRAINT_LT:
            indexMap_->Intersect(0, v);
            break;
        default:
            // can't filter, all rows
            break;
    }
}
} // namespace TraceStreamer
} // namespace SysTuning
