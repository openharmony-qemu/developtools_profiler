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

import { BaseElement, element } from '../../../../base-ui/BaseElement.js';
import { LitTable } from '../../../../base-ui/table/lit-table.js';
import '../../../../base-ui/table/lit-table-column.js';

import {
  queryBinderArgsByArgset,
  queryBinderByArgsId,
  queryBinderBySliceId,
  queryFlowsData,
  queryGpuDur,
  queryPrecedingData,
  queryRunnableTimeByRunning,
  queryThreadStateArgs,
  queryThreadWakeUp,
  queryThreadWakeUpFrom,
  queryThreadByItid, queryAnimationFrameFps,
} from '../../../database/SqlLite.js';
import { WakeupBean } from '../../../bean/WakeupBean.js';
import { SpApplication } from '../../../SpApplication.js';
import { TraceRow } from '../base/TraceRow.js';
import { CpuStruct } from '../../../database/ui-worker/ProcedureWorkerCPU.js';
import { ThreadStruct } from '../../../database/ui-worker/ProcedureWorkerThread.js';
import { FuncStruct } from '../../../database/ui-worker/ProcedureWorkerFunc.js';
import { ProcessMemStruct } from '../../../database/ui-worker/ProcedureWorkerMem.js';
import { ClockStruct } from '../../../database/ui-worker/ProcedureWorkerClock.js';
import { ColorUtils } from '../base/ColorUtils.js';
import { IrqStruct } from '../../../database/ui-worker/ProcedureWorkerIrq.js';
import { BinderArgBean } from '../../../bean/BinderArgBean.js';
import { JankStruct } from '../../../database/ui-worker/ProcedureWorkerJank.js';
import { LitIcon } from '../../../../base-ui/icon/LitIcon.js';
import { Utils } from '../base/Utils.js';
import { SpSystemTrace } from '../../SpSystemTrace.js';
import { AppStartupStruct } from '../../../database/ui-worker/ProcedureWorkerAppStartup.js';
import { SoStruct } from '../../../database/ui-worker/ProcedureWorkerSoInit.js';
import { SelectionParam } from '../../../bean/BoxSelection.js';
import { FrameAnimationStruct } from '../../../database/ui-worker/ProcedureWorkerFrameAnimation.js';

const INPUT_WORD =
  'This is the interval from when the task became eligible to run \n(e.g.because of notifying a wait queue it was a suspended on) to\n when it started running.';

export function getTimeString(ns: number): string {
  if (ns === 0) {
    return '0';
  }
  let currentTimeNs = ns;
  let hour1 = 3600_000_000_000;
  let minute1 = 60_000_000_000;
  let second1 = 1_000_000_000; // 1 second
  let millisecond1 = 1_000_000; // 1 millisecond
  let microsecond1 = 1_000; // 1 microsecond
  let res = '';
  if (currentTimeNs >= hour1) {
    res += Math.floor(currentTimeNs / hour1) + 'h ';
    currentTimeNs = currentTimeNs - Math.floor(currentTimeNs / hour1) * hour1;
  }
  if (currentTimeNs >= minute1) {
    res += Math.floor(currentTimeNs / minute1) + 'm ';
    currentTimeNs = currentTimeNs - Math.floor(ns / minute1) * minute1;
  }
  if (currentTimeNs >= second1) {
    res += Math.floor(currentTimeNs / second1) + 's ';
    currentTimeNs = currentTimeNs - Math.floor(currentTimeNs / second1) * second1;
  }
  if (currentTimeNs >= millisecond1) {
    res += Math.floor(currentTimeNs / millisecond1) + 'ms ';
    currentTimeNs = currentTimeNs - Math.floor(currentTimeNs / millisecond1) * millisecond1;
  }
  if (currentTimeNs >= microsecond1) {
    res += Math.floor(currentTimeNs / microsecond1) + 'μs ';
    currentTimeNs = currentTimeNs - Math.floor(currentTimeNs / microsecond1) * microsecond1;
  }
  if (currentTimeNs > 0) {
    res += currentTimeNs + 'ns ';
  }
  return res;
}

@element('tabpane-current-selection')
export class TabPaneCurrentSelection extends BaseElement {
  weakUpBean: WakeupBean | null | undefined;
  private currentSelectionTbl: LitTable | null | undefined;
  private tableObserver: MutationObserver | undefined;
  private scrollView: HTMLDivElement | null | undefined;
  // @ts-ignore
  private dpr: any = window.devicePixelRatio || window.webkitDevicePixelRatio || window.mozDevicePixelRatio || 1;

  set data(currentSelectionValue: any) {
    if (currentSelectionValue !== undefined && currentSelectionValue.constructor
      && currentSelectionValue.constructor.name !== 'SelectionParam') {
      this.setCpuData(currentSelectionValue);
    }
  }

  setCpuData(
    data: CpuStruct,
    callback: ((data: WakeupBean | null) => void) | undefined = undefined,
    scrollCallback?: (data: CpuStruct) => void
  ): void {
    this.setTableHeight('550px');
    let leftTitle: HTMLElement | null | undefined = this?.shadowRoot?.querySelector('#leftTitle');
    if (leftTitle) {
      leftTitle.innerText = 'Slice Details';
    }
    let list: any[] = [];
    let process = this.transferString(data.processName || 'Process');
    let processId = data.processId || data.tid;
    let state = '';
    if (data.end_state) {
      state = Utils.getEndState(data.end_state);
    } else if (data.end_state === '' || data.end_state === null) {
      state = '';
    } else {
      state = 'Unknown State';
    }
    list.push({
      name: 'Process',
      value: `${ process || 'Process' } [${ processId }]`,
    });
    let name = this.transferString(data.name ?? '');
    if (data.processId) {
      list.push({
        name: 'Thread',
        value: `<div style="white-space: nowrap;display: flex;align-items: center">
<div style="white-space:pre-wrap">${ name || 'Process' } [${ data.tid }]</div>
<lit-icon style="cursor:pointer;margin-left: 5px" id="thread-id" name="select" color="#7fa1e7" size="20"></lit-icon>
</div>`,
      });
    } else {
      list.push({
        name: 'Thread',
        value: `<div style="white-space: nowrap;display: flex;align-items: center">
<div style="white-space:pre-wrap">${ name || 'Process' } [${ data.tid }]</div>
</div>`,
      });
    }

    list.push({ name: 'CmdLine', value: `${ data.processCmdLine }` });
    list.push({
      name: 'StartTime(Relative)',
      value: getTimeString(data.startTime || 0),
    });
    list.push({
      name: 'StartTime(Absolute)',
      value: ((data.startTime || 0) + (window as any).recordStartNS) / 1000000000,
    });
    list.push({ name: 'Duration', value: getTimeString(data.dur || 0) });
    list.push({ name: 'Prio', value: data.priority || 0 });
    list.push({ name: 'End State', value: state });
    Promise.all([this.queryThreadStateDArgs(data.argSetID), this.queryCPUWakeUpFromData(data)]).then((resArr) => {
      let args = resArr[0];
      let bean = resArr[1];
      if (callback) {
        callback(bean);
      }
      if (args.length > 0) {
        args.forEach((arg) => {
          list.push({ name: arg.keyName, value: arg.strValue });
        });
      }
      this.currentSelectionTbl!.dataSource = list;
      let rightArea: HTMLElement | null | undefined = this?.shadowRoot?.querySelector('#table-right');
      let rightTitle: HTMLElement | null | undefined = this?.shadowRoot?.querySelector('#rightTitle');
      let rightButton: HTMLElement | null | undefined = this?.shadowRoot
                                                            ?.querySelector('#rightButton')
                                                            ?.shadowRoot?.querySelector('#custom-button');
      let rightStar: HTMLElement | null | undefined = this?.shadowRoot?.querySelector('#right-star');
      let threadClick = this.currentSelectionTbl?.shadowRoot?.querySelector('#thread-id');
      threadClick?.addEventListener('click', () => {
        //cpu点击
        if (scrollCallback) {
          scrollCallback(data);
        }
      });
      let canvas = this.initCanvas();
      if (bean !== null) {
        this.weakUpBean = bean;
        if (rightArea !== null && rightArea) {
          rightArea.style.visibility = 'visible';
        }
        if (rightTitle !== null && rightTitle) {
          rightTitle.style.visibility = 'visible';
          rightButton!.style.visibility = 'visible';
          rightStar!.style.visibility = 'hidden';
          SpSystemTrace.btnTimer = null;
        }
        this.drawRight(canvas, bean);
      } else {
        this.weakUpBean = null;
        if (rightArea !== null && rightArea) {
          rightArea.style.visibility = 'hidden';
        }
        if (rightTitle !== null && rightTitle) {
          rightTitle.style.visibility = 'hidden';
          rightButton!.style.visibility = 'hidden';
          rightStar!.style.visibility = 'hidden';
        }
      }
    });
  }

  setFunctionData(data: FuncStruct, scrollCallback: Function): void {
    //方法信息
    this.tabCurrentSelectionInit('Slice Details');
    let list: any[] = [];
    let name = this.transferString(data.funName ?? '');
    let isBinder = FuncStruct.isBinder(data);
    let isAsyncBinder = isBinder && FuncStruct.isBinderAsync(data);
    if (data.argsetid !== undefined && data.argsetid !== null) {
      this.setTableHeight('550px');
      if (isAsyncBinder) {
        Promise.all([
          queryBinderByArgsId(data.argsetid!, data.startTs!, !data.funName!.endsWith('rcv')),
          queryBinderArgsByArgset(data.argsetid),
        ]).then((result) => {
          let asyncBinderRes = result[0];
          let argsBinderRes = result[1];
          let asyncBinderStract: any;
          if (asyncBinderRes.length > 0) {
            asyncBinderRes[0].type = TraceRow.ROW_TYPE_FUNC;
            asyncBinderStract = asyncBinderRes[0];
          }
          if (argsBinderRes.length > 0) {
            argsBinderRes.forEach((item) => {
              list.push({
                name: item.keyName,
                value: item.strValue,
              });
            });
          }
          if (asyncBinderStract !== undefined) {
            list.unshift({
              name: 'Name',
              value: `<div style="white-space: nowrap;display: flex;align-items: center">
<div style="white-space:pre-wrap">${ name || 'binder' }</div>
<lit-icon style="cursor:pointer;transform: scaleX(-1);margin-left: 5px" id="function-jump" name="select" color="#7fa1e7" size="20"></lit-icon>
</div>`,
            });
          } else {
            list.unshift({ name: 'Name', value: name });
          }
          this.addTabPanelContent(list, data);
          this.currentSelectionTbl!.dataSource = list;
          let funcClick = this.currentSelectionTbl?.shadowRoot?.querySelector('#function-jump');
          funcClick?.addEventListener('click', () => {
            scrollCallback(asyncBinderStract);
          });
        });
      } else if (isBinder) {
        queryBinderArgsByArgset(data.argsetid).then((argset) => {
          let binderSliceId = -1;
          argset.forEach((item) => {
            if (item.keyName === 'destination slice id') {
              binderSliceId = Number(item.strValue);
              list.unshift({
                name: 'Name',
                value: `<div style="white-space: nowrap;display: flex;align-items: center">
<div style="white-space:pre-wrap">${ name || 'binder' }</div>
<lit-icon style="cursor:pointer;transform: scaleX(-1);margin-left: 5px" id="function-jump" name="select" color="#7fa1e7" size="20"></lit-icon>
</div>`,
              });
            }
            list.push({ name: item.keyName, value: item.strValue });
          });
          if (binderSliceId === -1) {
            list.unshift({ name: 'Name', value: name });
          }
          this.addTabPanelContent(list, data);
          this.currentSelectionTbl!.dataSource = list;
          let funcClick = this.currentSelectionTbl?.shadowRoot?.querySelector('#function-jump');
          funcClick?.addEventListener('click', () => {
            if (!Number.isNaN(binderSliceId) && binderSliceId !== -1) {
              queryBinderBySliceId(binderSliceId).then((result: any[]) => {
                if (result.length > 0) {
                  result[0].type = TraceRow.ROW_TYPE_FUNC;
                  scrollCallback(result[0]);
                }
              });
            }
          });
        });
      } else {
        queryBinderArgsByArgset(data.argsetid).then((argset) => {
          list.push({ name: 'Name', value: name });
          argset.forEach((item) => {
            list.push({ name: item.keyName, value: item.strValue });
          });
          this.addTabPanelContent(list, data);
          this.currentSelectionTbl!.dataSource = list;
        });
      }
    } else {
      this.setTableHeight('auto');
      list.push({ name: 'Name', value: name });
      list.push({
        name: 'StartTime',
        value: getTimeString(data.startTs || 0),
      });
      list.push({
        name: 'Duration',
        value: getTimeString(data.dur || 0),
      });
      list.push({ name: 'depth', value: data.depth });
      this.currentSelectionTbl!.dataSource = list;
    }
  }

  private addTabPanelContent(contentList: any[], data: FuncStruct): void{
    contentList.push({
      name: 'StartTime',
      value: getTimeString(data.startTs || 0),
    });
    contentList.push({
      name: 'Duration',
      value: getTimeString(data.dur || 0),
    });
    contentList.push({ name: 'depth', value: data.depth });
    contentList.push({ name: 'arg_set_id', value: data.argsetid });
  }

  private tabCurrentSelectionInit(leftTitleStr: string): void {
    this.initCanvas();
    let leftTitle: HTMLElement | null | undefined = this?.shadowRoot?.querySelector('#leftTitle');
    this.setTitleAndButtonStyle();
    if (leftTitle) {
      leftTitle.innerText = leftTitleStr;
    }
  }

  private setTitleAndButtonStyle(): void{
    let rightTitle: HTMLElement | null | undefined = this?.shadowRoot?.querySelector('#rightTitle');
    let rightButton: HTMLElement | null | undefined = this?.shadowRoot?.querySelector('#rightButton')?.
      shadowRoot?.querySelector('#custom-button');
    let rightStar: HTMLElement | null | undefined = this?.shadowRoot?.querySelector('#right-star');
    if (rightTitle) {
      rightTitle.style.visibility = 'hidden';
      rightButton!.style.visibility = 'hidden';
      rightStar!.style.visibility = 'hidden';
    }
  }

  setClockData(data: ClockStruct): void {
    this.setTableHeight('auto');
    //时钟信息
    this.tabCurrentSelectionInit('Counter Details');
    let list: any[] = [];
    list.push({
      name: 'Start time',
      value: getTimeString(data.startNS || 0),
    });
    list.push({
      name: 'Value',
      value: ColorUtils.formatNumberComma(data.value || 0),
    });
    list.push({ name: 'Duration', value: getTimeString(data.dur || 0) });
    this.currentSelectionTbl!.dataSource = list;
  }

  setMemData(data: ProcessMemStruct): void {
    this.setTableHeight('auto');
    //时钟信息
    this.initCanvas();
    let leftTitle: HTMLElement | null | undefined = this?.shadowRoot?.querySelector('#leftTitle');
    if (leftTitle) {
      leftTitle.innerText = 'Counter Details';
    }
    let list: any[] = [];
    list.push({
      name: 'Start time',
      value: getTimeString(data.startTime || 0),
    });
    list.push({ name: 'Value', value: data.value });
    list.push({ name: 'Delta', value: data.delta });
    list.push({
      name: 'Duration',
      value: getTimeString(data.duration || 0),
    });
    this.currentSelectionTbl!.dataSource = list;
  }

  setIrqData(data: IrqStruct): void {
    this.setTableHeight('550px');
    this.initCanvas();
    this.setTitleAndButtonStyle();
    let leftTitle: HTMLElement | null | undefined = this?.shadowRoot?.querySelector('#leftTitle');
    if (leftTitle) {
      leftTitle.innerText = 'Counter Details';
    }
    let list: any[] = [];
    list.push({
      name: 'Start time',
      value: getTimeString(data.startNS || 0),
    });
    list.push({ name: 'Name', value: data.name });
    list.push({ name: 'Duration', value: getTimeString(data.dur || 0) });
    queryBinderArgsByArgset(data.argSetId || 0).then((argsBinderRes) => {
      if (argsBinderRes.length > 0) {
        argsBinderRes.forEach((item) => {
          list.push({ name: item.keyName, value: item.strValue });
        });
      }
      this.currentSelectionTbl!.dataSource = list;
    });
  }

  setThreadData(
    data: ThreadStruct,
    scrollCallback: ((d: any) => void) | undefined,
    scrollWakeUp: (d: any) => void | undefined,
    scrollPreviousData: (d: any) => void | undefined,
    scrollNextData: (d: any) => void | undefined
  ): void {
    //线程信息
    this.setTableHeight('550px');
    this.initCanvas();
    let leftTitle: HTMLElement | null | undefined = this?.shadowRoot?.querySelector('#leftTitle');
    this.setTitleAndButtonStyle();
    if (leftTitle) {
      leftTitle.innerText = 'Thread State';
    }
    let list: any[] = [];
    list.push({
      name: 'StartTime',
      value: getTimeString(data.startTime || 0),
    });
    list.push({ name: 'Duration', value: getTimeString(data.dur || 0) });
    let state;
    if (data.state) {
      state = Utils.getEndState(data.state);
    } else if (data.state === '' || data.state === null) {
      state = '';
    } else {
      state = 'Unknown State';
    }
    if ('Running' === state) {
      state = state + ' on CPU ' + data.cpu;
    }
    if (data.cpu === null || data.cpu === undefined) {
      list.push({ name: 'State', value: `${ state }` });
    } else {
      list.push({
        name: 'State',
        value: `<div style="white-space: nowrap;display: flex;align-items: center">
            <div style="white-space:pre-wrap">${ state }</div>
            <lit-icon style="cursor:pointer;transform: scaleX(-1);margin-left: 5px" id="state-click" name="select" color="#7fa1e7" size="20"></lit-icon>
            </div>`,
      });
    }
    let slice = Utils.SCHED_SLICE_MAP.get(`${ data.id }-${ data.startTime }`);
    if (slice) {
      list.push({ name: 'Prio', value: `${ slice.priority }` });
    }
    let processName = Utils.PROCESS_MAP.get(data.pid!);
    if (processName === null || processName === undefined || processName === '' || processName.toLowerCase() == 'null') {
      processName = Utils.THREAD_MAP.get(data.tid!) || 'null';
    }
    list.push({
      name: 'Process',
      value: this.transferString(processName ?? '') + ' [' + data.pid + '] ',
    });
    let ThreadRow = document.querySelector('body > sp-application')!.shadowRoot!.querySelector('#sp-system-trace')!.shadowRoot?.querySelector<TraceRow<ThreadStruct>>(
      `trace-row[row-id='${data.tid}'][row-type='thread']`
    );
    ThreadRow?.dataList.forEach((item, index) => {
        if (item === data && index !== 0) {
          let previousState = ThreadRow?.dataList[index - 1].state;
          if (previousState === 'R') {
            previousState = 'Runnable';
          }
          if (previousState === 'S') {
            previousState = 'Sleeping';
          }
          list.push({
            name: 'Previous State',
            value: `<div style="white-space: nowrap;display: flex;align-items: center">
              <div style="white-space:pre-wrap">${previousState}</div>
              <lit-icon style="cursor:pointer;transform: scaleX(-1);margin-left: 5px" id="previous-state-click" name="select" color="#7fa1e7" size="20"></lit-icon>
              </div>`,
          });
        }
        if (item === data && index !== ThreadRow?.dataList.length! - 1) {
          let nextState = ThreadRow?.dataList[index + 1].state;
          if (nextState === 'R') {
            nextState = 'Runnable';
          }
          if (nextState === 'S') {
            nextState = 'Sleeping';
          }
          list.push({
            name: 'Next State',
            value: `<div style="white-space: nowrap;display: flex;align-items: center">
              <div style="white-space:pre-wrap">${nextState}</div>
              <lit-icon style="cursor:pointer;transform: scaleX(-1);margin-left: 5px" id="next-state-click" name="select" color="#7fa1e7" size="20"></lit-icon>
              </div>`,
          });
        }
      }
    );
    let cpu = new CpuStruct();
    cpu.id = data.id;
    cpu.startTime = data.startTime;
    Promise.all([
      this.queryThreadWakeUpFromData(data.id!, data.startTime!, data.dur!),
      this.queryThreadWakeUpData(data.id!, data.startTime!, data.dur!),
      this.queryThreadStateDArgs(data.argSetID),
    ]).then((result) => {
      let fromBean = result[0];
      let wakeUps = result[1];
      let args = result[2];
      if (fromBean !== null && fromBean !== undefined && fromBean.pid !== 0 && fromBean.tid !== 0) {
        list.push({
          name: 'wakeup from tid',
          value: `<div style="white-space: nowrap;display: flex;align-items: center">
            <div style="white-space:pre-wrap">${ fromBean.tid }</div>
            <lit-icon style="cursor:pointer;transform: scaleX(-1);margin-left: 5px" id="wakeup-from"  class="wakeup-click"  name="select" color="#7fa1e7" size="20"></lit-icon>
            </div>`,
        });
      }
      if (wakeUps !== null) {
        wakeUps.map((e) => {
          list.push({
            name: 'wakeup tid',
            value: `<div style="white-space: nowrap;display: flex;align-items: center">
            <div style="white-space:pre-wrap">${ e.tid }</div>
            <lit-icon style="cursor:pointer;transform: scaleX(-1);margin-left: 5px" id="wakeup-${ e.tid }" class="wakeup-click" name="select" color="#7fa1e7" size="20"></lit-icon>
            </div>`,
          });
        });
      }
      if (args.length > 0) {
        args.forEach((arg) => {
          list.push({ name: arg.keyName, value: arg.strValue });
        });
      }
      this.currentSelectionTbl!.dataSource = list;
      this.currentSelectionTbl?.shadowRoot?.querySelector('#next-state-click')?.addEventListener('click', () => {
        if (scrollNextData) {
          scrollNextData(data);
        }
      });
      this.currentSelectionTbl?.shadowRoot?.querySelector('#previous-state-click')?.addEventListener('click', () => {
        if (scrollPreviousData) {
          scrollPreviousData(data);
        }
      });
      this.currentSelectionTbl?.shadowRoot?.querySelector('#state-click')?.addEventListener('click', () => {
        //线程点击
        if (scrollCallback) {
          scrollCallback(data);
        }
      });
      this.currentSelectionTbl?.shadowRoot?.querySelector('#wakeup-from')?.addEventListener('click', () => {
        //点击跳转，唤醒和被唤醒的 线程
        if (fromBean && scrollWakeUp) {
          scrollWakeUp({
            processId: fromBean.pid,
            tid: fromBean.tid,
            startTime: fromBean.ts,
          });
        }
      });
      if (wakeUps) {
        wakeUps.map((up) => {
          this.currentSelectionTbl?.shadowRoot?.querySelector(`#wakeup-${ up.tid }`)?.addEventListener('click', () => {
            //点击跳转，唤醒和被唤醒的 线程
            if (up && scrollWakeUp !== undefined) {
              scrollWakeUp({
                tid: up.tid,
                startTime: up.ts,
                processId: up.pid,
              });
            }
          });
        });
      }
    });
  }

  setJankData(
    data: JankStruct,
    callback: ((data: Array<any>) => void) | undefined = undefined,
    scrollCallback: ((d: any) => void) | undefined
  ): void {
    //线程信息
    this.setTableHeight('750px');
    this.tabCurrentSelectionInit('Slice Details');
    let list: any[] = [];
    this.setJankCommonMessage(list, data);
    if (data.type == '0') {
      this.setJankType(data, list);
      let jankJumperList = new Array<JankTreeNode>();
      if (data.frame_type === 'render_service') {
        queryGpuDur(data.id!).then((it) => {
          if (it.length > 0) {
            list.push({ name: `<div>Gpu Duration</div>`, value: getTimeString(it[0].gpu_dur) });
          }
        });
        if (data.src_slice) {
          queryFlowsData(data.src_slice!.split(',')).then((it) => {
            if (it.length > 0) {
              list.push({
                name: `<div style="padding:5px 0 5px 0;">FrameTimeLine flows</div>`,
                value: '',
              });
              it.forEach((a: any) => {
                let appNode = new JankTreeNode(a.name, a.pid, 'app');
                appNode.children.push(new JankTreeNode(a.name, a.pid, 'frameTime'));
                jankJumperList.push(appNode);
                list.push({
                  name: `<div>Slice</div>`,
                  value:
                    a.cmdline +
                    ' [' +
                    a.name +
                    ']' +
                    `<lit-icon  class="jank_cla" style="display: inline-flex;cursor: pointer;transform: scaleX(-1);margin-left: 5px" id="actual frameTime" slice_name="${ a.name }" pid="${ a.pid }" name="select" color="#7fa1e7" size="20"></lit-icon>`,
                });
              });
              list.push({
                name: `<div style="padding:5px 0 5px 0;">Following flows</div>`,
                value: '',
              });
              it.forEach((a: any) => {
                list.push({
                  name: `<div>Slice</div>`,
                  value:
                    a.cmdline +
                    ' [' +
                    a.name +
                    ']' +
                    `<lit-icon class="jank_cla" style="display: inline-flex;cursor:pointer;transform: scaleX(-1);margin-left: 5px" id="${ a.type }-${ a.pid }" slice_name="${ a.name }"  pid="${ a.pid }" name="select" color="#7fa1e7" size="20"></lit-icon>`,
                });
              });
              this.currentSelectionTbl!.dataSource = list;
              this.addJankScrollCallBackEvent(scrollCallback, callback, jankJumperList);
            }
          });
        } else {
          this.currentSelectionTbl!.dataSource = list;
        }
      } else if (data.frame_type === 'app') {
        list.push({
          name: `<div style="padding:5px 0 5px 0;">FrameTimeLine flows</div>`,
          value: '',
        });
        list.push({
          name: `<div>Slice</div>`,
          value:
            data.cmdline +
            ' [' +
            data.name +
            ']' +
            `<lit-icon  class="jank_cla" style="display: inline-flex;cursor:pointer;transform: scaleX(-1);margin-left: 5px" id="actual frameTime"  slice_name="${ data.name }"  pid="${ data.pid }" name="select" color="#7fa1e7" size="20"></lit-icon>`,
        });
        let timeLineNode = new JankTreeNode(data.name!, data.pid!, 'frameTime');
        jankJumperList.push(timeLineNode);
        if (data.dst_slice) {
          queryPrecedingData(data.dst_slice).then((it) => {
            if (it.length > 0) {
              list.push({
                name: `<div style="padding:5px 0 5px 0;">Preceding flows</div>`,
                value: '',
              });
              it.forEach((a: any) => {
                let rsNode = new JankTreeNode(a.name, a.pid, 'render_service');
                jankJumperList.push(rsNode);
                list.push({
                  name: `<div>Slice</div>`,
                  value:
                    a.cmdline +
                    ' [' +
                    a.name +
                    ']' +
                    `<lit-icon class="jank_cla" style="display: inline-flex;cursor:pointer;transform: scaleX(-1);margin-left: 5px" id="${ a.type }-${ a.pid }" slice_name="${ a.name }" pid="${ a.pid }" name="select" color="#7fa1e7" size="20"></lit-icon>`,
                });
              });
              this.currentSelectionTbl!.dataSource = list;
              this.addJankScrollCallBackEvent(scrollCallback, callback, jankJumperList);
            }
          });
        } else {
          this.currentSelectionTbl!.dataSource = list;
          this.addJankScrollCallBackEvent(scrollCallback, callback, jankJumperList);
        }
      } else if (data.frame_type === 'frameTime') {
        queryGpuDur(data.id!).then((it) => {
          if (it.length > 0) {
            list.push({
              name: `<div>Gpu Duration</div>`,
              value: getTimeString(it[0].gpu_dur),
            });
          }
          if (data.name) {
            list.push({
              name: `<div style="padding:5px 0 5px 0;">App Frame</div>`,
              value: '',
            });
            list.push({
              name: `<div>Process</div>`,
              value: data.cmdline + ' ' + data.pid,
            });
            list.push({
              name: `<div>start time</div>`,
              value: getTimeString(data.ts || 0),
            });
            list.push({
              name: `<div>end time</div>`,
              value: getTimeString(data!.ts! + data.dur! || 0),
            });
          }
          if (data.rs_name) {
            list.push({
              name: `<div style="padding:5px 0 5px 0;">RenderService Frame</div>`,
              value: '',
            });
            list.push({
              name: `<div>Process</div>`,
              value: data.rs_name + ' ' + data.rs_pid,
            });
            list.push({
              name: `<div>start time</div>`,
              value: getTimeString(data.rs_ts || 0),
            });
            list.push({
              name: `<div>end time</div>`,
              value: getTimeString(data.rs_ts! + data.rs_dur! || 0),
            });
          }
          list.push({
            name: `<div style="padding:5px 0 5px 0;">Following</div>`,
            value: '',
          });
          list.push({
            name: `<div>Slice</div>`,
            value:
              data.cmdline +
              ' [' +
              data.name +
              ']' +
              `<lit-icon class="jank_cla" style="display: inline-flex;cursor:pointer;transform: scaleX(-1);margin-left: 5px" id="${ data.type }-${ data.pid }" slice_name="${ data.name }"  pid="${ data.pid }" name="select" color="#7fa1e7" size="20"></lit-icon>`,
          });
          let appNode = new JankTreeNode(data.name!, data.pid!, 'app');
          let rsNode = new JankTreeNode(data.rs_vsync!, data.rs_pid!, 'render_service');
          appNode.children.push(rsNode);
          jankJumperList.push(appNode);
          this.currentSelectionTbl!.dataSource = list;
          this.addJankScrollCallBackEvent(scrollCallback, callback, jankJumperList);
        });
      }
    } else {
      this.currentSelectionTbl!.dataSource = list;
    }
  }

  setStartupData(data: AppStartupStruct, scrollCallback: Function): void {
    this.setTableHeight('550px');
    this.initCanvas();
    let rightButton: HTMLElement | null | undefined = this?.shadowRoot?.querySelector('#rightButton')?.shadowRoot?.
      querySelector('#custom-button');
    let startUpRightTitle: HTMLElement | null | undefined = this?.shadowRoot?.querySelector('#rightTitle');
    if (startUpRightTitle) {
      startUpRightTitle.style.visibility = 'hidden';
      rightButton!.style.visibility = 'hidden';
    }
    let startUpLeftTitle: HTMLElement | null | undefined = this?.shadowRoot?.querySelector('#leftTitle');
    if (startUpLeftTitle) {
      startUpLeftTitle.innerText = 'Details';
    }
    let list: any[] = [];
    list.push({ name: 'Name', value: AppStartupStruct.getStartupName(data.startName) });
    list.push({
      name: 'StartTime(Relative)',
      value: `
      <div style="display: flex;white-space: nowrap;align-items: center">
<div style="white-space:pre-wrap">${ getTimeString(data.startTs || 0) }</div>
<lit-icon style="cursor:pointer;transform: scaleX(-1);margin-left: 5px" id="start-jump" name="select" color="#7fa1e7" size="20"></lit-icon>
</div>`,
    });
    list.push({
      name: 'StartTime(Absolute)',
      value: ((data.startTs || 0) + (window as any).recordStartNS) / 1000000000,
    });
    if (data.dur && data.dur > 0) {
      list.push({
        name: 'EndTime(Relative)',
        value: `<div style="white-space: nowrap;display: flex;align-items: center">
<div style="white-space:pre-wrap">${ getTimeString((data.startTs || 0) + (data.dur || 0)) }</div>
<lit-icon style="cursor:pointer;transform: scaleX(-1);margin-left: 5px" id="end-jump" name="select" color="#7fa1e7" size="20"></lit-icon>
</div>`,
      });
      list.push({
        name: 'EndTime(Absolute)',
        value: ((data.startTs || 0) + (data.dur || 0) + (window as any).recordStartNS) / 1000000000,
      });
    } else {
      list.push({
        name: 'EndTime(Relative)',
        value: `Unknown Time`,
      });
      list.push({
        name: 'EndTime(Absolute)',
        value: 'Unknown Time',
      });
    }
    list.push({ name: 'Duration', value: getTimeString(data.dur || 0) });
    this.currentSelectionTbl!.dataSource = list;
    let startIcon = this.currentSelectionTbl?.shadowRoot?.querySelector('#start-jump');
    let endIcon = this.currentSelectionTbl?.shadowRoot?.querySelector('#end-jump');
    let scrollClick = (type: number): void => {
      let recordNs: number = (window as any).recordStartNS;
      let useEnd = type === 1 && data.startName! < 4;
      queryThreadByItid(
        useEnd ? data.endItid! : data.itid!,
        useEnd ? recordNs + data.startTs! + data.dur! : recordNs + data.startTs!
      ).then((result) => {
        if (result.length > 0) {
          let pt: { pid: number; tid: number; dur: number, name: string, depth: number } = result[0];
          scrollCallback({
            pid: pt.pid,
            tid: pt.tid,
            type: 'func',
            dur: pt.dur,
            depth: pt.depth,
            funName: pt.name,
            startTime: useEnd ? (data.startTs || 0) + (data.dur || 0) : data.startTs,
            keepOpen: true,
          });
        }
      });
    };
    if (startIcon) {
      startIcon.addEventListener('click', () => scrollClick(0));
    }
    if (endIcon) {
      endIcon.addEventListener('click', () => scrollClick(1));
    }
  }

  setStaticInitData(data: SoStruct, scrollCallback: Function): void {
    this.setTableHeight('550px');
    this.initCanvas();
    let rightTitle: HTMLElement | null | undefined = this?.shadowRoot?.querySelector('#rightTitle');
    let rightButton: HTMLElement | null | undefined = this?.shadowRoot
                                                          ?.querySelector('#rightButton')
                                                          ?.shadowRoot?.querySelector('#custom-button');
    if (rightTitle) {
      rightTitle.style.visibility = 'hidden';
      rightButton!.style.visibility = 'hidden';
    }
    let leftTitle: HTMLElement | null | undefined = this?.shadowRoot?.querySelector('#leftTitle');
    if (leftTitle) {
      leftTitle.innerText = 'Details';
    }
    let list: any[] = [];
    list.push({ name: 'Name', value: data.soName });
    list.push({
      name: 'StartTime(Relative)',
      value: `<div style="white-space: nowrap;display: flex;align-items: center">
<div style="white-space:pre-wrap">${ getTimeString(data.startTs || 0) }</div>
<lit-icon id="start-jump" style="cursor:pointer;transform: scaleX(-1);margin-left: 5px" name="select" color="#7fa1e7" size="20"></lit-icon>
</div>`,
    });
    list.push({
      name: 'StartTime(Absolute)',
      value: ((data.startTs || 0) + (window as any).recordStartNS) / 1000000000,
    });
    list.push({ name: 'Duration', value: getTimeString(data.dur || 0) });
    this.currentSelectionTbl!.dataSource = list;
    let startIcon = this.currentSelectionTbl?.shadowRoot?.querySelector('#start-jump');
    if (startIcon) {
      startIcon.addEventListener('click', () => {
        let recordNs: number = (window as any).recordStartNS;
        queryThreadByItid(data.itid!, recordNs + data.startTs!).then((result) => {
          if (result.length > 0) {
            let pt: { pid: number; tid: number; dur: number, name: string, depth: number } = result[0];
            scrollCallback({
              pid: pt.pid,
              tid: pt.tid,
              type: 'func',
              dur: pt.dur,
              depth: pt.depth,
              funName: pt.name,
              startTime: data.startTs,
              keepOpen: true,
            });
          }
        });
      });
    }
  }

  async setFrameAnimationData(
    data: FrameAnimationStruct
  ): Promise<void> {
    this.setTableHeight('550px');
    this.tabCurrentSelectionInit('Animation Details');
    let list = [];
    let dataTs: number = data.ts < 0 ? 0 : data.ts;
    list.push({ name: 'Name', value: data.animationId });
    list.push({ name: 'Start time', value: `${ Utils.getTimeString(dataTs) }` });
    list.push({ name: 'End time', value: `${ Utils.getTimeString(dataTs + (data.dur || 0)) }` });
    list.push({ name: 'Duration', value: `${ Utils.getTimeString(data.dur || 0) }`, });
    if (data.status === 'Completion delay') {
      let result = await queryAnimationFrameFps(dataTs, dataTs + data.dur);
      if (result.length > 0) {
        let fixedNumber: number = 2;
        let fpsValue: number = result[0].fps / (data.dur / 1000_000_000) ;
        list.push({ name: 'FPS', value: `${ fpsValue.toFixed(fixedNumber) || 0 }` });
      }
    }
    this.currentSelectionTbl!.dataSource = list;
  }

  private setJankType(data: JankStruct, list: any[]): void {
    if (data.jank_tag === 1) {
      if (data.frame_type === 'render_service') {
        list.push({ name: 'Jank Type', value: 'RenderService Deadline Missed' });
      } else if (data.frame_type === 'app') {
        list.push({ name: 'Jank Type', value: 'APP Deadline Missed' });
      } else if (data.frame_type === 'frameTime') {
        list.push({ name: 'Jank Type', value: 'Deadline Missed' });
      }
    } else if (data.jank_tag === 3) {
      list.push({ name: 'Jank Type', value: 'Deadline Missed' });
    } else {
      list.push({ name: 'Jank Type', value: 'NONE' });
    }
  }

  private setJankCommonMessage(list: any[], data: JankStruct): void {
    list.push({ name: 'Name', value: data.name });
    list.push({ name: 'StartTime', value: getTimeString(data.ts || 0) });
    list.push({ name: 'Absolute Time', value: ((window as any).recordStartNS + data.ts) / 1000000000 });
    list.push({ name: 'Duration', value: data.dur ? getTimeString(data.dur) : ' ' });
    if (data.frame_type !== 'frameTime') {
      list.push({ name: 'Process', value: data.cmdline + ' ' + data.pid });
    }
  }

  private setTableHeight(height: string): void {
    this.scrollView!.scrollTop = 0;
    this.currentSelectionTbl!.style.height = height;
  }

  private addJankScrollCallBackEvent(
    scrollCallback: ((d: any) => void) | undefined,
    callback: ((data: Array<any>) => void) | undefined,
    jankJumperList: JankTreeNode[]
  ): void {
    let all = this.currentSelectionTbl?.shadowRoot?.querySelectorAll(`.jank_cla`);
    all!.forEach((a) => {
      a.addEventListener('click', () => {
        if (scrollCallback) {
          scrollCallback({
            rowId: a.id,
            name: a.getAttribute('slice_name'),
            pid: a.getAttribute('pid'),
          });
        }
      });
    });
    if (callback) {
      callback(jankJumperList);
    }
  }

  async queryThreadStateDArgs(argSetID: number | undefined): Promise<BinderArgBean[]> {
    let list: Array<BinderArgBean> = [];
    if (argSetID !== undefined) {
      list = await queryThreadStateArgs(argSetID);
    }
    return list;
  }

  /**
   * 查询出 线程被唤醒的 线程信息
   * @param data
   */
  async queryCPUWakeUpFromData(data: CpuStruct): Promise<WakeupBean | null> {
    let wb: WakeupBean | null = null;
    if (data.id === undefined || data.startTime === undefined) {
      return null;
    }
    let wakeup = await queryRunnableTimeByRunning(data.tid!, data.startTime);
    if (wakeup && wakeup[0]) {
      let wakeupTs = wakeup[0].ts as number;
      let recordStartTs = (window as any).recordStartNS;
      let wf = await queryThreadWakeUpFrom(data.id, wakeupTs);
      if (wf && wf[0]) {
        wb = wf[0];
        if (wb !== null) {
          wb.wakeupTime = wakeupTs - recordStartTs;
          wb.process = Utils.PROCESS_MAP.get(wb.pid!);
          wb.thread = Utils.THREAD_MAP.get(wb.tid!);
          wb.schedulingLatency = (data.startTime || 0) - (wb.wakeupTime || 0);
          if (wb.process === null) {
            wb.process = wb.thread;
          }
          if (wb.pid === undefined) {
            wb.pid = wb.tid;
          }
          wb.schedulingDesc = INPUT_WORD;
        }
      }
    }
    return wb;
  }

  /**
   * 查询出 线程被唤醒的 线程链信息
   * @param data
   */
  static async queryCPUWakeUpListFromBean(data: WakeupBean): Promise<WakeupBean | null> {
    let wb: WakeupBean | null = null;
    let wakeup = await queryRunnableTimeByRunning(data.tid!, data.ts!);
    if (wakeup && wakeup[0]) {
      let wakeupTs = wakeup[0].ts as number;
      let recordStartTs = (window as any).recordStartNS;
      let wf = await queryThreadWakeUpFrom(data.itid!, wakeupTs);
      if (wf && wf[0]) {
        wb = wf[0];
        if (wb !== null) {
          wb.wakeupTime = wakeupTs - recordStartTs;
          wb.process = Utils.PROCESS_MAP.get(wb.pid!);
          wb.thread = Utils.THREAD_MAP.get(wb.tid!);
          wb.schedulingLatency = (data.ts || 0) - (wb.wakeupTime || 0);
          if (wb.process === null) {
            wb.process = wb.thread;
          }
          if (wb.pid === undefined) {
            wb.pid = wb.tid;
          }
          wb.schedulingDesc = INPUT_WORD;
        }
      }
    }
    return wb;
  }

  /**
   * 查询出 线程唤醒了哪些线程信息
   */
  async queryThreadWakeUpFromData(itid: number, startTime: number, dur: number): Promise<WakeupBean | undefined> {
    let wakeUps = await queryThreadWakeUpFrom(itid, startTime + (window as any).recordStartNS);
    if (wakeUps !== undefined && wakeUps.length > 0) {
      return wakeUps[0];
    }
  }

  /**
   * 查询出 线程唤醒了哪些线程信息
   */
  async queryThreadWakeUpData(itid: number, startTime: number, dur: number): Promise<Array<WakeupBean>> {
    let list: Array<WakeupBean> = [];
    if (itid === undefined || startTime === undefined) {
      return list;
    }
    let wakeUps = await queryThreadWakeUp(itid, startTime, dur); //  3,4835380000
    if (wakeUps !== undefined && wakeUps.length > 0) {
      list.push(...wakeUps);
    }
    return list;
  }

  initCanvas(): HTMLCanvasElement | null {
    let canvas = this.shadowRoot!.querySelector<HTMLCanvasElement>('#rightDraw');
    let width = getComputedStyle(this.currentSelectionTbl!).getPropertyValue('width');
    let height = getComputedStyle(this.currentSelectionTbl!).getPropertyValue('height');
    if (canvas !== null) {
      canvas.width = Math.round(Number(width.replace('px', '')) * this.dpr);
      canvas.height = Math.round(Number(height.replace('px', '')) * this.dpr);
      canvas.style.width = width;
      canvas.style.height = height;
      canvas.getContext('2d')!.scale(this.dpr, this.dpr);
    }
    SpApplication.skinChange = (val: boolean): void => {
      this.drawRight(canvas, this.weakUpBean!);
    };
    return canvas;
  }

  drawRight(cavs: HTMLCanvasElement | null, wakeupBean: WakeupBean | null): void {
    if (cavs === null) {
      return;
    }
    let context = cavs.getContext('2d');
    if (context !== null) {
      //绘制竖线
      if (document.querySelector<SpApplication>('sp-application')!.dark) {
        context.strokeStyle = '#ffffff';
        context.fillStyle = '#ffffff';
      } else {
        context.strokeStyle = '#000000';
        context.fillStyle = '#000000';
      }
      context.lineWidth = 2;
      context.moveTo(10, 15);
      context.lineTo(10, 125);
      context.stroke();
      //绘制菱形
      context.lineWidth = 1;
      context.beginPath();
      context.moveTo(10, 30);
      context.lineTo(4, 40);
      context.lineTo(10, 50);
      context.lineTo(16, 40);
      context.lineTo(10, 30);
      context.closePath();
      context.fill();
      context.font = 12 + 'px sans-serif';
      //绘制wake up 文字
      let strList = [];
      strList.push('wakeup @ ' + getTimeString(wakeupBean?.wakeupTime || 0) + ' on CPU ' + wakeupBean?.cpu + ' by');
      strList.push('P:' + wakeupBean?.process + ' [ ' + wakeupBean?.pid + ' ]');
      strList.push('T:' + wakeupBean?.thread + ' [ ' + wakeupBean?.tid + ' ]');
      strList.forEach((str, index) => {
        if (context !== null) {
          context.fillText(str, 40, 40 + 16 * index);
        }
      });
      context.lineWidth = 2;
      context.lineJoin = 'bevel';
      context.moveTo(10, 95);
      context.lineTo(20, 90);
      context.moveTo(10, 95);
      context.lineTo(20, 100);
      context.moveTo(10, 95);
      context.lineTo(80, 95);
      context.lineTo(70, 90);
      context.moveTo(80, 95);
      context.lineTo(70, 100);
      context.stroke();
      //绘制latency
      context.font = 12 + 'px sans-serif';
      context.fillText('Scheduling latency:' + getTimeString(wakeupBean?.schedulingLatency || 0), 90, 100);
      //绘制最下方提示语句
      context.font = 10 + 'px sans-serif';
      INPUT_WORD.split('\n').forEach((str, index) => {
        context?.fillText(str, 90, 120 + 12 * index);
      });
    }
  }

  transferString(str: string): string {
    let s = '';
    if (str.length === 0) {
      return '';
    }
    s = str.replace(/&/g, '&amp;');
    s = s.replace(/</g, '&lt;');
    s = s.replace(/>/g, '&gt;');
    s = s.replace(/\'/g, '&#39;');
    s = s.replace(/\"/g, '&#quat;');
    return s;
  }

  initElements(): void {
    this.currentSelectionTbl = this.shadowRoot?.querySelector<LitTable>('#selectionTbl');
    this.scrollView = this.shadowRoot?.querySelector<HTMLDivElement>('#scroll_view');
    this.currentSelectionTbl?.addEventListener('column-click', (ev: any) => {});
  }

  addTableObserver(): void {
    let leftTable = this.shadowRoot?.querySelector('.table-left');
    this.tableObserver?.observe(leftTable!, {
      attributes: true,
      attributeFilter: ['style'],
      attributeOldValue: true,
    });
  }

  initHtml(): string {
    return `
        <style>
            .table-title{
                top: 0;
                background: var(--dark-background,#ffffff);
                position: sticky;
                width: 100%;
                display: flex;
            }
            .table-title > h2{
                font-size: 16px;
                font-weight: 400;
                visibility: visible;
                width: 50%;
                padding: 0 10px;
            }
            #rightTitle{
                width: 50%;
                display: flex;
                justify-content: space-between;
                padding: 0 10px;
                font-size: 16px;
                font-weight: 400;
                visibility: visible;
            }
            #rightTitle > h2{
                font-size: 16px;
                font-weight: 400;
            }           
            #rightButton{
                padding-top:12px;
            }
            .right{
                display: flex;
            }
            #right-star{
                padding-top: 10px;
                visibility: hidden;
            }
            .scroll-area{
                display: flex;
                flex-direction: row;
                flex: 1;
            }
            .table-left{
                width: 50%;
                height: auto;
                padding: 0 10px;
            }
            .table-right{
                width: 50%;
            }
        </style>
        <div id="scroll_view" style="display: flex;flex-direction: column;width: 100%;height: 100%;overflow: auto">
            <div style="width: 100%;height: auto;position: relative">
                <div class="table-title">
                    <h2 id="leftTitle"></h2>
                    <div id="rightTitle" >
                        <h2 id="rightText">Scheduling Latency</h2>
                        <div class="right">
                        <lit-button id="rightButton"  height="32px" width="164px" color="black" font_size="14px" border="1px solid black" 
                        >GetWakeupList</lit-button>
                        <lit-icon id="right-star" class="collect" name="star-fill" size="30"></lit-icon>
                        </div>
                    </div>
                </div>
            </div>
            <div class="scroll-area">
                <lit-table id="selectionTbl" class="table-left" no-head hideDownload>
                        <lit-table-column title="name" data-index="name" key="name" align="flex-start"  width="180px">
                            <template><div>{{name}}</div></template>
                        </lit-table-column>
                        <lit-table-column title="value" data-index="value" key="value" align="flex-start" >
                            <template><div style="display: flex;">{{value}}</div></template>
                        </lit-table-column>
                </lit-table>
                <div class="table-right">
                    <canvas id="rightDraw" style="width: 100%;height: 100%;"></canvas>
                </div>
            </div>
        </div>
        </div>
        `;
  }
}

export class JankTreeNode {
  name: string = '';
  pid: number = -1;
  frame_type: string = '';
  type: number = 0;

  constructor(name: string, pid: number, frame_type: string) {
    this.name = name;
    this.pid = pid;
    this.frame_type = frame_type;
  }

  children: Array<JankTreeNode> = [];
}
