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
import { SliceGroup } from '../../bean/StateProcessThread.js';

export class ProcedureLogicWorkerSPT extends LogicHandler {
  threadSlice: Array<ThreadSlice> = [];
  currentEventId: string = '';

  clearAll() {
    this.threadSlice.length = 0;
  }

  handle(data: any): void {
    this.currentEventId = data.id;
    if (data && data.type) {
      switch (data.type) {
        case 'spt-init':
          if (data.params.list) {
            this.threadSlice = convertJSON(data.params.list);
            self.postMessage({
              id: this.currentEventId,
              action: 'spt-init',
              results: [],
            });
          } else {
            this.getThreadState();
          }
          break;
        case 'spt-getPTS':
          self.postMessage({
            id: this.currentEventId,
            action: 'spt-getPTS',
            results: this.getPTSData(data.params.leftNs, data.params.rightNs, data.params.cpus),
          });
          break;
        case 'spt-getSPT':
          self.postMessage({
            id: this.currentEventId,
            action: 'spt-getSPT',
            results: this.getSPTData(data.params.leftNs, data.params.rightNs, data.params.cpus),
          });
          break;
        case 'spt-getCpuPriority':
          self.postMessage({
            id: this.currentEventId,
            action: 'spt-getCpuPriority',
            results: this.threadSlice,
          });
          break;
        case 'spt-getCpuPriorityByTime':
          const result = this.threadSlice.filter((item: ThreadSlice) => {
            return !(item.endTs! < data.params.leftNs || item.startTs! > data.params.rightNs);
          });
          self.postMessage({
            id: this.currentEventId,
            action: 'spt-getCpuPriorityByTime',
            results: result,
          });
          break;
      }
    }
  }

  queryData(queryName: string, sql: string, args: any) {
    self.postMessage({
      id: this.currentEventId,
      type: queryName,
      isQuery: true,
      args: args,
      sql: sql,
    });
  }

  getThreadState() {
    this.queryData(
      'spt-init',
      `
    select
       state,
       dur,
       (ts - start_ts) as startTs,
       (ts - start_ts + dur) as endTs,
       cpu,
       tid,
       itid as itId,
       arg_setid as argSetID,
       pid
from thread_state,trace_range where dur > 0 and (ts - start_ts) >= 0;
`,
      {}
    );
  }

  getPTSData(ptsLeftNs: number, ptsRightNs: number, cpus: Array<number>) {
    let ptsFilter = this.threadSlice.filter(
      (it) =>
        Math.max(ptsLeftNs, it.startTs!) < Math.min(ptsRightNs, it.startTs! + it.dur!) &&
        (it.cpu === null || it.cpu === undefined || cpus.includes(it.cpu))
    );
    let group: any = {};
    ptsFilter.forEach((slice) => {
      let item = {
        title: `S-${slice.state}`,
        count: 1,
        state: slice.state,
        tid: slice.tid,
        pid: slice.pid,
        minDuration: slice.dur || 0,
        maxDuration: slice.dur || 0,
        wallDuration: slice.dur || 0,
        avgDuration: `${slice.dur}`,
      };
      if (group[`${slice.pid}`]) {
        let process = group[`${slice.pid}`];
        process.count += 1;
        process.wallDuration += slice.dur;
        process.minDuration = Math.min(process.minDuration, slice.dur!);
        process.maxDuration = Math.max(process.maxDuration, slice.dur!);
        process.avgDuration = (process.wallDuration / process.count).toFixed(2);
        let thread = process.children.find((child: any) => child.title === `T-${slice.tid}`);
        if (thread) {
          thread.count += 1;
          thread.wallDuration += slice.dur;
          thread.minDuration = Math.min(thread.minDuration, slice.dur!);
          thread.maxDuration = Math.max(thread.maxDuration, slice.dur!);
          thread.avgDuration = (thread.wallDuration / thread.count).toFixed(2);
          let state = thread.children.find((child: any) => child.title === `S-${slice.state}`);
          if (state) {
            state.count += 1;
            state.wallDuration += slice.dur;
            state.minDuration = Math.min(state.minDuration, slice.dur!);
            state.maxDuration = Math.max(state.maxDuration, slice.dur!);
            state.avgDuration = (state.wallDuration / state.count).toFixed(2);
          } else {
            thread.children.push(item);
          }
        } else {
          process.children.push({
            title: `T-${slice.tid}`,
            count: 1,
            pid: slice.pid,
            tid: slice.tid,
            minDuration: slice.dur || 0,
            maxDuration: slice.dur || 0,
            wallDuration: slice.dur || 0,
            avgDuration: `${slice.dur}`,
            children: [item],
          });
        }
      } else {
        group[`${slice.pid}`] = {
          title: `P-${slice.pid}`,
          count: 1,
          pid: slice.pid,
          minDuration: slice.dur || 0,
          maxDuration: slice.dur || 0,
          wallDuration: slice.dur || 0,
          avgDuration: `${slice.dur}`,
          children: [
            {
              title: `T-${slice.tid}`,
              count: 1,
              pid: slice.pid,
              tid: slice.tid,
              minDuration: slice.dur || 0,
              maxDuration: slice.dur || 0,
              wallDuration: slice.dur || 0,
              avgDuration: `${slice.dur}`,
              children: [item],
            },
          ],
        };
      }
    });
    return Object.values(group);
  }

  getSPTData(sptLeftNs: number, sptRightNs: number, cpus: Array<number>) {
    let sptFilter = this.threadSlice.filter(
      (it) =>
        Math.max(sptLeftNs, it.startTs!) < Math.min(sptRightNs, it.startTs! + it.dur!) &&
        (it.cpu === null || it.cpu === undefined || cpus.includes(it.cpu))
    );
    let group: any = {};
    sptFilter.forEach((slice) => {
      let item = {
        title: `T-${slice.tid}`,
        count: 1,
        state: slice.state,
        pid: slice.pid,
        tid: slice.tid,
        minDuration: slice.dur || 0,
        maxDuration: slice.dur || 0,
        wallDuration: slice.dur || 0,
        avgDuration: `${slice.dur}`,
      };
      if (group[`${slice.state}`]) {
        let state = group[`${slice.state}`];
        state.count += 1;
        state.wallDuration += slice.dur;
        state.minDuration = Math.min(state.minDuration, slice.dur!);
        state.maxDuration = Math.max(state.maxDuration, slice.dur!);
        state.avgDuration = (state.wallDuration / state.count).toFixed(2);
        let process = state.children.find((child: any) => child.title === `P-${slice.pid}`);
        if (process) {
          process.count += 1;
          process.wallDuration += slice.dur;
          process.minDuration = Math.min(process.minDuration, slice.dur!);
          process.maxDuration = Math.max(process.maxDuration, slice.dur!);
          process.avgDuration = (process.wallDuration / process.count).toFixed(2);
          let thread = process.children.find((child: any) => child.title === `T-${slice.tid}`);
          if (thread) {
            thread.count += 1;
            thread.wallDuration += slice.dur;
            thread.minDuration = Math.min(thread.minDuration, slice.dur!);
            thread.maxDuration = Math.max(thread.maxDuration, slice.dur!);
            thread.avgDuration = (thread.wallDuration / thread.count).toFixed(2);
          } else {
            process.children.push(item);
          }
        } else {
          state.children.push({
            title: `P-${slice.pid}`,
            count: 1,
            state: slice.state,
            pid: slice.pid,
            minDuration: slice.dur || 0,
            maxDuration: slice.dur || 0,
            wallDuration: slice.dur || 0,
            avgDuration: `${slice.dur}`,
            children: [item],
          });
        }
      } else {
        group[`${slice.state}`] = {
          title: `S-${slice.state}`,
          count: 1,
          state: slice.state,
          minDuration: slice.dur || 0,
          maxDuration: slice.dur || 0,
          wallDuration: slice.dur || 0,
          avgDuration: `${slice.dur}`,
          children: [
            {
              title: `P-${slice.pid}`,
              count: 1,
              state: slice.state,
              pid: slice.pid,
              minDuration: slice.dur || 0,
              maxDuration: slice.dur || 0,
              wallDuration: slice.dur || 0,
              avgDuration: `${slice.dur}`,
              children: [item],
            },
          ],
        };
      }
    });
    return Object.values(group);
  }
}

export class ThreadSlice {
  state?: string;
  dur?: number;
  startTs?: number;
  endTs?: number;
  cpu?: number | null;
  tid?: number;
  pid?: number;
  itId?: number;
  priorityType?: string;
  end_state?: string;
  priority?: number;
  argSetID?: number;
}
