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
import { SelectionParam, BoxJumpParam, SelectionData, Counter, Fps } from '../../../dist/trace/bean/BoxSelection.js';

describe('BoxSelection Test', () => {
  let selectionParam = new SelectionParam();
  let boxJumpParam = new BoxJumpParam();
  let selectionData = new SelectionData();
  let counter = new Counter();
  let fps = new Fps();
  it('BoxSelectionTest01', function () {
    selectionParam = {
      cpus: 1,
      threadIds: 2,
      trackIds: 1,
      funTids: 2,
      heapIds: 1,
      nativeMemory: 3,
      leftNs: 1,
      rightNs: 1,
      hasFps: true,
      statisticsSelectData: 1,
    };

    expect(selectionParam).not.toBeUndefined();
    expect(selectionParam).toMatchInlineSnapshot(
{
  cpus: expect.any(Number),
  threadIds: expect.any(Number),
  trackIds: expect.any(Number),
  funTids: expect.any(Number),
  heapIds: expect.any(Number),
  nativeMemory: expect.any(Number),
  leftNs: expect.any(Number),
  rightNs: expect.any(Number),
  hasFps: expect.any(Boolean) }, `
{
  "cpus": Any<Number>,
  "funTids": Any<Number>,
  "hasFps": Any<Boolean>,
  "heapIds": Any<Number>,
  "leftNs": Any<Number>,
  "nativeMemory": Any<Number>,
  "rightNs": Any<Number>,
  "statisticsSelectData": 1,
  "threadIds": Any<Number>,
  "trackIds": Any<Number>,
}
`);
  });

  it('BoxSelectionTest02', function () {
    boxJumpParam = {
      leftNs: 0,
      rightNs: 0,
      state: '',
      processId: 0,
      threadId: 0,
    };
    expect(boxJumpParam).not.toBeUndefined();
    expect(boxJumpParam).toMatchInlineSnapshot(
{
  leftNs: expect.any(Number),
  rightNs: expect.any(Number),
  state: expect.any(String),
  processId: expect.any(Number),
  threadId: expect.any(Number) }, `
{
  "leftNs": Any<Number>,
  "processId": Any<Number>,
  "rightNs": Any<Number>,
  "state": Any<String>,
  "threadId": Any<Number>,
}
`);
  });

  it('BoxSelectionTest03', function () {
    selectionData = {
      name: 'name',
      process: 'process',
      pid: 'pid',
      thread: 'thread',
      tid: 'tid',
      wallDuration: 0,
      avgDuration: 'avgDuration',
      occurrences: 0,
      state: 'state',
      trackId: 0,
      delta: 'delta',
      rate: 'rate',
      avgWeight: 'avgWeight',
      count: 'count',
      first: 'first',
      last: 'last',
      min: 'min',
      max: 'max',
      stateJX: 'stateJX',
    };
    expect(selectionData).not.toBeUndefined();
    expect(selectionData).toMatchInlineSnapshot(
{
  process: expect.any(String),
  pid: expect.any(String),
  thread: expect.any(String),
  tid: expect.any(String),
  wallDuration: expect.any(Number),
  avgDuration: expect.any(String),
  occurrences: expect.any(Number),
  state: expect.any(String),
  trackId: expect.any(Number),
  delta: expect.any(String),
  rate: expect.any(String),
  avgWeight: expect.any(String),
  count: expect.any(String),
  first: expect.any(String),
  last: expect.any(String),
  min: expect.any(String),
  max: expect.any(String),
  stateJX: expect.any(String) }, `
{
  "avgDuration": Any<String>,
  "avgWeight": Any<String>,
  "count": Any<String>,
  "delta": Any<String>,
  "first": Any<String>,
  "last": Any<String>,
  "max": Any<String>,
  "min": Any<String>,
  "name": "name",
  "occurrences": Any<Number>,
  "pid": Any<String>,
  "process": Any<String>,
  "rate": Any<String>,
  "state": Any<String>,
  "stateJX": Any<String>,
  "thread": Any<String>,
  "tid": Any<String>,
  "trackId": Any<Number>,
  "wallDuration": Any<Number>,
}
`);
  });

  it('BoxSelectionTest04', function () {
    counter = {
      id: 0,
      trackId: 0,
      name: '',
      value: 0,
      startTime: 0,
    };
    expect(counter).not.toBeUndefined();
    expect(counter).toMatchInlineSnapshot(
{
  id: expect.any(Number),
  trackId: expect.any(Number),
  name: expect.any(String),
  value: expect.any(Number),
  startTime: expect.any(Number) }, `
{
  "id": Any<Number>,
  "name": Any<String>,
  "startTime": Any<Number>,
  "trackId": Any<Number>,
  "value": Any<Number>,
}
`);
  });

  it('BoxSelectionTest05', function () {
    fps = {
      startNS: 0,
      timeStr: '',
      fps: 0,
    };
    expect(fps).not.toBeUndefined();
    expect(fps).toMatchInlineSnapshot(
{
  startNS: expect.any(Number),
  timeStr: expect.any(String),
  fps: expect.any(Number) }, `
{
  "fps": Any<Number>,
  "startNS": Any<Number>,
  "timeStr": Any<String>,
}
`);
  });
});
