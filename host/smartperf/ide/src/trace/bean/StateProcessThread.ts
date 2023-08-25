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

export class StateProcessThread {
  id: string = '';
  pid: string = '';
  title: string | null | undefined = '';
  children: Array<StateProcessThread> = [];
  process: string = '';
  processId: number = -1;
  thread: string = '';
  threadId: number = -1;
  state: string = '';
  wallDuration: number = 0;
  avgDuration: string = '';
  count: number = 0;
  minDuration: number = 0;
  maxDuration: number = 0;
  stdDuration: string = '';
}

export class SPTChild {
  process: string = '';
  processId: number = 0;
  processName: string = '';
  thread: string = '';
  threadId: number = 0;
  threadName: string = '';
  state: string = '';
  startNs: number = 0;
  startTime: string = '';
  absoluteTime: number = 0;
  duration: number = 0;
  cpu: number | undefined = undefined;
  core: string = '';
  priority: number = 0;
  prior: string = '';
  note: string = '-';
}

export class SPT {
  start_ts: number = 0;
  end_ts: number = 0;
  state: string = '';
  dur: number = 0;
  cpu: number = 0;
  priority: string = '-';
  note: string = '-';
  process: string = '';
  processId: number = 0;
  thread: string = '';
  threadId: number = 0;
}
