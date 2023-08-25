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

export const initDistributedTermData = (metricData: Array<{
  threadId: string;
  threadName: string;
  processId: string;
  processName: string;
  funName: string;
  dur: string;
  ts: string;
  chainId: string;
  spanId: string;
  parentSpanId: string;
  flag: string;
  trace_name: string;
}>): DistributedTermListItem => {
  info('Distributed Term data length is:', metricData.length);
  let distributedTermListItems: Array<DistributedTermItem> = [];
  const splitChar = ',';
  for (let sqlIndex = 0; sqlIndex < metricData.length; sqlIndex++) {
    let threadIdsList = metricData[sqlIndex].threadId.split(splitChar);
    let threadNamesList = metricData[sqlIndex].threadName.split(splitChar);
    let processIdList = metricData[sqlIndex].processId.split(splitChar);
    let processNameList =
      metricData[sqlIndex].processName === null ?
        `${threadIdsList.length  }` : metricData[sqlIndex].processName.split(splitChar);
    let funNameList = metricData[sqlIndex].funName.split(splitChar);
    let timeList = metricData[sqlIndex].ts.split(splitChar);
    let durList = metricData[sqlIndex].dur.split(splitChar);
    let flag = metricData[sqlIndex].flag;
    let flagList = flag.split(splitChar);
    let traceNameList = metricData[sqlIndex].trace_name;
    let chainIdList = metricData[sqlIndex].chainId;
    let spanIdList = metricData[sqlIndex].spanId;
    let parentSpanIdList = metricData[sqlIndex].parentSpanId;
    let distributedTermListItem: DistributedTermItem = {};
    for (let index = 0; index < flagList.length; index++) {
      let across: boolean = true;
      let receiverTime: number = 0;
      let senderTime: number = 0;
      let delay: number = 0;
      if (flag.indexOf('S,C') >= 0 || flag.indexOf('C,S') >= 0) {
        across = false;
        if (flagList[index] === 'S') {
          receiverTime = Number(timeList[index]);
        }
        if (flagList[index] === 'C') {
          senderTime = Number(timeList[index]);
        }
        delay = receiverTime - senderTime;
      }
      let type = {
        acrossTheDevice: across,
        traceName: traceNameList,
        traceId: {
          chainID: chainIdList,
          spanID: spanIdList,
          parentSpanID: parentSpanIdList,
        },
        functionName: funNameList[index],
        processInfo: {
          processId: processIdList[index],
          processName: processNameList[index],
        },
        threadInfoItem: {
          threadId: threadIdsList[index],
          threadName: threadNamesList[index],
        },
        dur: durList[index],
        delay: delay,
      };
      if ('C' === flagList[index]) {
        distributedTermListItem.sender = type;
      } else {
        distributedTermListItem.receiver = type;
      }
    }
    distributedTermListItems?.push(distributedTermListItem);
  }
  return {
    distributedTermItem: distributedTermListItems,
  };
};

export interface DistributedTermListItem {
  distributedTermItem: Array<DistributedTermItem>;
}

export interface DistributedTermItem {
  sender?: SenderOrReceiverItem;
  receiver?: SenderOrReceiverItem;
}

export interface SenderOrReceiverItem {
  acrossTheDevice?: boolean;
  traceName: string;
  traceId: TraceIdItem;
  functionName: string;
  processInfo: ProcessInfoItem;
  threadInfoItem: ThreadInfoItem;
  dur: string;
  delay: number;
}

export interface TraceIdItem {
  chainID: string;
  spanID: string;
  parentSpanID: string;
}

export interface ProcessInfoItem {
  processId: string;
  processName: string;
}

export interface ThreadInfoItem {
  threadId: string;
  threadName: string;
}
