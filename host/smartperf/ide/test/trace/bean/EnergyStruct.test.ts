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
import {PowerDetailsEnergy, SystemDetailsEnergy,} from '../../../dist/trace/bean/EnergyStruct.js';

describe('EnergyStruct Test', () => {
  let powerDetailsEnergy = new PowerDetailsEnergy();
  let systemDetailsEnergy = new SystemDetailsEnergy();

  it('powerDetailsEnergyTest', function () {
    expect(powerDetailsEnergy).not.toBeUndefined();
    expect(powerDetailsEnergy).toMatchInlineSnapshot(
{
  charge: expect.any(Number),
  background_time: expect.any(Number),
  screen_on_time: expect.any(Number),
  screen_off_time: expect.any(Number),
  load: expect.any(String),
  usage: expect.any(Number),
  duration: expect.any(Number),
  camera_id: expect.any(Number),
  foreground_count: expect.any(Number),
  background_count: expect.any(Number),
  screen_on_count: expect.any(Number),
  screen_off_count: expect.any(Number),
  count: expect.any(Number),
  appName: expect.any(String),
  uid: expect.any(Number),
  foreground_duration: expect.any(Number),
  foreground_energy: expect.any(Number),
  background_duration: expect.any(Number),
  background_energy: expect.any(Number),
  screen_on_duration: expect.any(Number),
  screen_on_energy: expect.any(Number),
  screen_off_duration: expect.any(Number),
  screen_off_energy: expect.any(Number),
  energy: expect.any(Number),
  energyConsumptionRatio: expect.any(String) }, `
{
  "appName": Any<String>,
  "background_count": Any<Number>,
  "background_duration": Any<Number>,
  "background_energy": Any<Number>,
  "background_time": Any<Number>,
  "camera_id": Any<Number>,
  "charge": Any<Number>,
  "count": Any<Number>,
  "duration": Any<Number>,
  "energy": Any<Number>,
  "energyConsumptionRatio": Any<String>,
  "event": undefined,
  "foreground_count": Any<Number>,
  "foreground_duration": Any<Number>,
  "foreground_energy": Any<Number>,
  "load": Any<String>,
  "screen_off_count": Any<Number>,
  "screen_off_duration": Any<Number>,
  "screen_off_energy": Any<Number>,
  "screen_off_time": Any<Number>,
  "screen_on_count": Any<Number>,
  "screen_on_duration": Any<Number>,
  "screen_on_energy": Any<Number>,
  "screen_on_time": Any<Number>,
  "uid": Any<Number>,
  "usage": Any<Number>,
}
`);
  });

  it('systemDetailsEnergyTest', function () {
    expect(systemDetailsEnergy).not.toBeUndefined();
    expect(systemDetailsEnergy).toMatchInlineSnapshot(
{
  eventName: expect.any(String),
  type: expect.any(String),
  pid: expect.any(Number),
  uid: expect.any(Number),
  state: expect.any(Number),
  workId: expect.any(String),
  name: expect.any(String),
  interval: expect.any(Number),
  level: expect.any(Number),
  tag: expect.any(String),
  message: expect.any(String),
  log_level: expect.any(String) }, `
{
  "eventName": Any<String>,
  "interval": Any<Number>,
  "level": Any<Number>,
  "log_level": Any<String>,
  "message": Any<String>,
  "name": Any<String>,
  "pid": Any<Number>,
  "state": Any<Number>,
  "tag": Any<String>,
  "ts": 0,
  "type": Any<String>,
  "uid": Any<Number>,
  "workId": Any<String>,
}
`);
  });
});
