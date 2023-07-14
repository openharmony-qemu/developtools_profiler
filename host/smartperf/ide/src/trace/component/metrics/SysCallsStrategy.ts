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

export const initSysCallsStrategy = (metricData: Array<{
  frequency: string;
  minDur: string;
  maxDur: string;
  avgDur: number;
  funName: string;
}>): FunctionListItem => {
  info('System Calls Strategy data length is:', metricData.length);
  let functionListItems: Array<FunctionItem> = [];
  for (let sqlIndex = 0; sqlIndex < metricData.length; sqlIndex++) {
    let functionNames = metricData[sqlIndex].funName;
    let durMaxes = metricData[sqlIndex].maxDur;
    let durMines = metricData[sqlIndex].minDur;
    let durAvgs = Math.floor(metricData[sqlIndex].avgDur).toString();
    let functionItem: FunctionItem = {
      functionName: functionNames,
      durMax: durMaxes,
      durMin: durMines,
      durAvg: durAvgs,
    };
    functionListItems?.push(functionItem);
  }
  return {
    function: functionListItems,
  };
};

export interface FunctionListItem {
  function: Array<FunctionItem>;
}

export interface FunctionItem {
  functionName: string;
  durMax: string;
  durMin: string;
  durAvg: string;
}
