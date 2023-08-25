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
import { LitTable } from '../../../base-ui/table/lit-table.js';
import { procedurePool } from '../../database/Procedure.js';
import { info } from '../../../log/Log.js';
import '../../../base-ui/chart/pie/LitChartPie.js';
import { LitChartPie } from '../../../base-ui/chart/pie/LitChartPie.js';
import '../../../base-ui/progress-bar/LitProgressBar.js';
import { LitProgressBar } from '../../../base-ui/progress-bar/LitProgressBar.js';
import './TableNoData.js';
import { TableNoData } from './TableNoData.js';

@element('top20-process-thread-count')
export class Top20ProcessThreadCount extends BaseElement {
  traceChange: boolean = false;
  private processThreadCountTbl: LitTable | null | undefined;
  private processThreadCountPie: LitChartPie | null | undefined;
  private processThreadCountProgress: LitProgressBar | null | undefined;
  private nodata: TableNoData | null | undefined;
  private processThreadCountData: Array<any> = [];

  initElements(): void {
    this.nodata = this.shadowRoot!.querySelector<TableNoData>('#nodata');
    this.processThreadCountProgress = this.shadowRoot!.querySelector<LitProgressBar>('#loading');
    this.processThreadCountTbl = this.shadowRoot!.querySelector<LitTable>('#tb-process-thread-count');
    this.processThreadCountPie = this.shadowRoot!.querySelector<LitChartPie>('#pie');

    this.processThreadCountTbl!.addEventListener('row-click', (evt: any) => {
      let data = evt.detail.data;
      data.isSelected = true;
      // @ts-ignore
      if ((evt.detail as any).callBack) {
        // @ts-ignore
        (evt.detail as any).callBack(true);
      }
    });

    this.processThreadCountTbl!.addEventListener('column-click', (evt) => {
      // @ts-ignore
      this.sortByColumn(evt.detail);
    });
    this.processThreadCountTbl!.addEventListener('row-hover', (evt: any) => {
      if (evt.detail.data) {
        let data = evt.detail.data;
        data.isHover = true;
        if ((evt.detail as any).callBack) {
          (evt.detail as any).callBack(true);
        }
      }
      this.processThreadCountPie?.showHover();
    });
  }

  init() {
    if (!this.traceChange) {
      if (this.processThreadCountTbl!.recycleDataSource.length > 0) {
        this.processThreadCountTbl?.reMeauseHeight();
      }
      return;
    }
    this.traceChange = false;
    this.processThreadCountProgress!.loading = true;
    this.queryLogicWorker('scheduling-Process ThreadCount', 'query Process Thread Count Analysis Time:', (res) => {
      this.nodata!.noData = res === undefined || res.length === 0;
      this.processThreadCountTbl!.recycleDataSource = res;
      this.processThreadCountData = res;
      this.processThreadCountTbl?.reMeauseHeight();
      this.processThreadCountPie!.config = {
        appendPadding: 10,
        data: res,
        angleField: 'threadNumber',
        colorField: 'pid',
        radius: 0.8,
        label: {
          type: 'outer',
        },
        hoverHandler: (data) => {
          if (data) {
            this.processThreadCountTbl!.setCurrentHover(data);
          } else {
            this.processThreadCountTbl!.mouseOut();
          }
        },
        tip: (obj) => {
          return `<div>
                             <div>pid:${obj.obj.pid}</div> 
                             <div>p_name:${obj.obj.pName}</div> 
                             <div>thread count:${obj.obj.threadNumber}</div> 
                        </div>
                `;
        },
        interactions: [
          {
            type: 'element-active',
          },
        ],
      };
      this.processThreadCountProgress!.loading = false;
    });
  }

  clearData() {
    this.traceChange = true;
    this.processThreadCountPie!.dataSource = [];
    this.processThreadCountTbl!.recycleDataSource = [];
  }

  queryLogicWorker(option: string, log: string, handler: (res: any) => void) {
    let processThreadCountTime = new Date().getTime();
    procedurePool.submitWithName('logic1', option, {}, undefined, handler);
    let durTime = new Date().getTime() - processThreadCountTime;
    info(log, durTime);
  }

  sortByColumn(detail: any) {
    // @ts-ignore
    function compare(processThreadCountProperty, sort, type) {
      return function (a: any, b: any) {
        if (type === 'number') {
          // @ts-ignore
          return sort === 2
            ? parseFloat(b[processThreadCountProperty]) - parseFloat(a[processThreadCountProperty])
            : parseFloat(a[processThreadCountProperty]) - parseFloat(b[processThreadCountProperty]);
        } else {
          if (sort === 2) {
            return b[processThreadCountProperty].toString().localeCompare(a[processThreadCountProperty].toString());
          } else {
            return a[processThreadCountProperty].toString().localeCompare(b[processThreadCountProperty].toString());
          }
        }
      };
    }

    if (detail.key === 'NO' || detail.key === 'pid' || detail.key === 'threadNumber') {
      this.processThreadCountData.sort(compare(detail.key, detail.sort, 'number'));
    } else {
      this.processThreadCountData.sort(compare(detail.key, detail.sort, 'string'));
    }
    this.processThreadCountTbl!.recycleDataSource = this.processThreadCountData;
  }

  initHtml(): string {
    return `
        <style>
        :host {
            width: 100%;
            height: 100%;
            background-color: var(--dark-background5,#F6F6F6);
        }
        .pie-chart{
            display: flex;
            box-sizing: border-box;
            width: 500px;
            height: 500px;
        }
        .tb_thread_count{
            flex: 1;
            overflow: auto ;
            border-radius: 5px;
            border: solid 1px var(--dark-border1,#e0e0e0);
            margin: 15px;
            padding: 5px 15px
        }
        .thread-root{
            width: 100%;
            height: 100%;
            box-sizing: border-box;
            display: flex;
            flex-direction: row;
        }
        </style>
        <lit-progress-bar id="loading" style="height: 1px;width: 100%" loading></lit-progress-bar>
        <table-no-data id="nodata" contentHeight="500px">
        <div class="thread-root">
            <div style="display: flex;flex-direction: column;align-items: center">
                <div>Statistics By Thread Count</div>
                <lit-chart-pie id="pie" class="pie-chart"></lit-chart-pie>
            </div>
            <div class="tb_thread_count">
                <lit-table id="tb-process-thread-count" hideDownload style="height: auto">
                    <lit-table-column width="1fr" title="NO" data-index="NO" key="NO" align="flex-start" order></lit-table-column>
                    <lit-table-column width="1fr" title="pid" data-index="pid" key="pid" align="flex-start" order></lit-table-column>
                    <lit-table-column width="1fr" title="p_name" data-index="pName" key="pName" align="flex-start" order></lit-table-column>
                    <lit-table-column width="1fr" title="thread count" data-index="threadNumber" key="threadNumber" align="flex-start" order></lit-table-column>        
                </lit-table>
            </div>
        </div>
        </table-no-data>
        `;
  }
}
