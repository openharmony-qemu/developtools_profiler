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

import { convertJSON, LogicHandler } from './ProcedureLogicWorkerCommon.js';

export class ProcedureLogicWorkerCpuState extends LogicHandler {
  currentEventId: string = '';

  handle(data: any): void {
    this.currentEventId = data.id;
    if (data && data.type) {
      switch (data.type) {
        case 'CpuState-getCpuState':
          if (data.params.list) {
            let arr = convertJSON(data.params.list) || [];
            self.postMessage({
              id: data.id,
              action: data.action,
              results: this.supplementCpuState(arr),
            });
            arr = [];
          } else {
            this.getCpuState(data.params.cpu);
          }
          break;
      }
    }
  }

  clearAll() {}

  getCpuState(cpu: number) {
    this.queryData(
      this.currentEventId,
      'CpuState-getCpuState',
      `
            select (A.ts - B.start_ts) as startTs,
                A.dur,
                (A.ts - B.start_ts + A.dur) as endTs,
                (case
                    when state = 'Running' then 0
                    when state = 'S' then 1
                    when state = 'R' or state = 'R+' then 2
                    else 3 end)    as value
                from thread_state A,trace_range B
            where cpu = $cpu and startTs >= 0;
        `,
      { $cpu: cpu }
    );
  }

  supplementCpuState(arr: Array<CpuState>): Array<CpuState> {
    let source: Array<CpuState> = [];
    if (arr.length > 0) {
      let first = arr[0];
      if (first.startTs > 0) {
        let cs: CpuState = new CpuState();
        cs.startTs = 0;
        cs.value = 3;
        cs.dur = first.startTs;
        cs.endTs = first.startTs;
        source.push(cs);
      }
      source.push(first);
      for (let i = 1, len = arr.length; i < len; i++) {
        let last = arr[i - 1];
        let current = arr[i];
        if (current.startTs > last.endTs) {
          let cs: CpuState = new CpuState();
          cs.startTs = last.endTs;
          cs.value = 3;
          cs.dur = current.startTs - last.endTs;
          cs.endTs = current.startTs;
          source.push(cs);
        }
        source.push(current);
      }
    }
    return source;
  }
}

export class CpuState {
  startTs: number = 0;
  endTs: number = 0;
  dur: number = 0;
  value: number = 0;
}
