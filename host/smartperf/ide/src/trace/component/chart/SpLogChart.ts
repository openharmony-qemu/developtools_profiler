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

import { SpSystemTrace } from '../SpSystemTrace.js';
import { TraceRow } from '../trace/base/TraceRow.js';
import { queryLogData } from '../../database/SqlLite.js';
import { renders } from '../../database/ui-worker/ProcedureWorker.js';
import { LogRender, LogStruct } from '../../database/ui-worker/ProcedureWorkerLog.js';

const ONE_DAY_NS = 86400000000000;

export class SpLogChart {
  private trace: SpSystemTrace;

  constructor(trace: SpSystemTrace) {
    this.trace = trace;
  }

  async init() {
    let oneDayTime = (window as any).recordEndNS - ONE_DAY_NS;
    let dataArray = await queryLogData(oneDayTime);
    if (dataArray.length === 0) {
      return;
    }
    let folder = await this.initFolder(dataArray);
    this.trace.rowsEL?.appendChild(folder);
  }

  async initFolder(dataArray: LogStruct[]): Promise<TraceRow<LogStruct>> {
    let logsRow = TraceRow.skeleton<LogStruct>();
    logsRow.rowId = 'logs';
    logsRow.index = 0;
    logsRow.rowType = TraceRow.ROW_TYPE_LOGS;
    logsRow.rowParentId = '';
    logsRow.style.height = '42px';
    logsRow.folder = false;
    logsRow.name = 'Logs';
    logsRow.favoriteChangeHandler = this.trace.favoriteChangeHandler;
    logsRow.selectChangeHandler = this.trace.selectChangeHandler;
    logsRow.supplier = () =>
      new Promise((resolve): void => {
        resolve(dataArray);
      });
    logsRow.onThreadHandler = (useCache) => {
      let context: CanvasRenderingContext2D;
      if (logsRow.currentContext) {
        context = logsRow.currentContext;
      } else {
        context = logsRow.collect ? this.trace.canvasFavoritePanelCtx! : this.trace.canvasPanelCtx!;
      }
      logsRow.canvasSave(context);
      (renders.logs as LogRender).renderMainThread(
        {
          context: context,
          useCache: useCache,
          type: 'logs',
        },
        logsRow
      );
      logsRow.canvasRestore(context);
    };
    return logsRow;
  }
}
