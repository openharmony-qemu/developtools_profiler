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
  queryTaskPoolTotalNum,
} from '../../../../database/SqlLite.js';
import { BaseStruct } from '../../../../database/ui-worker/ProcedureWorkerCommon.js';
import { SpSystemTrace } from '../../../SpSystemTrace.js';
import { LitProgressBar } from '../../../../../base-ui/progress-bar/LitProgressBar.js';

const ALLOCATION_TASK = 'H:Task Allocation:';
const PERFORM_TASK = 'H:Task Perform:';
const END_TASK = 'H:Task PerformTask End:';
const TABLE_CURRENCY = 'Task Concurrency';

@element('tabpane-task-frames')
export class TabPaneTaskFrames extends BaseElement {
  private taskFramesTbl: LitTable | null | undefined;
  private range: HTMLLabelElement | null | undefined;
  private taskFramesSource: Array<TaskTabStruct> = [];
  private taskFramesGroupSource: Array<Array<TaskTabStruct>> = [];
  private progressEL: LitProgressBar | null | undefined;
  static TaskArray: Array<FuncStruct> = [];
  static IsShowConcurrency: boolean = false;

  set data(framesParam: SelectionParam) {
    if (TabPaneTaskFrames.TaskArray && TabPaneTaskFrames.TaskArray.length > 0) {
      this.progressEL!.loading = true;
      //点选
      this.setTaskData(TabPaneTaskFrames.TaskArray, framesParam, true);
    } else {
      if (!framesParam) {
        this.taskFramesTbl!!.recycleDataSource = [];
        this.taskFramesSource = [];
        this.taskFramesGroupSource = [];
        return;
      }
      //框选
      this.range!.textContent = `Selected range: 
      ${ parseFloat(((framesParam.rightNs - framesParam.leftNs) / 1000000.0).toFixed(5)) } ms`;
      this.progressEL!.loading = true;
      this.queryDataByDB(framesParam, false);
    }
  }

  setTaskData(taskArray: Array<FuncStruct>, framesParam: SelectionParam, isClick: boolean): void {
    if (taskArray.length < 2) {
      this.taskFramesTbl!!.recycleDataSource = [];
      this.taskFramesSource = [];
      this.taskFramesGroupSource = [];
      return;
    } else {
      let allocationTime = 0;
      let executeTime = 0;
      let returnTime = 0;
      let allocationStartTime = 0;
      let executeStartTime = 0;
      let returnEndTime = 0;
      let priorityId = 1;
      let executeId = '';
      let executeStruct: FuncStruct | undefined = undefined;
      taskArray.forEach((item) => {
        if (item.funName!.indexOf(ALLOCATION_TASK) >= 0) {
          allocationStartTime = item.startTs!;
          priorityId = TabPaneTaskFrames.getPriorityId(item.funName!);
          executeId = TabPaneTaskFrames.getExecuteId(item.funName!);
        } else if (item.funName!.indexOf(PERFORM_TASK) >= 0) {
          executeStruct = item;
          executeStartTime = item.startTs!;
          executeTime = item.dur!;
        } else if (item.funName!.indexOf(END_TASK) >= 0) {
          returnEndTime = item.startTs! + item.dur!;
        }
      });
      allocationTime = executeStartTime - allocationStartTime;
      returnTime = returnEndTime == 0 ? 0 : returnEndTime - (executeStartTime + executeTime);
      if (TabPaneTaskFrames.IsShowConcurrency) {
        let tableList: TaskTabStruct[] = [];
        this.buildConcurrencyTable(executeStruct!, tableList, framesParam, isClick);
      } else {
        this.buildNoConcurrencyTable(executeId, priorityId, allocationTime, executeTime, returnTime);
      }
    }
  }

  private buildConcurrencyTable(executeStruct: FuncStruct,
                                tableList: TaskTabStruct[], framesParam: SelectionParam, isClick: boolean): void {
    this.countConcurrency(executeStruct, tableList, framesParam, isClick).then((result) => {
      let concurrencyColumn: TaskTabStruct = new TaskTabStruct();
      concurrencyColumn.executeId = TABLE_CURRENCY;
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
      this.taskFramesGroupSource = [filterList];
      this.taskFramesSource = filterList;
      this.taskFramesTbl!!.recycleDataSource = this.taskFramesSource;
      this.progressEL!.loading = false;
    });
  }
  private buildNoConcurrencyTable(executeId: string, priorityId: number, sTime: number,
                                  eTime: number, rTime: number): void {
    let task: TaskTabStruct = new TaskTabStruct();
    task.executeId = executeId;
    task.taskPriority = Priority[priorityId];
    task.taskST = this.getMsTime(sTime);
    task.taskET = this.getMsTime(eTime);
    task.taskRT = this.getMsTime(rTime);
    this.taskFramesSource = [task];
    this.taskFramesGroupSource = [[task]];
    this.taskFramesTbl!!.recycleDataSource = this.taskFramesSource;
    this.progressEL!.loading = false;
  }
  async queryDataByDB(framesParam: SelectionParam, isClick: boolean): Promise<void> {
    this.taskFramesGroupSource = [];
    let tableList: TaskTabStruct[] = [];
    this.taskFramesTbl!.recycleDataSource = [];
    let groups = new Map();
    framesParam.taskFramesData.forEach((obj) => {
      const key = obj.ipid;
      if (!groups.has(key)) {
        groups.set(key, []);
      }
      groups.get(key).push(obj);
    });
    for (let [key, groupsValue] of groups) {
      let tempTableList: TaskTabStruct[] = [];
      let tempExecuteTaskList: FuncStruct[] = [];
      let tempExecuteTaskIds: number[] = [];
      for (let index = 0 ; index < groupsValue.length ; index++) {
        let data = groupsValue[index];
        let executeId = TabPaneTaskFrames.getExecuteId(data.funName!);
        if (data.funName!.indexOf(PERFORM_TASK) >= 0) {
          tempExecuteTaskList.push(data);
        }
        tempExecuteTaskIds.push(parseInt(executeId));
      }
      let uniqueArr = [...new Set(tempExecuteTaskIds)];
      let taskList = await queryTaskListByExecuteTaskIds(uniqueArr, key);
      for (let index = 0 ; index < taskList.length ; index++) {
        this.pushTaskToList(taskList[index], tempTableList);
      }
      let filterArray = await this.handleConcurrency(tempExecuteTaskList, tempTableList, framesParam, isClick);
      this.taskFramesGroupSource.push(filterArray);
    }
    this.taskFramesGroupSource.forEach(framesSource => {
      tableList.push(...framesSource);
    });
    this.taskFramesSource = tableList;
    this.taskFramesTbl!.recycleDataSource = tableList;
    this.progressEL!.loading = false;
  }

  initElements(): void {
    this.taskFramesTbl = this.shadowRoot?.querySelector<LitTable>('#tb-frames');
    this.range = this.shadowRoot?.querySelector('#task-frames-time-range');
    this.progressEL = this.shadowRoot?.querySelector('.progress') as LitProgressBar;
    this.taskFramesTbl!.addEventListener('column-click', (evt: CustomEventInit) => {
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
        .task-frames-label{
          height: 20px;
          text-align: end;
        }
        :host{
            padding: 10px 10px;
            display: flex;
            flex-direction: column;
        }
        </style>
        <label id="task-frames-time-range" class="task-frames-label" style="width: 100%;font-size: 10pt;margin-bottom: 5px">
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

  sortByColumn(framesDetail: {
    sort: number,
    key: string,
  }): void {
    // @ts-ignore
    let compare = function (property, sort, type) {
      return function (taskFramesLeftData: TaskTabStruct, taskFramesRightData: TaskTabStruct): number {
        if (taskFramesLeftData.executeId === TABLE_CURRENCY) {
          return 1;
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
    let tableList: TaskTabStruct[] = [];
    this.taskFramesGroupSource.forEach(framesGroup => {
      if (framesDetail.key === 'taskPriority') {
        framesGroup.sort(compare(framesDetail.key, framesDetail.sort, 'string'));
      } else {
        framesGroup.sort(compare(framesDetail.key, framesDetail.sort, 'number'));
      }
      tableList.push(...framesGroup);
    });
    this.taskFramesSource = tableList;
    this.taskFramesTbl!.recycleDataSource = tableList;
  }

  static getExecuteId(funName: string): string {
    let strArray = funName.split(',');
    let executeStr = '';
    let executeId = '';
    let endStr = '';
    if (strArray.length >= 2) {
      executeStr = strArray[1];
      if (funName.indexOf(ALLOCATION_TASK) >= 0 || funName.indexOf(PERFORM_TASK) >= 0) {
        executeId = executeStr.split(':')[1].trim();
      } else if (funName.indexOf(END_TASK) >= 0) {
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
      if (funName.indexOf(ALLOCATION_TASK) >= 0) {
        priorityId = executeStr.split(':')[1].trim();
      }
    }
    return parseInt(priorityId);
  }

  private async countConcurrency(
    selectFuncStruct: FuncStruct | undefined,
    tableList: TaskTabStruct[],
    framesParam: SelectionParam,
    isClick: boolean
  ): Promise<number> {
    let selectStartTime = selectFuncStruct!.startTs! + (window as any).recordStartNS;
    let selectEndTime = selectFuncStruct!.startTs! + selectFuncStruct!.dur! + (window as any).recordStartNS;
    if (!isClick) {
      selectStartTime = framesParam.recordStartNs + framesParam.leftNs;
      selectEndTime = framesParam.recordStartNs + framesParam.rightNs;
    }
    let res = await Promise.all([
      queryTaskPoolTotalNum(selectFuncStruct!.itid!),
      queryConcurrencyTask(selectFuncStruct!.itid!, selectStartTime, selectEndTime),
    ]);
    let tasks: Array<TaskTabStruct> = res[1];
    let maxConcurrency = 0;
    let timePointArray: { time: number, isStart: boolean }[] = [];
    for (let i = 0 ; i < tasks.length ; i++) {
      timePointArray.push({ time: tasks[i].startTs!, isStart: true });
      timePointArray.push({ time: tasks[i].startTs! + tasks[i].dur!, isStart: false });
    }
    timePointArray.sort((timePointA, timePointB) => {
      if (timePointA.time === timePointB.time) {
        return timePointA.isStart ? -1 : 1;
      }
      return timePointA.time - timePointB.time;
    });
    let currentConcurrency = 0;
    for (let i = 0; i < timePointArray.length; i++) {
      if (timePointArray[i].isStart) {
        currentConcurrency++;
      } else {
        currentConcurrency--;
      }
      if (currentConcurrency > maxConcurrency) {
        maxConcurrency = currentConcurrency;
        if (maxConcurrency === res[0].length) {
          break;
        }
      }
    }
    for (const item of res[1]) {
      this.pushTaskToList(item, tableList);
    }
    return maxConcurrency;
  }

  private async handleConcurrency(
    executeTaskList: FuncStruct[],
    tableList: TaskTabStruct[],
    framesParam: SelectionParam,
    isClick: boolean
  ): Promise<Array<TaskTabStruct>> {
    let maxNumConcurrency = 0;
    if (executeTaskList.length > 0) {
      let countConcurrencyPromise = await this.countConcurrency(executeTaskList[0], tableList, framesParam, isClick);
      maxNumConcurrency = countConcurrencyPromise;
      let concurrencyColumn: TaskTabStruct = new TaskTabStruct();
      concurrencyColumn.executeId = TABLE_CURRENCY;
      concurrencyColumn.taskPriority = `${ maxNumConcurrency }`;
      tableList.push(concurrencyColumn);
      //去重
      let filterList = [];
      let map = new Map();
      for (const ConcurrencyTblItem of tableList) {
        if (!map.has(ConcurrencyTblItem.executeId)) {
          map.set(ConcurrencyTblItem.executeId, true);
          filterList.push(ConcurrencyTblItem);
        }
      }
      return filterList;
    }
    return tableList;
  }

  private pushTaskToList(value: TaskTabStruct, tableList: TaskTabStruct[]): void {
    let allocationTask = SpSystemTrace.DATA_TASK_POOL_CALLSTACK.get(value.allocationTaskRow!);
    let executeTask = SpSystemTrace.DATA_TASK_POOL_CALLSTACK.get(value.executeTaskRow!);
    let returnTask = SpSystemTrace.DATA_TASK_POOL_CALLSTACK.get(value.returnTaskRow!);
    let tempTask: TaskTabStruct = new TaskTabStruct();
    let executeStartTime = executeTask!.ts!;
    let executeTime = executeTask!.dur! === -1 ? (window as any).recordEndNS - executeTask!.ts : executeTask!.dur;
    let allocationStartTime = allocationTask!.ts!;
    let returnEndTime = 0;
    if (returnTask) {
      returnEndTime = returnTask!.ts! + returnTask!.dur! - (executeStartTime + executeTime);
    }
    tempTask.executeId = value.executeId;
    tempTask.taskPriority = Priority[value.priority!];
    tempTask.taskST = this.getMsTime(executeStartTime - allocationStartTime);
    tempTask.taskET = this.getMsTime(executeTime);
    tempTask.taskRT = this.getMsTime(returnEndTime);
    tableList.push(tempTask);
  }
}

enum Priority {
  HIGH,
  MEDIUM,
  LOW,
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
