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
import '../../../StackBar.js'
import { getTabRunningPercent, querySearchFuncData, queryCpuFreqUsageData, queryCpuFreqFilterId } from '../../../../database/SqlLite.js';
import { Utils } from '../../base/Utils.js';
import { log } from '../../../../../log/Log.js';
import { resizeObserver } from '../SheetUtils.js';

@element('tabpane-freqdatacut')
export class TabPaneFreqDataCut extends BaseElement {
    private threadStatesTbl: LitTable | null | undefined;
    private threadStatesTblSource: Array<SelectionData> = [];
    private currentSelectionParam: SelectionParam | any;
    private threadStatesDIV: Element | null | undefined;
    private initData: Array<SelectionData> = [];

    set data(threadStatesParam: SelectionParam | any) {
        if (this.currentSelectionParam === threadStatesParam) {
            return;
        }
        this.currentSelectionParam = threadStatesParam;
        this.threadStatesTblSource = [];
        this.threadStatesTbl!.recycleDataSource = [];
        let tableValue: any = this.threadStatesTbl;
        tableValue.value = [];
        let divRoot1: any = this.shadowRoot?.querySelector('#dataCutThreadId');
        divRoot1.value = '';
        let divRoot2: any = this.shadowRoot?.querySelector('#dataCutThreadFunc');
        divRoot2.value = '';
        getTabRunningPercent(threadStatesParam.threadIds, threadStatesParam.leftNs, threadStatesParam.rightNs).then((result) => {
            queryCpuFreqFilterId().then(r => {
                let IdMap = new Map();
                let queryId = new Array();
                for (let i = 0; i < r.length; i++) {
                    queryId.push(r[i].id);
                    IdMap.set(r[i].id, r[i].cpu);
                }
                queryCpuFreqUsageData(queryId).then((res) => {
                    if (result != null && result.length > 0) {
                        log('getTabRunningPercent result size : ' + result.length);
                        let sum = 0;
                        let dealArr = new Array();
                        for (let i of res) {
                            dealArr.push({ 'startNS': i.startNS + threadStatesParam.recordStartNs, 'dur': i.dur, 'value': i.value, 'cpu': IdMap.get(i.filter_id) });
                        }
                        let targetList = new Array();
                        let cpuArr = new Array();
                        let finalResultArr = new Array();
                        finalResultArr.push({ 'thread': '', 'count': 0, 'cpu': '', 'freq': '', 'dur': 0, 'percent': '100.00', 'state': 'Running', children: new Array() });
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
                                    finalResultArr[0].thread = finalResultArr[0].thread == '' ? e.tid + '_' + e.thread : finalResultArr[0].thread;
                                    finalResultArr[0].children.push({ 'thread': e.tid + '_' + e.thread, 'count': 0, 'cpu': e.cpu, 'freq': '', 'dur': 0, 'percent': 0, 'state': 'Running', children: new Array() });
                                }
                            }
                        }
                        // 用来存放数据切割之前的汇总数据
                        let resultList = new Array();
                        // 通过循环获取每个running状态线程的相关信息，此处或许可以进行算法优化
                        const tsMutiple = 1000000000;
                        for (let i = 0; i < targetList.length; i++) {
                            for (let j = 0; j < dealArr.length; j++) {
                                if (targetList[i].cpu == dealArr[j].cpu) {
                                    if (targetList[i].ts > dealArr[j].startNS) {
                                        if (targetList[i].ts < (dealArr[j].startNS + dealArr[j].dur)) {
                                            if (targetList[i].dur < (dealArr[j].startNS + dealArr[j].dur - targetList[i].ts)) {
                                                resultList.push({ 'thread': targetList[i].tid + '_' + targetList[i].thread, 'count': (dealArr[j].value * targetList[i].dur) / 1000, 'cpu': targetList[i].cpu, 'freq': dealArr[j].value, 'dur': targetList[i].dur, 'percent': targetList[i].dur / sum * 100, 'state': 'Running', 'ts': targetList[i].ts / tsMutiple });
                                                break;
                                            } else {
                                                resultList.push({ 'thread': targetList[i].tid + '_' + targetList[i].thread, 'count': (dealArr[j].value * (dealArr[j].startNS + dealArr[j].dur - targetList[i].ts)) / 1000, 'cpu': targetList[i].cpu, 'freq': dealArr[j].value, 'dur': (dealArr[j].startNS + dealArr[j].dur - targetList[i].ts), 'percent': (dealArr[j].startNS + dealArr[j].dur - targetList[i].ts) / sum * 100, 'state': 'Running', 'ts': targetList[i].ts / tsMutiple });
                                            }
                                        }
                                    } else {
                                        if ((targetList[i].ts + targetList[i].dur) > dealArr[j].startNS) {
                                            if ((targetList[i].dur + targetList[i].ts - dealArr[j].startNS) < dealArr[j].dur) {
                                                resultList.push({ 'thread': targetList[i].tid + '_' + targetList[i].thread, 'count': (dealArr[j].value * (targetList[i].dur + targetList[i].ts - dealArr[j].startNS)) / 1000, 'cpu': targetList[i].cpu, 'freq': dealArr[j].value, 'dur': (targetList[i].dur + targetList[i].ts - dealArr[j].startNS), 'percent': (targetList[i].dur + targetList[i].ts - dealArr[j].startNS) / sum * 100, 'state': 'Running', 'ts': dealArr[j].startNS / tsMutiple });
                                                break;
                                            } else {
                                                resultList.push({ 'thread': targetList[i].tid + '_' + targetList[i].thread, 'count': (dealArr[j].value * dealArr[j].dur) / 1000, 'cpu': targetList[i].cpu, 'freq': dealArr[j].value, 'dur': dealArr[j].dur, 'percent': dealArr[j].dur / sum * 100, 'state': 'Running', 'ts': dealArr[j].startNS / tsMutiple });
                                            }
                                        } else {
                                            resultList.push({ 'thread': targetList[i].tid + '_' + targetList[i].thread, 'count': 0, 'cpu': targetList[i].cpu, 'freq': 'unknown', 'dur': targetList[i].dur, 'percent': targetList[i].dur / sum * 100, 'state': 'Running', 'ts': targetList[i].ts / tsMutiple });
                                            break;
                                        }
                                    }
                                }
                            }
                        }
                        // 深拷贝，用来进行数据切割操作，避免数据污染
                        this.initData = JSON.parse(JSON.stringify(resultList));
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
                            resultList[i].ts = (resultList[i].ts * tsMutiple - threadStatesParam.recordStartNs) / tsMutiple;
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
                        this.initData = [];
                    }
                })
            })

        });
    }
    initElements(): void {
        this.threadStatesTbl = this.shadowRoot?.querySelector<LitTable>('#tb-running-percent');
        // 暂时屏蔽列排序功能，后续增加则重写排序方法
        this.threadStatesDIV = this.shadowRoot?.querySelector('#dataCut');
        this.threadStatesDIV?.children[2].children[0].addEventListener('click', (e) => {
            this.dataSingleCut(this.threadStatesDIV?.children[0], this.threadStatesDIV?.children[1], this.initData);
        })
        this.threadStatesDIV?.children[2].children[1].addEventListener('click', (e) => {
            this.dataLoopCut(this.threadStatesDIV?.children[0], this.threadStatesDIV?.children[1], this.initData);
        })
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
        #dataCut{
            display: flex;
            justify-content: space-between;
            width:100%;
            height:20px;
            margin-bottom:2px;
            align-items:center;
        }
        button{
            width:40%;
            height:100%;
            border: solid 1px #666666;
            background-color: rgba(0,0,0,0);
            border-radius:10px;
        }
        button:hover{
            background-color:#666666;
            color:white;
        }
        </style>
        <div id='dataCut'>
            <input id="dataCutThreadId" type="text" style="width: 15%;height:90%;border-radius:10px;border:solid 1px #979797;font-size:15px;text-indent:3%" placeholder="请输入线程ID" value='' />
            <input id="dataCutThreadFunc" type="text" style="width: 20%;height:90%;border-radius:10px;border:solid 1px #979797;font-size:15px;text-indent:3%" placeholder="请输入方法名" value='' />
            <div style="width:20%;height: 100%;display:flex;justify-content: space-around;">
                <button>Single</button>
                <button>Loop</button>
            </div>
        </div>
        <lit-table id="tb-running-percent" style="height: auto; overflow-x:auto;width:calc(100vw - 270px)" tree>
            <lit-table-column class="running-percent-column" width="320px" title="ThreadName" data-index="thread" key="thread" align="flex-start" retract>
            </lit-table-column>
            <lit-table-column class="running-percent-column" width="240px" title="周期起始时间(s)" data-index="ts" key="ts" align="flex-start">
            </lit-table-column>
            <lit-table-column class="running-percent-column" width="240px" title="算力消耗(Hz·ms)" data-index="count" key="count" align="flex-start">
            </lit-table-column>
            <lit-table-column class="running-percent-column" width="120px" title="CPU" data-index="cpu" key="cpu" align="flex-start">
            </lit-table-column>
            <lit-table-column class="running-percent-column" width="200px" title="Freq(KHz)" data-index="freq" key="freq" align="flex-start">
            </lit-table-column>
            <lit-table-column class="running-percent-column" width="200px" title="duration(ns)" data-index="dur" key="dur" align="flex-start">
            </lit-table-column>
            <lit-table-column class="running-percent-column" width="120px" title="Percent(%)" data-index="percent" key="percent" align="flex-start">
            </lit-table-column>
            <lit-table-column class="running-percent-column" width="240px" title="State" data-index="state" key="state" align="flex-start">
            </lit-table-column>
        </lit-table>
        `
    }
    dataLoopCut(threadId: any, threadFunc: any, resultList: any) {
        let threadIdValue = threadId.value.trim();
        let threadFuncName = threadFunc.value.trim();
        let leftNS = this.currentSelectionParam.leftNs;
        let rightNS = this.currentSelectionParam.rightNs;
        let tableValue: any = this.threadStatesTbl;
        tableValue.value = [];
        if (/^[0-9]*$/.test(threadIdValue)) {
            querySearchFuncData(threadFuncName, threadIdValue, leftNS, rightNS).then(res => {
                let displayArr = JSON.parse(JSON.stringify(resultList));
                let timeDur = this.currentSelectionParam.recordStartNs;
                let cutArr = new Array();
                // 根据线程id及方法名获取的数据，处理后用作切割时间依据，时间跨度为整个方法开始时间到末个方法开始时间
                for (let i of res) {
                    cutArr.push({ 'ts': i.startTime + timeDur });
                }
                // 将数据进行切割处理
                let finalArr = new Array();
                let finalResultArr = new Array();
                const tsMutiple = 1000000000;
                finalResultArr.push({ 'thread': displayArr[0].thread, 'ts': '', 'count': 0, 'cpu': '', 'freq': '', 'dur': 0, 'percent': 0, 'state': 'Running', children: new Array() });
                for (let i = 0; i < cutArr.length - 1; i++) {
                    for (let j = 0; j < displayArr.length; j++) {
                        displayArr[j].ts = displayArr[j].ts * tsMutiple;
                        if (displayArr[j].ts >= cutArr[i].ts) {
                            if ((displayArr[j].ts + displayArr[j].dur) <= cutArr[i + 1].ts) {
                                finalArr.push({ 'thread': displayArr[j].thread, 'count': (displayArr[j].freq * displayArr[j].dur) / 1000, 'cpu': displayArr[j].cpu, 'freq': displayArr[j].freq, 'dur': displayArr[j].dur, 'percent': displayArr[j].percent, 'state': 'Running', 'ts': (displayArr[j].ts - timeDur) / tsMutiple, 'id': i });
                            } else {
                                if (cutArr[i + 1].ts - displayArr[j].ts > 0) {
                                    finalArr.push({ 'thread': displayArr[j].thread, 'count': (displayArr[j].freq * (cutArr[i + 1].ts - displayArr[j].ts)) / 1000, 'cpu': displayArr[j].cpu, 'freq': displayArr[j].freq, 'dur': cutArr[i + 1].ts - displayArr[j].ts, 'percent': displayArr[j].percent * ((cutArr[i + 1].ts - displayArr[j].ts) / displayArr[j].dur), 'state': 'Running', 'ts': (displayArr[j].ts - timeDur) / tsMutiple, 'id': i });
                                    break;
                                }
                            }
                        } else {
                            if ((displayArr[j].ts + displayArr[j].dur) > cutArr[i + 1].ts) {
                                finalArr.push({ 'thread': displayArr[j].thread, 'count': (displayArr[j].freq * (cutArr[i + 1].ts - cutArr[i].ts)) / 1000, 'cpu': displayArr[j].cpu, 'freq': displayArr[j].freq, 'dur': cutArr[i + 1].ts - cutArr[i].ts, 'percent': displayArr[j].percent * ((cutArr[i + 1].ts - cutArr[i].ts) / displayArr[j].dur), 'state': 'Running', 'ts': (cutArr[i].ts - timeDur) / tsMutiple, 'id': i });
                            }
                            if ((displayArr[j].ts + displayArr[j].dur) > cutArr[i].ts && (displayArr[j].ts + displayArr[j].dur) < cutArr[i + 1].ts) {
                                finalArr.push({ 'thread': displayArr[j].thread, 'count': (displayArr[j].freq * (displayArr[j].dur + displayArr[j].ts - cutArr[i].ts)) / 1000, 'cpu': displayArr[j].cpu, 'freq': displayArr[j].freq, 'dur': displayArr[j].dur + displayArr[j].ts - cutArr[i].ts, 'percent': displayArr[j].percent * ((displayArr[j].dur + displayArr[j].ts - cutArr[i].ts) / displayArr[j].dur), 'state': 'Running', 'ts': (cutArr[i].ts - timeDur) / tsMutiple, 'id': i });
                            }
                        }
                    }
                    finalResultArr[0].children.push({ 'thread': displayArr[0].thread, 'ts': (cutArr[i].ts - timeDur) / tsMutiple, 'count': 0, 'cpu': '', 'freq': '', 'dur': 0, 'percent': 0, 'state': 'Running', children: new Array(), 'id': i });
                }

                for (let i = 0; i < finalArr.length; i++) {
                    for (let j = i + 1; j < finalArr.length; j++) {
                        if (finalArr[i].cpu === finalArr[j].cpu && finalArr[i].freq === finalArr[j].freq && finalArr[i].id === finalArr[j].id) {
                            finalArr[i].dur += finalArr[j].dur;
                            finalArr[i].percent += finalArr[j].percent;
                            finalArr[i].count += finalArr[j].count;
                            finalArr.splice(j, 1);
                            j--;
                        }
                    }
                    finalArr[i].percent = Number((finalArr[i].percent).toFixed(2));
                }

                let newArr1 = JSON.parse(JSON.stringify(finalResultArr[0]));
                let newArr2 = JSON.parse(JSON.stringify(finalArr));
                let finalResult = new Array(this.mergeTree(newArr1, newArr2));
                this.threadStatesTblSource = finalResult[0].children.length > 0 ? finalResult : [];
                this.threadStatesTbl!.recycleDataSource = finalResult[0].children.length > 0 ? finalResult : [];
            })
        } else {
            alert('请输入正确的线程ID');
        }
    }
    dataSingleCut(threadId: any, threadFunc: any, resultList: any) {
        let threadIdValue = threadId.value.trim();
        let threadFuncName = threadFunc.value.trim();
        let leftNS = this.currentSelectionParam.leftNs;
        let rightNS = this.currentSelectionParam.rightNs;
        let tableValue: any = this.threadStatesTbl;
        tableValue.value = [];
        if (/^[0-9]*$/.test(threadIdValue)) {
            querySearchFuncData(threadFuncName, threadIdValue, leftNS, rightNS).then(result => {
                let [...targetList] = JSON.parse(JSON.stringify(resultList));
                let timeDur = this.currentSelectionParam.recordStartNs;
                let dealArr = new Array();
                for (let i of result) {
                    if (i.startTime + timeDur + i.dur < this.currentSelectionParam.rightNs + timeDur) {
                        dealArr.push({ 'ts': i.startTime + timeDur, 'dur': i.dur });
                    }
                }
                let finalResultArr = new Array();
                finalResultArr.push({ 'thread': targetList[0].thread, 'ts': '', 'count': 0, 'cpu': '', 'freq': '', 'dur': 0, 'percent': 0, 'state': 'Running', children: new Array() });
                let resList = new Array();
                const tsMutiple = 1000000000;
                for (let i = 0; i < dealArr.length; i++) {
                    for (let j = 0; j < targetList.length; j++) {
                        targetList[j].ts = targetList[j].ts * tsMutiple;
                        if (dealArr[i].ts < targetList[j].ts) {
                            if (dealArr[i].ts + dealArr[i].dur > targetList[j].ts) {
                                if (dealArr[i].ts + dealArr[i].dur > targetList[j].ts + targetList[j].dur) {
                                    resList.push({ 'thread': targetList[i].thread, 'ts': (targetList[j].ts - timeDur) / tsMutiple, 'count': (targetList[j].freq * targetList[j].dur) / 1000, 'cpu': targetList[j].cpu, 'freq': targetList[j].freq, 'dur': targetList[j].dur, 'percent': targetList[j].percent, 'state': 'Running', 'id': i });
                                } else {
                                    resList.push({ 'thread': targetList[j].thread, 'ts': (targetList[j].ts - timeDur) / tsMutiple, 'count': (dealArr[i].ts + dealArr[i].dur - targetList[j].ts) * targetList[j].freq / 1000, 'cpu': targetList[j].cpu, 'freq': targetList[j].freq, 'dur': dealArr[i].ts + dealArr[i].dur - targetList[j].ts, 'percent': (dealArr[i].ts + dealArr[i].dur - targetList[j].ts) / targetList[j].dur * targetList[j].percent, 'state': 'Running', 'id': i });
                                    break;
                                }
                            }
                        } else {
                            if (targetList[j].ts + targetList[j].dur > dealArr[i].ts) {
                                if (targetList[j].ts + targetList[j].dur > dealArr[i].ts + dealArr[i].dur) {
                                    resList.push({ 'thread': targetList[j].thread, 'ts': (dealArr[i].ts - timeDur) / tsMutiple, 'count': dealArr[i].dur * targetList[j].freq / 1000, 'cpu': targetList[j].cpu, 'freq': targetList[j].freq, 'dur': dealArr[i].dur, 'percent': dealArr[i].dur / targetList[j].dur * targetList[j].percent, 'state': 'Running', 'id': i });
                                    break;
                                } else {
                                    resList.push({ 'thread': targetList[j].thread, 'ts': (dealArr[i].ts - timeDur) / tsMutiple, 'count': (targetList[j].ts + targetList[j].dur - dealArr[i].ts) * targetList[j].freq / 1000, 'cpu': targetList[j].cpu, 'freq': targetList[j].freq, 'dur': targetList[j].ts + targetList[j].dur - dealArr[i].ts, 'percent': (targetList[j].ts + targetList[j].dur - dealArr[i].ts) / targetList[j].dur * targetList[j].percent, 'state': 'Running', 'id': i });
                                }
                            }
                        }
                    }
                    finalResultArr[0].children.push({ 'thread': targetList[0].thread, 'ts': (dealArr[i].ts - timeDur) / tsMutiple, 'count': 0, 'cpu': '', 'freq': '', 'dur': 0, 'percent': 0, 'state': 'Running', children: new Array(), 'id': i });
                }
                // 合并相同周期内的数据
                for (let i = 0; i < resList.length; i++) {
                    for (let j = i + 1; j < resList.length; j++) {
                        if (resList[i].cpu === resList[j].cpu && resList[i].freq === resList[j].freq && resList[i].id === resList[j].id) {
                            resList[i].dur += resList[j].dur;
                            resList[i].percent += resList[j].percent;
                            resList[i].count += resList[j].count;
                            resList.splice(j, 1);
                            j--;
                        }
                    }
                    resList[i].percent = Number((resList[i].percent).toFixed(2));
                }

                let newArr1 = JSON.parse(JSON.stringify(finalResultArr[0]));
                let newArr2 = JSON.parse(JSON.stringify(resList));
                let finalResult = new Array(this.mergeTree(newArr1, newArr2));

                this.threadStatesTblSource = finalResult[0].children.length > 0 ? finalResult : [];
                this.threadStatesTbl!.recycleDataSource = finalResult[0].children.length > 0 ? finalResult : [];
            })
        } else {
            alert('请输入正确的线程ID');
        }
    }

    mergeTree(arr1: any, arr2: any) {
        for (let i = 0; i < arr1.children.length; i++) {
            // 改成map对象做标记
            let cpuArr = new Array();
            let flagMap = new Map();
            let flag = 0;
            for (let j = 0; j < arr2.length; j++) {
                if (arr1.children[i].id == arr2[j].id) {
                    if (!cpuArr.includes(arr2[j].cpu)) {
                        flagMap.set(arr2[j].cpu, flag);
                        cpuArr.push(arr2[j].cpu);
                        arr1.children[i].children.push({ 'thread': arr2[j].thread, 'count': 0, 'cpu': arr2[j].cpu, 'freq': '', 'dur': 0, 'percent': 0, 'state': 'Running', 'ts': '', children: new Array(), 'id': arr2[j].id });
                        if (arr1.children[i].children[flag].cpu == arr2[j].cpu && arr1.children[i].children[flag].id == arr2[j].id) {
                            arr1.children[i].children[flag].children.push(arr2[j]);
                            arr1.children[i].children[flag].dur += arr2[j].dur;
                            arr1.children[i].children[flag].percent += arr2[j].percent;
                            arr1.children[i].children[flag].count += arr2[j].count;
                            arr1.children[i].percent += arr1.children[i].children[flag].percent;
                            arr1.children[i].children[flag].percent = Number(arr1.children[i].children[flag].percent.toFixed(2));
                            arr1.children[i].dur += arr1.children[i].children[flag].dur;
                            arr1.children[i].count += arr1.children[i].children[flag].count;
                            flag++;
                            arr2.splice(j, 1);
                            j--;
                        }
                    } else {
                        // 利用map做数据处理
                        let count = flagMap.get(arr2[j].cpu);
                        if (arr1.children[i].children[count].cpu == arr2[j].cpu && arr1.children[i].children[count].id == arr2[j].id) {
                            arr1.children[i].children[count].children.push(arr2[j]);
                            arr1.children[i].children[count].dur += arr2[j].dur;
                            arr1.children[i].children[count].percent += arr2[j].percent;
                            arr1.children[i].children[count].count += arr2[j].count;
                            arr1.children[i].percent += arr2[j].percent;
                            arr1.children[i].children[count].percent = Number(arr2[j].percent.toFixed(2));
                            arr1.children[i].dur += arr2[j].dur;
                            arr1.children[i].count += arr2[j].count;
                            arr2.splice(j, 1);
                            j--;
                        }
                    }
                } else {
                    break;
                }
            }
            arr1.children[i].children.sort((a: any, b: any) => a.cpu - b.cpu);
            arr1.percent += arr1.children[i].percent;
            arr1.children[i].percent = Number(arr1.children[i].percent.toFixed(2));
            arr1.dur += arr1.children[i].dur;
            arr1.count += arr1.children[i].count;
        }
        arr1.percent = Number(arr1.percent.toFixed(2));
        return arr1;
    }
}