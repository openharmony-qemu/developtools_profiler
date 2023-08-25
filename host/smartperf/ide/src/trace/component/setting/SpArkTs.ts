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
import '../../../base-ui/select/LitAllocationSelect.js';

import '../../../base-ui/switch/lit-switch.js';
import { LitAllocationSelect } from '../../../base-ui/select/LitAllocationSelect.js';
import { SpRecordTrace } from '../SpRecordTrace.js';
import { Cmd } from '../../../command/Cmd.js';
import { LitRadioBox } from '../../../base-ui/radiobox/LitRadioBox.js';
import { SpCheckDesBox } from './SpCheckDesBox.js';
import LitSwitch from '../../../base-ui/switch/lit-switch.js';

@element('sp-ark-ts')
export class SpArkTs extends BaseElement {
  private processInput: LitAllocationSelect | undefined | null;
  private spCheckDesBox: SpCheckDesBox | undefined | null;
  private radioBox: LitRadioBox | undefined | null;
  private interval: HTMLInputElement | undefined | null;
  private memorySwitch: LitSwitch | undefined | null;
  private cpuSwitch: LitSwitch | undefined | null;

  set startSamp(jsHeapStart: boolean) {
    if (jsHeapStart) {
      this.setAttribute('startSamp', '');
    } else {
      this.removeAttribute('startSamp');
    }
  }

  get startSamp(): boolean {
    return this.hasAttribute('startSamp');
  }

  get process(): string {
    if (this.processInput!.value.length > 0) {
      return this.processInput!.value;
    }
    return '';
  }

  get radioBoxType(): number {
    let memorySwitch = this.shadowRoot?.querySelector('#memory-switch');
    let type: string;
    if (memorySwitch!.getAttribute('checked') !== null) {
      this.radioBox = this.shadowRoot?.querySelector(`lit-radio[checked]`);
      type = this.radioBox?.getAttribute('type') || '';
    } else {
      type = '-1';
    }
    return Number(type);
  }

  get grabNumeric(): boolean {
    if (this.radioBoxType === 0) {
      this.spCheckDesBox = this.shadowRoot?.querySelector('#snapshot');
      let isChecked = this.spCheckDesBox?.getAttribute('checked');
      return isChecked === 'true';
    } else {
      return false;
    }
  }

  get grabAllocations(): boolean {
    if (this.radioBoxType === 1) {
      this.spCheckDesBox = this.shadowRoot?.querySelector('#timeline');
      let isChecked = this.spCheckDesBox?.getAttribute('checked');
      return isChecked === 'true';
    } else {
      return false;
    }
  }

  get intervalValue(): number {
    if (this.radioBoxType === 0) {
      return Number(this.interval!.value);
    } else {
      return 0;
    }
  }

  get grabCpuProfiler(): boolean {
    let isChecked = this.cpuSwitch?.getAttribute('checked');
    return isChecked !== null;
  }

  get intervalCpuValue(): number {
    let interval = this.shadowRoot?.querySelector<HTMLInputElement>('#cpuInterval');
    if (interval) {
      return Number(interval!.value);
    } else {
      return 0;
    }
  }

  initElements(): void {
    this.interval = this.shadowRoot?.querySelector('#interval');
    this.processInput = this.shadowRoot?.querySelector<LitAllocationSelect>('lit-allocation-select');
    let processInput = this.processInput?.shadowRoot?.querySelector('.multipleSelect') as HTMLDivElement;
    this.cpuSwitch = this.shadowRoot?.querySelector('#cpu-switch') as LitSwitch;
    processInput!.addEventListener('mousedown', (ev) => {
      if (SpRecordTrace.serialNumber == '') {
        this.processInput!.processData = [];
        this.processInput!.initData();
      }
    });
    processInput!.addEventListener('mouseup', () => {
      if (SpRecordTrace.serialNumber == '') {
        this.processInput!.processData = [];
        this.processInput!.initData();
      } else {
        Cmd.getDebugProcess().then((processList) => {
          this.processInput!.processData = processList;
          this.processInput!.initData();
        });
      }
    });
    this.interval!.addEventListener('focusout', () => {
      if (this.interval!.value === '') {
        this.interval!.value = '10';
      }
    });

    let litSwitch = this.shadowRoot?.querySelector('lit-switch') as LitSwitch;
    litSwitch.addEventListener('change', (event: any) => {
      let detail = event.detail;
      if (detail.checked) {
        this.unDisable();
        this.unMemoryDisable();
      } else {
        this.disable();
        this.memoryDisable();
      }
    });
    this.memorySwitch = this.shadowRoot?.querySelector('#memory-switch') as LitSwitch;
    this.memorySwitch.addEventListener('change', (event: any) => {
      let detail = event.detail;
      if (detail.checked) {
        this.unMemoryDisable();
      } else {
        if (!this.cpuSwitch?.checked) {
          litSwitch.checked = false;
          this.disable();
        }
        this.memoryDisable();
      }
    });

    this.cpuSwitch = this.shadowRoot?.querySelector('#cpu-switch') as LitSwitch;
    this.cpuSwitch.addEventListener('change', (event: any) => {
      let detail = event.detail;
      let interval = this.shadowRoot?.querySelectorAll<HTMLInputElement>('#cpuInterval');
      if (!detail.checked && !this.memorySwitch?.checked) {
        litSwitch.checked = false;
        this.disable();
      } else if (detail.checked) {
        interval!.forEach((item) => {
          item.disabled = false;
          item.style.background = 'var(--dark-background5,#FFFFFF)';
        });
      } else {
        interval!.forEach((item) => {
          item.disabled = true;
          item.style.color = '#b7b7b7';
          item.style.background = 'var(--dark-background1,#f5f5f5)';
        });
        litSwitch.checked = true;
        this.startSamp = true;
      }
    });
    this.disable();
    this.memoryDisable();
  }

  private memoryDisable() {
    let interval = this.shadowRoot?.querySelectorAll<HTMLInputElement>('#interval');
    interval!.forEach((item) => {
      item.disabled = true;
      item.style.color = '#b7b7b7';
      item.style.background = 'var(--dark-background1,#f5f5f5)';
    });
    let radioBoxes = this.shadowRoot?.querySelectorAll<LitRadioBox>('lit-radio');
    radioBoxes!.forEach((item) => {
      item.disabled = true;
    });
    let checkBoxes = this.shadowRoot?.querySelectorAll<SpCheckDesBox>('check-des-box');
    checkBoxes!.forEach((item) => {
      item.disabled = true;
    });
  }

  private unMemoryDisable() {
    let interval = this.shadowRoot?.querySelectorAll<HTMLInputElement>('#interval');
    interval!.forEach((item) => {
      item.disabled = false;
      item.style.background = 'var(--dark-background5,#FFFFFF)';
    });
    let radioBoxes = this.shadowRoot?.querySelectorAll<LitRadioBox>('lit-radio');
    radioBoxes!.forEach((item) => {
      item.disabled = false;
    });
    let checkBoxes = this.shadowRoot?.querySelectorAll<SpCheckDesBox>('check-des-box');
    checkBoxes!.forEach((item) => {
      item.disabled = false;
    });
  }

  private disable() {
    this.startSamp = false;
    this.processInput!.setAttribute('disabled', '');
    let heapConfigs = this.shadowRoot?.querySelectorAll<HTMLInputElement>('.select');
    heapConfigs!.forEach((item) => {
      item.disabled = true;
    });
    let switches = this.shadowRoot?.querySelectorAll<LitSwitch>('.switch');
    switches!.forEach((item) => {
      item.disabled = true;
      item.checked = false;
    });
    let interval = this.shadowRoot?.querySelectorAll<HTMLInputElement>('.inputstyle');
    interval!.forEach((item) => {
      item.disabled = true;
      item.style.color = '#b7b7b7';
      item.style.background = 'var(--dark-background1,#f5f5f5)';
    });
  }

  private unDisable() {
    this.startSamp = true;
    this.processInput!.removeAttribute('disabled');
    let heapConfigs = this.shadowRoot?.querySelectorAll<HTMLInputElement>('.select');
    heapConfigs!.forEach((item) => {
      item.disabled = false;
    });
    let switches = this.shadowRoot?.querySelectorAll<LitSwitch>('.switch');
    switches!.forEach((item) => {
      item.disabled = false;
      item.checked = true;
    });
    let interval = this.shadowRoot?.querySelectorAll<HTMLInputElement>('.inputstyle');
    interval!.forEach((item) => {
      item.disabled = false;
      item.style.background = 'var(--dark-background5,#FFFFFF)';
    });
  }

  initHtml(): string {
    return `
        <style>
        :host{
            display: inline-block;
            width: 100%;
            height: 100%;
            background: var(--dark-background3,#FFFFFF);
            border-radius: 0px 16px 16px 0px;
        }
        .root {
            padding-top: 30px;
            padding-left: 54px;
            margin-right: 30px;
            font-size:16px;
            margin-bottom: 30px;
        }
        .config-div {
           width: 80%;
           display: flex;
           flex-direction: column;
           margin-top: 5vh;
           margin-bottom: 5vh;
           gap: 25px;
        }
        .title {
          opacity: 0.9;
          font-family: Helvetica-Bold;
          font-size: 18px;
          text-align: center;
          line-height: 40px;
          font-weight: 700;
          margin-right: 10px;
        }
        .config-title{
            margin-left: 20px;
            font-weight: 700;
            line-height: 48px;
        }
        .memory {
            margin-left: 40px;
        }
        .des {
          color: #242424;
            font-family: Helvetica;
            font-size: 14px;
            text-align: left;
            line-height: 16px;
            font-weight: 400;
        }
        .select {
          border-radius: 15px;
        }
        input {
           width: 35%;
           height: 25px;
           border:0;
           outline:none;
           border-radius: 16px;
           text-indent:2%
        }
        input::-webkit-input-placeholder{
            color:var(--bark-prompt,#999999);
        }
        .inputstyle{
            background: var(--dark-background5,#FFFFFF);
            border: 1px solid var(--dark-background5,#999999);
            font-family: Helvetica;
            font-size: 14px;
            color: var(--dark-color1,#212121);
            text-align: left;
            line-height: 16px;
            font-weight: 400;
        }
        .inputstyle::-webkit-input-placeholder {
           background: var(--dark-background5,#FFFFFF);
        }
        .radio {
            font-family: Helvetica-Bold;
            font-size: 16px;
            color: #000000;
            line-height: 28px;
            font-weight: 700;
        }
        .unit {
            font-family: Helvetica;
            font-size: 14px;
            color: #000000;
            line-height: 28px;
            font-weight: 400;
        }
        lit-switch {
          display:inline;
          float: right;
          height: 38px;
          margin-top: 10px;
        }
        :host([startSamp]) .inputBoxes {
            background: var(--dark-background5,#FFFFFF);
        }
        :host(:not([startSamp])) .inputBoxes {
            color: #b7b7b7;
            background: var(--dark-background1,#f5f5f5);
        }
        </style>
        <div class="root">
            <div class="config-div">
                <div>
                  <span class="title">Start Ark Ts Record</span>
                  <lit-switch></lit-switch>
                </div>
            </div>
            <div class="config-div">
                <div>
                    <span class="title">Process</span>
                    <span class="des">Record process</span>
                </div>
                <lit-allocation-select style="width: 100%;" rounded="" default-value="" class="select inputBoxes" placement="bottom" ></lit-allocation-select>
            </div>
            <div class="config-div">
                <div>
                    <span class="title">Select profiling type</span>
                </div>
                <div>
                    <span class="config-title">Start cpu profiler</span>
                    <lit-switch class="switch" id='cpu-switch'></lit-switch>
                </div>
                <div style="margin-left: 40px;">
                    <span class="des">Interval(Available on recent OpenHarmony 4.0)</span>
                    <div style="margin-top: 12px;">
                        <input class="inputstyle inputBoxes" id='cpuInterval' type="text" id="interval" placeholder="" onkeyup="this.value=this.value.replace(/\\D/g,'').replace(/^0{1,}/g,'')" value="1000">
                        <span class="unit">μs</span>
                    </div>
                </div>
                <div>
                    <span class="config-title">Start memory profiler</span>
                    <lit-switch class="switch" id='memory-switch'></lit-switch>
                </div>
                <div class='memory'>
                    <lit-radio dis="round" class="radio" name="litRadio" checked type="0">Heap snapshot</lit-radio>
                    <div style="margin-left: 10px;">
                        <span class="des">Heap snapshot profiles show memory distribution among your page’s JavaScript objects and related DOM nodes.</span>
                        <div style="display: flex;margin-bottom: 12px;margin-top: 12px;">
                            <check-des-box checked="true" value ="lnclude numerical values in capture" id="snapshot">
                            </check-des-box>
                        </div>
                        <span class="des">Interval(Available on recent OpenHarmony 4.0)</span>
                        <div style="margin-top: 12px;">
                            <input class="inputstyle inputBoxes" type="text" id="interval" placeholder="" onkeyup="this.value=this.value.replace(/\\D/g,'').replace(/^0{1,}/g,'')" value="10">
                            <span class="unit">S</span>
                        </div>
                    </div>
                    <lit-radio dis="round" name="litRadio" class="radio" type="1">Allocation insteumentation on timeline</lit-radio>
                    <div style="margin-left: 10px;">
                        <span class="des">Allocation timelines show insturmented Javascript memory allocations over time. Once profile is recorded you can select a time interval to see objects that werre allocated within it and still alive by the end of recording. Use this profile type to isolate memory leaks.</span>
                        <div style="display: flex;margin-top: 12px;">
                        <check-des-box value ="record stack traces of allocations(extra performance overhead)" id="timeline">
                        </check-des-box>
                        </div>
                    </div>
                </div>
            </、div>
        </div>
        `;
  }
}
