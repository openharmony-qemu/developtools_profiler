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
//@ts-ignore
import { LitDrawer } from '../../../dist/base-ui/drawer/LitDrawer.js';
// @ts-ignore
window.ResizeObserver = window.ResizeObserver || jest.fn().mockImplementation(() => ({
    disconnect: jest.fn(),
    observe: jest.fn(),
    unobserve: jest.fn(),
}));

describe('LitDrawer Test', () => {
    it('LitDrawerTest01', () => {
        let litDrawer = new LitDrawer();
        expect(litDrawer).not.toBeUndefined();
    });
    it('LitDrawerTest02', () => {
        let litDrawer = new LitDrawer();
        expect(litDrawer.contentWidth).toBe('400px');
    });
    it('LitDrawerTest03', () => {
        let litDrawer = new LitDrawer();
        expect(litDrawer.contentPadding).toBe('20px');
    });
    it('LitDrawerTest04', () => {
        let litDrawer = new LitDrawer();
        expect(litDrawer.placement).toBe(null);
    });
    it('LitDrawerTest05', () => {
        let litDrawer = new LitDrawer();
        expect(litDrawer.title).toBe('');
    });
    it('LitDrawerTest06', () => {
        let litDrawer = new LitDrawer();
        expect(litDrawer.visible).toBe(false);
    });
    it('LitDrawerTest07', () => {
        let litDrawer = new LitDrawer();
        litDrawer.contentWidth = 'content-width'
        expect(litDrawer.contentWidth).toBe('content-width');
    });
    it('LitDrawerTest08', () => {
        let litDrawer = new LitDrawer();
        litDrawer.contentPadding = 'content-padding';
        expect(litDrawer.contentPadding).toBe('content-padding');
    });
    it('LitDrawerTest09', () => {
        let litDrawer = new LitDrawer();
        litDrawer.placement = 'placement'
        expect(litDrawer.placement).toBe('placement');
    });
    it('LitDrawerTest10', () => {
        let litDrawer = new LitDrawer();
        litDrawer.title = 'title'
        expect(litDrawer.title).toBe('title');
    });
    it('LitDrawerTest11', () => {
        let litDrawer = new LitDrawer();
        litDrawer.visible = true;
        expect(litDrawer.visible).toBe(true);
    });
    it('LitDrawerTest12', () => {
        let litDrawer = new LitDrawer();
        expect(litDrawer.mask).toBe(false);
    });
    it('LitDrawerTest13', () => {
        let litDrawer = new LitDrawer();
        litDrawer.mask = true;
        expect(litDrawer.mask).toBe(true);
    });
    it('LitDrawerTest14', () => {
        let litDrawer = new LitDrawer();
        expect(litDrawer.maskCloseable).toBe(false);
    });
    it('LitDrawerTest15', () => {
        let litDrawer = new LitDrawer();
        litDrawer.maskCloseable = true;
        expect(litDrawer.maskCloseable).toBe(true);
    });
    it('LitDrawerTest16', () => {
        let litDrawer = new LitDrawer();
        expect(litDrawer.closeable).toBe(false);
    });
    it('LitDrawerTest17', () => {
        let litDrawer = new LitDrawer();
        litDrawer.closeable = true;
        expect(litDrawer.closeable).toBe(true);
    });
    it('LitDrawerTest18', () => {
        let litDrawer = new LitDrawer();
        expect(litDrawer.adoptedCallback()).toBeUndefined();
    });
})