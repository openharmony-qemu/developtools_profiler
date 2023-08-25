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
import LitSwitch, { LitSwitchChangeEvent } from '../../../base-ui/switch/lit-switch.js';
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
    let hisysEventSwitch = this.shadowRoot?.querySelector('lit-switch') as LitSwitch;
    hisysEventSwitch.addEventListener('change', (event: CustomEventInit<LitSwitchChangeEvent>) => {
      let detail = event.detail;
      if (detail!.checked) {
        this.startSamp = true;
        this.unDisable();
      } else {
        this.startSamp = false;
        this.disable();
      }
    });
    this.eventProcessInput = this.shadowRoot?.querySelector<LitAllocationSelect>('lit-allocation-select');
    let hisyEventProcessInput = this.eventProcessInput?.shadowRoot?.querySelector('.multipleSelect') as HTMLDivElement;
    this.selectProcess = this.eventProcessInput!.shadowRoot?.querySelector('input') as HTMLInputElement;
    hisyEventProcessInput!.addEventListener('mousedown', (ev) => {
      if (SpRecordTrace.serialNumber === '') {
        this.eventProcessInput!.processData = [];
        this.eventProcessInput!.initData();
      }
    });
    hisyEventProcessInput!.addEventListener('mouseup', () => {
      if (SpRecordTrace.serialNumber === '') {
        this.eventProcessInput!.processData = [];
        this.eventProcessInput!.initData();
      } else {
        Cmd.getProcess().then((processList) => {
          this.eventProcessInput!.processData = processList;
          this.eventProcessInput!.initData();
        });
      }
    });
    this.disable();
  }

  private unDisable(): void {
    let hisysEventConfigVals = this.shadowRoot?.querySelectorAll<HTMLElement>('.config');
    hisysEventConfigVals!.forEach((hisysEventConfigVal) => {
      hisysEventConfigVal.removeAttribute('disabled');
    });
  }

  private disable(): void {
    let hisysEventConfigVals = this.shadowRoot?.querySelectorAll<HTMLElement>('.config');
    hisysEventConfigVals!.forEach((hisysEventConfigVal) => {
      if (hisysEventConfigVal.title !== 'Start Hisystem Event Tracker Record') {
        hisysEventConfigVal.setAttribute('disabled', '');
      }
    });
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
          <div class="hisys-event-config-div">
              <div>
                 <span class="event-title">Start Hisystem Event Tracker Record</span>
                 <lit-switch></lit-switch>
              </div>
          </div>
          <div class="hisys-event-config-div">
              <div>
                 <span class="event-title">AppName</span>
                 <span class="event-des">Record AppName</span>
              </div>
               <lit-allocation-select style="width: 100%;" rounded="" default-value="" class="event-select config" 
               placement="bottom" title="AppName"></lit-allocation-select>
          </div>
        </div>
        `;
  }
}
