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

import { BaseElement, element } from '../../../base-ui/BaseElement.js';
import LitSwitch from '../../../base-ui/switch/lit-switch.js';
import '../../../base-ui/select/LitAllocationSelect.js';

import '../../../base-ui/switch/lit-switch.js';

@element('sp-hi-log')
export class SpHilogRecord extends BaseElement {
  private vmTrackerSwitch: LitSwitch | undefined | null;

  get recordHilog(): boolean {
    return this.vmTrackerSwitch!.checked;
  }

  initElements(): void {
    this.vmTrackerSwitch = this.shadowRoot?.querySelector('.hilog-switch') as LitSwitch;
  }

  initHtml(): string {
    return `
        <style>
        :host{
            background: var(--dark-background3,#FFFFFF);
            border-radius: 0px 16px 16px 0px;
            display: inline-block;
            width: 100%;
            height: 100%;
        }
        .hilog-tracker {
            font-size:16px;
            margin-bottom: 30px;
            padding-top: 30px;
            padding-left: 54px;
            margin-right: 30px;
        }
        .hilog-config-div {
           width: 80%;
           display: flex;
           flex-direction: column;
           margin-top: 5vh;
           margin-bottom: 5vh;
           gap: 25px;
        }
        
        .hilog-title {
          text-align: center;
          line-height: 40px;
          font-weight: 700;
          margin-right: 10px;
          opacity: 0.9;
          font-family: Helvetica-Bold;
          font-size: 18px;
        }
        .hilog-switch {
          display:inline;
          float: right;
          height: 38px;
          margin-top: 10px;
        }
        </style>
        <div class="hilog-tracker">
            <div class="hilog-config-div">
              <div>
                 <span class="hilog-title">Start Hilog Record</span>
                 <lit-switch class="hilog-switch"></lit-switch>
              </div>
            </div>
        </div>
        `;
  }
}
