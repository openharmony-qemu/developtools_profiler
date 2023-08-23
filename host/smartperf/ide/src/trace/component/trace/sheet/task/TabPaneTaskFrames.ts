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

import { BaseElement, element } from '../../../../../base-ui/BaseElement.js';
import { SelectionParam } from '../../../../bean/BoxSelection';
import { LitTable } from '../../../../../base-ui/table/lit-table';
import { resizeObserver } from '../SheetUtils.js';
import { FuncStruct } from '../../../../database/ui-worker/ProcedureWorkerFunc.js';
import {
  queryConcurrencyTask,
  queryTaskListByExecuteTaskIds,
  queryTaskPoolTotalNum
} from '../../../../database/SqlLite.js';
import { BaseStruct } from '../../../../database/ui-worker/ProcedureWorkerCommon.js';
import { SpSystemTrace } from '../../../SpSystemTrace.js';
import {LitProgressBar} from '../../../../../base-ui/progress-bar/LitProgressBar.js';

@element('tabpane-task-frames')
export class TabPaneTaskFrames extends BaseElement {
  private taskFramesTbl: LitTable | null | undefined;
  private range: HTMLLabelElement | null | undefined;
  private taskFramesSource: Array<TaskTabStruct> = [];
  private progressEL: LitProgressBar | null | undefined;
  static TaskArray: Array<FuncStruct> = [];
  static IsShowConcurrency: boolean = false;

  set data(framesParam: SelectionParam) {
    if (TabPaneTaskFrames.TaskArray && TabPaneTaskFrames.TaskArray.length > 0) {
      //点选
      this.setTaskData(TabPaneTaskFrames.TaskArray, framesParam, true);
    } else {
      if (!framesParam) {
        this.taskFramesTbl!!.recycleDataSource = [];
        return;
      }
      //框选
      this.range!.textContent = `Selected range: 
    ${ parseFloat(((framesParam.rightNs - framesParam.leftNs) / 1000000.0).toFixed(5)) } ms`;
      this.progressEL!.loading = true;
      this.queryDataByDB(framesParam, false);
    }
  }

  setTaskData(taskArray: Array<FuncStruct>, framesParam: SelectionParam, isClick: boolean) {
    if (taskArray.length < 3) {
      this.taskFramesTbl!!.recycleDataSource = [];
      return;
    } else {
      let sTime, eTime = 0, rTime;
      let aStartTime = 0, pStartTime = 0, rEndTime = 0;
      let priorityId = 1;
      let executeId = '';
      let executeStruct;
      taskArray.forEach((item) => {
        if (item.funName!.indexOf('H:Task Allocation:') >= 0) {
          aStartTime = item.startTs!;
          priorityId = TabPaneTaskFrames.getPriorityId(item.funName!);
          executeId = TabPaneTaskFrames.getExecuteId(item.funName!);
        } else if (item.funName!.indexOf('H:Task Perform:') >= 0) {
          executeStruct = item;
          pStartTime = item.startTs!;
          eTime = item.dur!;
        } else if (item.funName!.indexOf('H:Task PerformTask End:') >= 0) {
          rEndTime = item.startTs! + item.dur!;
        }
      });
      sTime = pStartTime - aStartTime;
      rTime = rEndTime - (pStartTime + eTime);
      if (TabPaneTaskFrames.IsShowConcurrency) {
        let tableList: TaskTabStruct[] = [];
        let countConcurrencyPromise = this.countConcurrency(executeStruct, tableList, framesParam, isClick);
        countConcurrencyPromise.then(result => {
          let concurrencyColumn: TaskTabStruct = new TaskTabStruct();
          concurrencyColumn.executeId = 'Task Concurrency';
          concurrencyColumn.taskPriority = `${ result }`;
          tableList.push(concurrencyColumn);
          let filterList = [];
          let map = new Map();
          for (const item of tableList) {
            if (!map.has(item.executeId)) {
              map.set(item.executeId, true);
              filterList.push(item);
            }
          }
          this.taskFramesSource = filterList;
          this.taskFramesTbl!!.recycleDataSource = this.taskFramesSource;
        });
      } else {
        let task: TaskTabStruct = new TaskTabStruct();
        task.executeId = executeId;
        task.taskPriority = Priority[priorityId];
        task.taskST = this.getMsTime(sTime);
        task.taskET = this.getMsTime(eTime);
        task.taskRT = this.getMsTime(rTime);
        this.taskFramesSource = [task];
        this.taskFramesTbl!!.recycleDataSource = this.taskFramesSource;
      }
    }
  }

  queryDataByDB(framesParam: SelectionParam, isClick: boolean): void {
    let tableList: TaskTabStruct[] = [];
    let executeTaskList: FuncStruct[] = [];
    let executeTaskIds: number[] = [];
    this.taskFramesTbl!.recycleDataSource = [];
    for (let index = 0 ; index < framesParam.taskFramesData.length ; index++) {
      let data = framesParam.taskFramesData[index];
      for (let y = 0 ; y < data.length ; y++) {
        let executeId = TabPaneTaskFrames.getExecuteId(data[y].funName!);
        if (data[y].funName!.indexOf('H:Task Perform:') >= 0) {
          executeTaskList.push(data[y]);
        }
        executeTaskIds.push(parseInt(executeId));
      }
    }
    queryTaskListByExecuteTaskIds(executeTaskIds).then((taskList) => {
      for (let index = 0 ; index < taskList.length ; index++) {
        this.pushTaskToList(taskList[index], tableList);
      }
      this.handleConcurrency(executeTaskList, tableList, framesParam, isClick);
    });
  }


  initElements(): void {
    this.taskFramesTbl = this.shadowRoot?.querySelector<LitTable>('#tb-frames');
    this.range = this.shadowRoot?.querySelector('#task-frames-time-range');
    this.progressEL = this.shadowRoot?.querySelector('.progress') as LitProgressBar;
    this.taskFramesTbl!.addEventListener('column-click', (evt) => {
      // @ts-ignore
      this.sortByColumn(evt.detail);
    });
  }

  getMsTime(ts: number): number {
    return parseFloat((ts / 1000000.0).toFixed(6));
  }

  connectedCallback(): void {
    super.connectedCallback();
    resizeObserver(this.parentElement!, this.taskFramesTbl!);
  }

  initHtml(): string {
    return `
        <style>
        .frames-label{
          height: 20px;
          text-align: end;
        }
        :host{
            padding: 10px 10px;
            display: flex;
            flex-direction: column;
        }
        </style>
        <label id="task-frames-time-range" class="frames-label" style="width: 100%;font-size: 10pt;margin-bottom: 5px">
        Selected range:0.0 ms</label>
        <lit-progress-bar class="progress"></lit-progress-bar>
        <lit-table id="tb-frames" style="height: auto">
            <lit-table-column class="task-frames-column" title="Execute Id" width="1fr" data-index="executeId" 
            key="executeId"  align="flex-start" order>
            </lit-table-column>
            <lit-table-column class="task-frames-column" title="Task Priority" width="1fr" data-index="taskPriority" 
            key="taskPriority"  align="flex-start" order >
            </lit-table-column>
            <lit-table-column class="task-frames-column" title="Scheduling Time(ms)" width="1fr" 
            data-index="taskST" key="taskST"  align="flex-start" order >
            </lit-table-column>
            <lit-table-column class="task-frames-column" title="Execution Time(ms)" width="1fr" data-index="taskET" 
            key="taskET"  align="flex-start" order >
            </lit-table-column>
            <lit-table-column class="task-frames-column" title="Return Time(ms)" width="1fr" data-index="taskRT" 
            key="taskRT"  align="flex-start" order >
            </lit-table-column>
        </lit-table>
        `;
  }

  sortByColumn(framesDetail: any) {
    // @ts-ignore
    let compare = function (property, sort, type) {
      return function (taskFramesLeftData: TaskTabStruct, taskFramesRightData: TaskTabStruct) {
        if (taskFramesLeftData.executeId === 'Task Concurrency') {
          return 0;
        }
        if (type === 'number') {
          // @ts-ignore
          let forwardNum = parseFloat(taskFramesRightData[property]) - parseFloat(taskFramesLeftData[property]);
          // @ts-ignore
          let reserveNum = parseFloat(taskFramesLeftData[property]) - parseFloat(taskFramesRightData[property]);
          return sort === 2 ? forwardNum : reserveNum;
        } else {
          // @ts-ignore
          if (taskFramesRightData[property] > taskFramesLeftData[property]) {
            return sort === 2 ? 1 : -1;
          } else {
            // @ts-ignore
            if (taskFramesRightData[property] === taskFramesLeftData[property]) {
              return 0;
            } else {
              return sort === 2 ? -1 : 1;
            }
          }
        }
      };
    };
    if (framesDetail.key === 'taskPriority') {
      this.taskFramesSource.sort(compare(framesDetail.key, framesDetail.sort, 'string'));
    } else {
      this.taskFramesSource.sort(compare(framesDetail.key, framesDetail.sort, 'number'));
    }
    this.taskFramesTbl!.recycleDataSource = this.taskFramesSource;
  }

  static getExecuteId(funName: string): string {
    let strArray = funName.split(',');
    let executeStr = '';
    let executeId = '';
    let endStr = '';
    if (strArray.length >= 2) {
      executeStr = strArray[1];
      if (funName.indexOf('H:Task Allocation:') >= 0 || funName.indexOf('H:Task Perform:') >= 0) {
        executeId = executeStr.split(':')[1].trim();
      } else if (funName.indexOf('H:Task PerformTask End:') >= 0) {
        endStr = executeStr.split(':')[1].trim();
        if (endStr.indexOf('[') >= 0) {
          executeId = endStr.substring(0, endStr.indexOf('['));
        } else {
          executeId = endStr;
        }
      }
    }
    return executeId;
  }

  static getPriorityId(funName: string): number {
    let strArray = funName.split(',');
    let priorityId = '';
    if (strArray.length >= 2) {
      let executeStr = strArray[2];
      if (funName.indexOf('H:Task Allocation:') >= 0) {
        priorityId = executeStr.split(':')[1].trim();
      }
    }
    return parseInt(priorityId);
  }

  private async countConcurrency(selectFuncStruct: FuncStruct | undefined,
                                 tableList: TaskTabStruct[], framesParam: SelectionParam, isClick: boolean): Promise<number> {
    let selectStartTime = selectFuncStruct!.startTs! + (window as any).recordStartNS;
    let selectEndTime = selectFuncStruct!.startTs! + selectFuncStruct!.dur! + (window as any).recordStartNS;
    if (!isClick) {
      let startTime = framesParam.recordStartNs + framesParam.leftNs;
      let endTime = framesParam.recordStartNs + framesParam.rightNs;
      if ((selectStartTime <= startTime && startTime <= selectEndTime) ||
          (selectStartTime <= endTime && endTime <= selectEndTime)) {
        selectStartTime = startTime;
        selectEndTime = endTime;
      }
    }
    let maxConcurrency = 0;
    await Promise.all([queryTaskPoolTotalNum(selectFuncStruct!.funName!),
      queryConcurrencyTask(selectFuncStruct!.funName!, selectStartTime, selectEndTime)]).then(
        (res) => {
          let currentConcurrency = 0;
          let tasks:Array<TaskTabStruct> = res[1]
          for (let i = 0; i < tasks.length; i++) {
            const task = tasks[i];
            const endTime = task!.startTs! + task!.dur!;
            currentConcurrency++;
            for (let j = i + 1; j < tasks.length; j++) {
              const nextTask = tasks[j];
              if (nextTask.startTs! < endTime) {
                currentConcurrency++;
              }
            }
            // 更新最大并发度
            if (currentConcurrency > maxConcurrency) {
              maxConcurrency = currentConcurrency;
              if (maxConcurrency === res[0].length) {
                break;
              }
            }
            // 重置当前并发度
            currentConcurrency = 0;
          }
          for (const item of res[1]) {
            this.pushTaskToList(item, tableList);
          }
        }
    );
    return maxConcurrency;
  }

  private handleConcurrency(executeTaskList: FuncStruct[], tableList: TaskTabStruct[], framesParam: SelectionParam, isClick: boolean): void {
    let maxNumConcurrency = 0;
    if (executeTaskList.length > 0) {
      let handleConcurrency = async (): Promise<void> => {
        for (let i = 0 ; i < executeTaskList.length ; i++) {
          let countConcurrencyPromise = await this.countConcurrency(executeTaskList[i], tableList, framesParam, isClick);
          if (countConcurrencyPromise > maxNumConcurrency) {
            maxNumConcurrency = countConcurrencyPromise;
          }
        }
      };
      handleConcurrency().then(() => {
        let concurrencyColumn: TaskTabStruct = new TaskTabStruct();
        concurrencyColumn.executeId = 'Task Concurrency';
        concurrencyColumn.taskPriority = `${ maxNumConcurrency }`;
        tableList.push(concurrencyColumn);
        //去重
        let filterList = [];
        let map = new Map();
        for (const item of tableList) {
          if (!map.has(item.executeId)) {
            map.set(item.executeId, true);
            filterList.push(item);
          }
        }
        this.taskFramesSource = filterList;
        this.taskFramesTbl!.recycleDataSource = filterList;
        this.progressEL!.loading = false;
      });
    } else {
      this.taskFramesSource = tableList;
      this.taskFramesTbl!.recycleDataSource = tableList;
      this.progressEL!.loading = false;
    }
  }

  private pushTaskToList(value: TaskTabStruct, tableList: TaskTabStruct[]): void {
    let allocationTask = SpSystemTrace.DATA_TASK_POOL_CALLSTACK.get(value.allocationTaskRow!);
    let executeTask = SpSystemTrace.DATA_TASK_POOL_CALLSTACK.get(value.executeTaskRow!);
    let returnTask = SpSystemTrace.DATA_TASK_POOL_CALLSTACK.get(value.returnTaskRow!);
    let tempTask: TaskTabStruct = new TaskTabStruct();
    let executeStartTime = executeTask!.ts!;
    let executeTime = executeTask!.dur!;
    let aStartTime = allocationTask!.ts!;
    let rEndTime = returnTask!.ts! + returnTask!.dur!;
    tempTask.executeId = value.executeId;
    tempTask.taskPriority = Priority[value.priority!];
    tempTask.taskST = this.getMsTime(executeStartTime - aStartTime);
    tempTask.taskET = this.getMsTime(executeTime);
    tempTask.taskRT = this.getMsTime(rEndTime - (executeStartTime + executeTime));
    tableList.push(tempTask);
  }
}

enum Priority {
  HIGH,
  MEDIUM,
  LOW
}

export class TaskTabStruct extends BaseStruct {
  static maxValue: number = 0;
  static maxName: string = '';
  static index = 0;
  id: number | undefined;
  tid: number | undefined;
  ipid: number | undefined;
  executeId: string | undefined;
  startTs: number | undefined;
  funName: string | undefined;
  dur: number | undefined;
  taskST: number | undefined;
  taskET: number | undefined;
  taskRT: number | undefined;
  allocationTaskRow: number | undefined;
  executeTaskRow: number | undefined;
  returnTaskRow: number | undefined;
  priority: number | undefined;
  taskPriority: string | undefined;
  isUse: boolean = false;
}
