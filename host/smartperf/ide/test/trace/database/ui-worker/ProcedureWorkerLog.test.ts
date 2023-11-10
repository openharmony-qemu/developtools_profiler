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

jest.mock('../../../../dist/trace/database/ui-worker/ProcedureWorker.js', () => {
  return {};
});
// @ts-ignore
import { LogStruct, LogRender } from '../../../../dist/trace/database/ui-worker/ProcedureWorkerLog.js';

describe('ProcedureWorkerLog Test', () => {
  let canvas = document.createElement('canvas');
  canvas.width = 12;
  canvas.height = 12;
  const ctx = canvas.getContext('2d');
  it('ProcedureWorkerLog01', () => {
    let data = {
      id: 5230,
      startTs: 27351020209,
      level: 'E',
      depth: 3,
      tag: 'C01510/BinderInvoker1',
      context: '124: SendRequest: handle=0 result = 2',
      time: 15020293020884055,
      pid: 577,
      tid: 967,
      processName: 'distributeddata',
      dur: 1,
      frame: {
        x: 1385,
        y: 22,
        width: 1,
        height: 7,
      },
    };
    expect(LogStruct.draw(ctx!, data)).toBeUndefined();
  });

  it('ProcedureWorkerLog02', () => {
    let data = {
      id: 36,
      startTs: 76402676,
      level: 'W',
      depth: 2,
      tag: 'C01300/AbilityManagerService2',
      context: '[ability_manager_service.cpp(UpdateCallerInfo:6178)]UpdateCallerInfo.',
      time: 15020292748137880,
      pid: 559,
      tid: 559,
      processName: 'foundation',
      dur: 1,
      frame: {
        x: 3,
        y: 16,
        width: 1,
        height: 7,
      },
    };
    expect(LogStruct.draw(ctx!, data)).toBeUndefined();
  });

  it('ProcedureWorkerLog03', () => {
    let data = {
      id: 3,
      startTs: 1,
      level: 'I',
      depth: 1,
      tag: 'C02d0c/Hiprofiler1',
      context: 'ParseTimeExtend: update ts with 0 to 337274',
      time: 15020292747373852,
      pid: 1119,
      tid: 1172,
      processName: 'hiprofiler_plug',
      dur: 1,
      frame: {
        x: 0,
        y: 8,
        width: 1,
        height: 7,
      },
    };
    expect(LogStruct.draw(ctx!, data)).toBeUndefined();
  });

  it('ProcedureWorkerLog04', () => {
    let canvas = document.createElement('canvas') as HTMLCanvasElement;
    let context = canvas.getContext('2d');
    let data = {
      context: context!,
      useCache: true,
      type: 'logs',
      traceRange: [],
    };
    TraceRow.range = jest.fn(() => true);
    TraceRow.range!.startNS = jest.fn(() => 0);
    TraceRow.range!.endNS = jest.fn(() => 27763331331);
    TraceRow.range!.totalNS = jest.fn(() => 27763331331);
    let logRender = new LogRender();
    expect(logRender.renderMainThread(data, new TraceRow())).toBeUndefined();
  });
});
