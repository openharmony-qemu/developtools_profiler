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
import { SpRecordTrace } from '../SpRecordTrace.js';
import { Cmd } from '../../../command/Cmd.js';
import { LitAllocationSelect } from '../../../base-ui/select/LitAllocationSelect.js';
import { LitSelect } from '../../../base-ui/select/LitSelect.js';

@element('sp-hi-log')
export class SpHilogRecord extends BaseElement {
  private vmTrackerSwitch: LitSwitch | undefined | null;
  private processSelectEl: LitAllocationSelect | undefined | null;
  private logsSelectEl: LitSelect | undefined | null;

  get recordHilog(): boolean {
    return this.vmTrackerSwitch!.checked;
  }

  get appProcess(): string {
    return this.processSelectEl!.value || '';
  }

  get appLogLevel(): string {
    if (this.logsSelectEl!.value.trim() === '' || this.logsSelectEl!.value === 'ALL-Level') {
      return 'LEVEL_UNSPECIFIED';
    }
    return this.logsSelectEl!.value || '';
  }

  initElements(): void {
    this.vmTrackerSwitch = this.shadowRoot?.querySelector('.hilog-switch') as LitSwitch;
    this.processSelectEl = this.shadowRoot?.querySelector('.record-process-select') as LitAllocationSelect;
    this.logsSelectEl = this.shadowRoot?.querySelector('.record-logs-select') as LitSelect;
    let hiLogConfigList = this.shadowRoot?.querySelectorAll<HTMLDivElement>('.hilog-config-top');
    this.vmTrackerSwitch.addEventListener('change', ()=>{
      let configVisibility = 'none';
      if (this.vmTrackerSwitch?.checked) {
        configVisibility = 'block';
      }
      if (hiLogConfigList) {
        console.log(configVisibility);
        hiLogConfigList!.forEach(configEl => {
          configEl.style.display = configVisibility;
        });
      }
    });
    let processInputEl = this.processSelectEl.shadowRoot?.querySelector('.multipleSelect') as HTMLInputElement;
    processInputEl.addEventListener('mousedown', (ev) => {
      if (SpRecordTrace.serialNumber === '') {
        this.processSelectEl!.processData = [];
        this.processSelectEl!.initData();
      } else {
        Cmd.getProcess().then((processList) => {
          if (processList.length > 0 && this.recordHilog) {
            processInputEl!.setAttribute('readonly', 'readonly');
          }
          processList.unshift('ALL-Process');
          this.processSelectEl!.processData = processList;
          this.processSelectEl!.initData();
        });
      }
    });
  }

  attributeChangedCallback(name: string, oldValue: string, newValue: string) {
    super.attributeChangedCallback(name, oldValue, newValue);
  }

  getHiLogLevel(): string[] {
    return ['ALL-Level', 'Debug', 'Info', 'Warn', 'Error', 'Fatal'];
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
        .hilog-config-top {
           display: none;
           flex-direction: column;
           margin-top: 5vh;
           gap: 25px;
        }
        .config-title {
          line-height: 40px;
          font-weight: 700;
          margin-right: 10px;
          opacity: 0.9;
          font-family: Helvetica-Bold;
          font-size: 18px;
          text-align: center;
        }
        .config-title-des {
          line-height: 35px;
          font-weight: 400;
          opacity: 0.6;
          font-family: Helvetica;
          font-size: 14px;
          text-align: center;
        }
        .config-select{
          border-radius: 15px;
          width: 100%;
        }
        </style>
        <div class="hilog-tracker">
            <div class="hilog-config-div">
              <div>
                 <span class="hilog-title">Start Hilog Record</span>
                 <lit-switch class="hilog-switch"></lit-switch>
              </div>
              <div class="hilog-config-top">
                <div>
                  <span class="process-title config-title">Process</span>
                  <span class="config-title-des">Record process</span>
                </div>
                <lit-allocation-select default-value="" rounded="" class="record-process-select config-select" mode="multiple" canInsert="" title="Select Proces" placement="bottom" placeholder="">
                </lit-allocation-select>
              </div>
              <div class="hilog-config-top">
                <div>
                  <span class="logs-title config-title">Level</span>
                  <span class="config-title-des">Record logs level</span>
                </div>
                <lit-select default-value="" rounded="" class="record-logs-select config-select" canInsert="" title="Select Log Level" rounded placement = "bottom" placeholder=" ">
                  ${this.getHiLogLevel().map(
                    (level, index): string =>
                      '<lit-select-option class="div-button" value="' + level + '">' + level + '</lit-select-option>'
                  )
                  .join('')}
                </lit-select>
               
              </div>
            </div>
        </div>
        `;
  }
}
