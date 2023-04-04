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

import {cpu, CpuStruct, WakeupBean} from "./ProcedureWorkerCPU.js";
import {ColorUtils, drawFlagLine, drawLines, ns2s, ns2x, Rect} from "./ProcedureWorkerCommon.js";
import {CpuFreqStruct, freq} from "./ProcedureWorkerFreq.js";
import {proc, ProcessStruct} from "./ProcedureWorkerProcess.js";
import {mem, ProcessMemStruct} from "./ProcedureWorkerMem.js";
import {thread, ThreadStruct} from "./ProcedureWorkerThread.js";
import {func, FuncStruct} from "./ProcedureWorkerFunc.js";
import {fps, FpsStruct} from "./ProcedureWorkerFPS.js";
import {heap, HeapStruct} from "./ProcedureWorkerHeap.js";
import {timeline} from "./ProcedureWorkerTimeline.js";
import {cpuAbility, CpuAbilityMonitorStruct} from "./ProcedureWorkerCpuAbility.js";
import {memoryAbility, MemoryAbilityMonitorStruct} from "./ProcedureWorkerMemoryAbility.js";
import {DiskAbilityMonitorStruct, diskIoAbility} from "./ProcedureWorkerDiskIoAbility.js";
import {networkAbility, NetworkAbilityMonitorStruct} from "./ProcedureWorkerNetworkAbility.js";
import {hiPerfCpu, HiPerfCpuStruct} from "./ProcedureWorkerHiPerfCPU.js";
import {hiPerfProcess, HiPerfProcessStruct} from "./ProcedureWorkerHiPerfProcess.js";
import {hiPerfThread, HiPerfThreadStruct} from "./ProcedureWorkerHiPerfThread.js";
import {HiPerfEvent, HiPerfEventStruct} from "./ProcedureWorkerHiPerfEvent.js";
import {HiPerfReportStruct} from "./ProcedureWorkerHiPerfReport.js";

let dataList: any = {}
let dataList2: any = {}
let dataList3: any = {}
let dataList4: any = {}
let dataFilter: any = {}
let canvasList: any = {}
let contextList: any = {}

function drawSelection(context: any, params: any) {
    if (params.isRangeSelect && params.rangeSelectObject) {
        params.rangeSelectObject!.startX = Math.floor(ns2x(params.rangeSelectObject!.startNS!, params.startNS, params.endNS, params.totalNS, params.frame));
        params.rangeSelectObject!.endX = Math.floor(ns2x(params.rangeSelectObject!.endNS!, params.startNS, params.endNS, params.totalNS, params.frame));
        if (context) {
            context.globalAlpha = 0.5
            context.fillStyle = "#666666"
            context.fillRect(params.rangeSelectObject!.startX!, params.frame.y, params.rangeSelectObject!.endX! - params.rangeSelectObject!.startX!, params.frame.height)
            context.globalAlpha = 1
        }
    }
}

function drawWakeUp(context: CanvasRenderingContext2D | any, wake: WakeupBean | null, startNS: number, endNS: number, totalNS: number, frame: Rect, selectCpuStruct: CpuStruct | undefined = undefined, currentCpu: number | undefined = undefined) {
    if (wake) {
        let x1 = Math.floor(ns2x((wake.wakeupTime || 0), startNS, endNS, totalNS, frame));
        context.beginPath();
        context.lineWidth = 2;
        context.fillStyle = "#000000";
        if (x1 > 0 && x1 < frame.x + frame.width) {
            context.moveTo(x1, frame.y);
            context.lineTo(x1, frame.y + frame.height);
            if (currentCpu == wake.cpu) {
                let centerY = Math.floor(frame.y + frame.height / 2);
                context.moveTo(x1, centerY - 6);
                context.lineTo(x1 + 4, centerY);
                context.lineTo(x1, centerY + 6);
                context.lineTo(x1 - 4, centerY);
                context.lineTo(x1, centerY - 6);
                context.fill();
            }
        }
        if (selectCpuStruct) {
            let x2 = Math.floor(ns2x((selectCpuStruct.startTime || 0), startNS, endNS, totalNS, frame));
            let y = frame.y + frame.height - 10;
            context.moveTo(x1, y);
            context.lineTo(x2, y);

            let s = ns2s((selectCpuStruct.startTime || 0) - (wake.wakeupTime || 0));
            let distance = x2 - x1;
            if (distance > 12) {
                context.moveTo(x1, y);
                context.lineTo(x1 + 6, y - 3);
                context.moveTo(x1, y);
                context.lineTo(x1 + 6, y + 3);
                context.moveTo(x2, y);
                context.lineTo(x2 - 6, y - 3);
                context.moveTo(x2, y);
                context.lineTo(x2 - 6, y + 3);
                let measure = context.measureText(s);
                let tHeight = measure.actualBoundingBoxAscent + measure.actualBoundingBoxDescent
                let xStart = x1 + Math.floor(distance / 2 - measure.width / 2);
                if (distance > measure.width + 4) {
                    context.fillStyle = "#ffffff"
                    context.fillRect(xStart - 2, y - 4 - tHeight, measure.width + 4, tHeight + 4);
                    context.font = "10px solid";
                    context.fillStyle = "#000000";
                    context.textBaseline = "bottom";
                    context.fillText(s, xStart, y - 2);
                }

            }
        }
        context.strokeStyle = "#000000";
        context.stroke();
        context.closePath();
    }
}

function drawLoading(ctx: CanvasRenderingContext2D, startNS: number, endNS: number, totalNS: number, frame: any, left: number, right: number) {
}

self.onmessage = function (e: any) {
    if ((e.data.type as string).startsWith("clear")) {
        dataList = {};
        dataList2 = {};
        dataList3 = {};
        dataList4 = {};
        dataFilter = {};
        canvasList = {};
        contextList = {};
        // @ts-ignore
        self.postMessage({
            id: e.data.id,
            type: e.data.type,
            results: null,
        });
        return;
    }
    let res: any
    if (e.data.params.list) {
        dataList[e.data.type] = e.data.params.list;
        if (e.data.params.offscreen) {
            canvasList[e.data.type] = e.data.params.offscreen;
            contextList[e.data.type] = e.data.params.offscreen!.getContext('2d');
            contextList[e.data.type].scale(e.data.params.dpr, e.data.params.dpr);
        }
    }
    if (!dataFilter[e.data.type]) {
        dataFilter[e.data.type] = []//new Set();
    }
    let canvas = canvasList[e.data.type];
    let context = contextList[e.data.type];
    let type = e.data.type as string;
    let params = e.data.params;
    let online = e.data.params.online;
    let buf = e.data.params.buf;
    let isRangeSelect = e.data.params.isRangeSelect;
    let isHover = e.data.params.isHover;
    let xs = e.data.params.xs;
    let frame = e.data.params.frame;
    let flagMoveInfo = e.data.params.flagMoveInfo;
    let flagSelectedInfo = e.data.params.flagSelectedInfo;
    let hoverX = e.data.params.hoverX;
    let hoverY = e.data.params.hoverY;
    let startNS = e.data.params.startNS;
    let endNS = e.data.params.endNS;
    let totalNS = e.data.params.totalNS;
    let slicesTime: { startTime: number | null, endTime: number | null, color: string | null } = e.data.params.slicesTime;
    let range = e.data.params.range;
    let scale = e.data.params.scale;
    let canvasWidth = e.data.params.canvasWidth;
    let canvasHeight = e.data.params.canvasHeight;
    let useCache = e.data.params.useCache;
    let lineColor = e.data.params.lineColor;
    let wakeupBean: WakeupBean | null = e.data.params.wakeupBean;
    let intervalPerf: number = e.data.params.intervalPerf;
    let lazyRefresh = false;
    if (canvas) {
        if (canvas.width !== canvasWidth || canvas.height !== canvasHeight) {
            canvas.width = canvasWidth;
            canvas.height = canvasHeight;
            context.scale(e.data.params.dpr, e.data.params.dpr);
        }
    }
    if (type.startsWith("timeline")) {
        timeline(canvas, context, startNS, endNS, totalNS, frame,
            e.data.params.keyPressCode, e.data.params.keyUpCode,
            e.data.params.mouseDown, e.data.params.mouseUp,
            e.data.params.mouseMove, e.data.params.mouseOut,
            e.data.params.offsetLeft, e.data.params.offsetTop,
            (a: any) => {
                //@ts-ignore
                self.postMessage({
                    id: "timeline",
                    type: "timeline-range-changed",
                    results: a,
                });
            }
        );
        // @ts-ignore
        self.postMessage({
            id: e.data.id,
            type: type,
            results: null,
        });
    } else if (type.startsWith("cpu")) {
        if (lazyRefresh) {
            cpu(dataList[type], dataFilter[type], startNS, endNS, totalNS, frame, useCache || !range.refresh);
        } else {
            if (!useCache) {
                cpu(dataList[type], dataFilter[type], startNS, endNS, totalNS, frame, false);
            }
        }
        if (canvas) {
            context.clearRect(0, 0, canvas.width, canvas.height);
            let arr = dataFilter[type];
            if (arr.length > 0 && !range.refresh && !useCache && lazyRefresh) {
                drawLoading(context, startNS, endNS, totalNS, frame, arr[0].startTime, arr[arr.length - 1].startTime + arr[arr.length - 1].dur)
            }
            context.beginPath();
            drawLines(context, xs, frame.height, lineColor);
            CpuStruct.hoverCpuStruct = undefined;
            if (isHover) {
                for (let re of dataFilter[e.data.type]) {
                    if (re.frame && hoverX >= re.frame.x && hoverX <= re.frame.x + re.frame.width && hoverY >= re.frame.y && hoverY <= re.frame.y + re.frame.height) {
                        CpuStruct.hoverCpuStruct = re;
                        break;
                    }
                }
            } else {
                CpuStruct.hoverCpuStruct = e.data.params.hoverCpuStruct;
            }
            CpuStruct.selectCpuStruct = e.data.params.selectCpuStruct;
            context.font = "11px sans-serif";
            for (let re of dataFilter[type]) {
                CpuStruct.draw(context, re);
            }
            drawSelection(context, params);
            context.closePath();
            drawFlagLine(context, flagMoveInfo, flagSelectedInfo, startNS, endNS, totalNS, frame, slicesTime);
            let currentCpu = parseInt(type.replace("cpu", ""));
            drawWakeUp(context, wakeupBean, startNS, endNS, totalNS, frame, type == `cpu${CpuStruct.selectCpuStruct?.cpu || 0}` ? CpuStruct.selectCpuStruct : undefined, currentCpu);
        }
        // @ts-ignore
        self.postMessage({
            id: e.data.id,
            type: type,
            results: canvas ? undefined : dataFilter[type],
            hover: CpuStruct.hoverCpuStruct
        });
    } else if (type.startsWith("fps")) {
        if (lazyRefresh) {
            fps(dataList[type], dataFilter[type], startNS, endNS, totalNS, frame, useCache || !range.refresh);
        } else {
            if (!useCache) {
                fps(dataList[type], dataFilter[type], startNS, endNS, totalNS, frame, false);
            }
        }
        if (canvas) {
            context.clearRect(0, 0, canvas.width, canvas.height);
            let arr = dataFilter[type];
            if (arr.length > 0 && !range.refresh && !useCache && lazyRefresh) {
                drawLoading(context, startNS, endNS, totalNS, frame, arr[0].startNS, arr[arr.length - 1].startNS + arr[arr.length - 1].dur)
            }
            context.beginPath();
            drawLines(context, xs, frame.height, lineColor)
            FpsStruct.hoverFpsStruct = undefined;
            if (isHover) {
                for (let re of dataFilter[e.data.type]) {
                    if (re.frame && hoverX >= re.frame.x && hoverX <= re.frame.x + re.frame.width && hoverY >= re.frame.y && hoverY <= re.frame.y + re.frame.height) {
                        FpsStruct.hoverFpsStruct = re;
                        break;
                    }
                }
            } else {
                FpsStruct.hoverFpsStruct = e.data.params.hoverFpsStruct;
            }
            for (let re of dataFilter[type]) {
                FpsStruct.draw(context, re)
            }
            drawSelection(context, params);
            context.closePath();
            let maxFps = FpsStruct.maxFps + "FPS"
            let textMetrics = context.measureText(maxFps);
            context.globalAlpha = 0.8
            context.fillStyle = "#f0f0f0"
            context.fillRect(0, 5, textMetrics.width + 8, 18)
            context.globalAlpha = 1
            context.fillStyle = "#333"
            context.textBaseline = "middle"
            context.fillText(maxFps, 4, 5 + 9);
            drawWakeUp(context, wakeupBean, startNS, endNS, totalNS, frame);
            drawFlagLine(context, flagMoveInfo, flagSelectedInfo, startNS, endNS, totalNS, frame, slicesTime);
        }
        // @ts-ignore
        self.postMessage({
            id: e.data.id,
            type: type,
            results: canvas ? undefined : dataFilter[type],
            hover: FpsStruct.hoverFpsStruct
        });
    } else if (type.startsWith("freq")) {
        if (lazyRefresh) {
            freq(dataList[type], dataFilter[type], startNS, endNS, totalNS, frame, useCache || !range.refresh);
        } else {
            if (!useCache) {
                freq(dataList[type], dataFilter[type], startNS, endNS, totalNS, frame, false);
            }
        }
        if (canvas) {
            context.clearRect(0, 0, canvas.width, canvas.height);
            let arr = dataFilter[type];
            if (arr.length > 0 && !range.refresh && !useCache && lazyRefresh) {
                drawLoading(context, startNS, endNS, totalNS, frame, arr[0].startNS, arr[arr.length - 1].startNS + arr[arr.length - 1].dur)
            }
            context.beginPath();
            CpuFreqStruct.maxFreq = e.data.params.maxFreq;
            CpuFreqStruct.maxFreqName = e.data.params.maxFreqName;
            drawLines(context, xs, frame.height, lineColor)
            CpuFreqStruct.hoverCpuFreqStruct = undefined;
            if (isHover) {
                for (let re of dataFilter[type]) {
                    if (re.frame && hoverX >= re.frame.x && hoverX <= re.frame.x + re.frame.width && hoverY >= re.frame.y && hoverY <= re.frame.y + re.frame.height) {
                        CpuFreqStruct.hoverCpuFreqStruct = re;
                        break;
                    }
                }
            } else {
                CpuFreqStruct.hoverCpuFreqStruct = e.data.params.hoverCpuFreqStruct;
            }
            CpuFreqStruct.selectCpuFreqStruct = e.data.params.selectCpuFreqStruct;
            for (let re of dataFilter[type]) {
                CpuFreqStruct.draw(context, re)
            }
            drawSelection(context, params);
            context.closePath();
            let s = CpuFreqStruct.maxFreqName
            let textMetrics = context.measureText(s);
            context.globalAlpha = 0.8
            context.fillStyle = "#f0f0f0"
            context.fillRect(0, 5, textMetrics.width + 8, 18)
            context.globalAlpha = 1
            context.fillStyle = "#333"
            context.textBaseline = "middle"
            context.fillText(s, 4, 5 + 9)
            drawWakeUp(context, wakeupBean, startNS, endNS, totalNS, frame);
            drawFlagLine(context, flagMoveInfo, flagSelectedInfo, startNS, endNS, totalNS, frame, slicesTime);
        }
        // @ts-ignore
        self.postMessage({
            id: e.data.id,
            type: type,
            results: canvas ? undefined : dataFilter[type],
            hover: CpuFreqStruct.hoverCpuFreqStruct
        });
    } else if (type.startsWith("process")) {
        if (lazyRefresh) {
            proc(dataList[type], dataFilter[type], startNS, endNS, totalNS, frame, useCache || !range.refresh);
        } else {
            if (!useCache) {
                proc(dataList[type], dataFilter[type], startNS, endNS, totalNS, frame, false);
            }
        }
        if (canvas) {
            context.clearRect(0, 0, canvas.width, canvas.height);
            let arr = dataFilter[type];
            if (arr.length > 0 && !range.refresh && !useCache && lazyRefresh) {
                drawLoading(context, startNS, endNS, totalNS, frame, arr[0].startTime, arr[arr.length - 1].startTime + arr[arr.length - 1].dur)
            }
            context.beginPath();
            CpuStruct.cpuCount = e.data.params.cpuCount;
            drawLines(context, xs, frame.height, lineColor)
            let path = new Path2D();
            let miniHeight: number = 0;
            miniHeight = Math.round((frame.height - (CpuStruct.cpuCount * 2)) / CpuStruct.cpuCount)
            context.fillStyle = ColorUtils.colorForTid(e.data.params.pid || 0)
            for (let re of dataFilter[type]) {
                ProcessStruct.draw(context, path, re, miniHeight);
            }
            context.fill(path);
            drawSelection(context, params);
            drawWakeUp(context, wakeupBean, startNS, endNS, totalNS, frame);
            context.closePath();
            drawFlagLine(context, flagMoveInfo, flagSelectedInfo, startNS, endNS, totalNS, frame, slicesTime);
        }
        // @ts-ignore
        self.postMessage({id: e.data.id, type: type, results: canvas ? undefined : dataFilter[type], hover: undefined});
    } else if (type.startsWith("heap")) {
        if (lazyRefresh) {
            heap(dataList[type], dataFilter[type], startNS, endNS, totalNS, frame, useCache || !range.refresh);
        } else {
            if (!useCache) {
                heap(dataList[type], dataFilter[type], startNS, endNS, totalNS, frame, false);
            }
        }
        if (canvas) {
            context.clearRect(0, 0, canvas.width, canvas.height);
            let arr = dataFilter[type];
            if (arr.length > 0 && !range.refresh && !useCache && lazyRefresh) {
                drawLoading(context, startNS, endNS, totalNS, frame, arr[0].startTime, arr[arr.length - 1].startTime + arr[arr.length - 1].dur)
            }
            context.beginPath();
            drawLines(context, xs, frame.height, lineColor)
            HeapStruct.hoverHeapStruct = undefined;
            if (isHover) {
                for (let re of dataFilter[e.data.type]) {
                    if (re.frame && hoverX >= re.frame.x && hoverX <= re.frame.x + re.frame.width && hoverY >= re.frame.y && hoverY <= re.frame.y + re.frame.height) {
                        HeapStruct.hoverHeapStruct = re;
                        break;
                    }
                }
            } else {
                HeapStruct.hoverHeapStruct = e.data.params.hoverHeapStruct;
            }
            for (let re of dataFilter[type]) {
                HeapStruct.draw(context, re)
            }
            drawSelection(context, params);
            drawWakeUp(context, wakeupBean, startNS, endNS, totalNS, frame);
            context.closePath();
            drawFlagLine(context, flagMoveInfo, flagSelectedInfo, startNS, endNS, totalNS, frame, slicesTime);
        }
        // @ts-ignore
        self.postMessage({
            id: e.data.id,
            type: type,
            results: canvas ? undefined : dataFilter[type],
            hover: HeapStruct.hoverHeapStruct
        });
    } else if (type.startsWith("mem")) {
        if (lazyRefresh) {
            mem(dataList[type], dataFilter[type], startNS, endNS, totalNS, frame, useCache || !range.refresh);
        } else {
            if (!useCache) {
                mem(dataList[type], dataFilter[type], startNS, endNS, totalNS, frame, false);
            }
        }
        if (canvas) {
            context.clearRect(0, 0, canvas.width, canvas.height);
            let arr = dataFilter[type];
            if (arr.length > 0 && !range.refresh && !useCache && lazyRefresh) {
                drawLoading(context, startNS, endNS, totalNS, frame, arr[0].startTime, arr[arr.length - 1].startTime + arr[arr.length - 1].dur)
            }
            context.beginPath();
            drawLines(context, xs, frame.height, lineColor)
            ProcessMemStruct.hoverProcessMemStruct = undefined;
            if (isHover) {
                for (let re of dataFilter[type]) {
                    if (re.frame && hoverX >= re.frame.x && hoverX <= re.frame.x + re.frame.width && hoverY >= re.frame.y && hoverY <= re.frame.y + re.frame.height) {
                        ProcessMemStruct.hoverProcessMemStruct = re;
                        break;
                    }
                }
            } else {
                ProcessMemStruct.hoverProcessMemStruct = e.data.params.hoverProcessMemStruct;
            }
            for (let re of dataFilter[type]) {
                ProcessMemStruct.draw(context, re)
            }
            drawSelection(context, params);
            drawWakeUp(context, wakeupBean, startNS, endNS, totalNS, frame);
            context.closePath();
            drawFlagLine(context, flagMoveInfo, flagSelectedInfo, startNS, endNS, totalNS, frame, slicesTime);
        }
        // @ts-ignore
        self.postMessage({
            id: e.data.id,
            type: type,
            results: canvas ? undefined : dataFilter[type],
            hover: ProcessMemStruct.hoverProcessMemStruct
        });
    } else if (type.startsWith("thread")) {
        if (lazyRefresh) {
            thread(dataList[type], dataFilter[type], startNS, endNS, totalNS, frame, useCache || !range.refresh);
        } else {
            if (!useCache) {
                thread(dataList[type], dataFilter[type], startNS, endNS, totalNS, frame, false);
            }
        }
        if (canvas) {
            context.clearRect(0, 0, canvas.width, canvas.height);
            let arr = dataFilter[type];
            if (arr.length > 0 && !range.refresh && !useCache && lazyRefresh) {
                drawLoading(context, startNS, endNS, totalNS, frame, arr[0].startTime, arr[arr.length - 1].startTime + arr[arr.length - 1].dur)
            }
            context.beginPath();
            drawLines(context, xs, frame.height, lineColor)
            ThreadStruct.hoverThreadStruct = undefined;
            if (isHover) {
                for (let re of dataFilter[e.data.type]) {
                    if (re.frame && hoverX >= re.frame.x && hoverX <= re.frame.x + re.frame.width && hoverY >= re.frame.y && hoverY <= re.frame.y + re.frame.height) {
                        ThreadStruct.hoverThreadStruct = re;
                        break;
                    }
                }
            } else {
                ThreadStruct.hoverThreadStruct = e.data.params.hoverThreadStruct;
            }
            ThreadStruct.selectThreadStruct = e.data.params.selectThreadStruct;
            for (let re of dataFilter[type]) {
                ThreadStruct.draw(context, re)
            }
            drawSelection(context, params);
            drawWakeUp(context, wakeupBean, startNS, endNS, totalNS, frame);
            context.closePath();
            drawFlagLine(context, flagMoveInfo, flagSelectedInfo, startNS, endNS, totalNS, frame, slicesTime);
        }
        // @ts-ignore
        self.postMessage({
            id: e.data.id,
            type: type,
            results: canvas ? undefined : dataFilter[type],
            hover: ThreadStruct.hoverThreadStruct
        });
    } else if (type.startsWith("func")) {
        if (!e.data.params.isLive){
            self.postMessage({
                id: e.data.id,
                type: type,
                results: canvas ? undefined : dataFilter[type],
                hover: undefined
            });
            return;
        }
        if (lazyRefresh) {
            func(dataList[type], dataFilter[type], startNS, endNS, totalNS, frame, useCache || !range.refresh);
        } else {
            if (!useCache) {
                func(dataList[type], dataFilter[type], startNS, endNS, totalNS, frame, false);
            }
        }
        if (canvas) {
            if (canvas.height == 150) {
                canvas.width = frame.width;
                canvas.height = e.data.params.maxHeight;
                context.scale(e.data.params.dpr, e.data.params.dpr);
            }
            context.clearRect(0, 0, canvas.width, canvas.height);
            let arr = dataFilter[type];
            if (arr.length > 0 && !range.refresh && !useCache && lazyRefresh) {
                drawLoading(context, startNS, endNS, totalNS, frame, arr[0].startTs, arr[arr.length - 1].startTs + arr[arr.length - 1].dur)
            }
            context.beginPath();
            drawLines(context, xs, frame.height, lineColor)
            FuncStruct.hoverFuncStruct = undefined;
            if (isHover) {
                for (let re of dataFilter[type]) {
                    if (re.dur == 0 || re.dur == null || re.dur == undefined) {
                        if (re.frame && hoverX >= re.frame.x - 5 && hoverX <= re.frame.x + 5 && hoverY >= re.frame.y + (re.depth * 20) && hoverY <= re.frame.y + re.frame.height + (re.depth * 20)) {
                            FuncStruct.hoverFuncStruct = re;
                            break;
                        }
                    } else {
                        if (re.frame && hoverX >= re.frame.x && hoverX <= re.frame.x + re.frame.width && hoverY >= re.frame.y + (re.depth * 20) && hoverY <= re.frame.y + re.frame.height + (re.depth * 20)) {
                            FuncStruct.hoverFuncStruct = re;
                            break;
                        }
                    }
                }
            } else {
                FuncStruct.hoverFuncStruct = e.data.params.hoverFuncStruct;
            }
            FuncStruct.selectFuncStruct = e.data.params.selectFuncStruct;
            for (let re of dataFilter[type]) {
                FuncStruct.draw(context, re, totalNS)
            }
            drawSelection(context, params);
            drawWakeUp(context, wakeupBean, startNS, endNS, totalNS, frame);
            context.closePath();
            drawFlagLine(context, flagMoveInfo, flagSelectedInfo, startNS, endNS, totalNS, frame, slicesTime);
        }
        // @ts-ignore
        self.postMessage({
            id: e.data.id,
            type: type,
            results: canvas ? undefined : dataFilter[type],
            hover: FuncStruct.hoverFuncStruct
        });
    } else if (type.startsWith("native")) {
        if (canvas) {
            context.clearRect(0, 0, canvas.width, canvas.height);
            context.beginPath();
            drawLines(context, xs, frame.height, lineColor)
            drawSelection(context, params);
            context.closePath();
            drawFlagLine(context, flagMoveInfo, flagSelectedInfo, startNS, endNS, totalNS, frame, slicesTime);
        }
        // @ts-ignore
        self.postMessage({
            id: e.data.id,
            type: type,
            results: canvas ? undefined : dataFilter[type],
            hover: ThreadStruct.hoverThreadStruct
        });
    } else if (type.startsWith("HiPerf-Group") || type.startsWith("monitorGroup")) {
        if (canvas) {
            context.clearRect(0, 0, canvas.width, canvas.height);
            context.beginPath();
            drawLines(context, xs, frame.height, lineColor)
            drawSelection(context, params);
            context.closePath();
            drawFlagLine(context, flagMoveInfo, flagSelectedInfo, startNS, endNS, totalNS, frame, slicesTime);
        }
        // @ts-ignore
        self.postMessage({
            id: e.data.id,
            type: type,
            results: canvas ? undefined : dataFilter[type],
            hover: ThreadStruct.hoverThreadStruct
        });
    } else if (type.startsWith("HiPerf-Cpu")) {
        let groupBy10MS = scale > 100_000_000;
        if (lazyRefresh) {
            hiPerfCpu(dataList[type], dataList2, type, dataFilter[type], startNS, endNS, totalNS, frame, groupBy10MS, e.data.params.maxCpu, intervalPerf, useCache || !range.refresh);
        } else {
            if (!useCache) {
                hiPerfCpu(dataList[type], dataList2, type, dataFilter[type], startNS, endNS, totalNS, frame, groupBy10MS, e.data.params.maxCpu, intervalPerf, false);
            }
        }
        if (canvas) {
            context.clearRect(0, 0, canvas.width, canvas.height);
            let arr = dataFilter[type];
            if (arr.length > 0 && !range.refresh && !useCache && lazyRefresh) {
                drawLoading(context, startNS, endNS, totalNS, frame, arr[0].startNS, arr[arr.length - 1].startNS + arr[arr.length - 1].dur)
            }
            drawLines(context, xs, frame.height, lineColor)
            context.stroke();
            context.beginPath();
            HiPerfCpuStruct.hoverStruct = undefined;
            if (isHover) {
                let offset = groupBy10MS ? 0 : 3;
                for (let re of dataFilter[e.data.type]) {
                    if (re.frame && hoverX >= re.frame.x - offset && hoverX <= re.frame.x + re.frame.width + offset) {//&& hoverY >= re.frame.y && hoverY <= re.frame.y + re.frame.height
                        HiPerfCpuStruct.hoverStruct = re;
                        break;
                    }
                }
            } else {
                HiPerfCpuStruct.hoverStruct = e.data.params.hoverStruct;
            }
            HiPerfCpuStruct.selectStruct = e.data.params.selectStruct;
            context.fillStyle = ColorUtils.FUNC_COLOR[0];
            context.strokeStyle = ColorUtils.FUNC_COLOR[0];
            let path = new Path2D();
            for (let re of dataFilter[type]) {
                HiPerfCpuStruct.draw(context, path, re, groupBy10MS);
            }
            if (groupBy10MS) {
                context.fill(path);
            } else {
                context.stroke(path);
            }
            drawSelection(context, params);
            context.closePath();
            drawFlagLine(context, flagMoveInfo, flagSelectedInfo, startNS, endNS, totalNS, frame, slicesTime);
        }
        // @ts-ignore
        self.postMessage({
            id: e.data.id,
            type: type,
            results: canvas ? undefined : dataFilter[type],
            hover: HiPerfCpuStruct.hoverStruct
        });
    } else if (type.startsWith("HiPerf-Process")) {
        let groupBy10MS = scale > 100_000_000;
        if (lazyRefresh) {
            hiPerfProcess(dataList[type], dataFilter[type], startNS, endNS, totalNS, frame, groupBy10MS, intervalPerf, useCache || !range.refresh);
        } else {
            if (!useCache) {
                hiPerfProcess(dataList[type], dataFilter[type], startNS, endNS, totalNS, frame, groupBy10MS, intervalPerf, false);
            }
        }
        if (canvas) {
            context.clearRect(0, 0, canvas.width, canvas.height);
            let arr = dataFilter[type];
            if (arr.length > 0 && !range.refresh && !useCache && lazyRefresh) {
                drawLoading(context, startNS, endNS, totalNS, frame, arr[0].startNS, arr[arr.length - 1].startNS + arr[arr.length - 1].dur)
            }
            drawLines(context, xs, frame.height, lineColor)
            context.stroke();
            context.beginPath();
            HiPerfProcessStruct.hoverStruct = undefined;
            context.fillStyle = ColorUtils.FUNC_COLOR[0];
            context.strokeStyle = ColorUtils.FUNC_COLOR[0];
            if (isHover) {
                let offset = groupBy10MS ? 0 : 3;
                for (let re of dataFilter[e.data.type]) {
                    if (re.frame && hoverX >= re.frame.x - offset && hoverX <= re.frame.x + re.frame.width + offset) {//&& hoverY >= re.frame.y && hoverY <= re.frame.y + re.frame.height
                        HiPerfProcessStruct.hoverStruct = re;
                        break;
                    }
                }
            } else {
                HiPerfProcessStruct.hoverStruct = e.data.params.hoverStruct;
            }
            HiPerfProcessStruct.selectStruct = e.data.params.selectStruct;
            let path = new Path2D();
            for (let re of dataFilter[type]) {
                HiPerfProcessStruct.draw(context, path, re, groupBy10MS);
            }
            groupBy10MS ? context.fill(path) : context.stroke(path);
            context.closePath();
            drawSelection(context, params);
            drawFlagLine(context, flagMoveInfo, flagSelectedInfo, startNS, endNS, totalNS, frame, slicesTime);
        }
        // @ts-ignore
        self.postMessage({
            id: e.data.id,
            type: type,
            results: canvas ? undefined : dataFilter[type],
            hover: HiPerfProcessStruct.hoverStruct
        });
    } else if (type.startsWith("HiPerf-Thread")) {
        let groupBy10MS = scale > 100_000_000;
        if (lazyRefresh) {
            hiPerfThread(dataList[type], dataFilter[type], startNS, endNS, totalNS, frame, groupBy10MS, intervalPerf, useCache || !range.refresh);
        } else {
            if (!useCache) {
                hiPerfThread(dataList[type], dataFilter[type], startNS, endNS, totalNS, frame, groupBy10MS, intervalPerf, false);
            }
        }
        if (canvas) {
            context.clearRect(0, 0, canvas.width, canvas.height);
            let arr = dataFilter[type];
            if (arr.length > 0 && !range.refresh && !useCache && lazyRefresh) {
                drawLoading(context, startNS, endNS, totalNS, frame, arr[0].startNS, arr[arr.length - 1].startNS + arr[arr.length - 1].dur)
            }
            drawLines(context, xs, frame.height, lineColor)
            context.stroke();
            context.beginPath();
            HiPerfThreadStruct.hoverStruct = undefined;
            context.fillStyle = ColorUtils.FUNC_COLOR[0];
            context.strokeStyle = ColorUtils.FUNC_COLOR[0];
            if (isHover) {
                let offset = groupBy10MS ? 0 : 3;
                for (let re of dataFilter[e.data.type]) {
                    if (re.frame && hoverX >= re.frame.x - offset && hoverX <= re.frame.x + re.frame.width + offset) {//&& hoverY >= re.frame.y && hoverY <= re.frame.y + re.frame.height
                        HiPerfThreadStruct.hoverStruct = re;
                        break;
                    }
                }
            } else {
                HiPerfThreadStruct.hoverStruct = e.data.params.hoverStruct;
            }
            HiPerfThreadStruct.selectStruct = e.data.params.selectStruct;
            let path = new Path2D();
            for (let re of dataFilter[type]) {
                HiPerfThreadStruct.draw(context, path, re, groupBy10MS);
            }
            groupBy10MS ? context.fill(path) : context.stroke(path);
            context.stroke();
            context.closePath();
            drawSelection(context, params);
            drawFlagLine(context, flagMoveInfo, flagSelectedInfo, startNS, endNS, totalNS, frame, slicesTime);
        }
        // @ts-ignore
        self.postMessage({
            id: e.data.id,
            type: type,
            results: canvas ? undefined : dataFilter[type],
            hover: HiPerfThreadStruct.hoverStruct
        });
    } else if (type.startsWith("monitorCpu")) {
        if (lazyRefresh) {
            cpuAbility(dataList[type], dataFilter[type], startNS, endNS, totalNS, frame, useCache || !range.refresh);
        } else {
            if (!useCache) {
                cpuAbility(dataList[type], dataFilter[type], startNS, endNS, totalNS, frame, false);
            }
        }
        if (canvas) {
            context.clearRect(0, 0, canvas.width, canvas.height);
            let arr = dataFilter[type];
            if (arr.length > 0 && !range.refresh && !useCache && lazyRefresh) {
                drawLoading(context, startNS, endNS, totalNS, frame, arr[0].startNS, arr[arr.length - 1].startNS + arr[arr.length - 1].dur)
            }
            context.beginPath();
            CpuAbilityMonitorStruct.maxCpuUtilization = e.data.params.maxCpuUtilization;
            CpuAbilityMonitorStruct.maxCpuUtilizationName = e.data.params.maxCpuUtilizationName;
            drawLines(context, xs, frame.height, lineColor)
            CpuAbilityMonitorStruct.hoverCpuAbilityStruct = undefined;
            if (isHover) {
                for (let re of dataFilter[type]) {
                    if (re.frame && hoverX >= re.frame.x && hoverX <= re.frame.x + re.frame.width && hoverY >= re.frame.y && hoverY <= re.frame.y + re.frame.height) {
                        CpuAbilityMonitorStruct.hoverCpuAbilityStruct = re;
                        break;
                    }
                }
            }
            CpuAbilityMonitorStruct.selectCpuAbilityStruct = e.data.params.selectCpuAbilityStruct;
            for (let re of dataFilter[type]) {
                CpuAbilityMonitorStruct.draw(context, re)
            }
            drawSelection(context, params);
            context.closePath();
            let s = CpuAbilityMonitorStruct.maxCpuUtilizationName
            let textMetrics = context.measureText(s);
            context.globalAlpha = 0.8
            context.fillStyle = "#f0f0f0"
            context.fillRect(0, 5, textMetrics.width + 8, 18)
            context.globalAlpha = 1
            context.fillStyle = "#333"
            context.textBaseline = "middle"
            context.fillText(s, 4, 5 + 9)
            drawWakeUp(context, wakeupBean, startNS, endNS, totalNS, frame);
            drawFlagLine(context, flagMoveInfo, flagSelectedInfo, startNS, endNS, totalNS, frame, slicesTime);
        }
        // @ts-ignore
        self.postMessage({
            id: e.data.id,
            type: type,
            results: canvas ? undefined : dataFilter[type],
            hover: CpuAbilityMonitorStruct.hoverCpuAbilityStruct
        });
    } else if (type.startsWith("monitorMemory")) {
        if (lazyRefresh) {
            memoryAbility(dataList[type], dataFilter[type], startNS, endNS, totalNS, frame, useCache || !range.refresh);
        } else {
            if (!useCache) {
                memoryAbility(dataList[type], dataFilter[type], startNS, endNS, totalNS, frame, false);
            }
        }
        if (canvas) {
            context.clearRect(0, 0, canvas.width, canvas.height);
            let arr = dataFilter[type];
            if (arr.length > 0 && !range.refresh && !useCache && lazyRefresh) {
                drawLoading(context, startNS, endNS, totalNS, frame, arr[0].startNS, arr[arr.length - 1].startNS + arr[arr.length - 1].dur)
            }
            context.beginPath();
            MemoryAbilityMonitorStruct.maxMemoryByte = e.data.params.maxMemoryByte;
            MemoryAbilityMonitorStruct.maxMemoryByteName = e.data.params.maxMemoryByteName;
            drawLines(context, xs, frame.height, lineColor)
            MemoryAbilityMonitorStruct.hoverMemoryAbilityStruct = undefined;
            if (isHover) {
                for (let re of dataFilter[type]) {
                    if (re.frame && hoverX >= re.frame.x && hoverX <= re.frame.x + re.frame.width && hoverY >= re.frame.y && hoverY <= re.frame.y + re.frame.height) {
                        MemoryAbilityMonitorStruct.hoverMemoryAbilityStruct = re;
                        break;
                    }
                }
            }
            MemoryAbilityMonitorStruct.selectMemoryAbilityStruct = e.data.params.selectMemoryAbilityStruct;
            for (let re of dataFilter[type]) {
                MemoryAbilityMonitorStruct.draw(context, re)
            }
            drawSelection(context, params);
            context.closePath();
            let s = MemoryAbilityMonitorStruct.maxMemoryByteName
            let textMetrics = context.measureText(s);
            context.globalAlpha = 0.8
            context.fillStyle = "#f0f0f0"
            context.fillRect(0, 5, textMetrics.width + 8, 18)
            context.globalAlpha = 1
            context.fillStyle = "#333"
            context.textBaseline = "middle"
            context.fillText(s, 4, 5 + 9)
            drawWakeUp(context, wakeupBean, startNS, endNS, totalNS, frame);
            drawFlagLine(context, flagMoveInfo, flagSelectedInfo, startNS, endNS, totalNS, frame, slicesTime);
        }
        // @ts-ignore
        self.postMessage({
            id: e.data.id,
            type: type,
            results: canvas ? undefined : dataFilter[type],
            hover: MemoryAbilityMonitorStruct.hoverMemoryAbilityStruct
        });

    } else if (type.startsWith("monitorDiskIo")) {
        if (lazyRefresh) {
            diskIoAbility(dataList[type], dataFilter[type], startNS, endNS, totalNS, frame, useCache || !range.refresh);
        } else {
            if (!useCache) {
                diskIoAbility(dataList[type], dataFilter[type], startNS, endNS, totalNS, frame, false);
            }
        }
        if (canvas) {
            context.clearRect(0, 0, canvas.width, canvas.height);
            let arr = dataFilter[type];
            if (arr.length > 0 && !range.refresh && !useCache && lazyRefresh) {
                drawLoading(context, startNS, endNS, totalNS, frame, arr[0].startNS, arr[arr.length - 1].startNS + arr[arr.length - 1].dur)
            }
            context.beginPath();
            let maxDiskRate = e.data.params.maxDiskRate;
            let maxDiskRateName = e.data.params.maxDiskRateName;
            drawLines(context, xs, frame.height, lineColor)
            DiskAbilityMonitorStruct.hoverDiskAbilityStruct = undefined;
            if (isHover) {
                for (let re of dataFilter[type]) {
                    if (re.frame && hoverX >= re.frame.x && hoverX <= re.frame.x + re.frame.width && hoverY >= re.frame.y && hoverY <= re.frame.y + re.frame.height) {
                        DiskAbilityMonitorStruct.hoverDiskAbilityStruct = re;
                        break;
                    }
                }
            }
            DiskAbilityMonitorStruct.selectDiskAbilityStruct = e.data.params.selectDiskAbilityStruct;
            for (let re of dataFilter[type]) {
                DiskAbilityMonitorStruct.draw(context, re, maxDiskRate)
            }
            drawSelection(context, params);
            context.closePath();
            let textMetrics = context.measureText(maxDiskRateName);
            context.globalAlpha = 0.8
            context.fillStyle = "#f0f0f0"
            context.fillRect(0, 5, textMetrics.width + 8, 18)
            context.globalAlpha = 1
            context.fillStyle = "#333"
            context.textBaseline = "middle"
            context.fillText(maxDiskRateName, 4, 5 + 9)
            drawWakeUp(context, wakeupBean, startNS, endNS, totalNS, frame);
            drawFlagLine(context, flagMoveInfo, flagSelectedInfo, startNS, endNS, totalNS, frame, slicesTime);
        }
        // @ts-ignore
        self.postMessage({
            id: e.data.id,
            type: type,
            results: canvas ? undefined : dataFilter[type],
            hover: DiskAbilityMonitorStruct.hoverDiskAbilityStruct
        });
    } else if (type.startsWith("monitorNetwork")) {
        if (lazyRefresh) {
            networkAbility(dataList[type], dataFilter[type], startNS, endNS, totalNS, frame, useCache || !range.refresh)
        } else {
            if (!useCache) {
                networkAbility(dataList[type], dataFilter[type], startNS, endNS, totalNS, frame, false)
            }
        }
        if (canvas) {
            context.clearRect(0, 0, canvas.width, canvas.height);
            let arr = dataFilter[type];
            if (arr.length > 0 && !range.refresh && !useCache && lazyRefresh) {
                drawLoading(context, startNS, endNS, totalNS, frame, arr[0].startNS, arr[arr.length - 1].startNS + arr[arr.length - 1].dur)
            }
            context.beginPath();
            let maxNetworkRate = e.data.params.maxNetworkRate;
            let maxNetworkRateName = e.data.params.maxNetworkRateName;
            drawLines(context, xs, frame.height, lineColor)
            NetworkAbilityMonitorStruct.hoverNetworkAbilityStruct = undefined;
            if (isHover) {
                for (let re of dataFilter[type]) {
                    if (re.frame && hoverX >= re.frame.x && hoverX <= re.frame.x + re.frame.width && hoverY >= re.frame.y && hoverY <= re.frame.y + re.frame.height) {
                        NetworkAbilityMonitorStruct.hoverNetworkAbilityStruct = re;
                        break;
                    }
                }
            }
            NetworkAbilityMonitorStruct.selectNetworkAbilityStruct = e.data.params.selectNetworkAbilityStruct;
            for (let re of dataFilter[type]) {
                NetworkAbilityMonitorStruct.draw(context, re, maxNetworkRate)
            }
            drawSelection(context, params);
            context.closePath();
            let textMetrics = context.measureText(maxNetworkRateName);
            context.globalAlpha = 0.8
            context.fillStyle = "#f0f0f0"
            context.fillRect(0, 5, textMetrics.width + 8, 18)
            context.globalAlpha = 1
            context.fillStyle = "#333"
            context.textBaseline = "middle"
            context.fillText(maxNetworkRateName, 4, 5 + 9)
            drawWakeUp(context, wakeupBean, startNS, endNS, totalNS, frame);
            drawFlagLine(context, flagMoveInfo, flagSelectedInfo, startNS, endNS, totalNS, frame, slicesTime);
        }
        // @ts-ignore
        self.postMessage({
            id: e.data.id,
            type: type,
            results: canvas ? undefined : dataFilter[type],
            hover: NetworkAbilityMonitorStruct.hoverNetworkAbilityStruct
        });

    } else if (type.startsWith("HiPerf-Report-Fold")) {
        let groupBy10MS = scale > 100_000_000;
        if (lazyRefresh) {
            HiPerfEvent(dataList[type], dataList3, type, dataFilter[type], startNS, endNS, totalNS, frame, groupBy10MS, intervalPerf, useCache || !range.refresh);
        } else {
            if (!useCache) {
                HiPerfEvent(dataList[type], dataList3, type, dataFilter[type], startNS, endNS, totalNS, frame, groupBy10MS, intervalPerf, false);
            }
        }
        if (canvas) {
            context.clearRect(0, 0, canvas.width, canvas.height);
            let arr = dataFilter[type];
            if (arr.length > 0 && !range.refresh && !useCache && lazyRefresh) {
                drawLoading(context, startNS, endNS, totalNS, frame, arr[0].startNS, arr[arr.length - 1].startNS + arr[arr.length - 1].dur)
            }
            drawLines(context, xs, frame.height, lineColor)
            context.stroke();
            context.beginPath();
            HiPerfReportStruct.hoverStruct = undefined;
            context.fillStyle = ColorUtils.FUNC_COLOR[0];
            context.strokeStyle = ColorUtils.FUNC_COLOR[0];
            if (isHover) {
                let offset = groupBy10MS ? 0 : 3;
                for (let re of dataFilter[e.data.type]) {
                    if (re.frame && hoverX >= re.frame.x - offset && hoverX <= re.frame.x + re.frame.width + offset) {//&& hoverY >= re.frame.y && hoverY <= re.frame.y + re.frame.height
                        HiPerfReportStruct.hoverStruct = re;
                        break;
                    }
                }
            } else {
                HiPerfReportStruct.hoverStruct = e.data.params.hoverStruct;
            }
            HiPerfReportStruct.selectStruct = e.data.params.selectStruct;
            let path = new Path2D();
            for (let re of dataFilter[type]) {
                HiPerfReportStruct.draw(context, path, re, groupBy10MS);
            }
            groupBy10MS ? context.fill(path) : context.stroke(path);
            context.closePath();
            drawSelection(context, params);
            drawFlagLine(context, flagMoveInfo, flagSelectedInfo, startNS, endNS, totalNS, frame, slicesTime);
        }
        // @ts-ignore
        self.postMessage({
            id: e.data.id,
            type: type,
            results: canvas ? undefined : dataFilter[type],
            hover: HiPerfReportStruct.hoverStruct
        });
    } else if (type.startsWith("HiPerf-Report-Event")) {
        let groupBy10MS = scale > 100_000_000;
        if (lazyRefresh) {
            HiPerfEvent(dataList[type], dataList4, type, dataFilter[type], startNS, endNS, totalNS, frame, groupBy10MS, intervalPerf, useCache || !range.refresh);
        } else {
            if (!useCache) {
                HiPerfEvent(dataList[type], dataList4, type, dataFilter[type], startNS, endNS, totalNS, frame, groupBy10MS, intervalPerf, false);
            }
        }
        if (canvas) {
            context.clearRect(0, 0, canvas.width, canvas.height);
            let arr = dataFilter[type];
            if (arr.length > 0 && !range.refresh && !useCache && lazyRefresh) {
                drawLoading(context, startNS, endNS, totalNS, frame, arr[0].startNS, arr[arr.length - 1].startNS + arr[arr.length - 1].dur)
            }
            drawLines(context, xs, frame.height, lineColor)
            context.stroke();
            context.beginPath();
            HiPerfEventStruct.hoverStruct = undefined;
            context.fillStyle = ColorUtils.FUNC_COLOR[0];
            context.strokeStyle = ColorUtils.FUNC_COLOR[0];
            if (isHover) {
                let offset = groupBy10MS ? 0 : 3;
                for (let re of dataFilter[e.data.type]) {
                    if (re.frame && hoverX >= re.frame.x - offset && hoverX <= re.frame.x + re.frame.width + offset) {//&& hoverY >= re.frame.y && hoverY <= re.frame.y + re.frame.height
                        HiPerfEventStruct.hoverStruct = re;
                        break;
                    }
                }
            } else {
                HiPerfEventStruct.hoverStruct = e.data.params.hoverStruct;
            }
            HiPerfEventStruct.selectStruct = e.data.params.selectStruct;
            let path = new Path2D();
            for (let re of dataFilter[type]) {
                HiPerfEventStruct.draw(context, path, re, groupBy10MS);
            }
            groupBy10MS ? context.fill(path) : context.stroke(path);
            drawSelection(context, params);
            let maxEvent = HiPerfEventStruct.maxEvent!.get(type) || 0;
            let textMetrics = context.measureText(maxEvent);
            context.globalAlpha = 0.8
            context.fillStyle = "#f0f0f0"
            context.fillRect(0, 5, textMetrics.width + 8, 18)
            context.globalAlpha = 1
            context.fillStyle = "#333"
            context.textBaseline = "middle"
            context.fillText(maxEvent, 4, 5 + 9);
            context.stroke();
            context.closePath();
            drawFlagLine(context, flagMoveInfo, flagSelectedInfo, startNS, endNS, totalNS, frame, slicesTime);
        }
        // @ts-ignore
        self.postMessage({
            id: e.data.id,
            type: type,
            results: canvas ? undefined : dataFilter[type],
            hover: HiPerfEventStruct.hoverStruct
        });
    }
};
self.onmessageerror = function (e: any) {
}
