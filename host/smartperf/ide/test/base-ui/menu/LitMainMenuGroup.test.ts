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
import { LitMainMenuGroup } from '../../../dist/base-ui/menu/LitMainMenuGroup.js';

describe('litMainMenuGroup Test', () => {
  it('litMainMenuGroup01', () => {
    let litMainMenuGroup = new LitMainMenuGroup();
    expect(litMainMenuGroup).not.toBeUndefined();
    expect(litMainMenuGroup).not.toBeNull();
  });

  it('litMainMenuGroup02', () => {
    let litMainMenuGroup = new LitMainMenuGroup();
    expect(litMainMenuGroup.collapsed).toBeFalsy();
  });

  it('litMainMenuGroup03', () => {
    let litMainMenuGroup = new LitMainMenuGroup();
    litMainMenuGroup.collapsed = true;
    expect(litMainMenuGroup.collapsed).toBeTruthy();
  });

  it('litMainMenuGroup04', () => {
    let litMainMenuGroup = new LitMainMenuGroup();
    litMainMenuGroup.collapsed = false;
    expect(litMainMenuGroup.collapsed).toBeFalsy();
  });

  it('litMainMenuGroup06', () => {
    let litMainMenuGroup = new LitMainMenuGroup();
    expect(litMainMenuGroup.radius).toBeFalsy();
  });

  it('litMainMenuGroup07', () => {
    let litMainMenuGroup = new LitMainMenuGroup();
    expect(litMainMenuGroup.initHtml()).toMatchInlineSnapshot(`
"
        <style>
        :host(:not([collapsed])){ 
            width: 248px;
            display: flex;
            background-color: var(--dark-background);
            cursor: pointer;
            flex-direction: column;
        }
        :host{
            user-select: none;
            transition: background-color .3s;
        }
        :host(:not([collapsed])) ::slotted(lit-main-menu-item){
            display: flex;
        }
        :host(:not([collapsed])) .group-describe{
            height: 0;
            visibility: hidden;
        }
        :host([collapsed]):hover){
            background-color: #FFFFFF;
        }
        :host([collapsed]){
            width: 248px;
            display: flex;
            flex-direction: column;
            cursor: pointer;
            background-color: var(--dark-background);
        }
        :host([collapsed]) .group-describe{
            height: auto;
            visibility: visible;
        }
        :host([radius]) {
            border-radius: 16px 0px 0px 16px ;
        }
        :host([collapsed]) ::slotted(lit-main-menu-item){
            display: none;
        }
        .group-name{
            font-size: 14px;
            font-family: Helvetica;
            color: black;
            padding: 20px 24px 0px 24px;
            line-height: 16px;
            font-weight: 400;
            text-align: left;
        }
        .group-describe{
            color: black;
            font-size: 0.6rem;
            padding: 4px 24px 20px 24px;
        }
       
        </style>
       <div id="group">
         <div class="group-name"></div>
         <div class="group-describe"></div>
       </div>
        <slot></slot>
        "
`);
  });

  it('litMainMenuGroup04', () => {
    let litMainMenuGroup = new LitMainMenuGroup();
    litMainMenuGroup.nocollapsed = true;
    expect(litMainMenuGroup.nocollapsed).toBeTruthy();
  });

  it('litMainMenuGroup04', () => {
    let litMainMenuGroup = new LitMainMenuGroup();
    litMainMenuGroup.nocollapsed = false;
    expect(litMainMenuGroup.nocollapsed).toBeFalsy();
  });

  it('litMainMenuGroup05', () => {
    let litMainMenuGroup = new LitMainMenuGroup();
    expect(litMainMenuGroup.collapsed).toBeFalsy();
  });
});
