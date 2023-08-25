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
import LitSwitch, { LitSwitchChangeEvent } from '../../../base-ui/switch/lit-switch.js';
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
          if (maxUnwindLevel.value !== '') {
            systemConfig.unWindLevel = Number(maxUnwindLevel.value);
          }
      }
    });
    return systemConfig;
  }

  initElements(): void {
    this.switchChange();
    this.processInput = this.shadowRoot?.querySelector<LitSelectV>('lit-select-v');
    this.maximum = this.shadowRoot?.querySelector<HTMLInputElement>('#maxUnwindLevel');
    this.maximum?.addEventListener('keyup', (eve: Event) => {
      this.maximum!.value = this.maximum!.value.replace(/\D/g, '');
      if (this.maximum!.value !== '') {
        let mun = parseInt(this.maximum!.value);
        if (mun > 64 || mun < 0) {
          this.maximum!.value = '10';
        }
      }
    });
    this.selectProcess = this.processInput!.shadowRoot?.querySelector('input') as HTMLInputElement;
    this.selectProcess!.addEventListener('mousedown', (ev) => {
      if (SpRecordTrace.serialNumber === '') {
        this.processInput!.dataSource([], '');
      } else {
        Cmd.getProcess().then((processList) => {
          if (processList.length > 0 && this.startRecord) {
            this.selectProcess!.setAttribute('readonly', 'readonly');
          }
          this.processInput?.dataSource(processList, 'ALL-Process');
        });
      }
    });
    this.disable();
  }

  private switchChange(): void {
    let fileSystemSwitch = this.shadowRoot?.querySelector<LitSwitch>('#fileSystem');
    fileSystemSwitch!.addEventListener('change', (event: CustomEventInit<LitSwitchChangeEvent>) => {
      let detail = event.detail;
      if (detail!.checked) {
        this.startFileSystem = true;
      } else {
        this.startFileSystem = false;
      }
    });
    let pageFaultSwitch = this.shadowRoot?.querySelector<LitSwitch>('#pageFault');
    pageFaultSwitch!.addEventListener('change', (event: CustomEventInit<LitSwitchChangeEvent>) => {
      let detail = event.detail;
      if (detail!.checked) {
        this.startVirtualMemory = true;
      } else {
        this.startVirtualMemory = false;
      }
    });
    let bioLatencySwitch = this.shadowRoot?.querySelector<LitSwitch>('#bioLatency');
    bioLatencySwitch!.addEventListener('change', (event: CustomEventInit<LitSwitchChangeEvent>) => {
      let detail = event.detail;
      if (detail!.checked) {
        this.startIo = true;
      } else {
        this.startIo = false;
      }
    });
  }

  private unDisable(): void {
    let fileSystemConfigVals = this.shadowRoot?.querySelectorAll<HTMLElement>('.config');
    fileSystemConfigVals!.forEach((fileSystemConfigVal) => {
      fileSystemConfigVal.removeAttribute('disabled');
    });
  }

  private disable(): void {
    let fileSystemConfigVals = this.shadowRoot?.querySelectorAll<HTMLElement>('.config');
    fileSystemConfigVals!.forEach((fileSystemConfigVal) => {
      if (
        fileSystemConfigVal.title === 'Start FileSystem Record' ||
        fileSystemConfigVal.title === 'Start Page Fault Record' ||
        fileSystemConfigVal.title === 'Start BIO Latency Record'
      ) {
      } else {
        fileSystemConfigVal.setAttribute('disabled', '');
      }
    });
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
          <div class="file-system-config-div">
            <div>
               <span class="file-system-title">Start FileSystem Record</span>
               <lit-switch id="fileSystem"></lit-switch>
            </div>
          </div>
          <div class="file-system-config-div">
            <div>
               <span class="file-system-title">Start Page Fault Record</span>
               <lit-switch id="pageFault"></lit-switch>
            </div>
          </div>
          <div class="file-system-config-div">
            <div>
               <span class="file-system-title">Start BIO Latency Record</span>
               <lit-switch id="bioLatency"></lit-switch>
            </div>
          </div>
          <div class="file-system-config-div">
             <div>
                 <span class="file-system-title">Process</span>
                 <span class="file-system-des">Record process</span>
              </div>
            <lit-select-v default-value="" rounded="" class="file-system-select config" mode="multiple" canInsert="" 
            rounded placement = "bottom" title="Process"></lit-select-v>
          </div>
          <div class="file-system-config-div">
             <div>
                 <span class="file-system-title">Max Unwind Level</span>
              </div>
            <input class="fileSystem-input config" title="Max Unwind Level" id="maxUnwindLevel" value="10"/>
          </div>
        </div>
        `;
  }
}

export interface SystemConfig {
  process: string;
  unWindLevel: number;
}
