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
import { CpuSetting, CheckCpuSetting } from '../../../../dist/trace/component/schedulingAnalysis/CheckCpuSetting.js';
import '../../../../dist/trace/component/schedulingAnalysis/CheckCpuSetting.js';
//@ts-ignore
import {SpSchedulingAnalysis} from "../../../../dist/trace/component/schedulingAnalysis/SpSchedulingAnalysis.js";

// @ts-ignore
window.ResizeObserver = window.ResizeObserver || jest.fn().mockImplementation(() => ({
    disconnect: jest.fn(),
    observe: jest.fn(),
    unobserve: jest.fn(),
}));

describe('CheckCpuSetting Test', () => {
    it('CheckCpuSettingTest01', () => {
        let cpuSetting = new CpuSetting();
        expect(cpuSetting).not.toBeUndefined();
    });
    it('CheckCpuSettingTest02', () => {
        let checkCpuSetting = new CheckCpuSetting();
        expect(checkCpuSetting.init()).toBeUndefined();
    });
    it('CheckCpuSettingTest03', () => {
        let checkCpuSetting = new CheckCpuSetting();
        expect(checkCpuSetting.initDefaultSetting()).toBeUndefined();
    });
    it('CheckCpuSettingTest04', () => {
        let checkCpuSetting = new CheckCpuSetting();
        let cpuSetting = {
            cpu: 1,
            big: true,
            middle: true,
            small: true,
        }
        expect(checkCpuSetting.createTableLine(cpuSetting)).toBeUndefined();
    });
    it('CheckCpuSettingTest05', () => {
        let checkCpuSetting = new CheckCpuSetting();
        expect(checkCpuSetting.createHeaderDiv()).toBeUndefined();
    });

    it('CheckCpuSettingTest06', () => {
        CheckCpuSetting.resetCpuSettings();
        expect(CheckCpuSetting.init_setting).toBeFalsy();
    });
})