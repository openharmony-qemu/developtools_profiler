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
#include <fstream>
#include <string>
#include <iostream>
#include <regex>
#include <cmath>
#include "include/stalling_rate_trace.h"
#include "include/sp_log.h"
#include <string_view>
namespace OHOS {
namespace SmartPerf {
double StallingRateTrace::StallingRateResult(std::string file)
{
    double stalligRate = 0;
    char realPath[PATH_MAX] = {0x00};
    if ((realpath(file.c_str(), realPath) == nullptr)) {
        std::cout << "" << std::endl;
    }
    infile.open(realPath);
    if (infile.fail()) {
        LOGI("StallingRateTrace open file(%s) fialed ", file.c_str());
        return stalligRate;
    }
    stalligRate = SmartPerf::StallingRateTrace::CalculateTime();
    infile.close();
    return stalligRate;
}

double StallingRateTrace::CalculateTime()
{
    frameLossRate = 0;
    frameLossTime = 0;
    swiperFrameLossRate = 0;
    appFrameLossRate = 0;
    tabsFrameLossRate = 0;
    frameLossSwiperTime = 0;
    frameLossTabsTime = 0;
    std::string signS = "S|";
    std::string signF = "F|";
    std::string line;
    while (getline(infile, line)) {
        AppList(line, signS, signF);
        AppSwiperScroll(line, signS, signF);
        APPTabs(line, signS, signF);
    }
    CalcFrameRate();
    JudgFrameRate();
    MultiLaneFrameRate();
    return frameLossRate;
}

void StallingRateTrace::CalcFrameRate()
{
    if (appListDynamicStartTime != 0 && appListDynamicFinishTime != 0) {
        appFrameLossRate = (frameLossTime / (appListDynamicFinishTime - appListDynamicStartTime) * oneThousand);
    } else {
        appFrameLossRate = -1;
    }
    LOGI("result.appFrameLossRate: (%s)", std::to_string(appFrameLossRate).c_str());

    if (swiperDynamicFinishTime != 0 && swiperDynamicStartTime != 0) {
        swiperFrameLossRate = (frameLossSwiperTime / (swiperDynamicFinishTime - swiperDynamicStartTime) * oneThousand);
    } else {
        swiperFrameLossRate = -1;
    }
    LOGI("result.swiperFrameLossRate: (%s)", std::to_string(swiperFrameLossRate).c_str());

    if (appTabsDynamicStartTime != 0 && appTabsDynamicFinishTime != 0) {
        tabsFrameLossRate = (frameLossTabsTime / (appTabsDynamicFinishTime - appTabsDynamicStartTime) * oneThousand);
    } else {
        tabsFrameLossRate = -1;
    }
    LOGI("result.tabsFrameLossRate: (%s)", std::to_string(tabsFrameLossRate).c_str());
}

void StallingRateTrace::JudgFrameRate()
{
    auto hasDynamic = [](bool finishTime, bool startTime) {
        return finishTime != 0 || startTime != 0;
    };

    bool appListDynamicExists = hasDynamic(appListDynamicFinishTime, appListDynamicStartTime);
    bool swiperDynamicExists = hasDynamic(swiperDynamicFinishTime, swiperDynamicStartTime);
    bool tabsDynamicExists = hasDynamic(appTabsDynamicFinishTime, appTabsDynamicStartTime);

    if (!appListDynamicExists) {
        LOGI("no app list Dynamic");
        frameLossRate = swiperDynamicExists ? swiperFrameLossRate :
                        tabsDynamicExists ? tabsFrameLossRate : -1;
    } else if (!swiperDynamicExists) {
        LOGI("no swiper Dynamic");
        frameLossRate = appListDynamicExists ? appFrameLossRate :
                        tabsDynamicExists ? tabsFrameLossRate : -1;
    } else if (!tabsDynamicExists) {
        LOGI("no tabs Dynamic");
        frameLossRate = appListDynamicExists ? appFrameLossRate :
                        swiperDynamicExists ? swiperFrameLossRate : -1;
    } else {
        frameLossRate = -1;
    }
}

void StallingRateTrace::MultiLaneFrameRate()
{
    if (appFrameLossRate == 0) {
        LOGI("no app list hitchTime");
        if (swiperFrameLossRate > 0) {
            frameLossRate = swiperFrameLossRate;
        } else if (tabsFrameLossRate > 0) {
            frameLossRate = tabsFrameLossRate;
        } else {
            frameLossRate = 0;
        }
    } else if (swiperFrameLossRate == 0) {
        LOGI("no swiper list hitchTime");
        if (appFrameLossRate > 0) {
            frameLossRate = appFrameLossRate;
        } else if (tabsFrameLossRate > 0) {
            frameLossRate = tabsFrameLossRate;
        } else {
            frameLossRate = 0;
        }
    } else if (tabsFrameLossRate == 0) {
        LOGI("no tabs list hitchTime");
        if (appFrameLossRate > 0) {
            frameLossRate = appFrameLossRate;
        } else if (swiperFrameLossRate > 0) {
            frameLossRate = swiperFrameLossRate;
        }
    }
    AddMultiLaneFrameRate();
}

void StallingRateTrace::AddMultiLaneFrameRate()
{
    if (appFrameLossRate > 0 && swiperFrameLossRate > 0) {
        //app and swiper hitchTime 1
        if (appListDynamicStartTime < swiperDynamicStartTime) {
            frameLossRate = appFrameLossRate;
        } else {
            frameLossRate = swiperFrameLossRate;
        }
    } else if (appFrameLossRate > 0 && tabsFrameLossRate > 0) {
        //app and tabs hitchTime 2
        if (appListDynamicStartTime < appTabsDynamicStartTime) {
            frameLossRate = appFrameLossRate;
        } else {
            frameLossRate = appTabsDynamicStartTime;
        }
    } else if (tabsFrameLossRate > 0 && swiperFrameLossRate > 0) {
        //tabs and swiper hitchTime 3
        if (appTabsDynamicStartTime < swiperDynamicStartTime) {
            frameLossRate = tabsFrameLossRate;
        } else {
            frameLossRate = swiperFrameLossRate;
        }
    }
}


void StallingRateTrace::AppList(const std::string &line, const std::string &signS, const std::string &signF)
{
    if (line.find("H:LAUNCHER_APP_LAUNCH_FROM_ICON,") != std::string::npos ||
        line.find("H:APP_LIST_FLING,") != std::string::npos ||
        line.find("H:WEB_LIST_FLING") != std::string::npos ||
        line.find("H:ABILITY_OR_PAGE_SWITCH,") != std::string::npos ||
        line.find("H:APP_TRANSITION_TO_OTHER_APP,") != std::string::npos ||
        line.find("H:LAUNCHER_APP_LAUNCH_FROM_DOCK,") != std::string::npos ||
        line.find("H:LAUNCHER_APP_LAUNCH_FROM_APPCENTER,") != std::string::npos) {
        if (listFlag) {
            LOGI("AppList line start: (%s)", line.c_str());
            appListDynamicFinishTime = GetTimes(line, signF);
            LOGI("appListDynamicFinishTime: (%s)", std::to_string(appListDynamicFinishTime).c_str());
            listFlag = false;
        } else {
            LOGI("AppList line finish: (%s)", line.c_str());
            appListDynamicStartTime = GetTimes(line, signS);
            LOGI("appListDynamicStartTime: (%s)", std::to_string(appListDynamicStartTime).c_str());
            listFlag = true;
            frameLossTime = 0;
        }
    }
    if (listFlag) {
        GetRsHardWareRate(nowFrameRate, line, SWIM_APPLIST);
        if (upperScreenFlag) {
            if (line.find("|H:Present Fence ") != std::string::npos) {
                fenceId = GetFenceId(line);
                LOGI("fenceId: (%d)", fenceId);
            }
            std::string waitFenceId = "|H:Waiting for Present Fence " + std::to_string(fenceId);
            if (line.find(waitFenceId) != std::string::npos) {
                nowTime = std::stod(SmartPerf::StallingRateTrace::GetOnScreenTimeStart(line));
                GetFrameLossTime(nowTime, lastTime, roundTime, frameLossTime);
                LOGI("frameLossTime: (%s)", std::to_string(frameLossTime).c_str());
                lastTime = nowTime;
                upperScreenFlag = false;
            }
        }
    }
}

void StallingRateTrace::GetFrameLossTime(double curTime, double prevTime, double drawTime, double &totalFrameLossTime)
{
    if ((curTime - prevTime) > drawTime && prevTime != 0) {
        double diffTime = (curTime - prevTime) - drawTime;
        LOGI("diffTime: (%s)", std::to_string(diffTime).c_str());
        totalFrameLossTime += diffTime;
        LOGI("totalFrameLossTime: (%s)", std::to_string(totalFrameLossTime).c_str());
    }
}

void StallingRateTrace::GetRsHardWareRate(double curFrameRate, const std::string &line, SWIM_TYPE type)
{
    if (line.find("H:RSHardwareThread::CommitAndReleaseLayers") != std::string::npos) {
        switch (type) {
            case SWIM_APPLIST:
                upperScreenFlag = true;
                break;
            case SWIM_APPSWIPER:
                upperScreenSwiperFlag = true;
                break;
            case SWIM_APPTABS:
                upperScreenTabsFlag = true;
                break;
            default:
                break;
        }
        curFrameRate = GetFrameRate(line);
        LOGI("curFrameRate1: (%s)", std::to_string(curFrameRate).c_str());
        if (curFrameRate != 0) {
            UpdateRoundTime(curFrameRate, type);
        }
    } else if (line.find("H:RSHardwareThread::PerformSetActiveMode setting active mode") != std::string::npos) {
        switch (type) {
            case SWIM_APPLIST:
                upperScreenFlag = true;
                break;
            case SWIM_APPSWIPER:
                upperScreenSwiperFlag = true;
                break;
            case SWIM_APPTABS:
                upperScreenTabsFlag = true;
                break;
            default:
                break;
        }
        curFrameRate = GetFrameRate(line);
        LOGI("curFrameRate2: (%s)", std::to_string(curFrameRate).c_str());
        if (curFrameRate != 0) {
            UpdateRoundTime(curFrameRate, type);
        }
    }
}

void StallingRateTrace::UpdateRoundTime(double curFrameRate, SWIM_TYPE type)
{
    const double kadunNum = 1.5;
    const double num = 1;
    if (curFrameRate != 0) {
        switch (type) {
            case SWIM_APPLIST:
                roundTime = (num / curFrameRate) * kadunNum;
                break;
            case SWIM_APPSWIPER:
                roundSwiperTime = (num / curFrameRate) * kadunNum;
                break;
            case SWIM_APPTABS:
                roundTabsTime = (num / curFrameRate) * kadunNum;
                break;
            default:
                break;
        }
    }
}

void StallingRateTrace::AppSwiperScroll(const std::string &line, const std::string &signS, const std::string &signF)
{
    if (line.find("H:APP_SWIPER_NO_ANIMATION_SWITCH") != std::string::npos ||
        line.find("H:APP_SWITCH_FRAME_ANIMATION") != std::string::npos ||
        line.find("H:APP_SWIPER_SCROLL,") != std::string::npos) {
        if (swiperScrollFlag == 0) {
            LOGI("AppSwiperScroll line start: (%s)", line.c_str());
            swiperDynamicStartTime = GetTimes(line, signS);
            LOGI("swiperDynamicStartTime: (%s)", std::to_string(swiperDynamicStartTime).c_str());
            frameLossSwiperTime = 0;
            swiperScrollFlag = 1;
            swiperFlag = true;
        }
    }
    if (line.find("H:APP_SWIPER_FLING,") != std::string::npos ||
        line.find("H:APP_SWIPER_NO_ANIMATION_SWITCH") != std::string::npos ||
        line.find("H:APP_SWITCH_FRAME_ANIMATION") != std::string::npos) {
        if (swiperFlingFlag == 1) {
            LOGI("AppSwiper FinishTime line: (%s)", line.c_str());
            swiperDynamicFinishTime = GetTimes(line, signF);
            LOGI("swiperDynamicFinishTime: (%s)", std::to_string(swiperDynamicFinishTime).c_str());
            swiperFlag = false;
        }
        if (swiperDynamicFinishTime == 0) {
            swiperFlingFlag = 0;
        }
        swiperFlingFlag++;
    }
    if (swiperFlag) {
        GetRsHardWareRate(nowSwiperFrameRate, line, SWIM_APPSWIPER);
        if (upperScreenSwiperFlag) {
            if (line.find("|H:Present Fence ") != std::string::npos) {
                fenceIdSwiper = GetFenceId(line);
                LOGI("fenceIdSwiper: (%d)", fenceIdSwiper);
            }
            std::string waitFenceId = "|H:Waiting for Present Fence " + std::to_string(fenceIdSwiper);
            if (line.find(waitFenceId) != std::string::npos) {
                nowSwiperTime = std::stod(SmartPerf::StallingRateTrace::GetOnScreenTimeStart(line));
                LOGI("nowSwiperTime: (%s)", std::to_string(nowSwiperTime).c_str());
                GetFrameLossTime(nowSwiperTime, lastSwiperTime, roundSwiperTime, frameLossSwiperTime);
                LOGI("frameLossSwiperTime: (%s)", std::to_string(frameLossSwiperTime).c_str());
                lastSwiperTime = nowSwiperTime;
                upperScreenSwiperFlag = false;
            }
        }
    }
}

void StallingRateTrace::APPTabs(const std::string &line, const std::string &signS, const std::string &signF)
{
    static const std::string appTabsNoAnimation = "H:APP_TABS_NO_ANIMATION_SWITCH";
    static const std::string appTabsFrameAnimation = "H:APP_TABS_FRAME_ANIMATION";
    static const std::string appTabsScroll = "H:APP_TABS_SCROLL,";

    if (line.find(appTabsNoAnimation) != std::string::npos ||
        line.find(appTabsFrameAnimation) != std::string::npos ||
        line.find(appTabsScroll) != std::string::npos) {
        if (tabsFlag) {
            LOGI("APPTabs line start: (%s)", line.c_str());
            appTabsDynamicFinishTime = GetTimes(line, signF);
            LOGI("appTabsDynamicFinishTime: (%s)", std::to_string(appTabsDynamicFinishTime).c_str());
            tabsFlag = false;
        } else {
            LOGI("APPTabs line finish: (%s)", line.c_str());
            appTabsDynamicStartTime = GetTimes(line, signS);
            LOGI("appTabsDynamicStartTime: (%s)", std::to_string(appTabsDynamicStartTime).c_str());
            tabsFlag = true;
            frameLossTabsTime = 0;
        }
    }
    if (tabsFlag) {
        GetRsHardWareRate(nowTabsFrameRate, line, SWIM_APPTABS);
        if (upperScreenTabsFlag) {
            if (line.find("|H:Present Fence ") != std::string::npos) {
                fenceIdTabs = GetFenceId(line);
                LOGI("fenceIdTabs: (%s)", std::to_string(fenceIdTabs).c_str());
            }
            std::string waitFenceId = "|H:Waiting for Present Fence " + std::to_string(fenceIdTabs);
            if (line.find(waitFenceId) != std::string::npos) {
                nowTabsTime = std::stod(SmartPerf::StallingRateTrace::GetOnScreenTimeStart(line));
                LOGI("nowTabsTime: (%s)", std::to_string(nowTabsTime).c_str());
                LOGI("lastTabsTime: (%s)", std::to_string(lastTabsTime).c_str());
                LOGI("roundTabsTime: (%s)", std::to_string(roundTabsTime).c_str());
                GetFrameLossTime(nowTabsTime, lastTabsTime, roundTabsTime, frameLossTabsTime);
                LOGI("app tabs frameLossTabsTime: (%s)", std::to_string(frameLossTabsTime).c_str());
                lastTabsTime = nowTabsTime;
                upperScreenTabsFlag = false;
            }
        }
    }
}

double StallingRateTrace::GetFrameRate(const std::string &line) const
{
    double rate = 0;
    std::string delimiter = "rate: ";
    if (line.find("now:") != std::string::npos && line.find("rate:") != std::string::npos) {
        std::string delimiter1 = ", now:";
        size_t pos1 = line.find(delimiter);
        std::string result1 = line.substr(pos1 + delimiter.length());
        size_t pos2 = line.find(delimiter1);
        std::string result2 = result1.substr(0, pos2);
        rate = std::stod(result2.c_str());
    }
    if (line.find("rate:") != std::string::npos) {
        size_t pos = line.find(delimiter);
        std::string result = line.substr(pos + delimiter.length());
        rate = std::stod(result.c_str());
    }
    return rate;
}

int StallingRateTrace::GetFenceId(const std::string &line) const
{
    std::string delimiter = "H:Present Fence ";
    size_t pos = line.find(delimiter);
    std::string result = line.substr(pos + delimiter.length());
    int presentFenceId = std::atoi(result.c_str());
    return presentFenceId;
}

std::string StallingRateTrace::GetOnScreenTimeStart(const std::string &line) const
{
    std::string startTime = "0";
    size_t subNum = 7;
    size_t positionFirst = line.find("....");
    size_t positionSecond = line.find(":");
    startTime = line.substr(positionFirst + subNum, positionSecond - positionFirst - subNum);
    return startTime;
}

double StallingRateTrace::GetTimes(const std::string &line, const std::string &sign) const
{
    std::string_view lineView(line);

    size_t positionFirst = lineView.find("....");
    size_t positionSecond = lineView.find(":");

    if (positionFirst != std::string_view::npos && positionSecond != std::string_view::npos) {
        if (lineView.find(sign) != std::string_view::npos) {
            size_t subNum = 7;
            // 使用 string_view 直接计算并转换为 double
            std::string_view timeStr = lineView.substr(positionFirst + subNum, positionSecond - positionFirst - subNum);
            return std::stod(std::string(timeStr)); // 转换为 string 再进行 stod
        }
    }
    return 0.0;
}
}
}
