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
  drawWakeUp,
  drawWakeUpList,
  isFrameContainPoint,
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
import { setSelectState, intersectData } from './Utils.js';

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
  static mouseCurrentPosition = 0;
  static offsetMouse = 0;
  static moveable = true;
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
  spacerEL: HTMLDivElement | undefined | null;
  favoriteRowsEL: HTMLDivElement | undefined | null;
  visibleRows: Array<TraceRow<any>> = [];
  collectRows: Array<TraceRow<any>> = [];
  currentRow: TraceRow<any> | undefined | null;
  keyboardEnable = true;
  mouseEventEnable = true;
  currentRowType = ''; /*保存当前鼠标所在行的类型*/
  observerScrollHeightEnable: boolean = false;
  observerScrollHeightCallback: Function | undefined;
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
  private rangeSelect!: RangeSelect;
  chartManager: SpChartManager | undefined | null;
  private loadTraceCompleted: boolean = false;
  private rangeTraceRow: Array<TraceRow<any>> | undefined = [];
  canvasFavoritePanel: HTMLCanvasElement | null | undefined; //绘制收藏泳道图
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

  set snapshotFile(data: FileInfo) {
    this.snapshotFiles = data;
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

  initElements(): void {
    let sideColor =
      document!.querySelector('body > sp-application')?.shadowRoot?.querySelector!(
        '#main-menu'
      )?.shadowRoot?.querySelector('div.bottom > div.color');
    this.traceSheetEL = this.shadowRoot?.querySelector('.trace-sheet');
    let rightButton: HTMLElement | null | undefined = this.traceSheetEL?.shadowRoot
      ?.querySelector('#current-selection > tabpane-current-selection')
      ?.shadowRoot?.querySelector('#rightButton');
    let rightStar: HTMLElement | null | undefined = this.traceSheetEL?.shadowRoot
      ?.querySelector('#current-selection > tabpane-current-selection')
      ?.shadowRoot?.querySelector('#right-star');
    this.rowsEL = this.shadowRoot?.querySelector<HTMLDivElement>('.rows');
    this.tipEL = this.shadowRoot?.querySelector<HTMLDivElement>('.tip');
    this.rowsPaneEL = this.shadowRoot?.querySelector<HTMLDivElement>('.rows-pane');
    this.spacerEL = this.shadowRoot?.querySelector<HTMLDivElement>('.spacer');
    this.canvasFavoritePanel = this.shadowRoot?.querySelector<HTMLCanvasElement>('.panel-canvas-favorite');
    this.timerShaftEL = this.shadowRoot?.querySelector('.timer-shaft');
    this.favoriteRowsEL = this.shadowRoot?.querySelector('.favorite-rows');
    this.rangeSelect = new RangeSelect(this);
    rightButton?.addEventListener('click', (event: any) => {
      if (SpSystemTrace.btnTimer) {
        return;
      }
      this.wakeupListNull();
      SpSystemTrace.wakeupList.unshift(CpuStruct.wakeupBean!);
      this.queryCPUWakeUpList(CpuStruct.wakeupBean!);
      setTimeout(() => {
        requestAnimationFrame(() => this.refreshCanvas(false));
      }, 300);
      rightStar!.style.visibility = 'visible';
      rightStar!.style.cursor = 'pointer';
      SpSystemTrace.btnTimer = setTimeout(() => {
        SpSystemTrace.btnTimer = null; // 2.清空节流阀，方便下次开启定时器
      }, 2000);
    });
    rightStar?.addEventListener('click', () => {
      let wakeupLists = [];
      for (let i = 0; i < SpSystemTrace.wakeupList.length; i++) {
        wakeupLists.unshift(CpuStruct.selectCpuStruct?.cpu);
        wakeupLists.push(SpSystemTrace.wakeupList[i].cpu);
      }
      let wakeupCpuLists = Array.from(new Set(wakeupLists)).sort();
      for (let i = 0; i < wakeupCpuLists.length; i++) {
        let cpuFavoriteRow: any = this.shadowRoot?.querySelector<TraceRow<any>>(
          `trace-row[row-type='cpu-data'][row-id='${wakeupCpuLists[i]}']`
        );
        cpuFavoriteRow!.setAttribute('collect-type', '');
        let replaceRow = document.createElement('div');
        replaceRow.setAttribute('row-id', cpuFavoriteRow.rowId + '-' + cpuFavoriteRow.rowType);
        replaceRow.setAttribute('type', 'replaceRow');
        replaceRow.setAttribute('row-parent-id', cpuFavoriteRow.rowParentId);
        replaceRow.style.display = 'none';
        cpuFavoriteRow.rowHidden = !cpuFavoriteRow.hasAttribute('scene');
        if (this.rowsEL!.contains(cpuFavoriteRow)) {
          this.rowsEL!.replaceChild(replaceRow, cpuFavoriteRow);
        }
        this.favoriteRowsEL!.append(cpuFavoriteRow);
        this.currentClickRow = null;
        cpuFavoriteRow.setAttribute('draggable', 'true');
        cpuFavoriteRow.addEventListener('dragstart', () => {
          this.currentClickRow = cpuFavoriteRow;
        });
        cpuFavoriteRow.addEventListener('dragover', (ev: any) => {
          ev.preventDefault();
          ev.dataTransfer.dropEffect = 'move';
        });
        cpuFavoriteRow.addEventListener('drop', (ev: any) => {
          if (this.favoriteRowsEL != null && this.currentClickRow != null && this.currentClickRow !== cpuFavoriteRow) {
            let rect = cpuFavoriteRow.getBoundingClientRect();
            if (ev.clientY >= rect.top && ev.clientY < rect.top + rect.height / 2) {
              //向上移动
              this.favoriteRowsEL.insertBefore(this.currentClickRow, cpuFavoriteRow);
            } else if (ev.clientY <= rect.bottom && ev.clientY > rect.top + rect.height / 2) {
              //向下移动
              this.favoriteRowsEL.insertBefore(this.currentClickRow, cpuFavoriteRow.nextSibling);
            }
            this.refreshFavoriteCanvas();
          }
        });
        cpuFavoriteRow.addEventListener('dragend', () => {
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
          this.currentClickRow = null;
        });
      }
      this.refreshFavoriteCanvas();
      this.refreshCanvas(true);
    });
    sideColor?.addEventListener('click', (event: any) => {
      requestAnimationFrame(() => this.refreshCanvas(true));
    });
    document?.addEventListener('triangle-flag', (event: any) => {
      let temporaryTime = this.timerShaftEL?.drawTriangle(event.detail.time, event.detail.type);
      if (event.detail.timeCallback && temporaryTime) event.detail.timeCallback(temporaryTime);
    });
    document?.addEventListener('flag-change', (event: any) => {
      this.timerShaftEL?.modifyFlagList(event.detail);
      if (event.detail.hidden) {
        this.selectFlag = undefined;
        this.traceSheetEL?.setAttribute('mode', 'hidden');
        this.refreshCanvas(true);
      }
    });
    document?.addEventListener('slices-change', (event: any) => {
      this.timerShaftEL?.modifySlicesList(event.detail);
      if (event.detail.hidden) {
        this.slicestime = null;
        this.traceSheetEL?.setAttribute('mode', 'hidden');
        this.refreshCanvas(true);
      }
    });
    if (this.timerShaftEL?.collecBtn) {
      this.timerShaftEL.collecBtn.onclick = () => {
        if (this.timerShaftEL!.collecBtn!.hasAttribute('close')) {
          this.timerShaftEL!.collecBtn!.removeAttribute('close');
        } else {
          this.timerShaftEL!.collecBtn!.setAttribute('close', '');
        }
        if (this.collectRows.length > 0) {
          this.collectRows.forEach((row) => {
            row?.collectEL?.onclick?.(new MouseEvent('auto-collect', undefined));
          });
        }
      };
    }
    document?.addEventListener('collect', (event: any) => {
      let currentRow = event.detail.row;
      if (currentRow.collect) {
        if (
          !this.collectRows.find((find) => {
            return find === currentRow;
          })
        ) {
          this.collectRows.push(currentRow);
        }
        if (event.detail.type !== 'auto-collect' && this.timerShaftEL!.collecBtn!.hasAttribute('close')) {
          currentRow.collect = false;
          this.timerShaftEL!.collecBtn!.click();
          return;
        }
        let replaceRow = document.createElement('div');
        replaceRow.setAttribute('row-id', currentRow.rowId + '-' + currentRow.rowType);
        replaceRow.setAttribute('type', 'replaceRow');
        replaceRow.setAttribute('row-parent-id', currentRow.rowParentId);
        replaceRow.style.display = 'none';
        currentRow.rowHidden = !currentRow.hasAttribute('scene');
        // 添加收藏时，在线程名前面追加父亲ID
        let rowParentId = currentRow.rowParentId;
        if (rowParentId) {
          let parentRows = this.shadowRoot?.querySelectorAll<TraceRow<any>>(`trace-row[row-id='${rowParentId}']`);
          parentRows?.forEach((parentRow) => {
            if (
              parentRow?.name &&
              parentRow?.name != currentRow.name &&
              !parentRow.rowType!.startsWith('cpu') &&
              !parentRow.rowType!.startsWith('thread') &&
              !parentRow.rowType!.startsWith('func')
            ) {
              currentRow.name += '(' + parentRow.name + ')';
            }
          });
        }
        if (this.rowsEL!.contains(currentRow)) {
          this.rowsEL!.replaceChild(replaceRow, currentRow);
        } else {
          if (currentRow.hasParentRowEl) {
            let parent = currentRow.parentRowEl;
            parent!.replaceTraceRow(replaceRow, currentRow);
          }
        }
        this.favoriteRowsEL!.append(currentRow);
      } else {
        this.favoriteRowsEL!.removeChild(currentRow);
        if (event.detail.type !== 'auto-collect') {
          let rowIndex = this.collectRows.indexOf(currentRow);
          if (rowIndex !== -1) {
            this.collectRows.splice(rowIndex, 1);
          }
        }
        let row = currentRow;
        let allowExpansionRow = [];
        while (row.hasParentRowEl) {
          let parent = row.parentRowEl;
          allowExpansionRow.push(parent);
          row = parent;
        }
        for (let index: number = allowExpansionRow.length - 1; index >= 0; index--) {
          if (!allowExpansionRow[index]?.expansion && allowExpansionRow[index]?.hasAttribute('scene')) {
            allowExpansionRow[index].expansion = true;
          }
        }
        allowExpansionRow.length = 0;
        let replaceRow = this.rowsEL!.querySelector<HTMLCanvasElement>(
          `div[row-id='${currentRow.rowId}-${currentRow.rowType}']`
        );
        if (replaceRow != null) {
          // 取消收藏时，删除父亲ID
          let rowNameArr = currentRow.name.split('(');
          if (rowNameArr.length > 1) {
            let tempName = '';
            tempName += rowNameArr[0];
            currentRow.name = tempName;
          } else {
            currentRow.name = rowNameArr[0];
          }
          this.rowsEL!.replaceChild(currentRow, replaceRow);
          currentRow.style.boxShadow = `0 10px 10px #00000000`;
        }
        this.canvasFavoritePanel!.style.transform = `translateY(${
          this.favoriteRowsEL!.scrollTop - currentRow.clientHeight
        }px)`;
      }
      this.timerShaftEL?.displayCollect(this.collectRows.length !== 0);
      this.refreshFavoriteCanvas();
      this.refreshCanvas(true);
      this.linkNodes.forEach((itln) => {
        if (itln[0].rowEL === currentRow) {
          if (itln[0].rowEL.collect) {
            itln[0].rowEL.translateY = itln[0].rowEL.getBoundingClientRect().top - 195;
          } else {
            itln[0].rowEL.translateY = itln[0].rowEL.offsetTop - this.rowsPaneEL!.scrollTop;
          }
          itln[0].y = itln[0].rowEL.translateY + itln[0].offsetY;
        } else if (itln[1].rowEL === currentRow) {
          if (itln[1].rowEL.collect) {
            itln[1].rowEL.translateY = itln[1].rowEL.getBoundingClientRect().top - 195;
          } else {
            itln[1].rowEL.translateY = itln[1].rowEL.offsetTop - this.rowsPaneEL!.scrollTop;
          }
          itln[1].y = itln[1].rowEL.translateY + itln[1].offsetY;
        }
      });
      // 收藏夹元素拖动排序功能
      this.currentClickRow = null;
      currentRow.setAttribute('draggable', 'true');
      currentRow.addEventListener('dragstart', () => {
        this.currentClickRow = currentRow;
      });
      currentRow.addEventListener('dragover', (ev: any) => {
        ev.preventDefault();
        ev.dataTransfer.dropEffect = 'move';
      });
      currentRow.addEventListener('drop', (ev: any) => {
        if (this.favoriteRowsEL != null && this.currentClickRow != null && this.currentClickRow !== currentRow) {
          let rect = currentRow.getBoundingClientRect();
          if (ev.clientY >= rect.top && ev.clientY < rect.top + rect.height / 2) {
            //向上移动
            this.favoriteRowsEL.insertBefore(this.currentClickRow, currentRow);
          } else if (ev.clientY <= rect.bottom && ev.clientY > rect.top + rect.height / 2) {
            //向下移动
            this.favoriteRowsEL.insertBefore(this.currentClickRow, currentRow.nextSibling);
          }
          this.refreshFavoriteCanvas();
        }
      });
      currentRow.addEventListener('dragend', () => {
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
        this.currentClickRow = null;
      });
    });
    SpSystemTrace.scrollViewWidth = this.getScrollWidth();
    this.rangeSelect.selectHandler = (rows, refreshCheckBox) => {
      rows.forEach((item) => {
        this.setAttribute('clickRow', item.rowType!);
        this.setAttribute('rowName', item.name);
        this.setAttribute('rowId', item.rowId!);
      });
      if (rows.length == 0) {
        this.shadowRoot!.querySelectorAll<TraceRow<any>>('trace-row').forEach((it) => {
          it.checkType = '-1';
          if (it.folder) {
            it.childrenList.forEach((item) => {
              it.checkType = '-1';
            });
          }
        });
        this.refreshCanvas(true);
        this.traceSheetEL?.setAttribute('mode', 'hidden');
        return;
      }
      if (refreshCheckBox) {
        if (rows.length > 0) {
          this.shadowRoot?.querySelectorAll<TraceRow<any>>('trace-row').forEach((row) => {
            row.checkType = '0';
            if (row.folder) {
              row.childrenList.forEach((ite) => {
                ite.checkType = '0';
              });
            }
          });
          rows.forEach((it) => {
            it.checkType = '2';
          });
        } else {
          this.shadowRoot?.querySelectorAll<TraceRow<any>>('trace-row').forEach((row) => {
            row.checkType = '-1';
            if (row.folder) {
              row.childrenList.forEach((it) => {
                it.checkType = '-1';
              });
            }
          });
          return;
        }
      }
      if (!this.isSelectClick) {
        this.rangeTraceRow = [];
      }
      let selection = new SelectionParam();
      selection.leftNs = TraceRow.rangeSelectObject?.startNS || 0;
      selection.rightNs = TraceRow.rangeSelectObject?.endNS || 0;
      selection.recordStartNs = (window as any).recordStartNS;
      let native_memory = ['All Heap & Anonymous VM', 'All Heap', 'All Anonymous VM'];
      rows.forEach((it) => {
        if (it.rowType == TraceRow.ROW_TYPE_CPU) {
          selection.cpus.push(parseInt(it.rowId!));
          info('load CPU traceRow id is : ', it.rowId);
        } else if (it.rowType == TraceRow.ROW_TYPE_CPU_STATE) {
          let filterId = parseInt(it.rowId!);
          if (selection.cpuStateFilterIds.indexOf(filterId) == -1) {
            selection.cpuStateFilterIds.push(filterId);
          }
        } else if (it.rowType == TraceRow.ROW_TYPE_CPU_FREQ) {
          let filterId = parseInt(it.rowId!);
          if (selection.cpuFreqFilterIds.indexOf(filterId) == -1) {
            selection.cpuFreqFilterIds.push(filterId);
          }
        } else if (it.rowType == TraceRow.ROW_TYPE_CPU_FREQ_LIMIT) {
          selection.cpuFreqLimitDatas.push(it.dataList!);
        } else if (it.rowType == TraceRow.ROW_TYPE_PROCESS) {
          this.pushPidToSelection(selection, it.rowId!);
          if (it.getAttribute('hasStartup') === 'true') {
            selection.startup = true;
          }
          if (it.getAttribute('hasStaticInit') === 'true') {
            selection.staticInit = true;
          }
          let processChildRows: Array<TraceRow<any>> = [
            ...this.shadowRoot!.querySelectorAll<TraceRow<any>>(`trace-row[row-parent-id='${it.rowId}']`),
          ];
          if (!it.expansion) {
            processChildRows = [...it.childrenList];
          }
          selection.processIds.push(parseInt(it.rowId!));
          processChildRows.forEach((th) => {
            th.rangeSelect = true;
            th.checkType = '2';
            if (th.rowType == TraceRow.ROW_TYPE_THREAD) {
              selection.threadIds.push(parseInt(th.rowId!));
            } else if (th.rowType == TraceRow.ROW_TYPE_FUNC) {
              if (th.asyncFuncName) {
                selection.funAsync.push({
                  name: th.asyncFuncName,
                  pid: th.asyncFuncNamePID || 0,
                });
              } else {
                selection.funTids.push(parseInt(th.rowId!));
              }
            } else if (th.rowType == TraceRow.ROW_TYPE_MEM) {
              selection.processTrackIds.push(parseInt(th.rowId!));
            }
          });
          info('load process traceRow id is : ', it.rowId);
        } else if (it.rowType == TraceRow.ROW_TYPE_NATIVE_MEMORY) {
          let memoryRows: Array<TraceRow<any>> = [
            ...this.shadowRoot!.querySelectorAll<TraceRow<any>>(`trace-row[row-parent-id='${it.rowId}']`),
          ];
          if (!it.expansion) {
            memoryRows = [...it.childrenList];
          }
          memoryRows.forEach((th) => {
            th.rangeSelect = true;
            th.checkType = '2';
            if (th.getAttribute('heap-type') === 'native_hook_statistic') {
              selection.nativeMemoryStatistic.push(th.rowId!);
            } else {
              selection.nativeMemory.push(th.rowId!);
            }
          });
          info('load nativeMemory traceRow id is : ', it.rowId);
        } else if (it.rowType == TraceRow.ROW_TYPE_STATIC_INIT) {
          selection.staticInit = true;
          this.pushPidToSelection(selection, it.rowParentId!);
          info('load thread traceRow id is : ', it.rowId);
        } else if (it.rowType == TraceRow.ROW_TYPE_APP_STARTUP) {
          selection.startup = true;
          this.pushPidToSelection(selection, it.rowParentId!);
          info('load thread traceRow id is : ', it.rowId);
        } else if (it.rowType == TraceRow.ROW_TYPE_THREAD) {
          this.pushPidToSelection(selection, it.rowParentId!);
          selection.threadIds.push(parseInt(it.rowId!));
          info('load thread traceRow id is : ', it.rowId);
        } else if (it.rowType == TraceRow.ROW_TYPE_FUNC) {
          TabPaneTaskFrames.TaskArray = [];
          this.pushPidToSelection(selection, it.rowParentId!);
          if (it.asyncFuncName) {
            selection.funAsync.push({
              name: it.asyncFuncName,
              pid: it.asyncFuncNamePID || 0,
            });
          } else {
            selection.funTids.push(parseInt(it.rowId!));
          }

          let isIntersect = (filterFunc: FuncStruct, rangeData: RangeSelectStruct) =>
            Math.max(filterFunc.startTs! + filterFunc.dur!, rangeData!.endNS || 0) - Math.min(filterFunc.startTs!, rangeData!.startNS || 0) <
              filterFunc.dur! + (rangeData!.endNS || 0) - (rangeData!.startNS || 0) && filterFunc.funName!.indexOf('H:Task ') >= 0;
          let taskData = it.dataList.filter((taskData: FuncStruct) => {
            taskData!.tid = parseInt(it.rowId!);
            return isIntersect(taskData, TraceRow.rangeSelectObject!);
          });
          if (taskData.length > 0) {
            selection.taskFramesData.push(...taskData);
          }
          info('load func traceRow id is : ', it.rowId);
        } else if (it.rowType == TraceRow.ROW_TYPE_MEM || it.rowType == TraceRow.ROW_TYPE_VIRTUAL_MEMORY) {
          if (it.rowType == TraceRow.ROW_TYPE_MEM) {
            selection.processTrackIds.push(parseInt(it.rowId!));
          } else {
            selection.virtualTrackIds.push(parseInt(it.rowId!));
          }
          info('load memory traceRow id is : ', it.rowId);
        } else if (it.rowType == TraceRow.ROW_TYPE_FPS) {
          selection.hasFps = true;
          info('load FPS traceRow id is : ', it.rowId);
        } else if (it.rowType == TraceRow.ROW_TYPE_HEAP) {
          if (it.getAttribute('heap-type') === 'native_hook_statistic') {
            selection.nativeMemoryStatistic.push(it.rowId!);
          } else {
            selection.nativeMemory.push(it.rowId!);
          }
          info('load nativeMemory traceRow id is : ', it.rowId);
        } else if (it.rowType == TraceRow.ROW_TYPE_MONITOR) {
          let abilityChildRows: Array<TraceRow<any>> = [
            ...this.shadowRoot!.querySelectorAll<TraceRow<any>>(`trace-row[row-parent-id='${it.rowId}']`),
          ];
          if (!it.expansion) {
            abilityChildRows = [...it.childrenList];
          }
          abilityChildRows.forEach((th) => {
            th.rangeSelect = true;
            th.checkType = '2';
            if (th.rowType == TraceRow.ROW_TYPE_CPU_ABILITY) {
              selection.cpuAbilityIds.push(th.rowId!);
            } else if (th.rowType == TraceRow.ROW_TYPE_MEMORY_ABILITY) {
              selection.memoryAbilityIds.push(th.rowId!);
            } else if (th.rowType == TraceRow.ROW_TYPE_DISK_ABILITY) {
              selection.diskAbilityIds.push(th.rowId!);
            } else if (th.rowType == TraceRow.ROW_TYPE_NETWORK_ABILITY) {
              selection.networkAbilityIds.push(th.rowId!);
            } else if (th.rowType == TraceRow.ROW_TYPE_DMA_ABILITY) {
              selection.dmaAbilityData.push(...intersectData(th)!);
            } else if (th.rowType == TraceRow.ROW_TYPE_GPU_MEMORY_ABILITY) {
              selection.gpuMemoryAbilityData.push(...intersectData(th)!);
            } else if (th.rowType === TraceRow.ROW_TYPE_PURGEABLE_TOTAL_ABILITY) {
              selection.purgeableTotalAbility.push(...intersectData(th));
            } else if (th.rowType === TraceRow.ROW_TYPE_PURGEABLE_PIN_ABILITY) {
              selection.purgeablePinAbility.push(...intersectData(th));
            }
          });
        } else if (it.rowType == TraceRow.ROW_TYPE_CPU_ABILITY) {
          selection.cpuAbilityIds.push(it.rowId!);
          info('load CPU Ability traceRow id is : ', it.rowId);
        } else if (it.rowType == TraceRow.ROW_TYPE_MEMORY_ABILITY) {
          selection.memoryAbilityIds.push(it.rowId!);
          info('load Memory Ability traceRow id is : ', it.rowId);
        } else if (it.rowType == TraceRow.ROW_TYPE_DISK_ABILITY) {
          selection.diskAbilityIds.push(it.rowId!);
          info('load DiskIo Ability traceRow id is : ', it.rowId);
        } else if (it.rowType == TraceRow.ROW_TYPE_NETWORK_ABILITY) {
          selection.networkAbilityIds.push(it.rowId!);
          info('load Network Ability traceRow id is : ', it.rowId);
        } else if (it.rowType == TraceRow.ROW_TYPE_DMA_ABILITY) {
          selection.dmaAbilityData.push(...intersectData(it)!);
        } else if (it.rowType == TraceRow.ROW_TYPE_GPU_MEMORY_ABILITY) {
          selection.gpuMemoryAbilityData.push(...intersectData(it)!);
        } else if (it.rowType?.startsWith(TraceRow.ROW_TYPE_SDK)) {
          if (it.rowType == TraceRow.ROW_TYPE_SDK) {
            let sdkRows: Array<TraceRow<any>> = [
              ...this.shadowRoot!.querySelectorAll<TraceRow<any>>(`trace-row[row-parent-id='${it.rowId}']`),
            ];
            if (!it.expansion) {
              sdkRows = [...it.childrenList];
            }
            sdkRows.forEach((th) => {
              th.rangeSelect = true;
              th.checkType = '2';
            });
          }
          if (it.rowType == TraceRow.ROW_TYPE_SDK_COUNTER) {
            selection.sdkCounterIds.push(it.rowId!);
          }
          if (it.rowType == TraceRow.ROW_TYPE_SDK_SLICE) {
            selection.sdkSliceIds.push(it.rowId!);
          }
        } else if (it.rowType?.startsWith('hiperf')) {
          if (it.rowType == TraceRow.ROW_TYPE_HIPERF_EVENT || it.rowType == TraceRow.ROW_TYPE_HIPERF_REPORT) {
            return;
          }
          selection.perfSampleIds.push(1);
          if (it.rowType == TraceRow.ROW_TYPE_HIPERF_PROCESS) {
            let hiperfProcessRows: Array<TraceRow<any>> = [
              ...this.shadowRoot!.querySelectorAll<TraceRow<any>>(`trace-row[row-parent-id='${it.rowId}']`),
            ];
            if (!it.expansion) {
              hiperfProcessRows = [...it.childrenList];
            }
            hiperfProcessRows.forEach((th) => {
              th.rangeSelect = true;
              th.checkType = '2';
            });
          }
          if (it.rowType == TraceRow.ROW_TYPE_HIPERF || it.rowId == 'HiPerf-cpu-merge') {
            selection.perfAll = true;
          }
          if (it.rowType == TraceRow.ROW_TYPE_HIPERF_CPU) {
            selection.perfCpus.push(it.index);
          }
          if (it.rowType == TraceRow.ROW_TYPE_HIPERF_PROCESS) {
            selection.perfProcess.push(parseInt(it.rowId!.split('-')[0]));
          }
          if (it.rowType == TraceRow.ROW_TYPE_HIPERF_THREAD) {
            selection.perfThread.push(parseInt(it.rowId!.split('-')[0]));
          }
        } else if (it.rowType == TraceRow.ROW_TYPE_FILE_SYSTEM) {
          if (it.rowId == 'FileSystemLogicalWrite') {
            if (selection.fileSystemType.length == 0) {
              selection.fileSystemType = [0, 1, 3];
            } else {
              if (selection.fileSystemType.indexOf(3) == -1) {
                selection.fileSystemType.push(3);
              }
            }
          } else if (it.rowId == 'FileSystemLogicalRead') {
            if (selection.fileSystemType.length == 0) {
              selection.fileSystemType = [0, 1, 2];
            } else {
              if (selection.fileSystemType.indexOf(2) == -1) {
                selection.fileSystemType.push(2);
              }
            }
          } else if (it.rowId == 'FileSystemVirtualMemory') {
            selection.fileSysVirtualMemory = true;
          } else if (it.rowId == 'FileSystemDiskIOLatency') {
            selection.diskIOLatency = true;
          } else {
            if (!selection.diskIOLatency) {
              let arr = it.rowId!.split('-').reverse();
              let ipid = parseInt(arr[0]);
              if (selection.diskIOipids.indexOf(ipid) == -1) {
                selection.diskIOipids.push(ipid);
              }
              if (arr[1] == 'read') {
                selection.diskIOReadIds.indexOf(ipid) == -1 ? selection.diskIOReadIds.push(ipid) : '';
              } else if (arr[1] == 'write') {
                selection.diskIOWriteIds.indexOf(ipid) == -1 ? selection.diskIOWriteIds.push(ipid) : '';
              }
            }
          }
        } else if (it.rowType == TraceRow.ROW_TYPE_POWER_ENERGY) {
          selection.powerEnergy.push(it.rowId!);
        } else if (it.rowType == TraceRow.ROW_TYPE_SYSTEM_ENERGY) {
          selection.systemEnergy.push(it.rowId!);
        } else if (it.rowType == TraceRow.ROW_TYPE_ANOMALY_ENERGY) {
          selection.anomalyEnergy.push(it.rowId!);
        } else if (it.rowType == TraceRow.ROW_TYPE_SYSTEM_ENERGY) {
          info('load anomaly Energy traceRow id is : ', it.rowId);
        } else if (it.rowType == TraceRow.ROW_TYPE_VM_TRACKER_SMAPS) {
          selection.smapsType.push(...intersectData(it)!);
          let sMapsChildRows: Array<TraceRow<any>> = [
            ...this.shadowRoot!.querySelectorAll<TraceRow<any>>(`trace-row[row-parent-id='${it.rowId}']`),
          ];
          if (!it.expansion) {
            sMapsChildRows = [...it.childrenList];
          }
          sMapsChildRows.forEach((item) => {
            item.rangeSelect = true;
            if (item.rowType == TraceRow.ROW_TYPE_VM_TRACKER_SMAPS) {
              selection.smapsType.push(...intersectData(item)!);
            }
          });
        } else if (it.rowType == TraceRow.ROW_TYPE_VMTRACKER_SHM) {
          selection.vmtrackershm.push(...intersectData(it)!);
        } else if (it.rowType == TraceRow.ROW_TYPE_CLOCK) {
          selection.clockMapData.set(
            it.rowId || '',
            it.dataList.filter((clockData) => {
              return Utils.getTimeIsCross(
                clockData.startNS,
                clockData.startNS + clockData.dur,
                TraceRow.rangeSelectObject?.startNS || 0,
                TraceRow.rangeSelectObject?.endNS || 0
              );
            })
          );
        } else if (it.rowType == TraceRow.ROW_TYPE_IRQ) {
          it.dataList.forEach((irqData) => {
            if (
              Utils.getTimeIsCross(
                irqData.startNS,
                irqData.startNS + irqData.dur,
                TraceRow.rangeSelectObject?.startNS || 0,
                TraceRow.rangeSelectObject?.endNS || 0
              )
            ) {
              if (selection.irqMapData.has(irqData.name)) {
                selection.irqMapData.get(irqData.name)?.push(irqData);
              } else {
                selection.irqMapData.set(irqData.name, [irqData]);
              }
            }
          });
        } else if (it.rowType === TraceRow.ROW_TYPE_VM_TRACKER) {
          let vMTrackerChildRows: Array<TraceRow<any>> = [
            ...this.shadowRoot!.querySelectorAll<TraceRow<any>>(`trace-row[row-parent-id='${it.rowId}']`),
          ];
          if (!it.expansion) {
            vMTrackerChildRows = [...it.childrenList];
          }
          vMTrackerChildRows.forEach((th) => {
            th.rangeSelect = true;
            if (th.rowType === TraceRow.ROW_TYPE_DMA_VMTRACKER) {
              selection.dmaVmTrackerData.push(...intersectData(th)!);
            } else if (th.rowType === TraceRow.ROW_TYPE_SYS_MEMORY_GPU) {
              let vMTrackerGpuChildRows: Array<TraceRow<any>> = [
                ...this.shadowRoot!.querySelectorAll<TraceRow<any>>(`trace-row[row-parent-id='${th.rowId}']`),
              ];
              if (!th.expansion) {
                vMTrackerGpuChildRows = [...th.childrenList];
              }
              vMTrackerGpuChildRows.forEach((item) => {
                item.rangeSelect = true;
                if (item.rowType == TraceRow.ROW_TYPE_GPU_MEMORY_VMTRACKER) {
                  selection.gpuMemoryTrackerData.push(...intersectData(item)!);
                } else if (item.rowType == TraceRow.ROW_TYPE_SYS_MEMORY_GPU_GL) {
                  selection.gpu.gl =
                    item.dataList.filter(
                      (it) =>
                        (it.startNs >= selection.leftNs && it.startNs <= selection.rightNs) ||
                        (it.endNs >= selection.leftNs && it.endNs <= selection.rightNs)
                    ).length > 0;
                } else if (item.rowType == TraceRow.ROW_TYPE_SYS_MEMORY_GPU_TOTAL) {
                  selection.gpu.gpuTotal =
                    item.dataList.filter(
                      (it) =>
                        (it.startNs >= selection.leftNs && it.startNs <= selection.rightNs) ||
                        (it.endNs >= selection.leftNs && it.endNs <= selection.rightNs)
                    ).length > 0;
                } else if (item.rowType == TraceRow.ROW_TYPE_SYS_MEMORY_GPU_WINDOW) {
                  selection.gpu.gpuWindow =
                    item.dataList.filter(
                      (it) =>
                        (it.startNs >= selection.leftNs && it.startNs <= selection.rightNs) ||
                        (it.endNs >= selection.leftNs && it.endNs <= selection.rightNs)
                    ).length > 0;
                }
              });
            } else if (th.rowType === TraceRow.ROW_TYPE_PURGEABLE_TOTAL_VM) {
              selection.purgeableTotalVM.push(...intersectData(th));
            } else if (th.rowType === TraceRow.ROW_TYPE_PURGEABLE_PIN_VM) {
              selection.purgeablePinVM.push(...intersectData(th));
            } else if (th.rowType === TraceRow.ROW_TYPE_VM_TRACKER_SMAPS) {
              let sMapsChildRows: Array<TraceRow<any>> = [
                ...this.shadowRoot!.querySelectorAll<TraceRow<any>>(`trace-row[row-parent-id='${th.rowId}']`),
              ];
              if (!th.expansion) {
                sMapsChildRows = [...th.childrenList];
              }
              sMapsChildRows.forEach((item) => {
                item.rangeSelect = true;
                if (item.rowType == TraceRow.ROW_TYPE_VM_TRACKER_SMAPS) {
                  selection.smapsType.push(...intersectData(item)!);
                }
              });
            } else if (th.rowType == TraceRow.ROW_TYPE_VMTRACKER_SHM) {
              selection.vmtrackershm.push(...intersectData(th)!);
            }
          });
        } else if (it.rowType == TraceRow.ROW_TYPE_SYS_MEMORY_GPU) {
          let vMTrackerGpuChildRows: Array<TraceRow<any>> = [
            ...this.shadowRoot!.querySelectorAll<TraceRow<any>>(`trace-row[row-parent-id='${it.rowId}']`),
          ];
          if (!it.expansion) {
            vMTrackerGpuChildRows = [...it.childrenList];
          }
          vMTrackerGpuChildRows.forEach((th) => {
            th.rangeSelect = true;
            if (th.rowType == TraceRow.ROW_TYPE_GPU_MEMORY_VMTRACKER) {
              selection.gpuMemoryTrackerData.push(...intersectData(th)!);
            } else if (th.rowType == TraceRow.ROW_TYPE_SYS_MEMORY_GPU_GL) {
              selection.gpu.gl =
                th.dataList.filter(
                  (it) =>
                    (it.startNs >= selection.leftNs && it.startNs <= selection.rightNs) ||
                    (it.endNs >= selection.leftNs && it.endNs <= selection.rightNs)
                ).length > 0;
            } else if (th.rowType == TraceRow.ROW_TYPE_SYS_MEMORY_GPU_TOTAL) {
              selection.gpu.gpuTotal =
                th.dataList.filter(
                  (it) =>
                    (it.startNs >= selection.leftNs && it.startNs <= selection.rightNs) ||
                    (it.endNs >= selection.leftNs && it.endNs <= selection.rightNs)
                ).length > 0;
            } else if (th.rowType == TraceRow.ROW_TYPE_SYS_MEMORY_GPU_WINDOW) {
              selection.gpu.gpuWindow =
                th.dataList.filter(
                  (it) =>
                    (it.startNs >= selection.leftNs && it.startNs <= selection.rightNs) ||
                    (it.endNs >= selection.leftNs && it.endNs <= selection.rightNs)
                ).length > 0;
            }
          });
        } else if (it.rowType == TraceRow.ROW_TYPE_GPU_MEMORY_VMTRACKER) {
          selection.gpuMemoryTrackerData.push(...intersectData(it)!);
        } else if (it.rowType == TraceRow.ROW_TYPE_DMA_VMTRACKER) {
          selection.dmaVmTrackerData.push(...intersectData(it)!);
        } else if (it.rowType == TraceRow.ROW_TYPE_SYS_MEMORY_GPU_GL) {
          selection.gpu.gl =
            it.dataList.filter(
              (it) =>
                (it.startNs >= selection.leftNs && it.startNs <= selection.rightNs) ||
                (it.endNs >= selection.leftNs && it.endNs <= selection.rightNs)
            ).length > 0;
        } else if (it.rowType == TraceRow.ROW_TYPE_SYS_MEMORY_GPU_TOTAL) {
          selection.gpu.gpuTotal =
            it.dataList.filter(
              (it) =>
                (it.startNs >= selection.leftNs && it.startNs <= selection.rightNs) ||
                (it.endNs >= selection.leftNs && it.endNs <= selection.rightNs)
            ).length > 0;
        } else if (it.rowType == TraceRow.ROW_TYPE_SYS_MEMORY_GPU_WINDOW) {
          selection.gpu.gpuWindow =
            it.dataList.filter(
              (it) =>
                (it.startNs >= selection.leftNs && it.startNs <= selection.rightNs) ||
                (it.endNs >= selection.leftNs && it.endNs <= selection.rightNs)
            ).length > 0;
        } else if (it.rowType == TraceRow.ROW_TYPE_JANK) {
          let isIntersect = (filterJank: JanksStruct, rangeData: RangeSelectStruct) =>
            Math.max(filterJank.ts! + filterJank.dur!, rangeData!.endNS || 0) - Math.min(filterJank.ts!, rangeData!.startNS || 0) <
            filterJank.dur! + (rangeData!.endNS || 0) - (rangeData!.startNS || 0);
          if (it.name == 'Actual Timeline') {
            selection.jankFramesData = [];
            let jankDatas = it.dataList.filter((jankData: any) => {
              return isIntersect(jankData, TraceRow.rangeSelectObject!);
            });
            selection.jankFramesData.push(jankDatas);
          } else if (it.folder) {
            selection.jankFramesData = [];
            it.childrenList.forEach((child) => {
              if (child.rowType == TraceRow.ROW_TYPE_JANK && child.name == 'Actual Timeline') {
                let jankDatas = child.dataList.filter((jankData: any) => {
                  return isIntersect(jankData, TraceRow.rangeSelectObject!);
                });
                selection.jankFramesData.push(jankDatas);
              }
            });
          }
        } else if (it.rowType == TraceRow.ROW_TYPE_HEAP_TIMELINE) {
          let endNS = TraceRow.rangeSelectObject?.endNS ? TraceRow.rangeSelectObject?.endNS : TraceRow.range?.endNS;
          let startNS = TraceRow.rangeSelectObject?.startNS
            ? TraceRow.rangeSelectObject?.startNS
            : TraceRow.range?.startNS;
          let minNodeId, maxNodeId;
          if (!it.dataList || it.dataList.length === 0) {
            return;
          }
          for (let sample of it.dataList) {
            if (sample.timestamp * 1000 <= startNS!) {
              minNodeId = sample.lastAssignedId;
            }
            if (sample.timestamp * 1000 >= endNS!) {
              if (maxNodeId === undefined) {
                maxNodeId = sample.lastAssignedId;
              }
            }
          }

          // If the start time range of the selected box is greater than the end time of the sampled data
          if (startNS! >= it.dataList[it.dataList.length - 1].timestamp * 1000) {
            minNodeId = it.dataList[it.dataList.length - 1].lastAssignedId;
          }
          // If you select the box from the beginning
          if (startNS! <= TraceRow.range?.startNS!) {
            minNodeId = HeapDataInterface.getInstance().getMinNodeId(this.snapshotFiles!.id);
          }
          //If you select the box from the ending
          if (endNS! >= TraceRow.range?.endNS! || endNS! >= it.dataList[it.dataList.length - 1].timestampUs * 1000) {
            maxNodeId = HeapDataInterface.getInstance().getMaxNodeId(this.snapshotFiles!.id);
          }
          let summary = (this.traceSheetEL?.shadowRoot?.querySelector('#tabs') as LitTabs)
            ?.querySelector('#box-heap-summary')
            ?.querySelector('tabpane-summary') as TabPaneSummary;
          summary.initSummaryData(this.snapshotFiles!, minNodeId, maxNodeId);
          selection.jsMemory.push(1);
        } else if (it.rowType == TraceRow.ROW_TYPE_JS_CPU_PROFILER) {
          let isIntersect = (a: JsCpuProfilerStruct, b: RangeSelectStruct) =>
            Math.max(a.startTime! + a.totalTime!, b!.endNS || 0) - Math.min(a.startTime!, b!.startNS || 0) <
            a.totalTime! + (b!.endNS || 0) - (b!.startNS || 0);
          let frameSelectData = it.dataList.filter((frameSelectData: any) => {
            return isIntersect(frameSelectData, TraceRow.rangeSelectObject!);
          });
          let copyFrameSelectData = JSON.parse(JSON.stringify(frameSelectData));
          let frameSelectDataIdArr: Array<number> = [];
          for (let data of copyFrameSelectData) {
            frameSelectDataIdArr.push(data.id);
          }
          let jsCpuProfilerData = copyFrameSelectData.filter((item: any) => {
            if (item.depth == 0) {
              setSelectState(item, frameSelectDataIdArr);
              return item;
            }
          });
          selection.jsCpuProfilerData = jsCpuProfilerData;
        } else if (it.rowType == TraceRow.ROW_TYPE_FRAME_ANIMATION) {
          let isIntersect = (animationStruct: FrameAnimationStruct, selectStruct: RangeSelectStruct) =>
            Math.max(animationStruct.ts! + animationStruct.dur!, selectStruct!.endNS || 0) -
              Math.min(animationStruct.ts!, selectStruct!.startNS || 0) <
            animationStruct.dur! + (selectStruct!.endNS || 0) - (selectStruct!.startNS || 0);
          let frameAnimationList = it.dataList.filter((frameAnimationBean: FrameAnimationStruct) => {
            return isIntersect(frameAnimationBean, TraceRow.rangeSelectObject!);
          });
          selection.frameAnimation.push(...frameAnimationList);
        } else if (it.rowType == TraceRow.ROW_TYPE_FRAME_DYNAMIC) {
          let appName = it.getAttribute('model-name');
          let isSelect = (dynamicStruct: FrameDynamicStruct, b: RangeSelectStruct) =>
            dynamicStruct.ts >= b.startNS! && dynamicStruct.ts <= b.endNS!;
          let frameDynamicList = it.dataList.filter(
            (frameAnimationBean: FrameDynamicStruct) =>
              isSelect(frameAnimationBean, TraceRow.rangeSelectObject!) &&
              frameAnimationBean.groupId !== -1 &&
              frameAnimationBean.appName === appName
          );
          selection.frameDynamic.push(...frameDynamicList);
        } else if (it.rowType == TraceRow.ROW_TYPE_FRAME_SPACING) {
          let appName = it.getAttribute('model-name');
          let isSelect = (a: FrameSpacingStruct, b: RangeSelectStruct) =>
            a.currentTs >= b.startNS! && a.currentTs <= b.endNS!;
          let frameDatas = it.dataList.filter((frameData: FrameSpacingStruct) => {
            return (
              isSelect(frameData, TraceRow.rangeSelectObject!) &&
              frameData.groupId !== -1 &&
              frameData.frameSpacingResult !== -1 &&
              frameData.nameId === appName
            );
          });
          selection.frameSpacing.push(...frameDatas);
        } else if (it.rowType == TraceRow.ROW_TYPE_PURGEABLE_TOTAL_ABILITY) {
          selection.purgeableTotalAbility.push(...intersectData(it));
        } else if (it.rowType == TraceRow.ROW_TYPE_PURGEABLE_PIN_ABILITY) {
          selection.purgeablePinAbility.push(...intersectData(it));
        } else if (it.rowType == TraceRow.ROW_TYPE_PURGEABLE_TOTAL_VM) {
          selection.purgeableTotalVM.push(...intersectData(it));
        } else if (it.rowType == TraceRow.ROW_TYPE_PURGEABLE_PIN_VM) {
          selection.purgeablePinVM.push(...intersectData(it));
        }
        if (this.rangeTraceRow!.length !== rows.length) {
          let event = this.createPointEvent(it);
          SpStatisticsHttpUtil.addOrdinaryVisitAction({
            action: 'trace_row',
            event: event,
          });
        }
      });
      this.rangeTraceRow = rows;
      this.isSelectClick = false;
      if (selection.diskIOipids.length > 0 && !selection.diskIOLatency) {
        selection.promiseList.push(
          queryEbpfSamplesCount(
            TraceRow.rangeSelectObject?.startNS || 0,
            TraceRow.rangeSelectObject?.endNS || 0,
            selection.diskIOipids
          ).then((res) => {
            if (res.length > 0) {
              selection.fsCount = res[0].fsCount;
              selection.vmCount = res[0].vmCount;
            }
            return new Promise((resolve) => resolve(1));
          })
        );
      }
      this.selectStructNull();
      this.timerShaftEL?.removeTriangle('inverted');
      if (selection.promiseList.length > 0) {
        Promise.all(selection.promiseList).then(() => {
          selection.promiseList = [];
          this.traceSheetEL?.rangeSelect(selection);
        });
      } else {
        this.traceSheetEL?.rangeSelect(selection);
      }
      this.timerShaftEL!.selectionList.push(selection); // 保持选中对象，为后面的再次选中该框选区域做准备。
      this.selectionParam = selection;
    };
    // @ts-ignore
    new ResizeObserver((entries) => {
      TraceRow.FRAME_WIDTH = this.clientWidth - 249 - this.getScrollWidth();
      requestAnimationFrame(() => {
        this.timerShaftEL?.updateWidth(this.clientWidth - 1 - this.getScrollWidth());
        this.shadowRoot!.querySelectorAll<TraceRow<any>>('trace-row').forEach((it) => {
          it.updateWidth(this.clientWidth);
        });
      });
    }).observe(this);

    new ResizeObserver((entries) => {
      this.canvasPanelConfig();
      if (this.traceSheetEL!.getAttribute('mode') == 'hidden') {
        this.timerShaftEL?.removeTriangle('triangle');
      }
      this.refreshFavoriteCanvas();
      this.refreshCanvas(true);
    }).observe(this.rowsPaneEL!);
    new MutationObserver((mutations, observer) => {
      for (const mutation of mutations) {
        if (mutation.type === 'attributes') {
          if (this.style.visibility === 'visible') {
            if (TraceRow.rangeSelectObject && SpSystemTrace.sliceRangeMark) {
              this.timerShaftEL?.setSlicesMark(
                TraceRow.rangeSelectObject.startNS || 0,
                TraceRow.rangeSelectObject.endNS || 0,
                false
              );
              SpSystemTrace.sliceRangeMark = undefined;
              window.publish(window.SmartEvent.UI.RefreshCanvas, {});
            }
          }
        }
      }
    }).observe(this, {
      attributes: true,
      childList: false,
      subtree: false,
    });

    this.intersectionObserver = new IntersectionObserver((entries) => {
      entries.forEach((it) => {
        let tr = it.target as TraceRow<any>;
        if (!it.isIntersecting) {
          tr.sleeping = true;
          this.visibleRows = this.visibleRows.filter((it) => !it.sleeping);
        } else {
          if (
            !this.visibleRows.find(
              (vr) => vr.rowId === tr.rowId && vr.rowType === tr.rowType && vr.rowParentId === tr.rowParentId
            )
          ) {
            this.visibleRows.push(tr);
          }
          tr.sleeping = false;
        }
        if (this.handler) clearTimeout(this.handler);
        this.handler = setTimeout(() => this.refreshCanvas(false), 100);
      });
    });
    window.addEventListener('keydown', (ev) => {
      if (ev.key.toLocaleLowerCase() === 'escape') {
        this.shadowRoot?.querySelectorAll<TraceRow<any>>('trace-row').forEach((it) => {
          it.checkType = '-1';
        });
        TraceRow.rangeSelectObject = undefined;
        this.rangeSelect.rangeTraceRow = [];
        this.selectStructNull();
        this.timerShaftEL?.setSlicesMark();
        this.traceSheetEL?.setAttribute('mode', 'hidden');
        this.removeLinkLinesByBusinessType('janks', 'task');
      }
    });
    this.chartManager = new SpChartManager(this);
    this.canvasPanel = this.shadowRoot!.querySelector<HTMLCanvasElement>('#canvas-panel')!;
    this.canvasFavoritePanel = this.shadowRoot!.querySelector<HTMLCanvasElement>('#canvas-panel-favorite')!;
    this.canvasPanelCtx = this.canvasPanel.getContext('2d');

    this.canvasFavoritePanelCtx = this.canvasFavoritePanel.getContext('2d');
    this.canvasPanelConfig();
    window.subscribe(window.SmartEvent.UI.SliceMark, (data) => {
      this.sliceMarkEventHandler(data);
    });
    window.subscribe(window.SmartEvent.UI.TraceRowComplete, (tr) => {});
    window.subscribe(window.SmartEvent.UI.RefreshCanvas, () => {
      this.refreshCanvas(false);
    });
    window.subscribe(window.SmartEvent.UI.KeyboardEnable, (tr) => {
      this.keyboardEnable = tr.enable;
      if (!this.keyboardEnable) {
        this.stopWASD();
      }
    });
    window.subscribe(window.SmartEvent.UI.MouseEventEnable, (tr) => {
      this.mouseEventEnable = tr.mouseEnable;
      if (this.mouseEventEnable) {
        this.removeAttribute('disable');
      } else {
        this.setAttribute('disable', '');
      }
    });
  }

  pushPidToSelection(selection: SelectionParam, id: string) {
    let pid = parseInt(id);
    if (!selection.processIds.includes(pid)) {
      selection.processIds.push(pid);
    }
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
    let collectList = this.favoriteRowsEL?.querySelectorAll<TraceRow<any>>(`trace-row[collect-type]`) || [];
    let height = 0;
    collectList.forEach((row, index) => {
      height += row.offsetHeight;
      if (index == collectList.length - 1) {
        row.style.boxShadow = `0 10px 10px #00000044`;
      } else {
        row.style.boxShadow = `0 10px 10px #00000000`;
      }
    });
    if (height > this.rowsPaneEL!.offsetHeight) {
      this.favoriteRowsEL!.style.height = this.rowsPaneEL!.offsetHeight + 'px';
    } else {
      this.favoriteRowsEL!.style.height = height + 'px';
    }
    this.favoriteRowsEL!.style.width = this.canvasPanel?.offsetWidth + 'px';
    this.spacerEL!.style.height = height + 'px';
    this.canvasFavoritePanel!.style.height = this.favoriteRowsEL!.style.height;
    this.canvasFavoritePanel!.style.width = this.canvasPanel?.offsetWidth + 'px';
    this.canvasFavoritePanel!.width = this.canvasFavoritePanel!.offsetWidth * dpr();
    this.canvasFavoritePanel!.height = this.canvasFavoritePanel!.offsetHeight * dpr();
    this.canvasFavoritePanel!.getContext('2d')!.scale(dpr(), dpr());
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

    this.canvasFavoritePanel!.style.left = `${this.timerShaftEL!.canvas!.offsetLeft!}px`;
    this.canvasFavoritePanel!.width = this.canvasFavoritePanel!.offsetWidth * dpr();
    this.canvasFavoritePanel!.height = this.canvasFavoritePanel!.offsetHeight * dpr();
    this.canvasFavoritePanelCtx!.scale(dpr(), dpr());
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

  timerShaftELFlagClickHandler = (flag: Flag | undefined | null) => {
    if (flag) {
      setTimeout(() => {
        this.traceSheetEL?.displayFlagData(flag);
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
        this.traceSheetEL?.displayCurrent(sliceTime); // 给当前pane准备数据
        let selection = this.timerShaftEL!.selectionMap.get(sliceTime.id);
        if (selection) {
          selection.isCurrentPane = true; // 设置当前面板为可以显示的状态
          this.traceSheetEL?.rangeSelect(selection); // 显示选中区域对应的面板
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

  // refresh main canvas and favorite canvas
  refreshCanvas(cache: boolean, from?: string) {
    if (this.visibleRows.length == 0) {
      return;
    }
    //clear main canvas
    this.canvasPanelCtx?.clearRect(0, 0, this.canvasPanel!.offsetWidth, this.canvasPanel!.offsetHeight);
    //clear favorite canvas
    this.canvasFavoritePanelCtx?.clearRect(
      0,
      0,
      this.canvasFavoritePanel!.offsetWidth,
      this.canvasFavoritePanel!.offsetHeight
    );
    //draw lines for main canvas
    let rowsContentHeight = this.getRowsContentHeight();
    let canvasHeight =
      rowsContentHeight > this.canvasPanel!.clientHeight ? this.canvasPanel!.clientHeight : rowsContentHeight;
    canvasHeight += this.canvasFavoritePanel!.clientHeight;
    drawLines(this.canvasPanelCtx!, TraceRow.range?.xs || [], canvasHeight, this.timerShaftEL!.lineColor());
    //draw lines for favorite canvas
    drawLines(
      this.canvasFavoritePanelCtx!,
      TraceRow.range?.xs || [],
      this.canvasFavoritePanel!.clientHeight,
      this.timerShaftEL!.lineColor()
    );
    //canvas translate
    this.canvasPanel!.style.transform = `translateY(${this.rowsPaneEL!.scrollTop}px)`;
    this.canvasFavoritePanel!.style.transform = `translateY(${this.favoriteRowsEL!.scrollTop}px)`;
    //draw trace row
    this.visibleRows.forEach((v, i) => {
      if (v.collect) {
        v.translateY = v.getBoundingClientRect().top - 195;
      } else {
        v.translateY = v.offsetTop - this.rowsPaneEL!.scrollTop;
      }
      v.draw(cache);
    });
    //draw flag line segment for canvas
    drawFlagLineSegment(
      this.canvasPanelCtx,
      this.hoverFlag,
      this.selectFlag,
      {
        x: 0,
        y: 0,
        width: this.timerShaftEL?.canvas?.clientWidth,
        height: this.canvasPanel?.clientHeight,
      },
      this.timerShaftEL!
    );
    //draw flag line segment for favorite canvas
    drawFlagLineSegment(
      this.canvasFavoritePanelCtx,
      this.hoverFlag,
      this.selectFlag,
      {
        x: 0,
        y: 0,
        width: this.timerShaftEL?.canvas?.clientWidth,
        height: this.canvasFavoritePanel?.clientHeight,
      },
      this.timerShaftEL!
    );
    //draw wakeup for main canvas
    drawWakeUp(
      this.canvasPanelCtx,
      CpuStruct.wakeupBean,
      TraceRow.range!.startNS,
      TraceRow.range!.endNS,
      TraceRow.range!.totalNS,
      {
        x: 0,
        y: 0,
        width: TraceRow.FRAME_WIDTH,
        height: this.canvasPanel!.clientHeight!,
      } as Rect
    );
    //draw wakeup for favorite canvas
    drawWakeUp(
      this.canvasFavoritePanelCtx,
      CpuStruct.wakeupBean,
      TraceRow.range!.startNS,
      TraceRow.range!.endNS,
      TraceRow.range!.totalNS,
      {
        x: 0,
        y: 0,
        width: TraceRow.FRAME_WIDTH,
        height: this.canvasFavoritePanel!.clientHeight!,
      } as Rect
    );
    // draw wakeuplist for main canvas
    for (let i = 0; i < SpSystemTrace.wakeupList.length; i++) {
      if (i + 1 == SpSystemTrace.wakeupList.length) {
        return;
      }
      drawWakeUpList(
        this.canvasPanelCtx,
        SpSystemTrace.wakeupList[i + 1],
        TraceRow.range!.startNS,
        TraceRow.range!.endNS,
        TraceRow.range!.totalNS,
        {
          x: 0,
          y: 0,
          width: this.timerShaftEL!.canvas!.clientWidth,
          height: this.canvasPanel!.clientHeight!,
        } as Rect
      );
      drawWakeUpList(
        this.canvasFavoritePanelCtx,
        SpSystemTrace.wakeupList[i + 1],
        TraceRow.range!.startNS,
        TraceRow.range!.endNS,
        TraceRow.range!.totalNS,
        {
          x: 0,
          y: 0,
          width: this.timerShaftEL!.canvas!.clientWidth,
          height: this.canvasFavoritePanel!.clientHeight!,
        } as Rect
      );
    }
    // Draw the connection curve
    if (this.linkNodes) {
      drawLinkLines(this.canvasPanelCtx!, this.linkNodes, this.timerShaftEL!, false);
      drawLinkLines(this.canvasFavoritePanelCtx!, this.linkNodes, this.timerShaftEL!, true);
    }
  }

  documentOnMouseDown = (ev: MouseEvent) => {
    if (!this.loadTraceCompleted || !this.mouseEventEnable) return;
    if (this.isWASDKeyPress()) {
      ev.preventDefault();
      ev.stopPropagation();
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
        this.timerShaftEL!.sportRuler!.frame.contains(x, y) &&
        x > (TraceRow.rangeSelectObject?.startX || 0) &&
        x < (TraceRow.rangeSelectObject?.endX || 0)
      ) {
        let findSlicestime = this.timerShaftEL!.sportRuler?.findSlicesTime(x, y); // 查找帽子
        if (!findSlicestime) {
          // 如果没有找到帽子，则绘制一个三角形的旗子
          let time = Math.round(
            (x * (TraceRow.range?.endNS! - TraceRow.range?.startNS!)) / this.timerShaftEL!.canvas!.offsetWidth +
              TraceRow.range?.startNS!
          );
          this.timerShaftEL!.sportRuler!.drawTriangle(time, 'triangle');
        }
      } else {
        this.rangeSelect.mouseDown(ev);
        this.rangeSelect.drag = true;
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

  documentOnMouseUp = (ev: MouseEvent) => {
    if (!this.loadTraceCompleted || !this.mouseEventEnable) return;
    if (this.isWASDKeyPress()) {
      ev.preventDefault();
      ev.stopPropagation();
      return;
    }
    TraceRow.isUserInteraction = false;
    this.rangeSelect.isMouseDown = false;
    if ((window as any).isSheetMove) return;
    if (this.isMouseInSheet(ev)) return;
    this.rangeSelect.mouseUp(ev);
    this.timerShaftEL?.documentOnMouseUp(ev);
    ev.preventDefault();
    ev.stopPropagation();
  };

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
        // 设置currentPane可以显示，并且修改调色板颜色和刚刚绘制的帽子颜色保持一致。
        this.traceSheetEL = this.shadowRoot?.querySelector('.trace-sheet');
        let currentPane = this.traceSheetEL?.displayTab<TabPaneCurrent>('tabpane-current');
        if (this.slicestime) {
          currentPane?.setCurrentSlicesTime(this.slicestime);
        }
        // 显示对应的面板信息
        this.timerShaftEL!.selectionList.forEach((selection, index) => {
          if (this.timerShaftEL!.selectionList.length - 1 == index) {
            // 把最新添加的 SelectionParam 对象设置为可以显示当前面板
            selection.isCurrentPane = true;
            this.traceSheetEL?.rangeSelect(selection);
          } else {
            // 其他 SelectionParam 对象设置为不显示当前面板
            selection.isCurrentPane = false;
          }
        });
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

  setSLiceMark(shiftKey: boolean): SlicesTime | null | undefined {
    if (CpuStruct.selectCpuStruct) {
      this.slicestime = this.timerShaftEL?.setSlicesMark(
        CpuStruct.selectCpuStruct.startTime || 0,
        (CpuStruct.selectCpuStruct.startTime || 0) + (CpuStruct.selectCpuStruct.dur || 0),
        shiftKey
      );
    } else if (ThreadStruct.selectThreadStruct) {
      this.slicestime = this.timerShaftEL?.setSlicesMark(
        ThreadStruct.selectThreadStruct.startTime || 0,
        (ThreadStruct.selectThreadStruct.startTime || 0) + (ThreadStruct.selectThreadStruct.dur || 0),
        shiftKey
      );
    } else if (FuncStruct.selectFuncStruct) {
      this.slicestime = this.timerShaftEL?.setSlicesMark(
        FuncStruct.selectFuncStruct.startTs || 0,
        (FuncStruct.selectFuncStruct.startTs || 0) + (FuncStruct.selectFuncStruct.dur || 0),
        shiftKey
      );
    } else if (IrqStruct.selectIrqStruct) {
      this.slicestime = this.timerShaftEL?.setSlicesMark(
        IrqStruct.selectIrqStruct.startNS || 0,
        (IrqStruct.selectIrqStruct.startNS || 0) + (IrqStruct.selectIrqStruct.dur || 0),
        shiftKey
      );
    } else if (TraceRow.rangeSelectObject) {
      this.slicestime = this.timerShaftEL?.setSlicesMark(
        TraceRow.rangeSelectObject.startNS || 0,
        TraceRow.rangeSelectObject.endNS || 0,
        shiftKey
      );
    } else if (JankStruct.selectJankStruct) {
      this.slicestime = this.timerShaftEL?.setSlicesMark(
        JankStruct.selectJankStruct.ts || 0,
        (JankStruct.selectJankStruct.ts || 0) + (JankStruct.selectJankStruct.dur || 0),
        shiftKey
      );
    } else if (AppStartupStruct.selectStartupStruct) {
      this.slicestime = this.timerShaftEL?.setSlicesMark(
        AppStartupStruct.selectStartupStruct.startTs || 0,
        (AppStartupStruct.selectStartupStruct.startTs || 0) + (AppStartupStruct.selectStartupStruct.dur || 0),
        shiftKey
      );
    } else if (SoStruct.selectSoStruct) {
      this.slicestime = this.timerShaftEL?.setSlicesMark(
        SoStruct.selectSoStruct.startTs || 0,
        (SoStruct.selectSoStruct.startTs || 0) + (SoStruct.selectSoStruct.dur || 0),
        shiftKey
      );
    } else if (FrameAnimationStruct.selectFrameAnimationStruct) {
      this.timerShaftEL?.setSlicesMark(
        FrameAnimationStruct.selectFrameAnimationStruct.ts || 0,
        (FrameAnimationStruct.selectFrameAnimationStruct.ts || 0) +
          (FrameAnimationStruct.selectFrameAnimationStruct.dur || 0)
      );
    } else if (JsCpuProfilerStruct.selectJsCpuProfilerStruct) {
      this.timerShaftEL?.setSlicesMark(
        JsCpuProfilerStruct.selectJsCpuProfilerStruct.startTime || 0,
        (JsCpuProfilerStruct.selectJsCpuProfilerStruct.startTime || 0) +
          (JsCpuProfilerStruct.selectJsCpuProfilerStruct.totalTime || 0)
      );
    } else {
      this.slicestime = this.timerShaftEL?.setSlicesMark();
    }
    return this.slicestime;
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

  documentOnKeyUp = (ev: KeyboardEvent) => {
    if (!this.loadTraceCompleted) return;
    let keyPress = ev.key.toLocaleLowerCase();
    if (keyPress === 'w' || keyPress === 'a' || keyPress === 's' || keyPress === 'd') {
      this.keyPressMap.set(keyPress, false);
    }
    TraceRow.isUserInteraction = false;
    this.observerScrollHeightEnable = false;
    this.keyboardEnable && this.timerShaftEL!.documentOnKeyUp(ev);
    if (ev.code == 'Enter') {
      if (ev.shiftKey) {
        this.dispatchEvent(
          new CustomEvent('previous-data', {
            detail: {},
            composed: false,
          })
        );
      } else {
        this.dispatchEvent(
          new CustomEvent('next-data', {
            detail: {},
            composed: false,
          })
        );
      }
    }
  };

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
  selectChangeHandler = (rows: Array<TraceRow<any>>) => {
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
  documentOnMouseMove = (ev: MouseEvent) => {
    if (!this.loadTraceCompleted || (window as any).flagInputFocus || !this.mouseEventEnable) return;
    if (this.isWASDKeyPress()) {
      this.hoverFlag = null;
      ev.preventDefault();
      return;
    }
    this.inFavoriteArea = this.favoriteRowsEL?.containPoint(ev);
    if ((window as any).isSheetMove) return;
    if (this.isMouseInSheet(ev)) {
      this.hoverStructNull();
      return;
    }
    let isMouseInTimeShaft = this.timerShaftEL?.containPoint(ev);
    if (isMouseInTimeShaft) {
      this.tipEL!.style.display = 'none';
      this.hoverStructNull();
    }
    let rows = this.visibleRows;
    if (this.timerShaftEL?.isScaling()) {
      return;
    }
    this.timerShaftEL?.documentOnMouseMove(ev);
    if (isMouseInTimeShaft) {
      return;
    }
    this.rangeSelect.mouseMove(rows, ev);
    if (this.rangeSelect.isMouseDown) {
      this.refreshCanvas(true);
    } else {
      if (!this.rowsPaneEL!.containPoint(ev, { left: 248 })) {
        this.tipEL!.style.display = 'none';
        this.hoverStructNull();
      }
      rows
        .filter((it) => it.focusContain(ev, this.inFavoriteArea!) && it.collect === this.inFavoriteArea)
        .filter((it) => {
          if (it.collect) {
            return true;
          } else {
            return (
              it.getBoundingClientRect().bottom + it.getBoundingClientRect().height >
              this.favoriteRowsEL!.getBoundingClientRect().bottom
            );
          }
        })
        .forEach((tr) => {
          if (this.currentRowType != tr.rowType) {
            this.hoverStructNull();
            this.tipEL!.style.display = 'none';
            this.currentRowType = tr.rowType || '';
          }
          if (tr.rowType == TraceRow.ROW_TYPE_CPU) {
            CpuStruct.hoverCpuStruct = undefined;
            for (let re of tr.dataListCache) {
              if (re.frame && isFrameContainPoint(re.frame, tr.hoverX, tr.hoverY)) {
                CpuStruct.hoverCpuStruct = re;
                break;
              }
            }
          } else {
            CpuStruct.hoverCpuStruct = undefined;
          }
          tr.focusHandler?.(ev);
        });
      requestAnimationFrame(() => this.refreshCanvas(true));
    }
  };

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
  }

  isWASDKeyPress() {
    return (
      this.keyPressMap.get('w') || this.keyPressMap.get('a') || this.keyPressMap.get('d') || this.keyPressMap.get('s')
    );
  }

  documentOnClick = (ev: MouseEvent) => {
    if (!this.loadTraceCompleted) return;
    if (this.isWASDKeyPress()) {
      this.hoverFlag = null;
      ev.preventDefault();
      ev.stopPropagation();
      return;
    }
    if ((window as any).isSheetMove) return;
    if (this.isMouseInSheet(ev)) return;
    if ((window as any).isPackUpTable) {
      (window as any).isPackUpTable = false;
      return;
    }
    let x = ev.offsetX - this.timerShaftEL!.canvas!.offsetLeft;
    let y = ev.offsetY;
    if (this.timerShaftEL?.getRangeRuler()?.frame.contains(x, y)) {
      this.clickEmptyArea();
      return;
    }
    if (this.rangeSelect.isDrag()) {
      return;
    }
    if (
      this.timerShaftEL!.sportRuler!.frame.contains(x, y) &&
      x > (TraceRow.rangeSelectObject?.startX || 0) &&
      x < (TraceRow.rangeSelectObject?.endX || 0)
    ) {
    } else {
      let inFavoriteArea = this.favoriteRowsEL?.containPoint(ev);
      let rows = this.visibleRows.filter((it) => it.focusContain(ev, inFavoriteArea!) && it.collect == inFavoriteArea);
      if (JankStruct.delJankLineFlag) {
        this.removeLinkLinesByBusinessType('janks');
      }
      if (rows && rows[0] && this.traceRowClickJudgmentConditions.get(rows[0]!.rowType!)?.()) {
        this.onClickHandler(rows[0]!.rowType!, rows[0]);
        this.documentOnMouseMove(ev);
      } else {
        this.clickEmptyArea();
      }
    }
    ev.preventDefault();
  };

  clickEmptyArea() {
    this.shadowRoot?.querySelectorAll<TraceRow<any>>('trace-row').forEach((it) => {
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
    this.traceSheetEL?.setAttribute('mode', 'hidden');
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
      TraceRow.ROW_TYPE_VMTRACKER_SHM,
      () => SnapshotStruct.hoverSnapshotStruct !== null && SnapshotStruct.hoverSnapshotStruct !== undefined,
    ],
    [
      TraceRow.ROW_TYPE_VM_TRACKER_SMAPS,
      () => SnapshotStruct.hoverSnapshotStruct !== null && SnapshotStruct.hoverSnapshotStruct !== undefined,
    ],
  ]);

  onClickHandler(clickRowType: string, row?: TraceRow<any>) {
    if (row) {
      this.currentRow = row;
      this.setAttribute('clickRow', clickRowType);
      this.setAttribute('rowName', row.name!);
      this.setAttribute('rowId', row.rowId!);
    }
    if (!this.loadTraceCompleted) return;
    this.shadowRoot?.querySelectorAll<TraceRow<any>>('trace-row').forEach((it) => (it.rangeSelect = false));
    this.selectStructNull();
    this.wakeupListNull();
    let threadClickHandler: any;
    let threadClickPreviousHandler: any;
    let threadClickNextHandler: any;
    let cpuClickHandler: any;
    let jankClickHandler: any;
    let snapshotClickHandler: any;
    let scrollToFuncHandler: any;
    threadClickHandler = (d: ThreadStruct) => {
      this.observerScrollHeightEnable = false;
      this.scrollToProcess(`${d.cpu}`, '', 'cpu-data', true);
      let cpuRow = this.shadowRoot?.querySelectorAll<TraceRow<CpuStruct>>(
        `trace-row[row-id='${d.cpu}'][row-type='cpu-data']`
      )[0];
      let findEntry = cpuRow!.dataList!.find((dat: any) => dat.startTime === d.startTime);
      if (
        findEntry!.startTime! + findEntry!.dur! < TraceRow.range!.startNS ||
        findEntry!.startTime! > TraceRow.range!.endNS
      ) {
        this.timerShaftEL?.setRangeNS(
          findEntry!.startTime! - findEntry!.dur! * 2,
          findEntry!.startTime! + findEntry!.dur! + findEntry!.dur! * 2
        );
      }
      this.hoverStructNull();
      this.selectStructNull();
      this.wakeupListNull();
      CpuStruct.hoverCpuStruct = findEntry;
      CpuStruct.selectCpuStruct = findEntry;
      this.timerShaftEL?.drawTriangle(findEntry!.startTime || 0, 'inverted');
      this.traceSheetEL?.displayCpuData(
        CpuStruct.selectCpuStruct!,
        (wakeUpBean) => {
          CpuStruct.wakeupBean = wakeUpBean;
          this.refreshCanvas(true);
        },
        cpuClickHandler
      );
    };

    threadClickPreviousHandler = (d: ThreadStruct) => {
      this.observerScrollHeightEnable = false;
      let threadRow = this.shadowRoot?.querySelector<TraceRow<ThreadStruct>>(
        `trace-row[row-id='${d.tid}'][row-type='thread']`
      );
      threadRow?.dataList.forEach((item, index) => {
        if (item === d && index !== 0) {
          let findEntry = threadRow?.dataList[index - 1];
          this.hoverStructNull();
          this.selectStructNull();
          this.wakeupListNull();
          ThreadStruct.hoverThreadStruct = findEntry;
          ThreadStruct.selectThreadStruct = findEntry;
          this.timerShaftEL?.drawTriangle(findEntry!.startTime || 0, 'inverted');
          this.traceSheetEL?.displayThreadData(
            ThreadStruct.selectThreadStruct!,
            threadClickHandler,
            cpuClickHandler,
            threadClickPreviousHandler,
            threadClickNextHandler
          );
        }
      });
    };
    threadClickNextHandler = (d: ThreadStruct) => {
      this.observerScrollHeightEnable = false;
      let threadRow = this.shadowRoot?.querySelector<TraceRow<ThreadStruct>>(
        `trace-row[row-id='${d.tid}'][row-type='thread']`
      );
      let findEntry = threadRow?.dataList.find((dat: any) => dat.startTime === d.startTime! + d.dur!);
      this.hoverStructNull();
      this.selectStructNull();
      this.wakeupListNull();
      ThreadStruct.hoverThreadStruct = findEntry;
      ThreadStruct.selectThreadStruct = findEntry;
      this.timerShaftEL?.drawTriangle(findEntry!.startTime || 0, 'inverted');
      this.traceSheetEL?.displayThreadData(
        ThreadStruct.selectThreadStruct!,
        threadClickHandler,
        cpuClickHandler,
        threadClickPreviousHandler,
        threadClickNextHandler
      );
    };
    cpuClickHandler = (d: CpuStruct) => {
      let traceRow = this.shadowRoot?.querySelector<TraceRow<any>>(
        `trace-row[row-id='${d.processId}'][row-type='process']`
      );
      if (traceRow) {
        traceRow.expansion = true;
      }
      this.observerScrollHeightEnable = true;
      let threadRow = this.shadowRoot?.querySelectorAll<TraceRow<ThreadStruct>>(
        `trace-row[row-id='${d.tid}'][row-type='thread']`
      )[0];
      let task = () => {
        if (threadRow) {
          let findEntry = threadRow!.dataList!.find((dat) => dat.startTime === d.startTime);
          if (
            findEntry!.startTime! + findEntry!.dur! < TraceRow.range!.startNS ||
            findEntry!.startTime! > TraceRow.range!.endNS
          ) {
            this.timerShaftEL?.setRangeNS(
              findEntry!.startTime! - findEntry!.dur! * 2,
              findEntry!.startTime! + findEntry!.dur! + findEntry!.dur! * 2
            );
          }
          this.hoverStructNull();
          this.selectStructNull();
          this.wakeupListNull();
          ThreadStruct.hoverThreadStruct = findEntry;
          ThreadStruct.selectThreadStruct = findEntry;
          this.timerShaftEL?.drawTriangle(findEntry!.startTime || 0, 'inverted');
          this.traceSheetEL?.displayThreadData(
            ThreadStruct.selectThreadStruct!,
            threadClickHandler,
            cpuClickHandler,
            threadClickPreviousHandler,
            threadClickNextHandler
          );
          this.scrollToProcess(`${d.tid}`, `${d.processId}`, 'thread', true);
        }
      };
      if (threadRow) {
        this.scrollToProcess(`${d.tid}`, `${d.processId}`, 'process', false);
        this.scrollToProcess(`${d.tid}`, `${d.processId}`, 'thread', true);
        if (threadRow!.isComplete) {
          task();
        } else {
          threadRow!.onComplete = task;
        }
      }
    };

    jankClickHandler = (d: any) => {
      this.observerScrollHeightEnable = true;
      let jankRowParent: any;
      if (d.rowId === 'actual frameTime') {
        jankRowParent = this.shadowRoot?.querySelector<TraceRow<JankStruct>>(`trace-row[row-id='frameTime']`);
      } else {
        jankRowParent = this.shadowRoot?.querySelector<TraceRow<JankStruct>>(`trace-row[row-id='${d.pid}']`);
      }
      // jankRowParent!.expansion = true;
      let jankRow: any;
      jankRowParent.childrenList.forEach((item: TraceRow<JankStruct>) => {
        if (item.rowId === `${d.rowId}` && item.rowType === 'janks') {
          jankRow = item;
        }
      });
      let task = () => {
        if (jankRow) {
          JankStruct.selectJankStructList.length = 0;
          let findJankEntry = jankRow!.dataList!.find((dat: any) => dat.name == d.name && dat.pid == d.pid);
          if (findJankEntry) {
            if (
              findJankEntry!.ts! + findJankEntry!.dur! < TraceRow.range!.startNS ||
              findJankEntry!.ts! > TraceRow.range!.endNS
            ) {
              this.timerShaftEL?.setRangeNS(
                findJankEntry!.ts! - findJankEntry!.dur! * 2,
                findJankEntry!.ts! + findJankEntry!.dur! + findJankEntry!.dur! * 2
              );
            }
            this.hoverStructNull();
            this.selectStructNull();
            this.wakeupListNull();
            JankStruct.hoverJankStruct = findJankEntry;
            JankStruct.selectJankStruct = findJankEntry;
            this.timerShaftEL?.drawTriangle(findJankEntry!.ts || 0, 'inverted');
            this.traceSheetEL?.displayJankData(
              JankStruct.selectJankStruct!,
              (datas) => {
                this.removeLinkLinesByBusinessType('janks');
                // 绘制跟自己关联的线
                datas.forEach((data) => {
                  let endParentRow = this.shadowRoot?.querySelector<TraceRow<any>>(
                    `trace-row[row-id='${data.pid}'][folder]`
                  );
                  this.drawJankLine(endParentRow, JankStruct.selectJankStruct!, data);
                });
              },
              jankClickHandler
            );
          }
          this.scrollToProcess(jankRow.rowId!, jankRow.rowParentId!, jankRow.rowType!, true);
        }
      };
      if (jankRow) {
        this.scrollToProcess(jankRow.rowId!, jankRow.rowParentId!, jankRow.rowType!, false);
      }
      task();
    };

    scrollToFuncHandler = (funcStract: any) => {
      this.observerScrollHeightEnable = true;
      this.moveRangeToCenter(funcStract.startTime!, funcStract.dur!);
      this.scrollToActFunc(funcStract, false);
    };

    snapshotClickHandler = (d: HeapSnapshotStruct) => {
      this.observerScrollHeightEnable = true;
      let snapshotRow = this.shadowRoot?.querySelector<TraceRow<HeapSnapshotStruct>>(
        `trace-row[row-id='heapsnapshot']`
      );
      let task = () => {
        if (snapshotRow) {
          let findEntry = snapshotRow!.dataList!.find((dat) => dat.startTs === d.startTs);
          this.hoverStructNull();
          this.selectStructNull();
          this.wakeupListNull();
          HeapSnapshotStruct.hoverSnapshotStruct = findEntry;
          HeapSnapshotStruct.selectSnapshotStruct = findEntry;
        }
      };
      if (snapshotRow) {
        if (snapshotRow!.isComplete) {
          task();
        } else {
          snapshotRow!.onComplete = task;
        }
      }
    };
    if (clickRowType === TraceRow.ROW_TYPE_CPU && CpuStruct.hoverCpuStruct) {
      CpuStruct.selectCpuStruct = CpuStruct.hoverCpuStruct;
      this.timerShaftEL?.drawTriangle(CpuStruct.selectCpuStruct!.startTime || 0, 'inverted');
      this.traceSheetEL?.displayCpuData(
        CpuStruct.selectCpuStruct,
        (wakeUpBean) => {
          CpuStruct.wakeupBean = wakeUpBean;
          this.refreshCanvas(false);
        },
        cpuClickHandler
      );
      this.timerShaftEL?.modifyFlagList(undefined);
    } else if (clickRowType === TraceRow.ROW_TYPE_THREAD && ThreadStruct.hoverThreadStruct) {
      ThreadStruct.selectThreadStruct = ThreadStruct.hoverThreadStruct;
      this.timerShaftEL?.drawTriangle(ThreadStruct.selectThreadStruct!.startTime || 0, 'inverted');
      this.traceSheetEL?.displayThreadData(
        ThreadStruct.selectThreadStruct,
        threadClickHandler,
        cpuClickHandler,
        threadClickPreviousHandler,
        threadClickNextHandler
      );
      this.timerShaftEL?.modifyFlagList(undefined);
    } else if (clickRowType === TraceRow.ROW_TYPE_FUNC && FuncStruct.hoverFuncStruct) {
      TabPaneTaskFrames.TaskArray = [];
      this.removeLinkLinesByBusinessType('task');
      FuncStruct.selectFuncStruct = FuncStruct.hoverFuncStruct;
      let hoverFuncStruct = FuncStruct.hoverFuncStruct;
      this.timerShaftEL?.drawTriangle(FuncStruct.selectFuncStruct!.startTs || 0, 'inverted');
      FuncStruct.selectFuncStruct = hoverFuncStruct;
      let flagConfig = FlagsConfig.getFlagsConfig('TaskPool');
      let showTabArray: Array<string> = ['current-selection'];
      if (flagConfig!.TaskPool === 'Enabled') {
        if (FuncStruct.selectFuncStruct !== undefined && FuncStruct.selectFuncStruct.funName !== undefined) {
          if (FuncStruct.selectFuncStruct.funName.indexOf('H:Task ') >= 0) {
            showTabArray.push('box-task-frames');
            this.drawTaskPollLine(row);
          }
        }
      }
      this.traceSheetEL?.displayFuncData(showTabArray, FuncStruct.selectFuncStruct, scrollToFuncHandler);
      this.timerShaftEL?.modifyFlagList(undefined);
    } else if (clickRowType === TraceRow.ROW_TYPE_CPU_FREQ && CpuFreqStruct.hoverCpuFreqStruct) {
      CpuFreqStruct.selectCpuFreqStruct = CpuFreqStruct.hoverCpuFreqStruct;
      this.traceSheetEL?.displayFreqData();
      this.timerShaftEL?.modifyFlagList(undefined);
    } else if (clickRowType === TraceRow.ROW_TYPE_CPU_STATE && CpuStateStruct.hoverStateStruct) {
      CpuStateStruct.selectStateStruct = CpuStateStruct.hoverStateStruct;
      this.traceSheetEL?.displayCpuStateData();
      this.timerShaftEL?.modifyFlagList(undefined);
    } else if (clickRowType === TraceRow.ROW_TYPE_CPU_FREQ_LIMIT && CpuFreqLimitsStruct.hoverCpuFreqLimitsStruct) {
      CpuFreqLimitsStruct.selectCpuFreqLimitsStruct = CpuFreqLimitsStruct.hoverCpuFreqLimitsStruct;
      this.traceSheetEL?.displayFreqLimitData();
      this.timerShaftEL?.modifyFlagList(undefined);
    } else if (clickRowType === TraceRow.ROW_TYPE_CLOCK && ClockStruct.hoverClockStruct) {
      ClockStruct.selectClockStruct = ClockStruct.hoverClockStruct;
      this.traceSheetEL?.displayClockData(ClockStruct.selectClockStruct);
      this.timerShaftEL?.modifyFlagList(undefined);
    } else if (clickRowType === TraceRow.ROW_TYPE_SYS_MEMORY_GPU_TOTAL && SnapshotStruct.hoverSnapshotStruct) {
      let gpuDumpTotalRow = this.shadowRoot?.querySelector<TraceRow<SnapshotStruct>>(
        `trace-row[row-id='Skia Gpu Dump Total']`
      );
      SnapshotStruct.selectSnapshotStruct = SnapshotStruct.hoverSnapshotStruct;
      this.traceSheetEL?.displayGpuSelectedData(
        'total',
        SnapshotStruct.selectSnapshotStruct.startNs,
        gpuDumpTotalRow!.dataList
      );
      this.timerShaftEL?.modifyFlagList(undefined);
    } else if (clickRowType === TraceRow.ROW_TYPE_SYS_MEMORY_GPU_WINDOW && SnapshotStruct.hoverSnapshotStruct) {
      let gpuDumpWindowRow = this.shadowRoot?.querySelector<TraceRow<SnapshotStruct>>(
        `trace-row[row-id='Skia Gpu Dump Window']`
      );
      SnapshotStruct.selectSnapshotStruct = SnapshotStruct.hoverSnapshotStruct;
      this.traceSheetEL?.displayGpuSelectedData(
        'window',
        SnapshotStruct.selectSnapshotStruct.startNs,
        gpuDumpWindowRow!.dataList
      );
      this.timerShaftEL?.modifyFlagList(undefined);
    } else if (clickRowType === TraceRow.ROW_TYPE_IRQ && IrqStruct.hoverIrqStruct) {
      IrqStruct.selectIrqStruct = IrqStruct.hoverIrqStruct;
      this.traceSheetEL?.displayIrqData(IrqStruct.selectIrqStruct);
      this.timerShaftEL?.modifyFlagList(undefined);
    } else if (
      clickRowType === TraceRow.ROW_TYPE_HEAP &&
      row &&
      row.getAttribute('heap-type') === 'native_hook_statistic' &&
      HeapStruct.hoverHeapStruct
    ) {
      HeapStruct.selectHeapStruct = HeapStruct.hoverHeapStruct;
      this.traceSheetEL?.displayNativeHookData(HeapStruct.selectHeapStruct, row.rowId!);
      this.timerShaftEL?.modifyFlagList(undefined);
    } else if (clickRowType === TraceRow.ROW_TYPE_JANK && JankStruct.hoverJankStruct) {
      JankStruct.selectJankStructList.length = 0;
      this.removeLinkLinesByBusinessType('janks');
      JankStruct.selectJankStruct = JankStruct.hoverJankStruct;
      this.timerShaftEL?.drawTriangle(JankStruct.selectJankStruct!.ts || 0, 'inverted');
      this.traceSheetEL?.displayJankData(
        JankStruct.selectJankStruct,
        (datas) => {
          datas.forEach((data) => {
            let endParentRow;
            if (data.frame_type == 'frameTime') {
              endParentRow = this.shadowRoot?.querySelector<TraceRow<JankStruct>>(
                `trace-row[row-id='frameTime'][row-type='janks']`
              );
            } else {
              endParentRow = this.shadowRoot?.querySelector<TraceRow<any>>(`trace-row[row-id='${data.pid}'][folder]`);
            }
            this.drawJankLine(endParentRow, JankStruct.selectJankStruct!, data);
          });
        },
        jankClickHandler
      );
    } else if (clickRowType === TraceRow.ROW_TYPE_HEAP_SNAPSHOT && HeapSnapshotStruct.hoverSnapshotStruct) {
      let snapshotRow = this.shadowRoot?.querySelector<TraceRow<HeapSnapshotStruct>>(
        `trace-row[row-id='heapsnapshot']`
      );
      HeapSnapshotStruct.selectSnapshotStruct = HeapSnapshotStruct.hoverSnapshotStruct;
      this.traceSheetEL?.displaySnapshotData(
        HeapSnapshotStruct.selectSnapshotStruct!,
        snapshotRow!.dataList,
        snapshotClickHandler
      );
    } else if (clickRowType === TraceRow.ROW_TYPE_JS_CPU_PROFILER && JsCpuProfilerStruct.hoverJsCpuProfilerStruct) {
      JsCpuProfilerStruct.selectJsCpuProfilerStruct = JsCpuProfilerStruct.hoverJsCpuProfilerStruct;
      let selectStruct = JsCpuProfilerStruct.selectJsCpuProfilerStruct;
      let dataArr: Array<JsCpuProfilerChartFrame> = [];
      let parentIdArr: Array<number> = [];
      let that = this;
      getTopJsCpuProfilerStruct(selectStruct.parentId);
      function getTopJsCpuProfilerStruct(parentId: number) {
        if (parentId === -1 && selectStruct.parentId === -1) {
          // 点击的函数是第一层，直接设置其children的isSelect为true，不用重新算totalTime
          let data = that.chartManager!.arkTsChart.chartFrameMap.get(selectStruct!.id);
          if (data && dataArr.length === 0) {
            let copyData = JSON.parse(JSON.stringify(data));
            setSelectChildrenState(copyData);
            dataArr.push(copyData);
          }
        } else {
          let parent = that.chartManager!.arkTsChart.chartFrameMap.get(parentId);
          if (parent) {
            parentIdArr.push(parent.id);
            getTopJsCpuProfilerStruct(parent.parentId!);
            if (parent.parentId === -1 && dataArr.length === 0) {
              let data = that.chartManager!.arkTsChart.chartFrameMap.get(parent.id);
              let copyParent = JSON.parse(JSON.stringify(data));
              copyParent.totalTime = selectStruct.totalTime;
              copyParent.selfTime = 0;
              // depth为0的isSelect改为true
              copyParent.isSelect = true;
              if (copyParent.children.length > 0) {
                getSelectStruct(copyParent);
              }
              dataArr.push(copyParent);
            }
          }
        }
      }

      function getSelectStruct(data: JsCpuProfilerChartFrame) {
        for (let child of data.children) {
          if (child.id === selectStruct!.id) {
            // 将点击的函数的children的isSelect改为true
            setSelectChildrenState(child);
          } else {
            getSelectStruct(child);
          }
          if (parentIdArr.includes(child.id)) {
            child.isSelect = true;
            child.totalTime = selectStruct.totalTime;
            child.selfTime = 0;
          }
        }
      }

      function setSelectChildrenState(data: JsCpuProfilerChartFrame) {
        data.isSelect = true;
        if (data.children.length > 0) {
          for (let child of data.children) {
            setSelectChildrenState(child);
          }
        }
      }
      that.traceSheetEL?.displayJsProfilerData(dataArr);
    } else if (clickRowType === TraceRow.ROW_TYPE_APP_STARTUP && AppStartupStruct.hoverStartupStruct) {
      AppStartupStruct.selectStartupStruct = AppStartupStruct.hoverStartupStruct;
      this.traceSheetEL?.displayStartupData(AppStartupStruct.selectStartupStruct, scrollToFuncHandler);
      this.timerShaftEL?.modifyFlagList(undefined);
    } else if (clickRowType === TraceRow.ROW_TYPE_STATIC_INIT && SoStruct.hoverSoStruct) {
      SoStruct.selectSoStruct = SoStruct.hoverSoStruct;
      this.traceSheetEL?.displayStaticInitData(SoStruct.selectSoStruct, scrollToFuncHandler);
      this.timerShaftEL?.modifyFlagList(undefined);
    } else if (clickRowType === TraceRow.ROW_TYPE_FRAME_ANIMATION && FrameAnimationStruct.hoverFrameAnimationStruct) {
      FrameAnimationStruct.selectFrameAnimationStruct = FrameAnimationStruct.hoverFrameAnimationStruct;
      this.traceSheetEL?.displayFrameAnimationData(FrameAnimationStruct.selectFrameAnimationStruct);
      this.timerShaftEL?.modifyFlagList(undefined);
    } else if (clickRowType === TraceRow.ROW_TYPE_FRAME_DYNAMIC && FrameDynamicStruct.hoverFrameDynamicStruct) {
      FrameDynamicStruct.selectFrameDynamicStruct = FrameDynamicStruct.hoverFrameDynamicStruct;
      this.traceSheetEL?.displayFrameDynamicData(row!, FrameDynamicStruct.selectFrameDynamicStruct);
      this.timerShaftEL?.modifyFlagList(undefined);
    } else if (clickRowType === TraceRow.ROW_TYPE_FRAME_SPACING && FrameSpacingStruct.hoverFrameSpacingStruct) {
      FrameSpacingStruct.selectFrameSpacingStruct = FrameSpacingStruct.hoverFrameSpacingStruct;
      this.traceSheetEL?.displayFrameSpacingData(FrameSpacingStruct.selectFrameSpacingStruct);
      this.timerShaftEL?.modifyFlagList(undefined);
    } else if (clickRowType === TraceRow.ROW_TYPE_VM_TRACKER_SMAPS && SnapshotStruct.hoverSnapshotStruct) {
      let smapsRow = this.shadowRoot?.querySelector<TraceRow<SnapshotStruct>>(`trace-row[row-id='Dirty']`);
      SnapshotStruct.selectSnapshotStruct = SnapshotStruct.hoverSnapshotStruct;
      this.traceSheetEL?.displaySmapsData(SnapshotStruct.selectSnapshotStruct!, smapsRow!.dataList);
    } else if (clickRowType === TraceRow.ROW_TYPE_VMTRACKER_SHM && SnapshotStruct.hoverSnapshotStruct) {
      let shmRow = this.shadowRoot?.querySelector<TraceRow<SnapshotStruct>>(`trace-row[row-id='SHM']`);
      SnapshotStruct.selectSnapshotStruct = SnapshotStruct.hoverSnapshotStruct;
      this.traceSheetEL?.displayShmData(SnapshotStruct.selectSnapshotStruct!, shmRow!.dataList);
    } else if (clickRowType === TraceRow.ROW_TYPE_PURGEABLE_TOTAL_ABILITY && SnapshotStruct.hoverSnapshotStruct) {
      let totalAbilityRow = this.shadowRoot?.querySelector<TraceRow<SnapshotStruct>>(
        `trace-row[row-id='System Purgeable Total']`
      );
      SnapshotStruct.selectSnapshotStruct = SnapshotStruct.hoverSnapshotStruct;
      this.traceSheetEL?.displayPurgTotalAbilityData(SnapshotStruct.hoverSnapshotStruct, totalAbilityRow!.dataList);
    } else if (clickRowType === TraceRow.ROW_TYPE_PURGEABLE_PIN_ABILITY && SnapshotStruct.hoverSnapshotStruct) {
      let pinAbilityRow = this.shadowRoot?.querySelector<TraceRow<SnapshotStruct>>(
        `trace-row[row-id='System Purgeable Pin']`
      );
      SnapshotStruct.selectSnapshotStruct = SnapshotStruct.hoverSnapshotStruct;
      this.traceSheetEL?.displayPurgPinAbilityData(SnapshotStruct.hoverSnapshotStruct, pinAbilityRow!.dataList);
    } else if (clickRowType === TraceRow.ROW_TYPE_PURGEABLE_TOTAL_VM && SnapshotStruct.hoverSnapshotStruct) {
      let totalVMRow = this.shadowRoot?.querySelector<TraceRow<SnapshotStruct>>(`trace-row[row-id='Purgeable Total']`);
      SnapshotStruct.selectSnapshotStruct = SnapshotStruct.hoverSnapshotStruct;
      this.traceSheetEL?.displayPurgTotalVMData(SnapshotStruct.hoverSnapshotStruct, totalVMRow!.dataList);
    } else if (clickRowType === TraceRow.ROW_TYPE_PURGEABLE_PIN_VM && SnapshotStruct.hoverSnapshotStruct) {
      let pinVMRow = this.shadowRoot?.querySelector<TraceRow<SnapshotStruct>>(`trace-row[row-id='Purgeable Pin']`);
      SnapshotStruct.selectSnapshotStruct = SnapshotStruct.hoverSnapshotStruct;
      this.traceSheetEL?.displayPurgPinVMData(SnapshotStruct.hoverSnapshotStruct, pinVMRow!.dataList);
    } else if (clickRowType === TraceRow.ROW_TYPE_DMA_ABILITY && SnapshotStruct.hoverSnapshotStruct) {
      let dmaAbilityRow = this.shadowRoot?.querySelector<TraceRow<SnapshotStruct>>(
        `trace-row[row-id='abilityMonitorDma']`
      );
      SnapshotStruct.selectSnapshotStruct = SnapshotStruct.hoverSnapshotStruct;
      this.traceSheetEL?.displayDmaAbility(SnapshotStruct.selectSnapshotStruct.startNs, dmaAbilityRow!.dataList);
    } else if (clickRowType === TraceRow.ROW_TYPE_DMA_VMTRACKER && SnapshotStruct.hoverSnapshotStruct) {
      let dmaVmTracker = this.shadowRoot?.querySelector<TraceRow<SnapshotStruct>>(
        `trace-row[row-type='dma-vmTracker']`
      );
      SnapshotStruct.selectSnapshotStruct = SnapshotStruct.hoverSnapshotStruct;
      this.traceSheetEL?.displayDmaVmTracker(SnapshotStruct.selectSnapshotStruct.startNs, dmaVmTracker!.dataList);
    } else if (clickRowType === TraceRow.ROW_TYPE_GPU_MEMORY_ABILITY && SnapshotStruct.hoverSnapshotStruct) {
      let gpuMemoryAbilityMonitor = this.shadowRoot?.querySelector<TraceRow<SnapshotStruct>>(
        `trace-row[row-id='abilityMonitorGpuMemory']`
      );
      SnapshotStruct.selectSnapshotStruct = SnapshotStruct.hoverSnapshotStruct;
      this.traceSheetEL?.displayGpuMemoryAbility(
        SnapshotStruct.selectSnapshotStruct.startNs,
        gpuMemoryAbilityMonitor!.dataList
      );
    } else if (clickRowType === TraceRow.ROW_TYPE_GPU_MEMORY_VMTRACKER && SnapshotStruct.hoverSnapshotStruct) {
      let gpuMemoryVmTracker = this.shadowRoot?.querySelector<TraceRow<SnapshotStruct>>(
        `trace-row[row-id='Skia Gpu Memory']`
      );
      SnapshotStruct.selectSnapshotStruct = SnapshotStruct.hoverSnapshotStruct;
      this.traceSheetEL?.displayGpuMemoryVmTracker(
        SnapshotStruct.selectSnapshotStruct.startNs,
        gpuMemoryVmTracker!.dataList
      );
    } else {
      if (!JankStruct.hoverJankStruct && JankStruct.delJankLineFlag) {
        this.removeLinkLinesByBusinessType('janks');
      }
      this.observerScrollHeightEnable = false;
      this.selectFlag = null;
      this.timerShaftEL?.removeTriangle('inverted');
      if (!SportRuler.isMouseInSportRuler) {
        this.traceSheetEL?.setAttribute('mode', 'hidden');
        this.refreshCanvas(true);
      }
    }
    if (!JankStruct.selectJankStruct) {
      this.removeLinkLinesByBusinessType('janks');
    }
    if (row) {
      let pointEvent = this.createPointEvent(row);
      SpStatisticsHttpUtil.addOrdinaryVisitAction({
        action: 'trace_row',
        event: pointEvent,
      });
    }
  }

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

  drawTaskPollLine(row?: TraceRow<any>) {
    let executeID = TabPaneTaskFrames.getExecuteId(FuncStruct.selectFuncStruct!.funName!);
    TabPaneTaskFrames.TaskArray.push(FuncStruct.selectFuncStruct!);
    if (!row) {
      return;
    }
    if (FuncStruct.selectFuncStruct!.funName!.indexOf('H:Task Perform:') >= 0) {
      TabPaneTaskFrames.IsShowConcurrency = true;
      queryBySelectExecute(executeID, FuncStruct.selectFuncStruct!.itid!).then((res) => {
        if (res.length === 1) {
          let allocationRowId = res[0].tid;
          let selectRow = this.shadowRoot?.querySelector<TraceRow<FuncStruct>>(
            `trace-row[row-id='${allocationRowId}'][row-type=\'func\']`
          );
          selectRow!.dataList.forEach((value) => {
            // allocation to execute
            if (value.id === res[0].allocation_task_row) {
              TabPaneTaskFrames.TaskArray.push(value);
              this.addPointPair(
                this.makePoint(
                  value.startTs!,
                  0,
                  selectRow?.translateY!,
                  selectRow,
                  (value.depth! + 0.5) * 20,
                  'task',
                  LineType.bezierCurve,
                  true
                ),
                this.makePoint(
                  FuncStruct.selectFuncStruct!.startTs!,
                  0,
                  row?.translateY!,
                  row,
                  (FuncStruct.selectFuncStruct!.depth! + 0.5) * 20,
                  'task',
                  LineType.bezierCurve,
                  true
                )
              );
            }
            // execute to return
            if (value.id === res[0].return_task_row) {
              TabPaneTaskFrames.TaskArray.push(value);
              this.addPointPair(
                this.makePoint(
                  FuncStruct.selectFuncStruct!.startTs!,
                  FuncStruct.selectFuncStruct!.dur!,
                  row?.translateY!,
                  row,
                  (FuncStruct.selectFuncStruct!.depth! + 0.5) * 20,
                  'task',
                  LineType.bezierCurve,
                  false
                ),
                this.makePoint(
                  value.startTs!,
                  value.dur!,
                  selectRow?.translateY!,
                  selectRow,
                  (value.depth! + 0.5) * 20,
                  'task',
                  LineType.bezierCurve,
                  false
                )
              );
            }
          });
          this.refreshCanvas(true);
        }
      });
    } else {
      TabPaneTaskFrames.IsShowConcurrency = false;
      queryBySelectAllocationOrReturn(executeID, FuncStruct.selectFuncStruct!.itid!).then((res) => {
        if (FuncStruct.selectFuncStruct!.funName!.indexOf('H:Task Allocation:') >= 0 && res.length > 0) {
          let executeRow = this.shadowRoot?.querySelector<TraceRow<FuncStruct>>(
            `trace-row[row-id='${res[0].tid}'][row-type='func']`
          );
          let endStruct: FuncStruct;
          row!.dataList.forEach((value) => {
            if (value.id === res[0].return_task_row) {
              TabPaneTaskFrames.TaskArray.push(value);
              endStruct = value;
            }
          });
          if (!executeRow) {
            return;
          }
          executeRow!.dataList.forEach((value) => {
            if (value.id === res[0].execute_task_row) {
              TabPaneTaskFrames.TaskArray.push(value);
              this.addPointPair(
                this.makePoint(
                  FuncStruct.selectFuncStruct!.startTs!,
                  0,
                  row?.translateY!,
                  row,
                  (FuncStruct.selectFuncStruct!.depth! + 0.5) * 20,
                  'task',
                  LineType.bezierCurve,
                  true
                ),
                this.makePoint(
                  value.startTs!,
                  0,
                  executeRow?.translateY!,
                  executeRow,
                  (value.depth! + 0.5) * 20,
                  'task',
                  LineType.bezierCurve,
                  true
                )
              );
              if (endStruct) {
                this.addPointPair(
                  this.makePoint(
                    value.startTs!,
                    value.dur!,
                    executeRow?.translateY!,
                    executeRow,
                    (value.depth! + 0.5) * 20,
                    'task',
                    LineType.bezierCurve,
                    false
                  ),
                  this.makePoint(
                    endStruct.startTs!,
                    endStruct.dur!,
                    row?.translateY!,
                    row,
                    (endStruct.depth! + 0.5) * 20,
                    'task',
                    LineType.bezierCurve,
                    false
                  )
                );
              }
            }
          });
        } else if (FuncStruct.selectFuncStruct!.funName!.indexOf('H:Task PerformTask End:') >= 0) {
          let executeRow = this.shadowRoot?.querySelector<TraceRow<FuncStruct>>(
            `trace-row[row-id='${res[0].tid}'][row-type='func']`
          );
          TabPaneTaskFrames.TaskArray.push(FuncStruct.selectFuncStruct!);
          let startStruct: FuncStruct;
          row!.dataList.forEach((value) => {
            if (value.id === res[0].allocation_task_row) {
              TabPaneTaskFrames.TaskArray.push(value);
              startStruct = value;
            }
          });
          executeRow!.dataList.forEach((value) => {
            if (value.id === res[0].execute_task_row) {
              TabPaneTaskFrames.TaskArray.push(value);
              this.addPointPair(
                this.makePoint(
                  startStruct!.startTs!,
                  0,
                  row?.translateY!,
                  row,
                  (startStruct!.depth! + 0.5) * 20,
                  'task',
                  LineType.bezierCurve,
                  true
                ),
                this.makePoint(
                  value.startTs!,
                  0,
                  executeRow?.translateY!,
                  executeRow,
                  (value.depth! + 0.5) * 20,
                  'task',
                  LineType.bezierCurve,
                  true
                )
              );
              this.addPointPair(
                this.makePoint(
                  value.startTs!,
                  value.dur!,
                  executeRow?.translateY!,
                  executeRow,
                  (value.depth! + 0.5) * 20,
                  'task',
                  LineType.bezierCurve,
                  false
                ),
                this.makePoint(
                  FuncStruct.selectFuncStruct!.startTs!,
                  FuncStruct.selectFuncStruct!.dur!,
                  row?.translateY!,
                  row,
                  (FuncStruct.selectFuncStruct!.depth! + 0.5) * 20,
                  'task',
                  LineType.bezierCurve,
                  false
                )
              );
            }
          });
        }
        this.refreshCanvas(true);
      });
    }
  }
  drawJankLine(endParentRow: any, selectJankStruct: JankStruct, data: any) {
    let collectList = this.favoriteRowsEL?.querySelectorAll<TraceRow<any>>(`trace-row[collect-type]`) || [];
    let startRow: any;
    if (selectJankStruct == undefined || selectJankStruct == null) {
      return;
    }
    if (selectJankStruct.frame_type == 'frameTime') {
      startRow = this.shadowRoot?.querySelector<TraceRow<JankStruct>>(
        "trace-row[row-id='actual frameTime'][row-type='janks']"
      );
    } else {
      startRow = this.shadowRoot?.querySelector<TraceRow<JankStruct>>(
        `trace-row[row-id='${`${selectJankStruct?.type}-${selectJankStruct?.pid}`}'][row-type='janks']`
      );
    }

    function collectionHasJank(jankRow: any): boolean {
      for (let item of collectList!) {
        if (item.rowId === jankRow.rowId && item.rowType === jankRow.rowType) {
          return false;
        }
      }
      return true;
    }

    if (endParentRow) {
      //终点的父泳道过滤出选中的Struct
      let endRowStruct: any;
      //泳道展开的情况，查找endRowStruct
      if (data.frame_type == 'frameTime') {
        endRowStruct = this.shadowRoot?.querySelector<TraceRow<JankStruct>>(
          "trace-row[row-id='actual frameTime'][row-type='janks']"
        );
      } else {
        endRowStruct = this.shadowRoot?.querySelector<TraceRow<JankStruct>>(
          `trace-row[row-id='${`${data.type}-${data.pid}`}'][row-type='janks']`
        );
      }
      //泳道未展开的情况，查找endRowStruct
      if (!endRowStruct) {
        if (data.frame_type == 'frameTime') {
          endParentRow.childrenList.forEach((item: TraceRow<JankStruct>) => {
            if (item.rowId === 'actual frameTime' && item.rowType === 'janks') {
              endRowStruct = item;
            }
          });
          //frameTime未展开
          if (!endRowStruct) {
            endParentRow = this.shadowRoot?.querySelector<TraceRow<JankStruct>>(
              "trace-row[row-id='frameTime'][folder]"
            );
            endParentRow?.childrenList?.forEach((item: TraceRow<JankStruct>) => {
              if (item.rowId === 'actual frameTime' && item.rowType === 'janks') {
                endRowStruct = item;
              }
            });
          }
        } else {
          endParentRow.childrenList.forEach((item: TraceRow<JankStruct>) => {
            if (item.rowId === `${data.type}-${data.pid}` && item.rowType === 'janks') {
              endRowStruct = item;
            }
          });
        }
      }
      if (endRowStruct) {
        let findJankEntry = endRowStruct!.dataList!.find((dat: any) => dat.name == data.name && dat.pid == data.pid);
        //连线规则：frametimeline的头----app的头，app的尾----renderservice的头
        let tts: number = 0;
        if (findJankEntry) {
          if (selectJankStruct.frame_type == 'app') {
            tts =
              findJankEntry.frame_type == 'frameTime'
                ? selectJankStruct.ts!
                : selectJankStruct.ts! + selectJankStruct.dur!;
            let startParentRow: any;
            // startRow为子泳道，子泳道不存在，使用父泳道
            if (startRow) {
              startParentRow = this.shadowRoot?.querySelector<TraceRow<JankStruct>>(
                `trace-row[row-id='${startRow.rowParentId}'][folder]`
              );
            } else {
              startRow = this.shadowRoot?.querySelector<TraceRow<JankStruct>>(
                `trace-row[row-id='${selectJankStruct?.pid}'][folder]`
              );
            }
            let endY = endRowStruct!.translateY! + 20 * (findJankEntry!.depth! + 0.5);
            let endRowEl = endRowStruct;
            let endOffSetY = 20 * (findJankEntry!.depth! + 0.5);
            let expansionFlag = collectionHasJank(endRowStruct);
            if (!endParentRow.expansion && expansionFlag) {
              endY = endParentRow!.translateY! + 10 * (findJankEntry!.depth! + 0.5);
              endRowEl = endParentRow;
              endOffSetY = 10 * (findJankEntry!.depth! + 0.5);
            }
            let startY = startRow!.translateY! + 20 * (selectJankStruct!.depth! + 0.5);
            let startRowEl = startRow;
            let startOffSetY = 20 * (selectJankStruct!.depth! + 0.5);
            expansionFlag = collectionHasJank(startRow);
            if (startParentRow && !startParentRow.expansion && expansionFlag) {
              startY = startParentRow!.translateY! + 10 * (selectJankStruct!.depth! + 0.5);
              startRowEl = startParentRow;
              startOffSetY = 10 * (selectJankStruct!.depth! + 0.5);
            }
            this.addPointPair(
              {
                x: ns2xByTimeShaft(tts, this.timerShaftEL!),
                y: startY,
                offsetY: startOffSetY,
                ns: tts,
                rowEL: startRowEl!,
                isRight: selectJankStruct.ts == tts,
                business: 'janks',
              },
              {
                x: ns2xByTimeShaft(findJankEntry.ts!, this.timerShaftEL!),
                y: endY,
                offsetY: endOffSetY,
                ns: findJankEntry.ts!,
                rowEL: endRowEl,
                isRight: true,
                business: 'janks',
              }
            );
          }
          if (findJankEntry.frame_type == 'app') {
            tts =
              selectJankStruct.frame_type == 'frameTime' ? findJankEntry.ts : findJankEntry.ts! + findJankEntry.dur!;
            let endY = endRowStruct!.translateY! + 20 * (findJankEntry!.depth! + 0.5);
            let endRowEl = endRowStruct;
            let endOffSetY = 20 * (findJankEntry!.depth! + 0.5);
            let expansionFlag = collectionHasJank(endRowStruct);
            if (!endParentRow.expansion && expansionFlag) {
              endY = endParentRow!.translateY! + 10 * (findJankEntry!.depth! + 0.5);
              endRowEl = endParentRow;
              endOffSetY = 10 * (findJankEntry!.depth! + 0.5);
            }
            let startY = startRow!.translateY! + 20 * (selectJankStruct!.depth! + 0.5);
            let startRowEl = startRow;
            expansionFlag = collectionHasJank(startRow);
            let startOffsetY = 20 * (selectJankStruct!.depth! + 0.5);
            let startParentRow = this.shadowRoot?.querySelector<TraceRow<JankStruct>>(
              `trace-row[row-id='${startRow.rowParentId}'][folder]`
            );
            if (startParentRow && !startParentRow.expansion && expansionFlag) {
              startY = startParentRow!.translateY! + 10 * (selectJankStruct!.depth! + 0.5);
              startRowEl = startParentRow;
              startOffsetY = 10 * (selectJankStruct!.depth! + 0.5);
            }
            this.addPointPair(
              {
                x: ns2xByTimeShaft(selectJankStruct.ts!, this.timerShaftEL!),
                y: startY,
                offsetY: startOffsetY,
                ns: selectJankStruct.ts!,
                rowEL: startRowEl!,
                isRight: true,
                business: 'janks',
              },
              {
                x: ns2xByTimeShaft(tts, this.timerShaftEL!),
                y: endY,
                offsetY: endOffSetY,
                ns: tts,
                rowEL: endRowEl!,
                isRight: selectJankStruct.ts == tts,
                business: 'janks',
              }
            );
          }
          if (data.children.length >= 1) {
            let endP;
            if (data.children[0].frame_type == 'frameTime') {
              endP = this.shadowRoot?.querySelector<TraceRow<any>>("trace-row[row-id='frameTime']");
            } else {
              endP = this.shadowRoot?.querySelector<TraceRow<any>>(
                `trace-row[row-id='${data.children[0].pid}'][folder]`
              );
            }
            this.drawJankLine(endP, findJankEntry, data.children[0]);
          }
          this.refreshCanvas(true);
        }
      }
    }
  }

  myMouseMove = (ev: MouseEvent) => {
    if (ev.ctrlKey) {
      ev.preventDefault();
      SpSystemTrace.offsetMouse = ev.clientX - SpSystemTrace.mouseCurrentPosition;
      let eventA = new KeyboardEvent('keypress', {
        key: 'a',
        code: '65',
        keyCode: 65,
      });
      let eventD = new KeyboardEvent('keypress', {
        key: 'd',
        code: '68',
        keyCode: 68,
      });
      if (ev.button == 0) {
        if (SpSystemTrace.offsetMouse < 0 && SpSystemTrace.moveable) {
          // 向右拖动，则泳道图右移
          this.timerShaftEL!.documentOnKeyPress(eventD);
          setTimeout(() => {
            this.timerShaftEL!.documentOnKeyUp(eventD);
          }, 350);
        }
        if (SpSystemTrace.offsetMouse > 0 && SpSystemTrace.moveable) {
          // 向左拖动，则泳道图左移
          this.timerShaftEL!.documentOnKeyPress(eventA);
          setTimeout(() => {
            this.timerShaftEL!.documentOnKeyUp(eventA);
          }, 350);
        }
      }
      SpSystemTrace.moveable = false;
    }
  };

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
    this.favoriteRowsEL?.addEventListener('scroll', this.favoriteRowsElOnScroll, { passive: true });
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
     * 获取并保存鼠标当前的x轴坐标位置，配合ctrl+鼠标左键拖动完成泳道图的左移或右移
     */
    this.addEventListener(
      'mousedown',
      (e) => {
        if (e.ctrlKey) {
          e.preventDefault();
          this.removeEventListener('mousemove', this.documentOnMouseMove);
          this.removeEventListener('click', this.documentOnClick);
          this.removeEventListener('mousedown', this.documentOnMouseDown);
          this.removeEventListener('mouseup', this.documentOnMouseUp);
          this.style.cursor = 'move';
          SpSystemTrace.moveable = true;
          SpSystemTrace.mouseCurrentPosition = e.clientX;
        }
      },
      { passive: false }
    );
    /**
     * ctrl+鼠标移动，实现泳道图左移或者右移。
     */
    this.addEventListener('mousemove', (ev) => throttle(this.myMouseMove, 350, ev)(), { passive: false });

    this.addEventListener(
      'mouseup',
      (e) => {
        if (e.ctrlKey) {
          e.preventDefault();
          SpSystemTrace.offsetMouse = 0;
          SpSystemTrace.mouseCurrentPosition = 0;
          SpSystemTrace.moveable = false;
          this.style.cursor = 'default';
          this.addEventListener('mousemove', this.documentOnMouseMove);
          this.addEventListener('click', this.documentOnClick);
          this.addEventListener('mousedown', this.documentOnMouseDown);
          this.addEventListener('mouseup', this.documentOnMouseUp);
        }
      },
      { passive: false }
    );

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
      this.favoriteRowsEL?.querySelectorAll<TraceRow<any>>(`trace-row[row-parent-id='${data.rowId}']`).forEach((it) => {
        it.checkType = data.isCheck ? '2' : '0';
      });
    });
  }

  scrollToProcess(rowId: string, rowParentId: string, rowType: string, smooth: boolean = true) {
    let traceRow = this.shadowRoot!.querySelector<TraceRow<any>>(`trace-row[row-id='${rowId}'][row-type='${rowType}']`);
    if (traceRow?.collect) {
      this.favoriteRowsEL!.scroll({
        top: (traceRow?.offsetTop || 0) - this.canvasFavoritePanel!.offsetHeight + (traceRow?.offsetHeight || 0),
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
    let rootRow = this.shadowRoot!.querySelector<TraceRow<any>>(`trace-row[row-id='${rowId}'][row-type='${rowType}']`);
    if (rootRow && rootRow!.collect) {
      this.favoriteRowsEL!.scroll({
        top: (rootRow?.offsetTop || 0) - this.canvasFavoritePanel!.offsetHeight + (++depth * 20 || 0),
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
          top: (rootRow?.offsetTop || 0) - this.canvasPanel!.offsetHeight + (++depth * 20 || 0),
          left: 0,
          behavior: smooth ? 'smooth' : undefined,
        });
      }
    }
  }

  scrollToFunction(rowId: string, rowParentId: string, rowType: string, smooth: boolean = true) {
    let condition = `trace-row[row-id='${rowId}'][row-type='${rowType}'][row-parent-id='${rowParentId}']`;
    let rootRow = this.shadowRoot!.querySelector<TraceRow<any>>(condition);
    if (rootRow?.collect) {
      this.favoriteRowsEL!.scroll({
        top: (rootRow?.offsetTop || 0) - this.canvasFavoritePanel!.offsetHeight + (rootRow?.offsetHeight || 0),
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

  rowScrollTo(offset: number, callback: Function) {
    const fixedOffset = offset;
    const onScroll = () => {
      if (this.rowsPaneEL!.scrollTop === fixedOffset) {
        this.rowsEL!.removeEventListener('scroll', onScroll);
        callback();
      }
    };

    this.rowsEL!.addEventListener('scroll', onScroll);
    onScroll();
    this.rowsPaneEL!.scrollTo({
      top: offset,
      behavior: 'smooth',
    });
  }

  disconnectedCallback() {
    this.timerShaftEL?.removeEventListener('range-change', this.timerShaftELRangeChange);
    this.rowsPaneEL?.removeEventListener('scroll', this.rowsElOnScroll);
    this.favoriteRowsEL?.removeEventListener('scroll', this.favoriteRowsElOnScroll);
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
    this.shadowRoot?.querySelectorAll<TraceRow<any>>('trace-row').forEach((it) => {
      it.checkType = '-1';
    });
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
      let scrollTop = this.rowsEL?.scrollTop || 0;
      let scrollHeight = this.rowsEL?.clientHeight || 0;
      this.rowsEL?.querySelectorAll('trace-row').forEach((it: any) => this.observer.observe(it));
      if (complete) {
        complete(res);
        window.publish(window.SmartEvent.UI.MouseEventEnable, {
          mouseEnable: true,
        });
      }
    });
  }

  search(query: string) {
    this.shadowRoot?.querySelectorAll<TraceRow<any>>('trace-row').forEach((item) => {
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
    let traceRow = this.shadowRoot!.querySelector<TraceRow<any>>(`trace-row[scene]`);
    let dataAll = `trace-row[row-type='cpu-data']`;
    if (traceRow) {
      dataAll = `trace-row[row-type='cpu-data'][scene]`;
    }
    let searchResults: Array<CpuStruct> = [];
    this.shadowRoot!.querySelectorAll<TraceRow<any>>(`${dataAll}`).forEach((item) => {
      let res = item!.dataList!.filter(
        (it) =>
          (it.name && it.name.indexOf(query) >= 0) ||
          it.tid == query ||
          it.processId == query ||
          (it.processName && it.processName.indexOf(query) >= 0)
      );
      searchResults.push(...res);
    });
    searchResults.sort((a, b) => (a.startTime || 0) - (b.startTime || 0));
    return searchResults;
  }

  async searchFunction(cpuList: Array<any>, query: string): Promise<Array<any>> {
    let processList: Array<string> = [];
    let traceRow = this.shadowRoot!.querySelector<TraceRow<any>>(`trace-row[scene]`);
    if (traceRow) {
      this.shadowRoot!.querySelectorAll<TraceRow<any>>(`trace-row[row-type='process'][scene]`).forEach((row) => {
        processList.push(row.rowId!);
      });
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
    let traceRow = this.shadowRoot!.querySelector<TraceRow<any>>(`trace-row[scene]`);
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

  searchThreadsAndProcesses(query: string): Array<any> {
    let searchResults: Array<any> = [];
    this.rowsEL!.querySelectorAll<TraceRow<any>>(`trace-row[row-type='thread'][row-type='process']`).forEach((item) => {
      if (item!.name.indexOf(query) >= 0) {
        let searchBean = new SearchThreadProcessBean();
        searchBean.name = item.name;
        searchBean.rowId = item.rowId;
        searchBean.type = 'thread||process';
        searchBean.rowType = item.rowType;
        searchBean.rowParentId = item.rowParentId;
        searchResults.push(searchBean);
      }
    });
    return searchResults;
  }

  showStruct(previous: boolean, currentIndex: number, structs: Array<any>) {
    if (structs.length == 0) {
      return 0;
    }
    let findIndex = -1;
    if (previous) {
      for (let i = structs.length - 1; i >= 0; i--) {
        let it = structs[i];
        if (
          i < currentIndex &&
          it.startTime! >= TraceRow.range!.startNS &&
          it.startTime! + it.dur! <= TraceRow.range!.endNS
        ) {
          findIndex = i;
          break;
        }
      }
    } else {
      findIndex = structs.findIndex((it, idx) => {
        return (
          idx > currentIndex &&
          it.startTime! >= TraceRow.range!.startNS &&
          it.startTime! + it.dur! <= TraceRow.range!.endNS
        );
      });
    }
    let findEntry: any;
    if (findIndex >= 0) {
      findEntry = structs[findIndex];
    } else {
      if (previous) {
        for (let i = structs.length - 1; i >= 0; i--) {
          let it = structs[i];
          if (it.startTime! + it.dur! < TraceRow.range!.startNS) {
            findIndex = i;
            break;
          }
        }
        if (findIndex == -1) {
          findIndex = structs.length - 1;
        }
      } else {
        findIndex = structs.findIndex((it) => it.startTime! > TraceRow.range!.endNS);
        if (findIndex == -1) {
          findIndex = 0;
        }
      }
      findEntry = structs[findIndex];
    }
    this.moveRangeToCenter(findEntry.startTime!, findEntry.dur!);
    this.shadowRoot!.querySelectorAll<TraceRow<any>>(`trace-row`).forEach((item) => {
      item.highlight = false;
    });
    if (findEntry.type == 'thread') {
      CpuStruct.selectCpuStruct = findEntry;
      CpuStruct.hoverCpuStruct = CpuStruct.selectCpuStruct;
      this.shadowRoot!.querySelectorAll<TraceRow<any>>(`trace-row[row-type='cpu-data']`).forEach((item) => {
        item.highlight = item.rowId == `${findEntry.cpu}`;
        item.draw(true);
      });
      this.scrollToProcess(`${findEntry.cpu}`, '', 'cpu-data', true);
      this.onClickHandler(TraceRow.ROW_TYPE_CPU);
    } else if (findEntry.type == 'func') {
      this.observerScrollHeightEnable = true;
      this.scrollToActFunc(findEntry, true);
    } else if (findEntry.type == 'thread||process') {
      let threadProcessRow = this.rowsEL?.querySelectorAll<TraceRow<ThreadStruct>>('trace-row')[0];
      if (threadProcessRow) {
        let filterRow = threadProcessRow.childrenList.filter(
          (row) => row.rowId === findEntry.rowId && row.rowId === findEntry.rowType
        )[0];
        filterRow!.highlight = true;
        this.closeAllExpandRows(findEntry.rowParentId);
        this.scrollToProcess(`${findEntry.rowId}`, `${findEntry.rowParentId}`, findEntry.rowType, true);
        let completeEntry = () => {
          let searchEntry = filterRow!.dataList!.find((dat) => dat.startTime === findEntry.startTime);
          this.hoverStructNull();
          this.selectStructNull();
          this.wakeupListNull();
          ThreadStruct.hoverThreadStruct = searchEntry;
          ThreadStruct.selectThreadStruct = searchEntry;
          this.scrollToProcess(`${findEntry.rowId}`, `${findEntry.rowParentId}`, findEntry.rowType, true);
        };
        if (filterRow!.isComplete) {
          completeEntry();
        } else {
          filterRow!.onComplete = completeEntry;
        }
      }
    } else if (findEntry.type == 'sdk') {
      let parentRow = this.shadowRoot!.querySelector<TraceRow<any>>(`trace-row[row-type='sdk'][folder]`);
      if (parentRow) {
        let sdkRow = parentRow.childrenList.filter(
          (child) => child.rowId === findEntry.rowId && child.rowType === findEntry.rowType
        )[0];
        sdkRow!.highlight = true;
      }
      this.hoverStructNull();
      this.selectStructNull();
      this.wakeupListNull();
      this.onClickHandler(findEntry.rowType!);
      this.closeAllExpandRows(findEntry.rowParentId);
      this.scrollToProcess(`${findEntry.rowId}`, `${findEntry.rowParentId}`, findEntry.rowType, true);
    }
    this.timerShaftEL?.drawTriangle(findEntry.startTime || 0, 'inverted');
    return findIndex;
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
    let targetRow = this.favoriteRowsEL!.querySelector<TraceRow<any>>(
      `trace-row[row-id='${funcRowID}'][row-type='func']`
    );
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
    if (funcStract.keepOpen !== true) {
      this.closeAllExpandRows(funcStract.pid);
    }
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
    this.canvasFavoritePanelCtx?.clearRect(
      0,
      0,
      this.canvasFavoritePanel!.clientWidth,
      this.canvasFavoritePanel!.clientHeight
    );
    this.favoriteRowsEL!.style.height = '0';
    this.canvasFavoritePanel!.style.height = '0';
    this.loadTraceCompleted = false;
    this.collectRows = [];
    this.visibleRows = [];
    TraceRowConfig.allTraceRowList.forEach((it) => {
      it.clearMemory();
    });
    TraceRowConfig.allTraceRowList = [];
    if (this.favoriteRowsEL) {
      this.favoriteRowsEL.querySelectorAll<TraceRow<any>>(`trace-row`).forEach((row) => {
        row.clearMemory();
        this.favoriteRowsEL!.removeChild(row);
      });
    }
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
      it.addEventListener('expansion-change', this.extracted(it));
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
      this.intersectionObserver?.observe(it);
    });
    return { status: true, msg: 'success' };
  };

  private extracted(it: any) {
    return () => {
      if (it.hasAttribute('expansion')) {
        this.shadowRoot?.querySelectorAll<any>(`[row-parent-id='${it.rowId}']`).forEach((child) => {
          if (child.folder) {
            child.addEventListener('expansion-change', this.extracted(child));
          }
          this.intersectionObserver?.observe(child);
        });
      } else {
        this.shadowRoot?.querySelectorAll<any>(`[row-parent-id='${it.rowId}']`).forEach((child) => {
          this.intersectionObserver?.unobserve(child);
        });
      }
      this.refreshCanvas(false);
    };
  }

  displayTip(row: TraceRow<any>, struct: any, html: string) {
    let x = row.hoverX + 248;
    let y = row.getBoundingClientRect().top - 195 + (this.rowsPaneEL?.scrollTop ?? 0);
    if ((struct === undefined || struct === null) && this.tipEL) {
      this.tipEL.style.display = 'none';
      return;
    }
    if (this.tipEL) {
      this.tipEL.innerHTML = html;
      if (row.rowType === TraceRow.ROW_TYPE_JS_CPU_PROFILER) {
        this.tipEL.style.maxWidth = row.clientWidth / 3 + 'px';
        this.tipEL.style.wordBreak = ' break-all';
        this.tipEL.style.height = 'unset';
        this.tipEL.style.display = 'block';
        y = row.getBoundingClientRect().top - 195 + (this.rowsPaneEL?.scrollTop ?? 0) + struct.depth * 20;
      } else {
        this.tipEL.style.display = 'flex';
        this.tipEL.style.height = row.style.height;
        y = row.getBoundingClientRect().top - 195 + (this.rowsPaneEL?.scrollTop ?? 0);
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
        .favorite-rows{
            width: 100%;
            position:fixed;
            overflow-y: auto;
            overflow-x: hidden;
            z-index:1001;
            background: var(--dark-background5,#ffffff);
            box-shadow: 0 10px 10px #00000044;
        }
        :host([disable]) .container{
            pointer-events: none;
        }
        .container{
            width: 100%;
            box-sizing: border-box;
            height: 100%;
            display: grid;
            grid-template-columns: 1fr;
            grid-template-rows: min-content 1fr min-content;
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
            <div class="rows-pane" style="position: relative;flex-direction: column;overflow-x: hidden;">
                <div class="favorite-rows">
                    <canvas id="canvas-panel-favorite" class="panel-canvas-favorite" ondragstart="return false"></canvas>
                </div>
                <canvas id="canvas-panel" class="panel-canvas" ondragstart="return false"></canvas>
                <div class="spacer" ondragstart="return false"></div>
                <div class="rows" ondragstart="return false"></div>
                <div id="tip" class="tip"></div>
            </div>
            <trace-sheet class="trace-sheet" mode="hidden" ondragstart="return false"></trace-sheet>
        </div>
        `;
  }
}
