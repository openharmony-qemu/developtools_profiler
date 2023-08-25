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

import { ChartStruct } from '../../trace/bean/FrameChartStruct.js';

export class PerfFile {
  path: string = '';
  fileId: number = 0;
  symbol: string = '';
  fileName: string = '';

  static setFileName(perfData: PerfFile) {
    if (perfData.path) {
      let number = perfData.path.lastIndexOf('/');
      if (number > 0) {
        perfData.fileName = perfData.path.substring(number + 1);
        return;
      }
    }
    perfData.fileName = perfData.path;
  }
}

export class PerfThread {
  tid: number = 0;
  pid: number = 0;
  threadName: string = '';
  processName: string = '';
}

export class PerfCall {
  sampleId: number = 0;
  depth: number = 0;
  name: string = '';
}

export class PerfCallChain {
  tid: number = 0;
  pid: number = 0;
  parentId: string = ''; //合并之后区分的id
  id: string = '';
  fileId: number = 0;
  symbolId: number = 0;
  topDownMerageId: string = ''; //top down合并使用的id
  topDownMerageParentId: string = ''; //top down合并使用的id
  bottomUpMerageId: string = ''; //bottom up合并使用的id
  bottomUpMerageParentId: string = ''; //bottom up合并使用的id
  sampleId: number = 0;
  callChainId: number = 0;
  name: string = '';
  fileName: string = '';
  threadState: string = '';
  startNS: number = 0;
  dur: number = 0;
  vaddrInFile: number = 0;
  path: string = '';
  depth: number = 0;
  canCharge: boolean = true;
  previousNode: PerfCallChain | undefined = undefined; //将list转换为一个链表结构
  nextNode: PerfCallChain | undefined = undefined;
}

export class PerfCallChainMerageData extends ChartStruct {
  id: string = '';
  parentId: string = '';
  tid: number = 0;
  pid: number = 0;
  currentTreeParentNode: PerfCallChainMerageData | undefined = undefined;
  symbolName: string = '';
  libName: string = '';
  symbol: string = '';
  path: string = '';
  self: string = '0s';
  weight: string = '';
  weightPercent: string = '';
  selfDur: number = 0;
  dur: number = 0;
  children: PerfCallChainMerageData[] = [];
  initChildren: PerfCallChainMerageData[] = [];
  isStore = 0;
  isSelected: boolean = false;
  canCharge: boolean = true;
  type: number = 0;
  vaddrInFile: number = 0;
  searchShow: boolean = true;
}

export class PerfSample {
  sampleId: number = 0;
  time: number = 0;
  timeString: string = '';
  core: number = 0;
  coreName: string = '';
  state: string = '';
  pid: number = 0;
  processName: string = '';
  tid: number = 0;
  threadName: string = '';
  depth: number = 0;
  addr: string = '';
  fileId: number = 0;
  symbolId: number = 0;
  backtrace: Array<string> = [];
}

export class PerfStack {
  symbol: string = '';
  symbolId: number = 0;
  path: string = '';
  fileId: number = 0;
  type: number = 0;
  vaddrInFile: number = 0;
}

export class PerfCmdLine {
  report_value: string = '';
}
