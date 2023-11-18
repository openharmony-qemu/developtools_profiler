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

const ROW_TYPE = 'hiperf';

export class PerfFireChartStruct {
  thread_id:number;
  name: string;
  depth: number;
  selfTime: number;
  totalTime: number;
  id: number;

  constructor(
    id: number,
    name: string,
    depth: number,
    selfTime: number,
    totalTime: number,
    thread_id:number,
  ) {
    this.id = id;
    this.name = name;
    this.depth = depth;
    this.selfTime = selfTime;
    this.totalTime = totalTime;
    this.thread_id = thread_id;
  }
}

// 绘图所需树结构模板
export class HiPerfChartFrame extends PerfFireChartStruct {
  startTime: number;
  endTime: number;
  children: Array<HiPerfChartFrame>;
  isSelect: boolean = false;
  line: number = 0;
  column: number = 0;
  thread_id: number=0;

  constructor(
    id: number,
    name: string,
    startTime: number,
    endTime: number,
    totalTime: number,
    depth: number,
    thread_id:number,
  ) {
    super(id, name, depth, 0, totalTime,thread_id);
    this.id = id;
    this.startTime = startTime;
    this.endTime = endTime;
    this.thread_id = thread_id;
    this.children = new Array<HiPerfChartFrame>();
  }
}