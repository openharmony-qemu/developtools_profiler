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
import LitSwitch from '../../../dist/base-ui/switch/lit-switch';

describe('LitSwitch Test', () => {
  let litSwitch = new LitSwitch();
  litSwitch.checked = true;
  litSwitch.checked = false;
  litSwitch.disabled = true;
  litSwitch.disabled = false;

  it('LitSwitchTest01', () => {
    expect(litSwitch.name).toBeNull();
  });

  it('LitSwitchTest02', () => {
    expect(litSwitch.disabled).toBeFalsy();
  });

  it('LitSwitchTest03', () => {
    expect(litSwitch.checked).toBeFalsy();
  });

  it('LitSwitchTest04', () => {
    LitSwitch.switch = document.querySelector('#switch') as HTMLInputElement;
    expect(litSwitch.connectedCallback()).toBeUndefined();
  });

  it('LitSwitchTest05', () => {
    expect(litSwitch.attributeChangedCallback('disabled', 'disabled', '')).toBeUndefined();
  });

  it('LitSwitchTest06', () => {
    expect(litSwitch.attributeChangedCallback('disabled', 'disabled', null)).toBeUndefined();
  });

  it('LitSwitchTest07', () => {
    expect(litSwitch.attributeChangedCallback('checked', 'disabled', '')).toBeUndefined();
  });

  it('LitSwitchTest08', () => {
    expect(litSwitch.attributeChangedCallback('checked', 'disabled', null)).toBeUndefined();
  });

  it('LitSwitchTest09', () => {
    expect(litSwitch.initHtml()).toMatchInlineSnapshot(`
"
        <style>
        :host{ 
            display:inline-block; 
            -webkit-tap-highlight-color: transparent;
        }
        #name{
            transition:0.31s width,0.31s height,0.31s background-color;
            width:2.41em;
            height:1.21em;
            background: #3391FF;
            display:flex;
            padding:0.124em;
            border-radius:1.21em;
            cursor:pointer;
        }
        
        :host(:not([checked])) #name {
           background: #999999;
        }
       
        #name::before{
            content:'';
            flex:0;
            transition:.2s cubic-bezier(.12, .4, .29, 1.46) flex;
        }
        #name::after{
            content:'';
            width:.4em;
            height:.4em;
            border-radius:1.2em;
            border:.4em solid #fff;
            background-color:#ffffff;
            transition:.3s background,.3s padding,.3s width,.3s height,.3s border-radius,.3s border;
            box-shadow: 0 2px 4px 0 rgba(0,35,11,0.2);
        }
        #name:active::after{
            padding:0 .3em;
        }
        #switch:checked+#name{
            background:#42b983);
        }
        #switch:checked+#name::before{
            flex:1;
        }
        #switch{
            position:absolute;
            clip:rect(0,0px,0px,0);
        }
        :host(:focus-within) #name::after,:host(:active) ::after{ 
            background:#42b983;
        }
        :host(:focus-within) #name{ 
            box-shadow: 0 0 10px rgba(0,0,0,0.1); 
        }
        :host(:focus-within) #switch,:host(:active) #switch{
            z-index:2
        }
        :host([disabled]){ 
            pointer-events: none; 
            opacity:.5; 
        }
        :host([disabled]) #name{ 
            pointer-events: all;  
            cursor: not-allowed; 
        }
        </style>
        <input type="checkbox" id="switch"><label id="name" for="switch"></label>
        "
`);
  });
});
