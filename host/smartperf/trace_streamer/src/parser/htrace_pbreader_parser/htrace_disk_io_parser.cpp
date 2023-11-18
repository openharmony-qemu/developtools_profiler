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
#include "htrace_disk_io_parser.h"
#include "clock_filter_ex.h"
#include "diskio_plugin_result.pbreader.h"
#include "htrace_event_parser.h"
#include "process_filter.h"
#include "stat_filter.h"
namespace SysTuning {
namespace TraceStreamer {
HtraceDiskIOParser::HtraceDiskIOParser(TraceDataCache* dataCache, const TraceStreamerFilters* ctx)
    : EventParserBase(dataCache, ctx)
{
}

HtraceDiskIOParser::~HtraceDiskIOParser()
{
    TS_LOGI("diskio ts MIN:%llu, MAX:%llu", static_cast<unsigned long long>(GetPluginStartTime()),
            static_cast<unsigned long long>(GetPluginEndTime()));
}
void HtraceDiskIOParser::Parse(ProtoReader::BytesView tracePacket, uint64_t ts)
{
    ProtoReader::DiskioData_Reader diskioData(tracePacket.data_, tracePacket.size_);
    auto stat = diskioData.statsdata();
    ProtoReader::StatsData_Reader statsData(stat);
    double rdCountPerSec = 0; // The amount of data read from the device per second kB_read/s
    double wrCountPerSec = 0; // The amount of data written to the device per second kB_wrtn/s
    uint64_t rdCount = 0;     // Total amount of data read kB_read
    uint64_t wrCount = 0;     // The total amount of data written kB_wrtn
    for (auto i = statsData.statsinfo(); i; ++i) {
        auto statsInfo = ProtoReader::IoStatData_Reader(i->ToBytes());
        rdCountPerSec += statsInfo.rd_per_sec();
        wrCountPerSec += statsInfo.wr_per_sec();
        rdCount += statsInfo.rd_kb();
        wrCount += statsInfo.wr_kb();
    }

    streamFilters_->statFilter_->IncreaseStat(TRACE_DISKIO, STAT_EVENT_RECEIVED);
    diskIOData_.push_back(TsDiskIOData{ts, diskioData.rd_sectors_kb(), diskioData.wr_sectors_kb(),
                                       diskioData.prev_rd_sectors_kb(), diskioData.prev_wr_sectors_kb(), rdCountPerSec,
                                       wrCountPerSec, rdCount, wrCount});
}
void HtraceDiskIOParser::Finish()
{
    auto cmp = [](const TsDiskIOData& a, const TsDiskIOData& b) { return a.ts < b.ts; };
    std::stable_sort(diskIOData_.begin(), diskIOData_.end(), cmp);
    bool first = true;
    uint64_t lastTs = 0;
    for (auto itor = diskIOData_.begin(); itor != diskIOData_.end(); itor++) {
        itor->ts = streamFilters_->clockFilter_->ToPrimaryTraceTime(TS_CLOCK_REALTIME, itor->ts);
        UpdatePluginTimeRange(TS_CLOCK_REALTIME, itor->ts, itor->ts);
        if (first) {
            lastTs = itor->ts;
            first = false;
            continue;
        }
        auto dur = itor->ts - lastTs;
        auto durS = 1.0 * dur / SEC_TO_NS;
        traceDataCache_->GetDiskIOData()->AppendNewData(
            itor->ts, itor->ts - lastTs, itor->rdSectorsKb, itor->wrSectorsKb,
            1.0 * (itor->rdSectorsKb - itor->prevRdSectorsKb) / durS,
            1.0 * (itor->wrSectorsKb - itor->prevWrSectorsKb) / durS, itor->rdCountPerSec, itor->wrCountPerSec,
            itor->rdCount, itor->wrCount);
        lastTs = itor->ts;
    }
    diskIOData_.clear();
    traceDataCache_->MixTraceTime(GetPluginStartTime(), GetPluginEndTime());
}
} // namespace TraceStreamer
} // namespace SysTuning
