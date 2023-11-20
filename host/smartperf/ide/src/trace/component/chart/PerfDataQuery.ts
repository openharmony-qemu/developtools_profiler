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

import { queryPerfFiles } from '../../database/SqlLite.js';
import { PerfCall, PerfFile } from '../../bean/PerfProfile.js';
import { info } from '../../../log/Log.js';
import { SpHiPerf } from './SpHiPerf.js';
import { procedurePool } from '../../database/Procedure.js';
import {PerfCallChain} from "../../database/logic-worker/ProcedureLogicWorkerPerf";

export class PerfDataQuery {
  filesData: any = {};
  callChainMap: Map<number, PerfCall> = new Map<number, PerfCall>();

  async initPerfCache() {
    await this.initPerfCallChainMap();
    await this.initPerfFiles();
  }

  async initPerfCallChainMap() {
    this.callChainMap.clear();
  }

  async initPerfFiles() {
    let files = await queryPerfFiles();
    info('PerfFiles Data size is: ', files!.length);
    files.forEach((file) => {
      this.filesData[file.fileId] = this.filesData[file.fileId] || [];
      PerfFile.setFileName(file);
      this.filesData[file.fileId].push(file);
    });
    let results = await new Promise<any>((resolve, reject) => {
      procedurePool.submitWithName('logic0', 'perf-init', SpHiPerf.stringResult, undefined, (res: any) => {
        resolve(res);
      });
    });
    this.callChainMap = results as any;
    info('Perf Files Data initialized');
  }

  getLibName(fileId: number, symbolId: number) {
    let name = 'unknown';
    if (symbolId == -1) {
      if (this.filesData[fileId] && this.filesData[fileId].length > 0) {
        name = this.filesData[fileId][0].fileName;
      }
    } else {
      if (this.filesData[fileId] && this.filesData[fileId].length > symbolId) {
        name = this.filesData[fileId][symbolId].fileName;
      }
    }
    return name.replace(/</g, '&lt;').replace(/>/g, '&gt;');
  }
}

export const perfDataQuery = new PerfDataQuery();
