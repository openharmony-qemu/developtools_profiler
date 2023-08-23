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
import { TabCpuAnalysis } from '../../../../dist/trace/component/schedulingAnalysis/TabCpuAnalysis.js';
//@ts-ignore
import {SpSchedulingAnalysis} from "../../../../dist/trace/component/schedulingAnalysis/SpSchedulingAnalysis.js";
// @ts-ignore
window.ResizeObserver = window.ResizeObserver || jest.fn().mockImplementation(() => ({
    disconnect: jest.fn(),
    observe: jest.fn(),
    unobserve: jest.fn(),
}));

describe('TabCpuAnalysis Test', () => {
    it('TabCpuAnalysisTest01', () => {
        let tabCpuAnalysis = new TabCpuAnalysis();
        expect(tabCpuAnalysis).not.toBeUndefined();
    });
    it('TabCpuAnalysisTest02', () => {
        let tabCpuAnalysis = new TabCpuAnalysis();
        tabCpuAnalysis.queryLogicWorker = jest.fn();
        SpSchedulingAnalysis.cpuCount = 3
        expect(tabCpuAnalysis.init()).toBeUndefined();
    });
    it('TabCpuAnalysisTest04', () => {
        let tabCpuAnalysis = new TabCpuAnalysis();
        tabCpuAnalysis.queryLogicWorker = jest.fn();
        expect(tabCpuAnalysis.queryLogicWorker('','',{})).toBeUndefined();
    });
})