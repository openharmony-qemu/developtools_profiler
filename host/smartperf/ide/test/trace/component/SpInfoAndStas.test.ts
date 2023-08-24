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
import { SpInfoAndStats } from '../../../dist/trace/component/SpInfoAndStas.js';

describe('SpInfoAndStasTest', () => {
  document.body.innerHTML = `<sp-info-and-stats id="ddd"></sp-info-and-stats>`;
  let spInfoAndStats = document.querySelector('#ddd') as SpInfoAndStats;
  it('SpInfoAndStasTest01', function () {
    let spInfoAndStats = new SpInfoAndStats();
    expect(spInfoAndStats.initElements()).toBeUndefined();
  });

  it('SpInfoAndStasTest03', function () {
    spInfoAndStats.initMetricItemData = jest.fn(() => true);
    expect(spInfoAndStats.initMetricItemData()).toBeTruthy();
  });

  it('SpInfoAndStasTest04', function () {
    let spInfoAndStats = new SpInfoAndStats();
    expect(
      spInfoAndStats.initDataTableStyle({
        children: [
          {
            length: 1,
            style: {
              backgroundColor: 'var(--dark-background5,#F6F6F6)',
            },
          },
        ],
      })
    ).toBeUndefined();
  });

  it('SpInfoAndStasTest06 ', function () {
    expect(spInfoAndStats.connectedCallback()).toBeUndefined();
  });

  it('SpInfoAndStasTest07 ', function () {
    expect(spInfoAndStats.disconnectedCallback()).toBeUndefined();
  });

  it('SpInfoAndStasTest08 ', function () {
    expect(spInfoAndStats.attributeChangedCallback([], [], [])).toBeUndefined();
  });

  it('SpInfoAndStasTest05', function () {
    expect(spInfoAndStats.initHtml()).toMatchInlineSnapshot(`
"
        <style>
            :host{
                width: 100%;
                background-color: var(--dark-background5,#F6F6F6);
                margin: 0;
                padding: 0;
            }
            .info-stats{
                display: flex;
                flex-direction: column;
                background-color: var(--dark-background5,#F6F6F6);
                position: absolute;
                top: 0;
                bottom: 0;
                left: 0;
                right: 0;
                grid-row-gap: 30px;
            }
            .metadata{
                width: 90%;
                color: #121212;
                padding: 1% 2% 0 2%;
                margin: 1% 2.5% 0 2.5%;
                border-radius: 16px;
                background-color: var(--dark-background3,#FFFFFF);
                position: relative;
            }
            #metaData-table{
                background-color: var(--dark-background5,#F6F6F6);
                margin-left: 10px;
                min-height: inherit;
                max-height: inherit;
                padding: 10px;
            }
            #stats-table{
                margin-bottom: 2%;
                margin-left: 10px;
                padding: 10px;
            }
            #dataValueResult{
                overflow-y: auto;
                background-color: var(--dark-background5,#F6F6F6);
                border-radius: 16px;
                min-height: inherit;
                max-height: inherit;
                margin-bottom: 1%;
            }
            
            #dataKeyResult{
                overflow-y: auto;
                background-color: var(--dark-background5,#F6F6F6);
                border-radius: 16px;
                min-height: inherit;
                max-height: inherit;
                margin-bottom: 2%;
            }
            p{
                 display: table-cell;
                 padding: 7px 10px 20px 10px;
                 color: #999999;
                 font-size:14px;
                 line-height: 20px;
                 font-weight: 400;
                 text-align: left;
            }
            .stats{
               flex-grow: 1;
               height: min-content;
               margin-bottom: 1%;
               max-height: 37vh;
               min-height: inherit;
               display: flex;
               flex-direction: column;
            }
            .info{
               max-height: inherit;
               min-height: inherit;
            }
            .tr{
               background-color: var(--dark-background5,#F6F6F6); 
            }
            .load-metric{
                width: 95%;
                bottom: 0;
            }

        </style>

        <div class="info-stats">
            <div class="metadata info">
                <p>System info and metadata</p>
                <div id="dataKeyResult">
                    <lit-table id="metaData-table" hideDownload>
                            <lit-table-column title="name" data-index="name" key="name" align="flex-start">
                            </lit-table-column>
                            <lit-table-column title="value" data-index="value" key="value" align="flex-start">
                            </lit-table-column>
                    </lit-table>
                </div>
                <lit-progress-bar class="load-metric"></lit-progress-bar>
            </div>
            <div class="metadata stats">
                <p>Debugging stats</p>
                <div id="dataValueResult">
                    <lit-table id="stats-table" hideDownload>
                            <lit-table-column title="name" data-index="event_name" key="name" align="flex-start">
                            </lit-table-column>
                            <lit-table-column title="value" data-index="count" key="value" align="flex-start">
                            </lit-table-column>
                            <lit-table-column title="type" data-index="stat_type" key="type" align="flex-start">
                            </lit-table-column>
                    </lit-table>
                </div>
            </div>
        </div>
        "
`);
  });

  it('SpInfoAndStasTest9', function () {
    expect(spInfoAndStats.initMetricItemData()).toBeTruthy();
  });
  it('SpInfoAndStasTest10', function () {
    expect(spInfoAndStats.initMetricItemData()).toBeTruthy();
  });
});
