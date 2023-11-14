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

#ifndef TABLE_FILTER_CONSTRAINTS_H
#define TABLE_FILTER_CONSTRAINTS_H

#include <string>
#include <vector>
#include "sqlite3.h"

namespace SysTuning {
namespace TraceStreamer {
class FilterConstraints {
public:
    struct Constraint {
        int32_t idxInaConstraint; // index in sqlite3_index_info.aConstraint[]
        int32_t col;              // Column this constraint refers to
        unsigned char op;         // SQLite op for the constraint
        bool isSupport = false;
    };
    using OrderBy = sqlite3_index_info::sqlite3_index_orderby;

    FilterConstraints() {}
    ~FilterConstraints() {}
    void AddConstraint(int32_t idx, int32_t col, unsigned char op, bool isSupport = false);
    void UpdateConstraint(int32_t idx, bool isSupport);
    void AddOrderBy(int32_t col, unsigned char desc);
    void Clear();

    const std::vector<OrderBy>& GetOrderBys() const
    {
        return orderBys_;
    }

    const std::vector<Constraint>& GetConstraints() const
    {
        return constraints_;
    }

    // idxStr format: C<N> col1 op1 ... colN opN O<M> col1 desc1 ... colM descM
    // like as "C2 0 2 1 4 O1 0 1"
    void ToString(std::string& idxStr) const;
    void FromString(const std::string& idxStr);

private:
    void GetColAndOp(const char** p, char** pNext, int32_t& col, unsigned char& op);

private:
    std::vector<Constraint> constraints_;
    std::vector<OrderBy> orderBys_;
    const std::size_t idxStrSize_ = 512;
};
} // namespace TraceStreamer
} // namespace SysTuning

#endif // TABLE_FILTER_CONSTRAINTS_H
