// @ts-nocheck
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
import './trace/TimerShaftElement.js';
import './trace/base/TraceRow.js';
import './trace/base/TraceRowRecyclerView.js';
import {
  getAsyncEvents,
  getCpuUtilizationRate,
  getFps,
  getFunDataByTid,
  queryCpuData,
  queryCpuFreq,
  queryCpuFreqData,
  queryCpuMax,
  queryCpuMaxFreq,
  queryProcess,
  queryProcessData,
  queryProcessMem,
  queryProcessMemData,
  queryProcessThreads,
  queryThreadData,
  queryTotalTime,
  threadPool,
} from '../database/SqlLite.js';
import { TraceRow } from './trace/base/TraceRow.js';
import { TimerShaftElement } from './trace/TimerShaftElement.js';
import { TimeRange } from './trace/timer-shaft/RangeRuler.js';
import { CpuStruct } from '../bean/CpuStruct.js';
import { CpuFreqStruct } from '../bean/CpuFreqStruct.js';
import { ProcessStruct } from '../bean/ProcessStruct.js';
import { ColorUtils } from './trace/base/ColorUtils.js';
import './trace/base/TraceSheet.js';
import { TraceSheet } from './trace/base/TraceSheet.js';
import { ThreadStruct } from '../bean/ThreadStruct.js';
import { ProcessMemStruct } from '../bean/ProcessMemStruct.js';
import { FuncStruct } from '../bean/FuncStruct.js';
import { FpsStruct } from '../bean/FpsStruct.js';
import { RangeSelect } from './trace/base/RangeSelect.js';
import { SelectionParam } from '../bean/BoxSelection.js';
import { procedurePool } from '../database/Procedure.js';
import { SportRuler } from './trace/timer-shaft/SportRuler.js';
import { TraceRowRecyclerView } from './trace/base/TraceRowRecyclerView.js';
import { TraceRowObject } from './trace/base/TraceRowObject.js';
import { Rect } from './trace/timer-shaft/Rect.js';

@element('sp-recycler-system-trace')
export class SpRecyclerSystemTrace extends BaseElement {
  static scrollViewWidth = 0;
  rowsEL: TraceRowRecyclerView | undefined | null;
  private timerShaftEL: TimerShaftElement | null | undefined;
  private range: TimeRange | undefined;
  private traceSheetEL: TraceSheet | undefined | null;
  private rangeSelect!: RangeSelect;
  private processThreads: Array<ThreadStruct> = [];
  private processAsyncEvent: Array<ProcessMemStruct> = [];
  private processMem: Array<any> = [];

  initElements(): void {
    this.rowsEL = this.shadowRoot?.querySelector<TraceRowRecyclerView>('.rows');
    this.timerShaftEL = this.shadowRoot?.querySelector('.recycle-timer-shaft');
    this.traceSheetEL = this.shadowRoot?.querySelector('.recycle-trace-sheet');
    this.rangeSelect = new RangeSelect(this.timerShaftEL);
    this.rangeSelect.rowsEL = this.rowsEL;
    document?.addEventListener('flag-change', (event: any) => {
      let flag = event.detail;
      this.timerShaftEL?.modifyFlagList(event.detail.type, event.detail.flagObj);
      if (flag.hidden) {
        this.traceSheetEL?.setAttribute('mode', 'hidden');
      }
    });
    SpRecyclerSystemTrace.scrollViewWidth = this.getScrollWidth();
    this.rangeSelect.selectHandler = (rows) => {
      let selection = new SelectionParam();
      // 框选的 cpu ,无则不传
      selection.cpus = [];
      // 框选的 线程 id,无则不传
      selection.threadIds = [];
      // 款选的函数的 线程id ,无则不传
      selection.funTids = [];
      // 框选的 内存 trackId ,无则不传
      selection.processTrackIds = [];
      // 框选的起始时间
      selection.leftNs = 0;
      // 框选的结束时间
      selection.rightNs = 0;
      rows.forEach((it) => {
        if (it.rowType == TraceRow.ROW_TYPE_CPU) {
          selection.cpus.push(parseInt(it.rowId!));
        } else if (it.rowType == TraceRow.ROW_TYPE_THREAD) {
          selection.threadIds.push(parseInt(it.rowId!));
        } else if (it.rowType == TraceRow.ROW_TYPE_FUNC) {
          selection.funTids.push(parseInt(it.rowId!));
        } else if (it.rowType == TraceRow.ROW_TYPE_MEM) {
          selection.trackIds.push(parseInt(it.rowId!));
        } else if (it.rowType == TraceRow.ROW_TYPE_FPS) {
          selection.hasFps = true;
        } else if (it.rowType == TraceRow.ROW_TYPE_HEAP) {
          selection.nativeMemory.push(parseInt(it.rowId!));
        }
        if (it.rangeSelect && it.rangeSelect.startNS) {
          selection.leftNs = it.rangeSelect.startNS;
        }
        if (it.rangeSelect && it.rangeSelect.endNS) {
          selection.rightNs = it.rangeSelect.endNS;
        }
      });
      this.traceSheetEL?.rangeSelect(selection);
    };
    // @ts-ignore
    new ResizeObserver((entries) => {
      let width = entries[0].contentRect.width - 1 - SpRecyclerSystemTrace.scrollViewWidth;
      requestAnimationFrame(() => {
        this.timerShaftEL?.updateWidth(width);
        this.shadowRoot!.querySelectorAll<TraceRow<any>>('trace-row').forEach((it) => it.updateWidth(width));
      });
    }).observe(this);
    // @ts-ignore
    new ResizeObserver((entries) => {
      let width = this.clientWidth - 1 - SpRecyclerSystemTrace.scrollViewWidth;
      requestAnimationFrame(() => {
        this.timerShaftEL?.updateWidth(width);
        this.shadowRoot!.querySelectorAll<TraceRow<any>>('trace-row').forEach((it) => it.updateWidth(width));
      });
    }).observe(window.document.body);
  }

  getScrollWidth() {
    let noScroll,
      scroll,
      oDiv = document.createElement('div');
    oDiv.style.cssText = 'position:absolute; top:-1000px;     width:100px; height:100px; overflow:hidden;';
    noScroll = document.body.appendChild(oDiv).clientWidth;
    oDiv.style.overflowY = 'scroll';
    scroll = oDiv.clientWidth;
    document.body.removeChild(oDiv);
    return noScroll - scroll + 1;
  }

  getVisibleRows(): Array<TraceRow<any>> {
    return [...this.rowsEL!.shadowRoot!.querySelectorAll<TraceRow<any>>('trace-row')];
  }

  timerShaftELRangeChange = (e: any) => {
    this.range = e.detail;
    TraceRow.range = this.range;
    let scrollTop = this.rowsEL?.scrollTop || 0;
    let scrollHeight = this.rowsEL?.clientHeight || 0;
    //在rowsEL显示范围内的 trace-row组件将收到时间区间变化通知
    this.getVisibleRows().forEach((it) => {
      it.dataListCache.length = 0;
      this.hoverStructNull();
      it.drawObject();
    });
  };

  rowsElOnScroll = (e: any) => {
    this.hoverStructNull();
    let rows = this.getVisibleRows();
    rows.forEach((it, index) => {
      if (index == 0 || index == rows.length - 1) {
        it.dataListCache.length = 0;
        it.drawObject();
      }
    });
  };

  documentOnMouseDown = (ev: MouseEvent) => {
    this.rangeSelect.mouseDown(ev);
  };

  documentOnMouseUp = (ev: MouseEvent) => {
    this.rangeSelect.mouseUp(ev);
  };

  documentOnMouseMove = (ev: MouseEvent) => {
    let rows = this.getVisibleRows();
    this.rangeSelect.mouseMove(rows, ev);
    rows.forEach((tr) => {
      let x = ev.offsetX - (tr.canvasContainer?.offsetLeft || 0);
      let y = ev.offsetY - (tr.canvasContainer?.offsetTop || 0) + (this.rowsEL?.scrollTop || 0);
      //判断鼠标是否在当前 trace-row
      if (x > tr.frame.x && x < tr.frame.x + tr.frame.width && y > tr.frame.y && y < tr.frame.y + tr.frame.height) {
        this.hoverStructNull();
        if (tr.rowType === TraceRow.ROW_TYPE_CPU) {
          CpuStruct.hoverCpuStruct = tr.onMouseHover(x, y);
          if (CpuStruct.hoverCpuStruct) {
            tr.tip = `<span>P：${CpuStruct.hoverCpuStruct.processName || 'Process'} [${
              CpuStruct.hoverCpuStruct.processId
            }]</span><span>T：${CpuStruct.hoverCpuStruct.name} [${CpuStruct.hoverCpuStruct.tid}]</span>`;
          }
          tr.setTipLeft(x, CpuStruct.hoverCpuStruct);
        } else if (tr.rowType === TraceRow.ROW_TYPE_CPU_FREQ) {
          CpuFreqStruct.hoverCpuFreqStruct = tr.onMouseHover(x, y);
          if (CpuFreqStruct.hoverCpuFreqStruct) {
            tr.tip = `<span>${ColorUtils.formatNumberComma(CpuFreqStruct.hoverCpuFreqStruct.value!)} kHz</span>`;
          }
          tr.setTipLeft(x, CpuFreqStruct.hoverCpuFreqStruct);
        } else if (tr.rowType === TraceRow.ROW_TYPE_THREAD) {
          ThreadStruct.hoverThreadStruct = tr.onMouseHover(x, y, false);
        } else if (tr.rowType === TraceRow.ROW_TYPE_FUNC) {
          FuncStruct.hoverFuncStruct = tr.onMouseHover(x, y, false);
        } else if (tr.rowType === TraceRow.ROW_TYPE_HEAP) {
          HeapStruct.hoverHeapStruct = tr.onMouseHover(x, y, false);
          if (HeapStruct.hoverHeapStruct) {
            tr.tip = `<span>${ColorUtils.formatNumberComma(HeapStruct.hoverHeapStruct.heapsize!)} byte</span>`;
          }
          tr.setTipLeft(x, HeapStruct.hoverHeapStruct);
        } else {
          this.hoverStructNull();
        }
      } else {
        tr.onMouseLeave(x, y);
      }
      tr.drawObject();
    });
  };

  hoverStructNull() {
    CpuStruct.hoverCpuStruct = undefined;
    CpuFreqStruct.hoverCpuFreqStruct = undefined;
    ThreadStruct.hoverThreadStruct = undefined;
    FuncStruct.hoverFuncStruct = undefined;
  }

  selectStructNull() {
    CpuStruct.selectCpuStruct = undefined;
    CpuFreqStruct.selectCpuFreqStruct = undefined;
    ThreadStruct.selectThreadStruct = undefined;
    FuncStruct.selectFuncStruct = undefined;
  }

  documentOnClick = (ev: MouseEvent) => {
    if (this.rangeSelect.isDrag()) {
      return;
    }
    this.rowsEL?.querySelectorAll<TraceRow<any>>('trace-row').forEach((it) => (it.rangeSelect = undefined));
    this.selectStructNull();
    if (CpuStruct.hoverCpuStruct) {
      CpuStruct.selectCpuStruct = CpuStruct.hoverCpuStruct;
      this.traceSheetEL?.displayCpuData(CpuStruct.hoverCpuStruct);
    } else if (ThreadStruct.hoverThreadStruct) {
      ThreadStruct.selectThreadStruct = ThreadStruct.hoverThreadStruct;
      this.traceSheetEL?.displayThreadData(ThreadStruct.hoverThreadStruct);
    } else if (FuncStruct.hoverFuncStruct) {
      FuncStruct.selectFuncStruct = FuncStruct.hoverFuncStruct;
      this.traceSheetEL?.displayFuncData(['current-selection'], FuncStruct.hoverFuncStruct);
    } else if (SportRuler.rulerFlagObj) {
    } else {
      this.traceSheetEL?.setAttribute('mode', 'hidden');
    }
    this.documentOnMouseMove(ev);
  };

  connectedCallback() {
    /**
     * 监听时间轴区间变化
     */
    this.timerShaftEL?.addEventListener('range-change', this.timerShaftELRangeChange);
    /**
     * 监听rowsEL的滚动时间，刷新可见区域的trace-row组件的时间区间（将触发trace-row组件重绘）
     */
    this.rowsEL?.addEventListener('scroll', this.rowsElOnScroll);
    /**
     * 监听document的mousemove事件 坐标通过换算后找到当前鼠标所在的trace-row组件，将坐标传入
     */
    document.addEventListener('mousemove', this.documentOnMouseMove);
    document.addEventListener('mousedown', this.documentOnMouseDown);
    document.addEventListener('mouseup', this.documentOnMouseUp);
    document.addEventListener('click', this.documentOnClick);
  }

  disconnectedCallback() {
    this.timerShaftEL?.removeEventListener('range-change', this.timerShaftELRangeChange);
    this.rowsEL?.removeEventListener('scroll', this.rowsElOnScroll);
    document.removeEventListener('mousemove', this.documentOnMouseMove);
    document.removeEventListener('click', this.documentOnClick);
  }

  loadDatabaseUrl(url: string, complete?: Function) {
    this.init({ url: url }).then(() => {
      let scrollTop = this.rowsEL?.scrollTop || 0;
      let scrollHeight = this.rowsEL?.clientHeight || 0;
      this.rowsEL?.querySelectorAll('trace-row').forEach((rowItem: any) => {
        let top = rowItem.offsetTop - (this.rowsEL?.offsetTop || 0);
        if (top + rowItem.clientHeight > scrollTop && top + rowItem.clientHeight < scrollTop + scrollHeight + rowItem.clientHeight) {
          (rowItem as TraceRow<any>).dataListCache.length = 0;
        }
      });
      if (complete) {
        complete();
      }
    });
  }

  loadDatabaseArrayBuffer(buf: ArrayBuffer, complete?: Function) {
    this.init({ buf }).then(() => {
      let scrollTop = this.rowsEL?.scrollTop || 0;
      let scrollHeight = this.rowsEL?.clientHeight || 0;
      this.rowsEL?.querySelectorAll('trace-row').forEach((item: any) => {
        let top = item.offsetTop - (this.rowsEL?.offsetTop || 0);
        if (top + item.clientHeight > scrollTop && top + item.clientHeight < scrollTop + scrollHeight + item.clientHeight) {
          (item as TraceRow<any>).dataListCache.length = 0;
        }
      });
      if (complete) {
        complete();
      }
    });
  }

  init = async (param: { buf?: ArrayBuffer; url?: string }) => {
    if (this.rowsEL) this.rowsEL.innerHTML = '';
    this.traceSheetEL?.setAttribute('mode', 'hidden');
    this.timerShaftEL?.reset();
    procedurePool.clearCache();
    param.buf && (await threadPool.initSqlite(param.buf));
    param.url && (await threadPool.initServer(param.url));
    this.processThreads = await queryProcessThreads();
    this.processMem = await queryProcessMem();
    this.processAsyncEvent = await getAsyncEvents();
    await this.initTotalTime();
    let cpuObjs = await this.initCpu();
    await this.initCpuRate();
    let freqObjs = await this.initCpuFreq();
    let fpsObjs = await this.initFPS();
    let processObjs = await this.initProcess();
    this.rowsEL.dataSource = [...cpuObjs, ...freqObjs, ...fpsObjs, ...processObjs];
    this.getVisibleRows().forEach((it) => it.drawObject());
  };
  initCpuRate = async () => {
    let rates = await getCpuUtilizationRate(0, this.timerShaftEL?.totalNS || 0);
    if (this.timerShaftEL) this.timerShaftEL.cpuUsage = rates;
  };
  initTotalTime = async () => {
    let res = await queryTotalTime();
    if (this.timerShaftEL) {
      this.timerShaftEL.totalNS = res[0].total;
      this.timerShaftEL.loadComplete = true;
    }
  };
  initCpu = async () => {
    let objs = [];
    let array = await queryCpuMax();
    if (array && array.length > 0 && array[0]) {
      let cpuMax = array[0].cpu;
      CpuStruct.cpuCount = cpuMax + 1;
      for (let i1 = 0; i1 < CpuStruct.cpuCount; i1++) {
        const cpuId = i1;
        let cpuTraceRow = new TraceRowObject();
        cpuTraceRow.rowId = `${cpuId}`;
        cpuTraceRow.rowType = TraceRow.ROW_TYPE_CPU;
        cpuTraceRow.rowParentId = '';
        cpuTraceRow.rowHeight = 40;
        cpuTraceRow.frame = new Rect(0, 0, this.rowsEL.clientWidth - 248, cpuTraceRow.rowHeight);
        cpuTraceRow.name = `Cpu ${cpuId}`;
        cpuTraceRow.supplier = () => queryCpuData(cpuId, 0, this.timerShaftEL?.totalNS || 0);
        cpuTraceRow.onThreadHandler = (row, ctx) => {
          procedurePool.submitWithName(
            'cpu',
            `cpu${cpuId}`,
            {
              list: cpuTraceRow.must ? cpuTraceRow.dataList : undefined,
              startNS: TraceRow.range?.startNS || 0,
              endNS: TraceRow.range?.endNS || 0,
              totalNS: TraceRow.range?.totalNS || 0,
              frame: cpuTraceRow.frame,
            },
            (res: any) => {
              cpuTraceRow.dataListCache = res;
              cpuTraceRow.must = false;
              row.clearCanvas();
              row.c!.beginPath();
              row.drawLines();
              for (let i = 0; i < res.length; i++) {
                CpuStruct.draw(ctx, res[i]);
              }
              row.drawSelection();
              row.c!.closePath();
            }
          );
        };
        objs.push(cpuTraceRow);
      }
    }
    return objs;
  };

  initCpuFreq = async () => {
    let objs = [];
    let freqList = await queryCpuFreq();
    let freqMaxList = await queryCpuMaxFreq();
    CpuFreqStruct.maxFreq = freqMaxList[0].maxFreq;
    let math = () => {
      let units: Array<string> = ['', 'K', 'M', 'G', 'T', 'E'];
      let sb = ' ';
      CpuFreqStruct.maxFreqName = ' ';
      if (CpuFreqStruct.maxFreq > 0) {
        let log10: number = Math.ceil(Math.log10(CpuFreqStruct.maxFreq));
        let pow10: number = Math.pow(10, log10);
        let afterCeil: number = Math.ceil(CpuFreqStruct.maxFreq / (pow10 / 4)) * (pow10 / 4);
        CpuFreqStruct.maxFreq = afterCeil;
        let unitIndex: number = Math.floor(log10 / 3);
        sb = `${afterCeil / Math.pow(10, unitIndex * 3)}${units[unitIndex + 1]}hz`;
      }
      CpuFreqStruct.maxFreqName = sb.toString();
    };
    math();
    for (let i = 0; i < freqList.length; i++) {
      const it = freqList[i];
      let cpuFreqTraceRow = new TraceRowObject();
      cpuFreqTraceRow.rowId = `${it.cpu}`;
      cpuFreqTraceRow.rowType = TraceRow.ROW_TYPE_CPU_FREQ;
      cpuFreqTraceRow.rowParentId = '';
      cpuFreqTraceRow.rowHeight = 40;
      cpuFreqTraceRow.frame = new Rect(0, 0, this.rowsEL.clientWidth - 248, cpuFreqTraceRow.rowHeight);
      cpuFreqTraceRow.name = `Cpu ${it.cpu} Frequency`;
      cpuFreqTraceRow.supplier = () => queryCpuFreqData(it.cpu);
      cpuFreqTraceRow.onThreadHandler = (row, ctx) => {
        procedurePool.submitWithName(
          'freq',
          `freq${it.cpu}`,
          {
            list: cpuFreqTraceRow.must ? cpuFreqTraceRow.dataList : undefined,
            startNS: TraceRow.range?.startNS || 0,
            endNS: TraceRow.range?.endNS || 0,
            totalNS: TraceRow.range?.totalNS || 0,
            frame: cpuFreqTraceRow.frame,
          },
          (res: any) => {
            cpuFreqTraceRow.dataListCache = res;
            cpuFreqTraceRow.must = false;
            row.clearCanvas();
            row.drawLines();
            row.c!.beginPath();
            for (let i = 0; i < res.length; i++) {
              CpuFreqStruct.draw(ctx, res[i]);
            }
            row.drawSelection();
            row.c!.closePath();
            let s = CpuFreqStruct.maxFreqName;
            let textMetrics = ctx.measureText(s);
            row.c!.globalAlpha = 0.8;
            row.c!.fillStyle = '#f0f0f0';
            row.c!.fillRect(0, 5, textMetrics.width + 8, 18);
            row.c!.globalAlpha = 1;
            row.c!.fillStyle = '#333';
            ctx.textBaseline = 'middle';
            ctx.fillText(maxFps, 4, 5 + 9);
          }
        );
      };
      objs.push(cpuFreqTraceRow);
    }
    return objs;
  };

  initFPS = async () => {
    let objs = [];
    let fpsRow = new TraceRowObject();
    fpsRow.rowId = `fps`;
    fpsRow.rowType = TraceRow.ROW_TYPE_FPS;
    fpsRow.rowParentId = '';
    FpsStruct.maxFps = 0;
    fpsRow.rowHeight = 40;
    fpsRow.frame = new Rect(0, 0, this.rowsEL.clientWidth - 248, fpsRow.rowHeight);
    fpsRow.name = 'FPS';
    fpsRow.supplier = () => getFps();
    fpsRow.onDrawHandler = (row, ctx) => {
      if (fpsRow.dataListCache.length > 0) {
        for (let i = 0; i < fpsRow.dataListCache.length; i++) {
          FpsStruct.draw(ctx, fpsRow.dataListCache[i]);
        }
      } else {
        if (fpsRow.dataList) {
          for (let i = 0; i < fpsRow.dataList.length; i++) {
            let it = fpsRow.dataList[i];
            if ((it.fps || 0) > FpsStruct.maxFps) {
              FpsStruct.maxFps = it.fps || 0;
            }
            if (i === fpsRow.dataList.length - 1) {
              it.dur = (TraceRow.range?.endNS || 0) - (it.startNS || 0);
            } else {
              it.dur = (fpsRow.dataList[i + 1].startNS || 0) - (it.startNS || 0);
            }
            if (
              (it.startNS || 0) + (it.dur || 0) > (TraceRow.range?.startNS || 0) &&
              (it.startNS || 0) < (TraceRow.range?.endNS || 0)
            ) {
              FpsStruct.setFrame(
                fpsRow.dataList[i],
                5,
                TraceRow.range?.startNS || 0,
                TraceRow.range?.endNS || 0,
                TraceRow.range?.totalNS || 0,
                fpsRow.frame
              );
              if (
                i > 0 &&
                (fpsRow.dataList[i - 1].frame?.x || 0) == (fpsRow.dataList[i].frame?.x || 0) &&
                (fpsRow.dataList[i - 1].frame?.width || 0) == (fpsRow.dataList[i].frame?.width || 0)
              ) {
              } else {
                fpsRow.dataListCache.push(fpsRow.dataList[i]);
                FpsStruct.draw(ctx, fpsRow.dataList[i]);
              }
            }
          }
        }
      }
      if (ctx) {
        let maxFps = FpsStruct.maxFps + 'FPS';
        let textMetrics = ctx.measureText(maxFps);
        ctx.globalAlpha = 0.8;
        ctx.fillStyle = '#f0f0f0';
        ctx.fillRect(0, 5, textMetrics.width + 8, 18);
        ctx.globalAlpha = 1;
        ctx.fillStyle = '#333';
        ctx.textBaseline = 'middle';
        ctx.fillText(maxFps, 4, 5 + 9);
      }
    };
    objs.push(fpsRow);
    return objs;
  };

  /**
   * 添加进程信息
   */
  initProcess = async () => {
    let objs = [];
    let processList = await queryProcess();
    for (let i = 0; i < processList.length; i++) {
      const it = processList[i];
      let processRow = new TraceRowObject<ProcessStruct>();
      processRow.rowId = `${it.pid}`;
      processRow.rowType = TraceRow.ROW_TYPE_PROCESS;
      processRow.rowParentId = '';
      processRow.frame = new Rect(0, 0, this.rowsEL.clientWidth - 248, processRow.rowHeight);
      processRow.folder = true;
      processRow.name = `${it.processName || 'Process'} ${it.pid}`;
      processRow.supplier = () => queryProcessData(it.pid || -1, 0, TraceRow.range?.totalNS || 0);
      processRow.onThreadHandler = (row, ctx) => {
        procedurePool.submitWithName(
          'process',
          `process ${it.pid} ${it.processName}`,
          {
            list: processRow.must ? processRow.dataList : undefined,
            startNS: TraceRow.range?.startNS || 0,
            endNS: TraceRow.range?.endNS || 0,
            totalNS: TraceRow.range?.totalNS || 0,
            frame: processRow.frame,
          },
          (res: any) => {
            processRow.dataListCache = res;
            processRow.must = false;
            row.clearCanvas();
            row.drawLines();
            row.c!.beginPath();
            for (let i = 0; i < res.length; i++) {
              ProcessStruct.draw(ctx, res[i]);
            }
            row.drawSelection();
            row.c!.closePath();
          }
        );
      };
      objs.push(processRow);

      /**
       * 添加进程内存信息
       */
      let processMem = this.processMem.filter((mem) => mem.pid === it.pid);
      processMem.forEach((mem) => {
        let row = new TraceRowObject<ProcessMemStruct>();
        row.rowId = `${mem.trackId}`;
        row.rowType = TraceRow.ROW_TYPE_MEM;
        row.rowParentId = `${it.pid}`;
        row.rowHidden = !processRow.expansion;
        row.rowHeight = 40;
        row.frame = new Rect(0, 0, this.rowsEL.clientWidth - 248, row.rowHeight);
        row.name = `${mem.trackName}`;
        row.children = true;
        row.supplier = () =>
          queryProcessMemData(mem.trackId).then((res) => {
            let maxValue = Math.max(...res.map((it) => it.value || 0));
            for (let j = 0; j < res.length; j++) {
              res[j].maxValue = maxValue;
              if (j == res.length - 1) {
                res[j].duration = this.range?.totalNS || 0;
              } else {
                res[j].duration = (res[j + 1].startTime || 0) - (res[j].startTime || 0);
              }
              if (j > 0) {
                res[j].delta = (res[j].value || 0) - (res[j - 1].value || 0);
              } else {
                res[j].delta = 0;
              }
            }
            return res;
          });
        row.onThreadHandler = (r, ctx) => {
          procedurePool.submitWithName(
            'mem',
            `mem ${mem.trackId} ${mem.trackName}`,
            {
              list: row.must ? row.dataList : undefined,
              startNS: TraceRow.range?.startNS || 0,
              endNS: TraceRow.range?.endNS || 0,
              totalNS: TraceRow.range?.totalNS || 0,
              frame: row.frame,
            },
            (res: any) => {
              row.dataListCache = res;
              row.must = false;
              r.clearCanvas();
              r.drawLines();
              r.c!.beginPath();
              for (let i = 0; i < res.length; i++) {
                ProcessMemStruct.draw(ctx, res[i]);
              }
              r.drawSelection();
              r.c!.closePath();
            }
          );
        };
        objs.push(row);
      });
      /**
       * 添加进程线程信息
       */
      let threads = this.processThreads.filter(
        (thread) => thread.pid === it.pid && thread.tid != 0 && thread.threadName != null
      );
      threads.forEach((thread, i) => {
        let threadRow = new TraceRowObject<ThreadStruct>();
        threadRow.rowId = `${thread.tid}`;
        threadRow.rowType = TraceRow.ROW_TYPE_THREAD;
        threadRow.rowParentId = `${it.pid}`;
        threadRow.rowHidden = !processRow.expansion;
        threadRow.rowHeight = 40;
        threadRow.frame = new Rect(0, 0, this.rowsEL.clientWidth - 248, threadRow.rowHeight);
        threadRow.name = `${thread.threadName} ${thread.tid}`;
        threadRow.children = true;
        threadRow.supplier = () =>
          queryThreadData(thread.tid || 0).then((res) => {
            getFunDataByTid(thread.tid || 0).then((funs) => {
              if (funs.length > 0) {
                let maxHeight = (Math.max(...funs.map((it) => it.depth || 0)) + 1) * 20 + 20;
                let funcRow = new TraceRowObject<FuncStruct>();
                funcRow.rowId = `${thread.tid}`;
                funcRow.rowType = TraceRow.ROW_TYPE_FUNC;
                funcRow.rowParentId = `${it.pid}`;
                funcRow.rowHidden = !processRow.expansion;
                funcRow.rowHeight = maxHeight;
                funcRow.frame = new Rect(0, 0, this.rowsEL.clientWidth - 248, funcRow.rowHeight);
                funcRow.name = `${thread.threadName} ${thread.tid}`;
                funcRow.children = true;
                funcRow.supplier = () => new Promise((resolve, reject) => resolve(funs));
                funcRow.onThreadHandler = (r, ctx) => {
                  procedurePool.submitWithName(
                    'func',
                    `func ${thread.tid} ${thread.threadName}`,
                    {
                      list: funcRow.must ? funcRow.dataList : undefined,
                      startNS: TraceRow.range?.startNS || 0,
                      endNS: TraceRow.range?.endNS || 0,
                      totalNS: TraceRow.range?.totalNS || 0,
                      frame: threadRow.frame,
                    },
                    (res: any) => {
                      funcRow.must = false;
                      funcRow.dataListCache = res;
                      r.clearCanvas();
                      r.drawLines();
                      r.c!.beginPath();
                      for (let i = 0; i < res.length; i++) {
                        FuncStruct.draw(ctx, res[i]);
                      }
                      r.drawSelection();
                      r.c!.closePath();
                    }
                  );
                };
              }
            });
            return res;
          });
        threadRow.onThreadHandler = (r, ctx) => {
          procedurePool.submitWithName(
            'thread',
            `thread ${thread.tid} ${thread.threadName}`,
            {
              list: threadRow.must ? threadRow.dataList : undefined,
              startNS: TraceRow.range?.startNS || 0,
              endNS: TraceRow.range?.endNS || 0,
              totalNS: TraceRow.range?.totalNS || 0,
              frame: threadRow.frame,
            },
            (res: any) => {
              threadRow.dataListCache = res;
              threadRow.must = false;
              r.clearCanvas();
              r.drawLines();
              r.c!.beginPath();
              for (let i = 0; i < res.length; i++) {
                ThreadStruct.draw(ctx, res[i]);
              }
              r.drawSelection();
              r.c!.closePath();
            }
          );
        };
        objs.push(threadRow);
      });
    }
    return objs;
  };

  insertAfter(newEl: HTMLElement, targetEl: HTMLElement) {
    let parentEl = targetEl.parentNode;
    if (parentEl!.lastChild == targetEl) {
      parentEl!.appendChild(newEl);
    } else {
      parentEl!.insertBefore(newEl, targetEl.nextSibling);
    }
  }

  initHtml(): string {
    return `
        <style>
        :host{
            display: block;
            width: 100%;
            height: 100%;
        }
        .recycle-timer-shaft{
            width: 100%;
            z-index: 2;
        }
        .rows{
            display: flex;
            box-sizing: border-box;
            flex-direction: column;
            overflow-y: auto;
            max-height: calc(100vh - 150px - 48px);
            flex: 1;
            width: 100%;
        }
        .container{
            width: 100%;
            box-sizing: border-box;
            height: 100%;
            display: grid;
            grid-template-columns: 1fr;
            grid-template-rows: min-content 1fr min-content;
        }

        </style>
        <div class="container">
            <timer-shaft-element class="recycle-timer-shaft">
            </timer-shaft-element>
            <trace-row-recycler-view class="rows">
            </trace-row-recycler-view>
            <trace-sheet class="recycle-trace-sheet" mode="hidden">
            </trace-sheet>
        </div>
        `;
  }
}
