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
export class MarkStruct {
  startTimeStr: string | undefined;
  endTimeStr: string | undefined;
  startTime: number | undefined;
  endTime: number | undefined;
  colorEl: HTMLInputElement | undefined;
  operate: HTMLButtonElement | undefined;
  isSelected: boolean = false;
  constructor(
    operate: HTMLButtonElement,
    colorEl?: HTMLInputElement | undefined,
    startTimeStr?: string,
    startTime?: number,
    endTimeStr?: string,
    endTime?: number
  ) {
    this.operate = operate;
    this.colorEl = colorEl;
    this.startTimeStr = startTimeStr;
    this.startTime = startTime;
    this.endTimeStr = endTimeStr;
    this.endTime = endTime;
  }
}
