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
  ProcedureLogicWorkerSPT,
  ThreadState,
  ThreadProcess,
  SPT,
} from '../../../../dist/trace/database/logic-worker/ProcedureLogicWorkerSPT.js';

describe('ProcedureLogicWorkerSPT Test', () => {
  it('ProcedureLogicWorkerSPTTest01', function () {
    let procedureLogicWorkerSPT = new ProcedureLogicWorkerSPT();
    expect(procedureLogicWorkerSPT).not.toBeUndefined();
  });

  it('ProcedureLogicWorkerSPTTest03', function () {
    let threadState = new ThreadState();
    expect(threadState).not.toBeUndefined();
  });

  it('ProcedureLogicWorkerSPTTest04', function () {
    let threadProcess = new ThreadProcess();
    expect(threadProcess).not.toBeUndefined();
  });

  it('ProcedureLogicWorkerSPTTest05', function () {
    let sPT = new SPT();
    expect(sPT).not.toBeUndefined();
  });

  it('ProcedureLogicWorkerSPTTest06', function () {
    let procedureLogicWorkerSPT = new ProcedureLogicWorkerSPT();
    let data = {
      id: 1,
      params: [
        {
          list: '',
        },
      ],
      type: 'spt-init',
    };
    procedureLogicWorkerSPT.getThreadState = jest.fn(() => true);
    expect(procedureLogicWorkerSPT.handle(data)).toBeUndefined();
  });

  it('ProcedureLogicWorkerSPTTest07', function () {
    let procedureLogicWorkerSPT = new ProcedureLogicWorkerSPT();
    let data = {
      id: 1,
      params: [
        {
          list: '',
        },
      ],
      type: 'spt-getThreadStateData',
    };
    procedureLogicWorkerSPT.getThreadProcessData = jest.fn(() => true);
    expect(procedureLogicWorkerSPT.handle(data)).toBeUndefined();
  });

  it('ProcedureLogicWorkerSPTTest08', function () {
    let procedureLogicWorkerSPT = new ProcedureLogicWorkerSPT();
    let data = {
      id: 1,
      params: [
        {
          list: '',
        },
      ],
      type: 'spt-getThreadProcessData',
    };
    procedureLogicWorkerSPT.initProcessThreadStateData = jest.fn(() => true);
    expect(procedureLogicWorkerSPT.handle(data)).toBeUndefined();
  });
  it('ProcedureLogicWorkerSPTTest09', function () {
    let procedureLogicWorkerSPT = new ProcedureLogicWorkerSPT();
    window.postMessage = jest.fn(() => true);
    expect(procedureLogicWorkerSPT.queryData()).toBeUndefined();
  });
  it('ProcedureLogicWorkerSPTTest10', function () {
    let procedureLogicWorkerSPT = new ProcedureLogicWorkerSPT();
    window.postMessage = jest.fn(() => true);
    expect(procedureLogicWorkerSPT.getThreadState()).toBeUndefined();
  });
  it('ProcedureLogicWorkerSPTTest11', function () {
    let procedureLogicWorkerSPT = new ProcedureLogicWorkerSPT();
    window.postMessage = jest.fn(() => true);
    expect(procedureLogicWorkerSPT.getThreadProcessData()).toBeUndefined();
  });
  it('ProcedureLogicWorkerSPTTest12', function () {
    let procedureLogicWorkerSPT = new ProcedureLogicWorkerSPT();
    window.postMessage = jest.fn(() => true);
    expect(procedureLogicWorkerSPT.getSPT()).toBeUndefined();
  });
  it('ProcedureLogicWorkerSPTTest13', function () {
    let procedureLogicWorkerSPT = new ProcedureLogicWorkerSPT();
    window.postMessage = jest.fn(() => true);
    expect(procedureLogicWorkerSPT.initProcessThreadStateData()).toBeUndefined();
  });
});
