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
import { LitSelect } from '../../../base-ui/select/LitSelect.js';
import { LitSlider } from '../../../base-ui/slider/LitSlider.js';
import LitSwitch, { LitSwitchChangeEvent } from '../../../base-ui/switch/lit-switch.js';
import '../../../base-ui/select/LitSelectV.js';
import '../../../base-ui/select/LitSelect.js';

import '../../../base-ui/switch/lit-switch.js';
import { info } from '../../../log/Log.js';
import { HdcDeviceManager } from '../../../hdc/HdcDeviceManager.js';
import { SpRecordTrace } from '../SpRecordTrace.js';
import { SpApplication } from '../../SpApplication.js';
import { LitSearch } from '../trace/search/Search.js';
import { Cmd } from '../../../command/Cmd.js';
import { CmdConstant } from '../../../command/CmdConstant.js';

@element('sp-record-perf')
export class SpRecordPerf extends BaseElement {
  private addOptionButton: HTMLButtonElement | undefined | null;
  private processSelect: LitSelectV | undefined | null;
  private cpuSelect: LitSelectV | undefined | null;
  private eventSelect: LitSelectV | undefined | null;

  private frequencySetInput: HTMLInputElement | undefined | null;
  private recordProcessInput: HTMLInputElement | undefined | null;
  private offCPUSwitch: LitSwitch | undefined | null;
  private callSelect: LitSelect | undefined | null;
  private perfConfigList: Array<any> = [];

  get show(): boolean {
    return this.hasAttribute('show');
  }

  set show(perfShow: boolean) {
    if (perfShow) {
      this.setAttribute('show', '');
    } else {
      this.removeAttribute('show');
    }
  }

  set startSamp(perfStart: boolean) {
    if (perfStart) {
      this.setAttribute('startSamp', '');
      this.recordProcessInput!.removeAttribute('readonly');
    } else {
      this.removeAttribute('startSamp');
      this.recordProcessInput!.setAttribute('readonly', 'readonly');
    }
  }

  get startSamp(): boolean {
    return this.hasAttribute('startSamp');
  }

  getSoftHardWareEvents(eventListResult: Map<string, string[]>) {
    let shEvents = [];
    let hardwareEvents = eventListResult.get('hardware');
    if (hardwareEvents) {
      for (let hardwareEvent of hardwareEvents) {
        shEvents.push(hardwareEvent);
      }
    }
    let softwareEvents = eventListResult.get('software');
    if (softwareEvents) {
      for (let softwareEvent of softwareEvents) {
        shEvents.push(softwareEvent);
      }
    }
    return shEvents;
  }

  parseEvent(eventListResult: string): Map<string, Array<string>> {
    let eventMap: Map<string, Array<string>> = new Map<string, Array<string>>();
    let events: Array<string> = [];
    let type: string = '';
    let lineValues: string[] = eventListResult.replace(/\r\n/g, '\r').replace(/\n/g, '\r').split(/\r/);
    for (let line of lineValues) {
      if (line.startsWith('Supported')) {
        let startSign: string = 'for';
        type = line.substring(line.indexOf(startSign) + startSign.length, line.lastIndexOf(':')).trim();
        events = new Array();
        eventMap.set(type, events);
      } else if (line.indexOf('not support') != -1 || line.trim().length == 0 || line.indexOf('Text file busy') != -1) {
        // do not need deal with it
      } else {
        let event: string = line.split(' ')[0];
        let ventMap = eventMap.get(type);
        if (ventMap != null) {
          ventMap.push(event);
        }
      }
    }
    return eventMap;
  }

  private unDisable() {
    if (this.processSelect) {
      this.processSelect.removeAttribute('disabled');
    }
    if (this.frequencySetInput) {
      this.frequencySetInput!.disabled = false;
    }
    if (this.offCPUSwitch) {
      this.offCPUSwitch!.disabled = false;
    }
    if (this.callSelect) {
      this.callSelect!.removeAttribute('disabled');
    }
    if (this.addOptionButton) {
      this.addOptionButton.disabled = false;
    }
  }

  private disable() {
    if (this.processSelect) {
      this.processSelect.setAttribute('disabled', '');
    }
    if (this.frequencySetInput) {
      this.frequencySetInput!.disabled = true;
    }
    if (this.offCPUSwitch) {
      this.offCPUSwitch!.disabled = true;
    }
    if (this.callSelect) {
      this.callSelect!.setAttribute('disabled', '');
    }
    if (this.addOptionButton) {
      this.addOptionButton.disabled = true;
    }
  }

  initConfigList(): void {
    this.perfConfigList = [
      {
        title: 'Start Hiperf Sampling',
        des: '',
        hidden: false,
        type: 'switch',
        value: false,
      },
      {
        type: 'select-multiple',
        title: 'Process',
        des: 'Record process',
        hidden: false,
        selectArray: [''],
      },
      {
        title: 'CPU',
        des: 'Record assign cpu num such as 0,1,2',
        hidden: true,
        type: 'select-multiple',
        selectArray: [''],
      },
      {
        title: 'Event List',
        des: 'Event type Default is cpu cycles',
        hidden: true,
        type: 'select-multiple',
        selectArray: [''],
      },
      {
        title: 'CPU Percent',
        des: 'Set the max percent of cpu time used for recording',
        hidden: true,
        type: 'lit-slider',
        litSliderStyle: {
          minRange: 0,
          maxRange: 100,
          defaultValue: '100',
          resultUnit: '%',
          stepSize: 1,
          lineColor: 'var(--dark-color3,#a88888)',
          buttonColor: '#a88888',
        },
      },
      {
        title: 'Frequency',
        des: 'Set event sampling frequency',
        hidden: false,
        type: 'input',
        value: '1000',
      },
      {
        title: 'Period',
        des: 'Set event sampling period for trace point events2',
        hidden: true,
        type: 'input',
        value: '1',
      },
      {
        title: 'Off CPU',
        des: 'Trace when threads are scheduled off cpu',
        hidden: false,
        type: 'switch',
        value: true,
      },
      {
        title: 'No Inherit',
        des: "Don't trace child processes",
        hidden: true,
        type: 'switch',
        value: false,
      },
      {
        title: 'Call Stack',
        des: 'Setup and enable call stack recording',
        hidden: false,
        type: 'select',
        selectArray: ['dwarf', 'fp', 'none'],
      },
      {
        title: 'Branch',
        des: 'Taken branch stack sampling',
        hidden: true,
        type: 'select',
        selectArray: ['none', 'any', 'any_call', 'any_ret', 'ind_call', 'call', 'user', 'kernel'],
      },
      {
        title: 'Mmap Pages',
        des: 'Used to receiving record data from kernel',
        hidden: true,
        type: 'Mmap-lit-slider',
        litSliderStyle: {
          minRange: 1,
          maxRange: 10,
          defaultValue: '8',
          resultUnit: 'MB',
          stepSize: 1,
          lineColor: 'var(--dark-color3,#46B1E3)',
          buttonColor: '#999999',
        },
      },
      {
        title: 'Clock Type',
        des: 'Set the clock id to use for the various time fields in the perf_event_type records',
        hidden: true,
        type: 'select',
        selectArray: ['monotonic', 'realtime', 'monotonic_raw', 'boottime', 'perf'],
      },
    ];
  }

  connectedCallback(): void {
    let traceMode = this.shadowRoot!.querySelector('#traceMode') as HTMLDivElement;
    let isLongTrace = SpApplication.isLongTrace;
    if (isLongTrace) {
      traceMode!.style.display = 'block';
    } else {
      traceMode!.style.display = 'none';
    }
  }

  initHtml(): string {
    return `
        <style>
       input {
           height: 25px;
           border-radius: 16px;
           outline:none;
           text-indent:2%
        }
        
        input::-webkit-input-placeholder{
            color:var(--bark-prompt,#999999);
        }
        
         :host([startSamp]) .record-perf-input {
            background: var(--dark-background5,#FFFFFF);
        }
        
        :host(:not([startSamp])) .record-perf-input {
            color: #999999;
        }
        
        :host{
            width: 100%;
            display: inline-block;
            height: 100%;
            background: var(--dark-background3,#FFFFFF);
            border-radius: 0px 16px 16px 0px;
        }

        .record-perf-config-div {
           display: flex;
           flex-direction: column;
           gap: 15px;
           width: 80%;
        }
        
        .root {
            padding-top: 30px;
            margin-right: 30px;
            padding-left: 54px;
            font-size:16px;
            margin-bottom: 30px;
        }

        :host([show]) .record-perf-config-div {
           display: flex;
           flex-direction: column;
           margin-bottom: 1vh;
        }

        :host(:not([show])) .record-perf-config-div {
           margin-top: 5vh;
           margin-bottom: 5vh;
           gap: 25px;
        }

        :host(:not([show])) .hidden {
           display: none;
        }

        #addOptions {
           border-radius: 15px;
           border-color:rgb(0,0,0,0.1);
           width: 150px;
           height: 40px;
           font-family: Helvetica;
           font-size: 1em;
           color: #FFFFFF;
           text-align: center;
           line-height: 20px;
           font-weight: 400;
           margin-right: 20%;
           float: right;
        }
        
        :host(:not([startSamp])) #addOptions {
           background: #999999;
        }
        :host([startSamp]) #addOptions {
           background: #3391FF;
        }

        .record-perf-title {
          opacity: 0.9;
          font-family: Helvetica-Bold;
          margin-right: 10px;
          font-size: 18px;
          text-align: center;
          line-height: 40px;
          font-weight: 700;
        }

        .record-perf-des {
          opacity: 0.6;
          font-family: Helvetica;
          line-height: 35px;
          font-size: 14px;
          text-align: center;
          font-weight: 400;
        }

        .record-perf-select {
          border-radius: 15px;
        }

        lit-switch {
          height: 38px;
          margin-top: 10px;
          display:inline;
          float: right;
        }
     
        .record-perf-input {
            line-height: 20px;
            font-weight: 400;
            border: 1px solid var(--dark-background5,#ccc);
            font-family: Helvetica;
            font-size: 14px;
            color: var(--dark-color1,#212121);
            text-align: left;
        }

        .sliderBody{
            width: 100%;
            height: min-content;
            display: grid;
            grid-template-columns: 1fr min-content;
        }

        .sliderInput {
            margin: 0 0 0 0;
            height: 40px;
            background-color: var(--dark-background5,#F2F2F2);
            -webkit-appearance:none;
            outline:0;
            font-size:14px;
            border-radius:20px;
            border:1px solid var(--dark-border,#c8cccf);
            color:var(--dark-color,#6a6f77);
            text-align: center;
        }
        </style>
        <div class="root">
            <div class="record-perf-title" id="traceMode" style="text-align:left;">
            <span style='color: red'>Long trace mode! If current data Trace is too large, it may not open!</span>
          </div>
          <div class="configList record-perf-config">
          </div>
          <button id ="addOptions">Advance Options</button>
        </div>
        `;
  }
}

export interface PerfConfig {
  process: string;
  cpu: string;
  eventList: string;
  cpuPercent: number;
  frequency: number;
  period: number;
  isOffCpu: boolean;
  noInherit: boolean;
  callStack: string;
  branch: string;
  mmap: number;
  clockType: string;
}
