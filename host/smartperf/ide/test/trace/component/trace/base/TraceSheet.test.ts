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
import { TraceSheet } from '../../../../../dist/trace/component/trace/base/TraceSheet.js';
const sqlit = require('../../../../../dist/trace/database/SqlLite.js');
jest.mock('../../../../../dist/trace/database/SqlLite.js');
const intersectionObserverMock = () => ({
  observe: () => null,
});
window.IntersectionObserver = jest.fn().mockImplementation(intersectionObserverMock);
window.ResizeObserver =
  window.ResizeObserver ||
  jest.fn().mockImplementation(() => ({
    disconnect: jest.fn(),
    observe: jest.fn(),
    unobserve: jest.fn(),
  }));

describe('TraceSheet Test', () => {
  beforeAll(() => {});
  let val = {
    hasFps: 1,
    cpus: { length: 1 },
    threadIds: [{ length: 2 }],
    funTids: { length: 1 },
    trackIds: { length: 1 },
    heapIds: { length: 1 },
    nativeMemory: { length: 1 },
    cpuAbilityIds: { length: 1 },
    memoryAbilityIds: { length: 1 },
    diskAbilityIds: { length: 1 },
    networkAbilityIds: { length: 1 },
  };
  let e = {
    detail: {
      title: 1,
      state: 0,
      threadId: 1,
      processId: 2,
    },
  };
  let selection = {
    hasFps: 1,
    cpus: { length: 1 },
    threadIds: [{ length: 2 }],
    funTids: { length: 1 },
    trackIds: { length: 1 },
    heapIds: { length: 1 },
    nativeMemory: { length: 1 },
    cpuAbilityIds: { length: 0 },
    memoryAbilityIds: { length: 0 },
    diskAbilityIds: { length: 0 },
    networkAbilityIds: { length: 0 },
    perfSampleIds: { length: 0 },
    processTrackIds: { length: 0 },
    fileSystemType: { length: 0 },
    virtualTrackIds: { length: 0 },
    sdkCounterIds: [
      {
        length: 0,
      },
    ],
    sdkSliceIds: [
      {
        length: 0,
      },
    ],
  };
  document.body.innerHTML = '<sp-system-trace style="visibility:visible;" id="sp-system-trace"></sp-system-trace>';
  it('TraceSheet Test01', () => {
    let traceSheet = new TraceSheet();
    expect(traceSheet).not.toBeUndefined();
  });

  it('TraceSheet Test08', () => {
    let traceSheet = new TraceSheet();
    expect(traceSheet.connectedCallback()).toBeUndefined();
  });
  it('TraceSheet Test09', () => {
    let traceSheet = new TraceSheet();
    expect(traceSheet.loadTabPaneData()).toBeUndefined();
  });

  it('TraceSheet Test10', () => {
    let traceSheet = new TraceSheet();
    expect(traceSheet.updateRangeSelect()).toBeFalsy();
  });
  it('TraceSheet Test11', () => {
    let traceSheet = new TraceSheet();
    expect(traceSheet.constructor()).toBeTruthy();
  });
  it('TraceSheet Test13', () => {
    let nativeHookResponseTypes = sqlit.queryNativeHookResponseTypes;
    let hookTypeData = [
      {
        lastLibId:33,
        value:'bc'
      },
    ];
    nativeHookResponseTypes.mockResolvedValue(hookTypeData);
    let traceSheet = new TraceSheet();
    let param = {
      leftNs: 0,
      rightNs: 1000,
      nativeMemory: ['All Heap & Anonymous VM', 'All Heap', 'Heap'],
    };
    expect(traceSheet.initFilterLibList(param)).toBeUndefined();
  });
});
