/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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
#include <iostream>
#include <sstream>
#include <thread>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "include/sp_utils.h"
#include "include/Capture.h"
#include "display_manager.h"
#include "wm_common.h"
#include "png.h"
namespace OHOS {
namespace SmartPerf {
using namespace OHOS::Media;
using namespace OHOS::Rosen;
std::map<std::string, std::string> Capture::ItemData()
{
    std::map<std::string, std::string> result;
    long long curTime = SPUtils::GetCurTime();
    std::string path = "data/local/tmp/capture/screenCap_" + std::to_string(curTime) + ".png";
    result["capture"] = path;
    Capture::TriggerGetCatch(curTime);
    return result;
}
void Capture::ThreadGetCatch(const std::string &curTime) const
{
    auto savePath = "data/local/tmp/capture/screenCap_" + curTime + ".png";
    std::string cmdResult;
    if (!SPUtils::FileAccess("/data/local/tmp/capture")) {
        SPUtils::LoadCmd("mkdir /data/local/tmp/capture", cmdResult);
        printf("/data/local/tmp/capture created! \n");
    };
    std::stringstream errorRecv;
    auto fd = open(savePath.c_str(), O_RDWR | O_CREAT, 0666);
    if (!TakeScreenCap(savePath)) {
        std::cout << "Screen Capture Failed：---"<< errorRecv.str() << std::endl;
    }
    (void) close(fd);
}
void Capture::TriggerGetCatch(long long curTime) const
{
    std::string curTimeStr = std::to_string(curTime);
    std::thread tStart(&Capture::ThreadGetCatch, this, curTimeStr);
    tStart.detach();
}
bool Capture::TakeScreenCap(const std::string &savePath) const
{
    // get PixelMap from DisplayManager API
    Rosen::DisplayManager &displayMgr = Rosen::DisplayManager::GetInstance();
    std::shared_ptr<Media::PixelMap> pixelMap = displayMgr.GetScreenshot(displayMgr.GetDefaultDisplayId());
    static constexpr int bitmapDepth = 8;
    if (pixelMap == nullptr) {
        std::cout << "Failed to get display pixelMap" << std::endl;
        return false;
    }
    auto width = static_cast<uint32_t>(pixelMap->GetWidth());
    auto height = static_cast<uint32_t>(pixelMap->GetHeight());
    auto data = pixelMap->GetPixels();
    auto stride = static_cast<uint32_t>(pixelMap->GetRowBytes());
    png_structp pngStruct = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    if (pngStruct == nullptr) {
        std::cout << "error: png_create_write_struct nullptr!" << std::endl;
        return false;
    }
    png_infop pngInfo = png_create_info_struct(pngStruct);
    if (pngInfo == nullptr) {
        std::cout << "error: png_create_info_struct error nullptr!" << std::endl;
        png_destroy_write_struct(&pngStruct, nullptr);
        return false;
    }
    FILE *fp = fopen(savePath.c_str(), "wb");
    if (fp == nullptr) {
        std::cout << "error: open file error!" << std::endl;
        png_destroy_write_struct(&pngStruct, &pngInfo);
        return false;
    }
    png_init_io(pngStruct, fp);
    png_set_IHDR(pngStruct, pngInfo, width, height, bitmapDepth, PNG_COLOR_TYPE_RGBA, PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
    png_set_packing(pngStruct);          // set packing info
    png_write_info(pngStruct, pngInfo);  // write to header
    for (uint32_t i = 0; i < height; i++) {
        png_write_row(pngStruct, data + (i * stride));
    }
    png_write_end(pngStruct, pngInfo);
    // free
    png_destroy_write_struct(&pngStruct, &pngInfo);
    (void)fclose(fp);
    return true;
}
}
}
