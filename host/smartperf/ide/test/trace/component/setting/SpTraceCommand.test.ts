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
import { SpTraceCommand } from '../../../../dist/trace/component/setting/SpTraceCommand.js';

describe('SPTraceCommand Test', () => {
  beforeAll(() => {
    document.body.innerHTML = `
            <trace-command id = "command"><trace-command>
        `;
  });
  it('new SPTraceCommand', function () {
    expect(new SpTraceCommand()).not.toBeNull();
  });

  it(' SpAllocations get Default attrValue', function () {
    let spEle = document.querySelector('#command') as SpTraceCommand;
    expect(spEle.hdcCommon).toEqual('');
  });

  it(' SpAllocations set  attrValue', function () {
    let spEle = document.querySelector('#command') as SpTraceCommand;
    spEle.hdcCommon = 'test';
    expect(spEle.hdcCommon).toEqual('test');
  });

  it(' SpTraceCommand test', function () {
    let spEle = document.querySelector('#command') as SpTraceCommand;
    expect(spEle.initHtml()).toMatchInlineSnapshot(`
"
        <style>
        :host{
            width: 100%;
            position: relative;
            background: var(--dark-background3,#FFFFFF);
            border-radius: 0px 16px 16px 0px;
        }

        #code-text{
            -webkit-appearance:none;
            opacity: 0.6;
            font-family: Helvetica;
            color: var(--dark-color,#000000);
            padding: 20px 56px 5px 56px;
            font-size:1em;
            margin-left: 10px;
            line-height: 20px;
            font-weight: 400;
            border: none;
            outline:none;
            resize:none;
            /*overflow:auto;*/
            z-index: 2;
            min-height: 500px;
            background: var(--dark-background3,#FFFFFF);
        }

        #copy-image{
            display: table-cell;
            white-space: nowrap;
            outline:0;
            float:right;
            z-index: 66;
            position: relative;
            top: 56px;
            right: 40px;
            cursor:pointer;
        }

        #copy-button{
            -webkit-appearance:none;
            outline:0;
            border: 0;
            background: var(--dark-background3,#FFFFFF);
            justify-content: end;
            z-index: 55;
            border-radius: 0px 16px 0px 0px;
        }

        #text-cmd{
            /*overflow-y:auto;*/
            display: grid;
            justify-content: stretch;
            align-content:  stretch;
            font-size:16px;
            background: var(--dark-background3,#FFFFFF);
            border-radius: 0px 16px 0px 0px;

        }

        /*定义滚动条高宽及背景 高宽分别对应横竖滚动条的尺寸*/
        ::-webkit-scrollbar
        {
          width: 6px;
          height: 10px;
          background-color: var(--dark-background3,#FFFFFF);
        }

        /*定义滚动条轨道 内阴影+圆角*/
        ::-webkit-scrollbar-track
        {
          background-color: var(--dark-background3,#FFFFFF);
        }

        /*定义滑块 内阴影+圆角*/
        ::-webkit-scrollbar-thumb
        {
          border-radius: 6px;
          background-color: var(--dark-background7,#e7c9c9);
        }
        </style>
        <div id="text-cmd">
            <button id="copy-button">
                <img id="copy-image" src="img/copy.png">
            </button>
            <textarea id="code-text" readonly></textarea>
        </div>
        "
`);
  });
  it(' SpTraceCommandtest01', function () {
    let spEle = document.querySelector('#command') as SpTraceCommand;
    spEle.show = false;
    expect(spEle.show).toBeFalsy();
  });
  it(' SpTraceCommandtest02', function () {
    let spEle = document.querySelector('#command') as SpTraceCommand;
    spEle.show = true;
    expect(spEle.show).toBeTruthy();
  });
  it(' SpTraceCommandtest03', function () {
    let spEle = document.querySelector('#command') as SpTraceCommand;
    expect(spEle.disconnectedCallback()).toBeUndefined();
  });
});
