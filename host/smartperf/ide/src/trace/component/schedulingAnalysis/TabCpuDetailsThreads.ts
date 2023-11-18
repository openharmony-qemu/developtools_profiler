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
import { LitTable } from '../../../base-ui/table/lit-table.js';
import { LitProgressBar } from '../../../base-ui/progress-bar/LitProgressBar.js';
import '../../../base-ui/progress-bar/LitProgressBar.js';
import { getDataNo } from './utils/Utils.js';
import './TableNoData.js';
import { TableNoData } from './TableNoData.js';

@element('tab-cpu-details-threads')
export class TabCpuDetailsThreads extends BaseElement {
  private tableNoData: TableNoData | null | undefined;
  private cpuDetailsThreadUsageTbl: LitTable | null | undefined;
  private progress: LitProgressBar | null | undefined;
  private cpuDetailsThreadPie: LitChartPie | null | undefined;
  private data: Array<any> = [];
  private cpuDetailsThreadSortColumn: string = '';
  private sortType: number = 0;

  initElements(): void {
    this.tableNoData = this.shadowRoot!.querySelector<TableNoData>('#table-no-data');
    this.progress = this.shadowRoot!.querySelector<LitProgressBar>('#loading');
    this.cpuDetailsThreadPie = this.shadowRoot!.querySelector<LitChartPie>('#cpu-thread-chart-pie');
    this.cpuDetailsThreadUsageTbl = this.shadowRoot!.querySelector<LitTable>('#tb-cpu-usage');

    this.shadowRoot!.querySelector<HTMLDivElement>('.cpu-thread-go-back')!.onclick = (e) => {
      if (!this.progress!.loading) {
        this.parentNode!.querySelector<HTMLDivElement>('.d-box')!.style.display = 'flex';
        this.setShow = false;
      }
    };

    this.cpuDetailsThreadUsageTbl!.addEventListener('row-click', (evt: any) => {
      // @ts-ignore
      let data = evt.detail.data;
      data.isSelected = true;
      // @ts-ignore
      if ((evt.detail as any).callBack) {
        // @ts-ignore
        (evt.detail as any).callBack(true);
      }
    });

    this.cpuDetailsThreadUsageTbl!.addEventListener('column-click', (evt: any) => {
      this.cpuDetailsThreadSortColumn = evt.detail.key;
      this.sortType = evt.detail.sort;
      // @ts-ignore
      this.sortByColumn(evt.detail);
    });
    this.cpuDetailsThreadUsageTbl!.addEventListener('row-hover', (evt: any) => {
      if (evt.detail.data) {
        let data = evt.detail.data;
        data.isHover = true;
        if ((evt.detail as any).callBack) {
          (evt.detail as any).callBack(true);
        }
      }
      this.cpuDetailsThreadPie?.showHover();
    });
  }

  init(cpu: number, it: any) {
    this.shadowRoot!.querySelector<HTMLDivElement>('.cpu-thread-subheading')!.textContent = 'Threads in Freq ' + it.value;
    this.progress!.loading = true;
    procedurePool.submitWithName(
      'logic1',
      'scheduling-CPU Frequency Thread',
      { cpu: cpu, freq: (it as any).value },
      undefined,
      (res: any) => {
        this.progress!.loading = false;
        this.queryPieChartDataByType(res);
      }
    );
  }

  set setShow(v: boolean) {
    if (v) {
      this.style.display = 'flex';
    } else {
      this.clearData();
      this.style.display = 'none';
    }
  }

  queryPieChartDataByType(res: any) {
    this.data = res || [];
    this.data = getDataNo(this.data);
    this.tableNoData!.noData = this.data.length == 0;
    this.noData(this.data.length == 0);
    this.cpuDetailsThreadPie!.config = {
      appendPadding: 0,
      data: this.data,
      angleField: 'dur',
      colorField: 'tName',
      radius: 1,
      label: {
        type: 'outer',
      },
      tip: (obj) => {
        return `<div>
                                <div>t_name:${obj.obj.tName}</div> 
                                <div>tid:${obj.obj.tid}</div>
                                <div>p_name:${obj.obj.pName}</div>
                                <div>p_pid:${obj.obj.pid}</div>
                                <div>duration:${obj.obj.durStr}</div>
                                <div>ratio:${obj.obj.ratio}%</div>
                            </div>
                                `;
      },
      hoverHandler: (data) => {
        if (data) {
          this.cpuDetailsThreadUsageTbl!.setCurrentHover(data);
        } else {
          this.cpuDetailsThreadUsageTbl!.mouseOut();
        }
      },
      interactions: [
        {
          type: 'element-active',
        },
      ],
    };
    if (this.cpuDetailsThreadSortColumn != '') {
      this.sortByColumn({ key: this.cpuDetailsThreadSortColumn, sort: this.sortType });
    } else {
      this.cpuDetailsThreadUsageTbl!.recycleDataSource = this.data;
    }
    this.cpuDetailsThreadUsageTbl?.reMeauseHeight();
  }

  noData(value: boolean) {
    this.shadowRoot!.querySelector<HTMLDivElement>('.cpu-thread-chart-box')!.style.display = value ? 'none' : 'block';
    this.shadowRoot!.querySelector<HTMLDivElement>('.cpu-thread-table-box')!.style.width = value ? '100%' : '60%';
  }

  clearData() {
    this.cpuDetailsThreadPie!.dataSource = [];
    this.cpuDetailsThreadUsageTbl!.recycleDataSource = [];
    this.noData(false);
  }

  sortByColumn(detail: any) {
    // @ts-ignore
    function compare(cpuDetailsThreadProperty, sort, type) {
      return function (a: any, b: any) {
        if (type === 'number') {
          // @ts-ignore
          return sort === 2
            ? parseFloat(b[cpuDetailsThreadProperty]) - parseFloat(a[cpuDetailsThreadProperty])
            : parseFloat(a[cpuDetailsThreadProperty]) - parseFloat(b[cpuDetailsThreadProperty]);
        } else {
          if (sort === 2) {
            return b[cpuDetailsThreadProperty].toString().localeCompare(a[cpuDetailsThreadProperty].toString());
          } else {
            return a[cpuDetailsThreadProperty].toString().localeCompare(b[cpuDetailsThreadProperty].toString());
          }
        }
      };
    }

    if (detail.key === 'durStr') {
      detail.key = 'dur';
      this.data.sort(compare(detail.key, detail.sort, 'number'));
    } else if (
      detail.key === 'value' ||
      detail.key === 'ratio' ||
      detail.key === 'index' ||
      detail.key === 'tid' ||
      detail.key === 'pid'
    ) {
      this.data.sort(compare(detail.key, detail.sort, 'number'));
    } else {
      this.data.sort(compare(detail.key, detail.sort, 'string'));
    }
    this.cpuDetailsThreadUsageTbl!.recycleDataSource = this.data;
  }

  initHtml(): string {
    return `
        <style>
        :host {
            width: 100%;
            height: 100%;
            background-color: var(--dark-background,#FFFFFF);
            display: none;
        }
        .cpu-thread-d-box{
            display: flex;
            margin: 20px;
            height: calc(100vh - 165px);
        }
        .cpu-thread-chart-box{
            width: 40%;
        }
        .cpu-thread-subheading{
            font-weight: bold;
        }
        #tb-cpu-usage{
            height: 100%;
        }
        .cpu-thread-back-box{
            background-color: var(--bark-expansion,#0C65D1);
            border-radius: 5px;
            color: #fff;
            display: flex;
            margin-right: 10px;
            width: 40px;
            height: 20px;
            justify-content: center;
            align-items: center;
        }
        .cpu-thread-table-box{
            width: 60%;
            max-height: calc(100vh - 165px);
            border: solid 1px var(--dark-border1,#e0e0e0);
            border-radius: 5px;
            padding: 10px;
        }
        #cpu-thread-chart-pie{
            height: 360px;
        }
        .cpu-thread-go-back{
            display:flex;
            align-items: center;
            cursor: pointer;
        }
        </style>
        <lit-progress-bar id="loading" style="height: 1px;width: 100%"></lit-progress-bar>
        <div class="cpu-thread-d-box">
            <div class="cpu-thread-chart-box">
                <div class="cpu-thread-go-back">
                    <div class="cpu-thread-back-box">
                        <lit-icon name="arrowleft"></lit-icon>
                    </div>
                    <!--<lit-icon name="arrowleft"></lit-icon>-->
                    <div class="cpu-thread-subheading">Threads in Freq</div>
                </div>
                <div style="margin-top:15px;text-align: center">Statistics By Duration</div>
                <lit-chart-pie  id="cpu-thread-chart-pie"></lit-chart-pie>
            </div>
            <div class="cpu-thread-table-box">
                <table-no-data id="table-no-data">
                    <lit-table id="tb-cpu-usage" hideDownload>
                        <lit-table-column width="100px" title="No" data-index="index" key="index" align="flex-start" order></lit-table-column>
                        <lit-table-column width="200px" title="t_name" data-index="tName" key="tName" align="flex-start" order></lit-table-column>
                        <lit-table-column width="100px" title="tid" data-index="tid" key="tid" align="flex-start" order></lit-table-column>
                        <lit-table-column width="200px" title="p_name" data-index="pName" key="pName" align="flex-start" order></lit-table-column>
                        <lit-table-column width="100px" title="p_pid" data-index="pid" key="pid" align="flex-start" order></lit-table-column>
                        <lit-table-column width="100px" title="duration" data-index="durStr" key="durStr" align="flex-start" order></lit-table-column>
                        <lit-table-column width="100px" title="%" data-index="ratio" key="ratio" align="flex-start" order></lit-table-column>
                    </lit-table>
                </table-no-data>
            </div>
        </div>
        `;
  }
}
