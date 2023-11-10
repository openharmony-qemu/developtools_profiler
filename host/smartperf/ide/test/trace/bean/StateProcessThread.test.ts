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
import { SliceGroup, SPTChild } from '../../../dist/trace/bean/StateProcessThread.js';

describe('StateProcessThread Test', () => {
    it('StateProcessThreadTest01', function () {
        let sliceGroup = new SliceGroup();
      sliceGroup = {
            title: 'id',
            count: 0,
            minDuration: 0,
            maxDuration: 0,
            avgDuration: 'stdDuration',
        };
        expect(sliceGroup).not.toBeUndefined();
        expect(sliceGroup).toMatchInlineSnapshot(
  {
    title: expect.any(String),
    count: expect.any(Number),
    minDuration: expect.any(Number),
    maxDuration: expect.any(Number),
    avgDuration: expect.any(String) }, `
{
  "avgDuration": Any<String>,
  "count": Any<Number>,
  "maxDuration": Any<Number>,
  "minDuration": Any<Number>,
  "title": Any<String>,
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
});
