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

import {SpSystemTrace} from "../SpSystemTrace.js";
import {
    getAsyncEvents,
    getFunDataByTid,
    queryEventCountMap,
    queryProcess,
    queryProcessAsyncFunc,
    queryProcessByTable, queryProcessContentCount,
    queryProcessData,
    queryProcessMem,
    queryProcessMemData,
    queryProcessThreads,
    queryProcessThreadsByTable,
    queryThreadData
} from "../../database/SqlLite.js";
import {Utils} from "../trace/base/Utils.js";
import {info} from "../../../log/Log.js";
import {TraceRow} from "../trace/base/TraceRow.js";
import {ProcessStruct} from "../../bean/ProcessStruct.js";
import {procedurePool} from "../../database/Procedure.js";
import {CpuStruct} from "../../bean/CpuStruct.js";
import {FuncStruct} from "../../bean/FuncStruct.js";
import {ProcessMemStruct} from "../../bean/ProcessMemStruct.js";
import {ThreadStruct} from "../../bean/ThreadStruct.js";

export class SpProcessChart {
    private trace: SpSystemTrace;
    private processAsyncFuncMap: any = {}
    private eventCountMap: any;
    private processThreads: Array<ThreadStruct> = []
    private processAsyncEvent: Array<ProcessMemStruct> = []
    private processMem: Array<any> = []
    private processThreadDataCountMap: Map<number, number> = new Map();
    private processFuncDataCountMap: Map<number, number> = new Map();
    private processMemDataCountMap: Map<number, number> = new Map();

    constructor(trace: SpSystemTrace) {
        this.trace = trace;
    }

    initAsyncFuncData = async () => {
        let asyncFuncList: any[] = await queryProcessAsyncFunc();
        info("AsyncFuncData Count is: ", asyncFuncList!.length)
        this.processAsyncFuncMap = Utils.groupBy(asyncFuncList, "pid");
    }

    async init() {
        let pidCountArray = await queryProcessContentCount();
        pidCountArray.forEach(it => {
            this.processThreadDataCountMap.set(it.pid, it.switch_count);
            this.processFuncDataCountMap.set(it.pid, it.slice_count);
            this.processMemDataCountMap.set(it.pid, it.mem_count);
        })
        let queryProcessThreadResult = await queryProcessThreads();
        let queryProcessThreadsByTableResult = await queryProcessThreadsByTable()
        this.processAsyncEvent = await getAsyncEvents();
        info("The amount of initialized process Event data is : ", this.processAsyncEvent!.length)
        this.processMem = await queryProcessMem();
        info("The amount of initialized process memory data is : ", this.processMem!.length)
        let eventCountList: Array<any> = await queryEventCountMap();
        this.eventCountMap = eventCountList.reduce((pre, current) => {
            pre[`${current.eventName}`] = current.count;
            return pre;
        }, {});
        this.processThreads = Utils.removeDuplicates(queryProcessThreadResult, queryProcessThreadsByTableResult, "tid")
        info("The amount of initialized process threads data is : ", this.processThreads!.length)
        if (this.eventCountMap["print"] == 0 &&
            this.eventCountMap["tracing_mark_write"] == 0 &&
            this.eventCountMap["sched_switch"] == 0) {
            return;
        }
        let time = new Date().getTime();
        let processes = await queryProcess();
        let processFromTable = await queryProcessByTable();
        let processList = Utils.removeDuplicates(processes, processFromTable, "pid")
        info("ProcessList Data size is: ", processList!.length)
        for (let i = 0; i < processList.length; i++) {
            const it = processList[i];
            if ((this.processThreadDataCountMap.get(it.pid) || 0) == 0 &&
                (this.processFuncDataCountMap.get(it.pid) || 0) == 0 &&
                (this.processMemDataCountMap.get(it.pid) || 0) == 0) {
                continue;
            }
            let processRow = new TraceRow<ProcessStruct>({
                canvasNumber: 1, alpha: false, contextId: "2d", isOffScreen: true, skeleton: false
            });
            processRow.rowId = `${it.pid}`
            processRow.index = i;
            processRow.rowType = TraceRow.ROW_TYPE_PROCESS
            processRow.rowParentId = '';
            processRow.folder = true;
            processRow.name = `${it.processName || "Process"} ${it.pid}`;
            processRow.supplier = () => queryProcessData(it.pid || -1, 0, TraceRow.range?.totalNS || 0);
            processRow.favoriteChangeHandler = this.trace.favoriteChangeHandler;
            processRow.selectChangeHandler = this.trace.selectChangeHandler;
            processRow.onThreadHandler = (useCache) => {
                procedurePool.submitWithName(`process${(processRow.index) % procedurePool.processLen.length}`, `process ${processRow.index} ${it.processName}`, {
                    list: processRow.must ? processRow.dataList : undefined,
                    offscreen: !processRow.isTransferCanvas ? processRow.offscreen[0] : undefined,
                    pid: it.pid,
                    xs: TraceRow.range?.xs,
                    dpr: processRow.dpr,
                    isHover: processRow.isHover,
                    flagMoveInfo: this.trace.hoverFlag,
                    flagSelectedInfo: this.trace.selectFlag,
                    hoverX: processRow.hoverX,
                    hoverY: processRow.hoverY,
                    canvasWidth: processRow.canvasWidth,
                    canvasHeight: processRow.canvasHeight,
                    isRangeSelect: processRow.rangeSelect,
                    rangeSelectObject: TraceRow.rangeSelectObject,
                    wakeupBean: CpuStruct.wakeupBean,
                    cpuCount: CpuStruct.cpuCount,
                    useCache: useCache,
                    lineColor: processRow.getLineColor(),
                    startNS: TraceRow.range?.startNS || 0,
                    endNS: TraceRow.range?.endNS || 0,
                    totalNS: TraceRow.range?.totalNS || 0,
                    slicesTime: TraceRow.range?.slicesTime,
                    range: TraceRow.range,
                    frame: processRow.frame
                }, !processRow.isTransferCanvas ? processRow.offscreen[0] : undefined, () => {
                    processRow.must = false;
                })
                processRow.isTransferCanvas = true;
            }
            this.trace.rowsEL?.appendChild(processRow)
            /**
             * Async Function
             */
            let asyncFuncList = this.processAsyncFuncMap[it.pid] || [];
            let asyncFuncGroup = Utils.groupBy(asyncFuncList, "funName");
            Reflect.ownKeys(asyncFuncGroup).map((key: any) => {
                let asyncFunctions: Array<any> = asyncFuncGroup[key];
                if (asyncFunctions.length > 0) {
                    let isIntersect = (a: any, b: any) => (Math.max(a.startTs + a.dur, b.startTs + b.dur) - Math.min(a.startTs, b.startTs) < a.dur + b.dur);
                    let depthArray: any = []
                    let createDepth = (currentDepth: number, index: number) => {
                        if (depthArray[currentDepth] == undefined || !isIntersect(depthArray[currentDepth], asyncFunctions[index])) {
                            asyncFunctions[index].depth = currentDepth;
                            depthArray[currentDepth] = asyncFunctions[index]
                        } else {
                            createDepth(++currentDepth, index)
                        }
                    }
                    asyncFunctions.forEach((it, i) => {
                        if (it.dur == -1) {
                            it.dur = TraceRow.range?.endNS || 0 - it.startTs;
                        }
                        createDepth(0, i);
                    });
                    const groupedBy: Array<any> = [];
                    for (let i = 0; i < asyncFunctions.length; i++) {
                        if (groupedBy[asyncFunctions[i].depth || 0]) {
                            groupedBy[asyncFunctions[i].depth || 0].push(asyncFunctions[i]);
                        } else {
                            groupedBy[asyncFunctions[i].depth || 0] = [asyncFunctions[i]];
                        }
                    }
                    let max = Math.max(...asyncFunctions.map(it => it.depth || 0)) + 1
                    let maxHeight = max * 20;
                    let funcRow = new TraceRow<FuncStruct>({
                        canvasNumber: max,
                        alpha: false,
                        contextId: '2d',
                        isOffScreen: SpSystemTrace.isCanvasOffScreen,
                        skeleton: false
                    });
                    funcRow.rowId = `${asyncFunctions[0].funName}-${it.pid}`
                    funcRow.asyncFuncName = asyncFunctions[0].funName;
                    funcRow.asyncFuncNamePID = it.pid;
                    funcRow.rowType = TraceRow.ROW_TYPE_FUNC
                    funcRow.rowParentId = `${it.pid}`
                    funcRow.rowHidden = !processRow.expansion
                    funcRow.style.width = `100%`;
                    funcRow.setAttribute("height", `${maxHeight}`);
                    funcRow.name = `${asyncFunctions[0].funName}`;
                    funcRow.setAttribute('children', '')
                    funcRow.supplier = () => new Promise((resolve) => resolve(asyncFunctions))
                    funcRow.favoriteChangeHandler = this.trace.favoriteChangeHandler;
                    funcRow.selectChangeHandler = this.trace.selectChangeHandler;
                    funcRow.onThreadHandler = (useCache) => {
                        let asy = async (useCache: boolean) => {
                            let scrollTop = this.trace.rowsEL?.scrollTop || 0;
                            let scrollHeight = this.trace.rowsEL?.clientHeight || 0;
                            let promises: Array<any> = [];
                            for (let k = 0; k < groupedBy.length; k++) {
                                let top = funcRow.offsetTop - (this.trace.rowsEL?.offsetTop || 0) - scrollTop + funcRow.canvas[k].offsetTop;
                                let isLive = ((top + funcRow.canvas[k].clientHeight >= 0) && (top < scrollHeight)) || funcRow.collect
                                let promise = await procedurePool.submitWithNamePromise(`cpu${k % procedurePool.cpusLen.length}`, `func-${asyncFunctions[0].funName}-${it.pid}-${k}`, {
                                    isLive: isLive,
                                    list: funcRow.must ? groupedBy[k] : undefined,
                                    offscreen: !funcRow.isTransferCanvas ? funcRow.offscreen[k] : undefined,//是否离屏
                                    dpr: funcRow.dpr,//屏幕dpr值
                                    xs: TraceRow.range?.xs,//线条坐标信息
                                    isHover: funcRow.isHover,
                                    flagMoveInfo: this.trace.hoverFlag,
                                    flagSelectedInfo: this.trace.selectFlag,
                                    hoverX: funcRow.hoverX,
                                    hoverY: funcRow.hoverY,
                                    depth: k,
                                    canvasWidth: funcRow.canvasWidth,
                                    canvasHeight: funcRow.canvasHeight,
                                    maxHeight: maxHeight,
                                    hoverFuncStruct: FuncStruct.hoverFuncStruct,
                                    selectFuncStruct: FuncStruct.selectFuncStruct,
                                    wakeupBean: CpuStruct.wakeupBean,
                                    isRangeSelect: funcRow.rangeSelect,
                                    rangeSelectObject: TraceRow.rangeSelectObject,
                                    useCache: useCache,
                                    lineColor: funcRow.getLineColor(),
                                    startNS: TraceRow.range?.startNS || 0,
                                    endNS: TraceRow.range?.endNS || 0,
                                    totalNS: TraceRow.range?.totalNS || 0,
                                    slicesTime: TraceRow.range?.slicesTime,
                                    range: TraceRow.range,
                                    frame: funcRow.frame
                                }, !funcRow.isTransferCanvas ? funcRow.offscreen[k] : undefined)
                                if (funcRow.isHover && promise.hover) {
                                    FuncStruct.hoverFuncStruct = promise.hover;
                                }
                                promises.push(promise);
                            }
                            if (funcRow.isHover && promises.every(it => !it.hover)) {
                                FuncStruct.hoverFuncStruct = undefined;
                            }
                            funcRow.must = false;
                            funcRow.isTransferCanvas = true;
                        }
                        asy(useCache).then()
                    }
                    this.trace.rowsEL?.appendChild(funcRow);
                }
            });

            /**
             * 添加进程内存信息
             */
            let processMem = this.processMem.filter(mem => mem.pid === it.pid);
            processMem.forEach(mem => {
                let row = new TraceRow<ProcessMemStruct>(
                    {canvasNumber: 1, alpha: false, contextId: "2d", isOffScreen: true, skeleton: false}
                );
                row.rowId = `${mem.trackId}`
                row.rowType = TraceRow.ROW_TYPE_MEM
                row.rowParentId = `${it.pid}`
                row.rowHidden = !processRow.expansion
                row.style.height = '40px'
                row.style.width = `100%`;
                row.name = `${mem.trackName}`;
                row.setAttribute('children', '');
                row.favoriteChangeHandler = this.trace.favoriteChangeHandler;
                row.selectChangeHandler = this.trace.selectChangeHandler;
                row.supplier = () => queryProcessMemData(mem.trackId).then(res => {
                    let maxValue = Math.max(...res.map(it => it.value || 0))
                    for (let j = 0; j < res.length; j++) {
                        res[j].maxValue = maxValue;
                        if (j == res.length - 1) {
                            res[j].duration = (TraceRow.range?.totalNS || 0) - (res[j].startTime || 0);
                        } else {
                            res[j].duration = (res[j + 1].startTime || 0) - (res[j].startTime || 0);
                        }
                        if (j > 0) {
                            res[j].delta = (res[j].value || 0) - (res[j - 1].value || 0);
                        } else {
                            res[j].delta = 0;
                        }
                    }
                    return res
                });
                row.onThreadHandler = (useCache) => {
                    procedurePool.submitWithName(`cpu${mem.trackId % procedurePool.cpusLen.length}`, `mem ${mem.trackId} ${mem.trackName}`, {
                        list: row.must ? row.dataList : undefined,
                        offscreen: !row.isTransferCanvas ? row.offscreen[0] : undefined,//是否离屏
                        dpr: row.dpr,//屏幕dpr值
                        xs: TraceRow.range?.xs,//线条坐标信息
                        isHover: row.isHover,
                        flagMoveInfo: this.trace.hoverFlag,
                        flagSelectedInfo: this.trace.selectFlag,
                        hoverX: row.hoverX,
                        hoverY: row.hoverY,
                        canvasWidth: row.canvasWidth,
                        canvasHeight: row.canvasHeight,
                        wakeupBean: CpuStruct.wakeupBean,
                        isRangeSelect: row.rangeSelect,
                        rangeSelectObject: TraceRow.rangeSelectObject,
                        useCache: useCache,
                        lineColor: row.getLineColor(),
                        startNS: TraceRow.range?.startNS || 0,
                        endNS: TraceRow.range?.endNS || 0,
                        totalNS: TraceRow.range?.totalNS || 0,
                        slicesTime: TraceRow.range?.slicesTime,
                        hoverProcessMemStruct: ProcessMemStruct.hoverProcessMemStruct,
                        range: TraceRow.range,
                        frame: row.frame
                    }, row.getTransferArray(), (res: any, hover: any) => {
                        row.must = false;
                        if (row.isHover) {
                            ProcessMemStruct.hoverProcessMemStruct = hover;
                        }
                        return;
                    });
                    row.isTransferCanvas = true;
                }
                this.trace.rowsEL?.appendChild(row)
            });
            /**
             * add thread list
             */
            let threads = this.processThreads.filter(thread => thread.pid === it.pid && thread.tid != 0);
            for (let j = 0; j < threads.length; j++) {
                let thread = threads[j];
                let threadRow = new TraceRow<ThreadStruct>({
                    canvasNumber: 1,
                    alpha: false,
                    contextId: "2d",
                    isOffScreen: true,
                    skeleton: false
                });
                threadRow.rowId = `${thread.tid}`
                threadRow.rowType = TraceRow.ROW_TYPE_THREAD
                threadRow.rowParentId = `${it.pid}`
                threadRow.rowHidden = !processRow.expansion
                threadRow.index = j
                threadRow.style.height = '30px'
                threadRow.setAttribute("height", `30`);
                threadRow.style.width = `100%`;
                threadRow.name = `${thread.threadName || 'Thread'} ${thread.tid}`;
                threadRow.setAttribute('children', '')
                threadRow.favoriteChangeHandler = this.trace.favoriteChangeHandler;
                threadRow.selectChangeHandler = this.trace.selectChangeHandler;
                threadRow.supplier = () => queryThreadData(thread.tid || 0).then(res => {
                    getFunDataByTid(thread.tid || 0).then((funs: Array<FuncStruct>) => {
                        if (funs.length > 0) {
                            let isBinder = (data: FuncStruct): boolean => {
                                return data.funName != null && (
                                    data.funName.toLowerCase().startsWith("binder transaction async") //binder transaction
                                    || data.funName.toLowerCase().startsWith("binder async")
                                    || data.funName.toLowerCase().startsWith("binder reply")
                                );
                            }
                            funs.forEach(fun => {
                                if (isBinder(fun)) {
                                } else {
                                    if (fun.dur == -1) {
                                        fun.dur = (TraceRow.range?.totalNS || 0) - (fun.startTs || 0);
                                    }
                                }
                            })
                            const groupedBy: Array<any> = [];
                            for (let i = 0; i < funs.length; i++) {
                                if (groupedBy[funs[i].depth || 0]) {
                                    groupedBy[funs[i].depth || 0].push(funs[i]);
                                } else {
                                    groupedBy[funs[i].depth || 0] = [funs[i]];
                                }
                            }
                            let max = Math.max(...funs.map(it => it.depth || 0)) + 1
                            let maxHeight = max * 20;
                            let funcRow = new TraceRow<FuncStruct>({
                                canvasNumber: max,
                                alpha: false,
                                contextId: '2d',
                                isOffScreen: SpSystemTrace.isCanvasOffScreen,
                                skeleton: false
                            });
                            funcRow.rowId = `${thread.tid}`
                            funcRow.rowType = TraceRow.ROW_TYPE_FUNC
                            funcRow.rowParentId = `${it.pid}`
                            funcRow.rowHidden = !processRow.expansion
                            funcRow.checkType = threadRow.checkType;
                            funcRow.style.width = `100%`;
                            funcRow.setAttribute("height", `${maxHeight}`);
                            funcRow.name = `${thread.threadName || 'Thread'} ${thread.tid}`;
                            funcRow.setAttribute('children', '')
                            funcRow.supplier = () => new Promise((resolve) => resolve(funs))
                            funcRow.favoriteChangeHandler = this.trace.favoriteChangeHandler;
                            funcRow.selectChangeHandler = this.trace.selectChangeHandler;
                            funcRow.onThreadHandler = (useCache) => {
                                let asy = async (useCache: boolean) => {
                                    let scrollTop = this.trace.rowsEL?.scrollTop || 0;
                                    let scrollHeight = this.trace.rowsEL?.clientHeight || 0;
                                    let promises: Array<any> = [];
                                    for (let k = 0; k < groupedBy.length; k++) {
                                        let top = funcRow.offsetTop - (this.trace.rowsEL?.offsetTop || 0) - scrollTop + funcRow.canvas[k].offsetTop;
                                        let isLive = ((top + funcRow.canvas[k].clientHeight >= 0) && (top < scrollHeight)) || funcRow.collect
                                        let promise = await procedurePool.submitWithNamePromise(`cpu${k % procedurePool.cpusLen.length}`, `func${thread.tid}${k}${thread.threadName}`, {
                                            isLive: isLive,
                                            list: funcRow.must ? groupedBy[k] : undefined,
                                            offscreen: !funcRow.isTransferCanvas ? funcRow.offscreen[k] : undefined,//是否离屏
                                            dpr: funcRow.dpr,//屏幕dpr值
                                            xs: TraceRow.range?.xs,//线条坐标信息
                                            isHover: funcRow.isHover,
                                            flagMoveInfo: this.trace.hoverFlag,
                                            flagSelectedInfo: this.trace.selectFlag,
                                            hoverX: funcRow.hoverX,
                                            hoverY: funcRow.hoverY,
                                            depth: k,
                                            canvasWidth: funcRow.canvasWidth,
                                            canvasHeight: funcRow.canvasHeight,
                                            maxHeight: maxHeight,
                                            hoverFuncStruct: FuncStruct.hoverFuncStruct,
                                            selectFuncStruct: FuncStruct.selectFuncStruct,
                                            wakeupBean: CpuStruct.wakeupBean,
                                            isRangeSelect: funcRow.rangeSelect,
                                            rangeSelectObject: TraceRow.rangeSelectObject,
                                            useCache: useCache,
                                            lineColor: funcRow.getLineColor(),
                                            startNS: TraceRow.range?.startNS || 0,
                                            endNS: TraceRow.range?.endNS || 0,
                                            totalNS: TraceRow.range?.totalNS || 0,
                                            slicesTime: TraceRow.range?.slicesTime,
                                            range: TraceRow.range,
                                            frame: funcRow.frame
                                        }, !funcRow.isTransferCanvas ? funcRow.offscreen[k] : undefined);
                                        if (funcRow.isHover && promise.hover) {
                                            FuncStruct.hoverFuncStruct = promise.hover;
                                        }
                                        promises.push(promise);
                                    }
                                    if (funcRow.isHover && promises.every(it => !it.hover)) {
                                        FuncStruct.hoverFuncStruct = undefined;
                                    }
                                    funcRow.must = false;
                                    funcRow.isTransferCanvas = true;
                                }
                                asy(useCache).then();
                            }
                            this.insertAfter(funcRow, threadRow)
                            this.trace.observer.observe(funcRow)
                            funcRow.draw();
                            if (threadRow.onComplete) {
                                threadRow.onComplete()
                            }
                            this.trace.getVisibleRows();//function 由于后插入dom，所以需要重新获取可见行
                        }
                    })
                    if ((res instanceof ArrayBuffer && res.byteLength <= 0) || (res.length <= 0)) {
                        threadRow.rowDiscard = true;
                    }
                    return res;
                })
                threadRow.onThreadHandler = (useCache) => {
                    procedurePool.submitWithName(`process${(threadRow.index) % procedurePool.processLen.length}`, `thread ${thread.tid} ${thread.threadName}`, {
                        list: threadRow.must ? threadRow.dataList : undefined,
                        offscreen: !threadRow.isTransferCanvas ? threadRow.offscreen[0] : undefined,//是否离屏
                        dpr: threadRow.dpr,//屏幕dpr值
                        xs: TraceRow.range?.xs,//线条坐标信息
                        isHover: threadRow.isHover,
                        flagMoveInfo: this.trace.hoverFlag,
                        flagSelectedInfo: this.trace.selectFlag,
                        hoverX: threadRow.hoverX,
                        hoverY: threadRow.hoverY,
                        canvasWidth: threadRow.canvasWidth,
                        canvasHeight: threadRow.canvasHeight,
                        hoverThreadStruct: ThreadStruct.hoverThreadStruct,
                        selectThreadStruct: ThreadStruct.selectThreadStruct,
                        wakeupBean: CpuStruct.wakeupBean,
                        isRangeSelect: threadRow.rangeSelect,
                        rangeSelectObject: TraceRow.rangeSelectObject,
                        useCache: useCache,
                        lineColor: threadRow.getLineColor(),
                        startNS: TraceRow.range?.startNS || 0,
                        endNS: TraceRow.range?.endNS || 0,
                        totalNS: TraceRow.range?.totalNS || 0,
                        slicesTime: TraceRow.range?.slicesTime,
                        range: TraceRow.range,
                        frame: threadRow.frame
                    }, !threadRow.isTransferCanvas ? threadRow.offscreen[0] : undefined, (res: any, hover: any) => {
                        threadRow.must = false;
                        if (threadRow.args.isOffScreen == true) {
                            if (threadRow.isHover) {
                                ThreadStruct.hoverThreadStruct = hover;
                            }
                            return;
                        }
                    })
                    threadRow.isTransferCanvas = true;
                }
                if (threadRow.rowId == threadRow.rowParentId) {
                    this.insertAfter(threadRow, processRow)
                } else {
                    this.trace.rowsEL?.appendChild(threadRow)
                }
            }

        }
        let durTime = new Date().getTime() - time;
        info('The time to load the Process data is: ', durTime)
    }

    insertAfter(newEl: HTMLElement, targetEl: HTMLElement) {
        let parentEl = targetEl.parentNode;
        if (parentEl!.lastChild == targetEl) {
            parentEl!.appendChild(newEl);
        } else {
            parentEl!.insertBefore(newEl, targetEl.nextSibling);
        }
    }
}