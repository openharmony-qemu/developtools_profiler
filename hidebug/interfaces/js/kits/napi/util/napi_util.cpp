/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "napi_util.h"
namespace OHOS {
namespace HiviewDFX {

napi_value CreateErrorMessage(napi_env env, const std::string& msg)
{
    napi_value result = nullptr;
    napi_value message = nullptr;
    napi_create_string_utf8(env, (char *)msg.data(), msg.size(), &message);
    napi_create_error(env, nullptr, message, &result);
    return result;
}

napi_value CreateErrorMessage(napi_env env, const std::string& errCode, const std::string& msg)
{
    napi_value result = nullptr;
    napi_value message = nullptr;
    napi_value code = nullptr;
    napi_create_string_utf8(env, errCode.data(), errCode.size(), &code);
    napi_create_string_utf8(env, msg.data(), msg.size(), &message);
    napi_create_error(env, code, message, &result);
    return result;
}

napi_value CreateUndefined(napi_env env)
{
    napi_value res = nullptr;
    napi_get_undefined(env, &res);
    return res;
}

bool AsyncTask::CreatePromise(napi_env env, napi_value &promise)
{
    if (napi_create_promise(env, &deferred_, &promise) != napi_ok) {
        return false;
    }
    napi_value resourceName;
    if (napi_create_string_utf8(env, resourceName_.c_str(), resourceName_.size(), &resourceName) != napi_ok) {
        return false;
    };
    if (napi_create_async_work(env, nullptr, resourceName, ExecuteCallBack, CompletedCallBack,
                               static_cast<void *>(this), &worker_) != napi_ok) {
        return false;
    }
    return napi_queue_async_work(env, worker_) == napi_ok;
}

void AsyncTask::ExecuteCallBack(napi_env env, void* data)
{
    auto asyncTaskPtr = reinterpret_cast<AsyncTask *>(data);
    asyncTaskPtr->Work(env);
}

void AsyncTask::CompletedCallBack(napi_env env, napi_status status, void* data)
{
    auto asyncTaskPtr = reinterpret_cast<AsyncTask *>(data);
    asyncTaskPtr->Done(env, status);
    napi_delete_async_work(env, asyncTaskPtr->worker_);
    delete asyncTaskPtr;
}
} // namespace HiviewDFX
} // namespace OHOS