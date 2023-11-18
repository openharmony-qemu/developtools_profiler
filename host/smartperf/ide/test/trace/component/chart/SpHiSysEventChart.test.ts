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
import { SpHiSysEventChart } from '../../../../dist/trace/component/chart/SpHiSysEventChart.js';
// @ts-ignore
import { SpChartManager } from '../../../../dist/trace/component/chart/SpChartManager.js';
const sqlite = require('../../../../dist/trace/database/SqlLite.js');
jest.mock('../../../../dist/trace/database/SqlLite.js');
jest.mock('../../../../dist/trace/database/ui-worker/ProcedureWorker.js', () => {
  return {};
});

describe('SpHiSysEventChart Test', () => {
  let spHiSysEvent = new SpHiSysEventChart(new SpChartManager());
  let hiSysEventList =sqlite.queryHiSysEventData;
  let hiSysEventListData = [{
    id: 1,
    domain:'STARTUP',
    eventName: 'PROCESS_EXIT',
    eventType: '4',
    ts: 1,
    tz: 'dad',
    pid: 1,
    tid: 1,
    uid: 1,
    info: '',
    level: 'MINOR',
    seq: '92860',
    contents: 'APP_PID',
    dur: 1,
    depth: 1,
  }]
  hiSysEventList.mockResolvedValue(hiSysEventListData);
  it('SpHiSysEventChart01', function () {
    expect(spHiSysEvent.init()).toBeTruthy();
  });
});
