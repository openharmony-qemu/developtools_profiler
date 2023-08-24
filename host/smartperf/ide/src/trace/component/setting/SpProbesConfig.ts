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
import { checkDesBean, SpCheckDesBox } from './SpCheckDesBox.js';
import { LitCheckBox, LitCheckBoxChangeEvent } from '../../../base-ui/checkbox/LitCheckBox.js';
import { LitRadioGroup } from '../../../base-ui/radiobox/LitRadioGroup.js';
import { info, log } from '../../../log/Log.js';
import { LitSlider } from '../../../base-ui/slider/LitSlider';

@element('probes-config')
export class SpProbesConfig extends BaseElement {
  private traceConfigList: Array<checkDesBean> | undefined;
  private memoryConfigList: Array<checkDesBean> | undefined;
  private abilityConfigList: Array<checkDesBean> | undefined;
  private hitraceConfigList: Array<any> | undefined;
  private hitrace: SpCheckDesBox | undefined;

  private _traceConfig: HTMLElement | undefined;
  private _memoryConfig: HTMLElement | undefined | null;
  private _abilityConfig: HTMLElement | undefined | null;
  private ftraceBufferSizeResult: HTMLDivElement | null | undefined;

  get traceConfig() {
    let selectedTrace = this._traceConfig?.querySelectorAll<SpCheckDesBox>(`check-des-box[checked]`) || [];
    let values = [];
    for (const litCheckBoxElement of selectedTrace) {
      values.push(litCheckBoxElement.value);
    }
    if (this.hitrace && this.hitrace.checked) {
      values.push(this.hitrace.value);
    }
    info('traceConfig is :', values);
    return values;
  }

  get ftraceBufferSize(): number {
    if (this.ftraceBufferSizeResult?.hasAttribute('percent')) {
      return Number(this.ftraceBufferSizeResult?.getAttribute('percent'));
    }
    return 20480;
  }

  get memoryConfig() {
    let values = [];
    let selectedMemory = this._memoryConfig?.querySelectorAll<SpCheckDesBox>(
      `check-des-box[checked]`
    ) as NodeListOf<SpCheckDesBox>;
    for (const litCheckBoxElement of selectedMemory) {
      values.push(litCheckBoxElement.value);
    }
    log('memoryConfig size is :' + values.length);
    return values;
  }

  get recordAbility() {
    let selectedMemory = this._abilityConfig?.querySelectorAll<SpCheckDesBox>(
      `check-des-box[checked]`
    ) as NodeListOf<SpCheckDesBox>;
    return selectedMemory.length > 0;
  }

  get traceEvents() {
    let values = [];
    if (this.hitrace && this.hitrace.checked) {
      let parent = this.shadowRoot?.querySelector('.user-events') as Element;
      const siblingNode = parent?.querySelectorAll<LitCheckBox>(`lit-check-box[name=userEvents][checked]`);
      for (const litCheckBoxElement of siblingNode) {
        values.push(litCheckBoxElement.value);
      }
    }
    log('traceEvents size is :' + values.length);
    return values;
  }

  get hilogConfig() {
    let logLevel = this.shadowRoot?.getElementById('logLevel') as LitCheckBox;
    if (logLevel.checked) {
      let logRadio = this.shadowRoot?.getElementById('log-radio') as LitRadioGroup;
      return logRadio.value;
    } else {
      return [];
    }
  }

  initElements(): void {
    this.traceConfigList = [
      {
        value: 'Scheduling details',
        isSelect: true,
        des: 'enables high-detailed tracking of scheduling events',
      },
      {
        value: 'CPU Frequency and idle states',
        isSelect: true,
        des: 'Records cpu frequency and idle state change viaftrace',
      },
      {
        value: 'Advanced ftrace config',
        isSelect: false,
        des:
          'Enable individual events and tune the kernel-tracing(ftrace) module.' +
          'The events enabled here are in addition to those from' +
          ' enabled by other probes.',
      },
    ];
    this._traceConfig = this.shadowRoot?.querySelector('.trace-config') as HTMLElement;
    this.traceConfigList.forEach((configBean) => {
      let checkDesBox = new SpCheckDesBox();
      checkDesBox.value = configBean.value;
      checkDesBox.checked = configBean.isSelect;
      checkDesBox.des = configBean.des;
      checkDesBox.addEventListener('onchange', () => {
        this.dispatchEvent(new CustomEvent('addProbe', {}));
      });
      this._traceConfig?.appendChild(checkDesBox);
    });
    this.memoryConfigList = [
      {
        value: 'Kernel meminfo',
        isSelect: false,
        des: 'polling of /proc/meminfo',
      },
      {
        value: 'Virtual memory stats',
        isSelect: false,
        des:
          'Periodically polls virtual memory stats from /proc/vmstat.' +
          ' Allows to gather statistics about swap,' +
          'eviction, compression and pagecache efficiency',
      },
    ];
    this._memoryConfig = this.shadowRoot?.querySelector('.memory-config');
    this.memoryConfigList.forEach((configBean) => {
      let checkDesBox = new SpCheckDesBox();
      checkDesBox.value = configBean.value;
      checkDesBox.checked = configBean.isSelect;
      checkDesBox.des = configBean.des;
      checkDesBox.addEventListener('onchange', () => {
        this.dispatchEvent(new CustomEvent('addProbe', {}));
      });
      this._memoryConfig?.appendChild(checkDesBox);
    });
    this.abilityConfigList = [
      {
        value: 'AbilityMonitor',
        isSelect: false,
        des: 'Tracks the AbilityMonitor',
      },
    ];
    this._abilityConfig = this.shadowRoot?.querySelector('.ability-config');
    this.abilityConfigList.forEach((configBean) => {
      let checkDesBox = new SpCheckDesBox();
      checkDesBox.value = configBean.value;
      checkDesBox.checked = configBean.isSelect;
      checkDesBox.des = configBean.des;
      checkDesBox.addEventListener('onchange', () => {
        this.dispatchEvent(new CustomEvent('addProbe', {}));
      });
      this._abilityConfig?.appendChild(checkDesBox);
    });

    this.hitraceConfigList = [
      { value: 'ability', isSelect: true },
      { value: 'accesscontrol', isSelect: false },
      { value: 'accessibility', isSelect: false },
      { value: 'account', isSelect: false },
      { value: 'ace', isSelect: true },
      { value: 'app', isSelect: true },
      { value: 'ark', isSelect: true },
      { value: 'binder', isSelect: true },
      { value: 'bluetooth', isSelect: false },
      { value: 'cloud', isSelect: false },
      { value: 'commonlibrary', isSelect: false },
      { value: 'daudio', isSelect: false },
      { value: 'dcamera', isSelect: false },
      { value: 'deviceauth', isSelect: false },
      { value: 'devicemanager', isSelect: false },
      { value: 'deviceprofile', isSelect: false },
      { value: 'dhfwk', isSelect: false },
      { value: 'dinput', isSelect: false },
      { value: 'disk', isSelect: true },
      { value: 'distributeddatamgr', isSelect: false },
      { value: 'dlpcre', isSelect: false },
      { value: 'dsched', isSelect: false },
      { value: 'dscreen', isSelect: false },
      { value: 'dslm', isSelect: false },
      { value: 'dsoftbus', isSelect: false },
      { value: 'ffrt', isSelect: false },
      { value: 'filemanagement', isSelect: false },
      { value: 'freq', isSelect: true },
      { value: 'graphic', isSelect: true },
      { value: 'gresource', isSelect: false },
      { value: 'hdcd', isSelect: false },
      { value: 'hdf', isSelect: false },
      { value: 'huks', isSelect: false },
      { value: 'i2c', isSelect: false },
      { value: 'idle', isSelect: true },
      { value: 'interconn', isSelect: false },
      { value: 'irq', isSelect: true },
      { value: 'mdfs', isSelect: false },
      { value: 'memory', isSelect: false },
      { value: 'memreclaim', isSelect: true },
      { value: 'misc', isSelect: false },
      { value: 'mmc', isSelect: true },
      { value: 'msdp', isSelect: false },
      { value: 'multimodalinput', isSelect: true },
      { value: 'musl', isSelect: false },
      { value: 'net', isSelect: false },
      { value: 'notification', isSelect: false },
      { value: 'nweb', isSelect: false },
      { value: 'ohos', isSelect: true },
      { value: 'pagecache', isSelect: true },
      { value: 'power', isSelect: false },
      { value: 'regulators', isSelect: false },
      { value: 'rpc', isSelect: true },
      { value: 'samgr', isSelect: false },
      { value: 'sched', isSelect: true },
      { value: 'sensors', isSelect: false },
      { value: 'sync', isSelect: true },
      { value: 'usb', isSelect: false },
      { value: 'ufs', isSelect: false },
      { value: 'useriam', isSelect: false },
      { value: 'window', isSelect: true },
      { value: 'workq', isSelect: true },
      { value: 'zaudio', isSelect: true },
      { value: 'zcamera', isSelect: true },
      { value: 'zimage', isSelect: true },
      { value: 'zmedia', isSelect: true },
    ];
    this.hitrace = this.shadowRoot?.getElementById('hitrace') as SpCheckDesBox;
    let parent = this.shadowRoot?.querySelector('.user-events') as Element;
    this.hitraceConfigList?.forEach((hitraceConfig: any) => {
      let litCheckBox = new LitCheckBox();
      litCheckBox.setAttribute('name', 'userEvents');
      litCheckBox.value = hitraceConfig.value;
      litCheckBox.checked = hitraceConfig.isSelect;
      litCheckBox.addEventListener('change', (ev: CustomEventInit<LitCheckBoxChangeEvent>) => {
        let detail = ev.detail;
        if (this.hitrace?.checked === false) {
          this.hitrace.checked = detail!.checked;
        }
        if (detail!.checked === false && this.hitrace?.checked === true) {
          let hasChecked = false;
          const nodes = parent?.querySelectorAll<LitCheckBox>(`lit-check-box[name=userEvents]`);
          nodes.forEach((vv) => {
            if (vv.checked) {
              hasChecked = true;
            }
          });
          if (!hasChecked) {
            this.hitrace.checked = hasChecked;
          }
        }
        this.dispatchEvent(new CustomEvent('addProbe', {}));
      });
      parent.append(litCheckBox);
    });
    this.bufferSizeSliderInit();
  }

  private bufferSizeSliderInit() {
    let ftraceBufferSizeSlider = this.shadowRoot?.querySelector<LitSlider>('#ftrace-buff-size-slider') as LitSlider;
    this.ftraceBufferSizeResult = this.shadowRoot?.querySelector('#ftrace-buff-size-div') as HTMLDivElement;
    ftraceBufferSizeSlider.sliderStyle = {
      minRange: 2048,
      maxRange: 307200,
      defaultValue: '20480',
      resultUnit: 'KB',
      stepSize: 2,
      lineColor: 'var(--dark-color3,#46B1E3)',
      buttonColor: '#999999',
    };
    let ftraceBufferSizeSliderParent = ftraceBufferSizeSlider!.parentNode as Element;
    let ftraceBuffSizeResultInput = this.shadowRoot?.querySelector('.ftrace-buff-size-result') as HTMLInputElement;
    ftraceBuffSizeResultInput.value = ftraceBufferSizeSlider.sliderStyle.defaultValue;
    ftraceBufferSizeSlider.addEventListener('input', (evt) => {
      ftraceBuffSizeResultInput.parentElement!.classList.remove('border-red');
      if (this.ftraceBufferSizeResult!.hasAttribute('percent')) {
        ftraceBuffSizeResultInput.value = Number(this.ftraceBufferSizeResult!.getAttribute('percent')).toString();
      } else {
        ftraceBuffSizeResultInput.value = '20480';
      }
    });
    ftraceBufferSizeSliderParent.setAttribute('percent', '20480');
    ftraceBuffSizeResultInput.style.color = 'var(--dark-color1,#000000)';
    ftraceBuffSizeResultInput.addEventListener('input', (ev) => {
      if (this.ftraceBufferSizeResult!.hasAttribute('percent')) {
        this.ftraceBufferSizeResult!.removeAttribute('percent');
      }
      ftraceBuffSizeResultInput.style.color = 'var(--dark-color1,#000000)';
      ftraceBuffSizeResultInput.parentElement!.style.backgroundColor = 'var(--dark-background5,#F2F2F2)';
      ftraceBuffSizeResultInput.style.backgroundColor = 'var(--dark-background5,#F2F2F2)';
      if (ftraceBuffSizeResultInput.value.trim() === '') {
        ftraceBuffSizeResultInput.style.color = 'red';
        ftraceBufferSizeSliderParent.setAttribute('percent', '20480');
        return;
      }
      let ftraceBufferSize = Number(ftraceBuffSizeResultInput.value);
      if (
        ftraceBufferSize < ftraceBufferSizeSlider!.sliderStyle.minRange ||
        ftraceBufferSize > ftraceBufferSizeSlider!.sliderStyle.maxRange
      ) {
        ftraceBuffSizeResultInput.parentElement!.classList.add('border-red');
        ftraceBufferSizeSliderParent.setAttribute('percent', '20480');
      } else {
        ftraceBuffSizeResultInput.parentElement!.classList.remove('border-red');
        ftraceBufferSizeSlider!.percent = ftraceBuffSizeResultInput.value;
        let htmlInputElement = ftraceBufferSizeSlider!.shadowRoot?.querySelector('#slider') as HTMLInputElement;
        htmlInputElement.value = ftraceBuffSizeResultInput.value;
        ftraceBufferSizeSliderParent.setAttribute('percent', ftraceBuffSizeResultInput.value);
        ftraceBufferSizeSliderParent.setAttribute('percentValue', ftraceBuffSizeResultInput.value);
      }
    });
    ftraceBuffSizeResultInput.addEventListener('focusout', (ev) => {
      if (ftraceBuffSizeResultInput.value.trim() === '') {
        ftraceBuffSizeResultInput.parentElement!.classList.remove('border-red');
        ftraceBufferSizeSliderParent.setAttribute('percent', '20480');
        ftraceBuffSizeResultInput.value = '20480';
        ftraceBuffSizeResultInput.style.color = 'var(--dark-color,#6a6f77)';
        ftraceBufferSizeSliderParent.setAttribute('percent', ftraceBuffSizeResultInput.value);
        ftraceBufferSizeSliderParent.setAttribute('percentValue', ftraceBuffSizeResultInput.value);
        ftraceBufferSizeSlider!.percent = ftraceBuffSizeResultInput.value;
        let htmlInputElement = ftraceBufferSizeSlider!.shadowRoot?.querySelector('#slider') as HTMLInputElement;
        htmlInputElement.value = ftraceBuffSizeResultInput.value;
      }
    });
  }

  initHtml(): string {
    return `
        <style>
        .recordText {
           font-family: Helvetica-Bold;
           font-size: 1em;
           color: var(--dark-color1,#000000);
           line-height: 28px;
           font-weight: 700;
           margin-bottom: 20px;
        }

        :host{
            display: inline-block;
            background: var(--dark-background3,#FFFFFF);
            width: 100%;
            height: 100%;
            border-radius: 0px 16px 16px 0px;
        }

        .root {
            margin-right: 30px;
            padding-top: 30px;
            padding-left: 54px;
            margin-bottom: 30px;
            font-size:16px;
        }
        
        .config-page {
            height: 95%;
            font-size: 0.875em;
        }

        .trace-config{
           display: flex;
           flex-direction: column;
           width: 50%;
           gap: 10px;
           margin-bottom: 20px;
        }

        .memory-config{
           display: grid;
           grid-template-columns: repeat(2, 1fr);
           border-style: solid none none none;
           border-color: #D5D5D5;
           padding-top: 15px;
           margin-top: 15px;
           gap: 10px;
        }
        
        .ability-config{
           display: grid;
           grid-template-columns: repeat(2, 1fr);
           border-style: solid none none none;
           border-color: #D5D5D5;
           padding-top: 15px;
           margin-top: 15px;
           gap: 10px;
        }

        .span-col-2{
           grid-column: span 2 / auto;
        }

        .log-config{
           display: grid;
           grid-template-columns: repeat(2, 1fr);
           border-style: solid none none none;
           border-color: #D5D5D5;
           padding-top: 15px;
           gap: 10px;
        }

        #hitrace-cat{
           display: grid;
           grid-template-columns: 1fr 1fr;
        }
        .user-events{
           display: grid;
           grid-template-columns: repeat(4, 1fr);
           grid-template-rows: repeat(2, 1fr);
           gap: 10px;
           margin-left: 15px;;
        }
        #ftrace-buff-size-div {
            width: 100%;
            height: min-content;
            display: grid;
            grid-template-columns: 1fr min-content;
        }
        .buffer-size-des {
            opacity: 0.6;
            font-family: Helvetica;
            font-size: 1em;
            color: var(--dark-color,#000000);
            text-align: left;
            line-height: 20px;
            font-weight: 400;
        }
        .ftrace-buff-size-result-div{
            display: grid;
            grid-template-rows: 1fr;
            grid-template-columns:  min-content min-content;
            background-color: var(--dark-background5,#F2F2F2);
            -webkit-appearance:none;
            color:var(--dark-color,#6a6f77);
            width: 150px;
            margin: 0 20px 0 0;
            height: 40px;
            border-radius:20px;
            outline:0;
            border:1px solid var(--dark-border,#c8cccf);
        }
        .ftrace-buff-size-result{
            background-color: var(--dark-background5,#F2F2F2);
            -webkit-appearance:none;
            color:var(--dark-color,#6a6f77);
            border: none;
            text-align: center;
            width: 90px;
            font-size:14px;
            outline:0;
            margin: 5px 0 5px 5px;
        }
        .border-red {
           border:1px solid red;
        }
        </style>
        <div class="root">
            <div class="recordText" >Record mode</div>
            <div class="config-page">
                <div>
                    <div class="trace-config"></div>
                    <div class="span-col-2" id="hitrace-cat">
                      <check-des-box id="hitrace" checked="true" value ="Hitrace categories" des="Enables C++ codebase annotations (HTRACE_BEGIN() / os.Trace())">
                      </check-des-box>
                      <div class="user-events">
                          <slot></slot>
                      </div>
                    </div>
                    <div>
                       <div>
                          <p>Buffer Size</p>
                          <p class="buffer-size-des">The ftrace buffer size range is 2048 KB to 307200 KB</p>
                       </div>
                       <div id="ftrace-buff-size-div">
                          <lit-slider id="ftrace-buff-size-slider" defaultColor="var(--dark-color3,#46B1E3)" open dir="right">
                          </lit-slider>
                          <div class='ftrace-buff-size-result-div'>
                              <input class="ftrace-buff-size-result" type="text" value='20480' onkeyup="this.value=this.value.replace(/\\D/g,'')">
                              <span style="text-align: center; margin: 8px"> KB </span>
                           </div>
                       </div>
                    </div>
                </div>
                <div class="memory-config">
                    <div class="span-col-2">
                      <span>Memory Config</span>
                    </div>
                </div>
                <div class="ability-config">
                    <div class="span-col-2">
                      <span>Ability Config</span>
                    </div>
                </div>
            </div>
        </div>
        `;
  }

  //当 custom element首次被插入文档DOM时，被调用。
  public connectedCallback() {
    let parent = this.shadowRoot?.querySelector('.user-events') as Element;
    const siblingNode = parent?.querySelectorAll<LitCheckBox>(`lit-check-box[name=userEvents]`);
    this.hitrace!.addEventListener('onchange', (ev: CustomEventInit<LitCheckBoxChangeEvent>) => {
      let detail = ev.detail;
      siblingNode.forEach((node) => {
        node.checked = detail!.checked;
      });
      this.dispatchEvent(new CustomEvent('addProbe', {}));
    });
  }
}
