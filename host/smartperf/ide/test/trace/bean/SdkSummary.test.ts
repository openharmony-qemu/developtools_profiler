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
import { CounterSummary, SdkSliceSummary } from '../../../dist/trace/bean/SdkSummary.js';

describe('SdkSummary Test', () => {
  it('SdkSummaryTest01', function () {
    let counterSummary = new CounterSummary();
    counterSummary = {
      value: 0,
      ts: 0,
      counter_id: 0,
    };
    expect(counterSummary).not.toBeUndefined();
    expect(counterSummary).toMatchInlineSnapshot(
{
  value: expect.any(Number),
  ts: expect.any(Number),
  counter_id: expect.any(Number) }, `
{
  "counter_id": Any<Number>,
  "ts": Any<Number>,
  "value": Any<Number>,
}
`);
  });

  it('SdkSliceSummaryTest02', function () {
    let sdkSliceSummary = new SdkSliceSummary();
    sdkSliceSummary = {
      start_ts: 0,
      end_ts: 0,
      value: 0,
      column_id: 0,
      slice_message: 'slice_message',
    };
    expect(sdkSliceSummary).not.toBeUndefined();
    expect(sdkSliceSummary).toMatchInlineSnapshot(
{
  start_ts: expect.any(Number),
  end_ts: expect.any(Number),
  value: expect.any(Number),
  column_id: expect.any(Number),
  slice_message: expect.any(String) }, `
{
  "column_id": Any<Number>,
  "end_ts": Any<Number>,
  "slice_message": Any<String>,
  "start_ts": Any<Number>,
  "value": Any<Number>,
}
`);
  });
});
