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
import { LitCheckGroup } from '../../../dist/base-ui/checkbox/LitCheckGroup.js';

describe('LitCheckGroup Test', () => {
  it('LitCheckGroupTest01', function () {
    let litCheckGroup = new LitCheckGroup();
    expect(litCheckGroup).not.toBeUndefined();
    expect(litCheckGroup).not.toBeNull();
  });

  it('LitCheckGroupTest02', function () {
    let litCheckGroup = new LitCheckGroup();
    expect(litCheckGroup.direction).toBeNull();
  });

  it('LitCheckGroupTest03', function () {
    let litCheckGroup = new LitCheckGroup();
    expect(litCheckGroup.value).toEqual([]);
  });

  it('LitCheckGroupTest03', function () {
    let litCheckGroup = new LitCheckGroup();
    expect(litCheckGroup.initHtml()).toMatchInlineSnapshot(`
"<style>   
        :host {
            display: -webkit-flex; 
            display: flex;
            flex-direction: column;
        }
        :host(:not([direction])) {
            flex-direction: column;
        }
        :host([direction]) {
            flex-direction: null;
        }
        :host([layout="dispersion"]) {
           gap:10px;
        }
        :host([layout="compact"]) {
            gap:5px;
        }
        </style>
        <slot class="check-group"></slot>"
`);
  });
});
