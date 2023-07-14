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

export const initTraceStateStrategy = (metricData: Array<{
  event_name: string;
  stat_type: string;
  count: number;
  source: string;
  serverity: string;
}>): StatListItem => {
  info('Trace State Strategy data length is:', metricData.length);
  let statListItems: Array<StatItem> = [];
  for (let sqlIndex = 0; sqlIndex < metricData.length; sqlIndex++) {
    let names = metricData[sqlIndex].event_name;
    let counts = metricData[sqlIndex].count;
    let sources = metricData[sqlIndex].source;
    let severities = metricData[sqlIndex].serverity;
    let statListItem: StatItem = {
      name: names,
      count: counts,
      source: sources,
      severity: severities,
    };
    statListItems?.push(statListItem);
  }
  return {
    stat: statListItems,
  };
};

export interface StatListItem {
  stat: Array<StatItem>;
}

export interface StatItem {
  name: string;
  count: number;
  source: string;
  severity: string;
}
