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
jest.mock('../../../dist/trace/component/trace/base/TraceRow.js', () => {
    return {};
});

describe('LitTreeNode Test', () => {
    let litTreeNode = new LitTreeNode();
    litTreeNode.data = [];
    litTreeNode.checkable = 'true';
    litTreeNode.multiple = true;
    litTreeNode.iconName = '';
    litTreeNode.topDepth = true;
    litTreeNode.arrow = true;
    litTreeNode.open = 'true';
    litTreeNode.selected = true;
    litTreeNode.checked = false;

    it('LitTreeNodeTest01', () => {
        expect(litTreeNode.data).toStrictEqual([]);
    });
    it('LitTreeNodeTest02', () => {
        expect(litTreeNode.checkable).toStrictEqual("true");
    });
    it('LitTreeNodeTest03', () => {
        expect(litTreeNode.multiple).toStrictEqual(true);
    });
    it('LitTreeNodeTest04', () => {
        expect(litTreeNode.iconName).toStrictEqual('');
    });
    it('LitTreeNodeTest05', () => {
        expect(litTreeNode.topDepth).toStrictEqual(true);
    });
    it('LitTreeNodeTest06', () => {
        expect(litTreeNode.arrow).toStrictEqual(true);
    });
    it('LitTreeNodeTest07', () => {
        expect(litTreeNode.open).toStrictEqual('true');
    });
    it('LitTreeNodeTest07', () => {
        expect(litTreeNode.selected).toStrictEqual(true);
    });
    it('LitTreeNodeTest08', () => {
        expect(litTreeNode.checked).toStrictEqual(false);
    });
    it('LitTreeNodeTest09', () => {
        expect(litTreeNode.expand()).toBeUndefined();
    });
    it('LitTreeNodeTest10', () => {
        document.body.innerHTML = `<ul id="ul"></ul>`;
        let element = document.querySelector('#ul') as HTMLDivElement;
        expect(litTreeNode.collapseSection(element)).toBeUndefined();
    });
    it('LitTreeNodeTest11', () => {
        document.body.innerHTML = `<ul id="ul"></ul>`;
        let element = document.querySelector('#ul') as HTMLDivElement;
        expect(litTreeNode.expandSection(element)).toBeUndefined();
    });
    it('LitTreeNodeTest12', () => {
        expect(litTreeNode.attributeChangedCallback('title',[],'av')).toBeUndefined();
    });
    it('LitTreeNodeTest13', () => {
        expect(litTreeNode.drawLine('top')).toBeUndefined();
    });
    it('LitTreeNodeTest14', () => {
        expect(litTreeNode.drawLine('bottom')).toBeUndefined();
    });
    it('LitTreeNodeTest15', () => {
        expect(litTreeNode.drawLine('top-right')).toBeUndefined();
    });
    it('LitTreeNodeTest16', () => {
        expect(litTreeNode.drawLine('bottom-right')).toBeUndefined();
    });
    it('LitTreeNodeTest17', () => {
        expect(litTreeNode.collapse()).toBeUndefined();
    });
});