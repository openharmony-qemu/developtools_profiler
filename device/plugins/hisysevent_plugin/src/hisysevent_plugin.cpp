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
constexpr int PIPE_SIZE = 256 * 1024;
constexpr int MAX_STRING_LEN = 256 * 1024;
constexpr int MIN_STRING_LEN = 10;
constexpr int BYTE_BUFFER_SIZE = 1024;
} // namespace

HisyseventPlugin::HisyseventPlugin() : fp_(nullptr, nullptr) {}

HisyseventPlugin::~HisyseventPlugin()
{
    HILOG_INFO(LOG_CORE, "BEGN %s: ready!", __func__);
    Stop();
    HILOG_INFO(LOG_CORE, "END %s: success!", __func__);
}

int HisyseventPlugin::SetWriter(WriterStruct* writer)
{
    resultWriter_ = writer;

    HILOG_INFO(LOG_CORE, "END %s: success!", __func__);
    return 0;
}

int HisyseventPlugin::Start(const uint8_t* configData, uint32_t configSize)
{
    HILOG_INFO(LOG_CORE, "BEGN %s: ready!", __func__);
    CHECK_NOTNULL(configData, -1, "NOTE %s: param invalid", __func__);

    CHECK_TRUE(protoConfig_.ParseFromArray(configData, configSize) > 0, -1,
               "NOTE HisyseventPlugin: ParseFromArray failed");

    HILOG_DEBUG(LOG_CORE, "NOTE configData ParseFromArray sucessed,sourse data:%s", protoConfig_.msg().c_str());

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
    workThread_ = std::thread(&HisyseventPlugin::Run, this);

    HILOG_INFO(LOG_CORE, "END %s: success!", __func__);
    return 0;
}

int HisyseventPlugin::Stop()
{
    HILOG_INFO(LOG_CORE, "BEGN %s: ready!", __func__);
    running_ = false;
    COMMON::CustomPUnblock(pipeFds_);

    if (workThread_.joinable()) {
        workThread_.join();
    }

    if (fp_ != nullptr) {
        fp_.reset();
    }

    HILOG_INFO(LOG_CORE, "END %s: success!", __func__);
    return 0;
}

void HisyseventPlugin::Run(void)
{
    HILOG_INFO(LOG_CORE, "BEGN %s: ready!", __func__);
    std::unique_ptr<uint8_t[]> buffer = std::make_unique<uint8_t[]>(MAX_STRING_LEN);

    HILOG_INFO(LOG_CORE, "NOTE hisysevent_plugin_result.proto->HisyseventInfo:dataProto;Ready to output the result!");
    HisyseventInfo dataProto;

    fcntl(fileno(fp_.get()), F_SETPIPE_SZ, PIPE_SIZE);
    int aPipeSize = fcntl(fileno(fp_.get()), F_GETPIPE_SZ);
    HILOG_INFO(LOG_CORE, "{fp = %d, aPipeSize=%d, PIPE_SIZE=%d}", fileno(fp_.get()), aPipeSize, PIPE_SIZE);

    while (running_) {
        if (fgets(reinterpret_cast<char*>(buffer.get()), MAX_STRING_LEN, fp_.get()) != nullptr) {
            auto cptr = reinterpret_cast<char*>(buffer.get());
            if (!ParseSyseventLineInfo(cptr, strlen(cptr), dataProto))
                continue;
        }

        if (dataProto.ByteSizeLong() >= BYTE_BUFFER_SIZE) {
            WriteResult(dataProto);
            dataProto.clear_info();
        }
    }

    WriteResult(dataProto);
    dataProto.clear_info();

    HILOG_INFO(LOG_CORE, "END %s: success!", __func__);
}

std::string HisyseventPlugin::GetFullCmd()
{
    std::string cmd;

    if (!fullCmd_.empty()) {
        size_t i = 0;
        for (size_t cmdSize = fullCmd_.size() - 1; i < cmdSize; i++) {
            cmd.append(fullCmd_[i]).append(" ");
        }
        cmd.append(fullCmd_[i]);
    }
    return cmd;
}

inline bool HisyseventPlugin::InitHisyseventCmd()
{
    HILOG_INFO(LOG_CORE, "BEGN %s: ready!", __func__);
    if (!fullCmd_.empty()) {
        HILOG_INFO(LOG_CORE, "fullCmd_ is dirty.Then clear().");
        fullCmd_.clear();
    }

    fullCmd_.emplace_back("/bin/hisysevent"); // exe file path
    fullCmd_.emplace_back("hisysevent"); // exe file name
    fullCmd_.emplace_back("-rd");

    HILOG_INFO(LOG_CORE, "END %s: success!", __func__);
    return true;
}

inline bool HisyseventPlugin::ParseSyseventLineInfo(const char* data, size_t len, HisyseventInfo& dataProto)
{
    CHECK_TRUE(data != nullptr && len >= MIN_STRING_LEN, false, "NOTE %s: param invalid", __func__);

    if (google::protobuf::internal::IsStructurallyValidUTF8(data, strlen(data) - 1)) {
        auto* info = dataProto.add_info();
        info->set_id(id_);
        info->set_context(data, strlen(data) - 1); // - \n
        id_++;
    } else {
        HILOG_ERROR(LOG_CORE, "NOTE HisyseventPlugin: hisysevent context include invalid UTF-8 data");
        return false;
    }

    return true;
}

inline bool HisyseventPlugin::WriteResult(const HisyseventInfo& dataProto)
{
    // Cmd result resize and SerializeToArray and after save to protoBuffer_ ;Then write and flush;Then clear_info
    protoBuffer_.resize(dataProto.ByteSizeLong());
    dataProto.SerializeToArray(protoBuffer_.data(), protoBuffer_.size());
    // SerializeToArray after data=%s",protoBuffer_.data()
    resultWriter_->write(resultWriter_, protoBuffer_.data(), protoBuffer_.size());
    resultWriter_->flush(resultWriter_);
    return true;
}
