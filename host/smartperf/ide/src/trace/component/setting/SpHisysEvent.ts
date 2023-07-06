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
import { LitAllocationSelect } from '../../../base-ui/select/LitAllocationSelect.js';
import { SpRecordTrace } from '../SpRecordTrace.js';
import { Cmd } from '../../../command/Cmd.js';
import { CmdConstant } from '../../../command/CmdConstant.js';
import { HdcDeviceManager } from '../../../hdc/HdcDeviceManager.js';

@element('sp-hisys-event')
export class SpHisysEvent extends BaseElement {
  private eventProcessInput: LitAllocationSelect | undefined | null;
  private selectProcess: HTMLInputElement | undefined | null;
  private eventConfigList: Array<any> = [];

  set startSamp(start: boolean) {
    if (start) {
      this.setAttribute('startSamp', '');
    } else {
      this.removeAttribute('startSamp');
      let eventInput = this.eventProcessInput?.shadowRoot?.querySelector<HTMLInputElement>('#singleInput');
      eventInput!.value = '';
    }
  }

  get process(): string {
    if (this.eventProcessInput!.value.length > 0) {
      return this.eventProcessInput!.value;
    }
    return '';
  }

  get startSamp(): boolean {
    return this.hasAttribute('startSamp');
  }

  initElements(): void {
    this.initConfigList();
    let hisysEventConfigList = this.shadowRoot?.querySelector<HTMLDivElement>('.configList');
    this.eventConfigList.forEach((config) => {
      let hisysEventDiv = document.createElement('div');
      if (config.hidden) {
        hisysEventDiv.className = 'hisys-event-config-div hidden';
      } else {
        hisysEventDiv.className = 'hisys-event-config-div';
      }
      let hisysEventHeadDiv = document.createElement('div');
      hisysEventDiv.appendChild(hisysEventHeadDiv);
      let hisysEventTitle = document.createElement('span');
      hisysEventTitle.className = 'event-title';
      hisysEventTitle.textContent = config.title;
      hisysEventHeadDiv.appendChild(hisysEventTitle);
      let hisysEventDes = document.createElement('span');
      hisysEventDes.textContent = config.des;
      hisysEventDes.className = 'event-des';
      hisysEventHeadDiv.appendChild(hisysEventDes);
      switch (config.type) {
        case 'select':
          let hisysEventSelect = '';
          hisysEventSelect += `<lit-allocation-select style="width: 100%;" rounded="" default-value="" class="event-select config" placement="bottom" title="${config.title}"  placeholder="${config.selectArray[0]}">`;
          hisysEventSelect += `</lit-allocation-select>`;
          hisysEventDiv.innerHTML = hisysEventDiv.innerHTML + hisysEventSelect;
          break;
        case 'switch':
          let hisysEventSwitch = document.createElement('lit-switch') as LitSwitch;
          hisysEventSwitch.className = 'config';
          hisysEventSwitch.title = config.title;
          if (config.value) {
            hisysEventSwitch.checked = true;
          } else {
            hisysEventSwitch.checked = false;
          }
          if (config.title == 'Start Hisystem Event Tracker Record') {
            hisysEventSwitch.addEventListener('change', (event: any) => {
              let detail = event.detail;
              if (detail.checked) {
                this.startSamp = true;
                this.unDisable();
              } else {
                this.startSamp = false;
                this.disable();
              }
            });
          }
          hisysEventHeadDiv.appendChild(hisysEventSwitch);
          break;
        default:
          break;
      }
      hisysEventConfigList!.appendChild(hisysEventDiv);
    });
    this.eventProcessInput = this.shadowRoot?.querySelector<LitAllocationSelect>(
      "lit-allocation-select[title='AppName']"
    );
    let hisyEventProcessInput = this.eventProcessInput?.shadowRoot?.querySelector('.multipleSelect') as HTMLDivElement;
    this.selectProcess = this.eventProcessInput!.shadowRoot?.querySelector('input') as HTMLInputElement;
    let hisysEventProcessData: Array<string> = [];
    hisyEventProcessInput!.addEventListener('mousedown', (ev) => {
      if (SpRecordTrace.serialNumber == '') {
        this.eventProcessInput!.processData = [];
        this.eventProcessInput!.initData();
      }
    });
    hisyEventProcessInput!.addEventListener('mouseup', () => {
      if (SpRecordTrace.serialNumber == '') {
        this.eventProcessInput!.processData = [];
        this.eventProcessInput!.initData();
      } else {
        if (SpRecordTrace.isVscode) {
          let cmd = Cmd.formatString(CmdConstant.CMD_GET_APP_NMAE_DEVICES, [SpRecordTrace.serialNumber]);
          Cmd.execHdcCmd(cmd, (res: string) => {
            hisysEventProcessData = [];
            let hisyEventValuesVs: string[] = res.replace(/\r\n/g, '\r').replace(/\n/g, '\r').split(/\r/);
            for (let lineVal of hisyEventValuesVs) {
              if (lineVal.indexOf('__progname') != -1 || lineVal.indexOf('CMD') != -1) {
                continue;
              }
              let process = lineVal.trim();
              if (process != '') {
                hisysEventProcessData.push(process);
              }
            }
            this.eventProcessInput!.processData = hisysEventProcessData;
            this.eventProcessInput!.initData();
          });
        } else {
          HdcDeviceManager.connect(SpRecordTrace.serialNumber).then((conn) => {
            if (conn) {
              HdcDeviceManager.shellResultAsString(CmdConstant.CMD_GET_APP_NMAE, false).then((res) => {
                hisysEventProcessData = [];
                if (res) {
                  let hisyEventValues: string[] = res.replace(/\r\n/g, '\r').replace(/\n/g, '\r').split(/\r/);
                  for (let lineVal of hisyEventValues) {
                    if (lineVal.indexOf('__progname') != -1 || lineVal.indexOf('CMD') != -1) {
                      continue;
                    }
                    let process = lineVal.trim();
                    if (process != '') {
                      hisysEventProcessData.push(process);
                    }
                  }
                }
                this.eventProcessInput!.processData = hisysEventProcessData;
                this.eventProcessInput!.initData();
              });
            }
          });
        }
      }
    });
    this.disable();
  }

  private unDisable() {
    let hisysEventConfigVals = this.shadowRoot?.querySelectorAll<HTMLElement>('.config');
    hisysEventConfigVals!.forEach((hisysEventConfigVal) => {
      hisysEventConfigVal.removeAttribute('disabled');
    });
  }

  private disable() {
    let hisysEventConfigVals = this.shadowRoot?.querySelectorAll<HTMLElement>('.config');
    hisysEventConfigVals!.forEach((hisysEventConfigVal) => {
      if (hisysEventConfigVal.title != 'Start Hisystem Event Tracker Record') {
        hisysEventConfigVal.setAttribute('disabled', '');
      }
    });
  }

  initConfigList(): void {
    this.eventConfigList = [
      {
        title: 'Start Hisystem Event Tracker Record',
        des: '',
        hidden: false,
        type: 'switch',
        value: false,
      },
      {
        title: 'AppName',
        des: 'Record AppName',
        hidden: false,
        type: 'select',
        selectArray: [''],
      },
    ];
  }

  initHtml(): string {
    return `
        <style>
        .root {
            margin-bottom: 30px;
            padding-top: 30px;
            padding-left: 54px;
            margin-right: 30px;
            font-size:16px;
        }

        :host{
            background: var(--dark-background3,#FFFFFF);
            display: inline-block;
            width: 100%;
            height: 100%;
            border-radius: 0px 16px 16px 0px;
        }
        
        .hisys-event-config-div {
           width: 80%;
           display: flex;
           flex-direction: column;
           gap: 25px;
           margin-top: 5vh;
           margin-bottom: 5vh;
        }
        
        .event-title {
          font-weight: 700;
          opacity: 0.9;
          font-family: Helvetica-Bold;
          font-size: 18px;
          text-align: center;
          line-height: 40px;
          margin-right: 10px;
        }

        .event-des {
          font-size: 14px;
          opacity: 0.6;
          line-height: 35px;
          font-family: Helvetica;
          text-align: center;
          font-weight: 400;
        }

        .event-select {
          border-radius: 15px;
        }

        lit-switch {
          height: 38px;
          margin-top: 10px;
          display:inline;
          float: right;
        }
        input {
           outline:none;
           height: 25px;
           border-radius: 16px;
           text-indent:2%
        }
        input::-webkit-input-placeholder{
            color:var(--bark-prompt,#999999);
        }

        .event-input {
            border: 1px solid var(--dark-background5,#ccc);
            font-family: Helvetica;
            font-size: 14px;
            color: var(--dark-color1,#212121);
            text-align: left;
            line-height: 20px;
            font-weight: 400;
        }
        </style>
        <div class="root">
            <div class="configList hisys-event-config">
            </div>
        </div>
        `;
  }
}
