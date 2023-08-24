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
import { LitTreeNode } from '../../../dist/base-ui/tree/LitTreeNode.js';

describe('LitTreeNode Test', () => {
    let litTabPane = new LitTreeNode();
    litTabPane.data = 'true';
    litTabPane.checkable = 'true';
    litTabPane.multiple = 'multiple';
    litTabPane.iconName = 'iconName';
    litTabPane.topDepth = 'topDepth';
    litTabPane.arrow = 'arrow';
    litTabPane.open = 'open';
    litTabPane.selected = 'selected';
    litTabPane.checked = 'checked';
    it('LitTreeNode01', () => {
        expect(litTabPane.data).toBe('true');
    });
    it('LitTreeNode02', () => {
        expect(litTabPane.checkable).toBe('true');
    });
    it('LitTreeNode03', () => {
        expect(litTabPane.multiple).toBeTruthy();
    });
    it('LitTreeNode04', () => {
        expect(litTabPane.iconName).toBe('iconName');
    });
    it('LitTreeNode05', () => {
        expect(litTabPane.topDepth).toBeTruthy();
    });
    it('LitTreeNode06', () => {
        expect(litTabPane.arrow).toBeTruthy();
    });
    it('LitTreeNode07', () => {
        expect(litTabPane.selected).toBeTruthy();
    });
    it('LitTreeNode08', () => {
        expect(litTabPane.checked).toBeTruthy();
    });
})