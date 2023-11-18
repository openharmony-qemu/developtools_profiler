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

import { ProcedureLogicWorkerPerf } from './ProcedureLogicWorkerPerf.js';
import { ProcedureLogicWorkerNativeMemory } from './ProcedureLogicWorkerNativeNemory.js';
import { ProcedureLogicWorkerFileSystem } from './ProcedureLogicWorkerFileSystem.js';
import { ProcedureLogicWorkerSPT } from './ProcedureLogicWorkerSPT.js';
import { ProcedureLogicWorkerCpuState } from './ProcedureLogicWorkerCpuState.js';
import { ProcedureLogicWorkerSchedulingAnalysis } from './ProcedureLogicWorkerSchedulingAnalysis.js';
import { DataCache } from './ProcedureLogicWorkerCommon.js';
import { ProcedureLogicWorkerJsCpuProfiler } from './ProcedureLogicWorkerJsCpuProfiler.js';

let logicWorker: any = {
  perf: new ProcedureLogicWorkerPerf(),
  'native-memory': new ProcedureLogicWorkerNativeMemory(),
  fileSystem: new ProcedureLogicWorkerFileSystem(),
  CpuState: new ProcedureLogicWorkerCpuState(),
  spt: new ProcedureLogicWorkerSPT(),
  scheduling: new ProcedureLogicWorkerSchedulingAnalysis(),
  jsCpuProfile: new ProcedureLogicWorkerJsCpuProfiler(),
};

function match(req: any) {
  if (req.type === 'clear') {
    Reflect.ownKeys(logicWorker).forEach(key => logicWorker[key].clearAll());
    DataCache.getInstance().clearAll();
    return;
  }
  Reflect.ownKeys(logicWorker).filter((it) => {
    if (req.type && req.type.startsWith(it as string)) {
      logicWorker[it].handle(req);
    }
  });
}

self.onmessage = function (e: any) {
  match(e.data);
};
