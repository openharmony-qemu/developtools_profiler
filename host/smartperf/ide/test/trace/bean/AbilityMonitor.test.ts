/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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
//@ts-ignore
import {
  SystemCpuSummary,
  SystemDiskIOSummary,
  ProcessHistory,
  LiveProcess,
  SystemNetworkSummary,
  SystemMemorySummary,
  Dma
} from '../../../dist/trace/bean/AbilityMonitor.js';

jest.mock('../../../dist/trace/bean/NativeHook.js', () => {
  return {};
});

describe('AbilityMonitor Test', () => {
  let systemCpuSummary = new SystemCpuSummary();
  let systemDiskIOSummary = new SystemDiskIOSummary();
  let processHistory = new ProcessHistory();
  let liveProcess = new LiveProcess();
  let systemNetworkSummary = new SystemNetworkSummary();
  let systemMemorySummary = new SystemMemorySummary();
  let dma = new Dma()

  it('SystemCpuSummaryTest', function () {
    systemCpuSummary = {
      startTime: -1,
      startTimeStr: 'startTimeStr',
      duration: -1,
      durationStr: 'durationStr',
      totalLoad: -1,
      totalLoadStr: 'totalLoadStr',
      userLoad: -1,
      userLoadStr: 'userLoadStr',
      systemLoad: -1,
      systemLoadStr: 'systemLoadStr',
      threads: -1,
      threadsStr: 'threadsStr',
    };

    expect(systemCpuSummary).not.toBeUndefined();
    expect(systemCpuSummary).toMatchInlineSnapshot(
{
  startTime: expect.any(Number),
  startTimeStr: expect.any(String),
  duration: expect.any(Number),
  durationStr: expect.any(String),
  totalLoad: expect.any(Number),
  totalLoadStr: expect.any(String),
  userLoad: expect.any(Number),
  userLoadStr: expect.any(String),
  systemLoad: expect.any(Number),
  systemLoadStr: expect.any(String),
  threads: expect.any(Number),
  threadsStr: expect.any(String) }, `
{
  "duration": Any<Number>,
  "durationStr": Any<String>,
  "startTime": Any<Number>,
  "startTimeStr": Any<String>,
  "systemLoad": Any<Number>,
  "systemLoadStr": Any<String>,
  "threads": Any<Number>,
  "threadsStr": Any<String>,
  "totalLoad": Any<Number>,
  "totalLoadStr": Any<String>,
  "userLoad": Any<Number>,
  "userLoadStr": Any<String>,
}
`);
  });

  it('SystemCpuSummaryTest', function () {
    systemDiskIOSummary = {
      startTime: 1,
      startTimeStr: 'startTimeStr',
      duration: 1,
      durationStr: 'durationStr',
      dataRead: 1,
      dataReadStr: 'dataReadStr',
      dataReadSec: 1,
      dataReadSecStr: 'dataReadSecStr',
      dataWrite: 1,
      dataWriteStr: 'dataWriteStr',
      dataWriteSec: 1,
      dataWriteSecStr: 'dataWriteSecStr',
      readsIn: 1,
      readsInStr: 'readsInStr',
      readsInSec: 1,
      readsInSecStr: 'readsInSecStr',
      writeOut: 1,
      writeOutStr: 'writeOutStr',
      writeOutSec: 1,
      writeOutSecStr: 'writeOutSecStr',
    };
    expect(systemDiskIOSummary).not.toBeUndefined();
    expect(systemDiskIOSummary).toMatchInlineSnapshot(`
{
  "dataRead": 1,
  "dataReadSec": 1,
  "dataReadSecStr": "dataReadSecStr",
  "dataReadStr": "dataReadStr",
  "dataWrite": 1,
  "dataWriteSec": 1,
  "dataWriteSecStr": "dataWriteSecStr",
  "dataWriteStr": "dataWriteStr",
  "duration": 1,
  "durationStr": "durationStr",
  "readsIn": 1,
  "readsInSec": 1,
  "readsInSecStr": "readsInSecStr",
  "readsInStr": "readsInStr",
  "startTime": 1,
  "startTimeStr": "startTimeStr",
  "writeOut": 1,
  "writeOutSec": 1,
  "writeOutSecStr": "writeOutSecStr",
  "writeOutStr": "writeOutStr",
}
`);
  });


  it('ProcessHistoryTest', function () {
    processHistory = {
      processId: -1,
      alive: '',
      firstSeen: '',
      lastSeen: '',
      processName: '',
      responsibleProcess: '',
      userName: '',
      cpuTime: '',
    };
    expect(processHistory).not.toBeUndefined();
    expect(processHistory).toMatchInlineSnapshot(
{
  processId: expect.any(Number),
  alive: expect.any(String),
  firstSeen: expect.any(String),
  lastSeen: expect.any(String),
  processName: expect.any(String),
  responsibleProcess: expect.any(String),
  userName: expect.any(String),
  cpuTime: expect.any(String) }, `
{
  "alive": Any<String>,
  "cpuTime": Any<String>,
  "firstSeen": Any<String>,
  "lastSeen": Any<String>,
  "processId": Any<Number>,
  "processName": Any<String>,
  "responsibleProcess": Any<String>,
  "userName": Any<String>,
}
`);
  });

  it('LiveProcessTest', function () {
    liveProcess = {
      processId: -1,
      processName: '',
      responsibleProcess: '',
      userName: '',
      cpu: '',
      threads: -1,
    };
    expect(liveProcess).not.toBeUndefined();
    expect(liveProcess).toMatchInlineSnapshot(
{
  processId: expect.any(Number),
  processName: expect.any(String),
  responsibleProcess: expect.any(String),
  userName: expect.any(String),
  cpu: expect.any(String),
  threads: expect.any(Number) }, `
{
  "cpu": Any<String>,
  "processId": Any<Number>,
  "processName": Any<String>,
  "responsibleProcess": Any<String>,
  "threads": Any<Number>,
  "userName": Any<String>,
}
`);
  });

  it('SystemNetworkSummaryTest', function () {
    systemNetworkSummary = {
      startTime: -1,
      startTimeStr: '',
      duration: -1,
      durationStr: '',
      dataReceived: -1,
      dataReceivedStr: '',
      dataReceivedSec: -1,
      dataReceivedSecStr: '',
      dataSend: -1,
      dataSendStr: '',
      dataSendSec: -1,
      dataSendSecStr: '',
      packetsIn: -1,
      packetsInSec: -1,
      packetsOut: -1,
      packetsOutSec: -1,
    };
    expect(systemNetworkSummary).not.toBeUndefined();
    expect(systemNetworkSummary).toMatchInlineSnapshot(
{
  startTime: expect.any(Number),
  startTimeStr: expect.any(String),
  duration: expect.any(Number),
  durationStr: expect.any(String),
  dataReceived: expect.any(Number),
  dataReceivedStr: expect.any(String),
  dataReceivedSec: expect.any(Number),
  dataReceivedSecStr: expect.any(String),
  dataSend: expect.any(Number),
  dataSendStr: expect.any(String),
  dataSendSec: expect.any(Number),
  dataSendSecStr: expect.any(String),
  packetsIn: expect.any(Number),
  packetsInSec: expect.any(Number),
  packetsOut: expect.any(Number),
  packetsOutSec: expect.any(Number) }, `
{
  "dataReceived": Any<Number>,
  "dataReceivedSec": Any<Number>,
  "dataReceivedSecStr": Any<String>,
  "dataReceivedStr": Any<String>,
  "dataSend": Any<Number>,
  "dataSendSec": Any<Number>,
  "dataSendSecStr": Any<String>,
  "dataSendStr": Any<String>,
  "duration": Any<Number>,
  "durationStr": Any<String>,
  "packetsIn": Any<Number>,
  "packetsInSec": Any<Number>,
  "packetsOut": Any<Number>,
  "packetsOutSec": Any<Number>,
  "startTime": Any<Number>,
  "startTimeStr": Any<String>,
}
`);
  });

  it('systemMemorySummaryTest', function () {
    systemMemorySummary = {
      startTime: -1,
      startTimeStr: '',
      duration: -1,
      durationStr: '',
      memoryTotal: -1,
      memoryTotalStr: '',
      cached: -1,
      cachedStr: '',
      swapTotal: -1,
      swapTotalStr: '',
      appMemory: -1,
      cachedFiles: -1,
      compressed: -1,
      memoryUsed: -1,
      wiredMemory: -1,
      swapUsed: -1,
    };
    expect(systemMemorySummary).not.toBeUndefined();
    expect(systemMemorySummary).toMatchInlineSnapshot(
{
  startTime: expect.any(Number),
  startTimeStr: expect.any(String),
  duration: expect.any(Number),
  durationStr: expect.any(String),
  memoryTotal: expect.any(Number),
  memoryTotalStr: expect.any(String),
  cached: expect.any(Number),
  cachedStr: expect.any(String),
  swapTotal: expect.any(Number),
  swapTotalStr: expect.any(String),
  appMemory: expect.any(Number),
  cachedFiles: expect.any(Number),
  compressed: expect.any(Number),
  memoryUsed: expect.any(Number),
  wiredMemory: expect.any(Number),
  swapUsed: expect.any(Number) }, `
{
  "appMemory": Any<Number>,
  "cached": Any<Number>,
  "cachedFiles": Any<Number>,
  "cachedStr": Any<String>,
  "compressed": Any<Number>,
  "duration": Any<Number>,
  "durationStr": Any<String>,
  "memoryTotal": Any<Number>,
  "memoryTotalStr": Any<String>,
  "memoryUsed": Any<Number>,
  "startTime": Any<Number>,
  "startTimeStr": Any<String>,
  "swapTotal": Any<Number>,
  "swapTotalStr": Any<String>,
  "swapUsed": Any<Number>,
  "wiredMemory": Any<Number>,
}
`);
  });
  it('DmaTest', function () {
    dma = {
      processId: -1,
      timeStamp:'',
      startNs:-1,
      expTaskComm:  '',
      avgSize: -1,
      minSize:-1,
      maxSize: -1,
      bufName:'',
      expName:  '',
      size: -1,
      processName: '',
      process: '',
      fd: -1,
      ino: -1,
      expPid: -1,
      flag:  -1,
      avgSizes: '',
      minSizes: '',
      maxSizes: '',
      sizes:  '',
      sumSize: -1,
      sumSizes: '',
    };
    expect(dma).not.toBeUndefined();
    expect(dma).toMatchInlineSnapshot(`
{
  "avgSize": -1,
  "avgSizes": "",
  "bufName": "",
  "expName": "",
  "expPid": -1,
  "expTaskComm": "",
  "fd": -1,
  "flag": -1,
  "ino": -1,
  "maxSize": -1,
  "maxSizes": "",
  "minSize": -1,
  "minSizes": "",
  "process": "",
  "processId": -1,
  "processName": "",
  "size": -1,
  "sizes": "",
  "startNs": -1,
  "sumSize": -1,
  "sumSizes": "",
  "timeStamp": "",
}
`);
  });
});
