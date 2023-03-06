/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <fcntl.h>
#include <securec.h>
#include <sys/prctl.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <pwd.h>
#include <string>
#include <policycoreutils.h>
#include <selinux/selinux.h>

#include "bundle_mgr_interface.h"
#include "bundle_mgr_proxy.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "system_ability_manager_proxy.h"
#include "hilog/log.h"

using namespace std;

constexpr OHOS::HiviewDFX::HiLogLabel TRANS_LOG_LABLE = { LOG_CORE, 0xD002D0C, "TRANSITTO" };
constexpr int BUFFER_SIZE = 1024;

struct AppInfo {
    int uid;
    bool debug;
    char codePath[BUFFER_SIZE];
};

bool GetProcessPid(const std::string& processName, int& pid)
{
    DIR* dir = opendir("/proc");
    if (dir == nullptr) {
        return false;
    }
    struct dirent* ptr;
    int invalidPid = -1;
    int pidValue = invalidPid;
    while ((ptr = readdir(dir)) != nullptr) {
        if ((strcmp(ptr->d_name, ".") == 0) || (strcmp(ptr->d_name, "..") == 0)) {
            continue;
        }
        if ((!isdigit(*ptr->d_name)) || ptr->d_type != DT_DIR) {
            continue;
        }
        char filePath[BUFFER_SIZE] = {0};
        int len = snprintf_s(filePath, BUFFER_SIZE, BUFFER_SIZE - 1, "/proc/%s/cmdline", ptr->d_name);
        if (len < 0) {
            continue;
        }
        FILE* fp = fopen(filePath, "r");
        if (fp == nullptr) {
            continue;
        }

        char buf[BUFFER_SIZE] = {0};
        if (fgets(buf, sizeof(buf) - 1, fp) == nullptr) {
            fclose(fp);
            continue;
        }
        fclose(fp);
        std::string str(buf);
        size_t found = str.rfind("/");
        std::string fullProcess;
        if (found != std::string::npos) {
            fullProcess = str.substr(found + 1);
        } else {
            fullProcess = str;
        }
        if (fullProcess == processName) {
            pidValue = atoi(ptr->d_name);
            break;
        }
    }
    closedir(dir);
    if (pidValue != invalidPid) {
        pid = pidValue;
    }
    return pidValue != invalidPid;
}

std::string ReadFileToString(const std::string& fileName)
{
    std::ifstream inputString(fileName, std::ios::in);
    if (!inputString || !inputString.is_open()) {
        return "";
    }

    std::istreambuf_iterator<char> firstIt = {inputString};
    std::istreambuf_iterator<char> lastIt = {};

    std::string content(firstIt, lastIt);
    return content;
}

bool GetApplicationInfo(const string& bundleName, OHOS::AppExecFwk::ApplicationInfo& appInfo)
{
    OHOS::sptr<OHOS::ISystemAbilityManager> systemAbilityManager =
        OHOS::SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (systemAbilityManager == nullptr) {
        OHOS::HiviewDFX::HiLog::Error(TRANS_LOG_LABLE, "fail to get system abilityManger.");
        return false;
    }

    OHOS::sptr<OHOS::IRemoteObject> remoteObject =
        systemAbilityManager->GetSystemAbility(OHOS::BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
    if (remoteObject == nullptr) {
        OHOS::HiviewDFX::HiLog::Error(TRANS_LOG_LABLE, "fail to get bundle service.");
        return false;
    }

    OHOS::sptr<OHOS::AppExecFwk::BundleMgrProxy> bundleMgrProxy =
        OHOS::iface_cast<OHOS::AppExecFwk::BundleMgrProxy>(remoteObject);
    if (bundleMgrProxy == nullptr) {
        OHOS::HiviewDFX::HiLog::Error(TRANS_LOG_LABLE, "fail to get bundle proxy.");
        return false;
    }

    OHOS::HiviewDFX::HiLog::Info(TRANS_LOG_LABLE, "start to get ApplicationInfo");
    // 0: GET_BASIC_APPLICATION_INFO
    if (!bundleMgrProxy->GetApplicationInfo(bundleName, 0, OHOS::AppExecFwk::Constants::ANY_USERID, appInfo)) {
        OHOS::HiviewDFX::HiLog::Error(TRANS_LOG_LABLE, "fail to get application info.");
        return false;
    }
    OHOS::HiviewDFX::HiLog::Info(TRANS_LOG_LABLE, "get ApplicationInfo success uid is %{private}d.", appInfo.uid);
    return true;
}

bool ChangeUidGid(int uid, int gid)
{
    OHOS::HiviewDFX::HiLog::Info(TRANS_LOG_LABLE, "start change uid gid.");
    if (setresgid(gid, gid, gid) < 0) {
        OHOS::HiviewDFX::HiLog::Error(TRANS_LOG_LABLE, "fail set gid, errno is %{publci}d.", errno);
        return false;
    }

    if (setresuid(uid, uid, uid) < 0) {
        OHOS::HiviewDFX::HiLog::Error(TRANS_LOG_LABLE, "fail set uid, errno is %{publci}d.", errno);
        return false;
    }
    return true;
}

void InitEnv(const string& codePath, int uid)
{
    OHOS::HiviewDFX::HiLog::Info(TRANS_LOG_LABLE, "start set env, app baseDir is %{public}s.", codePath.c_str());
    if (chdir(codePath.c_str()) == -1) {
        OHOS::HiviewDFX::HiLog::Error(TRANS_LOG_LABLE,
            "fail to chdir, path is %{public}s, errno %{public}d.", codePath.c_str(), errno);
        return;
    }

    setenv("HOME", codePath.c_str(), 1);
    unsetenv("IFS");

    passwd* pw = getpwuid(uid);
    if (pw != nullptr) {
        setenv("LOGNAME", pw->pw_name, 1);
        setenv("SHELL", pw->pw_shell, 1);
        setenv("USER", pw->pw_name, 1);
    } else {
        OHOS::HiviewDFX::HiLog::Error(TRANS_LOG_LABLE, "fail to getpwuid, errno is %{public}d.", errno);
    }
    return;
}

bool SetSelinux()
{
    OHOS::HiviewDFX::HiLog::Info(TRANS_LOG_LABLE, "start change selinux context.");
    string seContext = "u:r:transitto_hap:s0";

    if (setcon(seContext.c_str()) != 0) {
        OHOS::HiviewDFX::HiLog::Error(TRANS_LOG_LABLE, "fail to set selinux context, errno is %{public}d.", errno);
        return false;
    }
    OHOS::HiviewDFX::HiLog::Info(TRANS_LOG_LABLE, "change selinux context successfully.");
    return true;
}

void Help()
{
    cout << "\ntransitto is a debuggable tool. your command can transit to the domain of debuggable bundle.\n"
            "usage:\n"
            "transitto <debuggable bundleName> <command>\n" << endl;
}

bool CheckValid(int argc, char** argv)
{
    if (argc <= 1) {
        cout << "argc is empty" << endl;
        Help();
        return false;
    }

    string secCom = argv[1];
    if (argc == 2 && (secCom == "-h" || secCom == "--help")) {
        Help();
        return false;
    }
 
    int oldUid = getuid();
    // 0, root, 2000 shell
    if (oldUid != 0 && oldUid != 2000) {
        cout << "only root or shell can run this object" << endl;
        Help();
        return false;
    }
    return true;
}

int main(int argc, char* argv[])
{
    if (!CheckValid(argc, argv)) {
        return -1;
    }

    string bundleName = argv[1];
    AppInfo* app = static_cast<AppInfo*>(mmap(nullptr, sizeof(AppInfo),
        PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0));
    if (app == MAP_FAILED || app == nullptr) {
        OHOS::HiviewDFX::HiLog::Error(TRANS_LOG_LABLE, "mmap fail.");
        return -1;
    }
    memset_s(app, sizeof(AppInfo), 0, sizeof(AppInfo));

    int pid = fork(); // for security_bounded_transition single thread
    if (pid == 0) {
        OHOS::AppExecFwk::ApplicationInfo appInfo {};
        if (GetApplicationInfo(bundleName, appInfo)) {
            app->uid = appInfo.uid;
            app->debug = appInfo.debug;
            int ret = strcpy_s(app->codePath, BUFFER_SIZE, appInfo.codePath.c_str());
            if (ret != EOK) {
                OHOS::HiviewDFX::HiLog::Error(TRANS_LOG_LABLE, "copy appinfo path fail, ret is %{public}d.", ret);
            }
        }
        _exit(0);
    } else {
        wait(nullptr);
    }

    int uid = app->uid;
    bool debug = app->debug;
    string codePath = app->codePath;
    munmap(app, sizeof(AppInfo));

    string commod = (argc > 2) ? argv[2] : ""; // 2 com
    // normal_hap uid = 200000 * usrid + bundleid % 200000, userid is 100 or 0(shared)
    if (uid <= 20000000 || !debug || !ChangeUidGid(uid, uid) || !SetSelinux()) {
        OHOS::HiviewDFX::HiLog::Error(TRANS_LOG_LABLE, "uid is %{public}d.", uid);
        return -1;
    }

    InitEnv(codePath, uid);
    if (argc > 2 && execvp(argv[2], argv + 2) < 0) { // 2: offset
        OHOS::HiviewDFX::HiLog::Error(TRANS_LOG_LABLE,
            "fail to execvp, com is %{public}s, errno %{public}d.", argv[2], errno); // 2: offset
        return -1;
    }

    execlp("/system/bin/sh", "sh", nullptr);
    return 0;
}