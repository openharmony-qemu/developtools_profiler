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

export const initMemoryStrategy = (metricData: Array<{
  maxNum: number;
  minNum: number;
  avgNum: number;
  name: string;
  processName: string;
}>): ProcessMetricsListItems => {
  info('Memory Strategy data length is:', metricData.length);
  let processMetricsListItems: Array<ProcessMetricsItems> = [];
  for (let sqlIndex = 0; sqlIndex < metricData.length; sqlIndex++) {
    let processName = metricData[sqlIndex].processName;
    let minNum = metricData[sqlIndex].minNum < 0 ? 0 : metricData[sqlIndex].minNum;
    let maxNum = metricData[sqlIndex].maxNum;
    let avgNum = metricData[sqlIndex].avgNum;
    let processInfoSource: ProcessMetricsItems = {
      processName: processName,
      overallCounters: {
        anonRss: {
          min: minNum,
          max: maxNum,
          avg: avgNum,
        },
      },
    };
    processMetricsListItems?.push(processInfoSource);
  }
  return {
    processMetrics: processMetricsListItems,
  };
};

export interface ProcessMetricsListItems {
  processMetrics: Array<ProcessMetricsItems>;
}

export interface ProcessMetricsItems {
  processName: string;
  overallCounters: AnonRssItem;
}

export interface AnonRssItem {
  anonRss: TypeItem;
}

export interface TypeItem {
  min: number;
  max: number;
  avg: number;
}
