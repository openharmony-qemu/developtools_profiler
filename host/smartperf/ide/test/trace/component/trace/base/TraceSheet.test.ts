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

  it('TraceSheet Test19', () => {
    let traceSheet = new TraceSheet();
    expect(traceSheet.initHtml()).toMatchInlineSnapshot(`
"
            <style>
                :host([mode='hidden']){
                    display: none;
                }
                :host{
                    display: block;
                    background-color: rebeccapurple;
                }
                .tabHeight{
                    height: 30vh;
                    background-color: var(--dark-background,#FFFFFF);
                }
            </style>
            <div id="container" style="border-top: 1px solid var(--dark-border1,#D5D5D5);">
                <lit-tabs id="tabs" position="top-left" activekey="1" mode="card" >
                    <div slot="right" style="margin: 0 10px; color: var(--dark-icon,#606060);display: flex;align-items: center;">
                        <div title="SO导入" id="import_div" style="width: 20px;height: 20px;display: flex;flex-direction: row;margin-right: 10px">
                            <input id="import-file" style="display: none;pointer-events: none" type="file" webkitdirectory>
                            <label style="width: 20px;height: 20px;cursor: pointer;" for="import-file">
                                <lit-icon id="import-btn" name="copy-csv" style="pointer-events: none" size="20">
                                </lit-icon>
                            </label>
                        </div>
                        <lit-icon title="下载数据" id="export-btn" name="import-so" style="font-weight: bold;cursor: pointer;margin-right: 10px" size="20">
                        </lit-icon>
                        <lit-icon title="最大化" id="max-btn" name="vertical-align-top" style="font-weight: bold;cursor: pointer;margin-right: 10px" size="20">
                        </lit-icon>
                        <lit-icon title="最小化" id="min-btn" name="down" style="font-weight: bold;cursor: pointer;" size="20">
                        </lit-icon>
                    </div>
                </lit-tabs>
            </div>"
`);
  });
});
