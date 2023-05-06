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
#pragma once
#include <iostream>
#include <fstream>
#include <vector>
#include <queue>
#include <regex>
#include <cstdio>
#include <string>
#include <memory>
using SpString = std::string;
using FpsResult = SpString;
using FilePath = const SpString;
using PackageName = const SpString;
using FileSteamPtr = std::shared_ptr<std::ifstream>;
using Line = SpString;
typedef enum {
    VIDEO,
    WEB,
    LARGE
} PageType;
#define PARAMS_EMPTY "params invalided!"
#define FILE_OPEN_FAILED "file open failed!"
#define TOUCHEVENT_FLAG "H:touchEventDispatch"
#define HTOUCHEVENT_FLAG "H:TouchEventDispatch"
#define ROSENRENDERWEB "H:RosenRenderWeb:"
#define ROSENRENDERTEXTURE "H:RosenRenderTexture:"
#define FLING "fling"
#define SP_EXIT(...) (fprintf(stderr, __VA_ARGS__), exit(0))
#define SP_FAILED_OPERATION(x) \
    (void)((x) || (SP_EXIT("failed Operation\n"), 0))
class ParseFPS {
public:
    ParseFPS();
    ~ParseFPS();
private:
    struct RecordFpsVars {
        //moveResponseTime - moveStartTime
        int frameNum = 0;
        unsigned int    tEventDisNum = 0;
        //The start point of the first frame after the hand is off
        SpString leaveStartTime = "0";
        //the last point
        SpString leaveEndTime = "0";
        //the last touchDis point's time
        SpString lastTouchEventTime = "0";
        //After the first frame after the hand is left to start rendering add, the end of the add is terminated
        bool isAddFrame = false;
        //DoComposition has the dot corresponding to the package name under the calculation end time
        bool isHasUI = false;
        //Start fetching the start point of the hand
        bool isStaticsLeaveTime = false;
        //The start time is taken once
        int startFlag = 0;
        FpsResult complexFps;
        SpString pidMatchStr;
    };
    struct TouchEvent {
        TouchEvent()
        {
            tEventDisNum = 0;
            touchFlag = false;
        }
        unsigned int    tEventDisNum;
        bool            touchFlag;
    };
public:
    FpsResult  ParseTraceFile(FilePath& filePath, PackageName& packageName);
private:
    inline  unsigned int GetTouchEventNum(Line& line, TouchEvent& touchEvent);
    inline  void StrSplit(const SpString &content, const SpString &sp, std::vector<SpString> &out);
    inline  void GetAndSetPageType(Line& line, PageType& pageType);
    inline  const FpsResult ParseBranch(FilePath& filePath, PackageName& pN, PageType& pT, TouchEvent& tE);
    inline  FpsResult  PraseFPSTrace(FilePath& filePath, float staticTime, SpString uiPoint);
    //Gets the statistical off start time marker bits
    inline  void DecHandOffTime(Line& line, RecordFpsVars& rfv);
    //Statistics of the start time of the handout
    inline  void StaticHandoffStartTime(Line& line, RecordFpsVars& rfv);
    //Count the number of rendered frames and the end time
    inline  bool CountRsEndTime(Line& line, RecordFpsVars& rfv, float staticTime, SpString uiPoint);

    const SpString videoPoint = "H:RSUniRender::Process:[RosenRenderTexture]";
    const SpString webPoint   = "H:RSUniRender::Process:[RosenRenderWeb]";
    const SpString sysUiPoint = "H:RSUniRender::Process:[SystemUi_ControlPanel]";
    const SpString doPoint    = "H:RSMainThread::DoComposition";
    const SpString uniProcess = "H:RSUniRender::Process:[";

    std::regex pattern;
    //filter pid
    std::regex pidPattern;
private:
    PageType            pageTypeClient;
    Line                lineClient;
    TouchEvent          touchEventClient;
    RecordFpsVars       rfV;
    std::queue<SpString> beQueue;
};