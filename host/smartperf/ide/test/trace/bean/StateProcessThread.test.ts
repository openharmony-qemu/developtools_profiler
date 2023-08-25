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
  StateProcessThread,
  SPTChild,
  SPT,
  ThreadState,
  ThreadProcess,
  SptSlice,
} from '../../../dist/trace/bean/StateProcessThread.js';

describe('StateProcessThread Test', () => {
    it('StateProcessThreadTest01', function () {
        let stateProcessThread = new StateProcessThread();
        stateProcessThread = {
            id: 'id',
            pid: 'pid',
            title: 'title',
            process: 'process',
            processId: -1,
            thread: 'thread',
            threadId: -1,
            state: 'state',
            wallDuration: 0,
            avgDuration: 'avgDuration',
            count: 0,
            minDuration: 0,
            maxDuration: 0,
            stdDuration: 'stdDuration',
        };
        expect(stateProcessThread).not.toBeUndefined();
        expect(stateProcessThread).toMatchInlineSnapshot(
{
  id: expect.any(String),
  pid: expect.any(String),
  title: expect.any(String),
  process: expect.any(String),
  processId: expect.any(Number),
  thread: expect.any(String),
  threadId: expect.any(Number),
  wallDuration: expect.any(Number),
  avgDuration: expect.any(String),
  count: expect.any(Number),
  minDuration: expect.any(Number),
  maxDuration: expect.any(Number),
  stdDuration: expect.any(String) }, `
{
  "avgDuration": Any<String>,
  "count": Any<Number>,
  "id": Any<String>,
  "maxDuration": Any<Number>,
  "minDuration": Any<Number>,
  "pid": Any<String>,
  "process": Any<String>,
  "processId": Any<Number>,
  "state": "state",
  "stdDuration": Any<String>,
  "thread": Any<String>,
  "threadId": Any<Number>,
  "title": Any<String>,
  "wallDuration": Any<Number>,
}
`);
  });

    it('SPTChildTest02', function () {
        let sptChild = new SPTChild();
        sptChild = {
            process: 'process',
            processId: 0,
            processName: 'processName',
            thread: 'thread',
            threadId: 0,
            threadName: 'threadName',
            state: 'state',
            startNs: 0,
            startTime: 'startTime',
            duration: 0,
            cpu: 1,
            core: 'core',
            priority: 0,
            prior: 'prior',
            note: 'note',
        };
        expect(sptChild).not.toBeUndefined();
        expect(sptChild).toMatchInlineSnapshot(
{
  process: expect.any(String),
  processId: expect.any(Number),
  processName: expect.any(String),
  thread: expect.any(String),
  threadId: expect.any(Number),
  threadName: expect.any(String),
  state: expect.any(String),
  startNs: expect.any(Number),
  startTime: expect.any(String),
  duration: expect.any(Number),
  cpu: expect.any(Number),
  core: expect.any(String),
  priority: expect.any(Number),
  prior: expect.any(String),
  note: expect.any(String) }, `
{
  "core": Any<String>,
  "cpu": Any<Number>,
  "duration": Any<Number>,
  "note": Any<String>,
  "prior": Any<String>,
  "priority": Any<Number>,
  "process": Any<String>,
  "processId": Any<Number>,
  "processName": Any<String>,
  "startNs": Any<Number>,
  "startTime": Any<String>,
  "state": Any<String>,
  "thread": Any<String>,
  "threadId": Any<Number>,
  "threadName": Any<String>,
}
`
    );
  });

    it('SPTTest03', function () {
        let spt = new SPT();
        spt = {
            process: 'process',
            processId: 0,
            thread: 'thread',
            threadId: 0,
            state: 'state',
            dur: 0,
            start_ts: 0,
            end_ts: 0,
            cpu: 0,
            priority: 'priority',
            note: 'note',
        };
        expect(spt).not.toBeUndefined();
        expect(spt).toMatchInlineSnapshot(
{
  process: expect.any(String),
  processId: expect.any(Number),
  thread: expect.any(String),
  threadId: expect.any(Number),
  state: expect.any(String),
  dur: expect.any(Number),
  start_ts: expect.any(Number),
  end_ts: expect.any(Number),
  cpu: expect.any(Number),
  priority: expect.any(String),
  note: expect.any(String) }, `
{
  "cpu": Any<Number>,
  "dur": Any<Number>,
  "end_ts": Any<Number>,
  "note": Any<String>,
  "priority": Any<String>,
  "process": Any<String>,
  "processId": Any<Number>,
  "start_ts": Any<Number>,
  "state": Any<String>,
  "thread": Any<String>,
  "threadId": Any<Number>,
}
`
    );
  });

    it('ThreadStateTest04', function () {
        let threadState = new ThreadState();
        threadState = {
            itid: 0,
            state: 'state',
            dur: 0,
            ts: 0,
            end_ts: 0,
            start_ts: 0,
            cpu: 0,
        };
        expect(threadState).not.toBeUndefined();
        expect(threadState).toMatchInlineSnapshot(
{
  itid: expect.any(Number),
  state: expect.any(String),
  dur: expect.any(Number),
  ts: expect.any(Number),
  end_ts: expect.any(Number),
  start_ts: expect.any(Number),
  cpu: expect.any(Number) }, `
{
  "cpu": Any<Number>,
  "dur": Any<Number>,
  "end_ts": Any<Number>,
  "itid": Any<Number>,
  "start_ts": Any<Number>,
  "state": Any<String>,
  "ts": Any<Number>,
}
`
    );
  });

    it('ThreadProcessTest05', function () {
        let threadProcess = new ThreadProcess();
        threadProcess = {
            id: 0,
            threadId: 0,
            thread: 'thread',
            processId: 0,
            process: 'process',
        };
        expect(threadProcess).not.toBeUndefined();
        expect(threadProcess).toMatchInlineSnapshot(
{
  id: expect.any(Number),
  threadId: expect.any(Number),
  thread: expect.any(String),
  processId: expect.any(Number),
  process: expect.any(String) }, `
{
  "id": Any<Number>,
  "process": Any<String>,
  "processId": Any<Number>,
  "thread": Any<String>,
  "threadId": Any<Number>,
}
`
    );
  });

    it('SptSliceTest06', function () {
        let sptSlice = new SptSlice();
        sptSlice = {
            itid: 0,
            ts: 0,
            priority: 0,
        };
        expect(sptSlice).not.toBeUndefined();
        expect(sptSlice).toMatchInlineSnapshot(
{
  itid: expect.any(Number),
  ts: expect.any(Number),
  priority: expect.any(Number) }, `
{
  "itid": Any<Number>,
  "priority": Any<Number>,
  "ts": Any<Number>,
}
`
    );
  });

});
