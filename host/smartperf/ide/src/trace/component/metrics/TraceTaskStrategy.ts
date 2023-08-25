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

export const initTraceTaskStrategy = (metricData: Array<{
  id: string;
  pid: string;
  process_name: string;
  thread_name: string;
}>): ProcessListItem => {
  info('Trace Task Strategy data length is:', metricData.length);
  let statListItems = [];
  for (let sqlIndex = 0; sqlIndex < metricData.length; sqlIndex++) {
    let pidList = metricData[sqlIndex].pid;
    let processNameList = metricData[sqlIndex].process_name;
    let threadNameList = metricData[sqlIndex].thread_name;
    let threadNames = [];
    let newArr = '';
    if (threadNameList !== null) {
      threadNames = threadNameList.split(',');
      newArr = threadNames.reduce((prev: any, item: any) => (prev.includes(item) ? prev : prev.concat(item)), []);
    }

    let statListItem = {
      pid: pidList,
      processName: processNameList,
      threadName: newArr,
    };
    statListItems?.push(statListItem);
  }
  return {
    process: statListItems,
  };
};

export interface ProcessListItem {
  process: Array<any>;
}

export interface ProcessItem {
  pid: string;
  processName: string;
  threadName: string;
}
