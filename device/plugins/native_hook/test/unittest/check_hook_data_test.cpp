/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021. All rights reserved.
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

#include <dlfcn.h>
#include <fcntl.h>
#include <gtest/gtest.h>
#include <vector>
#include <sys/syscall.h>
#include <sys/mman.h>
#include "token_setproc.h"
#include "accesstoken_kit.h"
#include "buffer_splitter.h"
#include "common.h"
#include "test_common.h"
#include "logging.h"
#include "parameters.h"
#include <fstream>
#include <iostream>
#include <thread>

#pragma clang optimize off

using namespace testing::ext;
using namespace OHOS::Security::AccessToken;

namespace {
constexpr int DEFAULT_MALLOC_SIZE = 10;
constexpr int DEFAULT_CALLOC_SIZE = 100;
constexpr int DEFAULT_REALLOC_SIZE = 1000;
constexpr int DATA_SIZE = 50;
constexpr int SLEEP_TIME = 10;
constexpr int WAIT_FLUSH = 15;
constexpr int HOOK_TIME = 10;
constexpr int SLEEP_FIVE = 5;

const std::string DEFAULT_NATIVE_DAEMON_CLIENT_PATH("/data/local/tmp/native_daemon_client");
constexpr int SHARE_MEMORY_SIZE = 1000 * 4096;
constexpr int BUFFER_SIZE = 100 * 1024;
#ifdef __aarch64__
constexpr int DEFAULT_DEPTH = 32;
constexpr int CALLOC_DEPTH = 13;
constexpr int REALLOC_DEPTH = 10;
constexpr int MALLOC_VEC_SIZE = 5;
constexpr int FREE_VEC_SIZE = 4;
constexpr int MALLOC_GET_DATE_SIZE = 3;
constexpr int FREE_GET_DATA_SIZE = 2;
#endif
[[maybe_unused]] constexpr int WAIT_TIME = 5;
constexpr int START_JS_REPORT = 1;
std::unique_ptr<uint8_t[]> g_buffer = std::make_unique<uint8_t[]>(BUFFER_SIZE);
const std::string DEFAULT_PATH("/data/local/tmp/");
const std::string TEST_PROC_NAME = "hiview";
static AccessTokenID g_selfTokenId;
static TEST_COMMON::MockNativeToken* g_mock = nullptr;
#ifdef __aarch64__
const std::string DEFAULT_LIBA_PATH("/system/lib64/liba.z.so");
const std::string DEFAULT_LIBB_PATH("/system/lib64/libb.z.so");
const std::string DEFAULT_LIBNATIVETEST_PATH("/data/local/tmp/libnativetest_so.z.so");
const int LIBA_MALLOC_SIZE = 888;
const int LIBB_MALLOC_SIZE = 666;
#endif

typedef char* (*DepthMallocSo)(int depth, int mallocSize);
typedef void (*DepthFreeSo)(int depth, char *p);

using StaticSpace = struct {
    int data[DATA_SIZE];
};

class CheckHookDataTest : public ::testing::Test {
public:
    static void SetUpTestCase()
    {
        g_selfTokenId = GetSelfTokenID();
        TEST_COMMON::SetTestEvironment(g_selfTokenId);
        g_mock = new (std::nothrow) TEST_COMMON::MockNativeToken(TEST_PROC_NAME);
    }

    static void TearDownTestCase()
    {
        if (g_mock != nullptr) {
            delete g_mock;
            g_mock = nullptr;
        }
        SetSelfTokenID(g_selfTokenId);
        TEST_COMMON::ResetTestEvironment();
    }
    void StartDaemonProcessArgs()
    {
        outFile_ = DEFAULT_PATH + "hooktest_"+ outFileType_ + mode_[modeIndex_] + ".txt";
        if (signal(SIGCHLD, SIG_IGN) == SIG_ERR) {
            return;
        }
        int processNum = fork();
        if (processNum == 0) {
            int waitProcMills = 300;
            OHOS::system::SetParameter("hiviewdfx.hiprofiler.memprofiler.start", "0");
            std::this_thread::sleep_for(std::chrono::milliseconds(waitProcMills));
            OHOS::system::SetParameter("hiviewdfx.hiprofiler.memprofiler.start", "1");
            std::this_thread::sleep_for(std::chrono::milliseconds(waitProcMills));
            command_.push_back(const_cast<char*>(DEFAULT_NATIVE_DAEMON_CLIENT_PATH.c_str()));
            command_.push_back(const_cast<char*>("-s"));
            command_.push_back(const_cast<char*>("-fn"));
            command_.push_back(const_cast<char*>(outFile_.c_str()));
            command_.push_back(const_cast<char*>("-sms"));
            command_.push_back(const_cast<char*>(std::to_string(SHARE_MEMORY_SIZE).c_str()));
            if (!extendPid_) {
                command_.push_back(const_cast<char*>("-p"));
                command_.push_back(const_cast<char*>(std::to_string(hookPid_).c_str()));
            } else {
                command_.push_back(const_cast<char*>("-pe"));
                command_.push_back(const_cast<char*>(std::to_string(hookPid_).c_str()));
            }
            command_.push_back(const_cast<char*>("-d"));
            command_.push_back(const_cast<char*>(std::to_string(HOOK_TIME).c_str()));
            if (modeIndex_ == 0) {
                command_.push_back(const_cast<char*>("-df"));
            }
            if (unwindDepth_ > 0) {
                command_.push_back(const_cast<char*>("-msd"));
                command_.push_back(const_cast<char*>(std::to_string(unwindDepth_).c_str()));
            }
            if (statisticsInterval_ > 0) {
                command_.push_back(const_cast<char*>("-si"));
                command_.push_back(const_cast<char*>(std::to_string(statisticsInterval_).c_str()));
            }
            if (sampleInterval_ > 0) {
                command_.push_back(const_cast<char*>("-spi"));
                command_.push_back(const_cast<char*>(std::to_string(sampleInterval_).c_str()));
            }
            if (!offlineSymbolization_) {
                command_.push_back(const_cast<char*>("-os"));
            }
            if (callframeCompress_) {
                command_.push_back(const_cast<char*>("-cc"));
            }
            if (!stringCompress_) {
                command_.push_back(const_cast<char*>("-sc"));
            }
            if (responseLibraryMode_) {
                command_.push_back(const_cast<char*>("-r"));
            }
            if (saveFile_) {
                command_.push_back(const_cast<char*>("-sf"));
            }
            if (isHookStandalone_) {
                command_.push_back(const_cast<char*>("-hsa"));
            }
            if (mallocFreeMatchingInterval_ > 0) {
                command_.push_back(const_cast<char*>("-mfmi"));
                command_.push_back(const_cast<char*>(std::to_string(mallocFreeMatchingInterval_).c_str()));
            }
            if (dumpdata_) {
                command_.push_back(const_cast<char*>("-dd"));
            }
            if (nmd_) {
                command_.push_back(const_cast<char*>("-nmd"));
            }
            if (jsReport_) {
                command_.push_back(const_cast<char*>("-jr"));
                command_.push_back(const_cast<char*>(std::to_string(START_JS_REPORT).c_str()));
                command_.push_back(const_cast<char*>("-mjsd"));
                command_.push_back(const_cast<char*>(std::to_string(jsMaxDepth_).c_str()));
            }
            command_.push_back(nullptr);
            execv(DEFAULT_NATIVE_DAEMON_CLIENT_PATH.c_str(), command_.data());
            _exit(1);
        } else {
            daemonPid_ = processNum;
        }
    }

    std::vector<std::string>& StringSplit(const std::string& str, char delim = ';')
    {
        std::stringstream ss(str);
        std::string item;
        static std::vector<std::string> elems;
        elems.clear();
        while (std::getline(ss, item, delim)) {
            if (!item.empty()) {
                elems.push_back(item);
            }
        }
        return elems;
    }

    void StopProcess(int processNum)
    {
        std::string stopCmd = "kill -9 " + std::to_string(processNum);
        system(stopCmd.c_str());
    }

    int32_t ReadFile(std::string file)
    {
        int fd = -1;
        ssize_t bytesRead = 0;
        char filePath[PATH_MAX + 1] = {0};

        if (snprintf_s(filePath, sizeof(filePath), sizeof(filePath) - 1, "%s", file.c_str()) < 0) {
            const int bufSize = 256;
            char buf[bufSize] = { 0 };
            strerror_r(errno, buf, bufSize);
            PROFILER_LOG_ERROR(LOG_CORE, "snprintf_s(%s) error, errno(%d:%s)", file.c_str(), errno, buf);
            return -1;
        }

        char* realPath = realpath(filePath, nullptr);
        if (realPath == nullptr) {
            const int bufSize = 256;
            char buf[bufSize] = { 0 };
            strerror_r(errno, buf, bufSize);
            PROFILER_LOG_ERROR(LOG_CORE, "realpath(%s) failed, errno(%d:%s)", file.c_str(), errno, buf);
            return -1;
        }

        fd = open(realPath, O_RDONLY | O_CLOEXEC);
        if (fd == -1) {
            const int bufSize = 256;
            char buf[bufSize] = { 0 };
            strerror_r(errno, buf, bufSize);
            PROFILER_LOG_ERROR(LOG_CORE, "%s:failed to open(%s), errno(%d:%s)", __func__, realPath, errno, buf);
            return -1;
        }
        if (g_buffer == nullptr) {
            PROFILER_LOG_ERROR(LOG_CORE, "%s:empty address, g_buffer is NULL", __func__);
            close(fd);
            return -1;
        }
        bytesRead = read(fd, g_buffer.get(), BUFFER_SIZE - 1);
        if (bytesRead <= 0) {
            close(fd);
            PROFILER_LOG_ERROR(LOG_CORE, "%s:failed to read(%s), errno=%d", __func__, realPath, errno);
            return -1;
        }
        close(fd);
        free(realPath);

        return bytesRead;
    }

    void DepthFree(int depth, void *p)
    {
        StaticSpace staticeData;
        if (depth == 0) {
            staticeData.data[0] = 1;
            free(p);
            return;
        }
        return (DepthFree(depth - 1, p));
    }

    char *DepthMalloc(int depth)
    {
        StaticSpace staticeData;
        if (depth == 0) {
            staticeData.data[0] = 1;
            return reinterpret_cast<char *>(malloc(DEFAULT_MALLOC_SIZE));
        }
        return (DepthMalloc(depth - 1));
    }

    void ApplyForMalloc(int depth)
    {
        char *p = DepthMalloc(depth);
        if (!p) {
            const int bufSize = 256;
            char buf[bufSize] = { 0 };
            strerror_r(errno, buf, bufSize);
            PROFILER_LOG_ERROR(LOG_CORE, "ApplyForMalloc: malloc failure, errno(%d:%s)", errno, buf);
            return;
        }
        DepthFree(depth, p);
    }

#ifdef __aarch64__
    void DlopenAndCloseSo(std::string filePath, int size, int depth)
    {
        char *ptr = nullptr;
        void *handle = nullptr;
        DepthMallocSo mallocFunc = nullptr;
        DepthFreeSo freeFunc = nullptr;

        handle = dlopen(filePath.data(), RTLD_LAZY);
        if (handle == nullptr) {
            fprintf(stderr, "library not exist!\n");
            exit(0);
        }
        mallocFunc = (DepthMallocSo)dlsym(handle, "DepthMallocSo");
        freeFunc = (DepthFreeSo)dlsym(handle, "DepthFreeSo");
        if (mallocFunc == nullptr || freeFunc == nullptr) {
            fprintf(stderr, "function not exist!\n");
            exit(0);
        }
        ptr = mallocFunc(depth, size);
        *ptr = 'a';
        freeFunc(depth, ptr);
        if (handle != nullptr) {
            usleep(100000); // sleep 100000 us
            dlclose(handle);
        }
    }
#endif

    void StartMallocProcess()
    {
        if (signal(SIGCHLD, SIG_IGN) == SIG_ERR) {
            return;
        }
        int processNum = fork();
        if (processNum == 0) {
            while (1) {
                ApplyForMalloc(unwindDepth_);
                usleep(5000); // sleep 5000 us
            }
        } else {
            hookPid_ = processNum;
        }
    }

#ifdef __aarch64__
    void StartDlopenProcess()
    {
        if (signal(SIGCHLD, SIG_IGN) == SIG_ERR) {
            return;
        }
        int processNum = fork();
        if (processNum == 0) {
            const std::vector<std::string> VEC_SO_PATH { DEFAULT_LIBA_PATH, DEFAULT_LIBB_PATH};
            std::string cmdCopyLib{"cp " + DEFAULT_LIBNATIVETEST_PATH + " " + DEFAULT_LIBA_PATH};
            system(cmdCopyLib.c_str());
            cmdCopyLib = "cp " + DEFAULT_LIBA_PATH + " " + DEFAULT_LIBB_PATH;
            system(cmdCopyLib.c_str());
            while (true) {
                DlopenAndCloseSo(VEC_SO_PATH[0], LIBA_MALLOC_SIZE, unwindDepth_);
                DlopenAndCloseSo(VEC_SO_PATH[1], LIBB_MALLOC_SIZE, unwindDepth_);
            }
        } else {
            hookPid_ = processNum;
        }
    }
#endif

    char* DepthCalloc(int depth, int callocSize)
    {
        StaticSpace staticeData;
        if (depth == 0) {
            staticeData.data[0] = 1;
            return reinterpret_cast<char *>(calloc(sizeof(char), callocSize));
        }
        return (DepthCalloc(depth - 1, callocSize));
    }

    void ApplyForCalloc(int depth)
    {
        int callocSize = DEFAULT_CALLOC_SIZE / sizeof(char);
        char *p = DepthCalloc(depth, callocSize);
        if (!p) {
            const int bufSize = 256;
            char buf[bufSize] = { 0 };
            strerror_r(errno, buf, bufSize);
            PROFILER_LOG_ERROR(LOG_CORE, "ApplyForCalloc: calloc failure, errno(%d:%s)", errno, buf);
            return;
        }
        DepthFree(depth, p);
    }

    void StartCallocProcess(int depth)
    {
        if (signal(SIGCHLD, SIG_IGN) == SIG_ERR) {
            return;
        }
        int processNum = fork();
        if (processNum == 0) {
            int firstSleep = 3; // avoid malloc before sending kill -36 signal
            int secondSleep = 2;
            sleep(firstSleep);
            sleep(secondSleep);
            auto ret = malloc(DEFAULT_MALLOC_SIZE);
            free(ret);
            while (1) {
                ApplyForCalloc(depth);
                usleep(5000); // sleep 5000 us
            }
        } else {
            hookPid_ = processNum;
        }
    }

    char *DepthRealloc(int depth, void *p, int reallocSize)
    {
        StaticSpace staticeData;
        if (depth == 0) {
            staticeData.data[0] = 1;
            return reinterpret_cast<char *>(realloc(p, reallocSize));
        }
        return (DepthRealloc(depth - 1, p, reallocSize));
    }

    void ApplyForRealloc(int depth)
    {
        int reallocSize = DEFAULT_REALLOC_SIZE;
        char *p = reinterpret_cast<char *>(malloc(DEFAULT_MALLOC_SIZE));
        if (!p) {
            const int bufSize = 256;
            char buf[bufSize] = { 0 };
            strerror_r(errno, buf, bufSize);
            PROFILER_LOG_ERROR(LOG_CORE, "ApplyForRealloc: malloc failure, errno(%d:%s)", errno, buf);
            return;
        }
        char *np = DepthRealloc(depth, p, reallocSize);
        if (!np) {
            free(p);
            const int bufSize = 256;
            char buf[bufSize] = { 0 };
            strerror_r(errno, buf, bufSize);
            PROFILER_LOG_ERROR(LOG_CORE, "ApplyForRealloc: realloc failure, errno(%d:%s)", errno, buf);
            return;
        }
        DepthFree(depth, np);
    }

    void StartReallocProcess(int depth)
    {
        if (signal(SIGCHLD, SIG_IGN) == SIG_ERR) {
            return;
        }
        int processNum = fork();
        if (processNum == 0) {
            while (1) {
                ApplyForRealloc(depth);
                usleep(5000); // sleep 5000 us
            }
        } else {
            hookPid_ = processNum;
        }
    }

    bool Getdata(BufferSplitter& totalbuffer, std::vector<std::string>& hookVec, char delimiter)
    {
        totalbuffer.NextWord(delimiter);
        if (!totalbuffer.CurWord()) {
            return false;
        }
        std::string curWord = std::string(totalbuffer.CurWord(), totalbuffer.CurWordSize());
        hookVec.push_back(curWord);
        return true;
    }

    void StartAndStopHook()
    {
#ifdef COVERAGE_TEST
        const int coverageSleepTime = 5; // sleep 5s
        sleep(coverageSleepTime);

#endif
        StartDaemonProcessArgs();
        StopProcess(hookPid_);
        StopProcess(daemonPid_);
        sleep(SLEEP_FIVE);
    }
    int daemonPid_ = -1;
    int hookPid_ = -1;
    int modeIndex_ = 0;
    int unwindDepth_ = 0;
    int statisticsInterval_ = 0;
    int sampleInterval_ = 0;
    int jsReport_ = 0;
    int jsMaxDepth_ = 0;
    int mallocFreeMatchingInterval_ = 0;
    bool offlineSymbolization_ = false;
    bool callframeCompress_ = false;
    bool stringCompress_ = false;
    bool rawString_ = false;
    bool responseLibraryMode_ = false;
    bool saveFile_ = false;
    bool isHookStandalone_ = true;
    bool dumpdata_ = false;
    bool extendPid_ = false;
    bool nmd_ = false;
    std::string outFile_ = "";
    std::string outFileType_ = "";
    std::string mode_[2] = {"dwarf", "fp"};
    std::vector<char*> command_;
};

/**
 * @tc.name: native hook
 * @tc.desc: Test hook malloc normal process.
 * @tc.type: FUNC
 */
#ifdef __aarch64__
HWTEST_F(CheckHookDataTest, DFX_DFR_Hiprofiler_0080, Function | MediumTest | Level0)
{
    for (size_t i = 0; i < 2; ++i) { // 2: 0 is dwarf, 1 is fp mode
        unwindDepth_ = 30;
        outFileType_ = "malloc_";
        modeIndex_ = i; // 0 is dwarf, 1 is fp mode
        saveFile_ = true;
        isHookStandalone_ = false;
        StartMallocProcess();
        sleep(1);
        StartDaemonProcessArgs();
        sleep(WAIT_FLUSH);
        StopProcess(hookPid_);
        syscall(SYS_tkill, daemonPid_, 2);

        int32_t ret = ReadFile(outFile_);
        ASSERT_NE(ret, -1);

        BufferSplitter totalbuffer(const_cast<char*>((char*)g_buffer.get()), ret + 1);
        std::vector<std::string> hookVec;
        std::string addr = "";
        int depth = 0;
        int addrPos = 3;
        bool isFirstHook = true;
        do {
            char delimiter = ';';
            Getdata(totalbuffer, hookVec, delimiter);

            if (hookVec.size() < 6) {
                continue;
            }
            if (hookVec[0] == "malloc" && !isFirstHook) {
                for (int i = 0; i < MALLOC_GET_DATE_SIZE; i++) {
                    EXPECT_TRUE(Getdata(totalbuffer, hookVec, delimiter));
                }
                delimiter = '\n';
                EXPECT_TRUE(Getdata(totalbuffer, hookVec, delimiter));
                ASSERT_EQ(static_cast<int>(hookVec.size()), MALLOC_VEC_SIZE);
                ASSERT_TRUE(COMMON::IsNumeric(hookVec[4].c_str()));
                ASSERT_EQ(atoi(hookVec[4].c_str()), DEFAULT_MALLOC_SIZE); // 4: fifth hook data, default malloc size

                addr = hookVec[addrPos];
                depth = 0;
            } else if (hookVec[0] == "free" && !isFirstHook) {
                for (int i = 0; i < FREE_GET_DATA_SIZE; i++) {
                    EXPECT_TRUE(Getdata(totalbuffer, hookVec, delimiter));
                }
                delimiter = '\n';
                EXPECT_TRUE(Getdata(totalbuffer, hookVec, delimiter));
                ASSERT_EQ(static_cast<int>(hookVec.size()), FREE_VEC_SIZE);
                EXPECT_STREQ(hookVec[addrPos].c_str(), addr.c_str());
                EXPECT_EQ(depth, DEFAULT_DEPTH);

                isFirstHook = false;
                addr = "";
                depth = 0;
            } else {
                depth++;
            }

            hookVec.clear();
        } while (totalbuffer.NextLine());
    }
}

/**
 * @tc.name: native hook
 * @tc.desc: Test hook calloc normal process.
 * @tc.type: FUNC
 */
HWTEST_F(CheckHookDataTest, DFX_DFR_Hiprofiler_0090, Function | MediumTest | Level3)
{
    for (size_t i = 0; i < 2; ++i) { // 2: 0 is dwarf, 1 is fp mode
        int setDepth = 1;
        unwindDepth_ = 100;
        outFileType_ = "calloc_";
        modeIndex_ = i;
        saveFile_ = true;
        isHookStandalone_ = false;
        StartCallocProcess(setDepth);
        sleep(1);
        StartDaemonProcessArgs();
        sleep(WAIT_FLUSH);
        StopProcess(hookPid_);
        syscall(SYS_tkill, daemonPid_, 2);

        int32_t ret = ReadFile(outFile_);
        ASSERT_NE(ret, -1);

        BufferSplitter totalbuffer(const_cast<char*>((char*)g_buffer.get()), ret + 1);
        std::vector<std::string> hookVec;
        std::string addr = "";
        int depth = 0;
        int addrPos = 3;
        bool isFirstHook = true;
        do {
            char delimiter = ';';
            Getdata(totalbuffer, hookVec, delimiter);

            if (hookVec.size() < 6) {
                continue;
            }
            if (hookVec[0] == "malloc" && !isFirstHook) {
                for (int i = 0; i < MALLOC_GET_DATE_SIZE; i++) {
                    EXPECT_TRUE(Getdata(totalbuffer, hookVec, delimiter));
                }
                delimiter = '\n';
                EXPECT_TRUE(Getdata(totalbuffer, hookVec, delimiter));
                ASSERT_EQ(static_cast<int>(hookVec.size()), MALLOC_VEC_SIZE);
                ASSERT_TRUE(COMMON::IsNumeric(hookVec[4].c_str()));
                ASSERT_EQ(atoi(hookVec[4].c_str()), DEFAULT_CALLOC_SIZE); // 4: fifth hook data, default malloc size

                addr = hookVec[addrPos];
                depth = 0;
            } else if (hookVec[0] == "free" && !isFirstHook) {
                for (int i = 0; i < FREE_GET_DATA_SIZE; i++) {
                    EXPECT_TRUE(Getdata(totalbuffer, hookVec, delimiter));
                }
                delimiter = '\n';
                EXPECT_TRUE(Getdata(totalbuffer, hookVec, delimiter));
                ASSERT_EQ(static_cast<int>(hookVec.size()), FREE_VEC_SIZE);
                EXPECT_STREQ(hookVec[addrPos].c_str(), addr.c_str());
                EXPECT_GE(depth, CALLOC_DEPTH);

                isFirstHook = false;
                addr = "";
                depth = 0;
            } else {
                depth++;
            }

            hookVec.clear();
        } while (totalbuffer.NextLine());
    }
}

/**
 * @tc.name: native hook
 * @tc.desc: Test hook realloc normal process.
 * @tc.type: FUNC
 */
HWTEST_F(CheckHookDataTest, DFX_DFR_Hiprofiler_0100, Function | MediumTest | Level3)
{
    for (size_t i = 0; i < 2; ++i) { // 2: 0 is dwarf, 1 is fp mode
        int setDepth = 100;
        outFileType_ = "realloc_";
        modeIndex_ = i;
        saveFile_ = true;
        isHookStandalone_ = false;
        StartReallocProcess(setDepth);
        sleep(1);
        StartDaemonProcessArgs();
        sleep(WAIT_FLUSH);
        StopProcess(hookPid_);
        syscall(SYS_tkill, daemonPid_, 2);

        int32_t ret = ReadFile(outFile_);
        ASSERT_NE(ret, -1);

        BufferSplitter totalbuffer(const_cast<char*>((char*)g_buffer.get()), ret + 1);
        std::vector<std::string> hookVec;
        std::string mallocAddr = "";
        std::string reallocAddr = "";
        int depth = 0;
        int addrPos = 3;
        bool isFirstHook = true;
        bool isRealloc = false;
        do {
            char delimiter = ';';
            Getdata(totalbuffer, hookVec, delimiter);

            if (hookVec.size() < 6) {
                continue;
            }
            if (hookVec[0] == "malloc" && !isFirstHook) {
                for (int i = 0; i < MALLOC_GET_DATE_SIZE; i++) {
                    EXPECT_TRUE(Getdata(totalbuffer, hookVec, delimiter));
                }
                delimiter = '\n';
                EXPECT_TRUE(Getdata(totalbuffer, hookVec, delimiter));
                ASSERT_EQ(static_cast<int>(hookVec.size()), MALLOC_VEC_SIZE);

                if (isRealloc) {
                    reallocAddr = hookVec[addrPos];
                    // 4: fifth hook data, default malloc size
                    ASSERT_TRUE(COMMON::IsNumeric(hookVec[4].c_str()));
                    ASSERT_GE(atoi(hookVec[4].c_str()), DEFAULT_REALLOC_SIZE);
                    EXPECT_GE(depth, REALLOC_DEPTH);
                    isFirstHook = false;
                } else {
                    mallocAddr = hookVec[addrPos];
                    // 4: fifth hook data, default malloc size
                    ASSERT_TRUE(COMMON::IsNumeric(hookVec[4].c_str()));
                    ASSERT_EQ(atoi(hookVec[4].c_str()), DEFAULT_MALLOC_SIZE);
                }

                isRealloc = true;
                depth = 0;
            } else if (hookVec[0] == "free" && !isFirstHook) {
                for (int i = 0; i < FREE_GET_DATA_SIZE; i++) {
                    EXPECT_TRUE(Getdata(totalbuffer, hookVec, delimiter));
                }
                delimiter = '\n';
                EXPECT_TRUE(Getdata(totalbuffer, hookVec, delimiter));
                ASSERT_EQ(static_cast<int>(hookVec.size()), FREE_VEC_SIZE);

                if (isRealloc) {
                    EXPECT_STREQ(hookVec[addrPos].c_str(), reallocAddr.c_str());
                    reallocAddr = "";
                } else {
                    EXPECT_STREQ(hookVec[addrPos].c_str(), mallocAddr.c_str());
                    mallocAddr = "";
                }

                isRealloc = false;
                depth = 0;
            } else {
                depth++;
            }

            hookVec.clear();
        } while (totalbuffer.NextLine());
    }
}

/**
 * @tc.name: native hook
 * @tc.desc: Test hook dlopen normal process. just for arm64
 * @tc.type: FUNC
 */
HWTEST_F(CheckHookDataTest, DFX_DFR_Hiprofiler_0110, Function | MediumTest | Level3)
{
    for (size_t i = 1; i < 2; ++i) {
        unwindDepth_ = 6;
        outFileType_ = "dlopen_";
        modeIndex_ = i;
        saveFile_ = true;
        isHookStandalone_ = false;
        StartDlopenProcess();
        sleep(1);
        StartDaemonProcessArgs();
        sleep(WAIT_FLUSH);
        StopProcess(hookPid_);
        syscall(SYS_tkill, daemonPid_, 2);

        std::ifstream infile;
        infile.open(outFile_, std::ios::in);
        ASSERT_TRUE(infile.is_open());
        std::string buf;
        uint8_t mallocPos = 0;
        uint8_t mallocSizePos = 6;
        uint8_t libUnwindDepth = 3;
        while (getline(infile, buf))
        {
            std::vector<std::string>& resultVec = StringSplit(buf);
            if (resultVec.size() < static_cast<size_t>(mallocSizePos)) {
                continue;
            }
            if (resultVec[mallocPos] == "malloc") {
                if (resultVec[mallocSizePos] == std::to_string(LIBA_MALLOC_SIZE)) {
                    std::cout << buf << std::endl;
                    for (size_t i = 0; i < libUnwindDepth; i++) {
                        getline(infile, buf);
                    }
                    std::cout << buf << std::endl;
                    EXPECT_TRUE((buf.find("liba.z.so") != std::string::npos) ||
                                (buf.find("libb.z.so") != std::string::npos));
                } else if (resultVec[mallocSizePos] == std::to_string(LIBB_MALLOC_SIZE)) {
                    std::cout << buf << std::endl;
                    for (size_t i = 0; i < libUnwindDepth; i++) {
                        getline(infile, buf);
                    }
                    std::cout << buf << std::endl;
                    EXPECT_TRUE((buf.find("liba.z.so") != std::string::npos) ||
                                (buf.find("libb.z.so") != std::string::npos));
                }
            }
        }
    }
}

/**
 * @tc.name: native hook
 * @tc.desc: Test hook statistics data normal process.
 * @tc.type: FUNC
 */
HWTEST_F(CheckHookDataTest, DFX_DFR_Hiprofiler_0120, Function | MediumTest | Level0)
{
    for (size_t i = 0; i < 2; ++i) { // 2: 0 is dwarf, 1 is fp mode
        unwindDepth_ = 10;
        statisticsInterval_ = 1;
        outFileType_ = "statistics_interval_";
        modeIndex_ = i;
        StartMallocProcess();
        sleep(1);
        StartDaemonProcessArgs();
        sleep(WAIT_FLUSH);
        StopProcess(hookPid_);
        syscall(SYS_tkill, daemonPid_, 2);

        std::ifstream infile;
        infile.open(outFile_, std::ios::in);
        ASSERT_TRUE(infile.is_open());
        std::string buf;
        std::string expectCallStackId;
        std::string statisticsCallStackId;
        while (getline(infile, buf)) {
            if (buf.find("stack_map") != std::string::npos) {
                if (!expectCallStackId.empty()) {
                    continue;
                }
                getline(infile, buf); // read stack_map id
                std::vector<std::string>& resultVec = StringSplit(buf, ':');
                expectCallStackId = resultVec[1];
                std::cout << "expectCallStackId: " << expectCallStackId << std::endl;
            } else if (buf.find("statistics_event") != std::string::npos) {
                getline(infile, buf); // read statistics_event pid
                getline(infile, buf); // read statistics_event callstack_id
                std::vector<std::string>& resultVec = StringSplit(buf, ':');
                statisticsCallStackId = resultVec[1];
                std::cout << "statisticsCallStackId: " << statisticsCallStackId << std::endl;
                if (expectCallStackId == statisticsCallStackId) {
                    break;
                }
            }
        }
        getline(infile, buf); // read statistics_event apply_count
        if (buf.find("type") != std::string::npos) {
            getline(infile, buf);
        }
        std::vector<std::string>& resultVec = StringSplit(buf, ':');
        ASSERT_FALSE(resultVec.empty());
        if (!resultVec.empty() && resultVec.size() > 1) {
            ASSERT_TRUE(COMMON::IsNumeric(resultVec[1].c_str()));
            uint16_t applyCount = std::atoi(resultVec[1].c_str());
            std::cout << "applyCount: " << applyCount << std::endl;
            EXPECT_TRUE(applyCount > 0);
        }
        sleep(SLEEP_TIME);
    }
}
#endif

/**
 * @tc.name: native hook
 * @tc.desc: Test hook offline symbolization data normal process.
 * @tc.type: FUNC
 */
HWTEST_F(CheckHookDataTest, DFX_DFR_Hiprofiler_0130, Function | MediumTest | Level0)
{
    for (size_t i = 0; i < 2; ++i) {
        unwindDepth_ = 10;
        outFileType_ = "offline_symbolization_";
        modeIndex_ = i;
        offlineSymbolization_ = true;
        StartMallocProcess();
        sleep(1);
        StartDaemonProcessArgs();
        sleep(WAIT_FLUSH);
        StopProcess(hookPid_);
        syscall(SYS_tkill, daemonPid_, 2);

        std::ifstream infile;
        infile.open(outFile_, std::ios::in);
        ASSERT_TRUE(infile.is_open());
        std::string buf;
        std::string symTable;
        std::string strTable;
        std::string ipString;
        while (getline(infile, buf)) {
            if (buf.find("stack_map") != std::string::npos) {
                getline(infile, buf); // read stack map id
                getline(infile, buf); // read stack map ip
                std::vector<std::string>& resultVec = StringSplit(buf, ':');
                ipString = resultVec[0];
                // delete whitespace characters
                ipString.erase(std::remove(ipString.begin(), ipString.end(), ' '), ipString.end());
                EXPECT_TRUE(ipString == "ip");
            } else if (buf.find("sym_table") != std::string::npos) {
                std::vector<std::string>& resultVec = StringSplit(buf, ':');
                symTable = resultVec[1];
                EXPECT_TRUE(symTable.size() > 0);
            } else if (buf.find("str_table") != std::string::npos) {
                std::vector<std::string>& resultVec = StringSplit(buf, ':');
                strTable = resultVec[1];
                EXPECT_TRUE(strTable.size() > 0);
                if (ipString == "ip" && symTable.size()) {
                    break;
                }
            }
        }
    }
}

/**
 * @tc.name: native hook
 * @tc.desc: Test hook callframe compress normal process.
 * @tc.type: FUNC
 */
#ifdef __aarch64__
HWTEST_F(CheckHookDataTest, DFX_DFR_Hiprofiler_0140, Function | MediumTest | Level3)
{
    for (size_t i = 0; i < 2; ++i) { // 2: 0 is dwarf, 1 is fp mode
        unwindDepth_ = 6;
        outFileType_ = "callframecompress_";
        modeIndex_ = i;
        callframeCompress_ = true;
        StartMallocProcess();
        sleep(1);
        StartDaemonProcessArgs();
        sleep(WAIT_FLUSH);
        StopProcess(hookPid_);
        syscall(SYS_tkill, daemonPid_, 2);

        std::ifstream infile;
        infile.open(outFile_, std::ios::in);
        ASSERT_TRUE(infile.is_open());
        std::string buf;
        bool findSymbolName;
        bool findfilePath;
        bool findFrameMap;
        bool findStackMap;
        while (getline(infile, buf))
        {
            if (!findSymbolName || buf.find("symbol_name") != std::string::npos) {
                findSymbolName = true;
            } else if (!findfilePath || buf.find("file_path") != std::string::npos) {
                findfilePath = true;
            } else if (!findFrameMap || buf.find("frame_map") != std::string::npos) {
                findFrameMap = true;
            } else if (!findStackMap || buf.find("stack_map") != std::string::npos) {
                findStackMap = true;
                if (findSymbolName && findfilePath && findFrameMap) {
                    break;
                }
            }
        }
        EXPECT_TRUE(findSymbolName);
        EXPECT_TRUE(findfilePath);
        EXPECT_TRUE(findFrameMap);
        EXPECT_TRUE(findStackMap);
    }
}

/**
 * @tc.name: native hook
 * @tc.desc: Test hook string compress normal process.
 * @tc.type: FUNC
 */
HWTEST_F(CheckHookDataTest, DFX_DFR_Hiprofiler_0150, Function | MediumTest | Level3)
{
    for (size_t i = 0; i < 2; ++i) {
        unwindDepth_ = 6;
        outFileType_ = "stringcompress_";
        modeIndex_ = i;
        stringCompress_ = true;
        StartMallocProcess();
        sleep(1);
        StartDaemonProcessArgs();
        sleep(WAIT_FLUSH);
        StopProcess(hookPid_);
        syscall(SYS_tkill, daemonPid_, 2);

        std::ifstream infile;
        infile.open(outFile_, std::ios::in);
        ASSERT_TRUE(infile.is_open());
        std::string buf;
        bool findFrameInfo;
        bool findSymbolNameId;
        while (getline(infile, buf))
        {
            if (!findFrameInfo || buf.find("frame_info") != std::string::npos) {
                findFrameInfo = true;
            } else if (!findSymbolNameId || buf.find("symbol_name_id") != std::string::npos) {
                findSymbolNameId = true;
                if (findFrameInfo) {
                    break;
                }
            }
        }
        EXPECT_TRUE(findFrameInfo);
        EXPECT_TRUE(findSymbolNameId);
    }
}

/**
 * @tc.name: native hook
 * @tc.desc: Test hook raw string normal process.
 * @tc.type: FUNC
 */
HWTEST_F(CheckHookDataTest, DFX_DFR_Hiprofiler_0160, Function | MediumTest | Level3)
{
    for (size_t i = 0; i < 2; ++i) { // 2: 0 is dwarf, 1 is fp mode
        unwindDepth_ = 6;
        outFileType_ = "rawstring_";
        modeIndex_ = i;
        rawString_ = true;
        StartMallocProcess();
        sleep(1);
        StartDaemonProcessArgs();
        sleep(WAIT_FLUSH);
        StopProcess(hookPid_);
        syscall(SYS_tkill, daemonPid_, 2);

        std::ifstream infile;
        infile.open(outFile_, std::ios::in);
        ASSERT_TRUE(infile.is_open());
        std::string buf;
        bool findFrameInfo;
        bool findSymbolName;
        while (getline(infile, buf))
        {
            if (!findFrameInfo || buf.find("frame_info") != std::string::npos) {
                findFrameInfo = true;
            } else if (!findSymbolName || buf.find("symbol_name") != std::string::npos) {
                findSymbolName = true;
                if (findFrameInfo) {
                    break;
                }
            }
        }
        EXPECT_TRUE(findFrameInfo);
        EXPECT_TRUE(findSymbolName);
    }
}

/**
 * @tc.name: native hook
 * @tc.desc: Test hook raw responseLibraryMode normal process.
 * @tc.type: FUNC
 */
HWTEST_F(CheckHookDataTest, DFX_DFR_Hiprofiler_0170, Function | MediumTest | Level3)
{
    for (size_t i = 1; i < 2; ++i) { // 1 is fp mode,  response_library_mode only fp mode is used
        unwindDepth_ = 6;
        outFileType_ = "responseLibraryMode";
        modeIndex_ = i;
        responseLibraryMode_ = true;
        StartMallocProcess();
        sleep(1);
        StartDaemonProcessArgs();
        sleep(WAIT_FLUSH);
        StopProcess(hookPid_);
        syscall(SYS_tkill, daemonPid_, 2);

        std::ifstream infile;
        infile.open(outFile_, std::ios::in);
        ASSERT_TRUE(infile.is_open());
        std::string buf;
        uint16_t ipCount = 0;

        while (getline(infile, buf)) {
            if (buf.find("stack_map") != std::string::npos) {
                while (getline(infile, buf)) {
                    if (buf.find("ip") != std::string::npos) {
                        ++ipCount;
                        continue;
                    } else if (buf.find("}") != std::string::npos) {
                        break;
                    }
                }
            }
            if (ipCount > 0) {
                break;
            }
        }
        EXPECT_TRUE(ipCount == 1); // response_library_mode callstack depth only is 1
    }
}

/**
 * @tc.name: native hook
 * @tc.desc: Test hook statistics data normal process.
 * @tc.type: FUNC
 */
HWTEST_F(CheckHookDataTest, DFX_DFR_Hiprofiler_0180, Function | MediumTest | Level0)
{
    for (size_t i = 0; i < 2; ++i) { // 2: 0 is dwarf, 1 is fp mode
        unwindDepth_ = 10;
        statisticsInterval_ = 1;
        sampleInterval_ = 256;
        outFileType_ = "sample_interval_";
        modeIndex_ = i;
        StartMallocProcess();
        sleep(1);
        StartDaemonProcessArgs();
        sleep(WAIT_FLUSH);
        StopProcess(hookPid_);
        syscall(SYS_tkill, daemonPid_, 2);

        std::ifstream infile;
        infile.open(outFile_, std::ios::in);
        ASSERT_TRUE(infile.is_open());
        std::string buf;
        std::string expectCallStackId;
        std::string statisticsCallStackId;
        while (getline(infile, buf)) {
            if (buf.find("stack_map") != std::string::npos) {
                if (!expectCallStackId.empty()) {
                    continue;
                }
                getline(infile, buf); // read stack_map id
                std::vector<std::string>& resultVec = StringSplit(buf, ':');
                expectCallStackId = resultVec[1];
                std::cout << "expectCallStackId: " << expectCallStackId << std::endl;
            } else if (buf.find("statistics_event") != std::string::npos) {
                getline(infile, buf); // read statistics_event pid
                getline(infile, buf); // read statistics_event callstack_id
                std::vector<std::string>& resultVec = StringSplit(buf, ':');
                statisticsCallStackId = resultVec[1];
                std::cout << "statisticsCallStackId: " << statisticsCallStackId << std::endl;
                if (expectCallStackId == statisticsCallStackId) {
                    break;
                }
            }
        }
        getline(infile, buf); // read statistics_event apply_count
        if (buf.find("type") != std::string::npos) {
            getline(infile, buf);
        }
        std::vector<std::string>& resultVec = StringSplit(buf, ':');
        ASSERT_FALSE(resultVec.empty());
        ASSERT_TRUE(COMMON::IsNumeric(resultVec[1].c_str()));
        uint16_t applyCount = std::atoi(resultVec[1].c_str());
        std::cout << "applyCount: " << applyCount << std::endl;
        EXPECT_TRUE(applyCount > 0);
        sleep(SLEEP_TIME);
    }
}

/**
 * @tc.name: native hook
 * @tc.desc: Test hook alloc free matching interval normal process.
 * @tc.type: FUNC
 */
HWTEST_F(CheckHookDataTest, DFX_DFR_Hiprofiler_0190, Function | MediumTest | Level3)
{
    for (size_t i = 0; i < 2; ++i) { // 2: 0 is dwarf, 1 is fp mode
        unwindDepth_ = 6;
        outFileType_ = "mallocFreeMatchingInterval_";
        modeIndex_ = i;
        mallocFreeMatchingInterval_ = 2;
        StartMallocProcess();
        sleep(1);
        StartDaemonProcessArgs();
        sleep(WAIT_FLUSH);
        StopProcess(hookPid_);
        syscall(SYS_tkill, daemonPid_, 2);

        std::ifstream infile;
        infile.open(outFile_, std::ios::in);
        ASSERT_TRUE(infile.is_open());
        std::string buf;
        bool findSymbolName;
        bool findfilePath;
        bool findFrameMap;
        bool findStackMap;
        while (getline(infile, buf))
        {
            if (!findSymbolName || buf.find("symbol_name") != std::string::npos) {
                findSymbolName = true;
            } else if (!findfilePath || buf.find("file_path") != std::string::npos) {
                findfilePath = true;
            } else if (!findFrameMap || buf.find("frame_map") != std::string::npos) {
                findFrameMap = true;
            } else if (!findStackMap || buf.find("stack_map") != std::string::npos) {
                findStackMap = true;
                if (findSymbolName && findfilePath && findFrameMap) {
                    break;
                }
            }
        }
        EXPECT_TRUE(findSymbolName);
        EXPECT_TRUE(findfilePath);
        EXPECT_TRUE(findFrameMap);
        EXPECT_TRUE(findStackMap);
    }
}


/**
 * @tc.name: native hook
 * @tc.desc: Test hook js statistics data normal process.
 * @tc.type: FUNC
 */
HWTEST_F(CheckHookDataTest, DFX_DFR_Hiprofiler_0200, Function | MediumTest | Level0)
{
    for (size_t i = 1; i < 2; ++i) { // 2: 0 is dwarf, 1 is fp mode
        unwindDepth_ = 10;
        statisticsInterval_ = 1;
        jsReport_ = 1;
        jsMaxDepth_ = 10;
        outFileType_ = "js_report_";
        modeIndex_ = i;
        StartMallocProcess();
        sleep(1);
        StartDaemonProcessArgs();
        sleep(WAIT_FLUSH);
        StopProcess(hookPid_);
        syscall(SYS_tkill, daemonPid_, 2);

        std::ifstream infile;
        infile.open(outFile_, std::ios::in);
        ASSERT_TRUE(infile.is_open());
        std::string buf;
        std::string expectCallStackId;
        std::string statisticsCallStackId;
        while (getline(infile, buf)) {
            if (buf.find("stack_map") != std::string::npos) {
                if (!expectCallStackId.empty()) {
                    continue;
                }
                getline(infile, buf); // read stack_map id
                std::vector<std::string>& resultVec = StringSplit(buf, ':');
                expectCallStackId = resultVec[1];
                std::cout << "expectCallStackId: " << expectCallStackId << std::endl;
            } else if (buf.find("statistics_event") != std::string::npos) {
                getline(infile, buf); // read statistics_event pid
                getline(infile, buf); // read statistics_event callstack_id
                std::vector<std::string>& resultVec = StringSplit(buf, ':');
                statisticsCallStackId = resultVec[1];
                std::cout << "statisticsCallStackId: " << statisticsCallStackId << std::endl;
                if (expectCallStackId == statisticsCallStackId) {
                    break;
                }
            }
        }
        getline(infile, buf); // read statistics_event apply_count
        if (buf.find("type") != std::string::npos) {
            getline(infile, buf);
        }
        std::vector<std::string>& resultVec = StringSplit(buf, ':');
        ASSERT_FALSE(resultVec.empty());
        ASSERT_TRUE(COMMON::IsNumeric(resultVec[1].c_str()));
        uint16_t applyCount = std::atoi(resultVec[1].c_str());
        std::cout << "applyCount: " << applyCount << std::endl;
        EXPECT_TRUE(applyCount > 0);
        sleep(SLEEP_TIME);
    }
}
#endif

/**
 * @tc.name: native hook
 * @tc.desc: Test no data queue normal process.
 * @tc.type: FUNC
 */
HWTEST_F(CheckHookDataTest, DFX_DFR_Hiprofiler_0210, Function | MediumTest | Level0)
{
    for (size_t i = 1; i < 2; ++i) { // 2: 0 is dwarf, 1 is fp mode
        unwindDepth_ = 10;
        statisticsInterval_ = 1;
        sampleInterval_ = 256;
        offlineSymbolization_ = true;
        outFileType_ = "no_data_queue_";
        modeIndex_ = i;
        StartMallocProcess();
        sleep(1);
        StartDaemonProcessArgs();
        sleep(WAIT_FLUSH);
        StopProcess(hookPid_);
        syscall(SYS_tkill, daemonPid_, 2);

        std::ifstream infile;
        infile.open(outFile_, std::ios::in);
        ASSERT_TRUE(infile.is_open());
        std::string buf;
        std::string expectCallStackId;
        std::string statisticsCallStackId;
        while (getline(infile, buf)) {
            if (buf.find("stack_map") != std::string::npos) {
                if (!expectCallStackId.empty()) {
                    continue;
                }
                getline(infile, buf); // read stack_map id
                std::vector<std::string>& resultVec = StringSplit(buf, ':');
                expectCallStackId = resultVec[1];
                std::cout << "expectCallStackId: " << expectCallStackId << std::endl;
            } else if (buf.find("statistics_event") != std::string::npos) {
                getline(infile, buf); // read statistics_event pid
                getline(infile, buf); // read statistics_event callstack_id
                std::vector<std::string>& resultVec = StringSplit(buf, ':');
                statisticsCallStackId = resultVec[1];
                std::cout << "statisticsCallStackId: " << statisticsCallStackId << std::endl;
                if (expectCallStackId == statisticsCallStackId) {
                    break;
                }
            }
        }
        getline(infile, buf); // read statistics_event apply_count
        if (buf.find("type") != std::string::npos) {
            getline(infile, buf);
        }
        std::vector<std::string>& resultVec = StringSplit(buf, ':');
        ASSERT_FALSE(resultVec.empty());
        ASSERT_TRUE(COMMON::IsNumeric(resultVec[1].c_str()));
        uint16_t applyCount = std::atoi(resultVec[1].c_str());
        std::cout << "applyCount: " << applyCount << std::endl;
        EXPECT_TRUE(applyCount > 0);
        sleep(SLEEP_TIME);
    }
}

/**
 * @tc.name: native hook
 * @tc.desc: Test dumpdata normal process.
 * @tc.type: FUNC
 */
HWTEST_F(CheckHookDataTest, DFX_DFR_Hiprofiler_0220, Function | MediumTest | Level0)
{
    for (size_t i = 1; i < 2; ++i) { // 2: 0 is dwarf, 1 is fp mode
        unwindDepth_ = 10;
        statisticsInterval_ = 1;
        modeIndex_ = i;
        dumpdata_ = true;
        StartMallocProcess();
        sleep(1);
        StartDaemonProcessArgs();
        sleep(WAIT_FLUSH);
        StopProcess(hookPid_);
        syscall(SYS_tkill, daemonPid_, 2);

        std::ifstream infile;
        infile.open(outFile_, std::ios::in);
        ASSERT_TRUE(infile.is_open());
        std::string buf;
        std::string expectCallStackId;
        std::string statisticsCallStackId;
        while (getline(infile, buf)) {
            if (buf.find("stack_map") != std::string::npos) {
                if (!expectCallStackId.empty()) {
                    continue;
                }
                getline(infile, buf); // read stack_map id
                std::vector<std::string>& resultVec = StringSplit(buf, ':');
                expectCallStackId = resultVec[1];
                std::cout << "expectCallStackId: " << expectCallStackId << std::endl;
            } else if (buf.find("statistics_event") != std::string::npos) {
                getline(infile, buf); // read statistics_event pid
                getline(infile, buf); // read statistics_event callstack_id
                std::vector<std::string>& resultVec = StringSplit(buf, ':');
                statisticsCallStackId = resultVec[1];
                std::cout << "statisticsCallStackId: " << statisticsCallStackId << std::endl;
                if (expectCallStackId == statisticsCallStackId) {
                    break;
                }
            }
        }
        getline(infile, buf); // read statistics_event apply_count
        if (buf.find("type") != std::string::npos) {
            getline(infile, buf);
        }
        std::vector<std::string>& resultVec = StringSplit(buf, ':');
        ASSERT_FALSE(resultVec.empty());
        ASSERT_TRUE(COMMON::IsNumeric(resultVec[1].c_str()));
        uint16_t applyCount = std::atoi(resultVec[1].c_str());
        std::cout << "applyCount: " << applyCount << std::endl;
        EXPECT_TRUE(applyCount > 0);
        sleep(SLEEP_TIME);
    }
}

/**
 * @tc.name: native hook
 * @tc.desc: Test nmd normal process.
 * @tc.type: FUNC
 */
#ifdef __aarch64__
HWTEST_F(CheckHookDataTest, DFX_DFR_Hiprofiler_0230, Function | MediumTest | Level0)
{
    for (size_t i = 1; i < 2; ++i) { // 2: 0 is dwarf, 1 is fp mode
        modeIndex_ = i;
        dumpdata_ = true;
        nmd_ = true;
        StartMallocProcess();
        sleep(1);
        StartDaemonProcessArgs();
        sleep(WAIT_FLUSH);
        StopProcess(hookPid_);
        syscall(SYS_tkill, daemonPid_, 2);

        std::ifstream infile;
        outFile_ = "/data/local/tmp/test_dump_file0.htrace";
        infile.open(outFile_, std::ios::in);
        ASSERT_TRUE(infile.is_open());
        std::string buf;
        bool nmdResult = false;
        while (getline(infile, buf)) {
            if (buf.find("End jemalloc ohos statistics") != std::string::npos) {
                nmdResult = true;
            }
        }
        EXPECT_TRUE(nmdResult);
        sleep(SLEEP_TIME);
    }
}
#endif

/**
 * @tc.name: native hook
 * @tc.desc: Test dumpdata with -pe command.
 * @tc.type: FUNC
 */
HWTEST_F(CheckHookDataTest, DFX_DFR_Hiprofiler_0240, Function | MediumTest | Level0)
{
    for (size_t i = 1; i < 2; ++i) { // 2: 0 is dwarf, 1 is fp mode
        unwindDepth_ = 10;
        statisticsInterval_ = 1;
        modeIndex_ = i;
        dumpdata_ = true;
        offlineSymbolization_ = true;
        extendPid_ = true;
        outFileType_ = "extend_pid_";
        StartMallocProcess();
        sleep(1);
        StartDaemonProcessArgs();
        sleep(WAIT_FLUSH);
        StopProcess(hookPid_);
        syscall(SYS_tkill, daemonPid_, 2);

        std::ifstream infile;
        infile.open(outFile_, std::ios::in);
        ASSERT_TRUE(infile.is_open());
        std::string buf;
        std::string expectCallStackId;
        std::string statisticsCallStackId;
        while (getline(infile, buf)) {
            if (buf.find("stack_map") != std::string::npos) {
                if (!expectCallStackId.empty()) {
                    continue;
                }
                getline(infile, buf); // read stack_map id
                std::vector<std::string>& resultVec = StringSplit(buf, ':');
                expectCallStackId = resultVec[1];
                std::cout << "expectCallStackId: " << expectCallStackId << std::endl;
            } else if (buf.find("statistics_event") != std::string::npos) {
                getline(infile, buf); // read statistics_event pid
                getline(infile, buf); // read statistics_event callstack_id
                std::vector<std::string>& resultVec = StringSplit(buf, ':');
                statisticsCallStackId = resultVec[1];
                std::cout << "statisticsCallStackId: " << statisticsCallStackId << std::endl;
                if (expectCallStackId == statisticsCallStackId) {
                    break;
                }
            }
        }
        getline(infile, buf); // read statistics_event apply_count
        if (buf.find("type") != std::string::npos) {
            getline(infile, buf);
        }
        std::vector<std::string>& resultVec = StringSplit(buf, ':');
        ASSERT_FALSE(resultVec.empty());
        ASSERT_TRUE(COMMON::IsNumeric(resultVec[1].c_str()));
        uint16_t applyCount = std::atoi(resultVec[1].c_str());
        std::cout << "applyCount: " << applyCount << std::endl;
        EXPECT_TRUE(applyCount > 0);
        sleep(SLEEP_TIME);
    }
}
}

#pragma clang optimize on