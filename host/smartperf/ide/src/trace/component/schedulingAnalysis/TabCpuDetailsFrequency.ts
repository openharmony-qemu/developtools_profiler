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

import { BaseElement, element } from '../../../base-ui/BaseElement.js';
import { LitChartPie } from '../../../base-ui/chart/pie/LitChartPie.js';
import { procedurePool } from '../../database/Procedure.js';
import { SpSchedulingAnalysis } from './SpSchedulingAnalysis.js';
import { TabCpuDetailsThreads } from './TabCpuDetailsThreads.js';
import './TabCpuDetailsThreads.js';
import { info } from '../../../log/Log.js';
import { LitTable } from '../../../base-ui/table/lit-table.js';
import { getDataNo } from './utils/Utils.js';
import '../../../base-ui/progress-bar/LitProgressBar.js';
import { LitProgressBar } from '../../../base-ui/progress-bar/LitProgressBar.js';
import './TableNoData.js';
import { TableNoData } from './TableNoData.js';

@element('tab-cpu-details-frequency')
export class TabCpuDetailsFrequency extends BaseElement {
  private tableNoData: TableNoData | null | undefined;
  private cpuDetailsFrequencyProgress: LitProgressBar | null | undefined;
  traceChange: boolean = false;
  private cpuDetailsFrequencyPie: LitChartPie | null | undefined;
  private cpuDetailsFrequencyUsageTbl: LitTable | null | undefined;
  private tabCpuDetailsThreads: TabCpuDetailsThreads | null | undefined;
  private cpu: number = 0;
  private cpuDetailsFrequencyData: Array<any> = [];
  private cpuDetailsFrequencySortColumn: string = '';
  private sortType: number = 0;

  initElements(): void {
    this.tableNoData = this.shadowRoot!.querySelector<TableNoData>('#table-no-data');
    this.cpuDetailsFrequencyProgress = this.shadowRoot!.querySelector<LitProgressBar>('#loading');
    this.cpuDetailsFrequencyPie = this.shadowRoot!.querySelector<LitChartPie>('#chart-pie');
    this.cpuDetailsFrequencyUsageTbl = this.shadowRoot!.querySelector<LitTable>('#fre-tb-cpu-usage');
    this.tabCpuDetailsThreads = this.shadowRoot!.querySelector<TabCpuDetailsThreads>('#tab-cpu-details-threads');

    this.cpuDetailsFrequencyUsageTbl!.addEventListener('row-click', (evt: any) => {
      let data = evt.detail.data;
      data.isSelected = true;
      // @ts-ignore
      if ((evt.detail as any).callBack) {
        // @ts-ignore
        (evt.detail as any).callBack(true);
      }
    });

    this.cpuDetailsFrequencyUsageTbl!.addEventListener('column-click', (evt: any) => {
      this.cpuDetailsFrequencySortColumn = evt.detail.key;
      this.sortType = evt.detail.sort;
      // @ts-ignore
      this.sortByColumn(evt.detail);
    });
    this.cpuDetailsFrequencyUsageTbl!.addEventListener('row-hover', (evt: any) => {
      if (evt.detail.data) {
        let data = evt.detail.data;
        data.isHover = true;
        if ((evt.detail as any).callBack) {
          (evt.detail as any).callBack(true);
        }
      }
      this.cpuDetailsFrequencyPie?.showHover();
    });
  }

  init(cpu: number) {
    this.cpu = cpu;
    this.queryPieChartDataByType('CPU Frequency', cpu);
  }

  queryPieChartDataByType(type: string, cpu: number) {
    if (this.traceChange) {
      return;
    }
    this.cpuDetailsFrequencyProgress!.loading = true;
    this.queryLoginWorker(`scheduling-${type}`, 'query Cpu Frequency Analysis Time:', (res) => {
      this.traceChange = true;
      this.cpuDetailsFrequencyProgress!.loading = false;
      this.cpuDetailsFrequencyData = res.get(cpu) || [];
      this.cpuDetailsFrequencyData = getDataNo(this.cpuDetailsFrequencyData);
      this.tableNoData!.noData = this.cpuDetailsFrequencyData.length == 0;
      this.noData(this.cpuDetailsFrequencyData.length == 0);
      this.cpuDetailsFrequencyPie!.config = {
        appendPadding: 0,
        data: this.cpuDetailsFrequencyData,
        angleField: 'sum',
        colorField: 'value',
        radius: 1,
        label: {
          type: 'outer',
        },
        tip: (freObj) => {
          return `<div>
                                <div>frequency:${freObj.obj.value}</div> 
                                <div>min:${freObj.obj.min}</div>
                                <div>max:${freObj.obj.max}</div>
                                <div>average:${freObj.obj.avg}</div>
                                <div>duration:${freObj.obj.sumTimeStr}</div>
                                <div>ratio:${freObj.obj.ratio}%</div>
                            </div>
                                `;
        },
        hoverHandler: (cpuDetailsFreqData) => {
          if (cpuDetailsFreqData) {
            this.cpuDetailsFrequencyUsageTbl!.setCurrentHover(cpuDetailsFreqData);
          } else {
            this.cpuDetailsFrequencyUsageTbl!.mouseOut();
          }
        },
        angleClick: (it) => {
          this.tabCpuDetailsThreads!.setShow = true;
          this.shadowRoot!.querySelector<HTMLDivElement>('.d-box')!.style.display = 'none';
          this.tabCpuDetailsThreads!.init(cpu, it);
        },
        interactions: [
          {
            type: 'element-active',
          },
        ],
      };
      if (this.cpuDetailsFrequencySortColumn != '') {
        this.sortByColumn({
          key: this.cpuDetailsFrequencySortColumn,
          sort: this.sortType,
        });
      } else {
        this.cpuDetailsFrequencyUsageTbl!.recycleDataSource = this.cpuDetailsFrequencyData;
      }
      this.cpuDetailsFrequencyUsageTbl?.reMeauseHeight();
    });
  }

  noData(value: boolean) {
    this.shadowRoot!.querySelector<HTMLDivElement>('.fre-chart-box')!.style.display = value ? 'none' : 'block';
    this.shadowRoot!.querySelector<HTMLDivElement>('.table-box')!.style.width = value ? '100%' : '60%';
  }

  clearData() {
    this.traceChange = false;
    this.cpuDetailsFrequencyPie!.dataSource = [];
    this.cpuDetailsFrequencyUsageTbl!.recycleDataSource = [];
    this.shadowRoot!.querySelector<HTMLDivElement>('.d-box')!.style.display = 'flex';
    this.tabCpuDetailsThreads!.setShow = false;
    this.noData(false);
  }

  set setShow(v: boolean) {
    if (v) {
      this.shadowRoot!.querySelector<HTMLDivElement>('.d-box')!.style.display = 'flex';
    } else {
      this.shadowRoot!.querySelector<HTMLDivElement>('.d-box')!.style.display = 'none';
    }
  }

  queryLoginWorker(cpuFrequencyType: string, log: string, handler: (res: any) => void) {
    let cpuDetailsFrequencyTime = new Date().getTime();
    procedurePool.submitWithName(
      'logic1',
      cpuFrequencyType,
      {
        endTs: SpSchedulingAnalysis.endTs,
        total: SpSchedulingAnalysis.totalDur,
      },
      undefined,
      handler
    );
    let durTime = new Date().getTime() - cpuDetailsFrequencyTime;
    info(log, durTime);
  }

  sortByColumn(detail: any) {
    // @ts-ignore
    function compare(cpuDetailsFrequencyProperty, sort, type) {
      return function (a: any, b: any) {
        if (type === 'number') {
          // @ts-ignore
          return sort === 2
            ? parseFloat(b[cpuDetailsFrequencyProperty]) - parseFloat(a[cpuDetailsFrequencyProperty])
            : parseFloat(a[cpuDetailsFrequencyProperty]) - parseFloat(b[cpuDetailsFrequencyProperty]);
        } else {
          if (sort === 2) {
            return b[cpuDetailsFrequencyProperty].toString().localeCompare(a[cpuDetailsFrequencyProperty].toString());
          } else {
            return a[cpuDetailsFrequencyProperty].toString().localeCompare(b[cpuDetailsFrequencyProperty].toString());
          }
        }
      };
    }

    if (detail.key === 'min') {
      detail.key = 'minValue';
      this.cpuDetailsFrequencyData.sort(compare(detail.key, detail.sort, 'number'));
    } else if (detail.key === 'max') {
      detail.key = 'maxValue';
      this.cpuDetailsFrequencyData.sort(compare(detail.key, detail.sort, 'number'));
    } else if (detail.key === 'avg') {
      detail.key = 'avgValue';
      this.cpuDetailsFrequencyData.sort(compare(detail.key, detail.sort, 'number'));
    } else if (detail.key === 'sumTimeStr') {
      detail.key = 'sum';
      this.cpuDetailsFrequencyData.sort(compare(detail.key, detail.sort, 'number'));
    } else if (detail.key === 'value' || detail.key === 'ratio' || detail.key === 'index') {
      this.cpuDetailsFrequencyData.sort(compare(detail.key, detail.sort, 'number'));
    } else {
      this.cpuDetailsFrequencyData.sort(compare(detail.key, detail.sort, 'string'));
    }
    this.cpuDetailsFrequencyUsageTbl!.recycleDataSource = this.cpuDetailsFrequencyData;
  }

  initHtml(): string {
    return `
        <style>
        #loading{
            height: 1px;
            width: 100%
        }
        :host {
            width: 100%;
            height: 100%;
            background-color: var(--dark-background,#FFFFFF);
        }
        .d-box{
            display: flex;
            margin: 20px;
            height: calc(100vh - 165px);
        }
        .fre-chart-box{
            width: 40%;
        }
        #fre-tb-cpu-usage{
            height: 100%;
        }
        .table-box{
            width: 60%;
            max-height: calc(100vh - 165px);
            padding: 10px;
            border: solid 1px var(--dark-border1,#e0e0e0);
            border-radius: 5px;
        }
        #chart-pie{
            height: 360px;
        }
        </style>
        <lit-progress-bar id="loading"></lit-progress-bar>
        <div class="d-box">
            <div class="fre-chart-box">
                <div style="text-align: center">Statistics By Duration</div>
                <lit-chart-pie  id="chart-pie"></lit-chart-pie>
            </div>
            <div class="table-box">
                <table-no-data id="table-no-data">
                    <lit-table id="fre-tb-cpu-usage" hideDownload>
                        <lit-table-column width="100px" title="No" data-index="index" key="index" align="flex-start" order></lit-table-column>
                        <lit-table-column width="150px" title="frequency" data-index="value" key="value" align="flex-start" order></lit-table-column>
                        <lit-table-column width="100px" title="min" data-index="min" key="min" align="flex-start" order></lit-table-column>
                        <lit-table-column width="100px" title="max" data-index="max" key="max" align="flex-start" order></lit-table-column>
                        <lit-table-column width="100px" title="average" data-index="avg" key="avg" align="flex-start" order></lit-table-column>
                        <lit-table-column width="100px" title="duration" data-index="sumTimeStr" key="sumTimeStr" align="flex-start" order></lit-table-column>
                        <lit-table-column width="100px" title="%" data-index="ratio" key="ratio" align="flex-start" order></lit-table-column>
                    </lit-table>
                </table-no-data>
            </div>
        </div>
        <tab-cpu-details-threads id="tab-cpu-details-threads"></tab-cpu-details-threads>
        `;
  }
}
