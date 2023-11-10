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

#include "rawtrace_parser.h"
#if IS_WASM
#include "../rpc/wasm_func.h"
#endif
#include "log.h"
#include "string_help.h"
namespace SysTuning {
namespace TraceStreamer {
RawTraceParser::RawTraceParser(TraceDataCache* dataCache, const TraceStreamerFilters* filters)
    : ParserBase(filters),
      cpuDetail_(std::make_unique<FtraceCpuDetailMsg>()),
      cpuDetailParser_(std::make_unique<CpuDetailParser>(dataCache, filters)),
      ftraceProcessor_(std::make_unique<FtraceProcessor>()),
      ksymsProcessor_(std::make_unique<KernelSymbolsProcessor>(dataCache, filters)),
      traceDataCache_(dataCache)
{
}
RawTraceParser::~RawTraceParser() {}
bool RawTraceParser::ParseCpuRawData(uint32_t cpuId, const std::string& buffer)
{
    TS_CHECK_TRUE(buffer.size() > 0, false, "buffer.size() is zero!");
    auto startPtr = reinterpret_cast<const uint8_t*>(buffer.c_str());
    auto endPtr = startPtr + buffer.size();
    cpuDetail_->set_cpu(cpuId);
    for (uint8_t* page = const_cast<uint8_t*>(startPtr); page < endPtr; page += FTRACE_PAGE_SIZE) {
        TS_CHECK_TRUE(ftraceProcessor_->HandlePage(*cpuDetail_.get(), *cpuDetailParser_.get(), page), false,
                      "handle page failed!");
    }
    cpuDetailParser_->FilterAllEvents(*cpuDetail_.get());
    return true;
}
void RawTraceParser::ParseTraceDataSegment(std::unique_ptr<uint8_t[]> bufferStr, size_t size)
{
    packagesBuffer_.insert(packagesBuffer_.end(), &bufferStr[0], &bufferStr[size]);
    auto packagesCurIter = packagesBuffer_.begin();
    if (ParseDataRecursively(packagesCurIter)) {
        packagesCurIter = packagesBuffer_.erase(packagesBuffer_.begin(), packagesCurIter);
    }
    return;
}
bool RawTraceParser::InitRawTraceFileHeader(std::deque<uint8_t>::iterator& packagesCurIter)
{
    TS_CHECK_TRUE(packagesBuffer_.size() >= sizeof(RawTraceFileHeader), false,
                  "buffer size less than rawtrace file header");
    RawTraceFileHeader header;
    auto ret = memcpy_s(&header, sizeof(RawTraceFileHeader), &(*packagesBuffer_.begin()), sizeof(RawTraceFileHeader));
    TS_CHECK_TRUE(ret == EOK, false, "Memcpy FAILED!Error code is %d, data size is %zu.", ret, packagesBuffer_.size());
    uint8_t cpuNum = (header.reserved >> 1) & 0x0F;
    uint8_t isArch32 = (header.reserved & 0x01);
    cpuRawMax_ = cpuNum + CONTENT_TYPE_CPU_RAW;
    TS_LOGI("magicNumber=%d, isArch32=%u, cpuNum=%u", header.magicNumber, isArch32, cpuNum);

    packagesCurIter += sizeof(RawTraceFileHeader);
    packagesCurIter = packagesBuffer_.erase(packagesBuffer_.begin(), packagesCurIter);
    hasGotHeader_ = true;
    return true;
}
bool RawTraceParser::InitEventFormats(const std::string& buffer)
{
    std::string line;
    std::istringstream iss(buffer);
    std::stringstream eventFormat;
    while (std::getline(iss, line)) {
        eventFormat << line << '\n';
        if (StartWith(line, eventEndCmd_)) {
            ftraceProcessor_->SetupEvent(eventFormat.str());
            eventFormat.str("");
        }
    }
    return true;
}
void RawTraceParser::WaitForParserEnd()
{
    cpuDetailParser_->FilterAllEvents(*cpuDetail_.get(), true);
    TS_LOGI("FilterAllEvents end!");
}
bool RawTraceParser::ParseDataRecursively(std::deque<uint8_t>::iterator& packagesCurIter)
{
    uint32_t type = INVALID_UINT8;
    uint32_t len = INVALID_UINT32;
    if (!hasGotHeader_) {
        TS_CHECK_TRUE(InitRawTraceFileHeader(packagesCurIter), false, "get rawtrace file header failed");
    }
    while (true) {
        auto ret = memcpy_s(&type, sizeof(type), &(*packagesCurIter), sizeof(type));
        TS_CHECK_TRUE(ret == EOK, false, "Memcpy FAILED!Error code is %d, data size is %zu.", ret,
                      packagesBuffer_.size());
        packagesCurIter += sizeof(type);
        ret = memcpy_s(&len, sizeof(len), &(*packagesCurIter), sizeof(len));
        TS_CHECK_TRUE(ret == EOK, false, "Memcpy FAILED!Error code is %d, data size is %zu.", ret,
                      packagesBuffer_.size());
        packagesCurIter += sizeof(len);
        uint32_t restDataLen = std::distance(packagesCurIter, packagesBuffer_.end());
        TS_CHECK_TRUE_RET(len <= restDataLen && packagesBuffer_.size() > 0, false);
        std::string bufferLine(packagesCurIter, packagesCurIter + len);
        packagesCurIter += len;
        packagesCurIter = packagesBuffer_.erase(packagesBuffer_.begin(), packagesCurIter);
        uint8_t curType = static_cast<uint8_t>(type);
        if (curType >= CONTENT_TYPE_CPU_RAW && curType <= cpuRawMax_) {
            auto cpuId = curType - CONTENT_TYPE_CPU_RAW;
            TS_CHECK_TRUE(ParseCpuRawData(cpuId, bufferLine), false, "cpu raw parse failed");
        } else if (curType == CONTENT_TYPE_CMDLINES) {
            TS_CHECK_TRUE(ftraceProcessor_->HandleCmdlines(bufferLine), false, "parse cmdlines failed");
        } else if (curType == CONTENT_TYPE_TGIDS) {
            TS_CHECK_TRUE(ftraceProcessor_->HandleTgids(bufferLine), false, "parse tgid failed");
        } else if (curType == CONTENT_TYPE_EVENTS_FORMAT) {
            TS_CHECK_TRUE(InitEventFormats(bufferLine), false, "init event format failed");
        } else if (curType == CONTENT_TYPE_HEADER_PAGE) {
            TS_CHECK_TRUE(ftraceProcessor_->HandleHeaderPageFormat(bufferLine), false, "init header page failed");
        } else if (curType == CONTENT_TYPE_PRINTK_FORMATS) {
            TS_CHECK_TRUE(PrintkFormatsProcessor::GetInstance().HandlePrintkSyms(bufferLine), false,
                          "init printk_formats failed");
        } else if (curType == CONTENT_TYPE_KALLSYMS) {
            TS_CHECK_TRUE(ksymsProcessor_->HandleKallSyms(bufferLine), false, "init printk_formats failed");
        } else {
            TS_LOGW("Raw Trace Type(%d) Unknown.", curType);
            return false;
        }
    }
    return true;
}
} // namespace TraceStreamer
} // namespace SysTuning
