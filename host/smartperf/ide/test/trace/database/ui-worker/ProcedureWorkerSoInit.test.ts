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
import { TraceRow } from '../../../../dist/trace/component/trace/base/TraceRow.js';
// @ts-ignore
import { Rect } from '../../../../dist/trace/component/trace/timer-shaft/Rect.js';
// @ts-ignore
import { soDataFilter, SoRender, SoStruct} from '../../../../dist/trace/database/ui-worker/ProcedureWorkerSoInit.js';

jest.mock('../../../../dist/trace/database/ui-worker/ProcedureWorker.js', () => {
    return {};
});
jest.mock('../../../../dist/trace/component/trace/base/TraceRow.js', () => {
    return {};
});

describe('ProcedureWorkerSoInit Test', () => {
    it('soDataFilterTest', () => {
        const canvas = document.createElement('canvas');
        canvas.width = 1;
        canvas.height = 1;
        const ctx = canvas.getContext('2d');
        let rect = new Rect(0, 10, 10, 10);
        let filter = [
            {
                startTs: 50,
                dur: 1520000,
                soName: 'Snapshot0',
                tid: 0,
                pid: 4243,
                depth: 0,
                itid: 2,
                textMetricsWidth: 50.5810546875,
                process: ''
            },
        ];
        let list = [
            {
                startTs: 50,
                dur: 1520000,
                soName: 'Snapshot0',
                tid: 0,
                pid: 4243,
                depth: 0,
                itid: 2,
                textMetricsWidth: 50.5810546875,
                process: ''
            },
        ];
        soDataFilter(list, filter, 100254, 100254, rect, { height: 40, width: 1407, x: 0, y: 0 },true);
    });

    it('SoStructTest01', () => {
        const data = {
            frame: {
                x: 20,
                y: 20,
                width: 100,
                height: 100,
            },
            startTs: 50,
            dur: 1520000,
            soName: 'Snapshot0',
            tid: 0,
            pid: 4243,
            depth: 0,
            itid: 2,
            textMetricsWidth: 50.5810546875,
            process: ''
        };
        const canvas = document.createElement('canvas');
        canvas.width = 1;
        canvas.height = 1;
        const ctx = canvas.getContext('2d');
        expect(SoStruct.draw(ctx, data)).toBeUndefined();
    });

    it('SoStructTest02', () => {
        const data = {
            frame: {
                x: 20,
                y: 20,
                width: 100,
                height: 100,
            },
            startTs: 50,
            dur: 1520000,
            soName: 'Snapshot0',
            tid: 0,
            pid: 4243,
            depth: 0,
            itid: 2,
            textMetricsWidth: 50.5810546875,
            process: ''
        };
        let node = {
            frame: {
                x: 20,
                y: 20,
                width: 100,
                height: 100,
            },
            startTs: 50,
            dur: 1520000,
            soName: 'Snapshot0',
            tid: 0,
            pid: 4243,
            depth: 0,
            itid: 2,
            textMetricsWidth: 50.5810546875,
            process: ''
        };
        expect(SoStruct.setSoFrame(node, 2, 0, 1, 2, data)).toBeUndefined();
    });
    it('SoStructTest03', () => {
        expect(SoStruct).not.toBeUndefined();
    });
});
