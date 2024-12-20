/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef OHOS_SP_LOG_H
#define OHOS_SP_LOG_H

namespace OHOS {
namespace SmartPerf {
typedef enum {
    SP_LOG_DEBUG,
    SP_LOG_INFO,
    SP_LOG_WARN,
    SP_LOG_ERROR,
} SpLogLevel;

void SpLog(SpLogLevel logLevel, const char *fmt, ...);

#define LOGD(fmt, ...) \
    SpLog(SP_LOG_DEBUG, (std::string("[") + "SP_daemon" + "][" + __FUNCTION__ + "]:" + fmt).c_str(), ##__VA_ARGS__)

#define LOGI(fmt, ...) \
    SpLog(SP_LOG_INFO, (std::string("[") + "SP_daemon" + "][" + __FUNCTION__ + "]:" + fmt).c_str(), ##__VA_ARGS__)

#define LOGW(fmt, ...) \
    SpLog(SP_LOG_WARN, (std::string("[") + "SP_daemon" + "][" + __FUNCTION__ + "]:" + fmt).c_str(), ##__VA_ARGS__)

#define LOGE(fmt, ...) \
    SpLog(SP_LOG_ERROR, (std::string("[") + "SP_daemon" + "][" + __FUNCTION__ + "]:" + fmt).c_str(), ##__VA_ARGS__)
} // namespace SmartPerf
} // namespace OHOS
#endif // OHOS_SP_LOG_H