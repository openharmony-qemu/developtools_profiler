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
#include "include/sp_parse_fps.h"
ParseFPS::ParseFPS()
{
    pattern = std::regex("(\\d+).(\\d{6})");
    pidPattern = std::regex("\\|(\\d+)\\|");
}
ParseFPS::~ParseFPS()
{
}
void ParseFPS::StrSplit(const SpString &content, const SpString &sp, std::vector<SpString> &out)
{
    size_t index = 0;
    while (index != SpString::npos) {
        size_t tEnd = content.find_first_of(sp, index);
        SpString tmp = content.substr(index, tEnd - index);
        if (tmp != "" && tmp != " ") {
            out.push_back(tmp);
        }
        if (tEnd == SpString::npos) {
            break;
        }
        index = tEnd + 1;
    }
}
void ParseFPS::GetAndSetPageType(Line& line, PageType& pageType)
{
    if (line.empty()) {
        return;
    }
    if (line.find(ROSENRENDERWEB) != SpString::npos) {
        pageType = WEB;
    } else if (line.find(ROSENRENDERTEXTURE) != SpString::npos) {
        pageType = VIDEO;
    } else {
        pageType = LARGE;
    }
}
unsigned int ParseFPS::GetTouchEventNum(Line& line, TouchEvent& touchEvent)
{
    if (line.empty()) {
        return 0;
    }
    if (line.find(TOUCHEVENT_FLAG) != SpString::npos || line.find(HTOUCHEVENT_FLAG) != SpString::npos) {
        ++touchEvent.tEventDisNum;
    }
    return touchEvent.tEventDisNum;
}
const FpsResult ParseFPS::ParseBranch(FilePath& filePath, PackageName& pN, PageType& pT, TouchEvent& tE)
{
    FpsResult fps = "0";
    if (tE.touchFlag) {
        std::vector<SpString> vecPackNames;
        //Get the time period in the renderservice
        float staticTime = 2.0f;
        this->StrSplit(pN, ".", vecPackNames);
        SpString uiPoint = uniProcess + vecPackNames.back();
        switch (pT) {
            case PageType::VIDEO:{
                if (filePath.find(FLING) != SpString::npos) {
                    staticTime = 0.5f;
                    fps =  PraseFPSTrace(filePath, staticTime, uiPoint);
                } else {
                    fps =  PraseFPSTrace(filePath, staticTime, videoPoint);
                }
                break;
            }
            case PageType::WEB:{
                fps =  PraseFPSTrace(filePath, staticTime, webPoint);
                break;
            }
            default:{
                fps =  PraseFPSTrace(filePath, staticTime, uiPoint);
                break;
            }
        }
    }
    return fps;
}
FpsResult  ParseFPS::ParseTraceFile(FilePath& filePath, PackageName& packageName)
{
    if (filePath.empty() || packageName.empty()) {
        return PARAMS_EMPTY;
    }
    FpsResult fps;
    FileSteamPtr inFile(new std::ifstream());
    inFile->open(filePath);
    if (inFile->fail()) {
        std::cout<<"File: "<<filePath<<" open failed!"<<std::endl;
        return FILE_OPEN_FAILED;
    } else {
        while (std::getline(*inFile, lineClient)) {
            if (this->GetTouchEventNum(lineClient, touchEventClient) > 0) {
                touchEventClient.touchFlag = true;
            }
            this->GetAndSetPageType(lineClient, pageTypeClient);
        }
        fps = this->ParseBranch(filePath, packageName, pageTypeClient, touchEventClient);
    }
    return "FPS:"+fps+"fps";
}
void ParseFPS::StaticHandoffStartTime(Line& line, RecordFpsVars& rfv)
{
    if (line.empty()) {
        return;
    }
    if (line.find(TOUCHEVENT_FLAG) != SpString::npos || line.find(HTOUCHEVENT_FLAG) != SpString::npos) {
        ++rfV.tEventDisNum;
        std::smatch result;
        unsigned int tNum = 4;
        if (tNum == rfV.tEventDisNum) {
            if (std::regex_search(line, result, pattern)) {
                rfV.leaveStartTime = result[0];
            }
        }
        if (rfV.tEventDisNum == touchEventClient.tEventDisNum) {
            if (std::regex_search(line, result, pattern)) {
                rfV.isStaticsLeaveTime = true;
            }
        }
    }
}
void ParseFPS::DecHandOffTime(Line& line, RecordFpsVars& rfv)
{
    if (line.empty()) {
        return;
    }
    if (rfV.isStaticsLeaveTime) {
        if (line.find(doPoint) != SpString::npos) {
            std::smatch result;
            if (std::regex_search(line, result, pattern)) {
                if (rfV.startFlag == 0) {
                    rfV.leaveStartTime = rfV.leaveEndTime = result[0];
                }
                ++rfV.startFlag;
            }
            if (rfV.pidMatchStr.empty()) {
                if (std::regex_search(line, result, pidPattern)) {
                    rfV.pidMatchStr = result[0];
                }
            }
            rfV.isAddFrame = true;
        }
    }
}
bool ParseFPS::CountRsEndTime(Line& line, RecordFpsVars& rfv, float staticTime, SpString uiPoint)
{
    if (line.empty()) {
        return false;
    }
    if (!rfV.pidMatchStr.empty() && rfV.isAddFrame) {
        SpString pid = rfV.pidMatchStr.substr(1, rfV.pidMatchStr.length() - 2);
        if (line.find(uiPoint) != SpString::npos) {
            rfV.isHasUI = true;
        }
        if (line.find("B|" + pid + "|") != SpString::npos && line.find("-" + pid) != SpString::npos) {
            beQueue.push(line);
        }
        if (line.find("E|" + pid + "|") != SpString::npos && line.find("-" + pid) != SpString::npos) {
            beQueue.pop();
        }
        if (beQueue.empty()) {
            rfV.isAddFrame = false;
            if (rfV.isHasUI) {
                rfV.isHasUI = false;
                if (std::stof(rfV.leaveEndTime) - std::stof(rfV.leaveStartTime) < staticTime) {
                    std::smatch result;
                    if (std::regex_search(line, result, pattern)) {
                        float intervalTime = 0.1;
                        float intervalResult = std::stof(result[0]) - std::stof(rfV.leaveEndTime);
                        if (intervalResult < intervalTime) {
                            ++rfV.frameNum;
                            rfV.leaveEndTime = result[0];
                        } else {
                            ++rfV.frameNum;
                            std::cout<<"NO."<<rfV.frameNum<<"fps Time: "<< intervalResult << "s" <<std::endl;
                            rfV.leaveEndTime = result[0];
                        }
                    }
                } else {
                    return true;
                }
            }
        }
    }
    return false;
}
FpsResult  ParseFPS::PraseFPSTrace(FilePath& filePath, float staticTime, SpString uiPoint)
{
    if (!this->lineClient.empty()) {
        this->lineClient.clear();
    }
    FileSteamPtr inFile(new std::ifstream());
    inFile->open(filePath);
    if (inFile->fail()) {
        std::cout<<"File: "<<filePath<<" open failed!"<<std::endl;
        return FILE_OPEN_FAILED;
    } else {
        // std::cout<<"File: "<<filePath<<" open success!"<<std::endl;
        while (std::getline(*inFile, this->lineClient)) {
            this->StaticHandoffStartTime(lineClient, rfV);
            this->DecHandOffTime(lineClient, rfV);
            if (this->CountRsEndTime(lineClient, rfV, staticTime, uiPoint)) {
                break;
            }
        }
        const auto duration = std::stof(rfV.leaveEndTime) - std::stof(rfV.leaveStartTime);
        const auto complexFps1 =  rfV.frameNum / duration;
        SP_FAILED_OPERATION((duration > 0 && rfV.frameNum > 0));
        rfV.complexFps = std::to_string(complexFps1);
    }
    return rfV.complexFps;
}