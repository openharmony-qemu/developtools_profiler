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

export class PerfBottomUpStruct {
  parentNode: PerfBottomUpStruct | undefined;
  symbolName: string;
  selfTime: number = 0;
  selfTimeStr?: string;
  selfTimePercent: string = '';
  totalTime: number = 0;
  totalTimeStr?: string;
  totalTimePercent: string = '';
  eventCount: number = 0;
  eventPercent: string = '';
  children: Array<PerfBottomUpStruct>;
  frameChildren?: Array<PerfBottomUpStruct>;
  isSearch: boolean = false;
  isSelected: boolean = false;
  tsArray: Array<number> = [];
  constructor(symbolName: string) {
    this.symbolName = symbolName;
    this.children = [];
  }

  addChildren(child: PerfBottomUpStruct): void {
    child.parentNode = this;
    this.children.push(child);
  }

  notifyParentUpdateSelfTime(): void {
    this.parentNode?.calculateSelfTime();
  }

  calculateSelfTime(): void {
    const sum = this.children.reduce((total, obj) => total + obj.totalTime, 0);
    this.selfTime = this.totalTime - sum;
  }
}
