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
import LitSwitch from '../../../base-ui/switch/lit-switch.js';

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
  private ftraceSlider: LitSlider | null | undefined;

  private ftraceBuffSizeResultInput: HTMLInputElement | null | undefined;

  set startSamp(allocationStart: boolean) {
    if (allocationStart) {
      this.setAttribute('startSamp', '');
    } else {
      this.removeAttribute('startSamp');
    }
  }

  get startSamp(): boolean {
    return this.hasAttribute('startSamp');
  }

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
      'check-des-box[checked]'
    ) as NodeListOf<SpCheckDesBox>;
    for (const litCheckBoxElement of selectedMemory) {
      values.push(litCheckBoxElement.value);
    }
    log(`memoryConfig size is :${values.length}`);
    return values;
  }

  get recordAbility() {
    let selectedMemory = this._abilityConfig?.querySelectorAll<SpCheckDesBox>(
      'check-des-box[checked]'
    ) as NodeListOf<SpCheckDesBox>;
    return selectedMemory.length > 0;
  }

  get traceEvents() {
    let values = [];
    if (this.hitrace && this.hitrace.checked) {
      let parent = this.shadowRoot?.querySelector('.user-events') as Element;
      const siblingNode = parent?.querySelectorAll<LitCheckBox>('lit-check-box[name=userEvents][checked]');
      for (const litCheckBoxElement of siblingNode) {
        values.push(litCheckBoxElement.value);
      }
    }
    log(`traceEvents size is :${values.length}`);
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
    ftraceBuffSizeResultInput!.onkeydown = (ev): void => {
      // @ts-ignore
      if (ev.key === '0' && ev.target.value.length === 1 && ev.target.value === '0') {
        ev.preventDefault();
      }
    };
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

  private unDisable() {
    this.startSamp = true;
    let checkDesBoxDis = this.shadowRoot?.querySelectorAll<SpCheckDesBox>('check-des-box');
    let litCheckBoxDis = this.shadowRoot?.querySelectorAll<LitCheckBox>('lit-check-box');

    let defaultSelected: any = [];
    defaultSelected = defaultSelected.concat(
      this.traceConfigList,
      this.memoryConfigList,
      this.abilityConfigList,
      this.hitraceConfigList
    );

    this.shadowRoot?.querySelector<SpCheckDesBox>("[value='Hitrace categories']")?.setAttribute('checked', 'true');
    this.ftraceSlider!.removeAttribute('disabled');

    checkDesBoxDis?.forEach((item: any) => {
      item.removeAttribute('disabled');
    });

    litCheckBoxDis?.forEach((item: any) => {
      item.removeAttribute('disabled');
    });

    defaultSelected.filter((item: any) => {
      if (item.isSelect)
        this.shadowRoot?.querySelector<SpCheckDesBox>(`[value='${item.value}']`)?.setAttribute('checked', 'true');
    });
  }

  private disable() {
    this.startSamp = false;
    let checkDesBoxDis = this.shadowRoot?.querySelectorAll<SpCheckDesBox>('check-des-box');
    let litCheckBoxDis = this.shadowRoot?.querySelectorAll<LitCheckBox>('lit-check-box');

    this.ftraceSlider!.setAttribute('disabled', '');

    checkDesBoxDis?.forEach((item: any) => {
      item.setAttribute('disabled', '');
      item.checked = false;
    });

    litCheckBoxDis?.forEach((item: any) => {
      item.setAttribute('disabled', '');
      item.checked = false;
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
        lit-switch {
          height: 38px;
          margin-top: 10px;
          display:inline;
          float: right;
        }
        </style>
        <div class="root">
            <div class="recordText" >
                <span class="record-title">Record mode</span>
                <lit-switch checked="true"></lit-switch>
                </div>
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
                          <div class='ftrace-buff-size-result-div' >
                              <input class="ftrace-buff-size-result" type="text" value='20480' oninput="if(this.value > 307200){this.value = '307200'} if(this.value > 0 && this.value.toString().startsWith('0')){ this.value = Number(this.value) }" onkeyup="this.value=this.value.replace(/\\D/g,'')">
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
    const siblingNode = parent?.querySelectorAll<LitCheckBox>('lit-check-box[name=userEvents]');
    this.hitrace!.addEventListener('onchange', (ev: CustomEventInit<LitCheckBoxChangeEvent>) => {
      let detail = ev.detail;
      siblingNode.forEach((node) => {
        node.checked = detail!.checked;
      });
      this.dispatchEvent(new CustomEvent('addProbe', {}));
    });
  }
}
