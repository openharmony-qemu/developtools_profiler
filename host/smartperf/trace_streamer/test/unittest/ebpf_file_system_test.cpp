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

#include <hwext/gtest-ext.h>
#include <hwext/gtest-tag.h>

#include "ebpf_data_parser.h"
#include "ebpf_stdtype.h"
#include "process_filter.h"
#include "trace_streamer_selector.h"
#include "ts_common.h"

using namespace testing::ext;
using namespace SysTuning::TraceStreamer;
using namespace SysTuning::EbpfStdtype;
namespace SysTuning {
namespace TraceStreamer {
class EbpfFileSystemTest : public ::testing::Test {
public:
    void SetUp()
    {
        stream_.InitFilter();
    }

    void TearDown() {}

public:
    SysTuning::TraceStreamer::TraceStreamerSelector stream_ = {};
};

const uint32_t PID_01 = 32;
const uint32_t TID_01 = 12;
const uint32_t PID_02 = 33;
const uint32_t TID_02 = 13;
const uint64_t START_TIME_01 = 1725645867369;
const uint64_t END_TIME_01 = 1725645967369;
const uint64_t START_TIME_02 = 1725645867369;
const uint64_t END_TIME_02 = 1725645967369;
const int32_t RET_01 = 8;
const int32_t RET_02 = -1;
const uint16_t IPS_NUM_00 = 0;
const uint16_t IPS_NUM_01 = 1;
const uint16_t IPS_NUM_02 = 2;
const uint64_t ARGS_01[ARGS_MAX] = {101, 102, 103, 104};
const uint64_t ARGS_02[ARGS_MAX] = {201, 202, 203, 204};
const char PROCESS_NAME_01[MAX_PROCESS_NAME_SZIE] = "process01";
const char PROCESS_NAME_02[MAX_PROCESS_NAME_SZIE] = "process02";
const uint64_t IPS_01[IPS_NUM_01] = {0x100000000};
const uint64_t IPS_02[IPS_NUM_02] = {0x100000000, 0x100000001};

/**
 * @tc.name: ParseFileSystemWithTypeOpen
 * @tc.desc: Test parse Ebpf data has one file system data with type open and no ips
 * @tc.type: FUNC
 */
HWTEST_F(EbpfFileSystemTest, ParseFileSystemWithTypeOpen, TestSize.Level1)
{
    TS_LOGI("test30-1");

    EbpfDataHeader ebpfHeader;
    ebpfHeader.header.clock = EBPF_CLOCK_BOOTTIME;

    std::deque<uint8_t> dequeBuffer = {};
    dequeBuffer.insert(dequeBuffer.end(), reinterpret_cast<uint8_t*>(&ebpfHeader),
                       reinterpret_cast<uint8_t*>(&ebpfHeader + 1));

    FsFixedHeader fsFixedHeader;
    fsFixedHeader.pid = PID_01;
    fsFixedHeader.tid = TID_01;
    fsFixedHeader.startTime = START_TIME_01;
    fsFixedHeader.endTime = END_TIME_01;
    fsFixedHeader.ret = RET_01;
    fsFixedHeader.nrUserIPs = IPS_NUM_00;
    fsFixedHeader.type = SYS_OPENAT2;
    for (auto i = 0; i < ARGS_MAX; i++) {
        fsFixedHeader.args[i] = ARGS_01[i];
    }
    strncpy_s(fsFixedHeader.processName, MAX_PROCESS_NAME_SZIE, PROCESS_NAME_01, MAX_PROCESS_NAME_SZIE);

    EbpfTypeAndLength ebpfTypeAndLength;
    ebpfTypeAndLength.length = sizeof(fsFixedHeader);
    ebpfTypeAndLength.type = ITEM_EVENT_FS;

    dequeBuffer.insert(dequeBuffer.end(), reinterpret_cast<uint8_t*>(&ebpfTypeAndLength),
                       reinterpret_cast<uint8_t*>(&ebpfTypeAndLength + 1));
    dequeBuffer.insert(dequeBuffer.end(), reinterpret_cast<uint8_t*>(&fsFixedHeader),
                       reinterpret_cast<uint8_t*>(&fsFixedHeader + 1));

    std::unique_ptr<EbpfDataParser> parser =
        std::make_unique<EbpfDataParser>(stream_.traceDataCache_.get(), stream_.streamFilters_.get());
    EXPECT_TRUE(parser->Init(dequeBuffer, dequeBuffer.size()));
    EXPECT_TRUE(parser->reader_->GetFileSystemEventMap().size());
    parser->ParseFileSystemEvent();
    parser->Finish();
    EXPECT_TRUE(parser->reader_->ebpfDataHeader_->header.clock == EBPF_CLOCK_BOOTTIME);
    auto callChainId = stream_.traceDataCache_->GetConstFileSystemSample().CallChainIds()[0];
    EXPECT_EQ(callChainId, INVALID_UINT32);
    auto type = stream_.traceDataCache_->GetConstFileSystemSample().Types()[0];
    EXPECT_EQ(type, OPEN);
    auto startTs = stream_.traceDataCache_->GetConstFileSystemSample().StartTs()[0];
    EXPECT_EQ(startTs, START_TIME_01);
    auto endTs = stream_.traceDataCache_->GetConstFileSystemSample().EndTs()[0];
    EXPECT_EQ(endTs, END_TIME_01);
    auto dur = stream_.traceDataCache_->GetConstFileSystemSample().Durs()[0];
    EXPECT_EQ(dur, END_TIME_01 - START_TIME_01);
    auto ExpectReturnValue = parser->ConvertToHexTextIndex(RET_01);
    auto returnValue = stream_.traceDataCache_->GetConstFileSystemSample().ReturnValues()[0];
    EXPECT_EQ(returnValue, ExpectReturnValue);
    auto errorCode = stream_.traceDataCache_->GetConstFileSystemSample().ErrorCodes()[0];
    EXPECT_EQ(errorCode, INVALID_UINT64);
    auto fd = stream_.traceDataCache_->GetConstFileSystemSample().Fds()[0];
    EXPECT_EQ(fd, RET_01);
    auto fileId = stream_.traceDataCache_->GetConstFileSystemSample().FileIds()[0];
    EXPECT_EQ(fileId, INVALID_UINT64);
    auto size = stream_.traceDataCache_->GetConstFileSystemSample().Sizes()[0];
    EXPECT_EQ(size, MAX_SIZE_T);
    auto i = 0;
    auto ExpectFirstArg = parser->ConvertToHexTextIndex(ARGS_01[i++]);
    auto firstArg = stream_.traceDataCache_->GetConstFileSystemSample().FirstArguments()[0];
    EXPECT_EQ(firstArg, ExpectFirstArg);
    auto ExpectSecondArg = parser->ConvertToHexTextIndex(ARGS_01[i++]);
    auto secondArg = stream_.traceDataCache_->GetConstFileSystemSample().SecondArguments()[0];
    EXPECT_EQ(secondArg, ExpectSecondArg);
    auto ExpectThirdArg = parser->ConvertToHexTextIndex(ARGS_01[i++]);
    auto thirdArg = stream_.traceDataCache_->GetConstFileSystemSample().ThirdArguments()[0];
    EXPECT_EQ(thirdArg, ExpectThirdArg);
    auto ExpectFourthArg = parser->ConvertToHexTextIndex(ARGS_01[i]);
    auto fourthArg = stream_.traceDataCache_->GetConstFileSystemSample().FourthArguments()[0];
    EXPECT_EQ(fourthArg, ExpectFourthArg);
}

/**
 * @tc.name: ParseFileSystemWithTypeClose
 * @tc.desc: Test parse Ebpf data has one file system data with type close and no ips and return value little to zero
 * @tc.type: FUNC
 */
HWTEST_F(EbpfFileSystemTest, ParseFileSystemWithTypeClose, TestSize.Level1)
{
    TS_LOGI("test30-2");

    EbpfDataHeader ebpfHeader;
    ebpfHeader.header.clock = EBPF_CLOCK_BOOTTIME;

    std::deque<uint8_t> dequeBuffer = {};
    dequeBuffer.insert(dequeBuffer.end(), reinterpret_cast<uint8_t*>(&ebpfHeader),
                       reinterpret_cast<uint8_t*>(&ebpfHeader + 1));

    FsFixedHeader fsFixedHeader;
    fsFixedHeader.pid = PID_02;
    fsFixedHeader.tid = TID_02;
    fsFixedHeader.startTime = START_TIME_02;
    fsFixedHeader.endTime = END_TIME_02;
    fsFixedHeader.ret = RET_02;
    fsFixedHeader.nrUserIPs = IPS_NUM_00;
    fsFixedHeader.type = SYS_CLOSE;
    for (auto i = 0; i < ARGS_MAX; i++) {
        fsFixedHeader.args[i] = ARGS_02[i];
    }
    strncpy_s(fsFixedHeader.processName, MAX_PROCESS_NAME_SZIE, PROCESS_NAME_02, MAX_PROCESS_NAME_SZIE);

    EbpfTypeAndLength ebpfTypeAndLength;
    ebpfTypeAndLength.length = sizeof(fsFixedHeader);
    ebpfTypeAndLength.type = ITEM_EVENT_FS;

    dequeBuffer.insert(dequeBuffer.end(), reinterpret_cast<uint8_t*>(&ebpfTypeAndLength),
                       reinterpret_cast<uint8_t*>(&ebpfTypeAndLength + 1));
    dequeBuffer.insert(dequeBuffer.end(), reinterpret_cast<uint8_t*>(&fsFixedHeader),
                       reinterpret_cast<uint8_t*>(&fsFixedHeader + 1));

    std::unique_ptr<EbpfDataParser> parser =
        std::make_unique<EbpfDataParser>(stream_.traceDataCache_.get(), stream_.streamFilters_.get());
    EXPECT_TRUE(parser->Init(dequeBuffer, dequeBuffer.size()));
    EXPECT_TRUE(parser->reader_->GetFileSystemEventMap().size());
    parser->ParseFileSystemEvent();
    parser->Finish();
    EXPECT_TRUE(parser->reader_->ebpfDataHeader_->header.clock == EBPF_CLOCK_BOOTTIME);
    auto callChainId = stream_.traceDataCache_->GetConstFileSystemSample().CallChainIds()[0];
    EXPECT_EQ(callChainId, INVALID_UINT32);
    auto type = stream_.traceDataCache_->GetConstFileSystemSample().Types()[0];
    EXPECT_EQ(type, CLOSE);
    auto startTs = stream_.traceDataCache_->GetConstFileSystemSample().StartTs()[0];
    EXPECT_EQ(startTs, START_TIME_02);
    auto endTs = stream_.traceDataCache_->GetConstFileSystemSample().EndTs()[0];
    EXPECT_EQ(endTs, END_TIME_02);
    auto dur = stream_.traceDataCache_->GetConstFileSystemSample().Durs()[0];
    EXPECT_EQ(dur, END_TIME_02 - START_TIME_02);
    auto ExpectReturnValue = parser->ConvertToHexTextIndex(0);
    auto returnValue = stream_.traceDataCache_->GetConstFileSystemSample().ReturnValues()[0];
    EXPECT_EQ(returnValue, ExpectReturnValue);
    auto ExpectErrorValue = parser->ConvertToHexTextIndex(-RET_02);
    auto errorCode = stream_.traceDataCache_->GetConstFileSystemSample().ErrorCodes()[0];
    EXPECT_EQ(errorCode, ExpectErrorValue);
    auto fd = stream_.traceDataCache_->GetConstFileSystemSample().Fds()[0];
    EXPECT_EQ(fd, ARGS_02[1]);
    auto fileId = stream_.traceDataCache_->GetConstFileSystemSample().FileIds()[0];
    EXPECT_EQ(fileId, INVALID_UINT64);
    auto size = stream_.traceDataCache_->GetConstFileSystemSample().Sizes()[0];
    EXPECT_EQ(size, MAX_SIZE_T);
    auto i = 0;
    auto ExpectFirstArg = parser->ConvertToHexTextIndex(ARGS_02[i++]);
    auto firstArg = stream_.traceDataCache_->GetConstFileSystemSample().FirstArguments()[0];
    EXPECT_EQ(firstArg, ExpectFirstArg);
    auto ExpectSecondArg = parser->ConvertToHexTextIndex(ARGS_02[i++]);
    auto secondArg = stream_.traceDataCache_->GetConstFileSystemSample().SecondArguments()[0];
    EXPECT_EQ(secondArg, ExpectSecondArg);
    auto ExpectThirdArg = parser->ConvertToHexTextIndex(ARGS_02[i++]);
    auto thirdArg = stream_.traceDataCache_->GetConstFileSystemSample().ThirdArguments()[0];
    EXPECT_EQ(thirdArg, ExpectThirdArg);
    auto ExpectFourthArg = parser->ConvertToHexTextIndex(ARGS_02[i]);
    auto fourthArg = stream_.traceDataCache_->GetConstFileSystemSample().FourthArguments()[0];
    EXPECT_EQ(fourthArg, ExpectFourthArg);
}

/**
 * @tc.name: ParseFileSystemWithTypeRead
 * @tc.desc: Test parse Ebpf data has one file system data with type read and no ips
 * @tc.type: FUNC
 */
HWTEST_F(EbpfFileSystemTest, ParseFileSystemWithTypeRead, TestSize.Level1)
{
    TS_LOGI("test30-3");

    EbpfDataHeader ebpfHeader;
    ebpfHeader.header.clock = EBPF_CLOCK_BOOTTIME;

    std::deque<uint8_t> dequeBuffer = {};
    dequeBuffer.insert(dequeBuffer.end(), reinterpret_cast<uint8_t*>(&ebpfHeader),
                       reinterpret_cast<uint8_t*>(&ebpfHeader + 1));

    FsFixedHeader fsFixedHeader;
    fsFixedHeader.pid = PID_01;
    fsFixedHeader.tid = TID_01;
    fsFixedHeader.startTime = START_TIME_01;
    fsFixedHeader.endTime = END_TIME_01;
    fsFixedHeader.ret = RET_01;
    fsFixedHeader.nrUserIPs = IPS_NUM_00;
    fsFixedHeader.type = SYS_READ;
    for (auto i = 0; i < ARGS_MAX; i++) {
        fsFixedHeader.args[i] = ARGS_01[i];
    }
    strncpy_s(fsFixedHeader.processName, MAX_PROCESS_NAME_SZIE, PROCESS_NAME_01, MAX_PROCESS_NAME_SZIE);

    EbpfTypeAndLength ebpfTypeAndLength;
    ebpfTypeAndLength.length = sizeof(fsFixedHeader);
    ebpfTypeAndLength.type = ITEM_EVENT_FS;

    dequeBuffer.insert(dequeBuffer.end(), reinterpret_cast<uint8_t*>(&ebpfTypeAndLength),
                       reinterpret_cast<uint8_t*>(&ebpfTypeAndLength + 1));
    dequeBuffer.insert(dequeBuffer.end(), reinterpret_cast<uint8_t*>(&fsFixedHeader),
                       reinterpret_cast<uint8_t*>(&fsFixedHeader + 1));

    std::unique_ptr<EbpfDataParser> parser =
        std::make_unique<EbpfDataParser>(stream_.traceDataCache_.get(), stream_.streamFilters_.get());
    EXPECT_TRUE(parser->Init(dequeBuffer, dequeBuffer.size()));
    EXPECT_TRUE(parser->reader_->GetFileSystemEventMap().size());
    parser->ParseFileSystemEvent();
    parser->Finish();
    EXPECT_TRUE(parser->reader_->ebpfDataHeader_->header.clock == EBPF_CLOCK_BOOTTIME);
    auto callChainId = stream_.traceDataCache_->GetConstFileSystemSample().CallChainIds()[0];
    EXPECT_EQ(callChainId, INVALID_UINT32);
    auto type = stream_.traceDataCache_->GetConstFileSystemSample().Types()[0];
    EXPECT_EQ(type, READ);
    auto startTs = stream_.traceDataCache_->GetConstFileSystemSample().StartTs()[0];
    EXPECT_EQ(startTs, START_TIME_01);
    auto endTs = stream_.traceDataCache_->GetConstFileSystemSample().EndTs()[0];
    EXPECT_EQ(endTs, END_TIME_01);
    auto dur = stream_.traceDataCache_->GetConstFileSystemSample().Durs()[0];
    EXPECT_EQ(dur, END_TIME_01 - START_TIME_01);
    auto ExpectReturnValue = parser->ConvertToHexTextIndex(RET_01);
    auto returnValue = stream_.traceDataCache_->GetConstFileSystemSample().ReturnValues()[0];
    EXPECT_EQ(returnValue, ExpectReturnValue);
    auto errorCode = stream_.traceDataCache_->GetConstFileSystemSample().ErrorCodes()[0];
    EXPECT_EQ(errorCode, INVALID_UINT64);
    auto fd = stream_.traceDataCache_->GetConstFileSystemSample().Fds()[0];
    EXPECT_EQ(fd, ARGS_01[0]);
    auto fileId = stream_.traceDataCache_->GetConstFileSystemSample().FileIds()[0];
    EXPECT_EQ(fileId, INVALID_UINT64);
    auto size = stream_.traceDataCache_->GetConstFileSystemSample().Sizes()[0];
    EXPECT_EQ(size, RET_01);
    auto i = 0;
    auto ExpectFirstArg = parser->ConvertToHexTextIndex(ARGS_01[i++]);
    auto firstArg = stream_.traceDataCache_->GetConstFileSystemSample().FirstArguments()[0];
    EXPECT_EQ(firstArg, ExpectFirstArg);
    auto ExpectSecondArg = parser->ConvertToHexTextIndex(ARGS_01[i++]);
    auto secondArg = stream_.traceDataCache_->GetConstFileSystemSample().SecondArguments()[0];
    EXPECT_EQ(secondArg, ExpectSecondArg);
    auto ExpectThirdArg = parser->ConvertToHexTextIndex(ARGS_01[i++]);
    auto thirdArg = stream_.traceDataCache_->GetConstFileSystemSample().ThirdArguments()[0];
    EXPECT_EQ(thirdArg, ExpectThirdArg);
    auto ExpectFourthArg = parser->ConvertToHexTextIndex(ARGS_01[i]);
    auto fourthArg = stream_.traceDataCache_->GetConstFileSystemSample().FourthArguments()[0];
    EXPECT_EQ(fourthArg, ExpectFourthArg);
}

/**
 * @tc.name: ParseFileSystemWithTypeWrite
 * @tc.desc: Test parse Ebpf data has one file system data with type read and no ips
 * @tc.type: FUNC
 */
HWTEST_F(EbpfFileSystemTest, ParseFileSystemWithTypeWrite, TestSize.Level1)
{
    TS_LOGI("test30-4");

    EbpfDataHeader ebpfHeader;
    ebpfHeader.header.clock = EBPF_CLOCK_BOOTTIME;

    std::deque<uint8_t> dequeBuffer = {};
    dequeBuffer.insert(dequeBuffer.end(), reinterpret_cast<uint8_t*>(&ebpfHeader),
                       reinterpret_cast<uint8_t*>(&ebpfHeader + 1));

    FsFixedHeader fsFixedHeader;
    fsFixedHeader.pid = PID_02;
    fsFixedHeader.tid = TID_02;
    fsFixedHeader.startTime = START_TIME_02;
    fsFixedHeader.endTime = END_TIME_02;
    fsFixedHeader.ret = RET_02;
    fsFixedHeader.nrUserIPs = IPS_NUM_00;
    fsFixedHeader.type = SYS_WRITE;
    for (auto i = 0; i < ARGS_MAX; i++) {
        fsFixedHeader.args[i] = ARGS_02[i];
    }
    strncpy_s(fsFixedHeader.processName, MAX_PROCESS_NAME_SZIE, PROCESS_NAME_02, MAX_PROCESS_NAME_SZIE);

    EbpfTypeAndLength ebpfTypeAndLength;
    ebpfTypeAndLength.length = sizeof(fsFixedHeader);
    ebpfTypeAndLength.type = ITEM_EVENT_FS;

    dequeBuffer.insert(dequeBuffer.end(), reinterpret_cast<uint8_t*>(&ebpfTypeAndLength),
                       reinterpret_cast<uint8_t*>(&ebpfTypeAndLength + 1));
    dequeBuffer.insert(dequeBuffer.end(), reinterpret_cast<uint8_t*>(&fsFixedHeader),
                       reinterpret_cast<uint8_t*>(&fsFixedHeader + 1));

    std::unique_ptr<EbpfDataParser> parser =
        std::make_unique<EbpfDataParser>(stream_.traceDataCache_.get(), stream_.streamFilters_.get());
    EXPECT_TRUE(parser->Init(dequeBuffer, dequeBuffer.size()));
    EXPECT_TRUE(parser->reader_->GetFileSystemEventMap().size());
    parser->ParseFileSystemEvent();
    parser->Finish();
    EXPECT_TRUE(parser->reader_->ebpfDataHeader_->header.clock == EBPF_CLOCK_BOOTTIME);
    auto callChainId = stream_.traceDataCache_->GetConstFileSystemSample().CallChainIds()[0];
    EXPECT_EQ(callChainId, INVALID_UINT32);
    auto type = stream_.traceDataCache_->GetConstFileSystemSample().Types()[0];
    EXPECT_EQ(type, WRITE);
    auto startTs = stream_.traceDataCache_->GetConstFileSystemSample().StartTs()[0];
    EXPECT_EQ(startTs, START_TIME_02);
    auto endTs = stream_.traceDataCache_->GetConstFileSystemSample().EndTs()[0];
    EXPECT_EQ(endTs, END_TIME_02);
    auto dur = stream_.traceDataCache_->GetConstFileSystemSample().Durs()[0];
    EXPECT_EQ(dur, END_TIME_02 - START_TIME_02);
    auto ExpectReturnValue = parser->ConvertToHexTextIndex(0);
    auto returnValue = stream_.traceDataCache_->GetConstFileSystemSample().ReturnValues()[0];
    EXPECT_EQ(returnValue, ExpectReturnValue);
    auto errorCode = stream_.traceDataCache_->GetConstFileSystemSample().ErrorCodes()[0];
    auto ExpectErrorValue = parser->ConvertToHexTextIndex(-RET_02);
    EXPECT_EQ(errorCode, ExpectErrorValue);
    auto fd = stream_.traceDataCache_->GetConstFileSystemSample().Fds()[0];
    EXPECT_EQ(fd, ARGS_02[0]);
    auto fileId = stream_.traceDataCache_->GetConstFileSystemSample().FileIds()[0];
    EXPECT_EQ(fileId, INVALID_UINT64);
    auto size = stream_.traceDataCache_->GetConstFileSystemSample().Sizes()[0];
    EXPECT_EQ(size, MAX_SIZE_T);
    auto i = 0;
    auto ExpectFirstArg = parser->ConvertToHexTextIndex(ARGS_02[i++]);
    auto firstArg = stream_.traceDataCache_->GetConstFileSystemSample().FirstArguments()[0];
    EXPECT_EQ(firstArg, ExpectFirstArg);
    auto ExpectSecondArg = parser->ConvertToHexTextIndex(ARGS_02[i++]);
    auto secondArg = stream_.traceDataCache_->GetConstFileSystemSample().SecondArguments()[0];
    EXPECT_EQ(secondArg, ExpectSecondArg);
    auto ExpectThirdArg = parser->ConvertToHexTextIndex(ARGS_02[i++]);
    auto thirdArg = stream_.traceDataCache_->GetConstFileSystemSample().ThirdArguments()[0];
    EXPECT_EQ(thirdArg, ExpectThirdArg);
    auto ExpectFourthArg = parser->ConvertToHexTextIndex(ARGS_02[i]);
    auto fourthArg = stream_.traceDataCache_->GetConstFileSystemSample().FourthArguments()[0];
    EXPECT_EQ(fourthArg, ExpectFourthArg);
}

/**
 * @tc.name: ParseFileSystemWithErrorType
 * @tc.desc: Test parse Ebpf data has one file system data with error type and no ips
 * @tc.type: FUNC
 */
HWTEST_F(EbpfFileSystemTest, ParseFileSystemWithErrorType, TestSize.Level1)
{
    TS_LOGI("test30-5");

    EbpfDataHeader ebpfHeader;
    ebpfHeader.header.clock = EBPF_CLOCK_BOOTTIME;

    std::deque<uint8_t> dequeBuffer = {};
    dequeBuffer.insert(dequeBuffer.end(), reinterpret_cast<uint8_t*>(&ebpfHeader),
                       reinterpret_cast<uint8_t*>(&ebpfHeader + 1));

    FsFixedHeader fsFixedHeader;
    fsFixedHeader.pid = PID_01;
    fsFixedHeader.tid = TID_01;
    fsFixedHeader.startTime = START_TIME_01;
    fsFixedHeader.endTime = END_TIME_01;
    fsFixedHeader.ret = RET_01;
    fsFixedHeader.nrUserIPs = IPS_NUM_00;
    fsFixedHeader.type = 0;
    for (auto i = 0; i < ARGS_MAX; i++) {
        fsFixedHeader.args[i] = ARGS_01[i];
    }
    strncpy_s(fsFixedHeader.processName, MAX_PROCESS_NAME_SZIE, PROCESS_NAME_01, MAX_PROCESS_NAME_SZIE);

    EbpfTypeAndLength ebpfTypeAndLength;
    ebpfTypeAndLength.length = sizeof(fsFixedHeader);
    ebpfTypeAndLength.type = ITEM_EVENT_FS;

    dequeBuffer.insert(dequeBuffer.end(), reinterpret_cast<uint8_t*>(&ebpfTypeAndLength),
                       reinterpret_cast<uint8_t*>(&ebpfTypeAndLength + 1));
    dequeBuffer.insert(dequeBuffer.end(), reinterpret_cast<uint8_t*>(&fsFixedHeader),
                       reinterpret_cast<uint8_t*>(&fsFixedHeader + 1));

    std::unique_ptr<EbpfDataParser> parser =
        std::make_unique<EbpfDataParser>(stream_.traceDataCache_.get(), stream_.streamFilters_.get());
    EXPECT_TRUE(parser->Init(dequeBuffer, dequeBuffer.size()));
    EXPECT_TRUE(parser->reader_->GetFileSystemEventMap().size());
    parser->ParseFileSystemEvent();
    parser->Finish();
    EXPECT_TRUE(parser->reader_->ebpfDataHeader_->header.clock == EBPF_CLOCK_BOOTTIME);
    EXPECT_FALSE(stream_.traceDataCache_->GetConstFileSystemSample().Size());
}

/**
 * @tc.name: ParseFileSystemWithIPsButNoSymTable
 * @tc.desc: Test parse Ebpf data has one file system data with ips but no maps
 * @tc.type: FUNC
 */
HWTEST_F(EbpfFileSystemTest, ParseFileSystemWithIPsButNoMaps, TestSize.Level1)
{
    TS_LOGI("test30-6");

    EbpfDataHeader ebpfHeader;
    ebpfHeader.header.clock = EBPF_CLOCK_BOOTTIME;

    std::deque<uint8_t> dequeBuffer = {};
    dequeBuffer.insert(dequeBuffer.end(), reinterpret_cast<uint8_t*>(&ebpfHeader),
                       reinterpret_cast<uint8_t*>(&ebpfHeader + 1));

    FsFixedHeader fsFixedHeader;
    fsFixedHeader.pid = PID_01;
    fsFixedHeader.tid = TID_01;
    fsFixedHeader.startTime = START_TIME_01;
    fsFixedHeader.endTime = END_TIME_01;
    fsFixedHeader.ret = RET_01;
    fsFixedHeader.nrUserIPs = IPS_NUM_02;
    fsFixedHeader.type = SYS_OPENAT2;
    for (auto i = 0; i < ARGS_MAX; i++) {
        fsFixedHeader.args[i] = ARGS_01[i];
    }
    strncpy_s(fsFixedHeader.processName, MAX_PROCESS_NAME_SZIE, PROCESS_NAME_01, MAX_PROCESS_NAME_SZIE);

    EbpfTypeAndLength ebpfTypeAndLength;
    ebpfTypeAndLength.length = sizeof(fsFixedHeader) + IPS_NUM_02 * sizeof(uint64_t);
    ebpfTypeAndLength.type = ITEM_EVENT_FS;

    dequeBuffer.insert(dequeBuffer.end(), reinterpret_cast<uint8_t*>(&ebpfTypeAndLength),
                       reinterpret_cast<uint8_t*>(&ebpfTypeAndLength + 1));
    dequeBuffer.insert(dequeBuffer.end(), reinterpret_cast<uint8_t*>(&fsFixedHeader),
                       reinterpret_cast<uint8_t*>(&fsFixedHeader + 1));
    dequeBuffer.insert(dequeBuffer.end(), reinterpret_cast<const uint8_t*>(IPS_02),
                       reinterpret_cast<const uint8_t*>(&IPS_02 + 1));

    std::unique_ptr<EbpfDataParser> parser =
        std::make_unique<EbpfDataParser>(stream_.traceDataCache_.get(), stream_.streamFilters_.get());
    EXPECT_TRUE(parser->Init(dequeBuffer, dequeBuffer.size()));
    EXPECT_TRUE(parser->reader_->GetFileSystemEventMap().size());
    parser->ParseFileSystemEvent();
    parser->Finish();
    EXPECT_TRUE(parser->reader_->ebpfDataHeader_->header.clock == EBPF_CLOCK_BOOTTIME);
    auto callChainId = stream_.traceDataCache_->GetConstFileSystemSample().CallChainIds()[0];
    EXPECT_EQ(callChainId, 0);
    auto callStackFirstLevelCallChainId = stream_.traceDataCache_->GetConstEbpfCallStackData().CallChainIds()[0];
    EXPECT_EQ(callStackFirstLevelCallChainId, 0);
    auto callStackSecondLevelCallChainId = stream_.traceDataCache_->GetConstEbpfCallStackData().CallChainIds()[1];
    EXPECT_EQ(callStackSecondLevelCallChainId, 0);
    auto callStackFirstLevelDepth = stream_.traceDataCache_->GetConstEbpfCallStackData().Depths()[0];
    EXPECT_EQ(callStackFirstLevelDepth, 0);
    auto callStackSecondLevelDepth = stream_.traceDataCache_->GetConstEbpfCallStackData().Depths()[1];
    EXPECT_EQ(callStackSecondLevelDepth, 1);
    auto ExpectCallStackFirstLevelIp = parser->ConvertToHexTextIndex(IPS_02[1]);
    auto callStackFirstLevelIp = stream_.traceDataCache_->GetConstEbpfCallStackData().Ips()[0];
    EXPECT_EQ(callStackFirstLevelIp, ExpectCallStackFirstLevelIp);
    auto ExpectCallStackSecondLevelIp = parser->ConvertToHexTextIndex(IPS_02[0]);
    auto callStackSecondLevelIp = stream_.traceDataCache_->GetConstEbpfCallStackData().Ips()[1];
    EXPECT_EQ(callStackSecondLevelIp, ExpectCallStackSecondLevelIp);
    auto callStackFirstLevelSymbolId = stream_.traceDataCache_->GetConstEbpfCallStackData().SymbolIds()[0];
    EXPECT_EQ(callStackFirstLevelSymbolId, INVALID_UINT64);
    auto callStackSecondLevelSymbolId = stream_.traceDataCache_->GetConstEbpfCallStackData().SymbolIds()[1];
    EXPECT_EQ(callStackSecondLevelSymbolId, INVALID_UINT64);
    auto callStackFirstLevelFilePathIds = stream_.traceDataCache_->GetConstEbpfCallStackData().FilePathIds()[0];
    EXPECT_EQ(callStackFirstLevelFilePathIds, INVALID_UINT64);
    auto callStackSecondLevelFilePathIds = stream_.traceDataCache_->GetConstEbpfCallStackData().FilePathIds()[1];
    EXPECT_EQ(callStackSecondLevelFilePathIds, INVALID_UINT64);
}
} // namespace TraceStreamer
} // namespace SysTuning
