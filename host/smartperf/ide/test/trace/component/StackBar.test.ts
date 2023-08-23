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
import { StackBar } from '../../../dist/trace/component/StackBar.js';

describe('StackBar Test', () => {
  let stackBar = new StackBar();

  it('StackBarTest01', function () {
    expect(stackBar.initHtml()).not.toBe('');
  });

  it('StackBarTest02', function () {
    expect(stackBar.initElements()).toBeUndefined();
  });

  it('StackBarTest03', function () {
    let stateWidth = stackBar.getStateWidth('state');
    let hasWidth = stateWidth > 0;
    expect(hasWidth).toBeTruthy();
  });

  it('StackBarTest04', function () {
    let htmlDivElement = stackBar.createBarElement(
      {
        state: '',
        color: '',
        value: 0,
      },
      5
    );
    let hasDivEl = htmlDivElement.toLocaleString().length > 5;
    expect(hasDivEl).toBeTruthy();
  });

  it('StackBarTest05', function () {
    expect(stackBar.initHtml()).toMatchInlineSnapshot(`
"
        <style>
            :host([mode='hidden']){
                display: none;
            }
            :host{
                display: block;
                /*background-color: rebeccapurple;*/
            }
            .state-text{
                width: 10%;display: inline-block;overflow: hidden;white-space: nowrap;padding: 5px; margin-right: 2px;font-size: 9pt;
            }
            </style>
            <div style="display: flex;flex-direction: row;width: 100%;" id="container">
            </div>
        "
`);
  });
});
