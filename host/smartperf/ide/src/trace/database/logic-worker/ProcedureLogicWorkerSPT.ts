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
import { StateProcessThread } from '../../bean/StateProcessThread.js';
import { Utils } from '../../component/trace/base/Utils.js';

export class ProcedureLogicWorkerSPT extends LogicHandler {
  arrTs: Array<ThreadState> = [];
  arrTp: Array<ThreadProcess> = [];
  spt_data: Array<SPT> = [];
  currentEventId: string = '';

  clearAll() {
    this.arrTs.length = 0;
    this.arrTp.length = 0;
    this.spt_data.length = 0;
  }

  handle(data: any): void {
    this.currentEventId = data.id;
    if (data && data.type) {
      switch (data.type) {
        case 'spt-init':
          this.getThreadState();
          break;
        case 'spt-getThreadStateData':
          this.arrTs = convertJSON(data.params.list) || [];
          this.getThreadProcessData();
          break;
        case 'spt-getThreadProcessData':
          this.arrTp = convertJSON(data.params.list) || [];
          this.initProcessThreadStateData();
          break;
        case 'spt-getPTS':
          self.postMessage({
            id: this.currentEventId,
            action: 'spt-getPTS',
            results: this.getPTSData(data.params.leftNs, data.params.rightNs),
          });
          break;
        case 'spt-getSPT':
          self.postMessage({
            id: this.currentEventId,
            action: 'spt-getSPT',
            results: this.getSPTData(data.params.leftNs, data.params.rightNs),
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
      'spt-getThreadStateData',
      `
    select itid,
       state,
       dur,
       ts,
       (ts - start_ts + dur) as end_ts,
       (ts - start_ts) as start_ts,
       cpu
from thread_state,trace_range where dur > 0 and (ts - start_ts) >= 0;
`,
      {}
    );
  }

  getThreadProcessData() {
    this.queryData(
      'spt-getThreadProcessData',
      `
    select A.id,
       A.tid as threadId,
       A.name as thread,
       IP.pid as processId,
       IP.name as process
from thread as A left join process as IP on A.ipid = IP.id
where IP.pid not null;
`,
      {}
    );
  }

  initProcessThreadStateData() {
    let mapTp: Map<number, ThreadProcess> = new Map<number, ThreadProcess>();
    for (let tp of this.arrTp) {
      mapTp.set(tp.id, tp);
    }
    this.spt_data = [];
    for (let tr of this.arrTs) {
      if (mapTp.has(tr.itid)) {
        let tp = mapTp.get(tr.itid);
        let spt = new SPT();
        spt.processId = tp!.processId;
        spt.process = tp!.process;
        spt.thread = tp!.thread;
        spt.threadId = tp!.threadId;
        spt.state = tr.state;
        spt.dur = tr.dur;
        spt.end_ts = tr.end_ts;
        spt.start_ts = tr.start_ts;
        spt.cpu = tr.cpu;
        this.spt_data.push(spt);
      }
    }
    this.arrTp = [];
    this.arrTs = [];
    self.postMessage({
      id: this.currentEventId,
      action: 'spt-init',
      results: [],
    });
  }

  getPTSData(ptsLeftNs: number, ptsRightNs: number): Array<StateProcessThread> {
    let pMap: Map<string, StateProcessThread> = new Map<string, StateProcessThread>();
    let ptMap: Map<string, StateProcessThread> = new Map<string, StateProcessThread>();
    let ptsMap: Map<string, StateProcessThread> = new Map<string, StateProcessThread>();
    this.spt_data.map((d) => {
      if (!(d.end_ts < ptsLeftNs || d.start_ts > ptsRightNs)) {
        if (pMap.has(d.processId + '')) {
          let ptsPMapObj = pMap.get(d.processId + '');
          ptsPMapObj!.count++;
          ptsPMapObj!.wallDuration += d.dur;
          ptsPMapObj!.avgDuration = (ptsPMapObj!.wallDuration / ptsPMapObj!.count).toFixed(2);
          if (d.dur > ptsPMapObj!.maxDuration) {
            ptsPMapObj!.maxDuration = d.dur;
          }
          if (d.dur < ptsPMapObj!.minDuration) {
            ptsPMapObj!.minDuration = d.dur;
          }
        } else {
          let ptsPMapObj = new StateProcessThread();
          ptsPMapObj.id = 'p' + d.processId;
          ptsPMapObj.title = (d.process == null || d.process == '' ? 'Process' : d.process) + '(' + d.processId + ')';
          ptsPMapObj.process = d.process;
          ptsPMapObj.processId = d.processId;
          ptsPMapObj.minDuration = d.dur;
          ptsPMapObj.maxDuration = d.dur;
          ptsPMapObj.count = 1;
          ptsPMapObj.avgDuration = d.dur + '';
          ptsPMapObj.wallDuration = d.dur;
          pMap.set(d.processId + '', ptsPMapObj);
        }
        if (ptMap.has(d.processId + '_' + d.threadId)) {
          let ptsPtMapObj = ptMap.get(d.processId + '_' + d.threadId);
          ptsPtMapObj!.count++;
          ptsPtMapObj!.wallDuration += d.dur;
          ptsPtMapObj!.avgDuration = (ptsPtMapObj!.wallDuration / ptsPtMapObj!.count).toFixed(2);
          if (d.dur > ptsPtMapObj!.maxDuration) {
            ptsPtMapObj!.maxDuration = d.dur;
          }
          if (d.dur < ptsPtMapObj!.minDuration) {
            ptsPtMapObj!.minDuration = d.dur;
          }
        } else {
          let ptsPtMapObj = new StateProcessThread();
          ptsPtMapObj.id = 'p' + d.processId + '_' + 't' + d.threadId;
          ptsPtMapObj.pid = 'p' + d.processId;
          ptsPtMapObj.title = (d.thread == null || d.thread == '' ? 'Thread' : d.thread) + '(' + d.threadId + ')';
          ptsPtMapObj.processId = d.processId;
          ptsPtMapObj.process = d.process;
          ptsPtMapObj.thread = d.thread;
          ptsPtMapObj.threadId = d.threadId;
          ptsPtMapObj.minDuration = d.dur;
          ptsPtMapObj.maxDuration = d.dur;
          ptsPtMapObj.count = 1;
          ptsPtMapObj.avgDuration = d.dur + '';
          ptsPtMapObj.wallDuration = d.dur;
          ptMap.set(d.processId + '_' + d.threadId, ptsPtMapObj);
        }
        if (ptsMap.has(d.processId + '_' + d.threadId + '_' + d.state)) {
          let ptsPtsMapObj = ptsMap.get(d.processId + '_' + d.threadId + '_' + d.state);
          ptsPtsMapObj!.count++;
          ptsPtsMapObj!.wallDuration += d.dur;
          ptsPtsMapObj!.avgDuration = (ptsPtsMapObj!.wallDuration / ptsPtsMapObj!.count).toFixed(2);
          if (d.dur > ptsPtsMapObj!.maxDuration) {
            ptsPtsMapObj!.maxDuration = d.dur;
          }
          if (d.dur < ptsPtsMapObj!.minDuration) {
            ptsPtsMapObj!.minDuration = d.dur;
          }
        } else {
          let ptsPtsMapObj = new StateProcessThread();
          ptsPtsMapObj.id = 'p' + d.processId + '_' + 't' + d.threadId + '_' + (d.state == 'R+' ? 'RP' : d.state);
          ptsPtsMapObj.pid = 'p' + d.processId + '_' + 't' + d.threadId;
          ptsPtsMapObj.title = Utils.getEndState(d.state);
          ptsPtsMapObj.processId = d.processId;
          ptsPtsMapObj.process = d.process;
          ptsPtsMapObj.thread = d.thread;
          ptsPtsMapObj.threadId = d.threadId;
          ptsPtsMapObj.state = d.state;
          ptsPtsMapObj.minDuration = d.dur;
          ptsPtsMapObj.maxDuration = d.dur;
          ptsPtsMapObj.count = 1;
          ptsPtsMapObj.avgDuration = d.dur + '';
          ptsPtsMapObj.wallDuration = d.dur;
          ptsMap.set(d.processId + '_' + d.threadId + '_' + d.state, ptsPtsMapObj);
        }
      }
    });
    let ptsArr: Array<StateProcessThread> = [];
    for (let key of pMap.keys()) {
      let ptsValues = pMap.get(key);
      ptsValues!.children = [];
      for (let itemKey of ptMap.keys()) {
        if (itemKey.startsWith(key + '_')) {
          let sp = ptMap.get(itemKey);
          sp!.children = [];
          for (let keySt of ptsMap.keys()) {
            if (keySt.startsWith(itemKey + '_')) {
              let spt = ptsMap.get(keySt);
              sp!.children.push(spt!);
            }
          }
          ptsValues!.children.push(sp!);
        }
      }
      ptsArr.push(ptsValues!);
    }
    return ptsArr;
  }

  getSPTData(sptLeftNs: number, sptRightNs: number): Array<StateProcessThread> {
    let statesMap: Map<string, StateProcessThread> = new Map<string, StateProcessThread>();
    let spMap: Map<string, StateProcessThread> = new Map<string, StateProcessThread>();
    let sptMap: Map<string, StateProcessThread> = new Map<string, StateProcessThread>();
    this.spt_data.map((d) => {
      if (!(d.end_ts < sptLeftNs || d.start_ts > sptRightNs)) {
        if (statesMap.has(d.state)) {
          let sptStateMapObj = statesMap.get(d.state);
          sptStateMapObj!.count++;
          sptStateMapObj!.wallDuration += d.dur;
          sptStateMapObj!.avgDuration = (sptStateMapObj!.wallDuration / sptStateMapObj!.count).toFixed(2);
          if (d.dur > sptStateMapObj!.maxDuration) {
            sptStateMapObj!.maxDuration = d.dur;
          }
          if (d.dur < sptStateMapObj!.minDuration) {
            sptStateMapObj!.minDuration = d.dur;
          }
        } else {
          let sptStateMapObj = new StateProcessThread();
          sptStateMapObj.id = d.state == 'R+' ? 'RP' : d.state;
          sptStateMapObj.title = Utils.getEndState(d.state);
          sptStateMapObj.state = d.state;
          sptStateMapObj.minDuration = d.dur;
          sptStateMapObj.maxDuration = d.dur;
          sptStateMapObj.count = 1;
          sptStateMapObj.avgDuration = d.dur + '';
          sptStateMapObj.wallDuration = d.dur;
          statesMap.set(d.state, sptStateMapObj);
        }
        if (spMap.has(d.state + '_' + d.processId)) {
          let sptSpMapObj = spMap.get(d.state + '_' + d.processId);
          sptSpMapObj!.count++;
          sptSpMapObj!.wallDuration += d.dur;
          sptSpMapObj!.avgDuration = (sptSpMapObj!.wallDuration / sptSpMapObj!.count).toFixed(2);
          if (d.dur > sptSpMapObj!.maxDuration) {
            sptSpMapObj!.maxDuration = d.dur;
          }
          if (d.dur < sptSpMapObj!.minDuration) {
            sptSpMapObj!.minDuration = d.dur;
          }
        } else {
          let sptSpMapObj = new StateProcessThread();
          sptSpMapObj.id = (d.state == 'R+' ? 'RP' : d.state) + '_' + d.processId;
          sptSpMapObj.pid = d.state == 'R+' ? 'RP' : d.state;
          sptSpMapObj.title = (d.process == null || d.process == '' ? 'Process' : d.process) + '(' + d.processId + ')';
          sptSpMapObj.processId = d.processId;
          sptSpMapObj.process = d.process;
          sptSpMapObj.state = d.state;
          sptSpMapObj.minDuration = d.dur;
          sptSpMapObj.maxDuration = d.dur;
          sptSpMapObj.count = 1;
          sptSpMapObj.avgDuration = d.dur + '';
          sptSpMapObj.wallDuration = d.dur;
          spMap.set(d.state + '_' + d.processId, sptSpMapObj);
        }
        if (sptMap.has(d.state + '_' + d.processId + '_' + d.threadId)) {
          let sptMapObject = sptMap.get(d.state + '_' + d.processId + '_' + d.threadId);
          sptMapObject!.count++;
          sptMapObject!.wallDuration += d.dur;
          sptMapObject!.avgDuration = (sptMapObject!.wallDuration / sptMapObject!.count).toFixed(2);
          if (d.dur > sptMapObject!.maxDuration) {
            sptMapObject!.maxDuration = d.dur;
          }
          if (d.dur < sptMapObject!.minDuration) {
            sptMapObject!.minDuration = d.dur;
          }
        } else {
          let sptMapObject = new StateProcessThread();
          sptMapObject.id = (d.state == 'R+' ? 'RP' : d.state) + '_' + d.processId + '_' + d.threadId;
          sptMapObject.pid = (d.state == 'R+' ? 'RP' : d.state) + '_' + d.processId;
          sptMapObject.title = (d.thread == null || d.thread == '' ? 'Thread' : d.thread) + '(' + d.threadId + ')';
          sptMapObject.processId = d.processId;
          sptMapObject.process = d.process;
          sptMapObject.thread = d.thread;
          sptMapObject.threadId = d.threadId;
          sptMapObject.state = d.state;
          sptMapObject.minDuration = d.dur;
          sptMapObject.maxDuration = d.dur;
          sptMapObject.count = 1;
          sptMapObject.avgDuration = d.dur + '';
          sptMapObject.wallDuration = d.dur;
          sptMap.set(d.state + '_' + d.processId + '_' + d.threadId, sptMapObject);
        }
      }
    });
    let sptArr: Array<StateProcessThread> = [];
    for (let key of statesMap.keys()) {
      let stateValue = statesMap.get(key);
      stateValue!.children = [];
      for (let spKey of spMap.keys()) {
        if (spKey.startsWith(key + '_')) {
          let sp = spMap.get(spKey);
          sp!.children = [];
          for (let stKey of sptMap.keys()) {
            if (stKey.startsWith(spKey + '_')) {
              let spt = sptMap.get(stKey);
              sp!.children.push(spt!);
            }
          }
          stateValue!.children.push(sp!);
        }
      }
      sptArr.push(stateValue!);
    }
    return sptArr;
  }
}

export class ThreadState {
  itid: number = 0;
  state: string = '';
  dur: number = 0;
  ts: number = 0;
  end_ts: number = 0;
  start_ts: number = 0;
  cpu: number = 0;
}

export class ThreadProcess {
  id: number = 0;
  threadId: number = 0;
  thread: string = '';
  processId: number = 0;
  process: string = '';
}

export class SPT {
  process: string = '';
  processId: number = 0;
  thread: string = '';
  threadId: number = 0;
  state: string = '';
  dur: number = 0;
  start_ts: number = 0;
  end_ts: number = 0;
  cpu: number = 0;
  priority: string = '-';
  note: string = '-';
}
