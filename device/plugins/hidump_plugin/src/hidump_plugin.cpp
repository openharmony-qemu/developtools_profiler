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

#include "common.h"
#include "hidump_plugin_result.pbencoder.h"
#include "hisysevent.h"
#include "securec.h"

namespace {
using namespace OHOS::Developtools::Profiler;
const int SLEEP_TIME = 50;
const int BUF_MAX_LEN = 64;
const int MS_PER_S = 1000;
const int US_PER_S = 1000000;
const char *FPS_FORMAT = "SP_daemon -profilerfps 31104000 -sections 10";

} // namespace

HidumpPlugin::HidumpPlugin() : fp_(nullptr, nullptr) {}

HidumpPlugin::~HidumpPlugin()
{
    PROFILER_LOG_INFO(LOG_CORE, "%s: ready!", __func__);
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
    PROFILER_LOG_INFO(LOG_CORE, "%s: success!", __func__);
}

std::string HidumpPlugin::GetCmdArgs(const HidumpConfig& protoConfig)
{
    std::string args;
    args += "sections: " + std::to_string(protoConfig.sections()) + ", report_fps: ";
    args += (protoConfig.report_fps() ? "true" : "false");
    return args;
}

int HidumpPlugin::Start(const uint8_t* configData, uint32_t configSize)
{
    PROFILER_LOG_INFO(LOG_CORE, "HidumpPlugin:Start ----> !");
    CHECK_TRUE(protoConfig_.ParseFromArray(configData, configSize) > 0, -1, "HidumpPlugin: ParseFromArray failed");
    std::vector<std::string> fullCmd;
    fullCmd.push_back("/system/bin/SP_daemon");
    fullCmd.push_back("SP_daemon");
    fullCmd.push_back("-profilerfps");
    fullCmd.push_back("31104000");
    fullCmd.push_back("-sections");
    fullCmd.push_back(std::to_string(protoConfig_.sections()));
    fp_ = std::unique_ptr<FILE, std::function<int (FILE*)>>(
        COMMON::CustomPopen(fullCmd, "r", pipeFds_, childPid_, true), [this](FILE* fp) -> int {
            return COMMON::CustomPclose(fp, pipeFds_, childPid_, true);
        });
    auto args = GetCmdArgs(protoConfig_);
    if (fp_.get() == nullptr) {
        const int bufSize = 256;
        char buf[bufSize] = {0};
        strerror_r(errno, buf, bufSize);
        COMMON::PluginWriteToHisysevent("hidump_plugin", "sh", args, COMMON::ErrorType::RET_FAIL, "failed");
        PROFILER_LOG_ERROR(LOG_CORE, "HidumpPlugin: CustomPopen(%s) Failed, errno(%d:%s)", FPS_FORMAT, errno, buf);
        return -1;
    }
    CHECK_NOTNULL(resultWriter_, -1, "HidumpPlugin: Writer is no set!");
    CHECK_NOTNULL(resultWriter_->write, -1, "HidumpPlugin: Writer.write is no set!");
    CHECK_NOTNULL(resultWriter_->flush, -1, "HidumpPlugin: Writer.flush is no set!");
    std::unique_lock<std::mutex> locker(mutex_);
    running_ = true;
    writeThread_ = std::thread([this] { this->Loop(); });
    int ret = COMMON::PluginWriteToHisysevent("hidump_plugin", "sh", args, COMMON::ErrorType::RET_SUCC, "success");
    PROFILER_LOG_INFO(LOG_CORE, "HidumpPlugin--> Start success! hisysevent report hidump_plugin result:%d", ret);
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
    PROFILER_LOG_INFO(LOG_CORE, "HidumpPlugin:stop thread success!");
    if (fp_ != nullptr) {
        fp_.reset();
    }
    PROFILER_LOG_INFO(LOG_CORE, "HidumpPlugin: stop success!");
    return 0;
}

int HidumpPlugin::SetWriter(WriterStruct* writer)
{
    resultWriter_ = writer;
    return 0;
}

void HidumpPlugin::Loop(void)
{
    PROFILER_LOG_INFO(LOG_CORE, "HidumpPlugin: Loop start");
    CHECK_NOTNULL(resultWriter_, NO_RETVAL, "%s: resultWriter_ nullptr", __func__);

    fcntl(fileno(fp_.get()), F_SETFL, O_NONBLOCK);
    while (running_) {
        char buf[BUF_MAX_LEN] = { 0 };

        if (fgets(buf, BUF_MAX_LEN - 1, fp_.get()) == nullptr) {
            std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_TIME));
            continue;
        }
        char* pTempBuf = buf;
        if (strncmp(pTempBuf, "fps:", strlen("fps:")) == 0) {
            pTempBuf += strlen("fps:");
            std::string stringBuf(pTempBuf);
            size_t npos = stringBuf.find("|");
            uint32_t fps = static_cast<uint32_t>(std::stoi(stringBuf.substr(0, npos)));
            if (fps > 0) {
                continue;
            }
        }
        if (resultWriter_->isProtobufSerialize) {
            HidumpInfo dataProto;
            if (!ParseHidumpInfo(dataProto, buf, sizeof(buf))) {
                continue;
            }
            if (dataProto.ByteSizeLong() > 0) {
                buffer_.resize(dataProto.ByteSizeLong());
                dataProto.SerializeToArray(buffer_.data(), buffer_.size());
                resultWriter_->write(resultWriter_, buffer_.data(), buffer_.size());
                resultWriter_->flush(resultWriter_);
                if (!dataReady_) {
                    dataReady_ = true;
                }
            }
        } else {
            ProtoEncoder::HidumpInfo hidumpInfo(resultWriter_->startReport(resultWriter_));
            if (!ParseHidumpInfo(hidumpInfo, buf, sizeof(buf))) {
                PROFILER_LOG_ERROR(LOG_CORE, "parse hidump info failed!");
            }
            int messageLen = hidumpInfo.Finish();
            if (!dataReady_ && messageLen > 0) {
                dataReady_ = true;
            }
            resultWriter_->finishReport(resultWriter_, messageLen);
            resultWriter_->flush(resultWriter_);
        }
    }

    PROFILER_LOG_INFO(LOG_CORE, "HidumpPlugin: Loop exit");
}

template <typename T>
bool HidumpPlugin::ParseHidumpInfo(T& hidumpInfoProto, char *buf, size_t len)
{
    UNUSED_PARAMETER(len);
    // format: fps:123|1501960484673
    if (strncmp(buf, "fps:", strlen("fps:")) != 0 && strncmp(buf, "sectionsFps:", strlen("sectionsFps:")) != 0) {
        if (strstr(buf, "inaccessible or not found") != nullptr) {
            PROFILER_LOG_ERROR(LOG_CORE, "HidumpPlugin: fps command not found!");
        } else {
            PROFILER_LOG_ERROR(LOG_CORE, "format error. %s", buf);
        }
        return false;
    }

    if (strncmp(buf, "fps:", strlen("fps:")) == 0) {
        buf += strlen("fps:");
    } else if (strncmp(buf, "sectionsFps:", strlen("sectionsFps:")) == 0) {
        buf += strlen("sectionsFps:");
    }

    char *tmp = strchr(buf, '|');
    CHECK_NOTNULL(tmp, false, "format error. %s", buf);
    *tmp = ' ';
    std::stringstream strvalue(buf);
    uint32_t fps = 0;
    strvalue >> fps;
    uint64_t time_ms;
    strvalue >> time_ms;

    auto* eve = hidumpInfoProto.add_fps_event();
    eve->set_fps(fps);
    eve->set_id(::FpsData::REALTIME);
    auto* time = eve->mutable_time();
    time->set_tv_sec(time_ms / MS_PER_S);
    time->set_tv_nsec((time_ms % MS_PER_S) * US_PER_S);

    return true;
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
