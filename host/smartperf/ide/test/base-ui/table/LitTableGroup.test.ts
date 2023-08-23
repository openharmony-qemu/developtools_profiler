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
import { LitTableGroup } from '../../../dist/base-ui/table/lit-table-group.js';

describe('LitTableGroup Test', () => {
  let litTableGroup = new LitTableGroup();
  litTableGroup.title = 'title';

  it('LitTableGroupTest01', () => {
    expect(litTableGroup.adoptedCallback()).toBeUndefined();
  });

  it('LitTableGroupTest02', () => {
    expect(litTableGroup.title).toBe('title');
  });

  it('LitTableGroupTest03', () => {
    expect(litTableGroup.shadowRoot.innerHTML).toMatchInlineSnapshot(`
"
        <style>
        :host{ }
        </style>
        <slot id="sl"></slot>
        "
`);
  });
});
