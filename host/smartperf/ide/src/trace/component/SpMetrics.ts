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

import { queryMetric } from '../database/SqlLite.js';

import '../../base-ui/table/lit-table.js';
import { LitProgressBar } from '../../base-ui/progress-bar/LitProgressBar.js';
import { SpStatisticsHttpUtil } from '../../statistics/util/SpStatisticsHttpUtil.js';

@element('sp-metrics')
export class SpMetrics extends BaseElement {
  private selectMetricEl: HTMLSelectElement | undefined;
  private runButtonEl: HTMLButtonElement | undefined | null;
  private responseJson: HTMLPreElement | undefined | null;
  private metricProgressLoad: LitProgressBar | undefined;

  reset(): void {
    this.selectMetricEl!.selectedIndex = 0;
    this.responseJson!.textContent = '';
  }

  initElements(): void {
    this.metricProgressLoad = this.shadowRoot?.querySelector('.sp-load-metric') as LitProgressBar;
    this.selectMetricEl = this.shadowRoot?.querySelector('.sql-select') as HTMLSelectElement;
    this.runButtonEl = this.shadowRoot?.querySelector('.sql-select-button') as HTMLButtonElement;
    this.responseJson = this.shadowRoot?.querySelector('.response-json') as HTMLPreElement;
  }

  runClickListener = (): void => {
    SpStatisticsHttpUtil.addOrdinaryVisitAction({
      event: 'metrics',
      action: 'metrics',
    });
    this.responseJson!.textContent = '';
    this.metricProgressLoad!.loading = true;
    let index = this.selectMetricEl!.selectedIndex;
    let optionEl = this.selectMetricEl?.querySelectorAll<HTMLOptionElement>('option')[index];
    if (optionEl && optionEl.value !== '') {
      queryMetric(optionEl.value).then((result) => {
        this.metricProgressLoad!.loading = false;
        this.responseJson!.textContent = result.toString();
      });
    } else {
      this.metricProgressLoad!.loading = false;
    }
  };

  connectedCallback(): void {
    this.runButtonEl?.addEventListener('click', this.runClickListener);
  }

  disconnectedCallback(): void {
    this.runButtonEl?.removeEventListener('click', this.runClickListener);
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
                  <option>trace_mem</option>
                  <option>trace_mem_top10</option>
                  <option>trace_mem_unagg</option>
                  <option>trace_task_names</option>
                  <option>trace_stats</option>
                  <option>trace_metadata</option>
                  <option>sys_calls</option>
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
