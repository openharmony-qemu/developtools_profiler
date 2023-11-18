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
import { SpRecordTrace } from '../SpRecordTrace.js';
import { HdcDeviceManager } from '../../../hdc/HdcDeviceManager.js';
import { LitAllocationSelect } from '../../../base-ui/select/LitAllocationSelect.js';

@element('sp-hisys-event')
export class SpHisysEvent extends BaseElement {
  private domainInputEL: LitAllocationSelect | undefined | null;
  private eventNameInputEL: LitAllocationSelect | undefined | null;
  private sysEventConfigList: NodeListOf<LitAllocationSelect> | undefined | null;
  private sysEventSwitch: LitSwitch | undefined | null;
  private domainInputEl: HTMLInputElement | undefined | null;
  private nameInputEl: HTMLInputElement | undefined | null;
  private eventConfig: unknown = {};

  set startSamp(start: boolean) {
    if (start) {
      this.setAttribute('startSamp', '');
      this.domainInputEL!.removeAttribute('readonly');
      this.eventNameInputEL!.removeAttribute('readonly');
    } else {
      this.removeAttribute('startSamp');
      this.domainInputEL!.setAttribute('readonly', 'readonly');
      this.eventNameInputEL!.setAttribute('readonly', 'readonly');
      this.domainInputEL!.value = '';
      this.eventNameInputEL!.value = '';
    }
  }

  get domain(): string {
    if (this.domainInputEL!.value.length > 0 && this.domainInputEL!.value !== 'ALL-Domain') {
      return this.domainInputEL!.value;
    }
    return '';
  }

  get eventName(): string {
    if (this.eventNameInputEL!.value.length > 0) {
      return this.eventNameInputEL!.value;
    }
    return '';
  }

  get startSamp(): boolean {
    return this.hasAttribute('startSamp');
  }

  get sysEventConfigPath(): string {
    return '/system/etc/hiview/hisysevent.def';
  }

  initElements(): void {
    this.domainInputEL = this.shadowRoot?.querySelector<LitAllocationSelect>('.record-domain-input');
    this.eventNameInputEL = this.shadowRoot?.querySelector<LitAllocationSelect>('.record-event-input');
    this.sysEventConfigList = this.shadowRoot?.querySelectorAll<LitAllocationSelect>('.record-input');
    this.sysEventSwitch = this.shadowRoot?.querySelector('lit-switch') as LitSwitch;
    this.sysEventSwitch?.addEventListener('change', (event: CustomEventInit<LitSwitchChangeEvent>) => {
      let detail = event.detail;
      this.startSamp = detail!.checked;
      this.updateDisable(detail!.checked);
    });
    this.updateDisable(false);
    this.domainInputEl = this.domainInputEL?.shadowRoot?.querySelector('.multipleSelect') as HTMLInputElement;
    this.nameInputEl = this.eventNameInputEL?.shadowRoot?.querySelector('.multipleSelect') as HTMLInputElement;
    this.domainInputEl.addEventListener('valuable', () => {
      this.eventNameInputEL!.value = '';
    });
  }

  connectedCallback(): void {
    super.connectedCallback();
    this.domainInputEl?.addEventListener('mousedown', this.domainInputEvent);
    this.nameInputEl?.addEventListener('mousedown', this.nameInputEvent);
  }

  disconnectedCallback(): void {
    super.disconnectedCallback();
    this.domainInputEl?.removeEventListener('mousedown', this.domainInputEvent);
    this.nameInputEl?.removeEventListener('mousedown', this.nameInputEvent);
  }

  domainInputEvent = (): void => {
    if (SpRecordTrace.serialNumber === '') {
      this.domainInputEL!.processData = [];
      this.domainInputEL!.initData();
    } else {
      HdcDeviceManager.fileRecv(this.sysEventConfigPath, () => {
      }).then((pullRes) => {
        pullRes.arrayBuffer().then((buffer) => {
          if (buffer.byteLength > 0) {
            let dec = new TextDecoder();
            this.eventConfig = JSON.parse(dec.decode(buffer));
            let domainList = Object.keys(this.eventConfig!);
            if (domainList.length > 0 && this.startSamp) {
              this.domainInputEl!.setAttribute('readonly', 'readonly');
              domainList.unshift('ALL-Domain');
            }
            this.domainInputEL!.processData = domainList;
            this.domainInputEL!.initData();
          }
        });
      });
    }
  };

  nameInputEvent = (): void => {
    if (SpRecordTrace.serialNumber === '') {
      this.eventNameInputEL!.processData = [];
      this.eventNameInputEL!.initData();
    } else {
      let domain = this.domainInputEL?.value;
      // @ts-ignore
      let eventConfigElement = this.eventConfig[domain];
      if (eventConfigElement) {
        let eventNameList = Object.keys(eventConfigElement);
        if (eventNameList?.length > 0 && this.startSamp) {
          this.nameInputEl!.setAttribute('readonly', 'readonly');
          this.eventNameInputEL!.processData = eventNameList;
          this.eventNameInputEL!.initData();
        }
      } else {
        this.eventNameInputEL!.value = '';
        this.eventNameInputEL!.processData = [];
        this.eventNameInputEL!.initData();
      }
    }
  };

  private updateDisable(isDisable: boolean): void {
    this.sysEventConfigList!.forEach((configEL) => {
      if (isDisable) {
        configEL.removeAttribute('disabled');
      } else {
        configEL.setAttribute('disabled', '');
      }
    });
  }

  private getCssStyle(): string {
    return `
        <style>
        :host{
          background: var(--dark-background3,#FFFFFF);
          display: inline-block;
          width: 100%;
          height: 100%;
          border-radius: 0px 16px 16px 0px;
        }
        :host([startSamp]) .record-input {
          background: var(--dark-background5,#FFFFFF);
        }
        :host(:not([startSamp])) .record-input {
          color: #999999;
        }
        .root {
          margin-bottom: 30px;
          padding-top: 30px;
          padding-left: 54px;
          margin-right: 30px;
          font-size:16px;
        }
        .hisys-event-config {
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
        lit-switch {
          height: 38px;
          margin-top: 10px;
          display:inline;
          float: right;
        }
        .record-input {
          line-height: 20px;
          font-weight: 400;
          border: 1px solid var(--dark-background5,#ccc);
          font-family: Helvetica;
          font-size: 14px;
          color: var(--dark-color1,#212121);
          text-align: left;
          width: auto;
        }
        </style>`;
  }

  initHtml(): string {
    return `
        ${this.getCssStyle()}
        <div class="root">
          <div class="hisys-event-config">
              <div>
                 <span class="event-title">Start Hisystem Event Tracker Record</span>
                 <lit-switch></lit-switch>
              </div>
          </div>
          <div class="hisys-event-config">
              <div>
                 <span class="event-title">Domain</span>
                 <span class="event-des">Record Domain Name</span>
              </div>
              <lit-allocation-select default-value="" rounded="" class="record-domain-input record-input" 
              mode="multiple" canInsert="" title="Select Proces" placement="bottom" placeholder="" readonly="readonly">
              </lit-allocation-select>
          </div>
          <div class="hisys-event-config">
              <div>
                 <span class="event-title">EventName</span>
                 <span class="event-des">Record Event Name</span>
              </div>
              <lit-allocation-select default-value="" rounded="" class="record-event-input record-input" 
              mode="multiple" canInsert="" title="Select Proces" placement="bottom" placeholder="" readonly="readonly">
              </lit-allocation-select>
          </div>
        </div>
        `;
  }
}
