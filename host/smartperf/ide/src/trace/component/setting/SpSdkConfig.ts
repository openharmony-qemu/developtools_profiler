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
import '../../../base-ui/select/LitSelectV.js';
import '../../../base-ui/select/LitSelect.js';

import '../../../base-ui/switch/lit-switch.js';
import LitSwitch, { LitSwitchChangeEvent } from '../../../base-ui/switch/lit-switch.js';
import { LitSelectV } from '../../../base-ui/select/LitSelectV.js';
import { LitAllocationSelect } from '../../../base-ui/select/LitAllocationSelect.js';

@element('sp-sdk-config')
export class SpSdkConfig extends BaseElement {
  private worker: Worker | undefined;
  private sdkConfigList: any;
  private customConfig: HTMLDivElement | undefined | null;
  private selectConfig: LitAllocationSelect | undefined | null;
  private list: Array<HTMLElement> | undefined;
  private pluginName: string = '';
  private sampleInterval: number = 5000;

  static get observedAttributes() {
    return ['configName', 'value', 'type'];
  }

  get show(): boolean {
    return this.hasAttribute('show');
  }

  set show(sdkConfigShow: boolean) {
    if (sdkConfigShow) {
      this.setAttribute('show', '');
    } else {
      this.removeAttribute('show');
    }
  }

  set startSamp(sdkConfigStart: boolean) {
    if (sdkConfigStart) {
      this.setAttribute('startSamp', '');
    } else {
      this.removeAttribute('startSamp');
    }
  }

  get startSamp(): boolean {
    return this.hasAttribute('startSamp');
  }

  set configName(configName: string) {
    if (configName != '') {
      this.setAttribute('configName', configName);
    } else {
      this.removeAttribute('configName');
    }
  }

  get configName(): string {
    return this.getAttribute('configName') || '';
  }

  get type(): string {
    return this.getAttribute('type') || '';
  }

  set type(type: string) {
    if (type != '') {
      this.setAttribute('type', type);
    } else {
      this.removeAttribute('type');
    }
  }

  private wasmMap: Map<string, any> = new Map<string, any>();
  private wasmList: Array<string> = [];

  private changGpu(gpuName: string) {
    let config = this.wasmMap.get(gpuName);
    this.pluginName = config?.pluginName;
    this.sampleInterval = config?.sampleInterval;
    let pam = {
      action: 'open',
      componentId: config.componentId,
      wasmJsName: config.wasmJsName,
      WasmName: config.wasmName,
    };
    this.worker!.postMessage(pam);
    this.worker!.onmessage = (event: MessageEvent) => {
      let results = event.data.results;
      this.sdkConfigList = results.settingConfig;
      this.initConfig();
    };
  }

  getPlugName(): string {
    return this.pluginName;
  }

  getSampleInterval(): number {
    return this.sampleInterval;
  }

  getGpuConfig(): {} {
    let configVal = this.shadowRoot?.querySelectorAll<HTMLElement>('.config');
    let gpuConfig = {};
    for (let i = 0; i < configVal!.length; i++) {
      let configName = configVal![i].getAttribute('configName');
      let type = configVal![i].getAttribute('type');
      if (type == 'enum') {
        let enumValue = configVal![i].getAttribute('value');
        if (enumValue != undefined && enumValue != 'undefined') {
          // @ts-ignore
          gpuConfig[configName!] = enumValue;
        }
      } else if (type == 'number' || type == 'integer' || type == 'num') {
        // @ts-ignore
        gpuConfig[configName!] = Number(configVal![i].value);
      } else if (type == 'boolean') {
        let attribute = configVal![i].getAttribute('value');
        // @ts-ignore
        gpuConfig[configName!] = attribute == 'true';
      } else {
        // @ts-ignore
        gpuConfig[configName!] = configVal![i].value;
      }
    }
    return gpuConfig;
  }

  initElements(): void {
    try {
      let spApplication = document.querySelector<HTMLElement>('sp-application');
      let wasmJsonUrl = `https://${window.location.host.split(':')[0]}:${window.location.port}/application/wasm.json`;
      if (spApplication!.hasAttribute('vs')) {
        wasmJsonUrl = `http://${window.location.host.split(':')[0]}:${window.location.port}/wasm.json`;
      }
      fetch(wasmJsonUrl)
        .then((res) => {
          if (res.ok) {
            res.text().then((text) => {
              this.wasmMap = new Map();
              this.wasmList = [];
              let wasmJson = JSON.parse(text);
              let wasmFiles = wasmJson.WasmFiles;
              wasmFiles.forEach((wasmFile: any) => {
                this.wasmMap.set(wasmFile.disPlayName, wasmFile);
                this.wasmList.push(wasmFile.disPlayName);
              });
            });
          }
        })
        .catch((err) => {});
      if (this.worker == null) {
        // @ts-ignore
        if (window.useWb) {
          return;
        }
        this.worker = new Worker('trace/database/ConfigWorker.js');
      }
    } catch (e) {}
    this.customConfig = this.shadowRoot?.querySelector<HTMLDivElement>('.configList');
    let switchButton = this.shadowRoot?.querySelector('.config_switch') as LitSwitch;
    switchButton.addEventListener('change', (event: CustomEventInit<LitSwitchChangeEvent>) => {
      let detail = event.detail;
      if (detail!.checked) {
        this.startSamp = true;
        this.isAbleShowConfig(false);
      } else {
        this.startSamp = false;
        this.isAbleShowConfig(true);
      }
    });
    this.selectConfig = this.shadowRoot?.querySelector<LitAllocationSelect>('lit-allocation-select');
    let inputDiv = this.selectConfig?.shadowRoot?.querySelector('.multipleSelect') as HTMLDivElement;
    let input = this.selectConfig?.shadowRoot?.querySelector<HTMLInputElement>('#singleInput');
    if (input) {
      inputDiv.addEventListener('inputClick', () => {
        this.selectConfig!.processData = this.wasmList;
        this.selectConfig!.initData();
      });
      inputDiv.addEventListener('valuable', (ev) => {
        this.changGpu(input!.value);
      });
    }
    this.list = [];
    this.list.push(this.selectConfig!);
    this.isAbleShowConfig(true);
  }

  initConfig() {
    this.customConfig!.innerHTML = '';
    this.list = [];
    this.list.push(this.selectConfig!);
    let sdkConfigSwitch = document.createElement('lit-switch') as LitSwitch;
    for (let key in this.sdkConfigList.configuration) {
      let html = '';
      let sdkConfigDiv = document.createElement('div');
      sdkConfigDiv.className = 'sdk-config-div';
      let sdkConfigHeadDiv = document.createElement('div');
      sdkConfigDiv.appendChild(sdkConfigHeadDiv);
      let sdkConfigTitle = document.createElement('span');
      sdkConfigTitle.className = 'sdk-config-title';
      sdkConfigTitle.textContent = key;
      sdkConfigHeadDiv.appendChild(sdkConfigTitle);
      let sdkConfigDes = document.createElement('span');
      sdkConfigDes.textContent = this.sdkConfigList.configuration[key].description;
      sdkConfigDes.className = 'sdk-config-des';
      sdkConfigHeadDiv.appendChild(sdkConfigDes);
      switch (this.sdkConfigList.configuration[key].type) {
        case 'string':
          if (this.sdkConfigList.configuration[key].enum) {
            let placeholder = '';
            if (this.sdkConfigList.configuration[key].default) {
              placeholder = this.sdkConfigList.configuration[key].default;
            }
            html += `<lit-select-v id="${key}" type="${this.sdkConfigList.configuration[key].type}" default-value="" rounded="" class="sdk-config-select config" mode="multiple" canInsert="" rounded placement = "bottom" configName ="${key}" placeholder="${placeholder}"></lit-select-v>`;
            sdkConfigDiv.innerHTML = sdkConfigDiv.innerHTML + html;
          } else {
            let inputElement = document.createElement('input');
            inputElement.className = 'sdk-config-input config';
            if (this.sdkConfigList.configuration[key].default) {
              inputElement.value = this.sdkConfigList.configuration[key].default;
            }
            inputElement.setAttribute('configName', key);
            inputElement.setAttribute('type', this.sdkConfigList.configuration[key].type);
            sdkConfigDiv.appendChild(inputElement);
            this.list.push(inputElement);
          }
          break;
        case 'number':
          let numberInput = document.createElement('input');
          numberInput.className = 'sdk-config-input config';
          if (this.sdkConfigList.configuration[key].default) {
            numberInput.value = this.sdkConfigList.configuration[key].default;
          }
          numberInput.setAttribute('configName', key);
          numberInput.setAttribute('type', 'num');
          numberInput.oninput = (ev) => {
            let inputValue = this.checkFloatInput(numberInput.value);
            numberInput.value = inputValue;
          };
          sdkConfigDiv.appendChild(numberInput);
          this.list.push(numberInput);
          break;
        case 'integer':
          let input = document.createElement('input');
          input.className = 'sdk-config-input config';
          if (this.sdkConfigList.configuration[key].default) {
            input.value = this.sdkConfigList.configuration[key].default;
          }
          input.setAttribute('configName', key);
          input.setAttribute('type', this.sdkConfigList.configuration[key].type);
          input.oninput = (ev) => {
            let inputValue = this.checkIntegerInput(input.value);
            input.value = inputValue;
            sdkConfigTitle.setAttribute('value', input.value);
          };
          sdkConfigDiv.appendChild(input);
          this.list.push(input);
          break;
        case 'boolean':
          sdkConfigSwitch.className = 'switch1 config';
          sdkConfigSwitch.setAttribute('configName', key);
          sdkConfigSwitch.setAttribute('type', this.sdkConfigList.configuration[key].type);
          if (this.sdkConfigList.configuration[key].default == 'true') {
            sdkConfigSwitch.setAttribute('checked', '');
            sdkConfigSwitch.setAttribute('value', 'true');
          } else {
            sdkConfigSwitch.removeAttribute('checked');
            sdkConfigSwitch.setAttribute('value', 'false');
          }
          sdkConfigHeadDiv.appendChild(sdkConfigSwitch);
          this.list.push(sdkConfigSwitch);
          break;
      }
      this.customConfig!.appendChild(sdkConfigDiv);
      if (this.sdkConfigList.configuration[key].enum) {
        let select = this.shadowRoot!.querySelector<LitSelectV>(`#${key}`);
        select!.setAttribute('type', 'enum');
        select!.setAttribute('value', this.sdkConfigList.configuration[key].default);
        select!.dataSource(this.sdkConfigList.configuration[key].enum, '');
        this.list.push(select!);
        select!.addEventListener('click', () => {
          select!.setAttribute('value', select!.value);
        });
      }
    }
    sdkConfigSwitch.addEventListener('change', () => {
      if (sdkConfigSwitch.hasAttribute('checked')) {
        sdkConfigSwitch.setAttribute('value', 'true');
      } else {
        sdkConfigSwitch.setAttribute('value', 'false');
      }
    });
  }

  checkIntegerInput(value: string): string {
    let inputValue = value
      .replace(/[^\-\d]|\-{2,}/g, '')
      .replace(/(\d)\-|-(0+)|^0+(\d)/g, '$1')
      .replace(/(-?\d{15})\d*/, '$1');
    return inputValue;
  }

  checkFloatInput(value: string): string {
    let inputValue = value
      .replace(/[^\d.]/g, '')
      .replace(/^\.|^0+(\d)/g, '')
      .replace(/(\.\d+)\.|(-)\.|(\d+|\.)-/g, '$1')
      .replace(/(-?\d{9})\d*/, '$1');
    return inputValue.replace(/\.{2,}|-(0){2,}|(-)0+(\d+)/g, '.');
  }

  isAbleShowConfig(isAbleShow: boolean) {
    if (this.list!) {
      if (isAbleShow) {
        this.list!.forEach((item) => {
          item.setAttribute('disabled', '');
        });
      } else {
        this.list!.forEach((item) => {
          item.removeAttribute('disabled');
        });
      }
    }
  }

  initHtml(): string {
    return `
        <style>
        .sdk-config-div {
           flex-direction: column;
           width: 80%;
           display: flex;
           gap: 15px;
        }
        :host{
            display: inline-block;
            width: 100%;
            height: 100%;
            background: var(--dark-background3,#FFFFFF);
            border-radius: 0px 16px 16px 0px;
        }
        .root {
            font-size:16px;
            padding-left: 54px;
            margin-right: 30px;
            padding-top: 30px;
            margin-bottom: 30px;
        }
        :host([show]) .sdk-config-div {
           display: flex;
           flex-direction: column;
           margin-bottom: 1vh;
        }

        :host(:not([show])) .sdk-config-div {
           margin-top: 5vh;
           margin-bottom: 5vh;
           gap: 25px;
        }
        
        :host(:not([show])) .hidden {
           display: none;
        }

        .sdk-config-title {
          opacity: 0.9;
          line-height: 40px;
          font-family: Helvetica-Bold;
          font-size: 18px;
          text-align: center;
          font-weight: 700;
          margin-right: 10px;
        }

        .sdk-config-des {
          opacity: 0.6;
          font-family: Helvetica;
          font-size: 14px;
          text-align: center;
          line-height: 35px;
          font-weight: 400;
        }

        .sdk-config-select {
          border-radius: 15px;
        }

        input {
           height: 25px;
           outline:none;
           border-radius: 16px;
           text-indent:2%
        }
        input::-webkit-input-placeholder{
            color:var(--bark-prompt,#999999);
        }
        lit-switch {
          display:inline;
          float: right;
          height: 38px;
          margin-top: 10px;
        }
        .sdk-config-input {
            border: 1px solid var(--dark-background5,#ccc);
            font-family: Helvetica;
            font-size: 14px;
            color: var(--dark-color1,#212121);
            text-align: left;
            line-height: 20px;
            font-weight: 400;
        }
        
        :host([startSamp]) .sdk-config-input {
            background: var(--dark-background5,#FFFFFF);
        }
        
        :host(:not([startSamp])) .sdk-config-input {
            color: var(--dark-color1,#212121);
        }
       
        </style>
        <div class="root">
            <div class="sdk-config-div">
                <div>
                    <span class="sdk-config-title">Start Custom Config</span>
                    <lit-switch class="config_switch" ></lit-switch>
                </div>
            </div>
            <div class="sdk-config-div" id="select_config">
                <lit-allocation-select show-search class="processSelect" rounded default-value="" id="pid" placement="bottom" style="width:100%"></lit-allocation-select>
            </div>
            <div class="configList">
            </div>
        </div>
        `;
  }
}
