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

#ifndef HIDEBUG_ASYNCTASK_H_
#define HIDEBUG_ASYNCTASK_H_

#include "napi/native_api.h"

#include <cstdint>
#include <functional>
#include <string>
#include <type_traits>

namespace OHOS {
namespace HiviewDFX {

napi_value CreateErrorMessage(napi_env env, const std::string& msg);

napi_value CreateErrorMessage(napi_env env, const std::string& errCode, const std::string& msg);

napi_value CreateUndefined(napi_env env);

bool MatchValueType(napi_env env, napi_value value, napi_valuetype targetType);

bool GetNapiArrayLength(napi_env env, napi_value value, uint32_t& ret);

bool GetNapiInt32Value(napi_env env, napi_value value, int32_t& ret);

bool GetNapiUint32Value(napi_env env, napi_value value, uint32_t& ret);

bool GetNapiDoubleValue(napi_env env, napi_value value, double& ret);

bool GetNapiBoolValue(napi_env env, napi_value value, bool& ret);

bool GetNapiStringValue(napi_env env, napi_value value, std::string& ret, size_t maxSize);

bool GetNapiObjectProperty(napi_env env, napi_value value, const std::string& propertyName, napi_value& ret);

bool GetTheOnlyStringParam(napi_env env, napi_callback_info info, std::string &fileName);

std::string GetFileNameParam(napi_env env, napi_callback_info info);
class AsyncTask {
public:
    explicit AsyncTask(const std::string& resourceName): resourceName_(resourceName) {};
    virtual ~AsyncTask() = default;

    template<typename T, typename = typename std::enable_if<std::is_base_of<AsyncTask, T>::value>::type>
    static napi_value GetPromise(napi_env env, std::function<void(T*)> setReqParam = [](T*) {})
    {
        napi_value promise = nullptr;
        T* asyncTask = new (std::nothrow) T();
        if (asyncTask == nullptr) {
            return nullptr;
        }
        setReqParam(asyncTask);
        if (!asyncTask->CreatePromise(env, promise)) {
            delete asyncTask;
            return nullptr;
        }
        return promise;
    };

protected:
    napi_async_work worker_ = nullptr;
    napi_deferred deferred_ = nullptr;
    std::string resourceName_;
    virtual void Work(napi_env env) = 0;
    virtual void Done(napi_env env, napi_status status) = 0;

private:
    bool CreatePromise(napi_env env, napi_value& promise);
    static void ExecuteCallBack(napi_env env, void* data);
    static void CompletedCallBack(napi_env env, napi_status status, void* data);
};
}
}

#endif //HIDEBUG_ASYNCTASK_H_
