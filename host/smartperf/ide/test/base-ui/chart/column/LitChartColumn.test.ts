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
import { LitChartColumn } from '../../../../dist/base-ui/chart/column/LitChartColumn.js';
// @ts-ignore
import { getProbablyTime } from '../../../../dist/trace/database/logic-worker/ProcedureLogicWorkerCommon.js';

// @ts-ignore
jest.mock('../../../../dist/base-ui/chart/column/LitChartColumn.js');
window.ResizeObserver =
  window.ResizeObserver ||
  jest.fn().mockImplementation(() => ({
    disconnect: jest.fn(),
    observe: jest.fn(),
    unobserve: jest.fn(),
  }));

const maybeHandler = jest.fn();

describe('litChartColumn Test', () => {
  it('litChartColumnTest01', function () {
    let litChartColumn = new LitChartColumn();
    expect(litChartColumn).not.toBeUndefined();
  });

  it('litChartColumnTest03', function () {
    document.body.innerHTML = `
        <div>
            <lit-chart-column id='chart-cloumn'>小按钮</lit-chart-column>
        </div> `;
    let clo = document.getElementById('chart-cloumn') as LitChartColumn;
    clo.config = {
      data: [
        {
          pid: 1,
          pName: '1',
          tid: 1,
          tName: '11',
          total: 12,
          size: 'big core',
          timeStr: '11',
        },
        {
          pid: 2,
          pName: '2',
          tid: 2,
          tName: '222',
          total: 13,
          size: 'big core',
          timeStr: '22',
        },
      ],
      appendPadding: 10,
      xField: 'tid',
      yField: 'total',
      seriesField: 'total',
      color: (a: any) => {
        if (a.size === 'big core') {
          return '#2f72f8';
        } else if (a.size === 'middle core') {
          return '#ffab67';
        } else if (a.size === 'small core') {
          return '#a285d2';
        } else {
          return '#0a59f7';
        }
      },
      tip: (a: any) => {
        if (a && a[0]) {
          let tip = '';
          let total = 0;
          for (let obj of a) {
            total += obj.obj.total;
            tip = `${tip}
                                <div style="display:flex;flex-direction: row;align-items: center;">
                                    <div style="width: 10px;height: 5px;background-color: ${obj.color};margin-right: 5px"></div>
                                    <div>${obj.type}:${obj.obj.timeStr}</div>
                                </div>
                            `;
          }
          tip = `<div>
                                        <div>tid:${a[0].obj.tid}</div>
                                        ${tip}
                                        ${a.length > 1 ? `<div>total:${getProbablyTime(total)}</div>` : ''}
                                    </div>`;
          return tip;
        } else {
          return '';
        }
      },
      label: null,
    };
    let mouseOutEvent: MouseEvent = new MouseEvent('mouseout', <MouseEventInit>{ movementX: 1, movementY: 2 });
    expect(clo.config).not.toBeUndefined();
    LitChartColumn.contains = jest.fn().mockResolvedValue(true);

    clo.dataSource = [
      {
        pid: 1,
        pName: '1',
        tid: 1,
        tName: '11',
        total: 12,
        size: 'big core',
        timeStr: '11',
      },
      {
        pid: 2,
        pName: '2',
        tid: 2,
        tName: '222',
        total: 13,
        size: 'big core',
        timeStr: '22',
      },
    ];
    expect(clo.data[0].obj.pid).toBe(2);
  });
});
