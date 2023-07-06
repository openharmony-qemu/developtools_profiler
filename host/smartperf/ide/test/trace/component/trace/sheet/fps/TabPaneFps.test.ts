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
//@ts-ignore
import { TabPaneFps } from '../../../../../../dist/trace/component/trace/sheet/fps/TabPaneFps.js';
const sqlit = require('../../../../../../dist/trace/database/SqlLite.js');
jest.mock('../../../../../../dist/trace/database/SqlLite.js');

window.ResizeObserver =
  window.ResizeObserver ||
  jest.fn().mockImplementation(() => ({
    disconnect: jest.fn(),
    observe: jest.fn(),
    unobserve: jest.fn(),
  }));

describe('TabPaneFps Test', () => {
  let tabPaneFps = new TabPaneFps();

  it('TabPaneFpsTest01', function () {
    let mockgetTabFps = sqlit.getTabFps;
    mockgetTabFps.mockResolvedValue([{ leftNs: 10, rightNs: 10 }]);
    let a = { rightNs: 1, leftNs: 1 };
    expect((tabPaneFps.data = a)).toBeTruthy();
  });

  it('TabPaneFpsTest02', function () {
    let mockgetTabFps = sqlit.getTabFps;
    mockgetTabFps.mockResolvedValue([]);
    let a = { rightNs: 1, leftNs: 1 };
    expect((tabPaneFps.data = a)).toBeTruthy();
  });

  it('TabPaneFpsTest03', function () {
    expect(tabPaneFps.initHtml()).toMatchInlineSnapshot(`
"
        <style>
        .fps-label{
            text-align: end;
        }
        :host{
            display: flex;
            flex-direction: column;
            padding: 10px 10px;
        }
        </style>
        <label id="fps-time-range" class="fps-label" style="width: 100%;height: 20px;font-size: 10pt;margin-bottom: 5px">Selected range:0.0 ms</label>
        <lit-table id="tb-fps" style="height: auto">
            <lit-table-column class="fps-column" width="1fr" title="Time" data-index="timeStr" key="timeStr" align="flex-start">
            </lit-table-column>
            <lit-table-column class="fps-column" width="1fr" title="FPS" data-index="fps" key="fps" align="flex-start" >
            </lit-table-column>
        </lit-table>
        "
`);
  });
});
