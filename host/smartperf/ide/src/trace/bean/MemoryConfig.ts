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
export class MemoryConfig {
  private static instance: MemoryConfig;
  pid: number = 0;
  iPid: number = 0;
  processName: string = '';
  interval: number = 0;
  snapshotDur: number = 0;

  public static getInstance(): MemoryConfig {
    if (!this.instance) {
      this.instance = new MemoryConfig();
    }
    return this.instance;
  }

  public updateConfig(pid: number, iPid: number, processName: string, interval: number): void {
    this.pid = pid;
    this.iPid = iPid;
    this.processName = processName;
    this.interval = interval;
    this.snapshotDur = (interval * 1000 * 1000) / 5;
  }
}
