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

import { DbPool } from './SqlLite.js';

class ConvertThread extends Worker {
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

  getConvertData(handler: (status: boolean, msg: string, results: Blob) => void) {
    this.busy = true;
    let id = this.uuid();
    this.taskMap[id] = (res: any) => {
      DbPool.sharedBuffer = res.buffer;
      handler(res.status, res.msg, res.results);
    };
    let pam = {
      id: id,
      action: 'getConvertData',
      buffer: DbPool.sharedBuffer!,
    };
    try {
      this.postMessage(pam, [DbPool.sharedBuffer!]);
    } catch (e: any) {}
  }
}

class ConvertPool {
  maxThreadNumber: number = 0;
  works: Array<ConvertThread> = [];
  progress: Function | undefined | null;
  static data: Array<string> = [];
  num = Math.floor(Math.random() * 10 + 1) + 20;
  init = async (type: string) => {
    // server
    await this.close();
    if (type === 'convert') {
      this.maxThreadNumber = 1;
    }
    for (let i = 0; i < this.maxThreadNumber; i++) {
      let thread: ConvertThread;
      if (type === 'convert') {
        thread = new ConvertThread('trace/database/ConvertTraceWorker.js');
      }
      thread!.onmessage = (event: MessageEvent) => {
        thread.busy = false;
        ConvertPool.data = event.data.results;
        if (Reflect.has(thread.taskMap, event.data.id)) {
          if (event.data.results) {
            let fun = thread.taskMap[event.data.id];
            if (fun) {
              fun(event.data);
            }
            Reflect.deleteProperty(thread.taskMap, event.data.id);
          } else {
            let fun = thread.taskMap[event.data.id];
            if (fun) {
              fun([]);
            }
            Reflect.deleteProperty(thread.taskMap, event.data.id);
          }
        }
      };
      thread!.onmessageerror = (e) => {};
      thread!.onerror = (e) => {};
      thread!.id = i;
      thread!.busy = false;
      this.works?.push(thread!);
    }
  };

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
      thread.getConvertData(() => {});
    }
  };

  // @ts-ignore
  submitWithName(
    name: string,
    handler: (status: boolean, msg: string, results: Blob) => void
  ): ConvertThread | undefined {
    let noBusyThreads = this.works;
    let thread: ConvertThread | undefined;
    if (noBusyThreads.length > 0) {
      //取第一个空闲的线程进行任务
      thread = noBusyThreads[0];
      thread!.getConvertData(handler);
    }
    return thread;
  }

  isIdle() {
    return this.works.every((it) => !it.busy);
  }
}

export const convertPool = new ConvertPool();
