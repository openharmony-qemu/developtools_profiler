/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
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

#include "hidebug_base.h"

#include <dlfcn.h>
#include <errno.h>
#include <inttypes.h>
#include <limits.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "securec.h"

#ifdef HIDEBUG_IN_INIT
#include "init_module_engine.h"
#include "init_log.h"
#define HIDEBUG_LOGE(...) INIT_LOGE(__VA_ARGS__)
#define HIDEBUG_LOGI(...) INIT_LOGI(__VA_ARGS__)
#define LOG_PRIV_PUBLIC ""
#else
#include <parameter.h>
#include <sysparam_errno.h>
#include <hilog/log.h>
#include "hichecker_wrapper.h"

#undef LOG_DOMAIN
#undef LOG_TAG
#define LOG_DOMAIN 0xD002D0A
#define LOG_TAG "HiDebug_Native"

#define HIDEBUG_LOGE(...) HILOG_ERROR(LOG_CORE, __VA_ARGS__)
#define HIDEBUG_LOGI(...) HILOG_INFO(LOG_CORE, __VA_ARGS__)
#define LOG_PRIV_PUBLIC "{public}"
#endif

#define PARAM_BUF_LEN 128
#define SLASH_CHR '/'
const char * const LIBC_HOOK_PARAM = "libc.hook_mode";

#ifdef HIDEBUG_BUILD_VARIANT_ROOT
#define MAX_PARA_LEN 50
#define MAX_PARA_CNT 20
#define QUERYNAME_LEN 80
#define COLON_CHR ':'

struct Params {
    char key[MAX_PARA_LEN];
    char value[MAX_PARA_LEN];
};

static void ParseKeyValue(const char *input, uint32_t *paramCnt, struct Params *result, const uint32_t paramSize)
{
    if (*paramCnt >= paramSize) {
        HIDEBUG_LOGE("Parameters is Full.");
        return;
    }
    const char *colonPos = strchr(input, COLON_CHR);
    if (colonPos == NULL) {
        HIDEBUG_LOGE("params is illegal.");
        return;
    }
    errno_t err = strncpy_s(result[*paramCnt].key, MAX_PARA_LEN, input, colonPos - input);
    if (err != EOK) {
        HIDEBUG_LOGE("strncpy_s copy key strings failed.");
        return;
    }
    err = strncpy_s(result[*paramCnt].value, MAX_PARA_LEN, colonPos + 1, strlen(colonPos + 1));
    if (err != EOK) {
        HIDEBUG_LOGE("strncpy_s copy value strings failed.");
        return;
    }
    (*paramCnt)++;
}

static uint32_t SplitParams(char *input, struct Params *result, const uint32_t paramSize)
{
    uint32_t paramCnt = 0;
    const char space[] = " ";
    char *param;
    char *next = NULL;
    param = strtok_s(input, space, &next);
    while (param != NULL) {
        ParseKeyValue(param, &paramCnt, result, paramSize);
        param = strtok_s(NULL, space, &next);
    }
    return paramCnt;
}

static bool QueryParams(const char *queryName, char *result, const uint32_t size)
{
    if (result == NULL || size > PARAM_BUF_LEN) {
        return false;
    }
#ifdef HIDEBUG_IN_INIT
    uint32_t bufferSize = size;
    int retLen = SystemReadParam(queryName, result, &bufferSize);
    if (retLen != 0 || bufferSize == 0 || bufferSize > PARAM_BUF_LEN - 1) {
        return false;
    }
    result[bufferSize] = '\0';
#else
    int retLen = GetParameter(queryName, NULL, result, size);
    if (retLen <= 0 || retLen > PARAM_BUF_LEN - 1) {
        return false;
    }
    result[retLen] = '\0';
#endif
    return true;
}

static bool ConcatenateParamName(char* queryName, const size_t size, const char* serviceName)
{
    if (strcat_s(queryName, size, serviceName) != EOK) {
        HIDEBUG_LOGE("strcat_s query name failed.");
        return false;
    }
    return true;
}

static bool InitHiDebugEnvParams(const char *queryName)
{
    char paramOutBuf[PARAM_BUF_LEN] = { 0 };
    if (!QueryParams(queryName, paramOutBuf, PARAM_BUF_LEN)) {
        return false;
    }
    struct Params params[MAX_PARA_CNT];
    uint32_t paramsCnt = SplitParams(paramOutBuf, params, MAX_PARA_CNT);
    for (uint32_t i = 0; i < paramsCnt; ++i) {
        if (setenv(params[i].key, params[i].value, 1) != 0) { // 1 : overwrite
            HIDEBUG_LOGE("setenv failed, err: %" LOG_PRIV_PUBLIC "s", strerror(errno));
        }
    }
    return paramsCnt > 0;
}

static bool InitEnvParam(const char *serviceName)
{
    char paramOutBuf[PARAM_BUF_LEN] = { 0 };
    if (!QueryParams("const.debuggable", paramOutBuf, PARAM_BUF_LEN) || strcmp(paramOutBuf, "1") != 0) {
        return false;
    }
    char onceName[QUERYNAME_LEN] = "hiviewdfx.debugenv.";
    if (!ConcatenateParamName(onceName, sizeof(onceName), serviceName)) {
        return false;
    }
    if (InitHiDebugEnvParams(onceName)) {
        return true;
    }
    char persistName[QUERYNAME_LEN] = "persist.hiviewdfx.debugenv.";
    return ConcatenateParamName(persistName, sizeof(persistName), serviceName) && InitHiDebugEnvParams(persistName);
}
#endif

static const char* FilterServiceName(const char *inputName)
{
    const char *ret = strrchr(inputName, SLASH_CHR);
    if (ret == NULL) {
        return inputName;
    }
    return ret + 1;
}

static int GetMallocHookStartupValue(const char *param, char *path, int size)
{
    if (path == NULL || size <= 0) {
        return -1;
    }

    const char *ptr = param;

    while (*ptr && *ptr != ':') {
        ++ptr;
    }
    if (*ptr == ':') {
        ++ptr;
    }
    const int paramLength = 7;
    if (strncmp(param, "startup", paramLength) == 0) {
        if (*ptr == '\"') {
            ++ptr;
            int idx = 0;
            while (idx < size - 1 && *ptr && *ptr != '\"') {
                path[idx++] = *ptr++;
            }
            path[idx] = '\0';
        } else {
            int idx = 0;
            while (idx < size - 1 && *ptr) {
                path[idx++] = *ptr++;
            }
            path[idx] = '\0';
        }
    }
    return 0;
}

static bool MatchMallocHookStartupProp(const char *thisName)
{
    char paramOutBuf[PARAM_BUF_LEN] = { 0 };
    char targetProcName[PARAM_BUF_LEN] = { 0 };

#ifdef HIDEBUG_IN_INIT
    uint32_t size = PARAM_BUF_LEN;
    int retLen = SystemReadParam(LIBC_HOOK_PARAM, paramOutBuf, &size);
    if (retLen != 0 || size <= 0) {
        return 0;
    }
    retLen = (int)size;
#else
    char defStrValue[PARAM_BUF_LEN] = { 0 };
    int retLen = GetParameter(LIBC_HOOK_PARAM, defStrValue, paramOutBuf, PARAM_BUF_LEN);
    if (retLen == 0) {
        return 0;
    }
#endif
    const int paramLength = 8;
    if (strncmp(paramOutBuf, "startup:", paramLength) != 0) {
        return false;
    }
    retLen = GetMallocHookStartupValue(paramOutBuf, targetProcName, PARAM_BUF_LEN);
    if (retLen == -1) {
        HIDEBUG_LOGE("malloc hook parse startup value failed");
        return false;
    }

    const int targetProcNameSize = strlen(targetProcName);
    if (strncmp(targetProcName, "init", targetProcNameSize) == 0) {
        HIDEBUG_LOGI("malloc hook: this target proc '%" LOG_PRIV_PUBLIC "s' no hook", targetProcName);
        return false;
    }

    if (strcmp(thisName, targetProcName) == 0) {
        return true;
    }
    return false;
}

static int SetupMallocHookAtStartup(const char *thisName)
{
    const int hookSignal = 36;
    if (!MatchMallocHookStartupProp(thisName)) {
        return 0;
    }
    HIDEBUG_LOGI("malloc send hook signal.");
    return raise(hookSignal);
}

bool InitEnvironmentParam(const char *inputName)
{
    if (inputName == NULL) {
        HIDEBUG_LOGE("input service name is null.");
        return false;
    }
    const char *serviceName = FilterServiceName(inputName);
    if (*serviceName == '\0') {
        HIDEBUG_LOGE("input service name is illegal.");
        return false;
    }
#ifndef HIDEBUG_IN_INIT
    InitHicheckerParamWrapper(serviceName);
#endif

#ifdef HAS_MUSL_STARTUP_MALLOC_HOOK_INTF
    setup_malloc_hook_mode();
#else
    SetupMallocHookAtStartup(serviceName);
#endif

#ifdef HIDEBUG_BUILD_VARIANT_ROOT
    return InitEnvParam(serviceName);
#else
    return true;
#endif
}
