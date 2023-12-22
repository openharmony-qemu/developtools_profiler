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

#include <hwext/gtest-ext.h>
#include <hwext/gtest-tag.h>
#include <dlfcn.h>
#include <fstream>
#include "gpu_data_plugin.h"
#include "plugin_module_api.h"

using namespace testing::ext;

namespace {
const std::string DEFAULT_TEST_PATH = "/data/local/tmp/";
#if defined(__LP64__)
const std::string SO_PATH = "/system/lib64/libgpudataplugin.z.so";
#else
const std::string SO_PATH = "/system/lib/libgpudataplugin.z.so";
#endif
const std::string DEFAULT_BIN_PATH("/data/local/tmp/gpudataplugintest");
constexpr uint32_t BUF_SIZE = 4 * 1024 * 1024;

std::string g_path;
std::string g_testPath;

struct TestVmstat {
    int64_t pgpgin;
    int64_t pgpgout;
};

class GpuDataPluginTest : public ::testing::Test {
public:
    static void SetUpTestCase() {}

    static void TearDownTestCase()
    {
        if (access(g_testPath.c_str(), F_OK) == 0) {
            std::string str = "rm -rf " + g_testPath;
            system(str.c_str());
        }
    }
};

string Getexepath()
{
    char buf[PATH_MAX] = "";
    std::string path = "/proc/self/exe";
    size_t rslt = readlink(path.c_str(), buf, sizeof(buf));
    if (rslt < 0 || (rslt >= sizeof(buf))) {
        return "";
    }
    buf[rslt] = '\0';
    for (int i = rslt; i >= 0; i--) {
        if (buf[i] == '/') {
            buf[i + 1] = '\0';
            break;
        }
    }
    return buf;
}

std::string GetFullPath(std::string path)
{
    if (path.size() > 0 && path[0] != '/') {
        return Getexepath() + path;
    }
    return path;
}

/**
 * @tc.name: gpu plugin
 * @tc.desc: Test whether the path exists.
 * @tc.type: FUNC
 */
HWTEST_F(GpuDataPluginTest, TestPath, TestSize.Level1)
{
    g_path = GetFullPath(DEFAULT_TEST_PATH);
    g_testPath = g_path;
    EXPECT_NE("", g_path);
    g_path += "/gpustat1.txt";
}

/**
 * @tc.name: gpu plugin
 * @tc.desc: gpu plugin registration test.
 * @tc.type: FUNC
 */
HWTEST_F(GpuDataPluginTest, TestPluginRegister, TestSize.Level1)
{
    void* handle = dlopen(SO_PATH.c_str(), RTLD_LAZY);
    ASSERT_NE(handle, nullptr);
    PluginModuleStruct* gpuPlugin = (PluginModuleStruct*)dlsym(handle, "g_pluginModule");
    ASSERT_NE(gpuPlugin, nullptr);
    EXPECT_STREQ(gpuPlugin->name, "gpu-plugin");
    EXPECT_EQ(gpuPlugin->resultBufferSizeHint, BUF_SIZE);

    // Serialize config
    GpuConfig protoConfig;
    int configLength = protoConfig.ByteSizeLong();
    ASSERT_EQ(configLength, 0);
    std::vector<uint8_t> configBuffer(configLength);
    EXPECT_TRUE(protoConfig.SerializeToArray(configBuffer.data(), configLength));

    // run plugin
    std::vector<uint8_t> dataBuffer(gpuPlugin->resultBufferSizeHint);
    EXPECT_EQ(gpuPlugin->callbacks->onPluginSessionStart(configBuffer.data(), configLength), RET_SUCC);
    ASSERT_GT(gpuPlugin->callbacks->onPluginReportResult(dataBuffer.data(), gpuPlugin->resultBufferSizeHint), 0);
    EXPECT_EQ(gpuPlugin->callbacks->onPluginSessionStop(), RET_SUCC);

    // 反序列化失败导致的start失败
    configLength++;
    std::vector<uint8_t> configBuffer2(configLength);
    EXPECT_TRUE(protoConfig.SerializeToArray(configBuffer2.data(), configLength));
    EXPECT_EQ(gpuPlugin->callbacks->onPluginSessionStart(configBuffer2.data(), configLength+1), RET_FAIL);
}


} // namespace
