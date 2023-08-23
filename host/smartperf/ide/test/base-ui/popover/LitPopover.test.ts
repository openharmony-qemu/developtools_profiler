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
import { LitPopover } from '../../../dist/base-ui/popover/LitPopover.js';

describe('LitPopover Test', () => {
  it('LitPopover01', () => {
    let litPopover = new LitPopover();
    expect(litPopover).not.toBeUndefined();
    expect(litPopover).not.toBeNull();
  });

  it('LitPopover02', () => {
    let litPopover = new LitPopover();
    expect(litPopover.open).toBeFalsy();
  });

  it('LitPopover03', () => {
    let litPopover = new LitPopover();
    litPopover.open = true;
    expect(litPopover.open).toBeTruthy();
  });

  it('LitPopover04', () => {
    let litPopover = new LitPopover();
    litPopover.open = false;
    expect(litPopover.open).toBeFalsy();
  });

  it('LitPopover05', () => {
    let litPopover = new LitPopover();
    litPopover.direction = 'topleft';
    expect(litPopover.direction).toEqual('topleft');
  });

  it('LitPopover06', () => {
    let litPopover = new LitPopover();
    expect(litPopover.direction).toEqual('topright');
  });

  it('LitPopover07', () => {
    let litPopover = new LitPopover();
    litPopover.type = 'multiple';
    litPopover.dataSource = [
      {
        text: '# Samples',
        isSelected: true,
      },
    ];
    expect(litPopover.select).toEqual(['# Samples']);
  });

  it('LitPopover07', () => {
    let litPopover = new LitPopover();
    litPopover.type = 'radio';
    litPopover.dataSource = [
      {
        text: '# Samples',
        isSelected: true,
      },
    ];
    expect(litPopover.select).toEqual(['# Samples']);
  });

  it('LitPopover08', () => {
    let litPopover = new LitPopover();
    litPopover.type = 'multiple-text';
    litPopover.dataSource = [
      {
        text: '# Samples',
        isSelected: true,
      },
    ];
    expect(litPopover.select).toEqual(['# Samples']);
  });

  it('LitPopover09', () => {
    let litPopover = new LitPopover();
    litPopover.type = 'radio';
    litPopover.title = 'tee';
    litPopover.dataSource = [
      {
        text: '# Samples',
        isSelected: true,
      },
    ];
    expect(litPopover.select).toEqual(['# Samples']);
  });

  it('LitPopover10', () => {
    let litPopover = new LitPopover();
    litPopover.type = 'multiple-text';
    litPopover.title = 'tee';
    litPopover.dataSource = [
      {
        text: '# Samples',
        isSelected: true,
      },
    ];
    expect(litPopover.trigger).not.toBeUndefined();
  });

  it('LitPopover11', () => {
    let litPopover = new LitPopover();
    litPopover.type = 'multiple-text';
    litPopover.title = 'tee';
    litPopover.dataSource = [
      {
        text: '# Samples',
        isSelected: false,
      },
    ];
    expect(litPopover.limit).toEqual({
      textLowerLimit: '0',
      textUpperLimit: '∞',
    });
  });

  it('LitPopover14', () => {
    let litPopover = new LitPopover();
    litPopover.type = 'data-ming';
    litPopover.title = 'tee';
    litPopover.dataSource = [
      {
        text: '# Samples',
        isSelected: false,
      },
    ];
    expect(litPopover.limit).toEqual({
      textLowerLimit: '',
      textUpperLimit: '',
    });
  });

  it('LitPopover15', () => {
    let litPopover = new LitPopover();
    litPopover.type = 'multiple-text';
    litPopover.title = 'tee';
    litPopover.dataSource = [
      {
        text: '# Samples',
        isSelected: true,
      },
    ];
    expect(litPopover.limit).toEqual({
      textLowerLimit: '0',
      textUpperLimit: '∞',
    });
  });

  it('LitPopover12', () => {
    let litPopover = new LitPopover();
    expect(litPopover.initHtml()).toMatchInlineSnapshot(`
"
        <style>
        :host {
            display:inline-block;
            position:relative;
            overflow:visible;
        }
        :host([direction="top"]) ::slotted(lit-pop-content){
            bottom:100%;
            left:50%;
            transform:translate(-50%,-10px) scale(0);
            transform-origin: center bottom;
        }
        :host([direction="top"]) ::slotted(lit-pop-content)::after{
            content: '';
            position: absolute; 
            top: 100%;
            left: 50%;
            border-top: 10px solid #FFF;
            border-right: 10px solid transparent;
            border-left: 10px solid transparent;

        }
        :host([direction="top"]) ::slotted(lit-pop-content[open]),
        :host([direction="top"][trigger="hover"]:hover) ::slotted(lit-pop-content),
        :host([direction="top"][trigger="focus"]:focus-within) ::slotted(lit-pop-content){
            transform:translate(-50%,-10px) scale(1);
        }
        
        :host([direction="bottom"]) ::slotted(lit-pop-content){
            top:100%;
            left:50%;
            transform:translate(-50%,10px) scale(0);
            transform-origin: center top;
        }
        :host([direction="bottom"]) ::slotted(lit-pop-content)::after{
            content: '';
            position: absolute;
            bottom: 100%;
            left: 50%;
            border-bottom: 10px solid #FFF; 
            border-right: 10px solid transparent;
            border-left: 10px solid transparent;
        }
        :host([direction="bottom"]) ::slotted(lit-pop-content[open]),
        :host([direction="bottom"][trigger="hover"]:hover) ::slotted(lit-pop-content),
        :host([direction="bottom"][trigger="focus"]:focus-within) ::slotted(lit-pop-content){
            transform:translate(-50%,10px) scale(1);
        }
        
        
        :host([direction="left"]) ::slotted(lit-pop-content){
            right:100%;
            top:50%;
            transform:translate(-10px,-50%) scale(0);
            transform-origin: right;
        }
        :host([direction="left"]) ::slotted(lit-pop-content)::after{
            content: '';
            position: absolute;
            bottom: 40%;
            left: 100%;
            border-left: 10px solid #FFF; 
            border-bottom: 10px solid transparent;
            border-top: 10px solid transparent;
        }
        :host([direction="left"]) ::slotted(lit-pop-content[open]),
        :host([direction="left"][trigger="hover"]:hover) ::slotted(lit-pop-content),
        :host([direction="left"][trigger="focus"]:focus-within) ::slotted(lit-pop-content){
            transform:translate(-10px,-50%) scale(1);
        }
        :host([direction="right"]) ::slotted(lit-pop-content){
            left:100%;
            top:50%;
            transform:translate(10px,-50%) scale(0);
            transform-origin: left;
        }
        :host([direction="right"]) ::slotted(lit-pop-content)::after{
            content: '';
            position: absolute;
            bottom: 40%;
            right: 100%;
            border-right: 10px solid #FFF; 
            border-bottom: 10px solid transparent;
            border-top: 10px solid transparent;
        }
        :host([direction="right"]) ::slotted(lit-pop-content[open]),
        :host([direction="right"][trigger="hover"]:hover) ::slotted(lit-pop-content),
        :host([direction="right"][trigger="focus"]:focus-within) ::slotted(lit-pop-content){
            transform:translate(10px,-50%) scale(1);
        }
        
        
        
        :host([direction="leftbottom"]) ::slotted(lit-pop-content){
            right:100%;
            top:0;
            transform:translate(-10px) scale(0);
        }
        :host([direction="leftbottom"]) ::slotted(lit-pop-content[open]),
        :host([direction="leftbottom"][trigger="hover"]:hover) ::slotted(lit-pop-content),
        :host([direction="leftbottom"][trigger="focus"]:focus-within) ::slotted(lit-pop-content){
            transform:translate(-10px) scale(1);
        }
        
        :host([direction="leftbottom"]) ::slotted(lit-pop-content)::after{
            content: '';
            position: absolute;
            top: 10%;
            left: 100%;
            border-left: 10px solid #FFF; 
            border-bottom: 10px solid transparent;
        }
        
        :host([direction="lefttop"]) ::slotted(lit-pop-content){
            right:100%;
            bottom:0;
            transform:translate(-10px) scale(0);
            transform-origin: right bottom;
        }
        :host([direction="lefttop"]) ::slotted(lit-pop-content)::after{
            content: '';
            position: absolute;
            bottom: 10%;
            left: 100%;
            border-left: 10px solid #FFF; 
            border-top: 10px solid transparent;
        }
        :host([direction="lefttop"]) ::slotted(lit-pop-content[open]),
        :host([direction="lefttop"][trigger="hover"]:hover) ::slotted(lit-pop-content),
        :host([direction="lefttop"][trigger="focus"]:focus-within) ::slotted(lit-pop-content){
            transform:translate(-10px) scale(1);
        }
        :host([direction="topright"]) ::slotted(lit-pop-content){
            bottom:100%;
            left:50%;
            transform:translate(0,-10px) scale(0);
            transform-origin: left bottom;
        }
        :host([direction="topright"]) ::slotted(lit-pop-content)::after{
            content: '';
            position: absolute; 
            top: 100%;
            left: 0%;
            border-top: 10px solid #FFF;
            border-right: 10px solid transparent;
        }
        :host([direction="topright"]) ::slotted(lit-pop-content[open]),
        :host([direction="topright"][trigger="hover"]:hover) ::slotted(lit-pop-content),
        :host([direction="topright"][trigger="focus"]:focus-within) ::slotted(lit-pop-content){
            transform:translate(0,-10px) scale(1);
        }
        
        
        :host([direction="topleft"]) ::slotted(lit-pop-content){
            bottom:100%;
            right:50%;
            transform:translate(0,-10px) scale(0);
            transform-origin: right bottom;
        }
        :host([direction="topleft"]) ::slotted(lit-pop-content)::after{
            content: '';
            position: absolute; 
            top: 100%;
            right: 0%;
            border-top: 10px solid #FFF;
            border-left: 10px solid transparent;
        }
        :host([direction="topleft"]) ::slotted(lit-pop-content[open]),
        :host([direction="topleft"][trigger="hover"]:hover) ::slotted(lit-pop-content),
        :host([direction="topleft"][trigger="focus"]:focus-within) ::slotted(lit-pop-content){
            transform:translate(0,-10px) scale(1);
        }
        
       
        :host([direction="rightbottom"]) ::slotted(lit-pop-content){
            left:100%;
            top:0;
            transform:translate(10px) scale(0);
            transform-origin: left top;
        }
        :host([direction="rightbottom"]) ::slotted(lit-pop-content)::after{
            content: '';
            position: absolute; 
            top: 10%;
            right: 100%;
            border-top: 10px solid #FFF;
            border-left: 10px solid transparent;
        }
        :host([direction="rightbottom"]) ::slotted(lit-pop-content[open]),
        :host([direction="rightbottom"][trigger="hover"]:hover) ::slotted(lit-pop-content),
        :host([direction="rightbottom"][trigger="focus"]:focus-within) ::slotted(lit-pop-content){
            transform:translate(10px) scale(1);
        }
        :host([direction="righttop"]) ::slotted(lit-pop-content){
            left:100%;
            bottom:0;
            transform:translate(10px) scale(0);
            transform-origin: left bottom;
        }
        :host([direction="righttop"]) ::slotted(lit-pop-content[open]),
        :host([direction="righttop"][trigger="hover"]:hover) ::slotted(lit-pop-content),
        :host([direction="righttop"][trigger="focus"]:focus-within) ::slotted(lit-pop-content){
            transform:translate(10px) scale(1);
        }
        :host([direction="righttop"]) ::slotted(lit-pop-content)::after{
            content: '';
            position: absolute; 
            bottom: 10%;
            right: 100%;
            border-bottom: 10px solid #FFF;
            border-left: 10px solid transparent;
        }
        
        :host([direction="bottomright"]) ::slotted(lit-pop-content),
        :host(:not([direction])) ::slotted(lit-pop-content){
            left:0;
            top:100%;
            transform:translate(0,10px) scale(0);
            transform-origin: left top;
        }
        :host([direction="bottomright"]) ::slotted(lit-pop-content)::after,
        :host(:not([direction])) ::slotted(lit-pop-content)::after{
            content: '';
            position: absolute; 
            left: 10%;
            bottom: 100%;
            border-bottom: 10px solid #FFF;
            border-right: 10px solid transparent;
        }
        :host(:not([direction])) ::slotted(lit-pop-content[open]),
        :host(:not([direction])[trigger="hover"]:hover) ::slotted(lit-pop-content),
        :host(:not([direction])[trigger="focus"]:focus-within) ::slotted(lit-pop-content),
        :host([direction="bottomright"]) ::slotted(lit-pop-content[open]),
        :host([direction="bottomright"][trigger="hover"]:hover) ::slotted(lit-pop-content),
        :host([direction="bottomright"][trigger="focus"]:focus-within) ::slotted(lit-pop-content){
            transform:translate(0,10px) scale(1);
        }
        
        :host([direction="bottomleft"]) ::slotted(lit-pop-content){
            right:0;
            top:100%;
            transform:translate(0,10px) scale(0);
            transform-origin: right top;
        }
        :host([direction="bottomleft"]) ::slotted(lit-pop-content)::after,
        :host(:not([direction])) ::slotted(lit-pop-content)::after{
            content: '';
            position: absolute; 
            right: 10%;
            bottom: 100%;
            border-bottom: 10px solid #FFF;
            border-left: 10px solid transparent;
        }
        :host([direction="bottomleft"]) ::slotted(lit-pop-content[open]),
        :host([direction="bottomleft"][trigger="hover"]:hover) ::slotted(lit-pop-content),
        :host([direction="bottomleft"][trigger="focus"]:focus-within) ::slotted(lit-pop-content){
            transform:translate(0,10px) scale(1);
        }
        
        :host ::slotted(lit-pop-content[open]),
        :host([trigger="hover"]:hover) ::slotted(lit-pop-content),
        :host([trigger="focus"]:focus-within) ::slotted(lit-pop-content){
            opacity:1;
            visibility:visible;
        }
        slot{
        }
        </style>
        <slot></slot>
      "
`);
  });

  it('LitPopover13', () => {
    let litPopover = new LitPopover();
    expect(litPopover.connectedCallback()).toBeUndefined();
  });

  it('LitPopover16', () => {
    const onclick = jest.fn();
    let litPopover = (document.body.innerHTML = `
            <lit-popover id='popover'></lit-popover>
        `);
    const popover = document.getElementById('popover');
    expect(onclick).not.toBeCalled();
    popover!.onclick = onclick;
    popover!.click();
    expect(onclick).toBeCalled();
    expect(onclick).toHaveBeenCalledTimes(1);
  });
});
