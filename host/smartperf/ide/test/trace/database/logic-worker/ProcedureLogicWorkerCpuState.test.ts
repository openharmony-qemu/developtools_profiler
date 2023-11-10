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
  ProcedureLogicWorkerCpuState,
  CpuState,
} from '../../../../dist/trace/database/logic-worker/ProcedureLogicWorkerCpuState.js';
describe('ProcedureLogicWorkerCpuState Test', () => {
  it('ProcedureLogicWorkerCpuStateTest01', function () {
    let procedureLogicWorkerCpuState = new ProcedureLogicWorkerCpuState();
    expect(procedureLogicWorkerCpuState).not.toBeUndefined();
  });

  it('ProcedureLogicWorkerCpuStateTest02', function () {
    let procedureLogicWorkerCpuState = new ProcedureLogicWorkerCpuState();
    let arr = [
      {
        startTs: 1,
        endTs: 1,
        length: 1,
      },
    ];
    expect(procedureLogicWorkerCpuState.supplementCpuState(arr)).toEqual([
      { dur: 1, endTs: 1, startTs: 0, value: 3 },
      { endTs: 1, length: 1, startTs: 1 },
    ]);
  });

  it('ProcedureLogicWorkerCpuStateTest03', function () {
    let procedureLogicWorkerCpuState = new ProcedureLogicWorkerCpuState();
    let data = {
      type: 'CpuState-getCpuState',
      params: {
        list: true,
      },
    };
    window.postMessage = jest.fn(() => true);
    expect(procedureLogicWorkerCpuState.handle(data)).toBeUndefined();
  });

  it('ProcedureLogicWorkerCpuStateTest04', function () {
    let cpuState = new CpuState();
    cpuState = {
      startTs: 0,
      endTs: 0,
      dur: 0,
      value: 0,
    };
    expect(cpuState).not.toBeUndefined();
  });
  it('ProcedureLogicWorkerCpuStateTest05', function () {
    let procedureLogicWorkerCpuState = new ProcedureLogicWorkerCpuState();
    let data = {
      type: 'CpuState-getCpuState',
      params: {
        list: false,
      },
    };
    window.postMessage = jest.fn(() => true);
    expect(procedureLogicWorkerCpuState.handle(data)).toBeUndefined();
  });
  it('ProcedureLogicWorkerCpuStateTest06', function () {
    let procedureLogicWorkerCpuState = new ProcedureLogicWorkerCpuState();

    window.postMessage = jest.fn(() => true);
    expect(procedureLogicWorkerCpuState.queryData()).toBeUndefined();
  });
  it('ProcedureLogicWorkerCpuStateTest07', function () {
    let procedureLogicWorkerCpuState = new ProcedureLogicWorkerCpuState();

    window.postMessage = jest.fn(() => true);
    expect(procedureLogicWorkerCpuState.getCpuState()).toBeUndefined();
  });
});
