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
import { info } from '../../../log/Log.js';
import { LitTable } from '../../../base-ui/table/lit-table.js';
import '../../../base-ui/progress-bar/LitProgressBar.js';
import { LitProgressBar } from '../../../base-ui/progress-bar/LitProgressBar.js';
import { getDataNo } from './utils/Utils.js';
import './TableNoData.js';
import { TableNoData } from './TableNoData.js';

@element('tab-cpu-details-irq')
export class TabCpuDetailsIrq extends BaseElement {
  private tableNoData: TableNoData | null | undefined;
  private cpuDetailsLrqUsageTbl: LitTable | null | undefined;
  private cpuDetailsLrqProgress: LitProgressBar | null | undefined;
  traceChange: boolean = false;
  private cpuDetailsLrqPie: LitChartPie | null | undefined;
  private cpuDetailsLrqData: Array<any> = [];
  private cpuDetailsLrqSortColumn: string = '';
  private sortType: number = 0;

  initElements(): void {
    this.tableNoData = this.shadowRoot!.querySelector<TableNoData>('#table-no-data');
    this.cpuDetailsLrqProgress = this.shadowRoot!.querySelector<LitProgressBar>('#loading');
    this.cpuDetailsLrqPie = this.shadowRoot!.querySelector<LitChartPie>('#chart-pie');
    this.cpuDetailsLrqUsageTbl = this.shadowRoot!.querySelector<LitTable>('#tb-cpu-irq');

    this.cpuDetailsLrqUsageTbl!.addEventListener('row-click', (evt: any) => {
      // @ts-ignore
      let data = evt.detail.data;
      data.isSelected = true;
      // @ts-ignore
      if ((evt.detail as any).callBack) {
        // @ts-ignore
        (evt.detail as any).callBack(true);
      }
    });

    this.cpuDetailsLrqUsageTbl!.addEventListener('column-click', (evt: any) => {
      this.cpuDetailsLrqSortColumn = evt.detail.key;
      this.sortType = evt.detail.sort;
      // @ts-ignore
      this.sortByColumn(evt.detail);
    });
    this.cpuDetailsLrqUsageTbl!.addEventListener('row-hover', (evt: any) => {
      if (evt.detail.data) {
        let data = evt.detail.data;
        data.isHover = true;
        if ((evt.detail as any).callBack) {
          (evt.detail as any).callBack(true);
        }
      }
      this.cpuDetailsLrqPie?.showHover();
    });
  }

  init(cpu: number) {
    this.queryPieChartDataByType('CPU Irq', cpu);
  }

  queryPieChartDataByType(type: string, cpu: number) {
    if (this.traceChange) {
      return;
    }
    this.cpuDetailsLrqProgress!.loading = true;
    this.queryLoginWorker(`scheduling-${type}`, 'query Cpu Frequency Analysis Time:', (res) => {
      this.traceChange = true;
      this.cpuDetailsLrqProgress!.loading = false;
      this.cpuDetailsLrqData = res.get(cpu) || [];
      this.cpuDetailsLrqData = getDataNo(this.cpuDetailsLrqData);
      this.tableNoData!.noData = this.cpuDetailsLrqData.length == 0;
      this.noData(this.cpuDetailsLrqData.length == 0);
      this.cpuDetailsLrqPie!.config = {
        appendPadding: 0,
        data: this.cpuDetailsLrqData,
        angleField: 'sum',
        colorField: 'value',
        radius: 1,
        label: {
          type: 'outer',
        },
        tip: (irqObj) => {
          return `<div>
                                <div>block:${irqObj.obj.block}</div> 
                                <div>name:${irqObj.obj.value}</div>
                                <div>min:${irqObj.obj.min}</div>
                                <div>max:${irqObj.obj.max}</div>
                                <div>average:${irqObj.obj.avg}</div>
                                <div>duration:${irqObj.obj.sumTimeStr}</div>
                                <div>ratio:${irqObj.obj.ratio}%</div>
                            </div>
                                `;
        },
        hoverHandler: (data) => {
          if (data) {
            this.cpuDetailsLrqUsageTbl!.setCurrentHover(data);
          } else {
            this.cpuDetailsLrqUsageTbl!.mouseOut();
          }
        },
        interactions: [
          {
            type: 'element-active',
          },
        ],
      };
      if (this.cpuDetailsLrqSortColumn != '') {
        this.sortByColumn({
          key: this.cpuDetailsLrqSortColumn,
          sort: this.sortType,
        });
      } else {
        this.cpuDetailsLrqUsageTbl!.recycleDataSource = this.cpuDetailsLrqData;
      }
      this.cpuDetailsLrqUsageTbl?.reMeauseHeight();
    });
  }

  noData(value: boolean) {
    this.shadowRoot!.querySelector<HTMLDivElement>('.irq-chart-box')!.style.display = value ? 'none' : 'block';
    this.shadowRoot!.querySelector<HTMLDivElement>('.table-box')!.style.width = value ? '100%' : '60%';
  }

  clearData() {
    this.traceChange = false;
    this.cpuDetailsLrqPie!.dataSource = [];
    this.cpuDetailsLrqUsageTbl!.recycleDataSource = [];
    this.noData(false);
  }

  queryLoginWorker(irqType: string, log: string, handler: (res: any) => void) {
    let cpuDetailsLrqTime = new Date().getTime();
    procedurePool.submitWithName(
      'logic1',
      irqType,
      {
        endTs: SpSchedulingAnalysis.endTs,
        total: SpSchedulingAnalysis.totalDur,
      },
      undefined,
      handler
    );
    let durTime = new Date().getTime() - cpuDetailsLrqTime;
    info(log, durTime);
  }

  sortByColumn(detail: any) {
    // @ts-ignore
    function compare(cpuDetailsLrqProperty, sort, type) {
      return function (a: any, b: any) {
        if (type === 'number') {
          // @ts-ignore
          return sort === 2
            ? parseFloat(b[cpuDetailsLrqProperty]) - parseFloat(a[cpuDetailsLrqProperty])
            : parseFloat(a[cpuDetailsLrqProperty]) - parseFloat(b[cpuDetailsLrqProperty]);
        } else {
          if (sort === 2) {
            return b[cpuDetailsLrqProperty].toString().localeCompare(a[cpuDetailsLrqProperty].toString());
          } else {
            return a[cpuDetailsLrqProperty].toString().localeCompare(b[cpuDetailsLrqProperty].toString());
          }
        }
      };
    }

    if (detail.key === 'min') {
      detail.key = 'minValue';
      this.cpuDetailsLrqData.sort(compare(detail.key, detail.sort, 'number'));
    } else if (detail.key === 'max') {
      detail.key = 'maxValue';
      this.cpuDetailsLrqData.sort(compare(detail.key, detail.sort, 'number'));
    } else if (detail.key === 'avg') {
      detail.key = 'avgValue';
      this.cpuDetailsLrqData.sort(compare(detail.key, detail.sort, 'number'));
    } else if (detail.key === 'sumTimeStr') {
      detail.key = 'sum';
      this.cpuDetailsLrqData.sort(compare(detail.key, detail.sort, 'number'));
    } else if (detail.key === 'ratio' || detail.key === 'index') {
      this.cpuDetailsLrqData.sort(compare(detail.key, detail.sort, 'number'));
    } else {
      this.cpuDetailsLrqData.sort(compare(detail.key, detail.sort, 'string'));
    }
    this.cpuDetailsLrqUsageTbl!.recycleDataSource = this.cpuDetailsLrqData;
  }

  initHtml(): string {
    return `
        <style>
        .irq-box{
            display: flex;
            margin: 20px;
            height: calc(100vh - 165px);
        }
        .irq-chart-box{
            width: 40%;
        }
        #tb-cpu-irq{
            height: 100%;
        }
        .table-box{
            border: solid 1px var(--dark-border1,#e0e0e0);
            border-radius: 5px;
            width: 60%;
            max-height: calc(100vh - 165px);
            padding: 10px;
        }
        :host {
            width: 100%;
            height: 100%;
            background-color: var(--dark-background,#FFFFFF);
        }
        #chart-pie{
            height: 360px;
        }
        </style>
        <lit-progress-bar id="loading" style="height: 1px;width: 100%"></lit-progress-bar>
        <div class="irq-box">
            <div class="irq-chart-box">
                <div style="text-align: center">Statistics By Duration</div>
                <lit-chart-pie  id="chart-pie"></lit-chart-pie>
            </div>
            <div class="table-box">
                <table-no-data id="table-no-data">
                    <lit-table id="tb-cpu-irq" hideDownload>
                        <lit-table-column width="100px" title="No" data-index="index" key="index" align="flex-start" order></lit-table-column>
                        <lit-table-column width="150px" title="block" data-index="block" key="block" align="flex-start" order></lit-table-column>
                        <!--<lit-table-column width="100px" title="id" data-index="id" key="id" align="flex-start" order></lit-table-column>-->
                        <lit-table-column title="name" data-index="value" key="value" align="flex-start" order width="150px"></lit-table-column>
                        <lit-table-column title="min" data-index="min" key="min" align="flex-start" order width="100px"></lit-table-column>
                        <lit-table-column title="max" data-index="max" key="max" align="flex-start" order width="100px"></lit-table-column>
                        <lit-table-column title="average" data-index="avg" key="avg" align="flex-start" order width="100px"></lit-table-column>
                        <lit-table-column title="duration" data-index="sumTimeStr" key="sumTimeStr" align="flex-start" order width="100px"></lit-table-column>
                        <lit-table-column title="%" data-index="ratio" key="ratio" align="flex-start" order width="100px"></lit-table-column>
                    </lit-table>
                </table-no-data>
            </div>
        </div>
        `;
  }
}
