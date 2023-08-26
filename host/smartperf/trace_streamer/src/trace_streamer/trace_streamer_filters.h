/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef TRACE_STREAMERTOKEN_H
#define TRACE_STREAMERTOKEN_H

#include <memory>
namespace SysTuning {
namespace TraceStreamer {
class AnimationFilter;
class SliceFilter;
class ProcessFilter;
class CpuFilter;
class MeasureFilter;
class FilterFilter;
class ClockFilterEx;
class SymbolsFilter;
class StatFilter;
class BinderFilter;
class ArgsFilter;
class IrqFilter;
class SystemEventMeasureFilter;
class HiSysEventMeasureFilter;
class FrameFilter;
class APPStartupFilter;
class TaskPoolFilter;
class PerfDataFilter;
class TraceStreamerFilters {
public:
    TraceStreamerFilters();
    ~TraceStreamerFilters();
    void FilterClear();
    std::unique_ptr<AnimationFilter> animationFilter_;
    std::unique_ptr<ClockFilterEx> clockFilter_;
    std::unique_ptr<FilterFilter> filterFilter_;
    std::unique_ptr<SliceFilter> sliceFilter_;
    std::unique_ptr<ProcessFilter> processFilter_;
    std::unique_ptr<CpuFilter> cpuFilter_;
    std::unique_ptr<MeasureFilter> cpuMeasureFilter_;
    std::unique_ptr<MeasureFilter> threadMeasureFilter_;
    std::unique_ptr<MeasureFilter> threadFilter_;
    std::unique_ptr<MeasureFilter> processMeasureFilter_;
    std::unique_ptr<MeasureFilter> processFilterFilter_;
    std::unique_ptr<SymbolsFilter> symbolsFilter_;
    std::unique_ptr<StatFilter> statFilter_;
    std::unique_ptr<MeasureFilter> clockRateFilter_;
    std::unique_ptr<MeasureFilter> clockEnableFilter_;
    std::unique_ptr<MeasureFilter> clockDisableFilter_;
    std::unique_ptr<MeasureFilter> clkRateFilter_;
    std::unique_ptr<MeasureFilter> clkEnableFilter_;
    std::unique_ptr<MeasureFilter> clkDisableFilter_;
    std::unique_ptr<BinderFilter> binderFilter_;
    std::unique_ptr<ArgsFilter> argsFilter_;
    std::unique_ptr<IrqFilter> irqFilter_;
    std::unique_ptr<SystemEventMeasureFilter> sysEventMemMeasureFilter_;
    std::unique_ptr<SystemEventMeasureFilter> sysEventVMemMeasureFilter_;
    std::unique_ptr<SystemEventMeasureFilter> sysEventSourceFilter_;
    std::unique_ptr<HiSysEventMeasureFilter> hiSysEventMeasureFilter_;
    std::unique_ptr<FrameFilter> frameFilter_;
    std::unique_ptr<APPStartupFilter> appStartupFilter_;
    std::unique_ptr<TaskPoolFilter> taskPoolFilter_;
    std::unique_ptr<PerfDataFilter> perfDataFilter_;
};
} // namespace TraceStreamer
} // namespace SysTuning

#endif // TRACE_STREAMERTOKEN_H
