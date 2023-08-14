/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021. All rights reserved.
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
#include <memory>

#include "common.h"
#include "grpc/impl/codegen/log.h"
#include "grpcpp/health_check_service_interface.h"
#include "logging.h"
#include "plugin_service.h"
#include "profiler_service.h"

namespace {
const std::string DEFAULT_SERVICE_LISTEN_URI = "0.0.0.0:";
}

int main(int argc, char* argv[])
{
    int lockFileFd = -1;
    if (COMMON::IsProcessRunning(lockFileFd)) { // process is running
        return 0;
    }

    gpr_set_log_verbosity(GPR_LOG_SEVERITY_DEBUG);
    grpc::EnableDefaultHealthCheckService(true);

    auto service = std::make_shared<ProfilerService>(std::make_shared<PluginService>());
    CHECK_NOTNULL(service, -1, "ProfilerService create failed!");

    std::string listenUri = DEFAULT_SERVICE_LISTEN_URI + std::to_string(COMMON::GetServicePort());
    if (argc > 1) {
        listenUri = argv[1];
    }
    if (service->StartService(listenUri)) {
        HILOG_INFO(LOG_CORE, "ProfilerService start SUCCESS!");
        service->WaitServiceDone();
    }
    close(lockFileFd);
    return 0;
}
