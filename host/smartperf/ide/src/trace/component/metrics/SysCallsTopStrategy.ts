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

import { info } from '../../../log/Log.js';

export const initSysCallsTopStrategy = (metricData: Array<{
  tid: string;
  pid: string;
  funName: string;
  frequency: string;
  minDur: number;
  maxDur: number;
  avgDur: number;
}>): ProcessInfoListItem => {
  info('System Calls Strategy data length is:', metricData.length);
  let ProcessInfoListItems: Array<ProcessInfoItem> = [];

  for (let sqlIndex = 0; sqlIndex < metricData.length; sqlIndex++) {
    let pidList = metricData[sqlIndex].pid;
    let tidList = metricData[sqlIndex].tid;
    let functionNames = metricData[sqlIndex].funName;
    let durMaxes = metricData[sqlIndex].maxDur;
    let durMines = metricData[sqlIndex].minDur < 0 ? 0 : metricData[sqlIndex].minDur;
    let durAvgs = Math.floor(metricData[sqlIndex].avgDur);

    let processInfoItem: ProcessInfoItem = {
      pid: pidList,
      threads: {
        tid: tidList,
        function: {
          functionName: functionNames,
          durMax: durMaxes,
          durMin: durMines,
          durAvg: durAvgs,
        },
      },
    };
    ProcessInfoListItems?.push(processInfoItem);
  }
  return {
    processInfo: ProcessInfoListItems,
  };
};

export interface ProcessInfoListItem {
  processInfo: Array<ProcessInfoItem>;
}

export interface ProcessInfoItem {
  pid: string;
  threads: ThreadsItem;
}

export interface ThreadsItem {
  tid: string;
  function: FunctionItem;
}

export interface FunctionItem {
  functionName: string;
  durMax: number;
  durMin: number;
  durAvg: number;
}
