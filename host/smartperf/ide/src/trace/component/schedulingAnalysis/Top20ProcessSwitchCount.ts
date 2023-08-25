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
import { LitChartPie } from '../../../base-ui/chart/pie/LitChartPie.js';
import '../../../base-ui/progress-bar/LitProgressBar.js';
import { LitProgressBar } from '../../../base-ui/progress-bar/LitProgressBar.js';
import './TableNoData.js';
import { TableNoData } from './TableNoData.js';

@element('top20-process-switch-count')
export class Top20ProcessSwitchCount extends BaseElement {
  traceChange: boolean = false;
  private processSwitchCountTbl: LitTable | null | undefined;
  private processSwitchCountPie: LitChartPie | null | undefined;
  private processSwitchCountProgress: LitProgressBar | null | undefined;
  private nodata: TableNoData | null | undefined;
  private processSwitchCountData: Array<any> = [];

  initElements(): void {
    this.nodata = this.shadowRoot!.querySelector<TableNoData>('#nodata');
    this.processSwitchCountProgress = this.shadowRoot!.querySelector<LitProgressBar>('#loading');
    this.processSwitchCountTbl = this.shadowRoot!.querySelector<LitTable>('#tb-process-switch-count');
    this.processSwitchCountPie = this.shadowRoot!.querySelector<LitChartPie>('#pie');

    this.processSwitchCountTbl!.addEventListener('row-click', (evt: any) => {
      let data = evt.detail.data;
      data.isSelected = true;
      // @ts-ignore
      if ((evt.detail as any).callBack) {
        // @ts-ignore
        (evt.detail as any).callBack(true);
      }
    });

    this.processSwitchCountTbl!.addEventListener('column-click', (evt) => {
      // @ts-ignore
      this.sortByColumn(evt.detail);
    });
    this.processSwitchCountTbl!.addEventListener('row-hover', (evt: any) => {
      if (evt.detail.data) {
        let data = evt.detail.data;
        data.isHover = true;
        if ((evt.detail as any).callBack) {
          (evt.detail as any).callBack(true);
        }
      }
      this.processSwitchCountPie?.showHover();
    });
  }

  init() {
    if (!this.traceChange) {
      if (this.processSwitchCountTbl!.recycleDataSource.length > 0) {
        this.processSwitchCountTbl?.reMeauseHeight();
      }
      return;
    }
    this.traceChange = false;
    this.processSwitchCountProgress!.loading = true;
    this.queryLogicWorker('scheduling-Process SwitchCount', 'query Process Switch Count Analysis Time:', (res) => {
      this.nodata!.noData = res === undefined || res.length === 0;
      this.processSwitchCountTbl!.recycleDataSource = res;
      this.processSwitchCountData = res;
      this.processSwitchCountTbl?.reMeauseHeight();
      this.processSwitchCountPie!.config = {
        appendPadding: 10,
        data: res,
        angleField: 'switchCount',
        colorField: 'pid',
        radius: 0.8,
        tip: (obj) => {
          return `<div>
                             <div>pid:${obj.obj.tid}</div> 
                             <div>p_name:${obj.obj.tName}</div> 
                             <div>sched_switch count:${obj.obj.switchCount}</div> 
                        </div>
                `;
        },
        label: {
          type: 'outer',
        },
        hoverHandler: (data) => {
          if (data) {
            this.processSwitchCountTbl!.setCurrentHover(data);
          } else {
            this.processSwitchCountTbl!.mouseOut();
          }
        },
        interactions: [
          {
            type: 'element-active',
          },
        ],
      };
      this.processSwitchCountProgress!.loading = false;
    });
  }

  clearData() {
    this.traceChange = true;
    this.processSwitchCountPie!.dataSource = [];
    this.processSwitchCountTbl!.recycleDataSource = [];
  }

  queryLogicWorker(option: string, log: string, handler: (res: any) => void) {
    let processSwitchCountTime = new Date().getTime();
    procedurePool.submitWithName('logic1', option, {}, undefined, handler);
    let durTime = new Date().getTime() - processSwitchCountTime;
    info(log, durTime);
  }

  sortByColumn(detail: any) {
    // @ts-ignore
    function compare(processSwitchCountProperty, sort, type) {
      return function (a: any, b: any) {
        if (type === 'number') {
          // @ts-ignore
          return sort === 2
            ? parseFloat(b[processSwitchCountProperty]) - parseFloat(a[processSwitchCountProperty])
            : parseFloat(a[processSwitchCountProperty]) - parseFloat(b[processSwitchCountProperty]);
        } else {
          if (sort === 2) {
            return b[processSwitchCountProperty].toString().localeCompare(a[processSwitchCountProperty].toString());
          } else {
            return a[processSwitchCountProperty].toString().localeCompare(b[processSwitchCountProperty].toString());
          }
        }
      };
    }

    if (detail.key === 'NO' || detail.key === 'pid' || detail.key === 'switchCount' || detail.key === 'tid') {
      this.processSwitchCountData.sort(compare(detail.key, detail.sort, 'number'));
    } else {
      this.processSwitchCountData.sort(compare(detail.key, detail.sort, 'string'));
    }
    this.processSwitchCountTbl!.recycleDataSource = this.processSwitchCountData;
  }

  initHtml(): string {
    return `
        <style>
        :host {
            width: 100%;
            height: 100%;
            background-color: var(--dark-background5,#F6F6F6);
        }
        .top-process-tb-switch-count{
            flex: 1;        
            overflow: auto ;
            border-radius: 5px;
            border: solid 1px var(--dark-border1,#e0e0e0);
            margin: 15px;
            padding: 5px 15px
        }
        .top-process-pie-chart{
            display: flex;
            box-sizing: border-box;
            width: 500px;
            height: 500px;
        }
        .top-process-switch-root{
            display: flex;
            flex-direction: row;
            box-sizing: border-box;
            width: 100%;
            height: 100%;
        }
        </style>
        <lit-progress-bar id="loading" style="height: 1px;width: 100%" loading></lit-progress-bar>
        <table-no-data id="nodata" contentHeight="500px">
        <div class="top-process-switch-root">
            <div style="display: flex;flex-direction: column;align-items: center">
                <div>Statistics By Sched_Switch Count</div>
                <lit-chart-pie id="pie" class="top-process-pie-chart"></lit-chart-pie>
            </div>
            <div class="top-process-tb-switch-count" >
                <lit-table id="tb-process-switch-count" hideDownload style="height: auto">
                    <lit-table-column width="1fr" title="NO" data-index="NO" key="NO" align="flex-start" order></lit-table-column>
                    <lit-table-column width="1fr" title="tid" data-index="tid" key="tid" align="flex-start" order></lit-table-column>
                    <lit-table-column width="1fr" title="t_name" data-index="tName" key="tName" align="flex-start" order></lit-table-column>
                    <lit-table-column width="1fr" title="pid" data-index="pid" key="pid" align="flex-start" order></lit-table-column>
                    <lit-table-column width="1fr" title="p_name" data-index="pName" key="pName" align="flex-start" order></lit-table-column>
                    <lit-table-column width="1fr" title="sched_switch count" data-index="switchCount" key="switchCount" align="flex-start" order></lit-table-column>        
                </lit-table>
            </div>
        </div>
        </table-no-data>
        `;
  }
}
