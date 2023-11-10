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
import {
  ProcedureLogicWorkerJsCpuProfiler,
  JsCpuProfilerSample,
} from '../../../../dist/trace/database/logic-worker/ProcedureLogicWorkerJsCpuProfiler.js';

describe('ProcedureLogicWorkerJsCpuProfiler Test', () => {
  it('ProcedureLogicWorkerJsCpuProfiler01', function () {
    let procedureLogicWorkerJsCpuProfiler = new ProcedureLogicWorkerJsCpuProfiler();
    expect(procedureLogicWorkerJsCpuProfiler).not.toBeUndefined();
  });

  it('ProcedureLogicWorkerJsCpuProfiler02', function () {
    let procedureLogicWorkerJsCpuProfiler = new ProcedureLogicWorkerJsCpuProfiler();
    let data = {
      id: '3b21cea8-3554-4aa6-8e4d-7d30672dedca',
      type: 'jsCpuProfiler-init',
      params: {
        list: new ArrayBuffer(20),
      },
    };
    window.postMessage = jest.fn(() => true);
    expect(procedureLogicWorkerJsCpuProfiler.handle(data)).toBeUndefined();
  });

  it('ProcedureLogicWorkerJsCpuProfiler03', function () {
    let procedureLogicWorkerJsCpuProfiler = new ProcedureLogicWorkerJsCpuProfiler();
    let data = {
      type: 'jsCpuProfiler-call-chain',
      params: {
        list: [],
      },
      id: '3b21cea8-3554-4aa6-8e4d-7d30672dedca',
    };
    window.postMessage = jest.fn(() => true);
    procedureLogicWorkerJsCpuProfiler.dataCache = [];
    expect(procedureLogicWorkerJsCpuProfiler.handle(data)).toBeUndefined();
  });

  it('ProcedureLogicWorkerJsCpuProfiler04', function () {
    let procedureLogicWorkerJsCpuProfiler = new ProcedureLogicWorkerJsCpuProfiler();
    let data = {
      type: 'jsCpuProfiler-samples',
      params: {
        list: [],
      },
      id: '3b21cea8-3554-4aa6-8e4d-7d30672dedca',
    };
    window.postMessage = jest.fn(() => true);
    expect(procedureLogicWorkerJsCpuProfiler.handle(data)).toBeUndefined();
  });

  it('ProcedureLogicWorkerJsCpuProfiler05', function () {
    let procedureLogicWorkerJsCpuProfiler = new ProcedureLogicWorkerJsCpuProfiler();
    let data = {
      type: 'jsCpuProfiler-call-tree',
      params: [
        {
          startTime: 0,
          endTime: 0,
          children: [],
          samplesIds: [],
          isSelect: false,
        },
      ],
      id: '3b21cea8-3554-4aa6-8e4d-7d30672dedca',
    };
    window.postMessage = jest.fn(() => true);
    expect(procedureLogicWorkerJsCpuProfiler.handle(data)).toBeUndefined();
  });

  it('ProcedureLogicWorkerJsCpuProfiler06', function () {
    let procedureLogicWorkerJsCpuProfiler = new ProcedureLogicWorkerJsCpuProfiler();
    let data = {
      type: 'jsCpuProfiler-bottom-up',
      params: [
        {
          startTime: 0,
          endTime: 0,
          children: [],
          samplesIds: [],
          isSelect: false,
        },
      ],
      id: '3b21cea8-3554-4aa6-8e4d-7d30672dedca',
    };
    window.postMessage = jest.fn(() => true);
    expect(procedureLogicWorkerJsCpuProfiler.handle(data)).toBeUndefined();
  });

  it('ProcedureLogicWorkerJsCpuProfiler07', function () {
    let procedureLogicWorkerJsCpuProfiler = new ProcedureLogicWorkerJsCpuProfiler();
    let data = {
      type: 'jsCpuProfiler-statistics',
      params: {
        leftNs: 0,
        rightNs: 10000000000,
        data: [],
      },
      id: '3b21cea8-3554-4aa6-8e4d-7d30672dedca',
    };
    window.postMessage = jest.fn(() => true);
    expect(procedureLogicWorkerJsCpuProfiler.handle(data)).toBeUndefined();
  });
  it('ProcedureLogicWorkerJsCpuProfiler09', function () {
    let procedureLogicWorkerJsCpuProfiler = new ProcedureLogicWorkerJsCpuProfiler();
    let chartFrame = {
      parent: true,
    };
    expect(procedureLogicWorkerJsCpuProfiler.copyParent([], chartFrame)).toBeUndefined();
  });
  it('ProcedureLogicWorkerJsCpuProfiler10', function () {
    let procedureLogicWorkerJsCpuProfiler = new ProcedureLogicWorkerJsCpuProfiler();
    expect(procedureLogicWorkerJsCpuProfiler.calStatistic([], 11, 43)).toBeTruthy();
  });
  it('ProcedureLogicWorkerJsCpuProfiler11', function () {
    let procedureLogicWorkerJsCpuProfiler = new ProcedureLogicWorkerJsCpuProfiler();
    procedureLogicWorkerJsCpuProfiler.dataCache = jest.fn(() => true);
    procedureLogicWorkerJsCpuProfiler.dataCache.clearAll = jest.fn(() => true);
    expect(procedureLogicWorkerJsCpuProfiler.clearAll()).toBeUndefined();
  });
  it('ProcedureLogicWorkerJsCpuProfiler12', function () {
    let procedureLogicWorkerJsCpuProfiler = new ProcedureLogicWorkerJsCpuProfiler();
    expect(procedureLogicWorkerJsCpuProfiler.getFullCallChainOfNode([])).toBeTruthy();
  });
  it('ProcedureLogicWorkerJsCpuProfiler13', function () {
    let procedureLogicWorkerJsCpuProfiler = new ProcedureLogicWorkerJsCpuProfiler();
    expect(procedureLogicWorkerJsCpuProfiler.symbolToChartFrame([], [])).toBeTruthy();
  });
  it('ProcedureLogicWorkerJsCpuProfiler14', function () {
    let procedureLogicWorkerJsCpuProfiler = new ProcedureLogicWorkerJsCpuProfiler();
    expect(procedureLogicWorkerJsCpuProfiler.chartFrameToTabStruct([], [])).toBeTruthy();
  });
  it('ProcedureLogicWorkerJsCpuProfiler15', function () {
    let procedureLogicWorkerJsCpuProfiler = new ProcedureLogicWorkerJsCpuProfiler();
    expect(procedureLogicWorkerJsCpuProfiler.isSymbolEqual([], [])).toBeTruthy();
  });
  it('ProcedureLogicWorkerJsCpuProfiler16', function () {
    expect(JsCpuProfilerSample).toBeUndefined();
  });
});
