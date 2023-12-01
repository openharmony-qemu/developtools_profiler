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
import {
  queryBySelectAllocationOrReturn,
  queryBySelectExecute,
  queryEbpfSamplesCount,
  querySceneSearchFunc,
  querySearchFunc,
  threadPool,
} from '../database/SqlLite.js';
import { RangeSelectStruct, TraceRow } from './trace/base/TraceRow.js';
import { TimerShaftElement } from './trace/TimerShaftElement.js';
import './trace/base/TraceSheet.js';
import { TraceSheet } from './trace/base/TraceSheet.js';
import { RangeSelect } from './trace/base/RangeSelect.js';
import { SelectionParam } from '../bean/BoxSelection.js';
import { procedurePool } from '../database/Procedure.js';
import { SpApplication } from '../SpApplication.js';
import { Flag } from './trace/timer-shaft/Flag.js';
import { SlicesTime, SportRuler } from './trace/timer-shaft/SportRuler.js';
import { SpHiPerf } from './chart/SpHiPerf.js';
import { SearchSdkBean, SearchThreadProcessBean } from '../bean/SearchFuncBean.js';
import { error, info } from '../../log/Log.js';
import {
  drawFlagLineSegment,
  drawLines,
  drawLinkLines,
  drawLogsLineSegment,
  drawWakeUp,
  drawWakeUpList,
  LineType,
  ns2x,
  ns2xByTimeShaft,
  PairPoint,
  Rect,
} from '../database/ui-worker/ProcedureWorkerCommon.js';
import { SpChartManager } from './chart/SpChartManager.js';
import { CpuStruct, WakeupBean } from '../database/ui-worker/ProcedureWorkerCPU.js';
import { ProcessStruct } from '../database/ui-worker/ProcedureWorkerProcess.js';
import { CpuFreqStruct } from '../database/ui-worker/ProcedureWorkerFreq.js';
import { CpuFreqLimitsStruct } from '../database/ui-worker/ProcedureWorkerCpuFreqLimits.js';
import { ThreadStruct } from '../database/ui-worker/ProcedureWorkerThread.js';
import { func, FuncStruct } from '../database/ui-worker/ProcedureWorkerFunc.js';
import { CpuStateStruct } from '../database/ui-worker/ProcedureWorkerCpuState.js';
import { HiPerfCpuStruct } from '../database/ui-worker/ProcedureWorkerHiPerfCPU.js';
import { HiPerfProcessStruct } from '../database/ui-worker/ProcedureWorkerHiPerfProcess.js';
import { HiPerfThreadStruct } from '../database/ui-worker/ProcedureWorkerHiPerfThread.js';
import { HiPerfEventStruct } from '../database/ui-worker/ProcedureWorkerHiPerfEvent.js';
import { HiPerfReportStruct } from '../database/ui-worker/ProcedureWorkerHiPerfReport.js';
import { FpsStruct } from '../database/ui-worker/ProcedureWorkerFPS.js';
import { CpuAbilityMonitorStruct } from '../database/ui-worker/ProcedureWorkerCpuAbility.js';
import { DiskAbilityMonitorStruct } from '../database/ui-worker/ProcedureWorkerDiskIoAbility.js';
import { MemoryAbilityMonitorStruct } from '../database/ui-worker/ProcedureWorkerMemoryAbility.js';
import { NetworkAbilityMonitorStruct } from '../database/ui-worker/ProcedureWorkerNetworkAbility.js';
import { ClockStruct } from '../database/ui-worker/ProcedureWorkerClock.js';
import { Utils } from './trace/base/Utils.js';
import { IrqStruct } from '../database/ui-worker/ProcedureWorkerIrq.js';
import { JanksStruct } from '../bean/JanksStruct.js';
import { JankStruct } from '../database/ui-worker/ProcedureWorkerJank.js';
import { TabPaneCurrent } from './trace/sheet/TabPaneCurrent.js';
import { HeapStruct } from '../database/ui-worker/ProcedureWorkerHeap.js';
import { SpStatisticsHttpUtil } from '../../statistics/util/SpStatisticsHttpUtil.js';
import { HeapSnapshotStruct } from '../database/ui-worker/ProcedureWorkerHeapSnapshot.js';
import { HeapDataInterface } from '../../js-heap/HeapDataInterface.js';
import { LitTabs } from '../../base-ui/tabs/lit-tabs.js';
import { TraceRowConfig } from './trace/base/TraceRowConfig.js';
import { TabPaneCurrentSelection } from './trace/sheet/TabPaneCurrentSelection.js';
import { SpChartList } from './trace/SpChartList.js';
import './trace/SpChartList.js';
import { AppStartupStruct } from '../database/ui-worker/ProcedureWorkerAppStartup.js';
import { SoStruct } from '../database/ui-worker/ProcedureWorkerSoInit.js';
import { TabPaneTaskFrames } from './trace/sheet/task/TabPaneTaskFrames.js';
import { FlagsConfig } from './SpFlags.js';
import { FrameDynamicStruct } from '../database/ui-worker/ProcedureWorkerFrameDynamic.js';
import { FrameAnimationStruct } from '../database/ui-worker/ProcedureWorkerFrameAnimation.js';
import { FrameSpacingStruct } from '../database/ui-worker/ProcedureWorkerFrameSpacing.js';
import { JsCpuProfilerStruct } from '../database/ui-worker/ProcedureWorkerCpuProfiler.js';
import { TabPaneSummary } from './trace/sheet/ark-ts/TabPaneSummary.js';
import { JsCpuProfilerChartFrame } from '../bean/JsStruct.js';
import { FileInfo } from '../../js-heap/model/UiStruct.js';
import { SnapshotStruct } from '../database/ui-worker/ProcedureWorkerSnapshot.js';
import { setSelectState, intersectData, isExistPidInArray } from './Utils.js';
import { LogStruct } from '../database/ui-worker/ProcedureWorkerLog.js';
import { TabPaneFrequencySample } from './trace/sheet/cpu/TabPaneFrequencySample.js';
import { TabPaneCounterSample } from './trace/sheet/cpu/TabPaneCounterSample.js';
import { LitSearch } from './trace/search/Search.js';
import { TabPaneFlag } from './trace/timer-shaft/TabPaneFlag.js';
import { LitTabpane } from '../../base-ui/tabs/lit-tabpane.js';
import { HiPerfCallChartStruct } from '../database/ui-worker/ProcedureWorkerHiPerfCallChart.js';
import { type HiSysEventStruct } from '../database/ui-worker/ProcedureWorkerHiSysEvent.js';
import { InitAnalysis } from '../database/logic-worker/ProcedureLogicWorkerCommon.js';
import { type SpKeyboard } from '../component/SpKeyboard.js';

function dpr() {
  return window.devicePixelRatio || 1;
}
//节流处理
function throttle(fn: any, t: number, ev: any): any {
  let timer: any = null;
  return function () {
    if (!timer) {
      timer = setTimeout(function () {
        if (ev) {
          fn(ev);
        } else {
          fn();
        }
        timer = null;
      }, t);
    }
  };
}

export class CurrentSlicesTime {
  startTime: number | undefined;
  endTime: number | undefined;
}

@element('sp-system-trace')
export class SpSystemTrace extends BaseElement {
  mouseCurrentPosition = 0;
  offsetMouse = 0;
  isMouseLeftDown = false;
  static scrollViewWidth = 0;
  static isCanvasOffScreen = true;
  static DATA_DICT: Map<number, string> = new Map<number, string>();
  static DATA_TASK_POOL_CALLSTACK: Map<number, { id: number; ts: number; dur: number; name: string }> = new Map<
    number,
    { id: number; ts: number; dur: number; name: string }
  >();
  static SDK_CONFIG_MAP: any;
  static sliceRangeMark: any;
  static wakeupList: Array<WakeupBean> = [];
  currentSlicesTime: CurrentSlicesTime = new CurrentSlicesTime();
  intersectionObserver: IntersectionObserver | undefined;
  tipEL: HTMLDivElement | undefined | null;
  rowsEL: HTMLDivElement | undefined | null;
  rowsPaneEL: HTMLDivElement | undefined | null;
  stateRowsId: Array<object> = [];
  spacerEL: HTMLDivElement | undefined | null;
  visibleRows: Array<TraceRow<any>> = [];
  collectRows: Array<TraceRow<any>> = [];
  currentRow: TraceRow<any> | undefined | null;
  keyboardEnable = true;
  mouseEventEnable = true;
  currentRowType = ''; /*保存当前鼠标所在行的类型*/
  observerScrollHeightEnable: boolean = false;
  observerScrollHeightCallback: Function | undefined;
  favoriteChartListEL: SpChartList | undefined | null;
  // @ts-ignore
  observer = new ResizeObserver((entries) => {
    if (this.observerScrollHeightEnable && this.observerScrollHeightCallback) {
      this.observerScrollHeightCallback();
    }
  });
  static btnTimer: any = null;
  isMousePointInSheet = false;
  hoverFlag: Flag | undefined | null = undefined;
  selectFlag: Flag | undefined | null;
  slicestime: SlicesTime | undefined | null = null;
  public timerShaftEL: TimerShaftElement | null | undefined;
  private traceSheetEL: TraceSheet | undefined | null;
  public rangeSelect!: RangeSelect;
  chartManager: SpChartManager | undefined | null;
  private loadTraceCompleted: boolean = false;
  private rangeTraceRow: Array<TraceRow<any>> | undefined = [];
  canvasFavoritePanelCtx: CanvasRenderingContext2D | null | undefined;
  canvasPanel: HTMLCanvasElement | null | undefined; //绘制取消收藏后泳道图
  canvasPanelCtx: CanvasRenderingContext2D | undefined | null;
  linkNodes: PairPoint[][] = [];
  public currentClickRow: HTMLDivElement | undefined | null;
  private litTabs: LitTabs | undefined | null;
  eventMap: any = {};
  private isSelectClick: boolean = false;
  private selectionParam: SelectionParam | undefined;
  private snapshotFiles: FileInfo | null | undefined;
  private tabCpuFreq: TabPaneFrequencySample | undefined | null;
  private tabCpuState: TabPaneCounterSample | undefined | null;
  private collapseAll: boolean = false;
  private currentCollectGroup: string = '1';
  private _list: Array<SlicesTime> = [];
  private expandRowList: Array<TraceRow<any>> = [];
  private _slicesList: Array<SlicesTime> = [];
  private _flagList: Array<any> = [];

  set snapshotFile(data: FileInfo) {
    this.snapshotFiles = data;
  }

  set slicesList(list: Array<SlicesTime>) {
    this._slicesList = list;
  }

  set flagList(list: Array<any>) {
    this._flagList = list;
  }

  addPointPair(startPoint: PairPoint, endPoint: PairPoint) {
    if (startPoint.rowEL.collect) {
      startPoint.rowEL.translateY = startPoint.rowEL.getBoundingClientRect().top - 195;
    } else {
      startPoint.rowEL.translateY = startPoint.rowEL.offsetTop - this.rowsPaneEL!.scrollTop;
    }
    if (endPoint.rowEL.collect) {
      endPoint.rowEL.translateY = endPoint.rowEL.getBoundingClientRect().top - 195;
    } else {
      endPoint.rowEL.translateY = endPoint.rowEL.offsetTop - this.rowsPaneEL!.scrollTop;
    }
    startPoint.y = startPoint.rowEL!.translateY! + startPoint.offsetY;
    endPoint.y = endPoint.rowEL!.translateY! + endPoint.offsetY;
    this.linkNodes.push([startPoint, endPoint]);
  }

  clearPointPair() {
    this.linkNodes.length = 0;
  }

  removeLinkLinesByBusinessType(...businessTypes: string[]) {
    this.linkNodes = this.linkNodes.filter((pointPair) => {
      return !(businessTypes.indexOf(pointPair[0].business) > -1);
    });
  }

  hiddenLinkLinesByBusinessType(...businessTypes: string[]) {
    this.linkNodes.map((value) => {
      if (businessTypes.indexOf(value[0].business) !== -1) {
        value[0].hidden = true;
        value[1].hidden = true;
      }
    });
  }

  showLinkLinesByBusinessType(...businessTypes: string[]) {
    this.linkNodes.map((value) => {
      if (businessTypes.indexOf(value[0].business) !== -1) {
        value[0].hidden = false;
        value[1].hidden = false;
      }
    });
  }

  // 清除上一次点击调用栈产生的三角旗子
  private clearTriangle(flagList: Array<Flag>) {
    this.timerShaftEL!.sportRuler!.times = [];
    for (let i = 0; i < flagList.length; i++) {
      if (flagList[i].type === 'triangle') {
        flagList.splice(i, 1);
        i--;
      }
    }
  }

  pushPidToSelection(selection: SelectionParam, id: string) {
    let pid = parseInt(id);
    if (!selection.processIds.includes(pid)) {
      selection.processIds.push(pid);
    }
  }

  getCollectRows(condition: (row: TraceRow<any>) => boolean): Array<TraceRow<any>> {
    return this.favoriteChartListEL!.getCollectRows(condition);
  }

  private createPointEvent(it: TraceRow<any>) {
    let event = this.eventMap[it.rowType + ''];
    if (event) {
      return event;
    } else {
      if (it.rowType === TraceRow.ROW_TYPE_HEAP) {
        event = it.name;
      } else if (it.rowType === TraceRow.ROW_TYPE_HIPERF_CPU) {
        event = 'HiPerf Cpu';
        if (it.rowId === 'HiPerf-cpu-merge') {
          event = 'HiPerf';
        }
      } else if (it.rowType === TraceRow.ROW_TYPE_FILE_SYSTEM) {
        if (it.rowId === 'FileSystemLogicalWrite') {
          event = 'FileSystem Logical Write';
        } else if (it.rowId === 'FileSystemLogicalRead') {
          event = 'FileSystem Logical Read';
        } else if (it.rowId === 'FileSystemVirtualMemory') {
          event = 'Page Fault Trace';
        } else if (it.rowId!.startsWith('FileSystemDiskIOLatency')) {
          event = 'Disk I/O Latency';
          if (it.rowId!.startsWith('FileSystemDiskIOLatency-')) {
            event = 'Bio Process';
          }
        }
      } else if (it.rowType === TraceRow.ROW_TYPE_STATE_ENERGY) {
        event = it.name;
      } else if (it.rowType === TraceRow.ROW_TYPE_VM_TRACKER) {
        if (it.rowParentId === '') {
          event = 'VM Tracker';
        } else {
          event = it.name;
        }
      } else if (it.rowType === TraceRow.ROW_TYPE_JANK) {
        if (it.rowId === 'frameTime' || it.rowParentId === 'frameTime') {
          event = 'FrameTimeLine';
        } else if (it.hasAttribute('frame_type')) {
          event = it.getAttribute('frame_type') + '';
        }
      } else if (it.rowType === TraceRow.ROW_TYPE_DELIVER_INPUT_EVENT) {
        event = 'DeliverInputEvent';
        if (it.rowParentId === TraceRow.ROW_TYPE_DELIVER_INPUT_EVENT) {
          event = 'DeliverInputEvent Func';
        }
      } else {
        event = it.name;
      }
      return event;
    }
  }

  refreshFavoriteCanvas() {
    this.favoriteChartListEL!.refreshFavoriteCanvas();
  }

  expansionAllParentRow(currentRow: TraceRow<any>) {
    let parentRow = this.rowsEL!.querySelector<TraceRow<any>>(
      `trace-row[row-id='${currentRow.rowParentId}'][folder][scene]`
    );
    if (parentRow) {
      parentRow.expansion = true;
      if (this.rowsEL!.querySelector<TraceRow<any>>(`trace-row[row-id='${parentRow.rowParentId}'][folder]`)) {
        this.expansionAllParentRow(parentRow);
      }
    }
  }

  canvasPanelConfig() {
    this.canvasPanel!.style.left = `${this.timerShaftEL!.canvas!.offsetLeft!}px`;
    this.canvasPanel!.width = this.canvasPanel!.offsetWidth * dpr();
    this.canvasPanel!.height = this.canvasPanel!.offsetHeight * dpr();
    this.canvasPanelCtx!.scale(dpr(), dpr());
  }

  getScrollWidth() {
    let totalScrollDiv,
      scrollDiv,
      overflowDiv = document.createElement('div');
    overflowDiv.style.cssText = 'position:absolute; top:-2000px;width:200px; height:200px; overflow:hidden;';
    totalScrollDiv = document.body.appendChild(overflowDiv).clientWidth;
    overflowDiv.style.overflowY = 'scroll';
    scrollDiv = overflowDiv.clientWidth;
    document.body.removeChild(overflowDiv);
    return totalScrollDiv - scrollDiv;
  }

  getShowTab(): Array<string> {
    let tabpane = this.traceSheetEL!.shadowRoot!.querySelectorAll('lit-tabpane') as NodeListOf<LitTabpane>;
    let showTab: Array<string> = [];
    for (let pane of tabpane) {
      if (pane.getAttribute('hidden') === 'false') {
        showTab.push(pane.getAttribute('id') || '');
      }
    }
    return showTab;
  }

  timerShaftELFlagClickHandler = (flag: Flag | undefined | null) => {
    if (flag) {
      setTimeout(() => {
        if (TraceRow.rangeSelectObject) {
          let showTab = this.getShowTab();
          this.traceSheetEL?.displayTab<TabPaneFlag>('box-flag', ...showTab).setCurrentFlag(flag);
        } else {
          this.traceSheetEL?.displayTab<TabPaneFlag>('box-flag').setCurrentFlag(flag);
        }
      }, 100);
    }
  };

  timerShaftELFlagChange = (hoverFlag: Flag | undefined | null, selectFlag: Flag | undefined | null) => {
    this.hoverFlag = hoverFlag;
    this.selectFlag = selectFlag;
    this.refreshCanvas(true, 'flagChange');
  };

  timerShaftELRangeClick = (sliceTime: SlicesTime | undefined | null) => {
    if (sliceTime) {
      setTimeout(() => {
        if (TraceRow.rangeSelectObject) {
          let showTab = this.getShowTab();
          this.traceSheetEL?.displayTab<TabPaneCurrent>('tabpane-current', ...showTab).setCurrentSlicesTime(sliceTime);
        } else {
          this.traceSheetEL?.displayTab<TabPaneCurrent>('tabpane-current').setCurrentSlicesTime(sliceTime);
        }
      }, 0);
    }
  };

  timerShaftELRangeChange = (e: any) => {
    TraceRow.range = e;
    if (TraceRow.rangeSelectObject) {
      TraceRow.rangeSelectObject!.startX = Math.floor(
        ns2x(
          TraceRow.rangeSelectObject!.startNS!,
          TraceRow.range?.startNS!,
          TraceRow.range?.endNS!,
          TraceRow.range?.totalNS!,
          this.timerShaftEL!.sportRuler!.frame
        )
      );
      TraceRow.rangeSelectObject!.endX = Math.floor(
        ns2x(
          TraceRow.rangeSelectObject!.endNS!,
          TraceRow.range?.startNS!,
          TraceRow.range?.endNS!,
          TraceRow.range?.totalNS!,
          this.timerShaftEL!.sportRuler!.frame
        )
      );
    }
    //在rowsEL显示范围内的 trace-row组件将收到时间区间变化通知
    this.linkNodes.forEach((it) => {
      it[0].x = ns2xByTimeShaft(it[0].ns, this.timerShaftEL!);
      it[1].x = ns2xByTimeShaft(it[1].ns, this.timerShaftEL!);
    });
    this.refreshCanvas(false, 'rangeChange');
  };
  tim: number = -1;
  top: number = 0;
  handler: any = undefined;
  rowsElOnScroll = (e: any) => {
    this.linkNodes.forEach((itln) => {
      if (itln[0].rowEL.collect) {
        itln[0].rowEL.translateY = itln[0].rowEL.getBoundingClientRect().top - 195;
      } else {
        itln[0].rowEL.translateY = itln[0].rowEL.offsetTop - this.rowsPaneEL!.scrollTop;
      }
      if (itln[1].rowEL.collect) {
        itln[1].rowEL.translateY = itln[1].rowEL.getBoundingClientRect().top - 195;
      } else {
        itln[1].rowEL.translateY = itln[1].rowEL.offsetTop - this.rowsPaneEL!.scrollTop;
      }
      itln[0].y = itln[0].rowEL.translateY + itln[0].offsetY;
      itln[1].y = itln[1].rowEL.translateY + itln[1].offsetY;
    });
    this.hoverStructNull();
    if (this.scrollTimer) {
      clearTimeout(this.scrollTimer);
    }
    this.scrollTimer = setTimeout(() => {
      TraceRow.range!.refresh = true;
      requestAnimationFrame(() => this.refreshCanvas(false));
    }, 200);
    requestAnimationFrame(() => this.refreshCanvas(false));
  };

  private scrollTimer: any;

  favoriteRowsElOnScroll = (e: any) => {
    this.rowsElOnScroll(e);
  };

  offset = 147;

  getRowsContentHeight(): number {
    return [...this.rowsEL!.querySelectorAll<TraceRow<any>>(`trace-row:not([sleeping])`)]
      .map((it) => it.clientHeight)
      .reduce((acr, cur) => acr + cur, 0);
  }

  documentOnMouseDown = (ev: MouseEvent) => {
    if (!this.loadTraceCompleted || !this.mouseEventEnable) return;
    if (this.isWASDKeyPress()) {
      ev.preventDefault();
      ev.stopPropagation();
      return;
    }
    this.isMouseLeftDown = true;
    if (ev.ctrlKey) {
      ev.preventDefault();
      this.style.cursor = 'move';
      this.mouseCurrentPosition = ev.clientX;
      return;
    }
    TraceRow.isUserInteraction = true;
    if (this.isMouseInSheet(ev)) return;
    this.observerScrollHeightEnable = false;
    if (ev.offsetX > this.timerShaftEL!.canvas!.offsetLeft) {
      let x = ev.offsetX - this.timerShaftEL!.canvas!.offsetLeft;
      let y = ev.offsetY;
      this.timerShaftEL?.documentOnMouseDown(ev);
      if (
        !(
          this.timerShaftEL!.sportRuler!.frame.contains(x, y) &&
          x > (TraceRow.rangeSelectObject?.startX || 0) &&
          x < (TraceRow.rangeSelectObject?.endX || 0)
        )
      ) {
        this.rangeSelect.mouseDown(ev);
        this.rangeSelect.drag = true;
      }
      //  如果鼠标摁下事件发生在traceRow范围或时间轴(sportRuler除外)范围内,清除上次点击调用栈产生的所有的三角旗子
      // ev.offsetY:鼠标在SpSystemTrace元素的y轴偏移量
      if (
        ev.offsetY > this.timerShaftEL!.clientHeight ||
        ev.offsetY < this.timerShaftEL!.clientHeight - this.timerShaftEL!.sportRuler!.frame.height
      ) {
        this.clearTriangle(this.timerShaftEL!.sportRuler!.flagList);
      }
    } else {
      this.rangeSelect.drag = false;
    }
  };

  onContextMenuHandler = (e: Event) => {
    setTimeout(() => {
      for (let key of this.keyPressMap.keys()) {
        if (this.keyPressMap.get(key)) {
          this.timerShaftEL?.stopWASD({ key: key });
          this.keyPressMap.set(key, false);
        }
      }
    }, 100);
  };

  cancelDrag(): void {
    this.rangeSelect.drag = false;
    this.rangeSelect.isMouseDown = false;
    TraceRow.rangeSelectObject = {
      startX: 0,
      endX: 0,
      startNS: 0,
      endNS: 0,
    };
  }

  documentOnMouseOut = (ev: MouseEvent) => {
    if (!this.loadTraceCompleted) return;
    TraceRow.isUserInteraction = false;
    if (this.isMouseInSheet(ev)) return;
    if (ev.offsetX > this.timerShaftEL!.canvas!.offsetLeft) {
      this.rangeSelect.mouseOut(ev);
      this.timerShaftEL?.documentOnMouseOut(ev);
    }
  };

  private keyPressMap: Map<string, boolean> = new Map([
    ['w', false],
    ['s', false],
    ['a', false],
    ['d', false],
    ['f', false],
  ]);
  documentOnKeyPress = (ev: KeyboardEvent) => {
    if (!this.loadTraceCompleted) return;
    let keyPress = ev.key.toLocaleLowerCase();
    TraceRow.isUserInteraction = true;
    if (this.isMousePointInSheet) {
      return;
    }
    this.observerScrollHeightEnable = false;
    if (this.keyboardEnable) {
      if (keyPress == 'm') {
        this.slicestime = this.setSLiceMark(ev.shiftKey);
        if (this.slicestime) {
          if (TraceRow.rangeSelectObject) {
            let showTab = this.getShowTab();
            this.traceSheetEL
              ?.displayTab<TabPaneCurrent>('tabpane-current', ...showTab)
              .setCurrentSlicesTime(this.slicestime);
          } else {
            this.traceSheetEL?.displayTab<TabPaneCurrent>('tabpane-current').setCurrentSlicesTime(this.slicestime);
          }
        }
      }
      if (keyPress === 'f') {
        // 设置当前的slicesTime
        this.setCurrentSlicesTime();
      }
      let keyPressWASD = keyPress === 'w' || keyPress === 'a' || keyPress === 's' || keyPress === 'd';
      if (keyPressWASD) {
        this.keyPressMap.set(keyPress, true);
        this.hoverFlag = null;
      }
      this.timerShaftEL!.documentOnKeyPress(ev, this.currentSlicesTime);
      if (keyPress === 'f') {
        this.verticalScrollToRow();
      }
    } else {
      this.stopWASD();
    }
  };

  verticalScrollToRow() {
    if (this.currentRow) {
      this.currentRow.scrollIntoView({ behavior: 'smooth', block: 'nearest' });
    }
  }

  setCurrentSlicesTime() {
    if (CpuStruct.selectCpuStruct) {
      if (CpuStruct.selectCpuStruct.startTime && CpuStruct.selectCpuStruct.dur) {
        this.currentSlicesTime.startTime = CpuStruct.selectCpuStruct.startTime;
        this.currentSlicesTime.endTime = CpuStruct.selectCpuStruct.startTime + CpuStruct.selectCpuStruct.dur;
      }
    } else if (ThreadStruct.selectThreadStruct) {
      if (ThreadStruct.selectThreadStruct.startTime && ThreadStruct.selectThreadStruct.dur) {
        this.currentSlicesTime.startTime = ThreadStruct.selectThreadStruct.startTime;
        this.currentSlicesTime.endTime =
          ThreadStruct.selectThreadStruct.startTime + ThreadStruct.selectThreadStruct.dur;
      }
    } else if (FuncStruct.selectFuncStruct) {
      if (FuncStruct.selectFuncStruct.startTs && FuncStruct.selectFuncStruct.dur) {
        this.currentSlicesTime.startTime = FuncStruct.selectFuncStruct.startTs;
        this.currentSlicesTime.endTime = FuncStruct.selectFuncStruct.startTs + FuncStruct.selectFuncStruct.dur;
      }
    } else if (IrqStruct.selectIrqStruct) {
      if (IrqStruct.selectIrqStruct.startNS && IrqStruct.selectIrqStruct.dur) {
        this.currentSlicesTime.startTime = IrqStruct.selectIrqStruct.startNS;
        this.currentSlicesTime.endTime = IrqStruct.selectIrqStruct.startNS + IrqStruct.selectIrqStruct.dur;
      }
    } else if (TraceRow.rangeSelectObject) {
      if (TraceRow.rangeSelectObject.startNS && TraceRow.rangeSelectObject.endNS) {
        this.currentSlicesTime.startTime = TraceRow.rangeSelectObject.startNS;
        this.currentSlicesTime.endTime = TraceRow.rangeSelectObject.endNS;
      }
    } else if (JankStruct.selectJankStruct) {
      if (JankStruct.selectJankStruct.ts && JankStruct.selectJankStruct.dur) {
        this.currentSlicesTime.startTime = JankStruct.selectJankStruct.ts;
        this.currentSlicesTime.endTime = JankStruct.selectJankStruct.ts + JankStruct.selectJankStruct.dur;
      }
    } else {
      this.currentSlicesTime.startTime = 0;
      this.currentSlicesTime.endTime = 0;
    }
  }

  stopWASD = () => {
    setTimeout(() => {
      for (let key of this.keyPressMap.keys()) {
        if (this.keyPressMap.get(key)) {
          this.timerShaftEL?.stopWASD({ key: key });
          this.keyPressMap.set(key, false);
        }
      }
    }, 100);
  };


  /**
   * 根据传入的参数实现卡尺和旗子的快捷跳转
   * @param list 要跳转的数组
   * @param type 标记类型（卡尺和旗子）
   * @param direction 跳转方向（前一个/后一个）
   */
  MarkJump(list: Array<any>, type: string, direction: string) {
    this.traceSheetEL = this.shadowRoot?.querySelector('.trace-sheet');
    let find = list.find((it) => it.selected);
    if (!find) {
      // 如果当前没有选中的，就选中第一个
      list.forEach((it) => (it.selected = false));
      list[0].selected = true;
    } else {
      for (let i = 0; i < list.length; i++) {
        // 将当前数组中选中的那条数据改为未选中
        if (list[i].selected) {
          list[i].selected = false;
          if (direction === 'previous') {
            if (i === 0) {
              // 如果当前选中的是第一个，就循环到最后一个上
              list[list.length - 1].selected = true;
              break;
            } else {
              // 选中当前的上一个
              list[i - 1].selected = true;
              break;
            }
          } else if (direction === 'next') {
            if (i === list.length - 1) {
              // 如果当前选中的是最后一个，就循环到第一个上
              list[0].selected = true;
              break;
            } else {
              // 选中当前的下一个
              list[i + 1].selected = true;
              break;
            }
          }
        }
      }
    }

    if (type === 'flag') {
      let currentPane = this.traceSheetEL?.displayTab<TabPaneFlag>('box-flag');
      list.forEach((flag, index) => {
        this.timerShaftEL!.sportRuler!.drawTriangle(flag.time, flag.type);
        if (flag.selected) {
          // 修改当前选中的旗子对应的表格中某行的背景
          currentPane!.setTableSelection(index + 1);
        }
      });
    } else if (type === 'slice') {
      this.refreshCanvas(true);
      let currentPane = this.traceSheetEL?.displayTab<TabPaneCurrent>('tabpane-current');
      list.forEach((slice, index) => {
        if (slice.selected) {
          // 修改当前选中的卡尺对应的表格中某行的背景
          currentPane!.setTableSelection(index + 1);
        }
      });
    }
  }

  isMouseInSheet = (ev: MouseEvent) => {
    this.isMousePointInSheet =
      this.traceSheetEL?.getAttribute('mode') != 'hidden' &&
      ev.offsetX > this.traceSheetEL!.offsetLeft &&
      ev.offsetY > this.traceSheetEL!.offsetTop;
    return this.isMousePointInSheet;
  };

  favoriteChangeHandler = (row: TraceRow<any>) => {
    info('favoriteChangeHandler', row.frame, row.offsetTop, row.offsetHeight);
  };

  verticalScrollHandler = (row: TraceRow<any>) => {
    row.scrollIntoView({ behavior: 'smooth', block: 'nearest' });
  };

  /**
   * 根据选择的traceRow 处理foler
   * @param currentRow 当前点击checkbox的row
   */
  setParentCheckStatus(currentRow: TraceRow<any>): void {
    if (currentRow.parentRowEl && currentRow.parentRowEl.folder && currentRow.parentRowEl.childrenList) {
      const parent = currentRow.parentRowEl;
      const childrenList = parent.childrenList;
      const selectList = [];
      const unSelectList = [];
      for (const child of childrenList) {
        if (child.offsetParent === null) {
          continue;
        }
        if (child.checkType === '2') {
          selectList.push(child);
        } else {
          unSelectList.push(child);
        }
      }
      if (unSelectList.length === 0) {
        parent.setAttribute('check-type', '2');
        parent.rangeSelect = true;
        parent.checkBoxEL!.checked = true;
        parent.checkBoxEL!.indeterminate = false;
      } else if (selectList.length === 0) {
        parent.setAttribute('check-type', '0');
        parent.rangeSelect = false;
        parent.checkBoxEL!.checked = false;
        parent.checkBoxEL!.indeterminate = false;
      } else {
        parent.setAttribute('check-type', '1');
        parent.rangeSelect = false;
        parent.checkBoxEL!.checked = false;
        parent.checkBoxEL!.indeterminate = true;
      }
    }
  }

  /**
   * 处理点击checkbox的逻辑
   * @param row 当前点击checkbox的row
   */
  selectChangeHandler = (row: TraceRow<any>): void => {
    this.setParentCheckStatus(row);
    const rows = [
      ...this.shadowRoot!.querySelectorAll<TraceRow<any>>("trace-row[check-type='2']"),
      ...this.favoriteChartListEL!.getAllSelectCollectRows(),
    ];
    this.isSelectClick = true;
    this.rangeSelect.rangeTraceRow = rows;
    let changeTraceRows: Array<TraceRow<any>> = [];
    if (this.rangeTraceRow!.length < rows.length) {
      rows!.forEach((currentTraceRow: TraceRow<any>) => {
        let changeFilter = this.rangeTraceRow!.filter(
          (prevTraceRow: TraceRow<any>) => prevTraceRow === currentTraceRow
        );
        if (changeFilter.length < 1) {
          changeTraceRows.push(currentTraceRow);
        }
      });
      if (changeTraceRows.length > 0) {
        changeTraceRows!.forEach((changeTraceRow: TraceRow<any>) => {
          let pointEvent = this.createPointEvent(changeTraceRow);
          SpStatisticsHttpUtil.addOrdinaryVisitAction({
            action: 'trace_row',
            event: pointEvent,
          });
        });
      }
    }
    this.rangeTraceRow = rows;
    this.rangeSelect.selectHandler?.(this.rangeSelect.rangeTraceRow, false);
  };
  inFavoriteArea: boolean | undefined;

  hoverStructNull() {
    CpuStruct.hoverCpuStruct = undefined;
    CpuFreqStruct.hoverCpuFreqStruct = undefined;
    ThreadStruct.hoverThreadStruct = undefined;
    FuncStruct.hoverFuncStruct = undefined;
    HiPerfCpuStruct.hoverStruct = undefined;
    HiPerfProcessStruct.hoverStruct = undefined;
    HiPerfThreadStruct.hoverStruct = undefined;
    HiPerfEventStruct.hoverStruct = undefined;
    HiPerfReportStruct.hoverStruct = undefined;
    CpuStateStruct.hoverStateStruct = undefined;
    CpuAbilityMonitorStruct.hoverCpuAbilityStruct = undefined;
    DiskAbilityMonitorStruct.hoverDiskAbilityStruct = undefined;
    MemoryAbilityMonitorStruct.hoverMemoryAbilityStruct = undefined;
    NetworkAbilityMonitorStruct.hoverNetworkAbilityStruct = undefined;
    CpuFreqLimitsStruct.hoverCpuFreqLimitsStruct = undefined;
    FpsStruct.hoverFpsStruct = undefined;
    ClockStruct.hoverClockStruct = undefined;
    IrqStruct.hoverIrqStruct = undefined;
    HeapStruct.hoverHeapStruct = undefined;
    JankStruct.hoverJankStruct = undefined;
    AppStartupStruct.hoverStartupStruct = undefined;
    SoStruct.hoverSoStruct = undefined;
    HeapSnapshotStruct.hoverSnapshotStruct = undefined;
    FrameAnimationStruct.hoverFrameAnimationStruct = undefined;
    FrameDynamicStruct.hoverFrameDynamicStruct = undefined;
    FrameSpacingStruct.hoverFrameSpacingStruct = undefined;
    JsCpuProfilerStruct.hoverJsCpuProfilerStruct = undefined;
    SnapshotStruct.hoverSnapshotStruct = undefined;
    HiPerfCallChartStruct.hoverPerfCallCutStruct = undefined;
    this.tipEL!.style.display = 'none';
  }

  selectStructNull() {
    CpuStruct.selectCpuStruct = undefined;
    CpuStruct.wakeupBean = null;
    CpuFreqStruct.selectCpuFreqStruct = undefined;
    ThreadStruct.selectThreadStruct = undefined;
    FuncStruct.selectFuncStruct = undefined;
    SpHiPerf.selectCpuStruct = undefined;
    CpuStateStruct.selectStateStruct = undefined;
    CpuFreqLimitsStruct.selectCpuFreqLimitsStruct = undefined;
    ClockStruct.selectClockStruct = undefined;
    IrqStruct.selectIrqStruct = undefined;
    JankStruct.selectJankStruct = undefined;
    HeapStruct.selectHeapStruct = undefined;
    AppStartupStruct.selectStartupStruct = undefined;
    SoStruct.selectSoStruct = undefined;
    HeapSnapshotStruct.selectSnapshotStruct = undefined;
    FrameSpacingStruct.selectFrameSpacingStruct = undefined;
    FrameAnimationStruct.selectFrameAnimationStruct = undefined;
    FrameDynamicStruct.selectFrameDynamicStruct = undefined;
    JsCpuProfilerStruct.selectJsCpuProfilerStruct = undefined;
    SnapshotStruct.selectSnapshotStruct = undefined;
    HiPerfCallChartStruct.selectStruct = undefined;
  }

  isWASDKeyPress() {
    return (
      this.keyPressMap.get('w') || this.keyPressMap.get('a') || this.keyPressMap.get('d') || this.keyPressMap.get('s')
    );
  }

  clickEmptyArea() {
    this.queryAllTraceRow().forEach((it) => {
      it.checkType = '-1';
      it.rangeSelect = false;
    });
    this.rangeSelect.rangeTraceRow = [];
    TraceRow.rangeSelectObject = undefined;
    this.selectStructNull();
    this.wakeupListNull();
    this.observerScrollHeightEnable = false;
    this.selectFlag = null;
    this.timerShaftEL?.removeTriangle('inverted');
    //   如果鼠标在SportRuler区域不隐藏tab页
    if (!SportRuler.isMouseInSportRuler) {
      this.traceSheetEL?.setAttribute('mode', 'hidden');
    }
    this.removeLinkLinesByBusinessType('task');
    this.refreshCanvas(true);
    JankStruct.delJankLineFlag = true;
  }

  //泳道图点击判定条件
  private traceRowClickJudgmentConditions: Map<string, () => boolean> = new Map<string, () => boolean>([
    [TraceRow.ROW_TYPE_CPU, () => CpuStruct.hoverCpuStruct !== null && CpuStruct.hoverCpuStruct !== undefined],
    [
      TraceRow.ROW_TYPE_THREAD,
      () => ThreadStruct.hoverThreadStruct !== null && ThreadStruct.hoverThreadStruct !== undefined,
    ],
    [TraceRow.ROW_TYPE_FUNC, () => FuncStruct.hoverFuncStruct !== null && FuncStruct.hoverFuncStruct !== undefined],
    [
      TraceRow.ROW_TYPE_CPU_FREQ,
      () => CpuFreqStruct.hoverCpuFreqStruct !== null && CpuFreqStruct.hoverCpuFreqStruct !== undefined,
    ],
    [
      TraceRow.ROW_TYPE_CPU_STATE,
      () => CpuStateStruct.hoverStateStruct !== null && CpuStateStruct.hoverStateStruct !== undefined,
    ],
    [
      TraceRow.ROW_TYPE_CPU_FREQ_LIMIT,
      () =>
        CpuFreqLimitsStruct.selectCpuFreqLimitsStruct !== null &&
        CpuFreqLimitsStruct.selectCpuFreqLimitsStruct !== undefined,
    ],
    [
      TraceRow.ROW_TYPE_CLOCK,
      () => ClockStruct.hoverClockStruct !== null && ClockStruct.hoverClockStruct !== undefined,
    ],
    [TraceRow.ROW_TYPE_IRQ, () => IrqStruct.hoverIrqStruct !== null && IrqStruct.hoverIrqStruct !== undefined],
    [
      TraceRow.ROW_TYPE_APP_STARTUP,
      () => AppStartupStruct.hoverStartupStruct !== null && AppStartupStruct.hoverStartupStruct !== undefined,
    ],
    [TraceRow.ROW_TYPE_STATIC_INIT, () => SoStruct.hoverSoStruct !== null && SoStruct.hoverSoStruct !== undefined],
    [TraceRow.ROW_TYPE_JANK, () => JankStruct.hoverJankStruct !== null && JankStruct.hoverJankStruct !== undefined],
    [TraceRow.ROW_TYPE_HEAP, () => HeapStruct.hoverHeapStruct !== null && HeapStruct.hoverHeapStruct !== undefined],
    [
      TraceRow.ROW_TYPE_SYS_MEMORY_GPU_TOTAL,
      () => SnapshotStruct.hoverSnapshotStruct !== null && SnapshotStruct.hoverSnapshotStruct !== undefined,
    ],
    [
      TraceRow.ROW_TYPE_SYS_MEMORY_GPU_WINDOW,
      () => SnapshotStruct.hoverSnapshotStruct !== null && SnapshotStruct.hoverSnapshotStruct !== undefined,
    ],
    [
      TraceRow.ROW_TYPE_HEAP_SNAPSHOT,
      () => HeapSnapshotStruct.hoverSnapshotStruct !== null && HeapSnapshotStruct.hoverSnapshotStruct !== undefined,
    ],
    [
      TraceRow.ROW_TYPE_FRAME_ANIMATION,
      () =>
        FrameAnimationStruct.hoverFrameAnimationStruct !== null &&
        FrameAnimationStruct.hoverFrameAnimationStruct !== undefined,
    ],
    [
      TraceRow.ROW_TYPE_FRAME_DYNAMIC,
      () =>
        FrameDynamicStruct.hoverFrameDynamicStruct !== null && FrameDynamicStruct.hoverFrameDynamicStruct !== undefined,
    ],
    [
      TraceRow.ROW_TYPE_FRAME_SPACING,
      () =>
        FrameSpacingStruct.hoverFrameSpacingStruct !== null && FrameSpacingStruct.hoverFrameSpacingStruct !== undefined,
    ],
    [
      TraceRow.ROW_TYPE_JS_CPU_PROFILER,
      () =>
        JsCpuProfilerStruct.hoverJsCpuProfilerStruct !== null &&
        JsCpuProfilerStruct.hoverJsCpuProfilerStruct !== undefined,
    ],
    [
      TraceRow.ROW_TYPE_PURGEABLE_TOTAL_ABILITY,
      () => SnapshotStruct.hoverSnapshotStruct !== null && SnapshotStruct.hoverSnapshotStruct !== undefined,
    ],
    [
      TraceRow.ROW_TYPE_PURGEABLE_PIN_ABILITY,
      () => SnapshotStruct.hoverSnapshotStruct !== null && SnapshotStruct.hoverSnapshotStruct !== undefined,
    ],
    [
      TraceRow.ROW_TYPE_PURGEABLE_TOTAL_VM,
      () => SnapshotStruct.hoverSnapshotStruct !== null && SnapshotStruct.hoverSnapshotStruct !== undefined,
    ],
    [
      TraceRow.ROW_TYPE_PURGEABLE_PIN_VM,
      () => SnapshotStruct.hoverSnapshotStruct !== null && SnapshotStruct.hoverSnapshotStruct !== undefined,
    ],
    [
      TraceRow.ROW_TYPE_DMA_ABILITY,
      () => SnapshotStruct.hoverSnapshotStruct !== null && SnapshotStruct.hoverSnapshotStruct !== undefined,
    ],
    [
      TraceRow.ROW_TYPE_DMA_VMTRACKER,
      () => SnapshotStruct.hoverSnapshotStruct !== null && SnapshotStruct.hoverSnapshotStruct !== undefined,
    ],
    [
      TraceRow.ROW_TYPE_GPU_MEMORY_ABILITY,
      () => SnapshotStruct.hoverSnapshotStruct !== null && SnapshotStruct.hoverSnapshotStruct !== undefined,
    ],
    [
      TraceRow.ROW_TYPE_GPU_MEMORY_VMTRACKER,
      () => SnapshotStruct.hoverSnapshotStruct !== null && SnapshotStruct.hoverSnapshotStruct !== undefined,
    ],
    [
      TraceRow.ROW_TYPE_GPU_RESOURCE_VMTRACKER,
      () => SnapshotStruct.hoverSnapshotStruct !== null && SnapshotStruct.hoverSnapshotStruct !== undefined,
    ],
    [
      TraceRow.ROW_TYPE_VMTRACKER_SHM,
      () => SnapshotStruct.hoverSnapshotStruct !== null && SnapshotStruct.hoverSnapshotStruct !== undefined,
    ],
    [
      TraceRow.ROW_TYPE_VM_TRACKER_SMAPS,
      () => SnapshotStruct.hoverSnapshotStruct !== null && SnapshotStruct.hoverSnapshotStruct !== undefined,
    ],
    [TraceRow.ROW_TYPE_LOGS, () => LogStruct.hoverLogStruct !== null && LogStruct.hoverLogStruct !== undefined],
  ]);

  makePoint(
    ts: number,
    dur: number,
    translateY: number,
    rowStruct: any,
    offsetY: number,
    business: string,
    lineType: LineType,
    isRight: boolean
  ): PairPoint {
    return {
      x: ns2xByTimeShaft(ts + dur, this.timerShaftEL!),
      y: translateY!,
      offsetY: offsetY,
      ns: ts + dur,
      rowEL: rowStruct!,
      isRight: isRight,
      business: business,
      lineType: lineType,
    };
  }

  translateByMouseMove(ev: MouseEvent): void {
    ev.preventDefault();
    let offset = 0;
    if (this.offsetMouse === 0) {
      this.offsetMouse = ev.clientX;
      offset = ev.clientX - this.mouseCurrentPosition;
    } else {
      offset = ev.clientX - this.offsetMouse;
    }
    this.offsetMouse = ev.clientX;
    const rangeRuler = this.timerShaftEL?.getRangeRuler()!;

    rangeRuler.translate(offset);
  }

  connectedCallback() {
    this.initPointToEvent();
    /**
     * 监听时间轴区间变化
     */
    this.timerShaftEL!.rangeChangeHandler = this.timerShaftELRangeChange;
    this.timerShaftEL!.rangeClickHandler = this.timerShaftELRangeClick;
    this.timerShaftEL!.flagChangeHandler = this.timerShaftELFlagChange;
    this.timerShaftEL!.flagClickHandler = this.timerShaftELFlagClickHandler;
    /**
     * 监听rowsEL的滚动时间，刷新可见区域的trace-row组件的时间区间（将触发trace-row组件重绘）
     */
    this.rowsPaneEL?.addEventListener('scroll', this.rowsElOnScroll, {
      passive: true,
    });
    /**
     * 监听document的mousemove事件 坐标通过换算后找到当前鼠标所在的trace-row组件，将坐标传入
     */
    this.addEventListener('mousemove', this.documentOnMouseMove);
    this.addEventListener('click', this.documentOnClick);
    this.addEventListener('mousedown', this.documentOnMouseDown);
    this.addEventListener('mouseup', this.documentOnMouseUp);
    this.addEventListener('mouseout', this.documentOnMouseOut);

    document.addEventListener('keypress', this.documentOnKeyPress);
    document.addEventListener('keyup', this.documentOnKeyUp);
    document.addEventListener('contextmenu', this.onContextMenuHandler);

    /**
     * 泳道图中添加ctrl+鼠标滚轮事件，对泳道图进行放大缩小。
     * 鼠标滚轮事件转化为键盘事件，keyPress和keyUp两个事件需要配合使用，
     * 否则泳道图会一直放大或一直缩小。
     * setTimeout()函数中的时间参数可以控制鼠标滚轮的频率。
     */
    document.addEventListener(
      'wheel',
      (e) => {
        if (e.ctrlKey) {
          if (e.deltaY > 0) {
            e.preventDefault();
            e.stopPropagation();
            let eventS = new KeyboardEvent('keypress', {
              key: 's',
              code: '83',
              keyCode: 83,
            });
            this.timerShaftEL!.documentOnKeyPress(eventS);
            setTimeout(() => {
              this.timerShaftEL!.documentOnKeyUp(eventS);
            }, 200);
          }
          if (e.deltaY < 0) {
            e.preventDefault();
            e.stopPropagation();
            let eventW = new KeyboardEvent('keypress', {
              key: 'w',
              code: '87',
              keyCode: 87,
            });
            this.timerShaftEL!.documentOnKeyPress(eventW);
            setTimeout(() => {
              this.timerShaftEL!.documentOnKeyUp(eventW);
            }, 200);
          }
        }
      },
      { passive: false }
    );

    SpApplication.skinChange2 = (val: boolean) => {
      this.timerShaftEL?.render();
    };
    window.subscribe(window.SmartEvent.UI.UploadSOFile, (data) => {
      this.chartManager?.importSoFileUpdate().then(() => {
        window.publish(window.SmartEvent.UI.Loading, false);
        let updateCanvas = this.traceSheetEL?.updateRangeSelect();
        if (updateCanvas) {
          this.refreshCanvas(true);
        }
      });
    });
    window.subscribe(window.SmartEvent.UI.CheckALL, (data) => {
      this.getCollectRows((row) => row.rowParentId === data.rowId).forEach((it) => {
        it.checkType = data.isCheck ? '2' : '0';
      });
    });
    window.subscribe(window.SmartEvent.UI.HoverNull, () => this.hoverStructNull());
  }

  favoriteAreaSearchHandler(row: TraceRow<any>): void {
    if (this.timerShaftEL!.collecBtn!.hasAttribute('close')) {
      this.timerShaftEL!.collecBtn!.removeAttribute('close');
      this.favoriteChartListEL!.showCollectArea();
    }
    this.favoriteChartListEL?.expandSearchRowGroup(row);
  }

  scrollToProcess(rowId: string, rowParentId: string, rowType: string, smooth: boolean = true) {
    let traceRow =
      this.shadowRoot!.querySelector<TraceRow<any>>(`trace-row[row-id='${rowId}'][row-type='${rowType}']`) ||
      this.favoriteChartListEL!.getCollectRow((row) => row.rowId === rowId && row.rowType === rowType);
    if (traceRow?.collect) {
      this.favoriteChartListEL!.scroll({
        top:
          (traceRow?.offsetTop || 0) -
          this.favoriteChartListEL!.getCanvas()!.offsetHeight +
          (traceRow?.offsetHeight || 0),
        left: 0,
        behavior: smooth ? 'smooth' : undefined,
      });
    } else {
      let row = this.shadowRoot!.querySelector<TraceRow<any>>(`trace-row[row-id='${rowParentId}'][folder]`);
      if (row && !row.expansion) {
        row.expansion = true;
      }
      if (traceRow && traceRow.offsetTop >= 0 && traceRow.offsetHeight >= 0) {
        this.rowsPaneEL!.scroll({
          top: (traceRow?.offsetTop || 0) - this.canvasPanel!.offsetHeight + (traceRow?.offsetHeight || 0),
          left: 0,
          behavior: smooth ? 'smooth' : undefined,
        });
      }
    }
  }

  scrollToDepth(rowId: string, rowParentId: string, rowType: string, smooth: boolean = true, depth: number) {
    let rootRow =
      this.shadowRoot!.querySelector<TraceRow<any>>(`trace-row[row-id='${rowId}'][row-type='${rowType}']`) ||
      this.favoriteChartListEL!.getCollectRow((row) => row.rowId === rowId && row.rowType === rowType);
    if (rootRow && rootRow!.collect) {
      this.favoriteAreaSearchHandler(rootRow);
      rootRow.expandFunc();
      this.favoriteChartListEL!.scroll({
        top: (rootRow?.offsetTop || 0) - this.favoriteChartListEL!.getCanvas()!.offsetHeight + (++depth * 20 || 0),
        left: 0,
        behavior: smooth ? 'smooth' : undefined,
      });
    } else {
      let row = this.shadowRoot!.querySelector<TraceRow<any>>(`trace-row[row-id='${rowParentId}'][folder]`);
      if (row && !row.expansion) {
        row.expansion = true;
      }
      if (rootRow) {
        rootRow.expandFunc();
      }
      if (rootRow && rootRow.offsetTop >= 0 && rootRow.offsetHeight >= 0) {
        this.rowsPaneEL!.scroll({
          top: (rootRow?.offsetTop || 0) - this.canvasPanel!.offsetHeight + (++depth * 20 || 0),
          left: 0,
          behavior: smooth ? 'smooth' : undefined,
        });
      }
    }
  }

  scrollToFunction(rowId: string, rowParentId: string, rowType: string, smooth: boolean = true) {
    let condition = `trace-row[row-id='${rowId}'][row-type='${rowType}'][row-parent-id='${rowParentId}']`;
    let rootRow =
      this.shadowRoot!.querySelector<TraceRow<any>>(condition) ||
      this.favoriteChartListEL!.getCollectRow((row) => {
        return row.rowId === rowId && row.rowType === rowType && row.rowParentId === rowParentId;
      });
    if (rootRow?.collect) {
      this.favoriteAreaSearchHandler(rootRow);
      this.favoriteChartListEL!.scroll({
        top:
          (rootRow?.offsetTop || 0) -
          this.favoriteChartListEL!.getCanvas()!.offsetHeight +
          (rootRow?.offsetHeight || 0),
        left: 0,
        behavior: smooth ? 'smooth' : undefined,
      });
    } else {
      let row = this.shadowRoot!.querySelector<TraceRow<any>>(`trace-row[row-id='${rowParentId}'][folder]`);
      if (row && !row.expansion) {
        row.expansion = true;
      }
      if (rootRow && rootRow.offsetTop >= 0 && rootRow.offsetHeight >= 0) {
        this.rowsPaneEL!.scroll({
          top: (rootRow?.offsetTop || 0) - this.canvasPanel!.offsetHeight + 20,
          left: 0,
          behavior: smooth ? 'smooth' : undefined,
        });
      }
    }
  }

  disconnectedCallback() {
    this.timerShaftEL?.removeEventListener('range-change', this.timerShaftELRangeChange);
    this.rowsPaneEL?.removeEventListener('scroll', this.rowsElOnScroll);
    this.removeEventListener('mousemove', this.documentOnMouseMove);
    this.removeEventListener('click', this.documentOnClick);
    this.removeEventListener('mousedown', this.documentOnMouseDown);
    this.removeEventListener('mouseup', this.documentOnMouseUp);
    this.removeEventListener('mouseout', this.documentOnMouseOut);
    document.removeEventListener('keypress', this.documentOnKeyPress);
    document.removeEventListener('keyup', this.documentOnKeyUp);
    document.removeEventListener('contextmenu', this.onContextMenuHandler);
    window.unsubscribe(window.SmartEvent.UI.SliceMark, this.sliceMarkEventHandler.bind(this));
  }

  sliceMarkEventHandler(ev: any) {
    SpSystemTrace.sliceRangeMark = ev;
    let startNS = ev.timestamp - (window as any).recordStartNS;
    let endNS = ev.maxDuration + startNS;
    TraceRow.rangeSelectObject = {
      startX: 0,
      startNS: startNS,
      endNS: endNS,
      endX: 0,
    };
    window.publish(window.SmartEvent.UI.MenuTrace, {});
    window.publish(window.SmartEvent.UI.TimeRange, {
      startNS: startNS - ev.maxDuration,
      endNS: endNS + ev.maxDuration,
    });
    this.queryAllTraceRow().forEach((it) => (it.checkType = '-1'));
    this.rangeSelect.rangeTraceRow = [];
    this.selectStructNull();
    this.wakeupListNull();
    this.traceSheetEL?.setAttribute('mode', 'hidden');
    this.removeLinkLinesByBusinessType('janks');
    TraceRow.range!.refresh = true;
    this.refreshCanvas(false);
  }

  loadDatabaseUrl(
    url: string,
    progress: Function,
    complete?: ((res: { status: boolean; msg: string }) => void) | undefined
  ) {
    this.observerScrollHeightEnable = false;
    this.init({ url: url }, '', progress).then((res) => {
      if (complete) {
        complete(res);
        window.publish(window.SmartEvent.UI.MouseEventEnable, {
          mouseEnable: true,
        });
      }
    });
  }

  loadDatabaseArrayBuffer(
    buf: ArrayBuffer,
    thirdPartyWasmConfigUrl: string,
    progress: (name: string, percent: number) => void,
    complete?: ((res: { status: boolean; msg: string }) => void) | undefined
  ) {
    this.observerScrollHeightEnable = false;
    this.init({ buf }, thirdPartyWasmConfigUrl, progress).then((res) => {
      this.rowsEL?.querySelectorAll('trace-row').forEach((it: any) => this.observer.observe(it));
      if (complete) {
        complete(res);
        window.publish(window.SmartEvent.UI.MouseEventEnable, {
          mouseEnable: true,
        });
      }
    });
  }
  queryAllTraceRow<T>(selectors?: string, filter?: (row: TraceRow<any>) => boolean): TraceRow<any>[] {
    return [
      ...this.shadowRoot!.querySelectorAll<TraceRow<any>>(selectors ?? 'trace-row'),
      ...this.favoriteChartListEL!.getCollectRows(filter),
    ];
  }
  search(query: string) {
    this.queryAllTraceRow().forEach((item) => {
      if (query == null || query == undefined || query == '') {
        if (
          item.rowType == TraceRow.ROW_TYPE_CPU ||
          item.rowType == TraceRow.ROW_TYPE_CPU_FREQ ||
          item.rowType == TraceRow.ROW_TYPE_NATIVE_MEMORY ||
          item.rowType == TraceRow.ROW_TYPE_FPS ||
          item.rowType == TraceRow.ROW_TYPE_PROCESS ||
          item.rowType == TraceRow.ROW_TYPE_CPU_ABILITY ||
          item.rowType == TraceRow.ROW_TYPE_MEMORY_ABILITY ||
          item.rowType == TraceRow.ROW_TYPE_DISK_ABILITY ||
          item.rowType == TraceRow.ROW_TYPE_NETWORK_ABILITY
        ) {
          item.expansion = false;
          item.rowHidden = false;
        } else {
          item.rowHidden = true;
        }
      } else {
        if (item.name.toLowerCase().indexOf(query.toLowerCase()) >= 0) {
          item.rowHidden = false;
        } else {
          item.rowHidden = true;
        }
      }
    });
    this.visibleRows.forEach((it) => (it.rowHidden = false && it.draw(true)));
  }

  searchCPU(query: string): Array<CpuStruct> {
    let traceRow =
      this.shadowRoot!.querySelector<TraceRow<any>>(`trace-row[scene]`) ||
      this.favoriteChartListEL!.getCollectRow((row) => row.hasAttribute('scene'));
    let dataAll = `trace-row[row-type='cpu-data']`;
    if (traceRow) {
      dataAll = `trace-row[row-type='cpu-data'][scene]`;
    }
    let searchResults: Array<CpuStruct> = [];
    let repairData = (item: TraceRow<any>) => {
      let res = item!.dataList!.filter(
        (it) =>
          (it.name && it.name.indexOf(query) >= 0) ||
          it.tid == query ||
          it.processId == query ||
          (it.processName && it.processName.indexOf(query) >= 0)
      );
      searchResults.push(...res);
    };
    this.shadowRoot!.querySelectorAll<TraceRow<any>>(`${dataAll}`).forEach(repairData);
    if (traceRow) {
      this.favoriteChartListEL!.getCollectRows(
        (row) => row.rowType === 'cpu-data' && row.hasAttribute('scene')
      )?.forEach(repairData);
    } else {
      this.favoriteChartListEL!.getCollectRows((row) => row.rowType === 'cpu-data')?.forEach(repairData);
    }
    searchResults.sort((a, b) => (a.startTime || 0) - (b.startTime || 0));
    return searchResults;
  }

  async searchFunction(cpuList: Array<any>, query: string): Promise<Array<any>> {
    let processList: Array<string> = [];
    let traceRow =
      this.shadowRoot!.querySelector<TraceRow<any>>(`trace-row[scene]`) ||
      this.favoriteChartListEL!.getCollectRow((row) => row.hasAttribute('scene'));
    if (traceRow) {
      this.shadowRoot!.querySelectorAll<TraceRow<any>>(`trace-row[row-type='process'][scene]`).forEach((row) => {
        processList.push(row.rowId!);
      });
      if (query.includes('_')) {
        query = query.replace(/_/g, '\\_');
      }
      if (query.includes('%')) {
        query = query.replace(/%/g, '\\%');
      }
      let list = await querySceneSearchFunc(query, processList);
      cpuList = cpuList.concat(list);
      cpuList.sort((a, b) => (a.startTime || 0) - (b.startTime || 0));
      return cpuList;
    } else {
      let list = await querySearchFunc(query);
      cpuList = cpuList.concat(list);
      cpuList.sort((a, b) => (a.startTime || 0) - (b.startTime || 0));
      return cpuList;
    }
  }

  searchSdk(dataList: Array<any>, query: string): Array<any> {
    let traceRow =
      this.shadowRoot!.querySelector<TraceRow<any>>(`trace-row[scene]`) ||
      this.favoriteChartListEL!.getCollectRow((row) => row.hasAttribute('scene'));
    let dataAll = `trace-row[row-type^='sdk']`;
    if (traceRow) {
      dataAll = `trace-row[row-type^='sdk'][scene]`;
    }
    let allTraceRow: any = [];
    let parentRows = this.shadowRoot!.querySelectorAll<TraceRow<any>>(`${dataAll}`);
    parentRows.forEach((parentRow: TraceRow<any>) => {
      allTraceRow.push(parentRow);
      if (parentRow.childrenList && parentRow.childrenList.length > 0) {
        allTraceRow.push(...parentRow.childrenList);
      }
    });
    allTraceRow.forEach((row: any) => {
      if (row!.name.indexOf(query) >= 0) {
        let searchSdkBean = new SearchSdkBean();
        searchSdkBean.startTime = TraceRow.range!.startNS;
        searchSdkBean.dur = TraceRow.range!.totalNS;
        searchSdkBean.name = row.name;
        searchSdkBean.rowId = row.rowId;
        searchSdkBean.type = 'sdk';
        searchSdkBean.rowType = row.rowType;
        searchSdkBean.rowParentId = row.rowParentId;
        dataList.push(searchSdkBean);
      }
    });
    return dataList;
  }

  scrollToActFunc(funcStract: any, highlight: boolean) {
    const toTargetDepth = (entry: any) => {
      if (entry) {
        this.hoverStructNull();
        this.selectStructNull();
        this.wakeupListNull();
        FuncStruct.hoverFuncStruct = entry;
        FuncStruct.selectFuncStruct = entry;
        this.onClickHandler(TraceRow.ROW_TYPE_FUNC);
        this.scrollToDepth(`${funcRowID}`, `${funcStract.pid}`, funcStract.type, true, entry.depth || 0);
      }
    };
    let funcRowID = funcStract.cookie == null ? funcStract.tid : `${funcStract.funName}-${funcStract.pid}`;
    let targetRow = this.favoriteChartListEL?.getCollectRow((row) => {
      return row.rowId === funcRowID && row.rowType === 'func';
    });
    if (targetRow) {
      targetRow.highlight = highlight;
      //如果目标泳道图在收藏上面，则跳转至收藏
      let searchEntry = targetRow!.dataList!.find((dat) => dat.startTs === funcStract.startTime);
      toTargetDepth(searchEntry);
      return;
    }
    let parentRow = this.shadowRoot!.querySelector<TraceRow<any>>(`trace-row[row-id='${funcStract.pid}'][folder]`);
    if (!parentRow) {
      return;
    }
    let filterRow = parentRow.childrenList.filter((child) => child.rowId == funcRowID && child.rowType == 'func')[0];
    if (filterRow == null) {
      let funcRow = this.shadowRoot?.querySelector<TraceRow<any>>(`trace-row[row-id='${funcRowID}'][row-type='func']`);
      if (funcRow) {
        filterRow = funcRow;
      } else {
        return;
      }
    }
    filterRow!.highlight = highlight;
    let row = this.shadowRoot!.querySelector<TraceRow<any>>(`trace-row[row-id='${funcStract.pid}'][folder]`);
    if (row && !row.expansion) {
      row.expansion = true;
    }
    const completeEntry = () => {
      let entry = filterRow!.dataList!.find((dat) => dat.startTs === funcStract.startTime);
      toTargetDepth(entry);
    };
    if (filterRow!.isComplete) {
      completeEntry();
    } else {
      FuncStruct.hoverFuncStruct = funcStract;
      FuncStruct.selectFuncStruct = funcStract;
      this.onClickHandler(TraceRow.ROW_TYPE_FUNC);
      this.scrollToProcess(`${funcStract.tid}`, `${funcStract.pid}`, 'process', false);
      this.scrollToFunction(`${funcStract.tid}`, `${funcStract.pid}`, 'func', true);
      filterRow!.onComplete = completeEntry;
    }
  }

  closeAllExpandRows(pid: string) {
    let expandRows = this.rowsEL?.querySelectorAll<TraceRow<ProcessStruct>>(`trace-row[row-type='process'][expansion]`);
    expandRows?.forEach((row) => {
      if (row.rowId != pid) {
        row.expansion = false;
      }
    });
  }

  moveRangeToCenter(startTime: number, dur: number) {
    let startNS = this.timerShaftEL?.getRange()?.startNS || 0;
    let endNS = this.timerShaftEL?.getRange()?.endNS || 0;
    let harfDur = Math.trunc((endNS - startNS) / 2 - dur / 2);
    let leftNs = startTime - harfDur;
    let rightNs = startTime + dur + harfDur;
    if (startTime - harfDur < 0) {
      leftNs = 0;
      rightNs += harfDur - startTime;
    }
    this.timerShaftEL?.setRangeNS(leftNs, rightNs);
    TraceRow.range!.refresh = true;
    this.refreshCanvas(true);
  }

  showPreCpuStruct(currentIndex: number, cpuStructs: Array<CpuStruct>): number {
    if (cpuStructs.length == 0) {
      return 0;
    }
    let findIndex = -1;
    for (let i = cpuStructs.length - 1; i >= 0; i--) {
      let it = cpuStructs[i];
      if (
        i < currentIndex &&
        it.startTime! >= TraceRow.range!.startNS &&
        it.startTime! + it.dur! <= TraceRow.range!.endNS
      ) {
        findIndex = i;
        break;
      }
    }
    if (findIndex >= 0) {
      let findEntry = cpuStructs[findIndex];
      CpuStruct.selectCpuStruct = findEntry;
      this.rowsEL!.querySelectorAll<TraceRow<any>>(`trace-row[row-type='cpu-data']`).forEach((item) => {
        item.highlight = item.rowId == `${findEntry.cpu}`;
        item.draw(true);
      });
      this.timerShaftEL?.drawTriangle(findEntry.startTime || 0, 'inverted');
    } else {
      for (let i = cpuStructs.length - 1; i >= 0; i--) {
        let it = cpuStructs[i];
        if (it.startTime! + it.dur! < TraceRow.range!.startNS) {
          findIndex = i;
          break;
        }
      }
      let findEntry: CpuStruct;
      if (findIndex == -1) {
        findIndex = cpuStructs.length - 1;
      }
      findEntry = cpuStructs[findIndex];
      CpuStruct.selectCpuStruct = findEntry;
      let startNS = this.timerShaftEL?.getRange()?.startNS || 0;
      let endNS = this.timerShaftEL?.getRange()?.endNS || 0;
      let harfDur = Math.trunc((endNS - startNS) / 2 - findEntry.dur! / 2);
      this.timerShaftEL?.setRangeNS(findEntry.startTime! - harfDur, findEntry.startTime! + findEntry.dur! + harfDur);
      this.rowsEL!.querySelectorAll<TraceRow<any>>(`trace-row[row-type='cpu-data']`).forEach((item) => {
        item.highlight = item.rowId == `${findEntry.cpu}`;
        item.draw(true);
      });
      this.timerShaftEL?.drawTriangle(findEntry.startTime || 0, 'inverted');
    }
    CpuStruct.hoverCpuStruct = CpuStruct.selectCpuStruct;
    this.onClickHandler(TraceRow.ROW_TYPE_CPU);
    return findIndex;
  }

  showNextCpuStruct(currentIndex: number, cpuStructs: Array<CpuStruct>): number {
    if (cpuStructs.length == 0) {
      return 0;
    }
    let findIndex = cpuStructs.findIndex((it, idx) => {
      return (
        idx > currentIndex &&
        it.startTime! >= TraceRow.range!.startNS &&
        it.startTime! + it.dur! <= TraceRow.range!.endNS
      );
    });
    if (findIndex >= 0) {
      let findEntry = cpuStructs[findIndex];
      CpuStruct.selectCpuStruct = findEntry;
      this.rowsEL!.querySelectorAll<TraceRow<any>>(`trace-row[row-type='cpu-data']`).forEach((item) => {
        item.highlight = item.rowId == `${findEntry.cpu}`;
        item.draw(true);
      });
      this.timerShaftEL?.drawTriangle(findEntry.startTime || 0, 'inverted');
    } else {
      findIndex = cpuStructs.findIndex((it) => it.startTime! > TraceRow.range!.endNS);
      let findEntry: CpuStruct;
      if (findIndex == -1) {
        findIndex = 0;
      }
      findEntry = cpuStructs[findIndex];
      CpuStruct.selectCpuStruct = findEntry;
      let startNS = this.timerShaftEL?.getRange()?.startNS || 0;
      let endNS = this.timerShaftEL?.getRange()?.endNS || 0;
      let harfDur = Math.trunc((endNS - startNS) / 2 - findEntry.dur! / 2);
      this.timerShaftEL?.setRangeNS(findEntry.startTime! - harfDur, findEntry.startTime! + findEntry.dur! + harfDur);
      this.rowsEL!.querySelectorAll<TraceRow<any>>(`trace-row[row-type='cpu-data']`).forEach((item) => {
        item.highlight = item.rowId == `${findEntry.cpu}`;
        item.draw(true);
      });
      this.timerShaftEL?.drawTriangle(findEntry.startTime || 0, 'inverted');
    }
    CpuStruct.hoverCpuStruct = CpuStruct.selectCpuStruct;
    this.onClickHandler(TraceRow.ROW_TYPE_CPU);
    return findIndex;
  }

  reset(progress: Function | undefined | null) {
    this.visibleRows.length = 0;
    this.tipEL!.style.display = 'none';
    this.canvasPanelCtx?.clearRect(0, 0, this.canvasPanel!.clientWidth, this.canvasPanel!.offsetHeight);
    this.loadTraceCompleted = false;
    this.collectRows = [];
    this.visibleRows = [];
    TraceRowConfig.allTraceRowList.forEach((it) => {
      it.clearMemory();
    });
    TraceRowConfig.allTraceRowList = [];
    this.favoriteChartListEL!.reset();
    if (this.rowsEL) {
      this.rowsEL.querySelectorAll<TraceRow<any>>(`trace-row`).forEach((row) => {
        row.clearMemory();
        this.rowsEL!.removeChild(row);
      });
      this.rowsEL.innerHTML = '';
    }
    this.traceSheetEL?.clearMemory();
    this.spacerEL!.style.height = '0px';
    this.rangeSelect.rangeTraceRow = [];
    SpSystemTrace.SDK_CONFIG_MAP = undefined;
    SpSystemTrace.sliceRangeMark = undefined;
    this.timerShaftEL?.displayCollect(false);
    this.timerShaftEL!.collecBtn!.removeAttribute('close');
    CpuStruct.wakeupBean = undefined;
    this.selectStructNull();
    this.hoverStructNull();
    this.wakeupListNull();
    this.traceSheetEL?.setAttribute('mode', 'hidden');
    progress && progress('rest timershaft', 8);
    this.timerShaftEL?.reset();
    progress && progress('clear cache', 10);
    HeapDataInterface.getInstance().clearData();
    procedurePool.clearCache();
    Utils.clearData();
    InitAnalysis.getInstance().isInitAnalysis = true;
    procedurePool.submitWithName('logic0', 'clear', {}, undefined, (res: any) => {});
    procedurePool.submitWithName('logic1', 'clear', {}, undefined, (res: any) => {});
  }

  init = async (param: { buf?: ArrayBuffer; url?: string }, wasmConfigUri: string, progress: Function) => {
    progress('Load database', 6);
    this.rowsPaneEL!.scroll({
      top: 0,
      left: 0,
    });
    if (param.buf) {
      let configJson = '';
      try {
        configJson = await fetch(wasmConfigUri).then((res) => res.text());
      } catch (e) {
        error('getWasmConfigFailed', e);
      }
      let parseConfig = FlagsConfig.getSpTraceStreamParseConfig();
      let { status, msg, sdkConfigMap } = await threadPool.initSqlite(param.buf, parseConfig, configJson, progress);
      if (!status) {
        return { status: false, msg: msg };
      }
      SpSystemTrace.SDK_CONFIG_MAP = sdkConfigMap == undefined ? undefined : sdkConfigMap;
    }
    if (param.url) {
      let { status, msg } = await threadPool.initServer(param.url, progress);
      if (!status) {
        return { status: false, msg: msg };
      }
    }
    await this.chartManager?.init(progress);
    let rowId: string = '';
    this.rowsEL?.querySelectorAll<TraceRow<any>>('trace-row').forEach((it: any) => {
      if (it.name.includes('Ark Ts')) {
        rowId = it.rowId;
      }
      if (it.folder) it.addEventListener('expansion-change', this.extracted(it));
    });
    progress('completed', 100);
    info('All TraceRow Data initialized');
    this.loadTraceCompleted = true;
    this.rowsEL!.querySelectorAll<TraceRow<any>>('trace-row').forEach((it) => {
      if (rowId !== '' && (it.rowId?.includes(rowId) || it.name.includes(rowId))) {
        it.addTemplateTypes('Ark Ts');
        for (let child of it.childrenList) {
          child.addTemplateTypes('Ark Ts');
        }
      }
      if (it.folder) {
        let offsetYTimeOut: any = undefined;
        it.addEventListener('expansion-change', (event: any) => {
          JankStruct.delJankLineFlag = false;
          if (offsetYTimeOut) {
            clearTimeout(offsetYTimeOut);
          }
          if (event.detail.expansion) {
            offsetYTimeOut = setTimeout(() => {
              this.linkNodes.forEach((linkNode) => {
                JankStruct.selectJankStructList?.forEach((selectStruct: any) => {
                  if (event.detail.rowId == selectStruct.pid) {
                    JankStruct.selectJankStruct = selectStruct;
                    JankStruct.hoverJankStruct = selectStruct;
                  }
                });
                if (linkNode[0].rowEL.collect) {
                  linkNode[0].rowEL.translateY = linkNode[0].rowEL.getBoundingClientRect().top - 195;
                } else {
                  linkNode[0].rowEL.translateY = linkNode[0].rowEL.offsetTop - this.rowsPaneEL!.scrollTop;
                }
                linkNode[0].y = linkNode[0].rowEL!.translateY! + linkNode[0].offsetY;
                if (linkNode[1].rowEL.collect) {
                  linkNode[1].rowEL.translateY = linkNode[1].rowEL.getBoundingClientRect().top - 195;
                } else {
                  linkNode[1].rowEL.translateY = linkNode[1].rowEL.offsetTop - this.rowsPaneEL!.scrollTop;
                }
                linkNode[1].y = linkNode[1].rowEL!.translateY! + linkNode[1].offsetY;
              });
            }, 300);
          } else {
            if (JankStruct!.selectJankStruct) {
              JankStruct.selectJankStructList?.push(<JankStruct>JankStruct!.selectJankStruct);
            }
            offsetYTimeOut = setTimeout(() => {
              this.linkNodes?.forEach((linkNode) => {
                if (linkNode[0].rowEL.collect) {
                  linkNode[0].rowEL.translateY = linkNode[0].rowEL.getBoundingClientRect().top - 195;
                } else {
                  linkNode[0].rowEL.translateY = linkNode[0].rowEL.offsetTop - this.rowsPaneEL!.scrollTop;
                }
                linkNode[0].y = linkNode[0].rowEL!.translateY! + linkNode[0].offsetY;
                if (linkNode[1].rowEL.collect) {
                  linkNode[1].rowEL.translateY = linkNode[1].rowEL.getBoundingClientRect().top - 195;
                } else {
                  linkNode[1].rowEL.translateY = linkNode[1].rowEL.offsetTop - this.rowsPaneEL!.scrollTop;
                }
                linkNode[1].y = linkNode[1].rowEL!.translateY! + linkNode[1].offsetY;
              });
            }, 300);
          }
          let refreshTimeOut = setTimeout(() => {
            this.refreshCanvas(true);
            clearTimeout(refreshTimeOut);
          }, 360);
        });
      }
      if (this.loadTraceCompleted) {
        this.traceSheetEL?.displaySystemLogsData();
      }
      this.intersectionObserver?.observe(it);
    });
    return { status: true, msg: 'success' };
  };

  private extracted(it: TraceRow<any>) {
    return () => {
      if (it.hasAttribute('expansion')) {
        it.childrenList.forEach((child) => {
          if (child.hasAttribute('scene') && !child.collect) {
            child.rowHidden = false;
          }
          if (child.folder) {
            child.addEventListener('expansion-change', this.extracted(child));
          }
          this.intersectionObserver?.observe(child);
        });
      } else {
        it.childrenList.forEach((child) => {
          if (child.hasAttribute('scene') && !child.collect) {
            child.rowHidden = true;
            this.intersectionObserver?.unobserve(child);
          }
          if (child.folder) {
            child.removeEventListener('expansion-change', this.extracted(child));
          }
        });
        this.linkNodes.map((value) => {
          if ('task' === value[0].business && value[0].rowEL.parentRowEl?.rowId === it.rowId) {
            value[0].hidden = true;
            value[1].hidden = true;
            this.clickEmptyArea();
          }
        });
      }
      if (!this.collapseAll) {
        this.refreshCanvas(false);
      }
    };
  }

  displayTip(row: TraceRow<any>, struct: any, html: string) {
    let x = row.hoverX + 248;
    let y = row.getBoundingClientRect().top - this.getBoundingClientRect().top;
    if ((struct === undefined || struct === null) && this.tipEL) {
      this.tipEL.style.display = 'none';
      return;
    }
    if (this.tipEL) {
      this.tipEL.innerHTML = html;
      if (row.rowType === TraceRow.ROW_TYPE_JS_CPU_PROFILER || row.rowType === TraceRow.ROW_TYPE_PERF_CALLCHART) {
        this.tipEL.style.maxWidth = row.clientWidth / 3 + 'px';
        this.tipEL.style.wordBreak = ' break-all';
        this.tipEL.style.height = 'unset';
        this.tipEL.style.display = 'block';
        y = y + struct.depth * 20;
      } else {
        this.tipEL.style.display = 'flex';
        this.tipEL.style.height = row.style.height;
      }
      if (x + this.tipEL.clientWidth > (this.canvasPanel!.clientWidth ?? 0)) {
        this.tipEL.style.transform = `translateX(${x - this.tipEL.clientWidth - 1}px) translateY(${y}px)`;
      } else {
        this.tipEL.style.transform = `translateX(${x}px) translateY(${y}px)`;
      }
    }
  }

  queryCPUWakeUpList(data: WakeupBean) {
    TabPaneCurrentSelection.queryCPUWakeUpListFromBean(data).then((a: any) => {
      if (a === null) {
        window.publish(window.SmartEvent.UI.WakeupList, SpSystemTrace.wakeupList);
        return null;
      }
      SpSystemTrace.wakeupList.push(a);
      this.queryCPUWakeUpList(a);
    });
  }

  wakeupListNull() {
    SpSystemTrace.wakeupList = [];
  }

  initPointToEvent() {
    this.eventMap = {
      'cpu-data': 'Cpu',
      'cpu-state': 'Cpu State',
      'cpu-freq': 'Cpu Frequency',
      'cpu-limit-freq': 'Cpu Freq Limit',
      process: 'Process',
      'native-memory': 'Native Memory',
      thread: 'Thread',
      func: 'Func',
      mem: 'Memory',
      'virtual-memory-cell': 'Virtual Memory',
      'virtual-memory-group': 'Virtual Memory',
      fps: 'FPS',
      'ability-monitor': 'Ability Monitor',
      'cpu-ability': 'Cpu Ability',
      'memory-ability': 'Memory Ability',
      'disk-ability': 'DiskIO Ability',
      'network-ability': 'Network Ability',
      sdk: 'Sdk',
      'sdk-counter': 'SDK Counter',
      'sdk-slice': 'Sdk Slice',
      energy: 'Energy',
      'power-energy': 'Power Event',
      'system-energy': 'System Event',
      'anomaly-energy': 'Anomaly Event',
      'clock-group': 'Clocks',
      clock: 'clock',
      'irq-group': 'Irqs',
      irq: 'irq',
      hiperf: 'HiPerf (All)',
      'hiperf-event': 'HiPerf Event',
      'hiperf-report': 'HiPerf Report',
      'hiperf-process': 'HiPerf Process',
      'hiperf-thread': 'HiPerf Thread',
      'js-memory': 'Js Memory',
    };
  }

  initHtml(): string {
    return `
        <style>
        :host{
            display: block;
            width: 100%;
            height: 100%;
        }
        .timer-shaft{
            width: 100%;
            z-index: 2;
        }
        .rows-pane{
            overflow: overlay;
            overflow-anchor: none;
            flex: 1;
            /*height: 100%;*/
            max-height: calc(100vh - 147px - 48px);
        }
        .rows{
            color: #fff;
            display: flex;
            box-sizing: border-box;
            flex-direction: column;
            overflow-y: auto;
            flex: 1;
            width: 100%;
            background: var(--dark-background4,#ffffff);
            /*scroll-behavior: smooth;*/
        }
        :host([disable]) .container{
            pointer-events: none;
        }
        .container{
            width: 100%;
            box-sizing: border-box;
            height: 100%;
            display: flex;
            flex-direction: column;
            /*grid-template-areas:    'head'*/
                                    /*'body'*/
                                    /*'sheet';*/
            position:relative;
        }
        .panel-canvas{
            position: absolute;
            top: 0;
            right: 0px;
            bottom: 0px;
            width: 100%;
            /*height: calc(100vh - 195px);*/
            height: 100%;
            box-sizing: border-box;
            /*background: #f0f0f0;*/
            /*border: 1px solid #000000;*/
            z-index: 0;
        }
        .panel-canvas-favorite{
            width: 100% ;
            display: block;
            position: absolute;
            height: 0;
            top: 0;
            right: 0;
            box-sizing: border-box;
            z-index: 100;
        }
        .trace-sheet{
            cursor: default;
        }
        .tip{
            z-index: 1001;
            position: absolute;
            top: 0;
            left: 0;
            /*height: 100%;*/
            background-color: white;
            border: 1px solid #f9f9f9;
            width: auto;
            font-size: 8px;
            color: #50809e;
            flex-direction: column;
            justify-content: center;
            align-items: flex-start;
            padding: 2px 10px;
            box-sizing: border-box;
            display: none;
            user-select: none;
        }

        </style>
        <div class="container">
            <timer-shaft-element class="timer-shaft" style="position: relative;top: 0"></timer-shaft-element>
            <sp-chart-list id="favorite-chart-list"></sp-chart-list>
            <div class="rows-pane" style="position: relative;flex-direction: column;overflow-x: hidden;">
                <canvas id="canvas-panel" class="panel-canvas" ondragstart="return false"></canvas>
                <div class="spacer" ondragstart="return false"></div>
                <div class="rows" ondragstart="return false"></div>
            </div>
            <div id="tip" class="tip"></div>
            <trace-sheet class="trace-sheet" mode="hidden" ondragstart="return false"></trace-sheet>
        </div>
        `;
  }
}
