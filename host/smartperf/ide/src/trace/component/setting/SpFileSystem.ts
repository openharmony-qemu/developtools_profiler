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
import { LitSelectV } from '../../../base-ui/select/LitSelectV.js';
import LitSwitch from '../../../base-ui/switch/lit-switch.js';
import '../../../base-ui/select/LitSelectV.js';
import '../../../base-ui/select/LitSelect.js';

import '../../../base-ui/switch/lit-switch.js';
import { LitSelect } from '../../../base-ui/select/LitSelect.js';
import { SpRecordTrace } from '../SpRecordTrace.js';
import { Cmd } from '../../../command/Cmd.js';
import { CmdConstant } from '../../../command/CmdConstant.js';
import { HdcDeviceManager } from '../../../hdc/HdcDeviceManager.js';

@element('sp-file-system')
export class SpFileSystem extends BaseElement {
  private processInput: LitSelectV | undefined | null;
  private maximum: HTMLInputElement | undefined | null;
  private selectProcess: HTMLInputElement | undefined | null;

  private configList: Array<any> = [];
  private list: Array<any> = [];

  private eventList: Array<any> = ['open', 'close', 'read', 'write'];

  set startRecord(start: boolean) {
    if (start) {
      this.unDisable();
      this.setAttribute('startRecord', '');
      this.selectProcess!.removeAttribute('readonly');
    } else {
      if (!this.startFileSystem && !this.startVirtualMemory && !this.startIo) {
        this.removeAttribute('startRecord');
        this.disable();
        this.selectProcess!.setAttribute('readonly', 'readonly');
      }
    }
  }

  get startRecord(): boolean {
    return this.hasAttribute('startRecord');
  }

  set startFileSystem(start: boolean) {
    if (start) {
      this.setAttribute('startSamp', '');
    } else {
      this.removeAttribute('startSamp');
    }
    this.startRecord = start;
  }

  get startFileSystem(): boolean {
    return this.hasAttribute('startSamp');
  }

  set startVirtualMemory(start: boolean) {
    if (start) {
      this.setAttribute('virtual', '');
    } else {
      this.removeAttribute('virtual');
    }
    this.startRecord = start;
  }

  get startVirtualMemory(): boolean {
    return this.hasAttribute('virtual');
  }

  set startIo(start: boolean) {
    if (start) {
      this.setAttribute('io', '');
    } else {
      this.removeAttribute('io');
    }
    this.startRecord = start;
  }

  get startIo(): boolean {
    return this.hasAttribute('io');
  }

  getSystemConfig(): SystemConfig | undefined {
    let configVal = this.shadowRoot?.querySelectorAll<HTMLElement>('.config');
    let systemConfig: SystemConfig = {
      process: '',
      unWindLevel: 0,
    };
    configVal!.forEach((value) => {
      switch (value.title) {
        case 'Process':
          let processSelect = value as LitSelectV;
          if (processSelect.all) {
            systemConfig.process = 'ALL';
            break;
          }
          if (processSelect.value.length > 0) {
            let result = processSelect.value.match(/\((.+?)\)/g);
            if (result) {
              systemConfig.process = result.toString().replaceAll('(', '').replaceAll(')', '');
            } else {
              systemConfig.process = processSelect.value;
            }
          }
          break;
        case 'Max Unwind Level':
          let maxUnwindLevel = value as HTMLInputElement;
          if (maxUnwindLevel.value != '') {
            systemConfig.unWindLevel = Number(maxUnwindLevel.value);
          }
      }
    });
    return systemConfig;
  }

  initElements(): void {
    this.initConfigList();
    let fileSystemConfigList = this.shadowRoot?.querySelector<HTMLDivElement>('.configList');
    this.configList.forEach((config) => {
      let fileSystemDiv = document.createElement('div');
      if (config.hidden) {
        fileSystemDiv.className = 'file-system-config-div hidden';
      } else {
        fileSystemDiv.className = 'file-system-config-div';
      }
      let fileSystemHeadDiv = document.createElement('div');
      fileSystemDiv.appendChild(fileSystemHeadDiv);
      let fileSystemTitle = document.createElement('span');
      fileSystemTitle.className = 'file-system-title';
      fileSystemTitle.textContent = config.title;
      fileSystemHeadDiv.appendChild(fileSystemTitle);
      let fileSystemDes = document.createElement('span');
      fileSystemDes.textContent = config.des;
      fileSystemDes.className = 'file-system-des';
      fileSystemHeadDiv.appendChild(fileSystemDes);
      switch (config.type) {
        case 'select-multiple':
          let multipleSelect = '';
          let placeholder = config.selectArray[0];
          if (config.title == 'Process') {
          } else if (config.title == 'SystemCall Event') {
            placeholder = 'ALL-Event';
          }
          multipleSelect += `<lit-select-v default-value="" rounded="" class="file-system-select config" mode="multiple" canInsert="" title="${config.title}" rounded placement = "bottom" placeholder="${placeholder}">`;
          config.selectArray.forEach((value: string) => {
            multipleSelect += `<lit-select-option value="${value}">${value}</lit-select-option>`;
          });
          multipleSelect += `</lit-select-v>`;
          fileSystemDiv.innerHTML = fileSystemDiv.innerHTML + multipleSelect;
          break;
        case 'input':
          let fileSystemInput = document.createElement('input');
          fileSystemInput.className = 'fileSystem-input config';
          fileSystemInput.textContent = config.value;
          fileSystemInput.value = config.value;
          fileSystemInput.title = config.title;
          if (config.title == 'Record Time') {
            fileSystemInput.oninput = (ev) => {
              fileSystemInput.value = fileSystemInput.value.replace(/\D/g, '');
            };
          }
          fileSystemDiv.appendChild(fileSystemInput);
          break;
        case 'select':
          let fileSystemSelect = '';
          fileSystemSelect += `<lit-select rounded="" default-value="" class="file-system-select config" placement="bottom" title="${config.title}"  placeholder="${config.selectArray[0]}">`;
          config.selectArray.forEach((value: string) => {
            fileSystemSelect += `<lit-select-option value="${value}">${value}</lit-select-option>`;
          });
          fileSystemSelect += `</lit-select>`;
          fileSystemDiv.innerHTML = fileSystemDiv.innerHTML + fileSystemSelect;
          break;
        case 'switch':
          let fileSystemSwitch = document.createElement('lit-switch') as LitSwitch;
          fileSystemSwitch.className = 'config';
          fileSystemSwitch.title = config.title;
          if (config.value) {
            fileSystemSwitch.checked = true;
          } else {
            fileSystemSwitch.checked = false;
          }
          if (config.title == 'Start FileSystem Record') {
            fileSystemSwitch.addEventListener('change', (event: any) => {
              let detail = event.detail;
              if (detail.checked) {
                this.startFileSystem = true;
              } else {
                this.startFileSystem = false;
              }
            });
          }
          if (config.title == 'Start Page Fault Record') {
            fileSystemSwitch.addEventListener('change', (event: any) => {
              let detail = event.detail;
              if (detail.checked) {
                this.startVirtualMemory = true;
              } else {
                this.startVirtualMemory = false;
              }
            });
          }
          if (config.title == 'Start BIO Latency Record') {
            fileSystemSwitch.addEventListener('change', (event: any) => {
              let detail = event.detail;
              if (detail.checked) {
                this.startIo = true;
              } else {
                this.startIo = false;
              }
            });
          }
          fileSystemHeadDiv.appendChild(fileSystemSwitch);
          break;
        default:
          break;
      }
      fileSystemConfigList!.appendChild(fileSystemDiv);
    });
    this.processInput = this.shadowRoot?.querySelector<LitSelectV>("lit-select-v[title='Process']");
    this.maximum = this.shadowRoot?.querySelector<HTMLInputElement>("input[title='Max Unwind Level']");
    this.maximum?.addEventListener('keyup', (eve: Event) => {
      this.maximum!.value = this.maximum!.value.replace(/\D/g, '');
      if (this.maximum!.value != '') {
        let mun = parseInt(this.maximum!.value);
        if (mun > 64 || mun < 0) {
          this.maximum!.value = '10';
        }
      }
    });
    this.selectProcess = this.processInput!.shadowRoot?.querySelector('input') as HTMLInputElement;
    let fileSystemProcessData: Array<string> = [];
    this.selectProcess!.addEventListener('mousedown', (ev) => {
      if (SpRecordTrace.serialNumber == '') {
        this.processInput!.dataSource([], '');
      }
    });

    this.selectProcess!.addEventListener('mouseup', () => {
      if (SpRecordTrace.serialNumber == '') {
        this.processInput?.dataSource([], 'ALL-Process');
      } else {
        if (SpRecordTrace.isVscode) {
          let cmd = Cmd.formatString(CmdConstant.CMD_GET_PROCESS_DEVICES, [SpRecordTrace.serialNumber]);
          Cmd.execHdcCmd(cmd, (res: string) => {
            fileSystemProcessData = [];
            let fileSystemValuesVs: string[] = res.replace(/\r\n/g, '\r').replace(/\n/g, '\r').split(/\r/);
            for (let lineVal of fileSystemValuesVs) {
              if (lineVal.indexOf('__progname') != -1 || lineVal.indexOf('PID CMD') != -1) {
                continue;
              }
              let fileSystemProcessVs: string[] = lineVal.trim().split(' ');
              if (fileSystemProcessVs.length == 2) {
                let processId = fileSystemProcessVs[0];
                let processName = fileSystemProcessVs[1];
                fileSystemProcessData.push(processName + '(' + processId + ')');
              }
            }
            if (fileSystemProcessData.length > 0 && this.startRecord) {
              this.processInput!.setAttribute('readonly', 'readonly');
            }
            this.processInput?.dataSource(fileSystemProcessData, 'ALL-Process');
          });
        } else {
          HdcDeviceManager.connect(SpRecordTrace.serialNumber).then((conn) => {
            if (conn) {
              HdcDeviceManager.shellResultAsString(CmdConstant.CMD_GET_PROCESS, false).then((res) => {
                fileSystemProcessData = [];
                if (res) {
                  let fileSystemValues: string[] = res.replace(/\r\n/g, '\r').replace(/\n/g, '\r').split(/\r/);
                  for (let lineVal of fileSystemValues) {
                    if (lineVal.indexOf('__progname') != -1 || lineVal.indexOf('PID CMD') != -1) {
                      continue;
                    }
                    let fileSystemProcess: string[] = lineVal.trim().split(' ');
                    if (fileSystemProcess.length == 2) {
                      let processId = fileSystemProcess[0];
                      let processName = fileSystemProcess[1];
                      fileSystemProcessData.push(processName + '(' + processId + ')');
                    }
                  }
                }
                if (fileSystemProcessData.length > 0 && this.startRecord) {
                  this.selectProcess!.setAttribute('readonly', 'readonly');
                }
                this.processInput?.dataSource(fileSystemProcessData, 'ALL-Process');
              });
            }
          });
        }
      }
    });
    this.disable();
  }

  private unDisable() {
    let fileSystemConfigVals = this.shadowRoot?.querySelectorAll<HTMLElement>('.config');
    fileSystemConfigVals!.forEach((fileSystemConfigVal) => {
      fileSystemConfigVal.removeAttribute('disabled');
    });
  }

  private disable() {
    let fileSystemConfigVals = this.shadowRoot?.querySelectorAll<HTMLElement>('.config');
    fileSystemConfigVals!.forEach((fileSystemConfigVal) => {
      if (
        fileSystemConfigVal.title == 'Start FileSystem Record' ||
        fileSystemConfigVal.title == 'Start Page Fault Record' ||
        fileSystemConfigVal.title == 'Start BIO Latency Record'
      ) {
      } else {
        fileSystemConfigVal.setAttribute('disabled', '');
      }
    });
  }

  initConfigList(): void {
    this.configList = [
      {
        title: 'Start FileSystem Record',
        des: '',
        hidden: false,
        type: 'switch',
        value: false,
      },
      {
        title: 'Start Page Fault Record',
        des: '',
        hidden: false,
        type: 'switch',
        value: false,
      },
      {
        title: 'Start BIO Latency Record',
        des: '',
        hidden: false,
        type: 'switch',
        value: false,
      },
      {
        title: 'Process',
        des: 'Record process',
        hidden: false,
        type: 'select-multiple',
        selectArray: [''],
      },
      {
        title: 'Max Unwind Level',
        des: '',
        hidden: false,
        type: 'input',
        value: '10',
      },
    ];
  }

  initHtml(): string {
    return `
        <style>
        .root {
            font-size:16px;
            margin-bottom: 30px;
            padding-top: 30px;
            padding-left: 54px;
            margin-right: 30px;
        }
        :host{
            display: inline-block;
            background: var(--dark-background3,#FFFFFF);
            border-radius: 0px 16px 16px 0px;
             width: 100%;
            height: 100%;
        }
        .file-system-config-div {
           display: flex;
           flex-direction: column;
           width: 80%;
           margin-top: 5vh;
           margin-bottom: 5vh;
           gap: 25px;
        }
        
        .file-system-title {
          line-height: 40px;
          font-weight: 700;
          margin-right: 10px;
          opacity: 0.9;
          font-family: Helvetica-Bold;
          font-size: 18px;
          text-align: center;
        }

        input {
           border-radius: 16px;
           text-indent:2%;
           height: 25px;
           outline:none;
        }
        
        .file-system-select {
          border-radius: 15px;
        }

        .file-system-des {
          line-height: 35px;
          font-weight: 400;
          opacity: 0.6;
          font-family: Helvetica;
          font-size: 14px;
          text-align: center;
        }

        lit-switch {
          height: 38px;
          margin-top: 10px;
          display:inline;
          float: right;
        }
        
        .fileSystem-input {
            color: var(--dark-color1,#212121);
            text-align: left;
            line-height: 20px;
            font-weight: 400;
            border: 1px solid var(--dark-background5,#ccc);
            font-family: Helvetica;
            font-size: 14px;
        }

        :host(:not([startSamp])) .fileSystem-input {
            color: #999999;
        }
        
         :host([startSamp]) .fileSystem-input {
            background: var(--dark-background5,#FFFFFF);
        }
        
        input::-webkit-input-placeholder{
            color:var(--bark-prompt,#999999);
        }
        </style>
        <div class="root">
            <div class="configList file-system-config">
            </div>
        </div>
        `;
  }
}

export interface SystemConfig {
  process: string;
  unWindLevel: number;
}
