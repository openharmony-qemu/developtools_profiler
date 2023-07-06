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
import { DrawerCpuTabs } from '../../../../dist/trace/component/schedulingAnalysis/DrawerCpuTabs.js';
import crypto from "crypto";
// @ts-ignore
window.ResizeObserver = window.ResizeObserver || jest.fn().mockImplementation(() => ({
    disconnect: jest.fn(),
    observe: jest.fn(),
    unobserve: jest.fn(),
}));

Object.defineProperty(global.self, 'crypto', {
    value: {
        getRandomValues: (arr: string | any[]) => crypto.randomBytes(arr.length),
    },
});

describe('DrawerCpuTabs Test', () => {
    it('DrawerCpuTabsTest01', () => {
        let drawerCpuTabs = new DrawerCpuTabs();
        expect(drawerCpuTabs.init(1,'1')).not.toBe(0);
    });
    it('DrawerCpuTabsTest02', () => {
        let drawerCpuTabs = new DrawerCpuTabs();
        drawerCpuTabs.init(1,'2');
        expect(drawerCpuTabs.cpuNumber).toEqual(1);
    });
    it('DrawerCpuTabsTest03', () => {
        let drawerCpuTabs = new DrawerCpuTabs();
        drawerCpuTabs.init(1,'3');
        expect(drawerCpuTabs.cpuNumber).toEqual(1);
    });
    it('DrawerCpuTabsTest04', () => {
        let drawerCpuTabs = new DrawerCpuTabs();
        expect(drawerCpuTabs.clearData()).toBeUndefined()
    });
})