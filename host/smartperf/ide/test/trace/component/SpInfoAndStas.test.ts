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
import { SpInfoAndStats } from '../../../dist/trace/component/SpInfoAndStas.js';
const sqlit = require('../../../dist/trace/database/SqlLite.js');
jest.mock('../../../dist/trace/database/SqlLite.js');
window.ResizeObserver =
  window.ResizeObserver ||
  jest.fn().mockImplementation(() => ({
    disconnect: jest.fn(),
    observe: jest.fn(),
    unobserve: jest.fn(),
  }));

describe('SpInfoAndStasTest', () => {
  document.body.innerHTML = `<sp-info-and-stats id="ddd"></sp-info-and-stats>`;
  let spInfoAndStats = document.querySelector('#ddd') as SpInfoAndStats;
  it('SpInfoAndStasTest01', function () {
    let spInfoAndStats = new SpInfoAndStats();
    expect(spInfoAndStats.initElements()).toBeUndefined();
  });

  it('SpInfoAndStasTest04', function () {
    let spInfoAndStats = new SpInfoAndStats();
    expect(
      spInfoAndStats.initDataTableStyle({
        children: [
          {
            length: 1,
            style: {
              backgroundColor: 'var(--dark-background5,#F6F6F6)',
            },
          },
        ],
      })
    ).toBeUndefined();
  });

  it('SpInfoAndStasTest06 ', function () {
    expect(spInfoAndStats.connectedCallback()).toBeUndefined();
  });

  it('SpInfoAndStasTest07 ', function () {
    expect(spInfoAndStats.disconnectedCallback()).toBeUndefined();
  });

  it('SpInfoAndStasTest08 ', function () {
    expect(spInfoAndStats.attributeChangedCallback([], [], [])).toBeUndefined();
  });

  it('SpInfoAndStasTest10', function () {
    let traceMetaData = sqlit.queryTraceMetaData;
    let data = [
      {
        name: 'a',
        valueText: '',
      },
    ];
    traceMetaData.mockResolvedValue(data);
    let selectTraceMetaData = sqlit.querySelectTraceStats;
    let selectData = [
      {
        event_name: '',
        stat_type: '',
        count: 1,
        source: 10,
        serverity: 23,
      },
    ];
    selectTraceMetaData.mockResolvedValue(selectData);
    let spInfoAndStats = new SpInfoAndStats();
    expect(spInfoAndStats.initMetricItemData()).toBeTruthy();
  });
  it('SpInfoAndStasTest11', function () {
    expect(spInfoAndStats.initInfoAndStatsData()).toBeUndefined();
  });
});
