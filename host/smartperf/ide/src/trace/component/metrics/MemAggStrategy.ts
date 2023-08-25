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

export const initMemoryAggStrategy = (metricData: Array<{
  processName: string;
  name: string;
  value: string;
  ts: string;
}>): ProcessValuesListItem => {
  info('Memory Agg Strategy data length is:', metricData.length);
  let processValuesListItems: Array<ProcessValuesItem> = [];
  const splitChar: string = ',';
  for (let sqlIndex = 0; sqlIndex < metricData.length; sqlIndex++) {
    let processNames = metricData[sqlIndex].processName;
    let processInfoSource: ProcessValuesItem = {
      processName: processNames,
    };
    if (metricData[sqlIndex].name === null) {
      let values = metricData[sqlIndex].value.split(splitChar);
      for (let index = 0; index < values.length; index++) {
        if (!processInfoSource) {
          continue;
        }
        processValuesListItems?.push(processInfoSource);
      }
    } else {
      let names = metricData[sqlIndex].name.split(splitChar);
      let values = metricData[sqlIndex].value.split(splitChar);
      let times = metricData[sqlIndex].ts.split(splitChar);
      let oomScoreValue = '0';
      for (let indexScore = 0; indexScore < names.length; indexScore++) {
        if ('oom_score_adj' === names[indexScore]) {
          oomScoreValue = values[indexScore];
          break;
        }
      }
      for (let index = 0; index < names.length; index++) {
        let typeItem: TypeItem = {
          ts: times[index],
          oom_score: oomScoreValue,
          value: values[index],
        };
        if (!processInfoSource) {
          continue;
        }
        if ('mem.rss.anon' === names[index]) {
          processInfoSource.anonRss = typeItem;
        }
        if ('mem.swap' === names[index]) {
          processInfoSource.swap = typeItem;
        }
        if ('mem.rss.file' === names[index]) {
          processInfoSource.fileRss = typeItem;
        }
        if ('oom_score_adj' === names[index]) {
          processInfoSource.anonAndSwap = typeItem;
        }
      }
    }
    processValuesListItems?.push(processInfoSource);
  }
  return {
    processValues: processValuesListItems,
  };
};

export interface ProcessValuesListItem {
  processValues: Array<ProcessValuesItem>;
}

export interface ProcessValuesItem {
  processName: string;
  anonRss?: TypeItem;
  swap?: TypeItem;
  fileRss?: TypeItem;
  anonAndSwap?: TypeItem;
}

export interface TypeItem {
  ts: string;
  oom_score: string;
  value: string;
}
