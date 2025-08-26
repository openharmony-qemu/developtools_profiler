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

#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "hilog/log.h"

namespace OHOS {
namespace HiviewDFX {
namespace {
#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002D0A
#undef LOG_TAG
#define LOG_TAG "NapiUtil"
constexpr int ONE_VALUE_LIMIT = 1;
const std::string DEFAULT_FILENAME = "undefined";
}

napi_value CreateErrorMessage(napi_env env, const std::string& msg)
{
    napi_value result = nullptr;
    napi_value message = nullptr;
    napi_create_string_utf8(env, msg.data(), msg.size(), &message);
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

bool MatchValueType(napi_env env, napi_value value, napi_valuetype targetType)
{
    napi_valuetype valueType = napi_undefined;
    if (napi_typeof(env, value, &valueType) != napi_ok) {
        return false;
    }
    return valueType == targetType;
}

bool GetNapiArrayLength(napi_env env, napi_value value, uint32_t& ret)
{
    bool isArray = false;
    if (napi_is_array(env, value, &isArray) != napi_ok || !isArray) {
        return false;
    }
    return napi_get_array_length(env, value, &ret) == napi_ok;
}

bool GetNapiInt32Value(napi_env env, napi_value value, int32_t& ret)
{
    if (MatchValueType(env, value, napi_number)) {
        return napi_get_value_int32(env, value, &ret) == napi_ok;
    }
    return false;
}

bool GetNapiUint32Value(napi_env env, napi_value value, uint32_t& ret)
{
    if (MatchValueType(env, value, napi_number)) {
        return napi_get_value_uint32(env, value, &ret) == napi_ok;
    }
    return false;
}

bool GetNapiDoubleValue(napi_env env, napi_value value, double& ret)
{
    if (MatchValueType(env, value, napi_number)) {
        return napi_get_value_double(env, value, &ret) == napi_ok;
    }
    return false;
}

bool GetNapiBoolValue(napi_env env, napi_value value, bool& ret)
{
    if (MatchValueType(env, value, napi_boolean)) {
        return napi_get_value_bool(env, value, &ret) == napi_ok;
    }
    return false;
}

bool GetNapiStringValue(napi_env env, napi_value value, std::string& ret, size_t maxSize)
{
    if (!MatchValueType(env, value, napi_string)) {
        HILOG_ERROR(LOG_CORE, "Type error, should be string type!");
        return false;
    }
    size_t bufLen = 0;
    napi_status status = napi_get_value_string_utf8(env, value, nullptr, 0, &bufLen);
    if (status != napi_ok) {
        HILOG_ERROR(LOG_CORE, "Get input filename param length failed.");
        return false;
    }
    if (bufLen > maxSize || bufLen == 0) {
        HILOG_ERROR(LOG_CORE, "input filename param length is illegal.");
        return false;
    }
    ret = std::string(bufLen, '\0');
    return napi_get_value_string_utf8(env, value, &ret[0], bufLen + 1, &bufLen) == napi_ok;
}

bool GetNapiObjectProperty(napi_env env, napi_value value, const std::string& propertyName, napi_value& ret)
{
    bool hasProperty = false;
    if (napi_has_named_property(env, value, propertyName.c_str(), &hasProperty) != napi_ok || !hasProperty) {
        return false;
    }
    return napi_get_named_property(env, value, propertyName.c_str(), &ret) == napi_ok;
};

bool GetTheOnlyStringParam(napi_env env, napi_callback_info info, std::string &fileName)
{
    size_t argc = ONE_VALUE_LIMIT;
    napi_value argv = nullptr;
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_get_cb_info(env, info, &argc, &argv, &thisVar, &data);
    if (argc != ONE_VALUE_LIMIT) {
        HILOG_ERROR(LOG_CORE, "invalid number = %{public}d of params.", ONE_VALUE_LIMIT);
        return false;
    }
    constexpr int paramLen = 128;
    return GetNapiStringValue(env, argv, fileName, paramLen);
}

std::string GetFileNameParam(napi_env env, napi_callback_info info)
{
    std::string fileName;
    if (!GetTheOnlyStringParam(env, info, fileName)) {
        return DEFAULT_FILENAME;
    }
    return fileName;
}

bool AsyncTask::CreatePromise(napi_env env, napi_callback_info info, napi_value& promise)
{
    if (napi_create_promise(env, &deferred_, &promise) != napi_ok) {
        promise = nullptr;
        return false;
    }
    napi_value err = InitAsyncTask(env, info);
    if (err) {
        napi_reject_deferred(env, deferred_, err);
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

napi_value AsyncTask::InitAsyncTask(napi_env env, napi_callback_info info)
{
    return nullptr;
}

napi_deferred AsyncTask::GetDeferred()
{
    return deferred_;
}

void AsyncTask::ExecuteCallBack(napi_env env, void* data)
{
    reinterpret_cast<AsyncTask *>(data)->Work(env);
}

void AsyncTask::CompletedCallBack(napi_env env, napi_status status, void* data)
{
    auto asyncTaskPtr = reinterpret_cast<AsyncTask *>(data);
    napi_value msg = nullptr;
    if (asyncTaskPtr->Done(env, status, msg)) {
        napi_resolve_deferred(env, asyncTaskPtr->GetDeferred(), msg);
    } else {
        napi_reject_deferred(env, asyncTaskPtr->GetDeferred(), msg);
    }
    napi_delete_async_work(env, asyncTaskPtr->worker_);
    delete asyncTaskPtr;
}
} // namespace HiviewDFX
} // namespace OHOS