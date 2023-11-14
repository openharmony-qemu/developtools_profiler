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

// @ts-ignore
import {
  NativeHookStatistics,
  NativeHookMalloc,
  NativeEventHeap,
  NativeHookProcess,
  NativeHookStatisticsTableData,
  NativeMemory,
  NativeHookSamplerInfo,
  NativeHookSampleQueryInfo,
  NativeHookCallInfo,
  NativeEvent,
} from '../../../dist/trace/bean/NativeHook.js';
jest.mock('../../../dist/trace/component/trace/base/TraceRow.js', () => {
  return {};
});

describe('NativeHook Test', () => {
  it('NativeHookStatisticsTest01', function () {
    let nativeHookStatistics = new NativeHookStatistics();
    nativeHookStatistics = {
      eventId: 0,
      eventType: 'eventType',
      subType: 'subType',
      heapSize: 0,
      addr: 'addr',
      startTs: 0,
      endTs: 0,
      sumHeapSize: 0,
      max: 0,
      count: 0,
      tid: 0,
      isSelected: false,
    };
    expect(nativeHookStatistics).not.toBeUndefined();
    expect(nativeHookStatistics).toMatchInlineSnapshot(
{
  eventId: expect.any(Number),
  eventType: expect.any(String),
  subType: expect.any(String),
  heapSize: expect.any(Number),
  addr: expect.any(String),
  startTs: expect.any(Number),
  endTs: expect.any(Number),
  sumHeapSize: expect.any(Number),
  max: expect.any(Number),
  count: expect.any(Number),
  tid: expect.any(Number),
  isSelected: expect.any(Boolean) }, `
{
  "addr": Any<String>,
  "count": Any<Number>,
  "endTs": Any<Number>,
  "eventId": Any<Number>,
  "eventType": Any<String>,
  "heapSize": Any<Number>,
  "isSelected": Any<Boolean>,
  "max": Any<Number>,
  "startTs": Any<Number>,
  "subType": Any<String>,
  "sumHeapSize": Any<Number>,
  "tid": Any<Number>,
}
`);
  });

  it('NativeEventHeapTest02', function () {
    let nativeHookMalloc = new NativeHookMalloc();
    nativeHookMalloc = {
      eventType: 'eventType',
      subType: 'subType',
      heapSize: 0,
      allocByte: 0,
      allocCount: 0,
      freeByte: 0,
      freeCount: 0,
    };
    expect(nativeHookMalloc).not.toBeUndefined();
    expect(nativeHookMalloc).toMatchInlineSnapshot(
{
  eventType: expect.any(String),
  subType: expect.any(String),
  heapSize: expect.any(Number),
  allocByte: expect.any(Number),
  allocCount: expect.any(Number),
  freeByte: expect.any(Number),
  freeCount: expect.any(Number) }, `
{
  "allocByte": Any<Number>,
  "allocCount": Any<Number>,
  "eventType": Any<String>,
  "freeByte": Any<Number>,
  "freeCount": Any<Number>,
  "heapSize": Any<Number>,
  "subType": Any<String>,
}
`);
  });

  it('NativeEventHeapTest03', function () {
    let nativeEventHeap = new NativeEventHeap();
    nativeEventHeap = {
      eventType: 'eventType',
      sumHeapSize: 0,
    };
    expect(nativeEventHeap).not.toBeUndefined();
    expect(nativeEventHeap).toMatchInlineSnapshot(
{
  eventType: expect.any(String),
  sumHeapSize: expect.any(Number) }, `
{
  "eventType": Any<String>,
  "sumHeapSize": Any<Number>,
}
`);
  });

  it('NativeHookProcessTest04', function () {
    let nativeHookProcess = new NativeHookProcess();
    nativeHookProcess = {
      ipid: 0,
      pid: 0,
      name: 'name',
    };
    expect(nativeHookProcess).not.toBeUndefined();
    expect(nativeHookProcess).toMatchInlineSnapshot(
{
  ipid: expect.any(Number),
  pid: expect.any(Number),
  name: expect.any(String) }, `
{
  "ipid": Any<Number>,
  "name": Any<String>,
  "pid": Any<Number>,
}
`);
  });

  it('NativeHookStatisticsTableDataTest05', function () {
    let nativeHookStatisticsTableData = new NativeHookStatisticsTableData();
    nativeHookStatisticsTableData = {
      memoryTap: '',
      existing: 0,
      existingString: '',
      allocCount: 0,
      freeCount: 0,
      totalBytes: 0,
      totalBytesString: '',
      maxStr: '',
      max: 0,
      totalCount: 0,
    };
    expect(nativeHookStatisticsTableData).not.toBeUndefined();
    expect(nativeHookStatisticsTableData).toMatchInlineSnapshot(
{
  memoryTap: expect.any(String),
  existing: expect.any(Number),
  existingString: expect.any(String),
  allocCount: expect.any(Number),
  freeCount: expect.any(Number),
  totalBytes: expect.any(Number),
  totalBytesString: expect.any(String),
  maxStr: expect.any(String),
  max: expect.any(Number),
  totalCount: expect.any(Number) }, `
{
  "allocCount": Any<Number>,
  "existing": Any<Number>,
  "existingString": Any<String>,
  "freeCount": Any<Number>,
  "max": Any<Number>,
  "maxStr": Any<String>,
  "memoryTap": Any<String>,
  "totalBytes": Any<Number>,
  "totalBytesString": Any<String>,
  "totalCount": Any<Number>,
}
`);
  });

  it('NativeMemoryTest06', function () {
    let nativeMemory = new NativeMemory();
    nativeMemory = {
      index: 0,
      eventId: 0,
      eventType: 'eventType',
      subType: 'subType',
      addr: 'addr',
      startTs: 0,
      timestamp: 'timestamp',
      heapSize: 0,
      heapSizeUnit: 'heapSizeUnit',
      symbol: 'symbol',
      library: 'library',
      isSelected: false,
    };
    expect(nativeMemory).not.toBeUndefined();
    expect(nativeMemory).toMatchInlineSnapshot(
{
  index: expect.any(Number),
  eventId: expect.any(Number),
  eventType: expect.any(String),
  subType: expect.any(String),
  addr: expect.any(String),
  startTs: expect.any(Number),
  timestamp: expect.any(String),
  heapSize: expect.any(Number),
  heapSizeUnit: expect.any(String),
  symbol: expect.any(String),
  library: expect.any(String),
  isSelected: expect.any(Boolean) }, `
{
  "addr": Any<String>,
  "eventId": Any<Number>,
  "eventType": Any<String>,
  "heapSize": Any<Number>,
  "heapSizeUnit": Any<String>,
  "index": Any<Number>,
  "isSelected": Any<Boolean>,
  "library": Any<String>,
  "startTs": Any<Number>,
  "subType": Any<String>,
  "symbol": Any<String>,
  "timestamp": Any<String>,
}
`);
  });

  it('NativeHookCallInfoTest07', function () {
    let nativeHookSamplerInfo = new NativeHookSamplerInfo();
    nativeHookSamplerInfo = {
      current: 'current',
      currentSize: 0,
      startTs: 0,
      heapSize: 0,
      snapshot: 'snapshot',
      growth: 'growth',
      total: 0,
      totalGrowth: 'totalGrowth',
      existing: 0,
      timestamp: 'timestamp',
      eventId: -1,
    };
    expect(nativeHookSamplerInfo).not.toBeUndefined();
    expect(nativeHookSamplerInfo).toMatchInlineSnapshot(
{
  current: expect.any(String),
  currentSize: expect.any(Number),
  startTs: expect.any(Number),
  heapSize: expect.any(Number),
  snapshot: expect.any(String),
  growth: expect.any(String),
  total: expect.any(Number),
  totalGrowth: expect.any(String),
  existing: expect.any(Number),
  timestamp: expect.any(String),
  eventId: expect.any(Number) }, `
{
  "current": Any<String>,
  "currentSize": Any<Number>,
  "eventId": Any<Number>,
  "existing": Any<Number>,
  "growth": Any<String>,
  "heapSize": Any<Number>,
  "snapshot": Any<String>,
  "startTs": Any<Number>,
  "timestamp": Any<String>,
  "total": Any<Number>,
  "totalGrowth": Any<String>,
}
`);
  });

  it('NativeHookCallInfoTest08', function () {
    let nativeHookSampleQueryInfo = new NativeHookSampleQueryInfo();
    nativeHookSampleQueryInfo = {
      eventId: -1,
      current: 0,
      eventType: 'eventType',
      subType: 'subType',
      growth: 0,
      existing: 0,
      addr: 'addr',
      startTs: 0,
      endTs: 0,
      total: 0,
    };

    expect(nativeHookSampleQueryInfo).not.toBeUndefined();
    expect(nativeHookSampleQueryInfo).toMatchInlineSnapshot(
{
  eventId: expect.any(Number),
  current: expect.any(Number),
  eventType: expect.any(String),
  subType: expect.any(String),
  growth: expect.any(Number),
  existing: expect.any(Number),
  addr: expect.any(String),
  startTs: expect.any(Number),
  endTs: expect.any(Number),
  total: expect.any(Number) }, `
{
  "addr": Any<String>,
  "current": Any<Number>,
  "endTs": Any<Number>,
  "eventId": Any<Number>,
  "eventType": Any<String>,
  "existing": Any<Number>,
  "growth": Any<Number>,
  "startTs": Any<Number>,
  "subType": Any<String>,
  "total": Any<Number>,
}
`);
  });

  it('NativeHookCallInfoTest09', function () {
    let nativeHookCallInfo = new NativeHookCallInfo();
    nativeHookCallInfo = {
      id: 'id',
      pid: 'pid',
      library: 'library',
      title: 'title',
      count: 0,
      type: 0,
      heapSize: 0,
      heapSizeStr: 'heapSizeStr',
      eventId: 0,
      threadId: 0,
      isSelected: false,
    };
    expect(nativeHookCallInfo).not.toBeUndefined();
    expect(nativeHookCallInfo).toMatchInlineSnapshot(
{
  id: expect.any(String),
  pid: expect.any(String),
  library: expect.any(String),
  title: expect.any(String),
  count: expect.any(Number),
  type: expect.any(Number),
  heapSize: expect.any(Number),
  heapSizeStr: expect.any(String),
  eventId: expect.any(Number),
  threadId: expect.any(Number),
  isSelected: expect.any(Boolean) }, `
{
  "count": Any<Number>,
  "eventId": Any<Number>,
  "heapSize": Any<Number>,
  "heapSizeStr": Any<String>,
  "id": Any<String>,
  "isSelected": Any<Boolean>,
  "library": Any<String>,
  "pid": Any<String>,
  "threadId": Any<Number>,
  "title": Any<String>,
  "type": Any<Number>,
}
`);
  });

  it('NativeHookCallInfoTest10', function () {
    let nativeHookSamplerInfo = new NativeHookSamplerInfo();
    expect(nativeHookSamplerInfo.merageObj(NativeHookSamplerInfo)).toBeUndefined();
  });

  it('NativeEventHeapTest11', function () {
    let nativeEvent = new NativeEvent();
    nativeEvent = {
      startTime: 0,
      heapSize: 0,
      eventType: '',
    };
    expect(nativeEvent).not.toBeUndefined();
    expect(nativeEvent).toMatchInlineSnapshot(
{
  startTime: expect.any(Number),
  heapSize: expect.any(Number),
  eventType: expect.any(String) }, `
{
  "eventType": Any<String>,
  "heapSize": Any<Number>,
  "startTime": Any<Number>,
}
`);
  });
});
