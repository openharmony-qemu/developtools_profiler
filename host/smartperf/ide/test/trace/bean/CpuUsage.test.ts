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
import { CpuUsage, Freq } from '../../../dist/trace/bean/CpuUsage.js';

describe('CpuUsage Test', () => {
  let cpuUsage = new CpuUsage();
  let freq = new Freq();

  it('CpuUsageTest01', function () {
    cpuUsage = {
      cpu: 0,
      usage: 0,
      usageStr: '',
      top1: 0,
      top2: 0,
      top3: 0,
      top1Percent: 0,
      top1PercentStr: '',
      top2Percent: 0,
      top2PercentStr: '',
      top3Percent: 0,
      top3PercentStr: '',
    };
    expect(cpuUsage).not.toBeUndefined();
    expect(cpuUsage).toMatchInlineSnapshot(
{
  cpu: expect.any(Number),
  usage: expect.any(Number),
  usageStr: expect.any(String),
  top1: expect.any(Number),
  top2: expect.any(Number),
  top3: expect.any(Number),
  top1Percent: expect.any(Number),
  top1PercentStr: expect.any(String),
  top2Percent: expect.any(Number),
  top2PercentStr: expect.any(String),
  top3Percent: expect.any(Number),
  top3PercentStr: expect.any(String) }, `
{
  "cpu": Any<Number>,
  "top1": Any<Number>,
  "top1Percent": Any<Number>,
  "top1PercentStr": Any<String>,
  "top2": Any<Number>,
  "top2Percent": Any<Number>,
  "top2PercentStr": Any<String>,
  "top3": Any<Number>,
  "top3Percent": Any<Number>,
  "top3PercentStr": Any<String>,
  "usage": Any<Number>,
  "usageStr": Any<String>,
}
`);
  });

  it('CpuUsageTest02', function () {
    cpuUsage = {
      cpu: 0,
      value: 0,
      startNs: 0,
      dur: 0,
    };
    expect(freq).not.toBeUndefined();
    expect(cpuUsage).toMatchInlineSnapshot(
{
  cpu: expect.any(Number),
  value: expect.any(Number),
  startNs: expect.any(Number),
  dur: expect.any(Number) }, `
{
  "cpu": Any<Number>,
  "dur": Any<Number>,
  "startNs": Any<Number>,
  "value": Any<Number>,
}
`);
  });
});
