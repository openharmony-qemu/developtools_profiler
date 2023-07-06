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

#ifndef INCLUDE_TUNING_BASE_CODEC_COV_H_
#define INCLUDE_TUNING_BASE_CODEC_COV_H_

#include <string>

namespace SysTuning {
namespace base {
int32_t PreNum(unsigned char byte);

bool IsUTF8(const uint8_t* data, int32_t len);

bool IsGBK(const uint8_t* data, int32_t len);

typedef enum { GBK, UTF8, UNKOWN } CODING;

CODING GetCoding(const uint8_t* data, int32_t len);

#ifdef _WIN32
std::string GbkToUtf8(const char* srcStr);
#endif
} // namespace base
} // namespace SysTuning

#endif
