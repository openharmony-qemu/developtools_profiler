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

import { BaseElement, element } from '../../base-ui/BaseElement.js';

import {
  querySelectTraceStats, querySystemCalls,
  queryTraceMemory,
  queryTraceMemoryTop,
  queryTraceMemoryUnAgg,
  queryTraceMetaData,
  queryTraceTaskName,
} from '../database/SqlLite.js';

import '../../base-ui/table/lit-table.js';
import { initMemoryAggStrategy } from './metrics/MemAggStrategy.js';
import { initMemoryStrategy } from './metrics/MemStrategy.js';
import { initTraceStateStrategy } from './metrics/TraceStatsStrategy.js';
import { initTraceTaskStrategy } from './metrics/TraceTaskStrategy.js';
import { initMetaDataStrategy } from './metrics/MetaDataStrategy.js';
import { PluginConvertUtils } from './setting/utils/PluginConvertUtils.js';
import { info } from '../../log/Log.js';
import { LitProgressBar } from '../../base-ui/progress-bar/LitProgressBar.js';
import { SpStatisticsHttpUtil } from '../../statistics/util/SpStatisticsHttpUtil.js';
import { initSysCallsStrategy } from './metrics/SysCallsStrategy.js';

@element('sp-metrics')
export class SpMetrics extends BaseElement {
  private _metric?: string;
  private _metricResult?: string;
  private selectMetricEl: HTMLSelectElement | undefined;
  private runButtonEl: HTMLButtonElement | undefined | null;
  private responseJson: HTMLPreElement | undefined | null;
  private metricOptionalSelects: Array<MetricQueryItem> | undefined;
  private metricProgressLoad: LitProgressBar | undefined;

  static get observedAttributes(): string[] {
    return ['metric', 'metricResult'];
  }

  get metric(): string {
    return this.getAttribute('metric') || '';
  }

  set metric(value: string) {
    this._metric = value;
  }

  get metricResult(): string {
    return this.getAttribute('metricResult') || '';
  }

  set metricResult(value: string) {
    this._metricResult = value;
    this.setAttribute('metricResult', value);
  }

  reset(): void {
    this.selectMetricEl!.selectedIndex = 0;
    this.responseJson!.textContent = '';
  }

  initElements(): void {
    this.metricProgressLoad = this.shadowRoot?.querySelector('.sp-load-metric') as LitProgressBar;
    this.selectMetricEl = this.shadowRoot?.querySelector('.sql-select') as HTMLSelectElement;
    this.runButtonEl = this.shadowRoot?.querySelector('.sql-select-button') as HTMLButtonElement;
    this.responseJson = this.shadowRoot?.querySelector('.response-json') as HTMLPreElement;
    if (this.selectMetricEl) {
      this.selectMetricEl.addEventListener('selectionchange', () => {
        if (this.selectMetricEl) {
          this.selectMetricEl.textContent = '';
        }
      });
    }
    this.initMetricDataHandle();
    this.initMetricSelectOption();
  }

  initMetric(queryItem: MetricQueryItem): void {
    this.initMetricData(queryItem).then(() => {
      this.metricProgressLoad!.loading = false;
    });
  }

  async initMetricData(queryItem: MetricQueryItem): Promise<void> {
    let spacesNumber = 4;
    let metricQuery = queryItem.metricQuery;
    let queryList = await metricQuery();
    info('current Metric Data size is: ', queryList!.length);
    let metric = queryItem.metricResultHandle;
    let resultData = metric(queryList);
    this.responseJson!.textContent = PluginConvertUtils.BeanToCmdTxtWithObjName(resultData, true,
      queryItem.metricName, spacesNumber);
  }

  attributeChangedCallback(name: string, oldValue: string, newValue: string): void {
    switch (name) {
      case 'metric':
        if (this.selectMetricEl) {
          this.selectMetricEl.textContent = newValue;
        }
        break;
      case 'metricResult':
        if (this.selectMetricEl) {
          this.selectMetricEl.textContent = newValue;
        }
        break;
    }
  }

  runClickListener = (): void => {
    SpStatisticsHttpUtil.addOrdinaryVisitAction({
      event: 'metrics',
      action: 'metrics',
    });
    this.metricProgressLoad!.loading = true;
    let selectedIndex = this.selectMetricEl!.selectedIndex;
    let value = this.selectMetricEl!.options[selectedIndex].value;
    let resultQuery = this.metricOptionalSelects?.filter((item) => {
      return item.metricName === value;
    });
    if (!resultQuery || resultQuery.length === 0) {
      return;
    }
    this.initMetric(resultQuery[0]);
  };

  connectedCallback(): void {
    // Run metric button to add listener
    this.runButtonEl?.addEventListener('click', this.runClickListener);
  }

  disconnectedCallback(): void {
    this.runButtonEl?.removeEventListener('click', this.runClickListener);
  }

  initMetricSelectOption(): void {
    for (let index = 0 ; index < this.metricOptionalSelects!.length ; index++) {
      let htmlElement = document.createElement('option');
      if (this.metricOptionalSelects) {
        htmlElement.textContent = this.metricOptionalSelects[index].metricName;
        this.selectMetricEl?.appendChild(htmlElement);
      }
    }
  }

  initMetricDataHandle(): void {
    this.metricOptionalSelects = [
      {
        metricName: 'trace_mem',
        metricQuery: queryTraceMemory,
        metricResultHandle: initMemoryStrategy,
      },
      {
        metricName: 'trace_mem_top10',
        metricQuery: queryTraceMemoryTop,
        metricResultHandle: initMemoryStrategy,
      },
      {
        metricName: 'trace_mem_unagg',
        metricQuery: queryTraceMemoryUnAgg,
        metricResultHandle: initMemoryAggStrategy,
      },
      {
        metricName: 'trace_task_names',
        metricQuery: queryTraceTaskName,
        metricResultHandle: initTraceTaskStrategy,
      },
      {
        metricName: 'trace_stats',
        metricQuery: querySelectTraceStats,
        metricResultHandle: initTraceStateStrategy,
      },
      {
        metricName: 'trace_metadata',
        metricQuery: queryTraceMetaData,
        metricResultHandle: initMetaDataStrategy,
      },
      {
        metricName: 'sys_calls',
        metricQuery: querySystemCalls,
        metricResultHandle: initSysCallsStrategy,
      },
    ];
  }

  initHtml(): string {
    return `
        <style>
        :host{
            width: 100%;
            height: 100%;
            background-color: var(--dark-background5,#F6F6F6);
            margin: 0;
            padding: 0;
            font-size:16px;
        }
        .metric{
            display: flex;
            flex-direction: column;
            position: absolute;
            top: 0;
            bottom: 0;
            left: 0;
            right: 0;
            background-color: var(--dark-background5,#F6F6F6);
        }
        .metric-select{
            color: #121212;
            border-radius: 16px;
            background-color: var(--dark-background3,#FFFFFF);
            padding: 1% 2%;
            margin: 2% 2.5% 0 2.5%;
            grid-row-gap: 30px;
        }
        .request{
            min-height: 15vh;
            overflow: auto;
            position: relative;
        }
        .sql-select{
            font-family: Helvetica,serif;
            color: var(--dark-color1,#212121);
            font-size:0.875em;
            line-height: 16px;
            font-weight: 400;
            text-align: left;
            width: 50%;
            height: 32px;
            flex-wrap: wrap;
            margin-top: 1%;
            border: 1px solid var(--dark-color1,#4D4D4D);
            border-radius: 16px;
            padding: 5px 10px 5px 10px;
            -webkit-appearance: none;
            background: url('img/down.png') no-repeat 98% center var(--dark-background3,#FFFFFF);
        }
        button{
            border-radius: 16px;
            flex-grow: 1;
            background-color: #0A59F7;
            height: 32px;
            width: 96px;
            font-size: 0.875em;
            color: var(--dark-background3,#FFFFFF);
            text-align: center;
            line-height: 20px;
            font-weight: 400;
            border:0 solid;
            margin-left: 2%;
            opacity: 0.6;
            cursor:pointer;
        }
        .response{
            flex-grow: 1;
            margin-bottom: 1%;
        }
        .response-json{
            background-color: var(--dark-background3,#FFFFFF);
            border-radius: 16px;
            display: table-cell;
            font-family: Helvetica,serif;
            color: var(--dark-color1,#212121);
            font-size:0.875em;
            line-height: 20px;
            font-weight: 400;
            text-align: left;
            height: 90%;
            width: 100%;
            border: none;
            outline:none;
            resize:none;
        }
        p{
             display: table-cell;
             padding: 20% 0;
             color: #999999;
             font-size:0.875em;
             line-height: 20px;
             font-weight: 400;
             text-align: left;
             width: 100%;
        }
        ::-webkit-scrollbar-thumb{
          background-color: var(--dark-background7,rgba(0,0,0,0.1));
          border-radius: 6px;
        }
        .sp-load-metric{
            width: 95%;
            bottom: 0;
        }
        ::-webkit-scrollbar{
          background-color: var(--dark-background3,#FFFFFF);
          width: 8px;
        }
        </style>
        <div class="metric">
            <div class="metric-select request">
                <p>Select a metric</p>
                <select class="sql-select">
                </select>
                <button class="sql-select-button">&nbsp;&nbsp; Run &nbsp;&nbsp;</button>
                <lit-progress-bar class="sp-load-metric"></lit-progress-bar>
            </div>
            <div class="metric-select response">
                 <textarea class="response-json" readonly>
                 </textarea>
            </div>
        </div>
        `;
  }
}

export interface MetricQueryItem {
  metricName: string;
  metricQuery: Function;
  metricResultHandle: Function;
}
