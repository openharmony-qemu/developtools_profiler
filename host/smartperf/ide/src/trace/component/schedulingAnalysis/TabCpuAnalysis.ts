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
import { SpSchedulingAnalysis } from './SpSchedulingAnalysis.js';
import { DrawerCpuTabs } from './DrawerCpuTabs.js';
import { LitChartPie } from '../../../base-ui/chart/pie/LitChartPie.js';
import { LitDrawer } from '../../../base-ui/drawer/LitDrawer.js';
import '../../../base-ui/drawer/LitDrawer.js';
import './DrawerCpuTabs.js';
import { procedurePool } from '../../database/Procedure.js';
import { info } from '../../../log/Log.js';
import { LitSelect } from '../../../base-ui/select/LitSelect';
import '../../../base-ui/progress-bar/LitProgressBar.js';
import { LitProgressBar } from '../../../base-ui/progress-bar/LitProgressBar.js';
import { pieChartColors } from '../../../base-ui/chart/pie/LitChartPieData.js';
import { SpStatisticsHttpUtil } from '../../../statistics/util/SpStatisticsHttpUtil.js';

@element('tab-cpu-analysis')
export class TabCpuAnalysis extends BaseElement {
  private cpuUsageGrid: HTMLDivElement | undefined;
  private cpuUsageChart: HTMLDivElement | undefined;
  private drawer: LitDrawer | undefined | null;
  private cpuPieMap: Map<number, LitChartPie> = new Map<number, LitChartPie>();
  private schedulingSelect: LitSelect | undefined | null;
  private drawerCpuTabs: DrawerCpuTabs | undefined | null;
  private progress: LitProgressBar | null | undefined;
  private loadingUsage: boolean = false;
  private loadingPieData: boolean = false;

  initElements(): void {
    this.progress = this.shadowRoot!.querySelector<LitProgressBar>('#loading');
    this.cpuUsageGrid = this.shadowRoot?.querySelector('#cpu_usage_table') as HTMLDivElement;
    this.cpuUsageChart = this.shadowRoot?.querySelector('#cpu_usage_chart') as HTMLDivElement;
    this.schedulingSelect = this.shadowRoot?.querySelector<LitSelect>('#scheduling_select');
    this.drawer = this.shadowRoot!.querySelector<LitDrawer>('#drawer-right');
    this.drawerCpuTabs = this.shadowRoot?.querySelector<DrawerCpuTabs>('#drawer-cpu-tabs');
    this.schedulingSelect!.onchange = (e) => {
      this.loadingPieData = true;
      this.progress!.loading = this.loadingUsage || this.loadingPieData;
      this.queryPieChartDataByType((e as any).detail.text);
    };
    this.drawer!.onClose = (e: any) => {
      this.drawerCpuTabs!.clearData();
    };
  }

  init() {
    this.cpuPieMap.clear();
    this.cpuUsageGrid!.innerHTML = '';
    this.cpuUsageChart!.innerHTML = '';
    this.schedulingSelect!.value = '1';
    this.cpuUsageGrid!.append(this.createUsageItem('usage', '%'));
    for (let i = 0; i < SpSchedulingAnalysis.cpuCount; i++) {
      let cpuPie = new LitChartPie();
      cpuPie.className = 'pie-chart';
      this.cpuPieMap.set(i, cpuPie);
      this.cpuUsageGrid!.append(this.createUsageItem(`CPU: ${i}`, 0));
      this.cpuUsageChart!.append(this.createUsageChartItem(i, cpuPie));
    }
    this.loadingUsage = true;
    this.loadingPieData = true;
    this.progress!.loading = this.loadingUsage || this.loadingPieData;
    this.queryLogicWorker('scheduling-getCpuUsage', 'query Cpu Usage Time:', (res) => {
      if (res && res.length > 0) {
        this.cpuUsageGrid!.innerHTML = '';
        this.cpuUsageGrid!.append(this.createUsageItem('usage', '%'));
        if (res instanceof Array) {
          for (let re of res) {
            this.cpuUsageGrid!.append(this.createUsageItem(`CPU: ${re.cpu}`, ((re.usage || 0) * 100).toFixed(2)));
          }
        }
      }
      this.loadingUsage = false;
      this.progress!.loading = this.loadingUsage || this.loadingPieData;
    });
    this.queryPieChartDataByType('CPU Idle');
    SpStatisticsHttpUtil.addOrdinaryVisitAction({
      event: 'CPU Data',
      action: 'trace_tab',
    });
  }

  queryPieChartDataByType(type: string) {
    SpStatisticsHttpUtil.addOrdinaryVisitAction({
      event: 'Analysis ' + type,
      action: 'scheduling_analysis',
    });
    let tip = '';
    if (type === 'CPU Frequency') {
      tip = 'freq:';
    } else if (type === 'CPU Idle') {
      tip = 'idle:';
    } else {
      tip = 'irq:';
    }
    this.queryLogicWorker(`scheduling-${type}`, `query ${type} Analysis Time:`, (res) => {
      for (let key of this.cpuPieMap.keys()) {
        this.cpuPieMap.get(key)!.config = {
          appendPadding: 10,
          data: res.get(key) || [],
          angleField: 'sum',
          colorField: 'value',
          radius: 0.8,
          tip: (obj) => {
            return `<div>
                                    <div>${tip}${obj.obj.value}</div> 
                                    <div>ratio:${obj.obj.ratio}%</div>
                                </div>
                                `;
          },
          label: {
            type: 'outer',
            color:
              type !== 'CPU Idle'
                ? undefined
                : (it) => {
                    return pieChartColors[(it as any).value];
                  },
          },
          interactions: [
            {
              type: 'element-active',
            },
          ],
        };
      }
      this.loadingPieData = false;
      this.progress!.loading = this.loadingUsage || this.loadingPieData;
    });
  }

  queryLogicWorker(cpuAnalysisType: string, log: string, handler: (res: any) => void) {
    let cpuAnalysisTime = new Date().getTime();
    procedurePool.submitWithName(
      'logic1',
      cpuAnalysisType,
      {
        endTs: SpSchedulingAnalysis.endTs,
        total: SpSchedulingAnalysis.totalDur,
      },
      undefined,
      handler
    );
    let durTime = new Date().getTime() - cpuAnalysisTime;
    info(log, durTime);
  }

  createUsageItem(name: string, value: any) {
    let div = document.createElement('div');
    div.className = 'usage_item_box';
    div.innerHTML = `<div class="usage_item">${name}</div><div class="usage_item">${value}</div>`;
    return div;
  }

  createUsageChartItem(cpu: number, pie: LitChartPie) {
    let div = document.createElement('div');
    div.className = 'usage_chart';
    div.style.cursor = 'pointer';
    div.innerHTML = `
            <div style="height: 40px;line-height: 40px;margin-left: 10px">CPU: ${cpu}</div>
        `;
    div.append(pie);
    div.addEventListener('click', (event) => {
      if (this.loadingUsage || this.loadingPieData) {
        return;
      }
      this.drawer!.title = `CPU: ${cpu}`;
      this.drawer!.visible = true;
      this.drawerCpuTabs!.init(cpu, this.schedulingSelect!.value);
    });
    return div;
  }

  initHtml(): string {
    return `
        <style>
        :host {
            width: 100%;
            height: 100%;
            background-color: var(--dark-background,#FFFFFF);
        }
        .cpu_usage{
            display: flex;
            flex-direction: row;
            box-sizing: border-box;
            padding: 15px;
            background-color: var(--dark-background,#FFFFFF);
        }
        .usage_item{
            height: 50px;
            line-height: 50px;
            text-align: center;
            border: solid 1px var(--dark-border1,#f0f0f0);
        }
        .usage_item_box{
            border: solid 1px var(--dark-border1,#f0f0f0);
            flex: 1;
        }
        .usage_chart{
            height: 360px;
            color: var(--dark-color1,#252525);
            background-color: var(--dark-background,#FFFFFF);
        }
        .pie-chart{
            display: flex;
            box-sizing: border-box;
            height: 300px;
        }
        .grid_usage{
            display: grid;
            width: 100%;
            box-sizing: border-box;
            grid-template-columns: repeat(4,calc((100% - 30px) / 4));
            grid-column-gap: 10px;
            grid-row-gap: 10px;
            padding: 10px;
            background-color: var(--dark-background5,#F6F6F6);
        }
        .cpu-statistics{
            height: 50px;
            display: flex;
            flex-direction: row;
            justify-content: space-between;
            align-items: center;
            padding:0px 15px;
            background-color: var(--dark-background,#FFFFFF);
        }
        </style>
        <div style="display: flex;flex-direction: column;overflow-x: hidden;overflow-y: auto;height: 100%">
            <lit-progress-bar id="loading" style="height: 1px;width: 100%"></lit-progress-bar>
            <div class="cpu_usage" id="cpu_usage_table"></div>
            <div class="cpu-statistics">
                <div>CPU Statistics By Duration</div>
                <lit-select default-value="1" id="scheduling_select">
                    <lit-select-option value="1">CPU Idle</lit-select-option>
                    <lit-select-option value="2">CPU Frequency</lit-select-option>
                    <lit-select-option value="3">CPU Irq</lit-select-option>
                </lit-select>
            </div>
            <div class="grid_usage" id="cpu_usage_chart"></div>
        </div>
        <lit-drawer id="drawer-right" title="CPU：0" content-width="65vw" placement="right" style="position: fixed" fixed mask mask-closeable closeable content-padding="0">
            <drawer-cpu-tabs id="drawer-cpu-tabs"></drawer-cpu-tabs>
        </lit-drawer>
        `;
  }
}
