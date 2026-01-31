/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2024. All rights reserved.
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

#ifndef HOOK_CLIENT_HOLDER_H
#define HOOK_CLIENT_HOLDER_H

#include <memory>
#include <sys/mman.h>
#include <sys/types.h>
#include <mutex>
#include <atomic>
#include "hook_common.h"

class HookSocketClient;

class HookClientHolder {
public:
    explicit HookClientHolder(std::shared_ptr<HookSocketClient>& sharedClient);
    HookSocketClient* Get();
    bool IsValid() const;
    bool SendStackWithPayload(const void* data, size_t size, const void* payload,
            size_t payloadSize, int smbIndex = 0);
    bool UpdateThreadName();
    void SendMmapFileRawData(int prot, int flags, off_t offset, const char* filePath, const StackRawData& rawdata);

private:
    std::weak_ptr<HookSocketClient> holder_;
};

#endif  // HOOK_CLIENT_HOLDER_H
