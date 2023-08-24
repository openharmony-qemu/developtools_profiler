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
import { LitCheckBoxWithText } from '../../../dist/base-ui/checkbox/LitCheckBoxWithText.js';

describe('checkBoxWithText Test', () => {
  it('checkBoxWithTextTest01', function () {
    let litCheckBoxWithText = new LitCheckBoxWithText();
    expect(litCheckBoxWithText).not.toBeUndefined();
    expect(litCheckBoxWithText).not.toBeNull();
  });

  it('checkBoxWithTextTest02', function () {
    let litCheckBoxWithText = new LitCheckBoxWithText();
    expect(litCheckBoxWithText.checked).toBeFalsy();
  });

  it('checkBoxWithTextTest03', function () {
    let litCheckBoxWithText = new LitCheckBoxWithText();
    litCheckBoxWithText.checked = true;
    expect(litCheckBoxWithText.checked).toBeTruthy();
  });

  it('checkBoxWithTextTest03', function () {
    let litCheckBoxWithText = new LitCheckBoxWithText();
    litCheckBoxWithText.checked = false;
    expect(litCheckBoxWithText.checked).toBeFalsy();
  });

  it('checkBoxWithTextTest04', function () {
    let litCheckBoxWithText = new LitCheckBoxWithText();
    expect(litCheckBoxWithText.text).toEqual('');
  });

  it('checkBoxWithTextTest05', function () {
    let litCheckBoxWithText = new LitCheckBoxWithText();
    litCheckBoxWithText.text = 'test';
    expect(litCheckBoxWithText.text).toEqual('test');
  });

  it('checkBoxWithTextTest05', function () {
    let litCheckBoxWithText = new LitCheckBoxWithText();
    expect(litCheckBoxWithText.lowerLimit).toEqual('0');
  });

  it('checkBoxWithTextTest05', function () {
    let litCheckBoxWithText = new LitCheckBoxWithText();
    litCheckBoxWithText.lowerLimit = '111';
    expect(litCheckBoxWithText.lowerLimit).toEqual('111');
  });

  it('checkBoxWithTextTest05', function () {
    let litCheckBoxWithText = new LitCheckBoxWithText();
    litCheckBoxWithText.upLimit = '111';
    expect(litCheckBoxWithText.upLimit).toEqual('111');
  });

  it('checkBoxWithTextTest05', function () {
    let litCheckBoxWithText = new LitCheckBoxWithText();
    expect(litCheckBoxWithText.initHtml()).toMatchInlineSnapshot(`
"
        <style>
        :host{
         display: grid;
         grid-template-columns: 1fr min-content min-content;
         grid-column-gap: 10px;
         text-align: center;
         height: 16px;
        }
        .input-style {
          width: 48px;
          height: 16px;
          border: 1px solid #B3B3B3;
          text-align: center;
        }

        </style>
        <check-des-box id ='checkbox' value=""></check-des-box>
        <input class="input-style" id="textLowerLimit" value="0"/>
        <input class="input-style" id="_upLimit" value="∞"/>
        "
`);
  });

  it('checkBoxWithTextTest06', function () {
    let litCheckBoxWithText = new LitCheckBoxWithText();
    expect(litCheckBoxWithText.attributeChangedCallback('checked')).toBeUndefined();
  });

  it('checkBoxWithTextTest07', function () {
    let litCheckBoxWithText = new LitCheckBoxWithText();
    expect(litCheckBoxWithText.attributeChangedCallback('text')).toBeUndefined();
  });

  it('checkBoxWithTextTest08', function () {
    let litCheckBoxWithText = new LitCheckBoxWithText();
    expect(litCheckBoxWithText.attributeChangedCallback('lowerLimit')).toBeUndefined();
  });

  it('checkBoxWithTextTest09', function () {
    let litCheckBoxWithText = new LitCheckBoxWithText();
    expect(litCheckBoxWithText.attributeChangedCallback('upLimit')).toBeUndefined();
  });
});
