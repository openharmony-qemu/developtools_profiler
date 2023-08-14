/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2022. All rights reserved.
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

#ifndef HIEBPF_UTILITIES_H_
#define HIEBPF_UTILITIES_H_

#include <string>
#include <vector>

namespace OHOS {
namespace Developtools {
namespace Hiebpf {
constexpr const int NUMBER_FORMAT_HEX_BASE = 16;
const std::string EMPTY_STRING = "";

#ifndef MMAP_FAILED
#define MMAP_FAILED (reinterpret_cast<void *>(-1))
#endif
#ifndef MAP_FAILED
#define MAP_FAILED MMAP_FAILED
#endif

std::string ReadFileToString(const std::string &fileName);
std::vector<std::string> StringSplit(std::string source, const std::string &split);
bool StringEndsWith(const std::string &string, const std::string &with);
} // namespace Hiebpf
} // namespace Developtools
} // namespace OHOS

#endif // HIEBPF_UTILITIES_H_
