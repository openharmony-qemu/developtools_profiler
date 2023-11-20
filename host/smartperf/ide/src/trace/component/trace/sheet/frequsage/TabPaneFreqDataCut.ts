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
import { SelectionData, SelectionParam } from '../../../../bean/BoxSelection';
import '../../../StackBar.js'
import { getTabRunningPercent, querySearchFuncData, queryCpuFreqUsageData, queryCpuFreqFilterId } from '../../../../database/SqlLite.js';
import { Utils } from '../../base/Utils.js';
import { resizeObserver } from '../SheetUtils.js';
import { SliceGroup } from '../../../../bean/StateProcessThread.js';

@element('tabpane-freqdatacut')
export class TabPaneFreqDataCut extends BaseElement {
    private threadStatesTbl: LitTable | null | undefined;
    private threadStatesTblSource: Array<SelectionData> = [];
    private currentSelectionParam: SelectionParam | any;
    private threadStatesDIV: Element | null | undefined;
    private initData: any = new Map();
    private processArr: any = new Array();
    private threadArr: any = new Array();
    private originData: any = new Array();

    set data(threadStatesParam: SelectionParam | any) {
        if (this.currentSelectionParam === threadStatesParam) {
            return;
        }
        this.currentSelectionParam = threadStatesParam;
        this.threadStatesTblSource = [];
        this.threadStatesTbl!.recycleDataSource = [];
        this.threadStatesTbl!.loading = true;
        this.initData = new Map();
        this.originData = new Array();
        let tableValue: any = this.threadStatesTbl;
        tableValue.value = [];
        let divRoot1: any = this.shadowRoot?.querySelector('#dataCutThreadId');
        divRoot1.value = '';
        let divRoot2: any = this.shadowRoot?.querySelector('#dataCutThreadFunc');
        divRoot2.value = '';
        // 查询running状态线程数据
        getTabRunningPercent(threadStatesParam.threadIds, threadStatesParam.leftNs, threadStatesParam.rightNs).then((result) => {
            // 查询cpu及id信息
            queryCpuFreqFilterId().then(r => {
                // 以键值对形式将cpu及id进行对应，后续会将频点数据与其对应cpu进行整合
                let IdMap = new Map();
                let queryId = new Array();
                for (let i = 0; i < r.length; i++) {
                    queryId.push(r[i].id);
                    IdMap.set(r[i].id, r[i].cpu);
                }
                // 通过id去查询频点数据
                queryCpuFreqUsageData(queryId).then((res) => {
                    if (result != null && result.length > 0) {
                        let sum = 0;
                        let dealArr = new Array();
                        for (let i of res) {
                            dealArr.push({ 'startNS': i.startNS + threadStatesParam.recordStartNs, 'dur': i.dur, 'value': i.value, 'cpu': IdMap.get(i.filter_id) });
                        }
                        let needDeal = new Map();
                        let cpuMap = new Map();
                        let pidArr = new Array();
                        let threadArr = new Array();
                        // 整理进程级的数组信息
                        let processArr:any = threadStatesParam.processIds.length > 1 ? [...new Set(threadStatesParam.processIds)] : threadStatesParam.processIds;
                        for (let i of processArr) {
                            pidArr.push({ 'process': Utils.PROCESS_MAP.get(i) == null ? 'Process ' + i : Utils.PROCESS_MAP.get(i) + ' ' + i, 'thread': Utils.PROCESS_MAP.get(i) == null ? 'Process ' + i : Utils.PROCESS_MAP.get(i) + ' ' + i, 'pid': i, 'tid': '', 'count': 0, 'cpu': '', 'freq': '', 'dur': 0, 'percent': 0, 'state': 'Running', children: new Array() });
                        }
                        // 拷贝给私有属性，以便后续进行数据切割时免除整理进程层级数据
                        this.processArr = JSON.parse(JSON.stringify(pidArr));
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
                        // 拷贝给私有属性，以便后续进行数据切割时免除整理线程层级数据
                        this.threadArr = JSON.parse(JSON.stringify(threadArr));
                        // 整理running线程数据、cpu层级信息
                        this.mergeFreqData(needDeal, cpuMap, dealArr, sum, threadStatesParam);
                        // 将频点数据放置到对应cpu层级下
                        this.mergeCpuData(cpuMap, needDeal);
                        // 将cpu层级数据放置到线程分组下
                        this.mergeThreadData(threadArr, cpuMap);
                        // 原始数据拷贝一份，后续切割后可以放置在对应的周期前以供查看
                        this.originData = JSON.parse(JSON.stringify(threadArr));
                        // 将线程层级数据放置到进程级分组下
                        this.mergePidData(pidArr, threadArr);
                        this.threadClick(pidArr);
                        this.threadStatesTbl!.loading = false;
                    } else {
                        this.threadStatesTblSource = [];
                        this.threadStatesTbl!.recycleDataSource = [];
                        this.threadStatesTbl!.loading = false;
                    }
                })
            });
        })
    }
    private threadClick(data: Array<SliceGroup>) {
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
                    } else if (label.includes('Cycle') && i === 2) {
                        for (let item of data) {
                            item.status = true;
                            for (let value of item.children ? item.children : []) {
                                value.status = true;
                                if (value.children != undefined && value.children.length > 0) {
                                    this.threadStatesTbl!.setStatus(value.children, false);
                                }
                            }
                        }
                        this.threadStatesTbl!.recycleDs = this.threadStatesTbl!.meauseTreeRowElement(data, RedrawTreeForm.Retract);
                    } else if (label.includes('CPU') && i === 3) {
                        this.threadStatesTbl!.setStatus(data, true);
                        this.threadStatesTbl!.recycleDs = this.threadStatesTbl!.meauseTreeRowElement(data, RedrawTreeForm.Expand);
                    }
                });
            }
        }
    }

    initElements(): void {
        this.threadStatesTbl = this.shadowRoot?.querySelector<LitTable>('#tb-running-percent');
        // 暂时屏蔽列排序功能，后续增加则重写排序方法
        this.threadStatesDIV = this.shadowRoot?.querySelector('#dataCut');
        this.threadStatesDIV?.children[2].children[0].addEventListener('click', (e) => {
            this.threadStatesTbl!.loading = true;
            this.dataSingleCut(this.threadStatesDIV?.children[0], this.threadStatesDIV?.children[1], this.initData);
        })
        this.threadStatesDIV?.children[2].children[1].addEventListener('click', (e) => {
            this.threadStatesTbl!.loading = true;
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
            <input id="dataCutThreadId" type="text" style="width: 15%;height:90%;border-radius:10px;border:solid 1px #979797;font-size:15px;text-indent:3%" placeholder="Please input threadId" value='' />
            <input id="dataCutThreadFunc" type="text" style="width: 20%;height:90%;border-radius:10px;border:solid 1px #979797;font-size:15px;text-indent:3%" placeholder="Please input funcName" value='' />
            <div style="width:20%;height: 100%;display:flex;justify-content: space-around;">
                <button>Single</button>
                <button>Loop</button>
            </div>
        </div>
        <lit-table id="tb-running-percent" style="height: auto; overflow-x:auto;width:calc(100vw - 270px);margin-top:5px" tree>
            <lit-table-column class="running-percent-column" width="320px" title="Process/Thread/Cycle/CPU" data-index="thread" key="thread" align="flex-start" retract>
            </lit-table-column>
            <lit-table-column class="running-percent-column" width="240px" title="周期起始时间(s)" data-index="ts" key="ts" align="flex-start">
            </lit-table-column>
            <lit-table-column class="running-percent-column" width="120px" title="CPU" data-index="cpu" key="cpu" align="flex-start">
            </lit-table-column>
            <lit-table-column class="running-percent-column" width="240px" title="算力消耗(Hz·ms)" data-index="count" key="count" align="flex-start">
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
    // single方式切割数据功能
    dataSingleCut(threadId: any, threadFunc: any, resultList: any) {
        let threadIdValue = threadId.value.trim();
        let threadFuncName = threadFunc.value.trim();
        let leftNS = this.currentSelectionParam.leftNs;
        let rightNS = this.currentSelectionParam.rightNs;
        let tableValue: any = this.threadStatesTbl;
        tableValue.value = [];
        if (/^[0-9]*$/.test(threadIdValue) && threadIdValue != '' && threadFuncName != '') {
            threadId.style.border = '1px solid rgb(151,151,151)';
            threadFunc.style.border = '1px solid rgb(151,151,151)';
            // 根据用户输入的线程ID，方法名去查询数据库，得到对应的方法起始时间，持续时间等数据，以便作为依据进行后续数据切割
            querySearchFuncData(threadFuncName, Number(threadIdValue), leftNS, rightNS).then(result => {
                if (result != null && result.length > 0) {
                    let targetMap = new Map();
                    // 新创建map对象接收传过来的实参map
                    resultList.forEach((item: any, key: any) => {
                        targetMap.set(key, JSON.parse(JSON.stringify(item)));
                    })
                    let timeDur = this.currentSelectionParam.recordStartNs;
                    // 周期切割依据数据整理
                    let dealArr = new Array();
                    for (let i of result) {
                        if (i.startTime + timeDur + i.dur < this.currentSelectionParam.rightNs + timeDur) {
                            dealArr.push({ 'ts': i.startTime + timeDur, 'dur': i.dur });
                        }
                    }
                    let cycleMap = new Map();
                    targetMap.forEach((item, key) => {
                        cycleMap.set(key, new Array());
                        for (let i = 0; i < dealArr.length; i++) {
                            let cpuArr = new Array();
                            let cpuMap = new Map();
                            let resList = new Array();
                            // 时间由纳秒转换为秒的倍数
                            const multiple = 1000000000;
                            // 算力倍数值
                            const countMutiple = 1000;
                            cpuMap.set(key, new Array());
                            cycleMap.get(key).push({ 'thread': item[0].thread, 'ts': (dealArr[i].ts - timeDur) / multiple, 'count': 0, 'cpu': '', 'freq': '', 'dur': 0, 'percent': 0, 'state': 'Running', children: new Array() });
                            let value = JSON.parse(JSON.stringify(item));
                            for (let j = 0; j < value.length; j++) {
                                value[j].ts = value[j].ts * multiple;
                                value[j].freq = value[j].freq == 'unknown'? 0 : value[j].freq;
                                if (!cpuArr.includes(value[j].cpu)) {
                                    cpuArr.push(value[j].cpu);
                                    cpuMap.get(key).push({ 'thread': value[j].thread, 'count': 0, 'cpu': value[j].cpu, 'freq': '', 'dur': 0, 'percent': 0, 'state': 'Running', children: new Array() });
                                }
                                if (dealArr[i].ts < value[j].ts) {
                                    if (dealArr[i].ts + dealArr[i].dur > value[j].ts) {
                                        if (dealArr[i].ts + dealArr[i].dur > value[j].ts + value[j].dur) {
                                            resList.push({ 'thread': value[j].thread, 'ts': (value[j].ts - timeDur) / multiple, 'count': (value[j].freq * value[j].dur) / countMutiple, 'cpu': value[j].cpu, 'freq': value[j].freq, 'dur': value[j].dur, 'percent': value[j].percent, 'state': 'Running', 'id': i });
                                        } else {
                                            resList.push({ 'thread': value[j].thread, 'ts': (value[j].ts - timeDur) / multiple, 'count': (dealArr[i].ts + dealArr[i].dur - value[j].ts) * value[j].freq / countMutiple, 'cpu': value[j].cpu, 'freq': value[j].freq, 'dur': dealArr[i].ts + dealArr[i].dur - value[j].ts, 'percent': (dealArr[i].ts + dealArr[i].dur - value[j].ts) / value[j].dur * value[j].percent, 'state': 'Running', 'id': i });
                                            break;
                                        }
                                    }
                                } else {
                                    if (value[j].ts + value[j].dur > dealArr[i].ts) {
                                        if (value[j].ts + value[j].dur > dealArr[i].ts + dealArr[i].dur) {
                                            resList.push({ 'thread': value[j].thread, 'ts': (dealArr[i].ts - timeDur) / multiple, 'count': dealArr[i].dur * value[j].freq / countMutiple, 'cpu': value[j].cpu, 'freq': value[j].freq, 'dur': dealArr[i].dur, 'percent': dealArr[i].dur / value[j].dur * value[j].percent, 'state': 'Running', 'id': i });
                                            break;
                                        } else {
                                            resList.push({ 'thread': value[j].thread, 'ts': (dealArr[i].ts - timeDur) / multiple, 'count': (value[j].ts + value[j].dur - dealArr[i].ts) * value[j].freq / countMutiple, 'cpu': value[j].cpu, 'freq': value[j].freq, 'dur': value[j].ts + value[j].dur - dealArr[i].ts, 'percent': (value[j].ts + value[j].dur - dealArr[i].ts) / value[j].dur * value[j].percent, 'state': 'Running', 'id': i });
                                        }
                                    }
                                }
                            }
                            this.mergeData(resList);
                            resList.sort((a, b) => b.count - a.count);
                            cpuMap.get(key).sort((a: any, b: any) => a.cpu - b.cpu);
                            cpuMap.get(key).forEach((item: any) => {
                                for (let s = 0; s < resList.length; s++) {
                                    if (item.cpu == resList[s].cpu) {
                                        item.children.push(resList[s]);
                                        item.count += resList[s].count;
                                        item.dur += resList[s].dur;
                                        item.percent += resList[s].percent;
                                    }
                                }
                                item.percent = Number(item.percent.toFixed(2));
                            });
                            // 将cpu数据放置到对应周期层级下
                            this.mergeCycleData(cycleMap.get(key)[i], cpuMap.get(key));
                        }
                    });
                    // 拷贝线程数组，防止数据污染
                    let threadArr = JSON.parse(JSON.stringify(this.threadArr));
                    // 拷贝进程数组，防止数据污染
                    let processArr = JSON.parse(JSON.stringify(this.processArr));
                    // 将周期层级防止到线程层级下
                    this.mergeThreadData(threadArr, cycleMap);
                    let totalData = JSON.parse(JSON.stringify(this.originData));
                    // 将原始数据放置到对应的线程层级下，周期数据前
                    this.mergeTotalData(threadArr, totalData);
                    // 合并数据到进程层级下
                    this.mergePidData(processArr, threadArr);
                    this.fixedDeal(processArr);
                    this.threadStatesTblSource = processArr;
                    this.threadStatesTbl!.recycleDataSource = processArr;
                    this.threadStatesTbl!.loading = false;
                    this.threadClick(processArr);
                } else {
                    this.threadStatesTblSource = [];
                    this.threadStatesTbl!.recycleDataSource = [];
                    this.threadStatesTbl!.loading = false;
                }
            })
        } else {
            this.threadStatesTbl!.loading = false;
            if(threadIdValue == ''){
                threadId.style.border = '1px solid rgb(255,0,0)';
                threadId.setAttribute('placeholder', 'Please input threadId!');
            }
            if(threadFuncName == ''){
                threadFunc.style.border = '1px solid rgb(255,0,0)';
                threadFunc.setAttribute('placeholder', 'Please input funcName!');
            }
            if(/^[0-9]*$/.test(threadIdValue) == false){
                threadId.style.border = '1px solid rgb(255,0,0)';
                threadId.setAttribute('placeholder', 'Please check the input!');
            }
        }
    }
    // Loop方式切割数据功能
    dataLoopCut(threadId: any, threadFunc: any, resultList: any) {
        let threadIdValue = threadId.value.trim();
        let threadFuncName = threadFunc.value.trim();
        let leftNS = this.currentSelectionParam.leftNs;
        let rightNS = this.currentSelectionParam.rightNs;
        let tableValue: any = this.threadStatesTbl;
        tableValue.value = [];
        if (/^[0-9]*$/.test(threadIdValue) && threadIdValue != '' && threadFuncName != '') {
            threadId.style.border = '1px solid rgb(151,151,151)';
            threadFunc.style.border = '1px solid rgb(151,151,151)';
            querySearchFuncData(threadFuncName, Number(threadIdValue), leftNS, rightNS).then(res => {
                if (res != null && res.length > 0) {
                    let targetMap = new Map();
                    // 新创建map对象接收传过来的实参map
                    resultList.forEach((item: any, key: any) => {
                        targetMap.set(key, JSON.parse(JSON.stringify(item)));
                    })
                    let timeDur = this.currentSelectionParam.recordStartNs;
                    let cutArr = new Array();
                    // 根据线程id及方法名获取的数据，处理后用作切割时间依据，时间跨度为整个方法开始时间到末个方法开始时间
                    for (let i of res) {
                        cutArr.push({ 'ts': i.startTime + timeDur });
                    }
                    let cycleMap = new Map();
                    targetMap.forEach((item, key) => {
                        cycleMap.set(key, new Array());
                        for (let i = 0; i < cutArr.length - 1; i++) {
                            let cpuArr = new Array();
                            let cpuMap = new Map();
                            let resList = new Array();
                            // 时间由纳秒转换为秒的倍数
                            const multiple = 1000000000;
                            // 算力倍数值
                            const countMutiple = 1000;
                            cpuMap.set(key, new Array());
                            cycleMap.get(key).push({ 'thread': item[0].thread, 'ts': (cutArr[i].ts - timeDur) / multiple, 'count': 0, 'cpu': '', 'freq': '', 'dur': 0, 'percent': 0, 'state': 'Running', children: new Array() });
                            let value = JSON.parse(JSON.stringify(item));
                            for (let j = 0; j < value.length; j++) {
                                value[j].ts = value[j].ts * multiple;
                                value[j].freq = value[j].freq == 'unknown'? 0 : value[j].freq;
                                if (!cpuArr.includes(value[j].cpu)) {
                                    cpuArr.push(value[j].cpu);
                                    cpuMap.get(key).push({ 'thread': value[j].thread, 'count': 0, 'cpu': value[j].cpu, 'freq': '', 'dur': 0, 'percent': 0, 'state': 'Running', children: new Array() });
                                }
                                if (value[j].ts >= cutArr[i].ts) {
                                    if ((value[j].ts + value[j].dur) <= cutArr[i + 1].ts) {
                                        resList.push({ 'thread': value[j].thread, 'count': (value[j].freq * value[j].dur) / countMutiple, 'cpu': value[j].cpu, 'freq': value[j].freq, 'dur': value[j].dur, 'percent': value[j].percent, 'state': 'Running', 'ts': (value[j].ts - timeDur) / multiple, 'id': i });
                                    } else {
                                        if (cutArr[i + 1].ts - value[j].ts > 0) {
                                            resList.push({ 'thread': value[j].thread, 'count': (value[j].freq * (cutArr[i + 1].ts - value[j].ts)) / countMutiple, 'cpu': value[j].cpu, 'freq': value[j].freq, 'dur': cutArr[i + 1].ts - value[j].ts, 'percent': value[j].percent * ((cutArr[i + 1].ts - value[j].ts) / value[j].dur), 'state': 'Running', 'ts': (value[j].ts - timeDur) / multiple, 'id': i });
                                            break;
                                        }
                                    }
                                } else {
                                    if ((value[j].ts + value[j].dur) > cutArr[i + 1].ts) {
                                        resList.push({ 'thread': value[j].thread, 'count': (value[j].freq * (cutArr[i + 1].ts - cutArr[i].ts)) / countMutiple, 'cpu': value[j].cpu, 'freq': value[j].freq, 'dur': cutArr[i + 1].ts - cutArr[i].ts, 'percent': value[j].percent * ((cutArr[i + 1].ts - cutArr[i].ts) / value[j].dur), 'state': 'Running', 'ts': (cutArr[i].ts - timeDur) / multiple, 'id': i });
                                    }
                                    if ((value[j].ts + value[j].dur) > cutArr[i].ts && (value[j].ts + value[j].dur) < cutArr[i + 1].ts) {
                                        resList.push({ 'thread': value[j].thread, 'count': (value[j].freq * (value[j].dur + value[j].ts - cutArr[i].ts)) / countMutiple, 'cpu': value[j].cpu, 'freq': value[j].freq, 'dur': value[j].dur + value[j].ts - cutArr[i].ts, 'percent': value[j].percent * ((value[j].dur + value[j].ts - cutArr[i].ts) / value[j].dur), 'state': 'Running', 'ts': (cutArr[i].ts - timeDur) / multiple, 'id': i });
                                    }
                                }
                            }
                            // 合并相同周期内的数据
                            this.mergeData(resList);
                            // 以算力消耗降序排列
                            resList.sort((a, b) => b.count - a.count);
                            // 以cpu升序排列
                            cpuMap.get(key).sort((a: any, b: any) => a.cpu - b.cpu);
                            cpuMap.get(key).forEach((item: any) => {
                                for (let s = 0; s < resList.length; s++) {
                                    if (item.cpu == resList[s].cpu) {
                                        item.children.push(resList[s]);
                                        item.count += resList[s].count;
                                        item.dur += resList[s].dur;
                                        item.percent += resList[s].percent;
                                    }
                                }
                                item.percent = Number(item.percent.toFixed(2));
                            });
                            // 将cpu数据放置到对应周期层级下
                            this.mergeCycleData(cycleMap.get(key)[i], cpuMap.get(key));
                        }
                    });
                    let threadArr = JSON.parse(JSON.stringify(this.threadArr));
                    let processArr = JSON.parse(JSON.stringify(this.processArr));
                    this.mergeThreadData(threadArr, cycleMap);
                    let totalData = JSON.parse(JSON.stringify(this.originData));
                    this.mergeTotalData(threadArr, totalData);
                    this.mergePidData(processArr, threadArr);
                    this.fixedDeal(processArr);
                    this.threadStatesTblSource = processArr;
                    this.threadStatesTbl!.recycleDataSource = processArr;
                    this.threadStatesTbl!.loading = false;
                    this.threadClick(processArr);
                } else {
                    this.threadStatesTblSource = [];
                    this.threadStatesTbl!.recycleDataSource = [];
                    this.threadStatesTbl!.loading = false;
                }
            })
        } else {
            this.threadStatesTbl!.loading = false;
            if(threadIdValue == ''){
                threadId.style.border = '1px solid rgb(255,0,0)';
                threadId.setAttribute('placeholder', 'Please input threadId!');
            }
            if(threadFuncName == ''){
                threadFunc.style.border = '1px solid rgb(255,0,0)';
                threadFunc.setAttribute('placeholder', 'Please input funcName!');
            }
            if(/^[0-9]*$/.test(threadIdValue) == false){
                threadId.style.border = '1px solid rgb(255,0,0)';
                threadId.setAttribute('placeholder', 'Please check the input!');
            }
        }
    }
    mergeFreqData(needDeal: any, cpuMap: any, dealArr: any, sum: number, threadStatesParam: SelectionParam | any) {
        needDeal.forEach((value: any, key: any) => {
            let resultList = new Array();
            cpuMap.set(key, new Array());
            let cpuArr = new Array();
            // 时间由纳秒转换为秒的倍数
            const multiple = 1000000000;
            // 算力倍数值
            const countMutiple = 1000;
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
                                    resultList.push({ 'thread': value[i].tid + '_' + value[i].thread, 'count': (dealArr[j].value * value[i].dur) / countMutiple, 'cpu': value[i].cpu, 'freq': dealArr[j].value, 'dur': value[i].dur, 'percent': value[i].dur / sum * 100, 'state': 'Running', 'ts': value[i].ts / multiple });
                                    break;
                                } else {
                                    resultList.push({ 'thread': value[i].tid + '_' + value[i].thread, 'count': (dealArr[j].value * (dealArr[j].startNS + dealArr[j].dur - value[i].ts)) / countMutiple, 'cpu': value[i].cpu, 'freq': dealArr[j].value, 'dur': (dealArr[j].startNS + dealArr[j].dur - value[i].ts), 'percent': (dealArr[j].startNS + dealArr[j].dur - value[i].ts) / sum * 100, 'state': 'Running', 'ts': value[i].ts / multiple });
                                }
                            }
                        } else {
                            if ((value[i].ts + value[i].dur) > dealArr[j].startNS) {
                                if ((value[i].dur + value[i].ts - dealArr[j].startNS) < dealArr[j].dur) {
                                    resultList.push({ 'thread': value[i].tid + '_' + value[i].thread, 'count': (dealArr[j].value * (value[i].dur + value[i].ts - dealArr[j].startNS)) / countMutiple, 'cpu': value[i].cpu, 'freq': dealArr[j].value, 'dur': (value[i].dur + value[i].ts - dealArr[j].startNS), 'percent': (value[i].dur + value[i].ts - dealArr[j].startNS) / sum * 100, 'state': 'Running', 'ts': dealArr[j].startNS / multiple });
                                    break;
                                } else {
                                    resultList.push({ 'thread': value[i].tid + '_' + value[i].thread, 'count': (dealArr[j].value * dealArr[j].dur) / countMutiple, 'cpu': value[i].cpu, 'freq': dealArr[j].value, 'dur': dealArr[j].dur, 'percent': dealArr[j].dur / sum * 100, 'state': 'Running', 'ts': dealArr[j].startNS / multiple });
                                }
                            } else {
                                resultList.push({ 'thread': value[i].tid + '_' + value[i].thread, 'count': 0, 'cpu': value[i].cpu, 'freq': 'unknown', 'dur': value[i].dur, 'percent': value[i].dur / sum * 100, 'state': 'Running', 'ts': value[i].ts / multiple });
                                break;
                            }
                        }
                    }
                }
            }
            this.initData.set(key, JSON.parse(JSON.stringify(resultList)));
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
                resultList[i].ts = (resultList[i].ts * 1000000000 - threadStatesParam.recordStartNs) / 1000000000;
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
    mergeData(resList: any) {
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
    }
    mergeCycleData(obj: any, arr: any) {
        for (let i = 0; i < arr.length; i++) {
            if (arr[i].count === 0 && arr[i].dur === 0) {
                continue;
            }
            obj.children.push(arr[i]);
            obj.count += arr[i].count;
            obj.dur += arr[i].dur;
            obj.percent += arr[i].percent;
        }
        obj.percent = Number(obj.percent.toFixed(2));
    }
    mergeTotalData(threadArr: any, totalData: any) {
        for (let i = 0; i < threadArr.length; i++) {
            for (let j = 0; j < totalData.length; j++) {
                if (threadArr[i].pid == totalData[j].pid && threadArr[i].tid == totalData[j].tid) {
                    totalData[j].thread = 'TotalData';
                    threadArr[i].children.unshift(totalData[j]);
                }
            }
        }
    }
    fixedDeal(arr: any){
        if(arr == undefined){
            return ;
        }
        for(let i = 0; i < arr.length; i++){
            arr[i].percent = arr[i].percent.toFixed(2);
            arr[i].percent = arr[i].percent > 100 ? 100 : arr[i].percent;
            this.fixedDeal(arr[i].children);
        }
    }
}