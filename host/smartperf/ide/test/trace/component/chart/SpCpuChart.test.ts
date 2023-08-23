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
import { SpChartManager } from '../../../../dist/trace/component/chart/SpChartManager.js';
// @ts-ignore
import { SpCpuChart } from '../../../../dist/trace/component/chart/SpCpuChart.js';
// @ts-ignore
import { HeapNode } from '../../../../dist/js-heap/model/DatabaseStruct.js';

const sqlit = require('../../../../dist/trace/database/SqlLite.js');
jest.mock('../../../../dist/trace/database/SqlLite.js');

window.ResizeObserver =
    window.ResizeObserver ||
    jest.fn().mockImplementation(() => ({
        disconnect: jest.fn(),
        observe: jest.fn(),
        unobserve: jest.fn(),
    }));

jest.mock('../../../../dist/js-heap/utils/Utils.js', () => {
    return {
        HeapNodeToConstructorItem: (node: HeapNode) => {},
    };
});
describe('SpCpuChart Test', () => {
    let MockqueryCpuMax = sqlit.queryCpuMax;
    MockqueryCpuMax.mockResolvedValue([{ cpu: 1 }]);

    let mockCpuSlice = sqlit.queryCpuSchedSlice;
    mockCpuSlice.mockResolvedValue([]);
    let ss = new SpChartManager();
    let trace = new SpCpuChart(ss);
    it('SpMpsChart01', async function () {
        await trace.init();
        expect(trace).toBeDefined();
    });
});
