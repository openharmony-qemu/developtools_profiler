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
  PerfFile,
  PerfThread,
  PerfCallChain,
  PerfCallChainMerageData,
  PerfSample,
  PerfStack,
  PerfCall,
  PerfCmdLine,
} from '../../../dist/trace/bean/PerfProfile.js';

jest.mock('../../../dist/trace/component/trace/base/TraceRow.js', () => {
  return {};
});

describe('PerfProfile Test', () => {
  let perfFile = new PerfFile();
  let perfThread = new PerfThread();
  let perfCallChain = new PerfCallChain();
  let perfCallChainMerageData = new PerfCallChainMerageData();
  let perfSample = new PerfSample();
  let perfStack = new PerfStack();
  let perfCall = new PerfCall();
  let perfCmdLine = new PerfCmdLine();

  it('PerfFile Test', function () {
    perfFile = {
      fileId: 0,
      symbol: 'symbol',
      path: 'path',
      fileName: 'fileName',
    };

    expect(perfFile).not.toBeUndefined();
    expect(perfFile).toMatchInlineSnapshot(
{
  fileId: expect.any(Number),
  symbol: expect.any(String),
  path: expect.any(String),
  fileName: expect.any(String) }, `
{
  "fileId": Any<Number>,
  "fileName": Any<String>,
  "path": Any<String>,
  "symbol": Any<String>,
}
`);
  });

  it('PerfThread Test', function () {
    perfThread = {
      tid: 0,
      pid: 0,
      threadName: 'threadName',
      processName: 'processName',
    };

    expect(perfThread).not.toBeUndefined();
    expect(perfThread).toMatchInlineSnapshot(
{
  tid: expect.any(Number),
  pid: expect.any(Number),
  threadName: expect.any(String),
  processName: expect.any(String) }, `
{
  "pid": Any<Number>,
  "processName": Any<String>,
  "threadName": Any<String>,
  "tid": Any<Number>,
}
`);
  });

  it('perfCallChain Test', function () {
    perfCallChain = {
      tid: 0,
      pid: 0,
      name: 'name',
      fileName: 'fileName',
      threadState: 'threadState',
      startNS: 0,
      dur: 0,
      sampleId: 0,
      callChainId: 0,
      vaddrInFile: 0,
      fileId: 0,
      symbolId: 0,
      parentId: 'parentId',
      id: 'id',
      topDownMerageId: 'topDownMerageId',
      topDownMerageParentId: 'topDownMerageParentId',
      bottomUpMerageId: 'bottomUpMerageId',
      bottomUpMerageParentId: 'bottomUpMerageParentId',
      depth: 0,
    };

    expect(perfCallChain).not.toBeUndefined();
    expect(perfCallChain).toMatchInlineSnapshot(
{
  tid: expect.any(Number),
  pid: expect.any(Number),
  name: expect.any(String),
  fileName: expect.any(String),
  threadState: expect.any(String),
  startNS: expect.any(Number),
  dur: expect.any(Number),
  sampleId: expect.any(Number),
  callChainId: expect.any(Number),
  vaddrInFile: expect.any(Number),
  fileId: expect.any(Number),
  symbolId: expect.any(Number),
  parentId: expect.any(String),
  id: expect.any(String),
  topDownMerageId: expect.any(String),
  topDownMerageParentId: expect.any(String),
  bottomUpMerageId: expect.any(String),
  bottomUpMerageParentId: expect.any(String),
  depth: expect.any(Number) }, `
{
  "bottomUpMerageId": Any<String>,
  "bottomUpMerageParentId": Any<String>,
  "callChainId": Any<Number>,
  "depth": Any<Number>,
  "dur": Any<Number>,
  "fileId": Any<Number>,
  "fileName": Any<String>,
  "id": Any<String>,
  "name": Any<String>,
  "parentId": Any<String>,
  "pid": Any<Number>,
  "sampleId": Any<Number>,
  "startNS": Any<Number>,
  "symbolId": Any<Number>,
  "threadState": Any<String>,
  "tid": Any<Number>,
  "topDownMerageId": Any<String>,
  "topDownMerageParentId": Any<String>,
  "vaddrInFile": Any<Number>,
}
`);
  });

  it('perfCallChain Test', function () {
    perfCallChainMerageData = {
      id: 'id',
      parentId: 'parentId',
      symbolName: 'symbolName',
      symbol: 'symbol',
      libName: 'libName',
      self: 'self',
      weight: 'weight',
      selfDur: 0,
      dur: 0,
      tid: 0,
      pid: 0,
      type: 0,
      isSelected: false,
    };

    expect(perfCallChainMerageData).not.toBeUndefined();
    expect(perfCallChainMerageData).toMatchInlineSnapshot(
{
  id: expect.any(String),
  parentId: expect.any(String),
  symbolName: expect.any(String),
  symbol: expect.any(String),
  libName: expect.any(String),
  self: expect.any(String),
  weight: expect.any(String),
  selfDur: expect.any(Number),
  dur: expect.any(Number),
  tid: expect.any(Number),
  pid: expect.any(Number),
  type: expect.any(Number),
  isSelected: expect.any(Boolean) }, `
{
  "dur": Any<Number>,
  "id": Any<String>,
  "isSelected": Any<Boolean>,
  "libName": Any<String>,
  "parentId": Any<String>,
  "pid": Any<Number>,
  "self": Any<String>,
  "selfDur": Any<Number>,
  "symbol": Any<String>,
  "symbolName": Any<String>,
  "tid": Any<Number>,
  "type": Any<Number>,
  "weight": Any<String>,
}
`);
  });

  it('perfSample Test', function () {
    perfSample = {
      sampleId: 0,
      time: 0,
      timeString: 'timeString',
      core: 0,
      coreName: 'coreName',
      state: 'state',
      pid: 0,
      processName: 'processName',
      tid: 0,
      threadName: 'threadName',
      depth: 0,
      addr: 'addr',
      fileId: 0,
      symbolId: 0,
    };
    expect(perfSample).not.toBeUndefined();
    expect(perfSample).toMatchInlineSnapshot(
{
  sampleId: expect.any(Number),
  time: expect.any(Number),
  timeString: expect.any(String),
  core: expect.any(Number),
  coreName: expect.any(String),
  state: expect.any(String),
  pid: expect.any(Number),
  processName: expect.any(String),
  tid: expect.any(Number),
  threadName: expect.any(String),
  depth: expect.any(Number),
  addr: expect.any(String),
  fileId: expect.any(Number),
  symbolId: expect.any(Number) }, `
{
  "addr": Any<String>,
  "core": Any<Number>,
  "coreName": Any<String>,
  "depth": Any<Number>,
  "fileId": Any<Number>,
  "pid": Any<Number>,
  "processName": Any<String>,
  "sampleId": Any<Number>,
  "state": Any<String>,
  "symbolId": Any<Number>,
  "threadName": Any<String>,
  "tid": Any<Number>,
  "time": Any<Number>,
  "timeString": Any<String>,
}
`);
  });

  it('perfStack Test', function () {
    perfStack = {
      symbol: '',
      path: '',
      fileId: 0,
      type: 0,
    };
    expect(perfStack).not.toBeUndefined();
    expect(perfStack).toMatchInlineSnapshot(
{
  symbol: expect.any(String),
  path: expect.any(String),
  fileId: expect.any(Number),
  type: expect.any(Number) }, `
{
  "fileId": Any<Number>,
  "path": Any<String>,
  "symbol": Any<String>,
  "type": Any<Number>,
}
`);
  });

  it('perfCall Test', function () {
    perfCall = {
      sampleId: 0,
      depth: 0,
      name: 'name',
    };
    expect(perfCall).not.toBeUndefined();
    expect(perfCall).toMatchInlineSnapshot(
{
  sampleId: expect.any(Number),
  depth: expect.any(Number),
  name: expect.any(String) }, `
{
  "depth": Any<Number>,
  "name": Any<String>,
  "sampleId": Any<Number>,
}
`);
  });

  it('PerfFile Test01', function () {
    let perfFile = new PerfFile();
    perfFile.setFileName = jest.fn(() => true)
    expect(perfFile.setFileName()).toBe(true);
  });

  it('PerfFile Test02', function () {
    let perfFile = new PerfFile();
    let perfF = {
      fileId: 0,
      symbol: 'symbol',
      path: 'path',
      fileName: 'fileName',
    };
    expect(PerfFile.setFileName(perfF)).toBe(undefined);
  });

  it('PerfCmdLine Test', function () {
    perfCmdLine = {
      report_value: 'report_value',
    };
    expect(perfCmdLine).not.toBeUndefined();
    expect(perfCmdLine).toMatchInlineSnapshot(
{
  report_value: expect.any(String) }, `
{
  "report_value": Any<String>,
}
`);
  });
});
