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

#include "index_map.h"

#include <algorithm>

#include "log.h"

namespace SysTuning {
namespace TraceStreamer {
IndexMap::IndexMap(TableRowId start, TableRowId end) : end_(end), current_(start), start_(start) {}

void IndexMap::CovertToIndexMap()
{
    if (converted_) {
        indexType_ = INDEX_TYPE_OUTER_INDEX;
        return;
    }
    converted_ = true;
    if (indexType_ == INDEX_TYPE_ID && HasData()) {
        for (auto i = start_; i < end_; i++) {
            rowIndex_.push_back(i);
        }
        current_ = start_ = 0;
        end_ = rowIndex_.size();
    }
    indexType_ = INDEX_TYPE_OUTER_INDEX;
    empty_ = false;
}

void IndexMap::Print()
{
    for (auto itor = rowIndex_.begin(); itor != rowIndex_.end(); itor++) {
        fprintf(stdout, "%d,", *itor);
    }
    fflush(stdout);
}
void IndexMap::Sort() {}

void IndexMap::Init()
{
    intersectEable_ = HasData();
}
bool IndexMap::HasData() const
{
    return !empty_;
}
void IndexMap::Intersect(TableRowId start, TableRowId end)
{
    if (indexType_ == INDEX_TYPE_OUTER_INDEX) {
        bool changed = false;
        rowIndexBak_.clear();
        for (auto i = rowIndex_.begin(); i != rowIndex_.end(); i++) {
            if (*i >= start && *i < end) {
                changed = true;
                rowIndexBak_.push_back(*i);
            }
        }
        if (changed) {
            rowIndex_ = rowIndexBak_;
        }
        start_ = current_ = 0;
        end_ = rowIndex_.size();
    } else {
        start_ = std::max(start_, start);
        end_ = std::min(end_, end);
        current_ = start_;
    }
    empty_ = false;
}

size_t IndexMap::Size() const
{
    return end_ - start_;
}

void IndexMap::Next()
{
    if (desc_) {
        if (current_ > start_ - 1) {
            current_--;
        }
    } else {
        if (current_ < end_) {
            current_++;
        }
    }
}
void IndexMap::FilterId(unsigned char op, sqlite3_value* argv)
{
    auto type = sqlite3_value_type(argv);
    if (type != SQLITE_INTEGER) {
        // other type consider it NULL
        Intersect(0, 0);
        return;
    }
    if (HasData()) {
        CovertToIndexMap();
    }

    auto v = static_cast<TableRowId>(sqlite3_value_int64(argv));
    switch (op) {
        case SQLITE_INDEX_CONSTRAINT_EQ:
            Intersect(v, v + 1);
            break;
        case SQLITE_INDEX_CONSTRAINT_GE:
            Intersect(v, end_);
            break;
        case SQLITE_INDEX_CONSTRAINT_GT:
            v++;
            Intersect(v, end_);
            break;
        case SQLITE_INDEX_CONSTRAINT_LE:
            v++;
            Intersect(0, v);
            break;
        case SQLITE_INDEX_CONSTRAINT_LT:
            Intersect(0, v);
            break;
        default:
            // can't filter, all rows
            break;
    }
}

void IndexMap::FilterTS(unsigned char op, sqlite3_value* argv, const std::deque<InternalTime>& times)
{
    auto v = static_cast<uint64_t>(sqlite3_value_int64(argv));
    auto getValue = [](const uint64_t& row) { return row; };
    switch (op) {
        case SQLITE_INDEX_CONSTRAINT_EQ:
            IntersectabcEqual(times, v, getValue);
            break;
        case SQLITE_INDEX_CONSTRAINT_GT:
            v++;
            [[fallthrough]];
        case SQLITE_INDEX_CONSTRAINT_GE: {
            IntersectGreaterEqual(times, v, getValue);
            break;
        }
        case SQLITE_INDEX_CONSTRAINT_LE:
            v++;
            [[fallthrough]];
        case SQLITE_INDEX_CONSTRAINT_LT:
            IntersectLessEqual(times, v, getValue);
            break;
        case SQLITE_INDEX_CONSTRAINT_ISNOTNULL: {
            RemoveNullElements(times, v);
            break;
        }
        default:
            break;
    } // end of switch (op)
}
void IndexMap::Merge(IndexMap* other)
{
    if (indexType_ == INDEX_TYPE_ID && other->indexType_ == INDEX_TYPE_ID) {
        if ((other->start_ >= start_ && other->start_ <= end_) || (start_ >= other->start_ && start_ <= other->end_)) {
            start_ = std::min(start_, other->start_);
            end_ = std::max(end_, other->end_);
        } else if (start_ > other->start_) {
            this->CovertToIndexMap();
            other->CovertToIndexMap();
            const std::vector<TableRowId> b = other->rowIndex_;
            uint32_t bIndex = 0;
            uint32_t bSize = b.size();
            while (bIndex != bSize) {
                rowIndex_.push_back(b[bIndex]);
            }
            start_ = current_ = 0;
            end_ = rowIndex_.size();
        } else {
            this->CovertToIndexMap();
            other->CovertToIndexMap();
            std::vector<TableRowId> c = other->rowIndex_;
            uint32_t aIndex = 0;
            uint32_t aSize = rowIndex_.size();
            while (aIndex != aSize) {
                c.push_back(rowIndex_[aIndex]);
            }
            start_ = current_ = 0;
            end_ = rowIndex_.size();
        }
        return;
    }
    this->CovertToIndexMap();
    other->CovertToIndexMap();
    const std::vector<TableRowId> b = other->rowIndex_;
    const std::vector<TableRowId>& a = rowIndex_;
    std::vector<TableRowId> c;
    uint32_t aIndex = 0;
    uint32_t aSize = a.size();
    uint32_t bIndex = 0;
    uint32_t bSize = b.size();
    while (aIndex != aSize || bIndex != bSize) {
        if (aIndex == aSize) {
            while (bIndex != bSize) {
                c.push_back(b[bIndex]);
                bIndex++;
            }
            break;
        }
        if (bIndex == bSize) {
            while (aIndex != aSize) {
                c.push_back(a[aIndex]);
                bIndex++;
            }
            break;
        }
        if (a[aIndex] < b[bIndex]) {
            c.push_back(a[aIndex]);
            aIndex++;
        } else if (a[aIndex] == b[bIndex]) {
            c.push_back(a[aIndex]);
            aIndex++;
            bIndex++;
        } else {
            c.push_back(b[bIndex]);
            bIndex++;
        }
    }
    rowIndex_ = c;
    start_ = current_ = 0;
    end_ = rowIndex_.size();
}

bool IndexMap::Eof() const
{
    if (desc_) {
        return current_ <= start_ - 1;
    } else {
        return current_ >= end_;
    }
}

TableRowId IndexMap::CurrentRow() const
{
    auto current = current_;
    if (indexType_ == INDEX_TYPE_ID) {
        return current;
    } else {
        return rowIndex_[current];
    }
}

void IndexMap::SortBy(bool desc)
{
    if (desc) {
        current_ = end_ - 1;
    } else {
        current_ = start_;
    }
    desc_ = desc;
}
} // namespace TraceStreamer
} // namespace SysTuning
