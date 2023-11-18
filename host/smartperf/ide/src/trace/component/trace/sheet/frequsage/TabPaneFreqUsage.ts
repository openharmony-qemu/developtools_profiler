/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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
import { LitTable } from '../../../../../base-ui/table/lit-table';
import { SelectionData, SelectionParam } from '../../../../bean/BoxSelection';
import '../../../StackBar.js';
import { getTabRunningPercent, queryCpuFreqUsageData, queryCpuFreqFilterId } from '../../../../database/SqlLite.js';
import { Utils } from '../../base/Utils.js';
import { resizeObserver } from '../SheetUtils.js';

@element('tabpane-frequsage')
export class TabPaneFreqUsage extends BaseElement {
  private threadStatesTbl: LitTable | null | undefined;
  private threadStatesTblSource: Array<SelectionData> = [];
  private currentSelectionParam: Selection | undefined;

  set data(threadStatesParam: SelectionParam | any) {
    if (this.currentSelectionParam === threadStatesParam) {
      return;
    }
    this.currentSelectionParam = threadStatesParam;
    this.threadStatesTblSource = [];
    this.threadStatesTbl!.recycleDataSource = [];
    let tableValue: any = this.threadStatesTbl;
    tableValue.value = [];
    getTabRunningPercent(threadStatesParam.threadIds, threadStatesParam.leftNs, threadStatesParam.rightNs).then(
      (result) => {
        queryCpuFreqFilterId().then((r) => {
          let IdMap = new Map();
          let queryId = new Array();
          for (let i = 0; i < r.length; i++) {
            queryId.push(r[i].id);
            IdMap.set(r[i].id, r[i].cpu);
          }
          queryCpuFreqUsageData(queryId).then((res) => {
            if (result != null && result.length > 0) {
              let sum = 0;
              let dealArr = new Array();
              for (let i of res) {
                dealArr.push({
                  startNS: i.startNS + threadStatesParam.recordStartNs,
                  dur: i.dur,
                  value: i.value,
                  cpu: IdMap.get(i.filter_id),
                });
              }
              let targetList = new Array();
              let cpuArr = new Array();
              let finalResultArr = new Array();
              finalResultArr.push({
                thread: '',
                count: 0,
                cpu: '',
                freq: '',
                dur: 0,
                percent: '100.00',
                state: 'Running',
                children: new Array(),
              });
              for (let e of result) {
                if (threadStatesParam.processIds.includes(e.pid) && e.state == 'Running') {
                  let process = Utils.PROCESS_MAP.get(e.pid);
                  let thread = Utils.THREAD_MAP.get(e.tid);
                  e.process = process == null || process.length == 0 ? '[NULL]' : process;
                  e.thread = thread == null || thread.length == 0 ? '[NULL]' : thread;
                  e.stateJX = e.state;
                  e.state = Utils.getEndState(e.stateJX);
                  sum += e.dur;
                  targetList.push(e);
                  if (!cpuArr.includes(e.cpu)) {
                    cpuArr.push(e.cpu);
                    finalResultArr[0].thread =
                      finalResultArr[0].thread == '' ? e.tid + '_' + e.thread : finalResultArr[0].thread;
                    finalResultArr[0].children.push({
                      thread: e.tid + '_' + e.thread,
                      count: 0,
                      cpu: e.cpu,
                      freq: '',
                      dur: 0,
                      percent: 0,
                      state: 'Running',
                      children: new Array(),
                    });
                  }
                }
              }
              let resultList = new Array();
              for (let i = 0; i < targetList.length; i++) {
                for (let j = 0; j < dealArr.length; j++) {
                  if (targetList[i].cpu == dealArr[j].cpu) {
                    if (targetList[i].ts > dealArr[j].startNS) {
                      if (targetList[i].ts < dealArr[j].startNS + dealArr[j].dur) {
                        if (targetList[i].dur < dealArr[j].startNS + dealArr[j].dur - targetList[i].ts) {
                          resultList.push({
                            thread: targetList[i].tid + '_' + targetList[i].thread,
                            count: (dealArr[j].value * targetList[i].dur) / 1000,
                            cpu: targetList[i].cpu,
                            freq: dealArr[j].value,
                            dur: targetList[i].dur,
                            percent: (targetList[i].dur / sum) * 100,
                            state: 'Running',
                            ts: targetList[i].ts,
                          });
                          break;
                        } else {
                          resultList.push({
                            thread: targetList[i].tid + '_' + targetList[i].thread,
                            count: (dealArr[j].value * (dealArr[j].startNS + dealArr[j].dur - targetList[i].ts)) / 1000,
                            cpu: targetList[i].cpu,
                            freq: dealArr[j].value,
                            dur: dealArr[j].startNS + dealArr[j].dur - targetList[i].ts,
                            percent: ((dealArr[j].startNS + dealArr[j].dur - targetList[i].ts) / sum) * 100,
                            state: 'Running',
                            ts: targetList[i].ts,
                          });
                        }
                      }
                    } else {
                      if (targetList[i].ts + targetList[i].dur > dealArr[j].startNS) {
                        if (targetList[i].dur + targetList[i].ts - dealArr[j].startNS < dealArr[j].dur) {
                          resultList.push({
                            thread: targetList[i].tid + '_' + targetList[i].thread,
                            count:
                              (dealArr[j].value * (targetList[i].dur + targetList[i].ts - dealArr[j].startNS)) / 1000,
                            cpu: targetList[i].cpu,
                            freq: dealArr[j].value,
                            dur: targetList[i].dur + targetList[i].ts - dealArr[j].startNS,
                            percent: ((targetList[i].dur + targetList[i].ts - dealArr[j].startNS) / sum) * 100,
                            state: 'Running',
                            ts: dealArr[j].startNS,
                          });
                          break;
                        } else {
                          resultList.push({
                            thread: targetList[i].tid + '_' + targetList[i].thread,
                            count: (dealArr[j].value * dealArr[j].dur) / 1000,
                            cpu: targetList[i].cpu,
                            freq: dealArr[j].value,
                            dur: dealArr[j].dur,
                            percent: (dealArr[j].dur / sum) * 100,
                            state: 'Running',
                            ts: dealArr[j].startNS,
                          });
                        }
                      } else {
                        resultList.push({
                          thread: targetList[i].tid + '_' + targetList[i].thread,
                          count: 0,
                          cpu: targetList[i].cpu,
                          freq: 'unknown',
                          dur: targetList[i].dur,
                          percent: (targetList[i].dur / sum) * 100,
                          state: 'Running',
                          ts: targetList[i].ts,
                        });
                        break;
                      }
                    }
                  }
                }
              }
              //合并同一线程内，当运行所在cpu和频点相同时，dur及percent进行累加求和，或许可以进行算法优化
              for (let i = 0; i < resultList.length; i++) {
                for (let j = i + 1; j < resultList.length; j++) {
                  if (resultList[i].cpu == resultList[j].cpu && resultList[i].freq == resultList[j].freq) {
                    resultList[i].dur += resultList[j].dur;
                    resultList[i].percent += resultList[j].percent;
                    resultList[i].count += resultList[j].count;
                    resultList.splice(j, 1);
                    j--;
                  }
                }
                resultList[i].percent = Number(resultList[i].percent.toFixed(2));
                resultList[i].ts = resultList[i].ts - threadStatesParam.recordStartNs;
              }
              finalResultArr[0].children.sort((a: any, b: any) => a.cpu - b.cpu);
              // 转成树结构数据进行展示
              for (let i = 0; i < finalResultArr[0].children.length; i++) {
                for (let j = 0; j < resultList.length; j++) {
                  if (finalResultArr[0].children[i].cpu == resultList[j].cpu) {
                    finalResultArr[0].children[i].children.push(resultList[j]);
                    finalResultArr[0].children[i].dur += resultList[j].dur;
                    finalResultArr[0].children[i].percent += resultList[j].percent;
                    finalResultArr[0].children[i].count += resultList[j].count;
                    resultList.splice(j, 1);
                    j--;
                  }
                }
                finalResultArr[0].children[i].percent = finalResultArr[0].children[i].percent.toFixed(2);
                finalResultArr[0].dur += finalResultArr[0].children[i].dur;
                finalResultArr[0].count += finalResultArr[0].children[i].count;
              }
              this.threadStatesTblSource = finalResultArr;
              this.threadStatesTbl!.recycleDataSource = finalResultArr;
            } else {
              this.threadStatesTblSource = [];
              this.threadStatesTbl!.recycleDataSource = [];
            }
          });
        });
      }
    );
  }
  initElements(): void {
    this.threadStatesTbl = this.shadowRoot?.querySelector<LitTable>('#tb-running-percent');
  }
  connectedCallback() {
    super.connectedCallback();
    resizeObserver(this.parentElement!, this.threadStatesTbl!);
  }
  initHtml(): string {
    return `
        <style>
        :host{
            padding: 10px 10px;
            display: flex;
            flex-direction: column;
        }
        </style>
        <lit-table id="tb-running-percent" style="height: auto; overflow-x:auto;width:calc(100vw - 270px)" tree>
            <lit-table-column class="running-percent-column" width="320px" title="ThreadName" data-index="thread" key="thread" align="flex-start" retract>
            </lit-table-column>
            <lit-table-column class="running-percent-column" width="240px" title="算力消耗(Hz·ms)" data-index="count" key="count" align="flex-start">
            </lit-table-column>
            <lit-table-column class="running-percent-column" width="120px" title="CPU" data-index="cpu" key="cpu" align="flex-start">
            </lit-table-column>
            <lit-table-column class="running-percent-column" width="200px" title="Freq(KHz)" data-index="freq" key="freq" align="flex-start">
            </lit-table-column>
            <lit-table-column class="running-percent-column" width="200px" title="duration(ns)" data-index="dur" key="dur" align="flex-start">
            </lit-table-column>
            <lit-table-column class="running-percent-column" width="240px" title="Percent(%)" data-index="percent" key="percent" align="flex-start">
            </lit-table-column>
            <lit-table-column class="running-percent-column" width="240px" title="State" data-index="state" key="state" align="flex-start">
            </lit-table-column>
        </lit-table>
        `;
  }
}
