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
import {TraceRow} from "../trace/base/TraceRow.js";
import {procedurePool} from "../../database/Procedure.js";
import {
    queryHiPerfCpuData,
    queryHiPerfCpuMergeData,
    queryHiPerfCpuMergeData2,
    queryHiPerfEventData,
    queryHiPerfEventList,
    queryHiPerfEventListData,
    queryHiPerfProcessData,
    queryHiPerfThreadData,
    queryPerfCmdline,
    queryPerfThread
} from "../../database/SqlLite.js";
import {Utils} from "../trace/base/Utils.js";
import {PerfThread} from "../../bean/PerfProfile.js";
import {HiPerfCpuStruct} from "../../database/ui-worker/ProcedureWorkerHiPerfCPU.js";
import {HiPerfThreadStruct} from "../../database/ui-worker/ProcedureWorkerHiPerfThread.js";
import {HiPerfProcessStruct} from "../../database/ui-worker/ProcedureWorkerHiPerfProcess.js";
import {info} from "../../../log/Log.js";
import {HiPerfEventStruct} from "../../database/ui-worker/ProcedureWorkerHiPerfEvent.js";

export interface ResultData {
    existA: boolean | null | undefined,
    existF: boolean | null | undefined,
    fValue: number
}

export class SpHiPerf {
    static hoverCpuStruct: HiPerfCpuStruct | undefined;
    static hoverEventuctStruct: HiPerfEventStruct | undefined;
    static selectCpuStruct: HiPerfCpuStruct | undefined;
    static hoverProcessStruct: HiPerfProcessStruct | undefined;
    static selectProcessStruct: HiPerfProcessStruct | undefined;
    static hoverThreadStruct: HiPerfThreadStruct | undefined;
    static selectThreadStruct: HiPerfThreadStruct | undefined;
    static stringResult: ResultData | undefined;

    private cpuData: Array<any> | undefined
    public maxCpuId: number = 0
    private rowFolder!: TraceRow<any>;
    private perfThreads: Array<PerfThread> | undefined;
    private trace: SpSystemTrace;
    private group: any;
    private eventTypeList: Array<{ id: number, report_value: string }> = [];

    constructor(trace: SpSystemTrace) {
        this.trace = trace;
    }

    async init() {
        await this.initCmdLine()
        this.perfThreads = await queryPerfThread();
        this.eventTypeList = await queryHiPerfEventList();
        info("PerfThread Data size is: ", this.perfThreads!.length)
        this.group = Utils.groupBy(this.perfThreads || [], "pid");
        this.cpuData = await queryHiPerfCpuMergeData2();
        this.maxCpuId = this.cpuData.length > 0 ? this.cpuData.reduce((max, v) => max.cpu_id >= v.cpu_id ? max : v).cpu_id : -Infinity;
        if (this.cpuData.length > 0) {
            await this.initFolder();
            await this.initCpuMerge();
            await this.initCpu();
            await this.initProcess();
        }
        info("HiPerf Data initialized")
    }

    getStringResult(s: string = "") {
        let list = s.split(" ").filter((e) => e);
        let sA = list.findIndex((item) => item == "-a");
        let sF = list.findIndex((item) => item == "-f");
        SpHiPerf.stringResult = {
            existA: sA !== -1,
            existF: sF !== -1,
            fValue: Number((1000 / (sF !== -1 ? parseInt(list[sF + 1]) : 1000)).toFixed(1)),
        }
    }

    async initCmdLine() {
        let perfCmdLines = await queryPerfCmdline();
        if (perfCmdLines.length > 0) {
            this.getStringResult(perfCmdLines[0].report_value)
        } else {
            SpHiPerf.stringResult = {
                existA: true,
                existF: false,
                fValue: 1,
            }
        }
    }

    async initFolder() {
        let row = new TraceRow({
            canvasNumber: 1,
            alpha: false,
            contextId: '2d',
            isOffScreen: SpSystemTrace.isCanvasOffScreen
        });
        row.setAttribute('disabled-check', '')
        row.rowId = `HiPerf`;
        row.index = 0;
        row.rowType = TraceRow.ROW_TYPE_HIPERF
        row.rowParentId = '';
        row.folder = true;
        if (SpHiPerf.stringResult?.existA === true) {
            row.name = `HiPerf (All)`;
        } else {
            let names = Reflect.ownKeys(this.group).map((pid: any) => {
                let array = this.group[pid] as Array<PerfThread>;
                let process = array.filter(th => th.pid === th.tid)[0];
                return process.processName;
            }).join(',');
            row.name = `HiPerf (${names})`;
        }
        row.supplier = () => new Promise<Array<any>>((resolve) => resolve([]));
        row.onThreadHandler = (useCache) => {
            procedurePool.submitWithName(`process${row.index}`, `HiPerf-Group`, {
                list: row.must ? row.dataList : undefined,
                offscreen: !row.isTransferCanvas ? row.offscreen[0] : undefined,
                xs: TraceRow.range?.xs,
                dpr: row.dpr,
                isHover: row.isHover,
                flagMoveInfo: this.trace.hoverFlag,
                flagSelectedInfo: this.trace.selectFlag,
                hoverX: row.hoverX,
                hoverY: row.hoverY,
                canvasWidth: row.canvasWidth,
                canvasHeight: row.canvasHeight,
                isRangeSelect: row.rangeSelect,
                rangeSelectObject: TraceRow.rangeSelectObject,
                useCache: useCache,
                lineColor: row.getLineColor(),
                startNS: TraceRow.range?.startNS || 0,
                endNS: TraceRow.range?.endNS || 0,
                totalNS: TraceRow.range?.totalNS || 0,
                slicesTime: TraceRow.range?.slicesTime,
                scale: TraceRow.range?.scale || 50,
                intervalPerf: SpHiPerf.stringResult?.fValue || 1,
                range:TraceRow.range,
                frame: row.frame
            }, !row.isTransferCanvas ? row.offscreen[0] : undefined, (res: any) => {
                row.must = false;
            })
            row.isTransferCanvas = true;
        }
        this.rowFolder = row;
        this.trace.rowsEL?.appendChild(row)
    }

    async initCpuMerge() {
        let row = new TraceRow({
            canvasNumber: 1,
            alpha: false,
            contextId: '2d',
            isOffScreen: SpSystemTrace.isCanvasOffScreen
        });
        row.rowId = `HiPerf-cpu-merge`;
        row.index = 0;
        row.rowType = TraceRow.ROW_TYPE_HIPERF_CPU
        row.rowParentId = 'HiPerf';
        row.rowHidden = !this.rowFolder.expansion
        row.folder = false;
        row.name = `HiPerf`;
        row.setAttribute('children', '')
        row.favoriteChangeHandler = this.trace.favoriteChangeHandler;
        row.selectChangeHandler = this.trace.selectChangeHandler;
        let that = this;
        row.supplier = () => queryHiPerfCpuMergeData();
        row.onThreadHandler = (useCache) => {
            procedurePool.submitWithName(`freq${row.index}`, `HiPerf-Cpu-Merge`, {
                list: row.must ? row.dataList : undefined,
                offscreen: !row.isTransferCanvas ? row.offscreen[0] : undefined,
                xs: TraceRow.range?.xs,
                dpr: row.dpr,
                isHover: row.isHover,
                flagMoveInfo: this.trace.hoverFlag,
                flagSelectedInfo: this.trace.selectFlag,
                hoverX: row.hoverX,
                hoverY: row.hoverY,
                canvasWidth: row.canvasWidth,
                canvasHeight: row.canvasHeight,
                hoverStruct: SpHiPerf.hoverCpuStruct,
                selectStruct: SpHiPerf.selectCpuStruct,
                isRangeSelect: row.rangeSelect,
                rangeSelectObject: TraceRow.rangeSelectObject,
                useCache: useCache,
                lineColor: row.getLineColor(),
                startNS: TraceRow.range?.startNS || 0,
                endNS: TraceRow.range?.endNS || 0,
                totalNS: TraceRow.range?.totalNS || 0,
                slicesTime: TraceRow.range?.slicesTime,
                scale: TraceRow.range?.scale || 50,
                intervalPerf: SpHiPerf.stringResult?.fValue || 1,
                range:TraceRow.range,
                frame: row.frame,
                maxCpu: (this.maxCpuId + 1)
            }, row.getTransferArray(), (res: any, hover: any) => {
                row.must = false;
                if (row.isHover) {
                    SpHiPerf.hoverCpuStruct = hover;
                }
            })
            row.isTransferCanvas = true;
        }
        this.trace.rowsEL?.appendChild(row)
    }

    async initCpu() {
        for (let i = 0; i <= this.maxCpuId; i++) {
            let row = new TraceRow({
                canvasNumber: 1,
                alpha: false,
                contextId: '2d',
                isOffScreen: SpSystemTrace.isCanvasOffScreen
            });
            row.rowId = `HiPerf-cpu-${i}`;
            row.index = i;
            row.rowType = TraceRow.ROW_TYPE_HIPERF_CPU
            row.rowParentId = 'HiPerf';
            row.rowHidden = !this.rowFolder.expansion
            row.folder = false;
            row.name = `Cpu ${i}`;
            row.setAttribute('children', '')
            row.favoriteChangeHandler = this.trace.favoriteChangeHandler;
            row.selectChangeHandler = this.trace.selectChangeHandler;
            let that = this;
            row.supplier = () => queryHiPerfCpuData(i);
            row.onThreadHandler = (useCache) => {
                procedurePool.submitWithName(`cpu${row.index}`, `HiPerf-Cpu-${i}`, {
                    list: row.must ? row.dataList : undefined,
                    offscreen: !row.isTransferCanvas ? row.offscreen[0] : undefined,
                    xs: TraceRow.range?.xs,
                    dpr: row.dpr,
                    isHover: row.isHover,
                    flagMoveInfo: this.trace.hoverFlag,
                    flagSelectedInfo: this.trace.selectFlag,
                    hoverX: row.hoverX,
                    hoverY: row.hoverY,
                    canvasWidth: row.canvasWidth,
                    canvasHeight: row.canvasHeight,
                    hoverStruct: SpHiPerf.hoverCpuStruct,
                    selectStruct: SpHiPerf.selectCpuStruct,
                    isRangeSelect: row.rangeSelect,
                    rangeSelectObject: TraceRow.rangeSelectObject,
                    useCache: useCache,
                    lineColor: row.getLineColor(),
                    startNS: TraceRow.range?.startNS || 0,
                    endNS: TraceRow.range?.endNS || 0,
                    totalNS: TraceRow.range?.totalNS || 0,
                    slicesTime: TraceRow.range?.slicesTime,
                    scale: TraceRow.range?.scale || 50,
                    intervalPerf: SpHiPerf.stringResult?.fValue || 1,
                    range:TraceRow.range,
                    frame: row.frame,
                    maxCpu: undefined
                }, row.getTransferArray(), (res: any, hover: any) => {
                    row.must = false;
                    if (row.isHover) {
                        SpHiPerf.hoverCpuStruct = hover;
                    }
                })
                row.isTransferCanvas = true;
            }
            this.trace.rowsEL?.appendChild(row)
        }
    }

    async initReport() {
        this.eventTypeList.forEach((it, index) => {
            let fold = new TraceRow({
                canvasNumber: 1,
                alpha: false,
                contextId: '2d',
                isOffScreen: SpSystemTrace.isCanvasOffScreen
            });
            fold.rowId = `Perf-Report-${it.id}-${it.report_value}`;
            fold.index = index;
            fold.rowType = TraceRow.ROW_TYPE_HIPERF_REPORT
            fold.rowParentId = 'HiPerf';
            fold.rowHidden = !this.rowFolder.expansion
            fold.folder = true;
            fold.name = `Event :${it.report_value}`;
            fold.folderPaddingLeft = 30;
            fold.favoriteChangeHandler = this.trace.favoriteChangeHandler;
            fold.selectChangeHandler = this.trace.selectChangeHandler;
            let that = this;
            fold.supplier = () => queryHiPerfEventListData(it.id);
            fold.onThreadHandler = (useCache) => {
                procedurePool.submitWithName(`process${(fold.index) % procedurePool.processLen.length}`, `HiPerf-Report-Fold-${it.report_value}-${it.id}`, {
                    list: fold.must ? fold.dataList : undefined,
                    offscreen: !fold.isTransferCanvas ? fold.offscreen[0] : undefined,
                    xs: TraceRow.range?.xs,
                    dpr: fold.dpr,
                    isHover: fold.isHover,
                    flagMoveInfo: this.trace.hoverFlag,
                    flagSelectedInfo: this.trace.selectFlag,
                    hoverX: fold.hoverX,
                    hoverY: fold.hoverY,
                    canvasWidth: fold.canvasWidth,
                    canvasHeight: fold.canvasHeight,
                    isRangeSelect: fold.rangeSelect,
                    rangeSelectObject: TraceRow.rangeSelectObject,
                    useCache: useCache,
                    lineColor: fold.getLineColor(),
                    startNS: TraceRow.range?.startNS || 0,
                    endNS: TraceRow.range?.endNS || 0,
                    totalNS: TraceRow.range?.totalNS || 0,
                    slicesTime: TraceRow.range?.slicesTime,
                    scale: TraceRow.range?.scale || 50,
                    intervalPerf: SpHiPerf.stringResult?.fValue || 1,
                    range:TraceRow.range,
                    frame: fold.frame
                }, fold.getTransferArray(), (res: any, hover: any) => {
                    fold.must = false;
                    if (fold.isHover) {
                    }
                })
                fold.isTransferCanvas = true;
            }
            this.trace.rowsEL?.appendChild(fold)

            for (let i = 0; i <= this.maxCpuId; i++) {
                let row = new TraceRow({
                    canvasNumber: 1,
                    alpha: false,
                    contextId: '2d',
                    isOffScreen: SpSystemTrace.isCanvasOffScreen
                });
                row.rowId = `HiPerf-Report-Event-${it.report_value}-${i}`;
                row.index = i;
                row.rowType = TraceRow.ROW_TYPE_HIPERF_EVENT
                row.rowParentId = fold.rowId;
                row.rowHidden = !fold.expansion
                row.folder = false;
                row.name = `Cpu ${i}`;
                row.setAttribute('children', '')
                row.favoriteChangeHandler = this.trace.favoriteChangeHandler;
                row.selectChangeHandler = this.trace.selectChangeHandler;
                let that = this;
                row.supplier = () => queryHiPerfEventData(it.id, row.index);
                row.onThreadHandler = (useCache) => {
                    procedurePool.submitWithName(`process${row.index % procedurePool.processLen.length}`, `HiPerf-Report-Event-${it.report_value}-${i}`, {
                        list: row.must ? row.dataList : undefined,
                        offscreen: !row.isTransferCanvas ? row.offscreen[0] : undefined,
                        xs: TraceRow.range?.xs,
                        dpr: row.dpr,
                        isHover: row.isHover,
                        flagMoveInfo: this.trace.hoverFlag,
                        flagSelectedInfo: this.trace.selectFlag,
                        hoverX: row.hoverX,
                        hoverY: row.hoverY,
                        canvasWidth: row.canvasWidth,
                        canvasHeight: row.canvasHeight,
                        hoverStruct: SpHiPerf.hoverEventuctStruct,
                        selectStruct: SpHiPerf.selectCpuStruct,
                        isRangeSelect: row.rangeSelect,
                        rangeSelectObject: TraceRow.rangeSelectObject,
                        useCache: useCache,
                        lineColor: row.getLineColor(),
                        startNS: TraceRow.range?.startNS || 0,
                        endNS: TraceRow.range?.endNS || 0,
                        totalNS: TraceRow.range?.totalNS || 0,
                        slicesTime: TraceRow.range?.slicesTime,
                        scale: TraceRow.range?.scale || 50,
                        intervalPerf: SpHiPerf.stringResult?.fValue || 1,
                        range:TraceRow.range,
                        frame: row.frame,
                        maxCpu: undefined
                    }, row.getTransferArray(), (res: any, hover: any) => {
                        row.must = false;
                        if (row.isHover) {
                            SpHiPerf.hoverEventuctStruct = hover;
                        }
                    })
                    row.isTransferCanvas = true;
                }
                this.trace.rowsEL?.appendChild(row)
            }
        })
    }

    async initProcess() {
        Reflect.ownKeys(this.group).forEach((key, index) => {
            let array = this.group[key] as Array<PerfThread>;
            let process = array.filter(th => th.pid === th.tid)[0];
            let row = new TraceRow({
                canvasNumber: 1,
                alpha: false,
                contextId: '2d',
                isOffScreen: SpSystemTrace.isCanvasOffScreen
            });
            row.rowId = `${process.pid}-Perf-Process`;
            row.index = index;
            row.rowType = TraceRow.ROW_TYPE_HIPERF_PROCESS
            row.rowParentId = 'HiPerf';
            row.rowHidden = !this.rowFolder.expansion
            row.folder = true;
            row.name = `${process.processName||'Process'} [${process.pid}]`;
            row.folderPaddingLeft = 30;
            row.favoriteChangeHandler = this.trace.favoriteChangeHandler;
            row.selectChangeHandler = this.trace.selectChangeHandler;
            let that = this;
            row.supplier = () => queryHiPerfProcessData(process.pid);
            row.onThreadHandler = (useCache) => {
                procedurePool.submitWithName(`process${(row.index) % procedurePool.processLen.length}`, `HiPerf-Process-${row.index}`, {
                    list: row.must ? row.dataList : undefined,
                    offscreen: !row.isTransferCanvas ? row.offscreen[0] : undefined,
                    xs: TraceRow.range?.xs,
                    dpr: row.dpr,
                    isHover: row.isHover,
                    flagMoveInfo: this.trace.hoverFlag,
                    flagSelectedInfo: this.trace.selectFlag,
                    hoverX: row.hoverX,
                    hoverY: row.hoverY,
                    canvasWidth: row.canvasWidth,
                    canvasHeight: row.canvasHeight,
                    hoverStruct: SpHiPerf.hoverProcessStruct,
                    selectStruct: SpHiPerf.selectProcessStruct,
                    isRangeSelect: row.rangeSelect,
                    rangeSelectObject: TraceRow.rangeSelectObject,
                    useCache: useCache,
                    lineColor: row.getLineColor(),
                    startNS: TraceRow.range?.startNS || 0,
                    endNS: TraceRow.range?.endNS || 0,
                    totalNS: TraceRow.range?.totalNS || 0,
                    slicesTime: TraceRow.range?.slicesTime,
                    scale: TraceRow.range?.scale || 50,
                    intervalPerf: SpHiPerf.stringResult?.fValue || 1,
                    range:TraceRow.range,
                    frame: row.frame
                }, row.getTransferArray(), (res: any, hover: any) => {
                    row.must = false;
                    if (row.isHover) {
                        SpHiPerf.hoverProcessStruct = hover;
                    }
                })
                row.isTransferCanvas = true;
            }
            this.trace.rowsEL?.appendChild(row)

            array.forEach((thObj, thIdx) => {
                let thread = new TraceRow({
                    canvasNumber: 1,
                    alpha: false,
                    contextId: '2d',
                    isOffScreen: SpSystemTrace.isCanvasOffScreen
                });
                thread.rowId = `${thObj.tid}-Perf-Thread`;
                thread.index = thIdx;
                thread.rowType = TraceRow.ROW_TYPE_HIPERF_THREAD
                thread.rowParentId = row.rowId;
                thread.rowHidden = !row.expansion
                thread.folder = false;
                thread.name = `${thObj.threadName||'Thread'} [${thObj.tid}]`;
                thread.setAttribute('children', '')
                thread.folderPaddingLeft = 30;
                thread.favoriteChangeHandler = this.trace.favoriteChangeHandler;
                thread.selectChangeHandler = this.trace.selectChangeHandler;
                let that = this;
                thread.supplier = () => queryHiPerfThreadData(thObj.tid);
                thread.onThreadHandler = (useCache) => {
                    procedurePool.submitWithName(`process${(thread.index) % procedurePool.processLen.length}`, `HiPerf-Thread-${row.index}-${thread.index}`, {
                        list: thread.must ? thread.dataList : undefined,
                        offscreen: !thread.isTransferCanvas ? thread.offscreen[0] : undefined,
                        xs: TraceRow.range?.xs,
                        dpr: thread.dpr,
                        isHover: thread.isHover,
                        flagMoveInfo: this.trace.hoverFlag,
                        flagSelectedInfo: this.trace.selectFlag,
                        hoverX: thread.hoverX,
                        hoverY: thread.hoverY,
                        canvasWidth: thread.canvasWidth,
                        canvasHeight: thread.canvasHeight,
                        hoverStruct: SpHiPerf.hoverThreadStruct,
                        selectStruct: SpHiPerf.selectThreadStruct,
                        isRangeSelect: thread.rangeSelect,
                        rangeSelectObject: TraceRow.rangeSelectObject,
                        useCache: useCache,
                        lineColor: thread.getLineColor(),
                        startNS: TraceRow.range?.startNS || 0,
                        endNS: TraceRow.range?.endNS || 0,
                        totalNS: TraceRow.range?.totalNS || 0,
                        slicesTime: TraceRow.range?.slicesTime,
                        scale: TraceRow.range?.scale || 50,
                        intervalPerf: SpHiPerf.stringResult?.fValue || 1,
                        range:TraceRow.range,
                        frame: thread.frame
                    }, thread.getTransferArray(), (res: any, hover: any) => {
                        thread.must = false;
                        if (thread.isHover) {
                            SpHiPerf.hoverThreadStruct = hover;
                        }
                    })
                    thread.isTransferCanvas = true;
                }
                this.trace.rowsEL?.appendChild(thread)
            });
        })
    }
}
