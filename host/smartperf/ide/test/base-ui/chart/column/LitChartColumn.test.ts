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
import '../../../../dist/base-ui/chart/column/LitChartColumn.js';
// @ts-ignore
import { getProbablyTime } from '../../../../dist/trace/database/logic-worker/ProcedureLogicWorkerCommon.js';

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
          pid: 11,
          pName: 'process01',
          tid: 332,
          tName: '11',
          total: 45,
          size: 'big core',
          timeStr: '91.11kb',
        },
        {
          pid: 21,
          pName: 'process02',
          tid: 21,
          tName: '222',
          total: 13,
          size: 'big core',
          timeStr: '211.00kb',
        },
      ],
      appendPadding: 10,
      xField: 'tid',
      yField: 'total',
      seriesField: 'total',
      color: (a: any) => {
        if (a.size === 'big core') {
          return '#2f72f8';
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
                                </div>
                            `;
          }
          tip = `<div>
                                        <div>tid:${a[0].obj.tid}</div>
                                    </div>`;
          return tip;
        } else {
          return '';
        }
      },
      label: null,
    };
    expect(clo.config).not.toBeUndefined();
    LitChartColumn.contains = jest.fn().mockResolvedValue(true);
    clo.dataSource = [
      {
        pid: 110,
        pName: 'process03',
        tid: 32,
        tName: '11',
        total: 121,
        size: 'big core',
        timeStr: '11.09kb',
      },
      {
        pid: 2,
        pName: 'process04',
        tid: 22,
        tName: 'thread',
        total: 131,
        size: 'big core',
        timeStr: '22.30kb',
      },
    ];
    expect(clo.data[0].obj.pid).toBe(2);
  });
  it('litChartColumnTest04', function () {
    let litChartColumn = new LitChartColumn();
    litChartColumn.litChartColumnTipEL = jest.fn(() => true);
    litChartColumn.litChartColumnTipEL.style = jest.fn(() => true);
    expect(litChartColumn.showTip(14, 5, 't')).toBeUndefined();
  });
  it('litChartColumnTest05', function () {
    let litChartColumn = new LitChartColumn();
    litChartColumn.litChartColumnTipEL = jest.fn(() => true);
    litChartColumn.litChartColumnTipEL.style = jest.fn(() => true);
    expect(litChartColumn.hideTip()).toBeUndefined();
  });
  it('litChartColumnTest06', function () {
    document.body.innerHTML = `
        <div>
            <lit-chart-column id='chart-cloumn'>小按钮</lit-chart-column>
        </div> `;
    let clo = document.getElementById('chart-cloumn') as LitChartColumn;
    let mouseOutEvent: MouseEvent = new MouseEvent('mouseout', <MouseEventInit>{ clientX: 1, clientY: 2 });
    clo.litChartColumnCanvas.dispatchEvent(mouseOutEvent);
  });
  it('litChartColumnTest07', function () {
    document.body.innerHTML = `
        <div>
            <lit-chart-column id='chart-cloumn'>小按钮</lit-chart-column>
        </div> `;
    let clo = document.getElementById('chart-cloumn') as LitChartColumn;
    let mouseOutEvent: MouseEvent = new MouseEvent('mousemove', <MouseEventInit>{ clientX: 1, clientY: 2 });
    clo.litChartColumnCanvas.dispatchEvent(mouseOutEvent);
  });
});
