/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022. All rights reserved.
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

#ifndef TYPE_HEADERS_H
#define TYPE_HEADERS_H

#include <sys/types.h>
#include <asm/types.h>
#include <inttypes.h>
#include <stdint.h>
#include <linux/types.h>
#include "linux/types-extra.h" // linux/types-extra.h is the complement of linux/tyeps.h,
                               // and it must be included after linux/types.h

#include "hiebpf_types.h"      // this header must be included after the above headers

#endif