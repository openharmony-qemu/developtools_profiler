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

@element('sp-vm-tracker')
export class SpVmTracker extends BaseElement {
  private vmTrackerProcessInput: LitAllocationSelect | undefined | null;
  private vmTrackerSelectProcess: HTMLInputElement | undefined | null;
  private configList: Array<any> = [];

  set startSamp(start: boolean) {
    if (start) {
      this.setAttribute('startSamp', '');
    } else {
      this.removeAttribute('startSamp');
      let input = this.vmTrackerProcessInput?.shadowRoot?.querySelector<HTMLInputElement>('#singleInput');
      input!.value = '';
    }
  }

  get process(): string {
    if (this.vmTrackerProcessInput!.value.length > 0) {
      let result = this.vmTrackerProcessInput!.value.match(/\((.+?)\)/g);
      if (result) {
        return result.toString().replace('(', '').replace(')', '');
      } else {
        return this.vmTrackerProcessInput!.value;
      }
    }
    return '';
  }

  get startSamp(): boolean {
    return this.hasAttribute('startSamp');
  }

  initElements(): void {
    this.initConfigList();
    let configList = this.shadowRoot?.querySelector<HTMLDivElement>('.configList');
    this.configList.forEach((config) => {
      let vmTrackerDiv = document.createElement('div');
      if (config.hidden) {
        vmTrackerDiv.className = 'vm-config-div hidden';
      } else {
        vmTrackerDiv.className = 'vm-config-div';
      }
      let headDiv = document.createElement('div');
      vmTrackerDiv.appendChild(headDiv);
      let vmTrackerTitle = document.createElement('span');
      vmTrackerTitle.className = 'title';
      vmTrackerTitle.textContent = config.title;
      headDiv.appendChild(vmTrackerTitle);
      let des = document.createElement('span');
      des.textContent = config.des;
      des.className = 'des';
      headDiv.appendChild(des);
      switch (config.type) {
        case 'select':
          let html1 = '';
          html1 += `<lit-allocation-select style="width: 100%;" rounded="" default-value="" class="select config" placement="bottom" title="${config.title}"  placeholder="${config.selectArray[0]}">`;
          html1 += `</lit-allocation-select>`;
          vmTrackerDiv.innerHTML = vmTrackerDiv.innerHTML + html1;
          break;
        case 'switch':
          let vmTrackerSwitch = document.createElement('lit-switch') as LitSwitch;
          vmTrackerSwitch.className = 'config';
          vmTrackerSwitch.title = config.title;
          if (config.value) {
            vmTrackerSwitch.checked = true;
          } else {
            vmTrackerSwitch.checked = false;
          }
          if (config.title == 'Start VM Tracker Record') {
            vmTrackerSwitch.addEventListener('change', (event: any) => {
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
          headDiv.appendChild(vmTrackerSwitch);
          break;
        default:
          break;
      }
      configList!.appendChild(vmTrackerDiv);
    });
    this.vmTrackerProcessInput = this.shadowRoot?.querySelector<LitAllocationSelect>(
      "lit-allocation-select[title='Process']"
    );
    let vmTrackerMul = this.vmTrackerProcessInput?.shadowRoot?.querySelector('.multipleSelect') as HTMLDivElement;
    this.vmTrackerSelectProcess = this.vmTrackerProcessInput!.shadowRoot?.querySelector('input') as HTMLInputElement;
    let processData: Array<string> = [];
    vmTrackerMul!.addEventListener('mousedown', (ev) => {
      if (SpRecordTrace.serialNumber == '') {
        this.vmTrackerProcessInput!.processData = [];
        this.vmTrackerProcessInput!.initData();
      }
    });
    vmTrackerMul!.addEventListener('mouseup', () => {
      if (SpRecordTrace.serialNumber == '') {
        this.vmTrackerProcessInput!.processData = [];
        this.vmTrackerProcessInput!.initData();
      } else {
        if (SpRecordTrace.isVscode) {
          let vmTrackerCmd = Cmd.formatString(CmdConstant.CMD_GET_PROCESS_DEVICES, [SpRecordTrace.serialNumber]);
          Cmd.execHdcCmd(vmTrackerCmd, (res: string) => {
            processData = [];
            let lineArray: string[] = res.replace(/\r\n/g, '\r').replace(/\n/g, '\r').split(/\r/);
            for (let lineVal of lineArray) {
              if (lineVal.indexOf('__progname') != -1 || lineVal.indexOf('PID CMD') != -1) {
                continue;
              }
              let processArray: string[] = lineVal.trim().split(' ');
              if (processArray.length == 2) {
                let processId = processArray[0];
                let processName = processArray[1];
                processData.push(processName + '(' + processId + ')');
              }
            }
            this.vmTrackerProcessInput!.processData = processData;
            this.vmTrackerProcessInput!.initData();
          });
        } else {
          HdcDeviceManager.connect(SpRecordTrace.serialNumber).then((conn) => {
            if (conn) {
              HdcDeviceManager.shellResultAsString(CmdConstant.CMD_GET_PROCESS, false).then((result) => {
                processData = [];
                if (result) {
                  let lineValues: string[] = result.replace(/\r\n/g, '\r').replace(/\n/g, '\r').split(/\r/);
                  for (let lineItem of lineValues) {
                    if (lineItem.indexOf('__progname') != -1 || lineItem.indexOf('PID CMD') != -1) {
                      continue;
                    }
                    let process: string[] = lineItem.trim().split(' ');
                    if (process.length == 2) {
                      let processId = process[0];
                      let processName = process[1];
                      processData.push(processName + '(' + processId + ')');
                    }
                  }
                }
                this.vmTrackerProcessInput!.processData = processData;
                this.vmTrackerProcessInput!.initData();
              });
            }
          });
        }
      }
    });
    this.disable();
  }

  private unDisable() {
    let configVal = this.shadowRoot?.querySelectorAll<HTMLElement>('.config');
    configVal!.forEach((configVal1) => {
      configVal1.removeAttribute('disabled');
    });
  }

  private disable() {
    let configVal = this.shadowRoot?.querySelectorAll<HTMLElement>('.config');
    configVal!.forEach((configVal1) => {
      if (configVal1.title != 'Start VM Tracker Record') {
        configVal1.setAttribute('disabled', '');
      }
    });
  }

  initConfigList(): void {
    this.configList = [
      {
        title: 'Start VM Tracker Record',
        des: '',
        hidden: false,
        type: 'switch',
        value: false,
      },
      {
        title: 'Process',
        des: 'Record process',
        hidden: false,
        type: 'select',
        selectArray: [''],
      },
    ];
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

        .vm-tracker {
            font-size:16px;
            margin-bottom: 30px;
            padding-top: 30px;
            padding-left: 54px;
            margin-right: 30px;
        }
        
        .title {
          text-align: center;
          line-height: 40px;
          font-weight: 700;
          margin-right: 10px;
          opacity: 0.9;
          font-family: Helvetica-Bold;
          font-size: 18px;
        }

        .vm-config-div {
           width: 80%;
           display: flex;
           flex-direction: column;
           margin-top: 5vh;
           margin-bottom: 5vh;
           gap: 25px;
        }

        .des {
          text-align: center;
          line-height: 35px;
          font-weight: 400;
          opacity: 0.6;
          font-family: Helvetica;
          font-size: 14px;
        }

        .select {
          border-radius: 15px;
        }

        lit-switch {
          display:inline;
          float: right;
          height: 38px;
          margin-top: 10px;
        }
        
        input::-webkit-input-placeholder{
            color:var(--bark-prompt,#999999);
        }

        .input {
            text-align: left;
            line-height: 20px;
            font-weight: 400;
            border: 1px solid var(--dark-background5,#ccc);
            font-family: Helvetica;
            font-size: 14px;
            color: var(--dark-color1,#212121);
        }

        :host([startSamp]) .input {
            background: var(--dark-background5,#FFFFFF);
        }
        
        input {
           outline:none;
           border-radius: 16px;
           height: 25px;
           text-indent:2%
        }
        
        :host(:not([startSamp])) .input {
            color: #999999;
        }
        </style>
        <div class="root vm-tracker">
            <div class="configList">
            </div>
        </div>
        `;
  }
}
