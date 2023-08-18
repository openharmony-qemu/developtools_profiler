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
#include "hidump_plugin.h"
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <cinttypes>
#include <csignal>
#include <sstream>
#include <sys/wait.h>
#include "securec.h"

namespace {
const int REDIRECT_STDOUT = 1;
const int REDIRECT_STDERR = 2;
const int SLEEP_TIME = 50;
const int BUF_MAX_LEN = 64;
const int MS_PER_S = 1000;
const int US_PER_S = 1000000;
const char *g_fpsFormat = "GP_daemon_fps 31104000";

static pid_t volatile g_child;
const int READ = 0;
const int WRITE = 1;
const int PIPE_LEN = 2;
const std::string BIN_COMMAND("/bin/sh");
} // namespace

HidumpPlugin::HidumpPlugin() : fp_(nullptr, nullptr) {}

HidumpPlugin::~HidumpPlugin()
{
    HILOG_INFO(LOG_CORE, "%s: ready!", __func__);
    std::unique_lock<std::mutex> locker(mutex_);
    if (running_) {
        running_ = false;
        if (writeThread_.joinable()) {
            writeThread_.join();
        }
    }
    locker.unlock();

    if (fp_ != nullptr) {
        fp_.reset();
    }
    HILOG_INFO(LOG_CORE, "%s: success!", __func__);
}

int HidumpPlugin::Start(const uint8_t* configData, uint32_t configSize)
{
    HILOG_INFO(LOG_CORE, "HidumpPlugin:Start ----> !");
    CHECK_TRUE(protoConfig_.ParseFromArray(configData, configSize) > 0, -1, "HidumpPlugin: ParseFromArray failed");

    fp_ = std::unique_ptr<FILE, int (*)(FILE*)>(CustomPopen(g_fpsFormat, "r"), CustomPclose);
    if (fp_.get() == nullptr) {
        const int bufSize = 256;
        char buf[bufSize] = {0};
        strerror_r(errno, buf, bufSize);
        HILOG_ERROR(LOG_CORE, "HidumpPlugin: CustomPopen(%s) Failed, errno(%d:%s)", g_fpsFormat, errno, buf);
        return -1;
    }
    CHECK_NOTNULL(resultWriter_, -1, "HidumpPlugin: Writer is no set!");
    CHECK_NOTNULL(resultWriter_->write, -1, "HidumpPlugin: Writer.write is no set!");
    CHECK_NOTNULL(resultWriter_->flush, -1, "HidumpPlugin: Writer.flush is no set!");
    std::unique_lock<std::mutex> locker(mutex_);
    running_ = true;
    writeThread_ = std::thread(&HidumpPlugin::Loop, this);

    HILOG_INFO(LOG_CORE, "HidumpPlugin: ---> Start success!");
    return 0;
}

int HidumpPlugin::Stop()
{
    std::unique_lock<std::mutex> locker(mutex_);
    running_ = false;
    locker.unlock();
    if (writeThread_.joinable()) {
        writeThread_.join();
    }
    HILOG_INFO(LOG_CORE, "HidumpPlugin:stop thread success!");
    if (fp_ != nullptr) {
        fp_.reset();
    }
    HILOG_INFO(LOG_CORE, "HidumpPlugin: stop success!");
    return 0;
}

int HidumpPlugin::SetWriter(WriterStruct* writer)
{
    resultWriter_ = writer;
    return 0;
}

void HidumpPlugin::Loop(void)
{
    HILOG_INFO(LOG_CORE, "HidumpPlugin: Loop start");
    HidumpInfo dataProto;

    fcntl(fileno(fp_.get()), F_SETFL, O_NONBLOCK);
    while (running_) {
        char buf[BUF_MAX_LEN] = { 0 };

        if (fgets(buf, BUF_MAX_LEN - 1, fp_.get()) == nullptr) {
            std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_TIME));
            continue;
        }
        if (!ParseHidumpInfo(dataProto, buf)) {
            continue;
        }
        if (dataProto.ByteSizeLong() > 0) {
            buffer_.resize(dataProto.ByteSizeLong());
            dataProto.SerializeToArray(buffer_.data(), buffer_.size());
            resultWriter_->write(resultWriter_, buffer_.data(), buffer_.size());
            resultWriter_->flush(resultWriter_);
            dataProto.clear_fps_event();
        }
    }
    buffer_.resize(dataProto.ByteSizeLong());
    dataProto.SerializeToArray(buffer_.data(), buffer_.size());
    resultWriter_->write(resultWriter_, buffer_.data(), buffer_.size());
    resultWriter_->flush(resultWriter_);
    dataProto.clear_fps_event();

    HILOG_INFO(LOG_CORE, "HidumpPlugin: Loop exit");
}

bool HidumpPlugin::ParseHidumpInfo(HidumpInfo& dataProto, char *buf)
{
    // format: fps:123|1501960484673
    if (strncmp(buf, "fps:", strlen("fps:")) != 0) {
        if (strstr(buf, "inaccessible or not found") != nullptr) {
            HILOG_ERROR(LOG_CORE, "HidumpPlugin: fps command not found!");
        } else {
            HILOG_ERROR(LOG_CORE, "format error. %s", buf);
        }
        return false;
    }

    buf += strlen("fps:");
    char *tmp = strchr(buf, '|');
    CHECK_NOTNULL(tmp, false, "format error. %s", buf);
    *tmp = ' ';
    std::stringstream strvalue(buf);
    uint32_t fps = 0;
    strvalue >> fps;
    uint64_t time_ms;
    strvalue >> time_ms;

    auto* eve = dataProto.add_fps_event();
    eve->set_fps(fps);
    eve->set_id(::FpsData::REALTIME);
    eve->mutable_time()->set_tv_sec(time_ms / MS_PER_S);
    eve->mutable_time()->set_tv_nsec((time_ms % MS_PER_S) * US_PER_S);

    return true;
}

FILE* HidumpPlugin::CustomPopen(const char* command, const char* type)
{
    CHECK_TRUE(command != nullptr && type != nullptr, nullptr, "HidumpPlugin:%s param invalid", __func__);

    int fd[PIPE_LEN];
    pipe(fd);

    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(1);
    }

    // child process
    if (pid == 0) {
        if (!strncmp(type, "r", strlen(type))) {
            close(fd[READ]);
            dup2(fd[WRITE], REDIRECT_STDOUT); // Redirect stdout to pipe
            dup2(fd[WRITE], REDIRECT_STDERR); // Redirect stderr to pipe
        } else {
            close(fd[WRITE]);
            dup2(fd[READ], 0); // Redirect stdin to pipe
        }

        setpgid(pid, pid);
        execl(BIN_COMMAND.c_str(), BIN_COMMAND.c_str(), "-c", command, nullptr);
        exit(0);
    } else {
        if (!strncmp(type, "r", strlen(type))) {
            // Close the WRITE end of the pipe since parent's fd is read-only
            close(fd[WRITE]);
        } else {
            // Close the READ end of the pipe since parent's fd is write-only
            close(fd[READ]);
        }
    }

    g_child = pid;

    if (!strncmp(type, "r", strlen(type))) {
        return fdopen(fd[READ], "r");
    }

    return fdopen(fd[WRITE], "w");
}

int HidumpPlugin::CustomPclose(FILE* fp)
{
    CHECK_NOTNULL(fp, -1, "HidumpPlugin:%s fp is null", __func__);
    int stat;
    if (fclose(fp) != 0) {
        const int bufSize = 256;
        char buf[bufSize] = { 0 };
        strerror_r(errno, buf, bufSize);
        HILOG_ERROR(LOG_CORE, "HidumpPlugin:%s fclose failed! errno(%d:%s)", __func__, errno, buf);
        return -1;
    }
    kill(g_child, SIGKILL);
    if (waitpid(g_child, &stat, 0) == -1) {
        if (errno != EINTR) {
            stat = errno;
        }
    }
    return stat;
}

void HidumpPlugin::SetConfig(HidumpConfig& config)
{
    protoConfig_ = config;
}

int HidumpPlugin::SetTestCmd(const char *test_cmd)
{
    CHECK_NOTNULL(test_cmd, -1, "HidumpPlugin:%s test_cmd is null", __func__);
    testCmd_ = const_cast<char *>(test_cmd);
    return 0;
}

const char *HidumpPlugin::GetTestCmd(void)
{
    return testCmd_;
}
