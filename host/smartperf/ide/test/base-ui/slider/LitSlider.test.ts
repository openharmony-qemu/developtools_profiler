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

// @ts-ignore
import { LitSlider } from '../../../dist/base-ui/slider/LitSlider.js';

describe('LitSlider Test', () => {
  let litSliderPanel = new LitSlider();

  litSliderPanel.disabledX = 'disabledX';
  litSliderPanel.customSlider = 'customSlider';
  litSliderPanel.customLine = 'customLine';
  litSliderPanel.customButton = 'customButton';
  litSliderPanel.percent = 'percent';
  litSliderPanel.resultUnit = 'resultUnit';

  it('LitSliderTest01', () => {
    expect(litSliderPanel.disabledX).toEqual('');
  });

  it('LitSliderTest02', () => {
    expect(litSliderPanel.customSlider).toEqual('');
  });

  it('LitSliderTest03', () => {
    expect(litSliderPanel.customLine).toEqual('customLine');
  });

  it('LitSliderTest04', () => {
    expect(litSliderPanel.customButton).toEqual('customButton');
  });

  it('LitSliderTest05', () => {
    expect(litSliderPanel.percent).toEqual('percent');
  });

  it('LitSliderTest06', () => {
    expect(litSliderPanel.resultUnit).toEqual('resultUnit');
  });

  it('LitSliderTest07', () => {
    expect(litSliderPanel.formatSeconds(10)).toBe('00:00:10');
  });

  it('LitSliderTest9', () => {
    expect(litSliderPanel.adoptedCallback()).toBeUndefined();
  });

  it('LitSliderTest10', () => {
    expect(litSliderPanel.disconnectedCallback()).toBeUndefined();
  });

  it('LitSliderTest11', () => {
    expect(litSliderPanel.disconnectedCallback()).toBeUndefined();
  });

  it('LitSliderTest12', function () {
    expect(litSliderPanel.attributeChangedCallback('percent', '', '0%' || null)).toBeUndefined();
  });

  it('LitSliderTest13', function () {
    expect(litSliderPanel.initHtml()).toMatchInlineSnapshot(`
"
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
            background: -webkit-linear-gradient(right, #46B1E3, #46B1E3) no-repeat;
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
        
        </style>
        <slot id="slot"></slot>
        <div id='slider-con' dir="right">
            <input id="slider" type="range" max="10000000">
        </div>
        "
`);
  });

  it('LitSliderTest14', () => {
    litSliderPanel.disabledX = false;
    expect(litSliderPanel.disabledX).toBeFalsy();
  });

  it('LitSliderTest15', () => {
    litSliderPanel.customSlider = false;
    expect(litSliderPanel.customSlider).toBeFalsy();
  });

  it('LitSliderTest16', () => {
    expect(litSliderPanel.formatSeconds(36000)).toBe('10:00:00');
  });

  it('LitSliderTest17', () => {
    expect(litSliderPanel.formatSeconds(4000)).toBe('01:06:40');
  });
});
