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

import { BaseElement, element } from '../../../../../base-ui/BaseElement.js';
import { LitChartPie } from '../../../../../base-ui/chart/pie/LitChartPie.js';
import { LitProgressBar } from '../../../../../base-ui/progress-bar/LitProgressBar.js';
import { LitTable } from '../../../../../base-ui/table/lit-table.js';
import { SelectionParam } from '../../../../bean/BoxSelection.js';
import { JsCpuProfilerChartFrame, JsCpuProfilerStatisticsStruct } from '../../../../bean/JsStruct.js';
import { procedurePool } from '../../../../database/Procedure.js';
import { SampleType } from '../../../../database/logic-worker/ProcedureLogicWorkerJsCpuProfiler.js';
import { ns2s } from '../../../../database/ui-worker/ProcedureWorkerCommon.js';
import { resizeObserver } from '../SheetUtils.js';

@element('tabpane-js-cpu-statistics')
export class TabPaneJsCpuStatistics extends BaseElement {
  private progress: LitProgressBar | null | undefined;
  private statisticsTable: LitTable | null | undefined;
  private statisticsSource: Array<JsCpuProfilerStatisticsStruct> = [];
  private tabTitle: HTMLDivElement | undefined | null;
  private sortKey = 'timeStr';
  private sortType = 2;
  private statisticsPie: LitChartPie | null | undefined;

  set data(data: SelectionParam | Array<JsCpuProfilerChartFrame>) {
    this.init();
    this.clearData();
    this.progress!.loading = true;

    this.getDataByWorker(data, (results: Map<SampleType, number>) => {
      this.progress!.loading = false;

      this.statisticsSource = this.setStatisticsData(results);
      this.queryPieChartDataByType(this.statisticsSource || []);
    });
  }

  private init() {
    const thTable = this.tabTitle!.querySelector('.th');
    const jsCpuStatTblNodes = thTable!.querySelectorAll('div');
    if (this.tabTitle!.hasAttribute('sort')) {
      this.tabTitle!.removeAttribute('sort');
      jsCpuStatTblNodes.forEach((item) => {
        item.querySelectorAll('svg').forEach((svg) => {
          svg.style.display = 'none';
        });
      });
    }
    this.sortKey = 'timeStr';
    this.sortType = 2;
  }

  private getDataByWorker(data: SelectionParam | Array<JsCpuProfilerChartFrame>, handler: Function): void {
    let params = undefined;
    if (data instanceof SelectionParam) {
      params = {
        data: data.jsCpuProfilerData,
        leftNs: data.leftNs,
        rightNs: data.rightNs,
      };
    } else {
      params = {
        data: data,
      };
    }
    procedurePool.submitWithName(
      'logic1',
      'jsCpuProfiler-statistics',
      params,
      undefined,
      (results: Map<SampleType, number>) => {
        handler(results);
      }
    );
  }

  private queryPieChartDataByType(res: Array<JsCpuProfilerStatisticsStruct>): void {
    this.statisticsPie!.config = {
      appendPadding: 0,
      data: res,
      angleField: 'time',
      colorField: 'type',
      radius: 1,
      label: {
        type: 'outer',
      },
      tip: (obj) => {
        return `<div>
                    <div>type: ${obj.obj.type}</div>
                    <div>total: ${ns2s(obj.obj.time)} (${obj.obj.percentage}%)</div>
                </div> `;
      },
      hoverHandler: (data) => {
        if (data) {
          this.statisticsTable!.setCurrentHover(data);
        } else {
          this.statisticsTable!.mouseOut();
        }
      },
      interactions: [
        {
          type: 'element-active',
        },
      ],
    };
    this.statisticsTable!.addEventListener('row-hover', (statisticsRowHover: any) => {
      if (statisticsRowHover.detail.data) {
        let data = statisticsRowHover.detail.data;
        data.isHover = true;
        if ((statisticsRowHover.detail as any).callBack) {
          (statisticsRowHover.detail as any).callBack(true);
        }
      }
      this.statisticsPie?.showHover();
      this.statisticsPie?.hideTip();
    });
    this.sortByColumn({ key: this.sortKey, sort: this.sortType });
    let total = this.totalData(this.statisticsSource);
    this.statisticsSource.unshift(total);
    this.statisticsTable!.recycleDataSource = this.statisticsSource;
    this.statisticsSource.shift();
    this.statisticsTable?.reMeauseHeight();
  }

  private totalData(source: Array<JsCpuProfilerStatisticsStruct>) {
    // 计算总的time作为表格的第一行显示
    let totalTime = 0;
    for (let item of source) {
      totalTime += item.time;
    }
    let totalData = this.toStatisticsStruct('', totalTime, totalTime);
    return totalData;
  }

  private clearData(): void {
    this.statisticsPie!.dataSource = [];
    this.statisticsTable!.recycleDataSource = [];
  }

  private setStatisticsData(results: Map<SampleType, number>): Array<JsCpuProfilerStatisticsStruct> {
    this.statisticsTable!.recycleDataSource = [];
    this.statisticsSource = [];
    let statisticsData: JsCpuProfilerStatisticsStruct;
    const totalTime = [...results.values()].reduce((prev, curr) => prev + curr);
    for (let [key, value] of results.entries()) {
      statisticsData = this.toStatisticsStruct(key, value, totalTime);
      this.statisticsSource.push(statisticsData);
    }
    return this.statisticsSource.sort((a, b) => b.time - a.time);
  }

  private toStatisticsStruct(
    type: string | SampleType,
    time: number,
    percentage: number
  ): JsCpuProfilerStatisticsStruct {
    const statisticsStruct = new JsCpuProfilerStatisticsStruct(
      type,
      time,
      ns2s(time),
      ((time / percentage) * 100).toFixed(1)
    );
    return statisticsStruct;
  }

  private sortByColumn(detail: any): void {
    // @ts-ignore
    function compare(key, sort, type) {
      return function (a: any, b: any) {
        // 不管哪一列的排序方式是0（默认排序），都按照time列从大到小排序
        if (sort === 0) {
          sort = 2;
          key = 'time';
          type = 'number';
        }
        if (type === 'number') {
          // @ts-ignore
          return sort === 2 ? parseFloat(b[key]) - parseFloat(a[key]) : parseFloat(a[key]) - parseFloat(b[key]);
        } else {
          if (sort === 2) {
            return b[key].toString().localeCompare(a[key].toString());
          } else {
            return a[key].toString().localeCompare(b[key].toString());
          }
        }
      };
    }
    if (detail.key === 'timeStr' || detail.key === 'percentage') {
      this.statisticsSource.sort(compare('time', detail.sort, 'number'));
    } else if (detail.key === 'type') {
      this.statisticsSource.sort(compare(detail.key, detail.sort, 'string'));
    }

    let total = this.totalData(this.statisticsSource);
    this.statisticsSource.unshift(total);
    this.statisticsTable!.recycleDataSource = this.statisticsSource;
    this.statisticsSource.shift();
  }

  public connectedCallback(): void {
    super.connectedCallback();
    resizeObserver(this.parentElement!, this.statisticsTable!);
  }

  public initElements(): void {
    this.progress = this.shadowRoot!.querySelector<LitProgressBar>('#loading');
    this.statisticsTable = this.shadowRoot?.querySelector('#statistics-table') as LitTable;
    this.statisticsPie = this.shadowRoot?.querySelector('#chart-pie') as LitChartPie;
    this.tabTitle = this.statisticsTable!.shadowRoot?.querySelector('.thead') as HTMLDivElement;

    this.statisticsTable!.addEventListener('column-click', (evt: any) => {
      this.sortKey = evt.detail.key;
      this.sortType = evt.detail.sort;
      // @ts-ignore
      this.sortByColumn(evt.detail);
    });
  }

  public initHtml(): string {
    return `
    <style>
    :host{
        height: 100%;
        background-color: var(--dark-background,#FFFFFF);
        display: flex;
        flex-direction: column;
    }
    .d-box{
        display: flex;
        margin: 20px;
        height: calc(100vh - 165px);
    }
    .chart-box{
        width: 40%;
    }
    .table-box{
        width: 60%;
        max-height: calc(100vh - 165px);
        border-left: solid 1px var(--dark-border1,#e0e0e0);
        border-radius: 5px;
        padding: 10px;
    }
    #chart-pie{
        height: 360px;
    }
    .js-cpu-statistics-tbl {
        height: auto
    }
    .statistics-column{
        min-width: 130px;
    }
    </style>
    <lit-progress-bar id="loading" style="height: 1px;width: 100%"></lit-progress-bar>
    <div class="d-box">
        <div class="chart-box">
            <div style="text-align: center">Statistics By Total</div>
            <lit-chart-pie id="chart-pie"></lit-chart-pie>
        </div>
        <div class="table-box">
            <lit-table id="statistics-table" class="js-cpu-statistics-tbl">
                <lit-table-column class="statistics-column" width="1fr" title="Type" data-index="type" key="type"  align="flex-start" order></lit-table-column>
                <lit-table-column class="statistics-column" width="1fr" title="Total" data-index="timeStr" key="timeStr"  align="flex-start" order></lit-table-column>
                <lit-table-column class="statistics-column" width="1fr" title="%" data-index="percentage" key="percentage"  align="flex-start" order></lit-table-column>
            </lit-table>
        </div>
    </div>
    `;
  }
}
