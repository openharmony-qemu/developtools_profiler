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

export const initMetaDataStrategy = (metricData: Array<{
  name: string;
  valueText: string;
}>): TraceMetadata => {
  info('Meta Strategy data length is:', metricData.length);
  let traceMetaDataList: Array<TraceMetadataItem> = [];
  let jsonText = '{';
  for (let index = 0; index < metricData.length; index++) {
    let name = metricData[index].name;
    let value = metricData[index].valueText;
    if (!value.match('^-?\\d+$')) {
      value = `"${  value.replace('\r|\n', '')  }"`;
    }
    jsonText += `'${  name  }'` + ': ' + `'${  value.toString()  }'` + ',';
    let indexNumber = 1;
    if (index >= metricData.length - indexNumber) {
      jsonText += '}';
    }
  }

  for (let sqlIndex = 0; sqlIndex < metricData.length; sqlIndex++) {
    let name = metricData[sqlIndex].name;
    let value = metricData[sqlIndex].valueText;
    if (!value.match('^-?\\d+$')) {
      value = `"${  value.replace('\r|\n', '')  }"`;
    }
    let traceMetaData = {
      name: name,
      value: value,
    };
    traceMetaDataList?.push(traceMetaData);
  }
  return {
    traceMetadata: traceMetaDataList,
  };
};

export interface TraceMetadata {
  traceMetadata: Array<TraceMetadataItem>;
}

export interface TraceMetadataItem {
  name: string;
  value: string;
}
