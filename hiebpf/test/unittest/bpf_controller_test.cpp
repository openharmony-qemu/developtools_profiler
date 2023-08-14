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
#include <string>
#include <thread>

#include <gtest/gtest.h>

#include "bpf_controller.h"

using namespace testing::ext;

namespace {
constexpr int ROUND_COUNT = 1000;
constexpr int BUF_SIZE = 512;
const std::string FILE_NAME = "/data/local/tmp/hiebpf.txt";
const std::string HIEBPF_FILE_NAME = "/data/local/tmp/hiebpf.data";
constexpr int FILE_MODE = 0644;
} // namespace

namespace OHOS {
namespace Developtools {
namespace Hiebpf {
class BpfControllerTest : public ::testing::Test {
public:
    static void SetUpTestCase() {};
    static void TearDownTestCase()
    {
        if (access(FILE_NAME.c_str(), F_OK) == 0) {
            std::string cmd = "rm " + FILE_NAME;
            system(cmd.c_str());
        }
        if (access(HIEBPF_FILE_NAME.c_str(), F_OK) == 0) {
            std::string cmd = "rm " + HIEBPF_FILE_NAME;
            system(cmd.c_str());
        }
    }

    void SetUp() {}
    void TearDown() {}
};

/**
 * @tc.name: Normal
 * @tc.desc: Test framework
 * @tc.type: FUNC
 */
HWTEST_F(BpfControllerTest, Normal, TestSize.Level1)
{
    BPFConfig cfg;
    cfg.selectEventGroups_.insert(HiebpfEventGroup::FS_GROUP_ALL);
    cfg.selectEventGroups_.insert(HiebpfEventGroup::MEM_GROUP_ALL);
    cfg.selectEventGroups_.insert(HiebpfEventGroup::BIO_GROUP_ALL);
    cfg.LIBBPFLogLevel_ = LIBBPF_NONE;
    cfg.BPFLogLevel_ = BPF_LOG_NONE;
    const uint32_t duration = 10;
    cfg.traceDuration_ = duration;
    std::unique_ptr<BPFController> pCtx = BPFController::MakeUnique(cfg);
    ASSERT_TRUE(pCtx != nullptr);
    std::thread threadContol([&pCtx]() {
        ASSERT_EQ(pCtx->Start(), 0);
    });
    sleep(1);

    // create fs/bio data
    int fd = open(FILE_NAME.c_str(), O_RDWR | O_CREAT, FILE_MODE);
    ASSERT_GT(fd, 0);
    char buf[BUF_SIZE] = {0};
    ASSERT_TRUE(memset_s(buf, sizeof(buf), '1', sizeof(buf)) == EOK);
    off_t offset = 0;
    for (int i = 0; i < ROUND_COUNT; i++) {
        pwrite(fd, buf, sizeof(buf), offset);
        fsync(fd);
        pread(fd, buf, sizeof(buf), offset);
        offset += sizeof(buf);
    }
    close(fd);

    // create mem data
    int num = 1;
    pid_t pid = fork();
    if (pid == 0) {
        num++;
        exit(0);
    }

    sleep(1);
    pCtx->Stop();
    if (threadContol.joinable()) {
        threadContol.join();
    }
}
} // namespace Hiebpf
} // namespace Developtools
} // namespace OHOS
