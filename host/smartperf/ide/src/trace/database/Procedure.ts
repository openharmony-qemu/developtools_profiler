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

import { query } from './SqlLite.js';

class ProcedureThread extends Worker {
  busy: boolean = false;
  isCancelled: boolean = false;
  id: number = -1;
  taskMap: any = {};
  name: string | undefined;

  uuid(): string {
    // @ts-ignore
    return ([1e7] + -1e3 + -4e3 + -8e3 + -1e11).replace(/[018]/g, (c) =>
      (c ^ (crypto.getRandomValues(new Uint8Array(1))[0] & (15 >> (c / 4)))).toString(16)
    );
  }

  queryFunc(type: string, args: any, transfer: any, handler: Function) {
    this.busy = true;
    let id = this.uuid();
    this.taskMap[id] = handler;
    let pam = {
      id: id,
      type: type,
      params: args,
    };
    if (transfer) {
      try {
        if (Array.isArray(transfer)) {
          if (transfer.length > 0) {
            this.postMessage(pam, [...transfer]);
          } else {
            this.postMessage(pam);
          }
        } else {
          this.postMessage(pam, [transfer]);
        }
      } catch (e: any) {}
    } else {
      this.postMessage(pam);
    }
  }

  cancel() {
    this.isCancelled = true;
    this.terminate();
  }
}

class ProcedurePool {
  static cpuCount = Math.floor((window.navigator.hardwareConcurrency || 4) / 2);
  maxThreadNumber: number = 1;
  works: Array<ProcedureThread> = [];
  timelineChange: ((a: any) => void) | undefined | null = null;
  cpusLen = ProcedurePool.build('cpu', 0);
  freqLen = ProcedurePool.build('freq', 0);
  processLen = ProcedurePool.build('process', 0);
  logicDataLen = ProcedurePool.build('logic', 2);
  names = [...this.cpusLen, ...this.processLen, ...this.freqLen];
  logicDataHandles = [...this.logicDataLen];

  onComplete: Function | undefined; //任务完成回调

  constructor(threadBuild: (() => ProcedureThread) | undefined = undefined) {
    this.init(threadBuild);
  }

  static build(name: string, len: number) {
    return [...Array(len).keys()].map((it) => `${name}${it}`);
  }

  init(threadBuild: (() => ProcedureThread) | undefined = undefined) {
    this.maxThreadNumber = this.names.length;
    for (let i = 0; i < this.maxThreadNumber; i++) {
      this.newThread();
    }
    for (let j = 0; j < this.logicDataHandles.length; j++) {
      this.logicDataThread();
    }
  }

  newThread() {
    // @ts-ignore
    if (window.useWb) {
      return;
    }
    let newThread: ProcedureThread = new ProcedureThread('trace/database/ui-worker/ProcedureWorker.js', {
      type: 'module',
    });
    newThread.name = this.names[this.works.length];
    newThread.onmessage = (event: MessageEvent) => {
      newThread.busy = false;
      if ((event.data.type as string) == 'timeline-range-changed') {
        this.timelineChange && this.timelineChange(event.data.results);
        newThread.busy = false;
        return;
      }
      if (Reflect.has(newThread.taskMap, event.data.id)) {
        if (event.data) {
          let fun = newThread.taskMap[event.data.id];
          if (fun) {
            fun(event.data.results, event.data.hover);
          }
          Reflect.deleteProperty(newThread.taskMap, event.data.id);
        }
      }
      if (this.isIdle() && this.onComplete) {
        this.onComplete();
      }
    };
    newThread.onmessageerror = (e) => {};
    newThread.onerror = (e) => {};
    newThread.id = this.works.length;
    newThread.busy = false;
    this.works?.push(newThread);
    return newThread;
  }

  logicDataThread() {
    // @ts-ignore
    if (window.useWb) {
      return;
    }
    let thread: ProcedureThread = new ProcedureThread('trace/database/logic-worker/ProcedureLogicWorker.js', {
      type: 'module',
    });
    thread.name = this.logicDataHandles[this.works.length - this.names.length];
    thread.onmessage = (event: MessageEvent) => {
      thread.busy = false;
      if (event.data.isQuery) {
        query(event.data.type, event.data.sql, event.data.args, 'exec-buf').then((res: any) => {
          thread.postMessage({
            type: event.data.type,
            params: {
              list: res,
            },
            id: event.data.id,
          });
        });
        return;
      }
      if (event.data.isSending) {
        if (Reflect.has(thread.taskMap, event.data.id)) {
          if (event.data) {
            let fun = thread.taskMap[event.data.id];
            if (fun) {
              fun(event.data.results, event.data.hover);
            }
            return;
          }
        }
      }
      if (Reflect.has(thread.taskMap, event.data.id)) {
        if (event.data) {
          let fun = thread.taskMap[event.data.id];
          if (fun) {
            fun(event.data.results, event.data.hover);
          }
          Reflect.deleteProperty(thread.taskMap, event.data.id);
        }
      }
      if (this.isIdle() && this.onComplete) {
        this.onComplete();
      }
    };
    thread.onmessageerror = (e) => {};
    thread.onerror = (e) => {};
    thread.id = this.works.length;
    thread.busy = false;
    this.works?.push(thread);
    return thread;
  }

  close = () => {
    for (let i = 0; i < this.works.length; i++) {
      let thread = this.works[i];
      thread.terminate();
    }
    this.works.length = 0;
  };

  clearCache = () => {
    for (let i = 0; i < this.works.length; i++) {
      let thread = this.works[i];
      thread.queryFunc('clear', {}, undefined, () => {});
    }
  };

  submitWithName(name: string, type: string, args: any, transfer: any, handler: Function): ProcedureThread | undefined {
    let noBusyThreads = this.works.filter((it) => it.name === name);
    let thread: ProcedureThread | undefined;
    if (noBusyThreads.length > 0) {
      //取第一个空闲的线程进行任务
      thread = noBusyThreads[0];
      thread!.queryFunc(type, args, transfer, handler);
    }
    return thread;
  }

  submitWithNamePromise(name: string, type: string, args: any, transfer: any): Promise<any> {
    return new Promise((resolve, reject) => {
      let noBusyThreads = this.works.filter((it) => it.name === name);
      let thread: ProcedureThread | undefined;
      if (noBusyThreads.length > 0) {
        //取第一个空闲的线程进行任务
        thread = noBusyThreads[0];
        thread!.queryFunc(type, args, transfer, (res: any, hover: any) => {
          resolve({
            res: res,
            hover: hover,
          });
        });
      }
    });
  }

  isIdle() {
    return this.works.every((it) => !it.busy);
  }
}

export const procedurePool = new ProcedurePool();
