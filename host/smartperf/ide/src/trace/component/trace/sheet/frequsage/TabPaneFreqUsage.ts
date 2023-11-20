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
import { LitTable, RedrawTreeForm } from '../../../../../base-ui/table/lit-table.js';
import { SelectionData, SelectionParam } from '../../../../bean/BoxSelection.js';
import '../../../StackBar.js'
import { getTabRunningPercent, queryCpuFreqUsageData, queryCpuFreqFilterId } from '../../../../database/SqlLite.js';
import { Utils } from '../../base/Utils.js';
import { resizeObserver } from '../SheetUtils.js';
import { SliceGroup } from '../../../../bean/StateProcessThread.js';

@element('tabpane-frequsage')
export class TabPaneFreqUsage extends BaseElement {
    private threadStatesTbl: LitTable | null | undefined;
    private threadStatesTblSource: Array<SelectionData> = [];
    private currentSelectionParam: Selection | undefined;

    set data(threadStatesParam: SelectionParam | any) {
        if (this.currentSelectionParam === threadStatesParam) {
            return;
        }
        this.threadStatesTbl!.loading = true;
        this.currentSelectionParam = threadStatesParam;
        this.threadStatesTblSource = [];
        this.threadStatesTbl!.recycleDataSource = [];
        let tableValue: any = this.threadStatesTbl;
        tableValue.value = [];
        // 查询框选区域内的状态为running的数据
        getTabRunningPercent(threadStatesParam.threadIds, threadStatesParam.leftNs, threadStatesParam.rightNs).then((result) => {
            // 查询该trace文件中的cpu核数及id
            queryCpuFreqFilterId().then(r => {
                // 将查询结果以键值对形式存入Map对象，仪表后续将频点数据与Cpu进行关联
                let IdMap = new Map();
                let queryId = new Array();
                for (let i = 0; i < r.length; i++) {
                    queryId.push(r[i].id);
                    IdMap.set(r[i].id, r[i].cpu);
                }
                // 通过cpu的id去查询总的cpu频点数据
                queryCpuFreqUsageData(queryId).then((res) => {
                    if (result != null && result.length > 0) {
                        let sum = 0;
                        let dealArr = new Array();
                        // 将cpu频点数据进行整合
                        for (let i of res) {
                            dealArr.push({ 'startNS': i.startNS + threadStatesParam.recordStartNs, 'dur': i.dur, 'value': i.value, 'cpu': IdMap.get(i.filter_id) });
                        }
                        let needDeal = new Map();
                        let cpuMap = new Map();
                        let pidArr = new Array();
                        let threadArr = new Array();
                        // 创建进程级的数组
                        let processArr: any = threadStatesParam.processIds.length > 1 ? [...new Set(threadStatesParam.processIds)] : threadStatesParam.processIds;
                        for (let i of processArr) {
                            pidArr.push({ 'process': Utils.PROCESS_MAP.get(i) == null ? 'Process ' + i : Utils.PROCESS_MAP.get(i) + ' ' + i, 'thread': Utils.PROCESS_MAP.get(i) == null ? 'Process ' + i : Utils.PROCESS_MAP.get(i) + ' ' + i, 'pid': i, 'tid': '', 'count': 0, 'cpu': '', 'freq': '', 'dur': 0, 'percent': 0, 'state': 'Running', children: new Array() });
                        }
                        // 将running线程数据存到map中
                        for (let e of result) {
                            if (processArr.includes(e.pid) && e.state == 'Running') {
                                if (needDeal.get(e.pid + '_' + e.tid) == undefined) {
                                    threadArr.push({ 'process': Utils.PROCESS_MAP.get(e.pid) == null ? 'Process ' + e.pid : Utils.PROCESS_MAP.get(e.pid) + ' ' + e.pid, 'thread': Utils.THREAD_MAP.get(e.tid) + ' ' + e.tid, 'pid': e.pid, 'tid': e.tid, 'count': 0, 'cpu': '', 'freq': '', 'dur': 0, 'percent': 0, 'state': 'Running', children: new Array() });
                                    needDeal.set(e.pid + '_' + e.tid, new Array());
                                }
                                let arr = needDeal.get(e.pid + '_' + e.tid);
                                let process = Utils.PROCESS_MAP.get(e.pid);
                                let thread = Utils.THREAD_MAP.get(e.tid);
                                e.process = process == null || process.length == 0 ? '[NULL]' : process;
                                e.thread = thread == null || thread.length == 0 ? '[NULL]' : thread;
                                e.stateJX = e.state;
                                e.state = Utils.getEndState(e.stateJX);
                                sum += e.dur;
                                arr.push(e);
                            }
                        }
                        // 整理running线程数据、cpu层级信息
                        this.mergeFreqData(needDeal, cpuMap, dealArr, sum, threadStatesParam);
                        // 将频点数据放置到对应cpu层级下
                        this.mergeCpuData(cpuMap, needDeal);
                        // 将cpu层级数据放置到线程分组下
                        this.mergeThreadData(threadArr, cpuMap);
                        // 将线程层级数据放置到进程级分组下
                        this.mergePidData(pidArr, threadArr);
                        // 百分比保留两位小数
                        this.fixedDeal(pidArr)
                        this.threadStatesTblSource = pidArr;
                        this.threadStatesTbl!.recycleDataSource = pidArr;
                        this.threadStatesTbl!.loading = false;
                        this.theadClick(pidArr);
                    } else {
                        this.threadStatesTblSource = [];
                        this.threadStatesTbl!.recycleDataSource = [];
                        this.threadStatesTbl!.loading = false;
                    }
                })
            })

        })
    }

    private theadClick(data: Array<SliceGroup>) {
        let labels = this.threadStatesTbl?.shadowRoot?.querySelector('.th > .td')!.querySelectorAll('label');
        if (labels) {
            for (let i = 0; i < labels.length; i++) {
                let label = labels[i].innerHTML;
                labels[i].addEventListener('click', (e) => {
                    if (label.includes('Process') && i === 0) {
                        this.threadStatesTbl!.setStatus(data, false);
                        this.threadStatesTbl!.recycleDs = this.threadStatesTbl!.meauseTreeRowElement(data, RedrawTreeForm.Retract);
                    } else if (label.includes('Thread') && i === 1) {
                        for (let item of data) {
                            item.status = true;
                            if (item.children != undefined && item.children.length > 0) {
                                this.threadStatesTbl!.setStatus(item.children, false);
                            }
                        }
                        this.threadStatesTbl!.recycleDs = this.threadStatesTbl!.meauseTreeRowElement(data, RedrawTreeForm.Retract);
                    } else if (label.includes('CPU') && i === 2) {
                        this.threadStatesTbl!.setStatus(data, true);
                        this.threadStatesTbl!.recycleDs = this.threadStatesTbl!.meauseTreeRowElement(data, RedrawTreeForm.Expand);
                    }
                });
            }
        }
    }
    mergeFreqData(needDeal: any, cpuMap: any, dealArr: any, sum: number, threadStatesParam: SelectionParam | any) {
        needDeal.forEach((value: any, key: any) => {
            let resultList = new Array();
            cpuMap.set(key, new Array());
            let cpuArr = new Array();
            const multiple = 1000;
            for (let i = 0; i < value.length; i++) {
                if (!cpuArr.includes(value[i].cpu)) {
                    cpuArr.push(value[i].cpu);
                    cpuMap.get(key).push({ 'process': Utils.PROCESS_MAP.get(value[i].pid) == null ? 'Process ' + value[i].pid : Utils.PROCESS_MAP.get(value[i].pid) + ' ' + value[i].pid, 'thread': value[i].tid + '_' + Utils.THREAD_MAP.get(value[i].tid), 'pid': value[i].pid, 'tid': value[i].tid, 'count': 0, 'cpu': value[i].cpu, 'freq': '', 'dur': 0, 'percent': 0, 'state': 'Running', children: new Array() });
                }
                for (let j = 0; j < dealArr.length; j++) {
                    if (value[i].cpu == dealArr[j].cpu) {
                        if (value[i].ts > dealArr[j].startNS) {
                            if (value[i].ts < (dealArr[j].startNS + dealArr[j].dur)) {
                                if (value[i].dur < (dealArr[j].startNS + dealArr[j].dur - value[i].ts)) {
                                    resultList.push({ 'thread': value[i].tid + '_' + value[i].thread, 'count': (dealArr[j].value * value[i].dur) / multiple, 'cpu': value[i].cpu, 'freq': dealArr[j].value, 'dur': value[i].dur, 'percent': value[i].dur / sum * 100, 'state': 'Running', 'ts': value[i].ts });
                                    break;
                                } else {
                                    resultList.push({ 'thread': value[i].tid + '_' + value[i].thread, 'count': (dealArr[j].value * (dealArr[j].startNS + dealArr[j].dur - value[i].ts)) / multiple, 'cpu': value[i].cpu, 'freq': dealArr[j].value, 'dur': (dealArr[j].startNS + dealArr[j].dur - value[i].ts), 'percent': (dealArr[j].startNS + dealArr[j].dur - value[i].ts) / sum * 100, 'state': 'Running', 'ts': value[i].ts });
                                }
                            }
                        } else {
                            if ((value[i].ts + value[i].dur) > dealArr[j].startNS) {
                                if ((value[i].dur + value[i].ts - dealArr[j].startNS) < dealArr[j].dur) {
                                    resultList.push({ 'thread': value[i].tid + '_' + value[i].thread, 'count': (dealArr[j].value * (value[i].dur + value[i].ts - dealArr[j].startNS)) / multiple, 'cpu': value[i].cpu, 'freq': dealArr[j].value, 'dur': (value[i].dur + value[i].ts - dealArr[j].startNS), 'percent': (value[i].dur + value[i].ts - dealArr[j].startNS) / sum * 100, 'state': 'Running', 'ts': dealArr[j].startNS });
                                    break;
                                } else {
                                    resultList.push({ 'thread': value[i].tid + '_' + value[i].thread, 'count': (dealArr[j].value * dealArr[j].dur) / multiple, 'cpu': value[i].cpu, 'freq': dealArr[j].value, 'dur': dealArr[j].dur, 'percent': dealArr[j].dur / sum * 100, 'state': 'Running', 'ts': dealArr[j].startNS });
                                }
                            } else {
                                resultList.push({ 'thread': value[i].tid + '_' + value[i].thread, 'count': 0, 'cpu': value[i].cpu, 'freq': 'unknown', 'dur': value[i].dur, 'percent': value[i].dur / sum * 100, 'state': 'Running', 'ts': value[i].ts });
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
                resultList[i].percent = Number((resultList[i].percent).toFixed(2));
                resultList[i].ts = resultList[i].ts - threadStatesParam.recordStartNs;
            }
            resultList.sort((a, b) => b.count - a.count);
            cpuMap.get(key).sort((a: any, b: any) => a.cpu - b.cpu);
            needDeal.set(key, resultList);
        })
    }
    mergeCpuData(cpuMap: any, needDeal: any) {
        cpuMap.forEach((value: any, key: any) => {
            let arr = needDeal.get(key);
            for (let i = 0; i < value.length; i++) {
                for (let j = 0; j < arr.length; j++) {
                    if (arr[j].cpu == value[i].cpu) {
                        value[i].children.push(arr[j]);
                        value[i].count += arr[j].count;
                        value[i].dur += arr[j].dur;
                        value[i].percent += arr[j].percent;
                    }
                }
                value[i].percent = Number(value[i].percent.toFixed(2));
            }
        });
    }
    mergeThreadData(threadArr: any, cpuMap: any) {
        for (let i = 0; i < threadArr.length; i++) {
            let cpuMapData = cpuMap.get(threadArr[i].pid + '_' + threadArr[i].tid);
            for (let j = 0; j < cpuMapData.length; j++) {
                threadArr[i].children.push(cpuMapData[j]);
                threadArr[i].count += cpuMapData[j].count;
                threadArr[i].dur += cpuMapData[j].dur;
                threadArr[i].percent += cpuMapData[j].percent;
            }
            threadArr[i].percent = Number(threadArr[i].percent.toFixed(2));
        }
    }
    mergePidData(pidArr: any, threadArr: any) {
        for (let i = 0; i < pidArr.length; i++) {
            for (let j = 0; j < threadArr.length; j++) {
                if (pidArr[i].pid == threadArr[j].pid) {
                    pidArr[i].children.push(threadArr[j]);
                    pidArr[i].count += threadArr[j].count;
                    pidArr[i].dur += threadArr[j].dur;
                    pidArr[i].percent += threadArr[j].percent;
                }
            }
            pidArr[i].percent = Number(pidArr[i].percent.toFixed(2));
        }
    }
    fixedDeal(arr: any) {
        if (arr == undefined) {
            return;
        }
        for (let i = 0; i < arr.length; i++) {
            arr[i].percent = arr[i].percent > 100 ? 100 : arr[i].percent;
            arr[i].percent = arr[i].percent.toFixed(2);
            this.fixedDeal(arr[i].children);
        }
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
            <lit-table-column class="running-percent-column" width="320px" title="Process/Thread/CPU" data-index="thread" key="thread" align="flex-start" retract>
            </lit-table-column>
            <lit-table-column class="running-percent-column" width="120px" title="CPU" data-index="cpu" key="cpu" align="flex-start">
            </lit-table-column>
            <lit-table-column class="running-percent-column" width="240px" title="算力消耗(Hz·ms)" data-index="count" key="count" align="flex-start">
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
        `
    }
    sortByColumn(treadStateDetail: any) {
        function compare(property: any, treadStatesSort: any, type: any) {
            return function (threadStatesLeftData: SelectionData | any, threadStatesRightData: SelectionData | any) {
                if (threadStatesLeftData.process == ' ' || threadStatesRightData.process == ' ') {
                    return 0;
                }
                if (type === 'number') {
                    return treadStatesSort === 2
                        ? parseFloat(threadStatesRightData[property]) - parseFloat(threadStatesLeftData[property])
                        : parseFloat(threadStatesLeftData[property]) - parseFloat(threadStatesRightData[property]);
                } else {
                    if (threadStatesRightData[property] > threadStatesLeftData[property]) {
                        return treadStatesSort === 2 ? 1 : -1;
                    } else if (threadStatesRightData[property] == threadStatesLeftData[property]) {
                        return 0;
                    } else {
                        return treadStatesSort === 2 ? -1 : 1;
                    }
                }
            };
        }

        if (treadStateDetail.key === 'name' || treadStateDetail.key === 'thread' || treadStateDetail.key === 'state') {
            this.threadStatesTblSource.sort(compare(treadStateDetail.key, treadStateDetail.sort, 'string'));
        } else {
            this.threadStatesTblSource.sort(compare(treadStateDetail.key, treadStateDetail.sort, 'number'));
        }
        this.threadStatesTbl!.recycleDataSource = this.threadStatesTblSource;
    }

}