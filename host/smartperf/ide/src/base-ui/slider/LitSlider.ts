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

import { BaseElement, element } from '../BaseElement.js';

@element('lit-slider')
export class LitSlider extends BaseElement {
  private litSliderStyle: LitSliderStyle | undefined | null;
  private litSlider: HTMLInputElement | undefined | null;
  private litSliderCon: HTMLDivElement | undefined | null;
  private litResult: HTMLInputElement | undefined | null;
  private slotEl: HTMLSlotElement | undefined | null;
  private currentValue: number = 0;
  private defaultTimeText: string | undefined | null;

  static get observedAttributes() {
    return ['percent', 'disabled-X', 'custom-slider', 'custom-line', 'custom-button', 'disabled'];
  }

  get sliderStyle(): LitSliderStyle {
    if (this.litSliderStyle) {
      return this.litSliderStyle;
    } else {
      return {
        minRange: 0,
        maxRange: 100,
        defaultValue: '0',
        resultUnit: '',
        stepSize: 1,
        lineColor: 'var(--dark-color3,#46B1E3)',
        buttonColor: '#999999',
      };
    }
  }

  get disabled() {
    return this.getAttribute('disabled') !== null;
  }

  set disabled(value) {
    if (value === null || value === false) {
      this.removeAttribute('disabled');
    } else {
      this.setAttribute('disabled', '');
    }
  }

  set sliderStyle(value: LitSliderStyle) {
    this.litSliderStyle = value;
    this.currentValue = Number(value.defaultValue);
    this.litSliderStyle.defaultValue = value.defaultValue;
    if (this.litSliderStyle.resultUnit === 'h:m:s') {
      let timeData = this.litSliderStyle.defaultValue.split(':');
      let timeSize = Number(timeData[0]) * 3600 + Number(timeData[1]) * 60 + Number(timeData[2]);
      this.defaultTimeText = timeSize.toString();
      let defaultSize =
        ((timeSize - this.litSliderStyle.minRange) * 100) /
        (this.litSliderStyle.maxRange - this.litSliderStyle.minRange);
      this.litSlider!.style.backgroundSize = defaultSize + '%';
    } else {
      this.defaultTimeText = this.litSliderStyle.defaultValue;
      this.litSlider!.style.backgroundSize = '0%';
      if (Number(this.litSliderStyle.defaultValue)) {
        let defaultSize =
          ((Number(this.litSliderStyle.defaultValue) - this.litSliderStyle.minRange) /
            (this.litSliderStyle.maxRange - this.litSliderStyle.minRange)) *
          100;
        this.litSlider!.style.backgroundSize = defaultSize + '%';
      }
    }
    let htmlInputElement = this.shadowRoot?.querySelector('#slider') as HTMLInputElement;
    let attribute = htmlInputElement.getAttribute('type');
    if (attribute === 'range') {
      htmlInputElement!.setAttribute('value', this.defaultTimeText!);
      htmlInputElement!.setAttribute('min', this.litSliderStyle!.minRange.toString());
      htmlInputElement!.setAttribute('max', this.litSliderStyle!.maxRange.toString());
      htmlInputElement!.setAttribute('step', this.litSliderStyle!.stepSize.toString());
    }
  }

  get disabledX() {
    return this.getAttribute('disabled-X') || '';
  }

  set disabledX(value: string) {
    if (value) {
      this.setAttribute('disabled-X', '');
    } else {
      this.removeAttribute('disabled-X');
    }
  }

  get customSlider() {
    return this.getAttribute('custom-slider') || '';
  }

  set customSlider(value: string) {
    if (value) {
      this.setAttribute('custom-slider', '');
    } else {
      this.removeAttribute('custom-slider');
    }
  }

  get customLine() {
    return this.getAttribute('custom-line') || '';
  }

  set customLine(value: string) {
    this.setAttribute('custom-line', value);
  }

  get customButton() {
    return this.getAttribute('custom-button') || '';
  }

  set customButton(value: string) {
    this.setAttribute('custom-button', value);
  }

  get percent() {
    return this.getAttribute('percent') || '';
  }

  set percent(value: string) {
    this.setAttribute('percent', value);
    let resultNumber =
      ((Number(value) - this.sliderStyle!.minRange) * 100) / (this.sliderStyle!.maxRange - this.sliderStyle!.minRange);
    this.litSlider!.style.backgroundSize = resultNumber + '%';
  }

  get resultUnit() {
    return this.getAttribute('resultUnit') || '';
  }

  set resultUnit(value: string) {
    this.setAttribute('resultUnit', value);
  }

  initElements(): void {
    this.litSlider = this.shadowRoot?.querySelector('#slider') as HTMLInputElement;
  }

  initHtml(): string {
    return `
        <style>
        /*
         * Outer box style
         */
        :host{ 
            box-sizing:border-box; 
            display:flex;
            
        }
        /*
         * The mouse is missing
         */
        :host([disabled]){ 
            opacity:0.8; 
            cursor:not-allowed; 
        }
        /*
         * Disable sliding
         */
        :host([disabled]) input[type="range"]{
            pointer-events:none;
        }
        /*
         * Currently the entire sliding container is controlled
         */
        #slider-con{ 
            cursor:pointer;
            display:flex;
            align-items:center;
            width:95%;
            grid-auto-flow: row dense;
            position: relative;
        }
        /*
         * Display prompt information
         */
        :host([showtips]){
            pointer-events:all;
        }
        
        #slider{
            background-color: var(--dark-background7,#D8D8D8);
            z-index: 5;
        }
    
        /*
         * Slider basic style
         */
        input[type="range"]{
            pointer-events:all;
            margin:0 -5px;
            width: 100%;
            -webkit-appearance: none;
            outline : 0;
            background: rgba(0,0,0,0.1);
            height: 10px;
            border-radius:2px;
            background: -webkit-linear-gradient(right, ${
              this.getAttribute('defaultColor') ? this.getAttribute('defaultColor') : '#46B1E3'
            }, ${this.getAttribute('defaultColor') ? this.getAttribute('defaultColor') : '#46B1E3'}) no-repeat;
        }
        
        /*
         * Slider-line slidedAble area component
         */
        input[type="range"]::-webkit-slider-runnable-track{
            display: flex;
            align-items: center;
            position: relative;
            height: 10px;
            border-radius:5px;
        }
        
         /*
         * Slider slider component
         */
        input[type="range"]::-webkit-slider-thumb{
            -webkit-appearance: none;
            position: relative;
            width:20px;
            height:20px;
            margin-top: -4px;
            border-radius: 5px;
            background:#999999;
            transition:0.2s cubic-bezier(.12, .4, .29, 1.46);
        }
        
        input[type="range"]:focus{
            z-index:2;
        }

        :host(:focus-within) #slider-con,:host(:hover) #slider-con{
            z-index:10
        }
        
        :host([disabled]) #slider{ 
            opacity:0.6; 
        }
        </style>
        <slot id="slot"></slot>
        <div id='slider-con' dir="right">
            <input id="slider" type="range" max="10000000">
        </div>
        `;
  }

  // It is called when the custom element is first inserted into the document DOM.
  connectedCallback() {
    this.slotEl = this.shadowRoot?.querySelector('#slot');
    this.litSliderCon = this.shadowRoot?.querySelector('#slider-con');
    // Add a slider for input event listeners
    this.litSlider?.addEventListener('input', this.inputChangeEvent);
    this.litSlider?.addEventListener('change', this.inputChangeEvent);
    this.litSliderStyle = this.sliderStyle;
  }

  inputChangeEvent = (event: any) => {
    if (this.litSlider) {
      this.currentValue = parseInt(this.litSlider?.value);
      let resultNumber =
        ((this.currentValue - this.litSliderStyle!.minRange) * 100) /
        (this.litSliderStyle!.maxRange - this.litSliderStyle!.minRange);
      this.percent = Number(resultNumber) + '%';
      this.litSliderCon?.style.setProperty('percent', this.currentValue + '%');
      let parentElement = this.parentNode as Element;
      parentElement.setAttribute('percent', this.currentValue + '');
      if (this.sliderStyle.resultUnit === 'h:m:s') {
        this.litSlider!.style.backgroundSize = this.percent;
      } else {
        this.litSlider!.style.backgroundSize = this.percent;
      }
      this.parentElement!.setAttribute('percent', this.litSlider?.value);
    }
  };

  disconnectedCallback() {
    this.litSlider?.removeEventListener('input', this.inputChangeEvent);
    this.litSlider?.removeEventListener('change', this.inputChangeEvent);
  }

  adoptedCallback() {}

  attributeChangedCallback(name: string, oldValue: string, newValue: string) {
    switch (name) {
      case 'percent':
        if (newValue === null || newValue === '0%') {
          let parentElement = this.parentNode as Element;
          parentElement?.removeAttribute('percent');
        } else {
          let parentElement = this.parentNode as Element;
        }
        break;
      default:
        break;
    }
  }

  formatSeconds(value: string) {
    let result = parseInt(value);
    let hours = Math.floor(result / 3600) < 10 ? '0' + Math.floor(result / 3600) : Math.floor(result / 3600);
    let minute =
      Math.floor((result / 60) % 60) < 10 ? '0' + Math.floor((result / 60) % 60) : Math.floor((result / 60) % 60);
    let second = Math.floor(result % 60) < 10 ? '0' + Math.floor(result % 60) : Math.floor(result % 60);
    let resultTime = '';
    if (hours === '00') {
      resultTime += `00:`;
    } else {
      resultTime += `${hours}:`;
    }
    if (minute === '00') {
      resultTime += `00:`;
    } else {
      resultTime += `${minute}:`;
    }
    resultTime += `${second}`;
    return resultTime;
  }
}

export interface LitSliderStyle {
  minRange: number;
  maxRange: number;
  defaultValue: string;
  resultUnit: string;
  stepSize: number;
  lineColor?: string;
  buttonColor?: string;
}

export interface LitSliderLineStyle {
  lineWith: number;
  lineHeight: number;
  border?: string;
  borderRadiusValue?: number;
  lineChangeColor?: string;
}

export interface LitSliderButtonStyle {
  buttonWith: number;
  buttonHeight: number;
  border?: string;
  borderRadiusValue?: number;
  buttonChangeColor?: string;
}
