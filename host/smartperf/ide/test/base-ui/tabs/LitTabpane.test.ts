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
import { LitTabpane } from '../../../dist/base-ui/tabs/lit-tabpane.js';

describe('LitTabPane Test', () => {
  let litTabPane = new LitTabpane();

  litTabPane.tab = 'tab';
  litTabPane.disabled = null || false;
  litTabPane.disabled = !null || !false;
  litTabPane.hidden = 'hidden';
  litTabPane.closeable = false;
  litTabPane.key = 'key';

  it('LitTabPaneTest1', () => {
    expect(litTabPane.attributeChangedCallback('disabled', 'disabled', '')).toBeUndefined();
  });

  it('LitTabPaneTest2', () => {
    expect(litTabPane.tab).toBe('tab');
  });

  it('LitTabPaneTest3', () => {
    expect(litTabPane.icon).toBeNull();
  });

  it('LitTabPaneTest4', () => {
    expect(litTabPane.disabled).toBeTruthy();
  });

  it('LitTabPaneTest5', () => {
    expect(litTabPane.hidden).toBeTruthy();
  });

  it('LitTabPaneTest6', () => {
    litTabPane.closeable = 'closeable';
    expect(litTabPane.closeable).toBeTruthy();
  });

  it('LitTabPaneTest7', () => {
    expect(litTabPane.key).toBe('key');
  });

  it('LitTabPaneTest9 ', function () {
    expect(litTabPane.connectedCallback()).toBeUndefined();
  });

  it('LitTabPaneTest10 ', function () {
    expect(litTabPane.disconnectedCallback()).toBeUndefined();
  });

  it('LitTabPaneTest11 ', function () {
    expect(litTabPane.adoptedCallback()).toBeUndefined();
  });
  it('LitTabPaneTest8', () => {
    expect(litTabPane.initHtml()).toMatchInlineSnapshot(`
"
        <style>
        :host(){
            scroll-behavior: smooth;
            -webkit-overflow-scrolling: touch;
            overflow: auto;
            width: 100%;
        }
        </style>
        <slot></slot>
        "
`);
  });
});
