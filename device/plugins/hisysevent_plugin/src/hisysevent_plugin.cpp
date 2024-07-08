/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022. All rights reserved.
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
#include "hisysevent_plugin.h"
#include "hisysevent_plugin_result.pbencoder.h"

#include <cinttypes>
#include <csignal>
#include <cstdio>
#include <fcntl.h>
#include <sstream>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

namespace {
using namespace OHOS::Developtools::Profiler;
constexpr int PIPE_SIZE = 256 * 1024;
constexpr int MAX_STRING_LEN = 256 * 1024;
constexpr int MIN_STRING_LEN = 10;
constexpr int BYTE_BUFFER_SIZE = 1024;
} // namespace

HisyseventPlugin::HisyseventPlugin() : fp_(nullptr, nullptr) {}

HisyseventPlugin::~HisyseventPlugin()
{
    PROFILER_LOG_INFO(LOG_CORE, "BEGN %s: ready!", __func__);
    Stop();
    PROFILER_LOG_INFO(LOG_CORE, "END %s: success!", __func__);
}

int HisyseventPlugin::SetWriter(WriterStruct* writer)
{
    resultWriter_ = writer;

    PROFILER_LOG_INFO(LOG_CORE, "END %s: success!", __func__);
    return 0;
}

int HisyseventPlugin::Start(const uint8_t* configData, uint32_t configSize)
{
    PROFILER_LOG_INFO(LOG_CORE, "BEGN %s: ready!", __func__);
    CHECK_NOTNULL(configData, -1, "NOTE %s: param invalid", __func__);

    CHECK_TRUE(protoConfig_.ParseFromArray(configData, configSize) > 0, -1,
               "NOTE HisyseventPlugin: ParseFromArray failed");

    PROFILER_LOG_DEBUG(LOG_CORE, "config sourse data:%s domain:%s event:%s", protoConfig_.msg().c_str(),
        protoConfig_.subscribe_domain().c_str(), protoConfig_.subscribe_event().c_str());

    CHECK_TRUE(InitHisyseventCmd(), -1, "HisyseventPlugin: Init HisyseventCmd failed");

    fp_ = std::unique_ptr<FILE, std::function<int (FILE*)>>(
        COMMON::CustomPopen(fullCmd_, "r", pipeFds_, childPid_, true), [this](FILE* fp) -> int {
            return COMMON::CustomPclose(fp, pipeFds_, childPid_, true);
        });

    CHECK_NOTNULL(fp_.get(), -1, "HisyseventPlugin: fullCmd_ Failed, errno(%d)", errno);
    CHECK_NOTNULL(resultWriter_, -1, "HisyseventPlugin: Writer is no set!!");
    CHECK_NOTNULL(resultWriter_->write, -1, "HisyseventPlugin: Writer.write is no set!!");
    CHECK_NOTNULL(resultWriter_->flush, -1, "HisyseventPlugin: Writer.flush is no set!!");
    id_ = 1;
    running_ = true;
    workThread_ = std::thread([this] { this->Run(); });

    PROFILER_LOG_INFO(LOG_CORE, "END %s: success!", __func__);
    return 0;
}

int HisyseventPlugin::Stop()
{
    PROFILER_LOG_INFO(LOG_CORE, "BEGN %s: ready!", __func__);
    running_ = false;
    COMMON::CustomPUnblock(pipeFds_);

    if (workThread_.joinable()) {
        workThread_.join();
    }

    if (fp_ != nullptr) {
        fp_.reset();
    }

    PROFILER_LOG_INFO(LOG_CORE, "END %s: success!", __func__);
    return 0;
}

void HisyseventPlugin::Run(void)
{
    PROFILER_LOG_INFO(LOG_CORE, "BEGN %s: ready!", __func__);
    std::unique_ptr<uint8_t[]> buffer = std::make_unique<uint8_t[]>(MAX_STRING_LEN);

    PROFILER_LOG_INFO(LOG_CORE,
                      "NOTE hisysevent_plugin_result.proto->HisyseventInfo:dataProto;Ready to output the result!");

    fcntl(fileno(fp_.get()), F_SETPIPE_SZ, PIPE_SIZE);
    int aPipeSize = fcntl(fileno(fp_.get()), F_GETPIPE_SZ);
    PROFILER_LOG_INFO(LOG_CORE, "{fp = %d, aPipeSize=%d, PIPE_SIZE=%d}", fileno(fp_.get()), aPipeSize, PIPE_SIZE);

    std::unique_ptr<HisyseventInfo> dataProto = nullptr;
    std::unique_ptr<ProtoEncoder::HisyseventInfo> hisyseventInfo = nullptr;
    if (resultWriter_->isProtobufSerialize) {
        dataProto = std::make_unique<HisyseventInfo>();
    } else {
        hisyseventInfo = std::make_unique<ProtoEncoder::HisyseventInfo>(resultWriter_->startReport(resultWriter_));
    }

    while (running_) {
        char* cptr = nullptr;
        if (fgets(reinterpret_cast<char*>(buffer.get()), MAX_STRING_LEN, fp_.get()) != nullptr) {
            cptr = reinterpret_cast<char*>(buffer.get());
        }
        if (resultWriter_->isProtobufSerialize) {
            if (!ParseSyseventLineInfo(cptr, strlen(cptr), dataProto.get())) {
                continue;
            }

            if (dataProto->ByteSizeLong() >= BYTE_BUFFER_SIZE) {
                WriteResult(dataProto.get());
                dataProto->clear_info();
            }
        } else {
            if (!ParseSyseventLineInfo(cptr, strlen(cptr), hisyseventInfo.get())) {
                continue;
            }

            if (hisyseventInfo->Size() >= BYTE_BUFFER_SIZE) {
                FlushDataOptimize(hisyseventInfo.get());
                hisyseventInfo.reset();
                hisyseventInfo =
                    std::make_unique<ProtoEncoder::HisyseventInfo>(resultWriter_->startReport(resultWriter_));
            }
        }
    }

    if (resultWriter_->isProtobufSerialize) {
        WriteResult(dataProto.get());
        dataProto.reset();
    } else {
        FlushDataOptimize(hisyseventInfo.get());
        hisyseventInfo.reset();
    }

    PROFILER_LOG_INFO(LOG_CORE, "END %s: success!", __func__);
}

std::string HisyseventPlugin::GetFullCmd()
{
    std::string cmd;

    if (!fullCmd_.empty()) {
        size_t i = 0;
        size_t dataLen = fullCmd_.size() > 1 ? fullCmd_.size() - 1 : 0;
        for (size_t cmdSize = dataLen; i < cmdSize; i++) {
            cmd.append(fullCmd_[i]).append(" ");
        }
        cmd.append(fullCmd_[i]);
    }
    return cmd;
}

inline bool HisyseventPlugin::InitHisyseventCmd()
{
    PROFILER_LOG_INFO(LOG_CORE, "BEGN %s: ready!", __func__);
    if (!fullCmd_.empty()) {
        PROFILER_LOG_INFO(LOG_CORE, "fullCmd_ is dirty.Then clear().");
        fullCmd_.clear();
    }

    fullCmd_.emplace_back("/bin/hisysevent"); // exe file path
    fullCmd_.emplace_back("hisysevent"); // exe file name
    fullCmd_.emplace_back("-rd");

    if (!protoConfig_.subscribe_domain().empty()) {
        fullCmd_.emplace_back("-o");
        fullCmd_.emplace_back(protoConfig_.subscribe_domain());
    }
    if (!protoConfig_.subscribe_event().empty()) {
        fullCmd_.emplace_back("-n");
        fullCmd_.emplace_back(protoConfig_.subscribe_event());
    }
    PROFILER_LOG_INFO(LOG_CORE, "END %s: success!", __func__);
    return true;
}

template <typename T>
inline bool HisyseventPlugin::ParseSyseventLineInfo(const char* data, size_t len, T hisyseventInfoProto)
{
    CHECK_TRUE(data != nullptr && len >= MIN_STRING_LEN, false, "NOTE %s: param invalid", __func__);
    size_t dataLen = strlen(data) > 1 ? strlen(data) - 1 : 0;
    if (google::protobuf::internal::IsStructurallyValidUTF8(data, dataLen)) {
        auto* info = hisyseventInfoProto->add_info();
        info->set_id(id_);
        size_t len = strlen(data) > 1 ? strlen(data) - 1 : 0;
        info->set_context(data, len); // - \n
        id_++;
    } else {
        PROFILER_LOG_ERROR(LOG_CORE, "NOTE HisyseventPlugin: hisysevent context include invalid UTF-8 data");
        return false;
    }
    return true;
}

template <typename T> inline bool HisyseventPlugin::WriteResult(const T hisyseventInfoProto)
{
    // Cmd result resize and SerializeToArray and after save to protoBuffer_ ;Then write and flush;Then clear_info
    protoBuffer_.resize(hisyseventInfoProto->ByteSizeLong());
    hisyseventInfoProto->SerializeToArray(protoBuffer_.data(), protoBuffer_.size());
    // SerializeToArray after data=%s",protoBuffer_.data()
    resultWriter_->write(resultWriter_, protoBuffer_.data(), protoBuffer_.size());
    resultWriter_->flush(resultWriter_);
    return true;
}

template <typename T> void HisyseventPlugin::FlushDataOptimize(const T hisyseventInfoProto)
{
    int messageLen = hisyseventInfoProto->Finish();
    resultWriter_->finishReport(resultWriter_, messageLen);
    resultWriter_->flush(resultWriter_);
}
