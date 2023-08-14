/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021. All rights reserved.
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
#ifndef PLUGIN_SERVICE_STUB_H
#define PLUGIN_SERVICE_STUB_H

#include "plugin_service.h"

#ifdef USE_PLUGIN_SERVICE_STUB
class PluginServiceStub {
public:
    static std::shared_ptr<PluginServiceStub> GetInstance();

    void SetCreateResult(bool value);
    bool GetCreateResult() const;

    void SetStartResult(bool value);
    bool GetStartResult() const;

    void SetStopResult(bool value);
    bool GetStopResult() const;

    void SetDestroyResult(bool value);
    bool GetDestroyResult() const;

    void SetAddResult(bool value);
    bool GetAddResult();

    void SetRemoveResult(bool value);
    bool GetRemoveResult();

    PluginServiceStub() = default;
    ~PluginServiceStub() = default;

private:
    bool createResult_ = true;
    bool destroyResult_ = true;
    bool startResult_ = true;
    bool stopResult_ = true;
    bool addResult_ = true;
    bool removeResult_ = true;
    TraceFileWriterPtr traceWriter_ = nullptr;
};
#endif

#endif // !PLUGIN_SERVICE_STUB_H