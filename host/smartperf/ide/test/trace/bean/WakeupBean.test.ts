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
import { WakeupBean } from '../../../dist/trace/bean/WakeupBean.js';

describe('WakeupBean Test', () => {
  let wakeUpBean = new WakeupBean();
  it('wakeUpBean', function () {
    wakeUpBean = {
      wakeupTime: 0,
      cpu: 0,
      process: '',
      pid: 0,
      thread: '',
      tid: 0,
      schedulingLatency: 0,
      schedulingDesc: '',
      ts: 0,
    };
    expect(wakeUpBean).not.toBeUndefined();
    expect(wakeUpBean).toMatchInlineSnapshot(
{
  wakeupTime: expect.any(Number),
  cpu: expect.any(Number),
  process: expect.any(String),
  pid: expect.any(Number),
  thread: expect.any(String),
  tid: expect.any(Number),
  schedulingLatency: expect.any(Number),
  schedulingDesc: expect.any(String),
  ts: expect.any(Number) }, `
{
  "cpu": Any<Number>,
  "pid": Any<Number>,
  "process": Any<String>,
  "schedulingDesc": Any<String>,
  "schedulingLatency": Any<Number>,
  "thread": Any<String>,
  "tid": Any<Number>,
  "ts": Any<Number>,
  "wakeupTime": Any<Number>,
}
`);
  });
});
