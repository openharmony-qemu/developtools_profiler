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
import { SpMetrics } from '../../../dist/trace/component/SpMetrics.js';

describe('SpMetrics Test', () => {
  let spMetrics = new SpMetrics();
  it('SpMetricsTest01', function () {
    expect(SpMetrics.initElements).toBeUndefined();
  });
  it('SpMetricsTest02', function () {
    expect(spMetrics.metric).toBe('');
  });
  it('SpMetricsTest03', function () {
    spMetrics.metric = true;
    expect(spMetrics.metric).toBe('');
  });
  it('SpMetricsTest04', function () {
    expect(spMetrics.metricResult).toBe('');
  });
  it('SpMetricsTest05', function () {
    spMetrics.metricResult = true;
    expect(spMetrics.metricResult).toBeTruthy();
  });

  it('SpMetricsTest06', function () {
    expect(spMetrics.attributeChangedCallback('metric')).toBeUndefined();
  });

  it('SpMetricsTest07', function () {
    expect(spMetrics.attributeChangedCallback('metricResult')).toBeUndefined();
  });

  it('SpMetricsTest08', function () {
    expect(spMetrics.initHtml()).toMatchInlineSnapshot(`
"
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
        ::-webkit-scrollbar
        {
          width: 8px;
          background-color: var(--dark-background3,#FFFFFF);
        }
        ::-webkit-scrollbar-thumb
        {
          border-radius: 6px;
          background-color: var(--dark-background7,rgba(0,0,0,0.1));
        }
        .sp-load-metric{
            width: 95%;
            bottom: 0;
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
        "
`);
  });

  it('SpMetricsTest09', function () {
    expect(spMetrics.reset()).toBeUndefined();
  });

  it('SpMetricsTest10', function () {
    expect(spMetrics.connectedCallback()).toBeUndefined();
  });

  it('SpMetricsTest11', function () {
    expect(spMetrics.disconnectedCallback()).toBeUndefined();
  });

  it('SpMetricsTest12', function () {
    expect(spMetrics.initMetricSelectOption()).toBeUndefined();
  });

  it('SpMetricsTest13', function () {
    expect(spMetrics.initMetricDataHandle()).toBeUndefined();
  });
});
