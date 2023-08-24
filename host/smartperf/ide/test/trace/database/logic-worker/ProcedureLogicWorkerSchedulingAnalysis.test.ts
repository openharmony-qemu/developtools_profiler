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
import { ProcedureLogicWorkerSchedulingAnalysis,
    FreqThread,
    ThreadCpuUsage,
    CpuAnalysis,
    CpuMeasure,
    Irq,
    CpuUsage,
} from '../../../../dist/trace/database/logic-worker/ProcedureLogicWorkerSchedulingAnalysis.js';

describe('ProcedureLogicWorkerSchedulingAnalysis Test', () => {
    it('ProcedureLogicWorkerSchedulingAnalysisTest01', function () {
        let procedureLogicWorkerSchedulingAnalysis = new ProcedureLogicWorkerSchedulingAnalysis();
        expect(procedureLogicWorkerSchedulingAnalysis).not.toBeUndefined();
    });
    it('ProcedureLogicWorkerSchedulingAnalysisTest02', function () {
        let procedureLogicWorkerSchedulingAnalysis = new ProcedureLogicWorkerSchedulingAnalysis();
        let data = {
            params: [{
                endTs:1,
                total:2,
                list:'',
            },
            ],
            type:'',
            id:1,
            action:'',
        }
        window.postMessage = jest.fn(() => true);
        expect(procedureLogicWorkerSchedulingAnalysis.handle(data)).toBeUndefined();
    });
    it('ProcedureLogicWorkerSchedulingAnalysisTest03', function () {
        let freqThread  = new FreqThread ();
        expect(freqThread).not.toBeUndefined();
    });
    it('ProcedureLogicWorkerSchedulingAnalysisTest04', function () {
        let threadCpuUsage = new ThreadCpuUsage();
        expect(threadCpuUsage).not.toBeUndefined();
    });
    it('ProcedureLogicWorkerSchedulingAnalysisTest05', function () {
        let cpuAnalysis = new CpuAnalysis();
        expect(cpuAnalysis).not.toBeUndefined();
    });
    it('ProcedureLogicWorkerSchedulingAnalysisTest06', function () {
        let cpuMeasure = new CpuMeasure();
        expect(cpuMeasure).not.toBeUndefined();
    });
    it('ProcedureLogicWorkerSchedulingAnalysisTest07', function () {
        let irq = new Irq();
        expect(irq).not.toBeUndefined();
    });
    it('ProcedureLogicWorkerSchedulingAnalysisTest08', function () {
        let cpuUsage = new CpuUsage();
        expect(cpuUsage).not.toBeUndefined();
    });
    it('ProcedureLogicWorkerSchedulingAnalysisTest09', function () {
        let procedureLogicWorkerSchedulingAnalysis = new ProcedureLogicWorkerSchedulingAnalysis();
        let arr = [{
            cpu: 1,
            dur: 1,
            ts: 1,
            freqArr: { cpu: 1, freq: 1, dur: 1 }
        }]
        expect(procedureLogicWorkerSchedulingAnalysis.handlerThreadFreqData(arr)).toStrictEqual([]);
    });
    it('ProcedureLogicWorkerSchedulingAnalysisTest10', function () {
        let procedureLogicWorkerSchedulingAnalysis = new ProcedureLogicWorkerSchedulingAnalysis();
        let arr = [{
            cpu: 1,
            dur: 1,
            ts: 1,
            freqArr: { cpu: 1, freq: 1, dur: 1 }
        }]
        expect(procedureLogicWorkerSchedulingAnalysis.handlerFreqThreadData(arr)).toStrictEqual([]);
    });
    it('ProcedureLogicWorkerSchedulingAnalysisTest11', function () {
        let procedureLogicWorkerSchedulingAnalysis = new ProcedureLogicWorkerSchedulingAnalysis();
        let arr = [{
            cpu: 1,
            dur: 1,
            ts: 1,
            freqArr: { cpu: 1, freq: 1, dur: 1 }
        }]
        expect(procedureLogicWorkerSchedulingAnalysis.groupFreqByCpu(arr)).toBeUndefined();
    });
    it('ProcedureLogicWorkerSchedulingAnalysisTest12', function () {
        let procedureLogicWorkerSchedulingAnalysis = new ProcedureLogicWorkerSchedulingAnalysis();
        let arr = [{
            cpu: 1,
            dur: 1,
            ts: 1,
            freqArr: { cpu: 1, freq: 1, dur: 1 }
        }]
        expect(procedureLogicWorkerSchedulingAnalysis.handleCPUIdle0Map(arr)).toBeUndefined();
    });
    it('ProcedureLogicWorkerSchedulingAnalysisTest13', function () {
        let procedureLogicWorkerSchedulingAnalysis = new ProcedureLogicWorkerSchedulingAnalysis();
        let arr = [{
            cpu: 1,
            dur: 1,
            ts: 1,
            freqArr: { cpu: 1, freq: 1, dur: 1 }
        }]
        expect(procedureLogicWorkerSchedulingAnalysis.getEffectiveFrequencyDur(arr)).toBeUndefined();
    });
    it('ProcedureLogicWorkerSchedulingAnalysisTest14', function () {
        let procedureLogicWorkerSchedulingAnalysis = new ProcedureLogicWorkerSchedulingAnalysis();
        let arr = [{
            cpu: 1,
            dur: 1,
            ts: 1,
            freqArr: { cpu: 1, freq: 1, dur: 1 }
        }]
        expect(procedureLogicWorkerSchedulingAnalysis.handleProcessThread(arr)).toBeUndefined();
    });
    it('ProcedureLogicWorkerSchedulingAnalysisTest15', function () {
        let procedureLogicWorkerSchedulingAnalysis = new ProcedureLogicWorkerSchedulingAnalysis();
        window.postMessage = jest.fn(() => true);
        expect(procedureLogicWorkerSchedulingAnalysis.getCpuFrequency('')).toBeUndefined();
    });
    it('ProcedureLogicWorkerSchedulingAnalysisTest16', function () {
        let procedureLogicWorkerSchedulingAnalysis = new ProcedureLogicWorkerSchedulingAnalysis();
        window.postMessage = jest.fn(() => true);
        expect(procedureLogicWorkerSchedulingAnalysis.getProcessAndThread()).toBeUndefined();
    });
    it('ProcedureLogicWorkerSchedulingAnalysisTest17', function () {
        let procedureLogicWorkerSchedulingAnalysis = new ProcedureLogicWorkerSchedulingAnalysis();
        window.postMessage = jest.fn(() => true);
        expect(procedureLogicWorkerSchedulingAnalysis.getCpuUsage()).toBeUndefined();
    });
    it('ProcedureLogicWorkerSchedulingAnalysisTest18', function () {
        let procedureLogicWorkerSchedulingAnalysis = new ProcedureLogicWorkerSchedulingAnalysis();
        window.postMessage = jest.fn(() => true);
        expect(procedureLogicWorkerSchedulingAnalysis.queryData('','',[])).toBeUndefined();
    });
    it('ProcedureLogicWorkerSchedulingAnalysisTest19', function () {
        let procedureLogicWorkerSchedulingAnalysis = new ProcedureLogicWorkerSchedulingAnalysis();
        window.postMessage = jest.fn(() => true);
        expect(procedureLogicWorkerSchedulingAnalysis.getThreadStateByCpu(1)).toBeUndefined();
    });
    it('ProcedureLogicWorkerSchedulingAnalysisTest20', function () {
        let procedureLogicWorkerSchedulingAnalysis = new ProcedureLogicWorkerSchedulingAnalysis();
        window.postMessage = jest.fn(() => true);
        expect(procedureLogicWorkerSchedulingAnalysis.getCpuIdle0()).toBeUndefined();
    });
    it('ProcedureLogicWorkerSchedulingAnalysisTest21', function () {
        let procedureLogicWorkerSchedulingAnalysis = new ProcedureLogicWorkerSchedulingAnalysis();
        window.postMessage = jest.fn(() => true);
        expect(procedureLogicWorkerSchedulingAnalysis.getCpuIdle()).toBeUndefined();
    });
    it('ProcedureLogicWorkerSchedulingAnalysisTest22', function () {
        let procedureLogicWorkerSchedulingAnalysis = new ProcedureLogicWorkerSchedulingAnalysis();
        window.postMessage = jest.fn(() => true);
        expect(procedureLogicWorkerSchedulingAnalysis.getCpuIrq()).toBeUndefined();
    });
    it('ProcedureLogicWorkerSchedulingAnalysisTest23', function () {
        let procedureLogicWorkerSchedulingAnalysis = new ProcedureLogicWorkerSchedulingAnalysis();
        window.postMessage = jest.fn(() => true);
        expect(procedureLogicWorkerSchedulingAnalysis.queryThreadCpuUsage([1],[0],[2])).toBeUndefined();
    });
    it('ProcedureLogicWorkerSchedulingAnalysisTest24', function () {
        let procedureLogicWorkerSchedulingAnalysis = new ProcedureLogicWorkerSchedulingAnalysis();
        window.postMessage = jest.fn(() => true);
        expect(procedureLogicWorkerSchedulingAnalysis.queryThreadRunTime(1)).toBeUndefined();
    });
    it('ProcedureLogicWorkerSchedulingAnalysisTest25', function () {
        let procedureLogicWorkerSchedulingAnalysis = new ProcedureLogicWorkerSchedulingAnalysis();
        window.postMessage = jest.fn(() => true);
        expect(procedureLogicWorkerSchedulingAnalysis.queryProcessThreadCount()).toBeUndefined();
    });
    it('ProcedureLogicWorkerSchedulingAnalysisTest26', function () {
        let procedureLogicWorkerSchedulingAnalysis = new ProcedureLogicWorkerSchedulingAnalysis();
        window.postMessage = jest.fn(() => true);
        expect(procedureLogicWorkerSchedulingAnalysis.queryProcessSwitchCount()).toBeUndefined();
    });
    it('ProcedureLogicWorkerSchedulingAnalysisTest27', function () {
        let procedureLogicWorkerSchedulingAnalysis = new ProcedureLogicWorkerSchedulingAnalysis();
        window.postMessage = jest.fn(() => true);
        expect(procedureLogicWorkerSchedulingAnalysis.queryThreadStateByTid(1)).toBeUndefined();
    });
    it('ProcedureLogicWorkerSchedulingAnalysisTest28', function () {
        let procedureLogicWorkerSchedulingAnalysis = new ProcedureLogicWorkerSchedulingAnalysis();
        let data = {
            params: [{
                endTs:1,
                total:2,
                list:'',
            },
            ],
            type:'scheduling-clearData',
            id:1,
            action:'',
        }
        window.postMessage = jest.fn(() => true);
        expect(procedureLogicWorkerSchedulingAnalysis.handle(data)).toBeUndefined();
    });
    it('ProcedureLogicWorkerSchedulingAnalysisTest29', function () {
        let procedureLogicWorkerSchedulingAnalysis = new ProcedureLogicWorkerSchedulingAnalysis();
        let data = {
            params: [{
                endTs:1,
                total:2,
                list:'',
            },
            ],
            type:'scheduling-initFreqData',
            id:1,
            action:'',
        }
        window.postMessage = jest.fn(() => true);
        expect(procedureLogicWorkerSchedulingAnalysis.handle(data)).toBeUndefined();
    });
    it('ProcedureLogicWorkerSchedulingAnalysisTest30', function () {
        let procedureLogicWorkerSchedulingAnalysis = new ProcedureLogicWorkerSchedulingAnalysis();
        let data = {
            params: [{
                endTs:1,
                total:2,
                list:'',
            },
            ],
            type:'scheduling-getProcessAndThread',
            id:1,
            action:'',
        }
        window.postMessage = jest.fn(() => true);
        expect(procedureLogicWorkerSchedulingAnalysis.handle(data)).toBeUndefined();
    });
    it('ProcedureLogicWorkerSchedulingAnalysisTest31', function () {
        let procedureLogicWorkerSchedulingAnalysis = new ProcedureLogicWorkerSchedulingAnalysis();
        let data = {
            params: [{
                endTs:1,
                total:2,
                list:'',
            },
            ],
            type:'scheduling-getCpuIdle0',
            id:1,
            action:'',
        }
        window.postMessage = jest.fn(() => true);
        expect(procedureLogicWorkerSchedulingAnalysis.handle(data)).toBeUndefined();
    });
    it('ProcedureLogicWorkerSchedulingAnalysisTest32', function () {
        let procedureLogicWorkerSchedulingAnalysis = new ProcedureLogicWorkerSchedulingAnalysis();
        let data = {
            params: [{
                endTs:1,
                total:2,
                list:'',
            },
            ],
            type:'scheduling-getCpuUsage',
            id:1,
            action:'',
        }
        window.postMessage = jest.fn(() => true);
        expect(procedureLogicWorkerSchedulingAnalysis.handle(data)).toBeUndefined();
    });
    it('ProcedureLogicWorkerSchedulingAnalysisTest33', function () {
        let procedureLogicWorkerSchedulingAnalysis = new ProcedureLogicWorkerSchedulingAnalysis();
        let data = {
            params: [{
                endTs:1,
                total:2,
                list:'',
            },
            ],
            type:'scheduling-CPU Frequency',
            id:1,
            action:'',
        }
        window.postMessage = jest.fn(() => true);
        expect(procedureLogicWorkerSchedulingAnalysis.handle(data)).toBeUndefined();
    });
    it('ProcedureLogicWorkerSchedulingAnalysisTest34', function () {
        let procedureLogicWorkerSchedulingAnalysis = new ProcedureLogicWorkerSchedulingAnalysis();
        let data = {
            params: [{
                endTs:1,
                total:2,
                list:'',
            },
            ],
            type:'scheduling-CPU Frequency Thread',
            id:1,
            action:'',
        }
        window.postMessage = jest.fn(() => true);
        expect(procedureLogicWorkerSchedulingAnalysis.handle(data)).toBeUndefined();
    });
    it('ProcedureLogicWorkerSchedulingAnalysisTest35', function () {
        let procedureLogicWorkerSchedulingAnalysis = new ProcedureLogicWorkerSchedulingAnalysis();
        let data = {
            params: [{
                endTs:1,
                total:2,
                list:'',
            },
            ],
            type:'scheduling-CPU Idle',
            id:1,
            action:'',
        }
        window.postMessage = jest.fn(() => true);
        expect(procedureLogicWorkerSchedulingAnalysis.handle(data)).toBeUndefined();
    });
    it('ProcedureLogicWorkerSchedulingAnalysisTest37', function () {
        let procedureLogicWorkerSchedulingAnalysis = new ProcedureLogicWorkerSchedulingAnalysis();
        let data = {
            params: [{
                endTs:1,
                total:2,
                list:'',
            },
            ],
            type:'scheduling-CPU Irq',
            id:1,
            action:'',
        }
        window.postMessage = jest.fn(() => true);
        expect(procedureLogicWorkerSchedulingAnalysis.handle(data)).toBeUndefined();
    });
    it('ProcedureLogicWorkerSchedulingAnalysisTest38', function () {
        let procedureLogicWorkerSchedulingAnalysis = new ProcedureLogicWorkerSchedulingAnalysis();
        let data = {
            params: [{
                endTs:1,
                total:2,
                list:'',
            },
            ],
            type:'scheduling-Thread CpuUsage',
            id:1,
            action:'',
        }
        window.postMessage = jest.fn(() => true);
        expect(procedureLogicWorkerSchedulingAnalysis.handle(data)).toBeUndefined();
    });
    it('ProcedureLogicWorkerSchedulingAnalysisTest39', function () {
        let procedureLogicWorkerSchedulingAnalysis = new ProcedureLogicWorkerSchedulingAnalysis();
        let data = {
            params: [{
                endTs:1,
                total:2,
                list:'',
            },
            ],
            type:'scheduling-Thread RunTime',
            id:1,
            action:'',
        }
        window.postMessage = jest.fn(() => true);
        expect(procedureLogicWorkerSchedulingAnalysis.handle(data)).toBeUndefined();
    });
    it('ProcedureLogicWorkerSchedulingAnalysisTest40', function () {
        let procedureLogicWorkerSchedulingAnalysis = new ProcedureLogicWorkerSchedulingAnalysis();
        let data = {
            params: [{
                endTs:1,
                total:2,
                list:'',
            },
            ],
            type:'scheduling-Process ThreadCount',
            id:1,
            action:'',
        }
        window.postMessage = jest.fn(() => true);
        expect(procedureLogicWorkerSchedulingAnalysis.handle(data)).toBeUndefined();
    });
    it('ProcedureLogicWorkerSchedulingAnalysisTest41', function () {
        let procedureLogicWorkerSchedulingAnalysis = new ProcedureLogicWorkerSchedulingAnalysis();
        let data = {
            params: [{
                endTs:1,
                total:2,
                list:'',
            },
            ],
            type:'scheduling-Process SwitchCount',
            id:1,
            action:'',
        }
        window.postMessage = jest.fn(() => true);
        expect(procedureLogicWorkerSchedulingAnalysis.handle(data)).toBeUndefined();
    });
    it('ProcedureLogicWorkerSchedulingAnalysisTest42', function () {
        let procedureLogicWorkerSchedulingAnalysis = new ProcedureLogicWorkerSchedulingAnalysis();
        let data = {
            params: [{
                endTs:1,
                total:2,
                list:'',
            },
            ],
            type:'scheduling-Thread Freq',
            id:1,
            action:'',
        }
        window.postMessage = jest.fn(() => true);
        expect(procedureLogicWorkerSchedulingAnalysis.handle(data)).toBeUndefined();
    });
})