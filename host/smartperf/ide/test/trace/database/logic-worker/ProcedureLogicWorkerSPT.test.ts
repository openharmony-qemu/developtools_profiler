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
import { ProcedureLogicWorkerSPT, SPT,} from '../../../../dist/trace/database/logic-worker/ProcedureLogicWorkerSPT.js';

describe('ProcedureLogicWorkerSPT Test', () => {
  it('ProcedureLogicWorkerSPTTest01', function () {
    let procedureLogicWorkerSPT = new ProcedureLogicWorkerSPT();
    expect(procedureLogicWorkerSPT).not.toBeUndefined();
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
  it('ProcedureLogicWorkerSPTTest12', function () {
    let procedureLogicWorkerSPT = new ProcedureLogicWorkerSPT();
    window.postMessage = jest.fn(() => true);
    expect(procedureLogicWorkerSPT.getSPTData()).toBeTruthy();
  });
  it('ProcedureLogicWorkerSPTTest14', function () {
    let procedureLogicWorkerSPT = new ProcedureLogicWorkerSPT();
    window.postMessage = jest.fn(() => true);
    expect(procedureLogicWorkerSPT.clearAll()).toBeUndefined();
  });
  it('ProcedureLogicWorkerSPTTest15', function () {
    let procedureLogicWorkerSPT = new ProcedureLogicWorkerSPT();
    window.postMessage = jest.fn(() => true);
    expect(procedureLogicWorkerSPT.getPTSData()).toBeTruthy();
  });
});
