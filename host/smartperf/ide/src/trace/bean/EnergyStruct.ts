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

export class PowerDetailsEnergy {
  constructor(eventName: string) {
    this.event = eventName;
  }

  event: string;
  charge: number = 0;
  background_time: number = 0;
  screen_on_time: number = 0;
  screen_off_time: number = 0;
  load: string = '-';
  usage: number = 0;
  duration: number = 0;
  camera_id: number = 0;
  foreground_count: number = 0;
  background_count: number = 0;
  screen_on_count: number = 0;
  screen_off_count: number = 0;
  count: number = 0;
  appName: string = '';
  uid: number = 0;
  foreground_duration: number = 0;
  foreground_energy: number = 0;
  background_duration: number = 0;
  background_energy: number = 0;
  screen_on_duration: number = 0;
  screen_on_energy: number = 0;
  screen_off_duration: number = 0;
  screen_off_energy: number = 0;
  energy: number = 0;
  energyConsumptionRatio: string = '';

  getTotalEnergy(isSimpleEnergy: boolean): number {
    if (isSimpleEnergy) {
      return this.energy;
    }
    return this.foreground_energy + this.background_energy + this.screen_on_energy + this.screen_off_energy;
  }
}

export class SystemDetailsEnergy {
  ts: number = 0;
  eventName: string = '';
  type: string = '';
  pid: number = -1;
  uid: number = -1;
  state: number = -1;
  workId: string = '';
  name: string = '';
  interval: number = -1;
  level: number = -1;
  tag: string = '';
  message: string = '';
  log_level: string = '';
}
