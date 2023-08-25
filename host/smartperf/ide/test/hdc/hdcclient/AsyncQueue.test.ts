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
// @ts-ignore
import { AsyncQueue, DataMessageQueue } from '../../../dist/hdc/hdcclient/AsyncQueue.js';

describe('AsyncQueueTest', () => {
  let queue = new DataMessageQueue();
  let dataMessageQueue = new DataMessageQueue();
  let asqueue = new AsyncQueue();
  it('AsyncQueueTest_DataMessageQueue_01', () => {
    expect(queue.push('abc')).toEqual(true);
  });

  it('AsyncQueueTest_DataMessageQueue_02', () => {
    expect(queue.size()).toEqual(1);
  });

  it('AsyncQueueTest_DataMessageQueue_03', () => {
    expect(queue.pop()).toEqual('abc');
  });

  it('AsyncQueueTest_DataMessageQueue_04', () => {
    expect(queue.push(null)).toEqual(false);
  });

  it('AsyncQueueTest_AsyncQueue_01', () => {
    let dataMessageQueue = new DataMessageQueue();
    dataMessageQueue.push('aaa');
    asqueue.enqueue(dataMessageQueue);
    expect(asqueue.dequeue()).toBeTruthy();
  });
});
