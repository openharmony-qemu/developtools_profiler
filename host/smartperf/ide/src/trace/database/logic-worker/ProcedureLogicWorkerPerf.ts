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

import { LogicHandler, ChartStruct, convertJSON, DataCache, HiPerfSymbol } from './ProcedureLogicWorkerCommon.js';
import { PerfBottomUpStruct } from '../../bean/PerfBottomUpStruct.js';
import { HiPerfChartFrame } from '../../bean/PerfStruct.js';

const systemRuleName: string = '/system/';
const numRuleName: string = '/max/min/';
const maxDepth: number = 128;

export class ProcedureLogicWorkerPerf extends LogicHandler {
  filesData: any = {};
  samplesData: any = {};
  threadData: any = {};
  callChainData: any = {};
  splitMapData: any = {};
  currentTreeMapData: any = {};
  currentTreeList: any[] = [];
  searchValue: string = '';
  dataSource: PerfCallChainMerageData[] = [];
  allProcess: PerfCallChainMerageData[] = [];
  queryFunc?: Function | undefined;
  isActualQuery: boolean = false;
  currentEventId: string = '';
  isAnalysis: boolean = false;
  isPerfBottomUp: boolean = false;
  isHideThread: boolean = false;
  isHideThreadState: boolean = false;
  processMap = new Map<number, { count: number; eventCount: number }>();
  threadMap = new Map<number, { count: number; eventCount: number }>();
  private lib: object | undefined;
  private symbol: object | undefined;
  perfCallData: any[] = [];
  private dataCache = DataCache.getInstance();
  private isTopDown: boolean = true;
  private samplesCpu = Array<HiPrefSample>();

  handle(data: any): void {
    this.currentEventId = data.id;
    if (data && data.type) {
      switch (data.type) {
        case 'perf-init':
          this.dataCache.perfCountToMs = data.params.fValue;
          this.initPerfFiles();
          break;
        case 'perf-queryPerfFiles':
          let files = convertJSON(data.params.list) || [];
          files.forEach((file: any) => {
            this.filesData[file.fileId] = this.filesData[file.fileId] || [];
            PerfFile.setFileName(file);
            this.filesData[file.fileId].push(file);
          });
          this.initPerfThreads();
          break;
        case 'perf-queryPerfThread':
          let threads = convertJSON(data.params.list) || [];
          threads.forEach((thread: any): void => {
            this.threadData[thread.tid] = thread;
          });
          this.initPerfCalls();
          break;
        case 'perf-queryPerfCalls':
          let perfCalls = convertJSON(data.params.list) || [];
          if (perfCalls.length !== 0) {
            perfCalls.forEach((perfCall: any): void => {
              this.dataCache.perfCallChainMap.set(perfCall.sampleId, perfCall);
            });
          }
          this.initPerfCallchains();
          break;
        case 'perf-queryPerfCallchains':
          let arr = convertJSON(data.params.list) || [];
          this.initPerfFire(arr);
          this.initPerfCallChainTopDown(arr);
          // @ts-ignore
          self.postMessage({
            id: data.id,
            action: data.action,
            results: this.dataCache.perfCallChainMap,
          });
          break;
        case 'perf-callstack-chart':
          if (data.params?.list) {
            // 拿到的sample的数据
            this.samplesCpu = convertJSON(data.params.list) || [];
            // 处理sample数据
            self.postMessage({
              id: data.id,
              action: data.action,
              results: this.combinePerfSampleBycallChainId(this.samplesCpu),
            });
          } else {
            this.perfCallData = data.params;
            this.queryCallData(data.params);
          }
          break;
        case 'perf-queryCallchainsGroupSample':
          this.samplesData = convertJSON(data.params.list) || [];
          let result;
          if (this.isAnalysis) {
            result = this.resolvingAction([
              {
                funcName: 'combineAnalysisCallChain',
                funcArgs: [true],
              },
            ]);
          } else if (this.isPerfBottomUp) {
            result = this.resolvingAction([
              {
                funcName: 'getBottomUp',
                funcArgs: [true],
              },
            ]);
          } else {
            if (this.lib) {
              let libData = this.combineCallChainForAnalysis(this.lib);
              this.freshPerfCallchains(libData, this.isTopDown);
              result = this.allProcess;
              this.lib = undefined;
            } else if (this.symbol) {
              let funData = this.combineCallChainForAnalysis(this.symbol);
              this.freshPerfCallchains(funData, this.isTopDown);
              result = this.allProcess;
              this.symbol = undefined;
            } else {
              result = this.resolvingAction([
                {
                  funcName: 'getCallChainsBySampleIds',
                  funcArgs: [this.isTopDown],
                },
              ]);
            }
          }
          self.postMessage({
            id: data.id,
            action: data.action,
            results: result,
          });
          break;
        case 'perf-action':
          if (data.params) {
            let filter = data.params.filter((item: any): boolean => item.funcName === 'getCurrentDataFromDb');
            // 从lib层跳转
            let libFilter = data.params.filter((item: any): boolean => item.funcName === 'showLibLevelData');
            // 从fun层跳转
            let funFilter = data.params.filter((item: any): boolean => item.funcName === 'showFunLevelData');
            // lib或者fun层Invert
            let callChainsFilter = data.params.filter(
              (item: any): boolean => item.funcName === 'getCallChainsBySampleIds'
            );
            let isHideSystemSoFilter = data.params.filter(
              (item: any): boolean => item.funcName === 'hideSystemLibrary'
            );
            let hideThreadFilter = data.params.filter((item: any): boolean => item.funcName === 'hideThread');
            let hideThreadStateFilter = data.params.filter((item: any): boolean => item.funcName === 'hideThreadState');
            let result;
            callChainsFilter.length > 0 ? (this.isTopDown = callChainsFilter[0].funcArgs[0]) : (this.isTopDown = true);
            if (libFilter.length !== 0) {
              this.lib = {
                libId: libFilter[0].funcArgs[0],
                libName: libFilter[0].funcArgs[1],
              };
            } else if (funFilter.length !== 0) {
              this.symbol = {
                symbolId: funFilter[0].funcArgs[0],
                symbolName: funFilter[0].funcArgs[1],
              };
            }

            if (filter.length === 0) {
              if (this.lib) {
                if (
                  callChainsFilter.length > 0 ||
                  isHideSystemSoFilter.length > 0 ||
                  hideThreadFilter.length > 0 ||
                  hideThreadStateFilter.length > 0
                ) {
                  this.samplesData = this.combineCallChainForAnalysis(this.lib);
                  result = this.resolvingAction(data.params);
                } else {
                  let libData = this.combineCallChainForAnalysis(this.lib);
                  this.freshPerfCallchains(libData, this.isTopDown);
                  result = this.allProcess;
                  this.lib = undefined;
                }
              } else if (this.symbol) {
                if (
                  callChainsFilter.length > 0 ||
                  isHideSystemSoFilter.length > 0 ||
                  hideThreadFilter.length > 0 ||
                  hideThreadStateFilter.length > 0
                ) {
                  this.samplesData = this.combineCallChainForAnalysis(this.symbol);
                  result = this.resolvingAction(data.params);
                } else {
                  let funData = this.combineCallChainForAnalysis(this.symbol);
                  this.freshPerfCallchains(funData, this.isTopDown);
                  result = this.allProcess;
                  this.symbol = undefined;
                }
              } else {
                result = this.resolvingAction(data.params);
              }
              self.postMessage({
                id: data.id,
                action: data.action,
                results: result,
              });
            } else {
              this.resolvingAction(data.params);
            }
          }
          break;
        case 'perf-reset':
          this.isHideThread = false;
          this.isHideThreadState = false;
      }
    }
  }

  initPerfFire(arr: Array<any>) {
    this.dataCache.perfCallChain = [];
    arr.forEach((item) => {
      this.dataCache.perfCallChain!.push({
        callchain_id: item.sampleId,
        depth: item.depth,
        name: item.name,
        fileId: item.fileId,
        symbolId: item.symbolId,
      } as HiPerfSymbol);
    });
    this.createCallChain();
  }


  getQueryCallDataTypeCondition(type : number, id: number) : string {
    if (type === 0) {
      return `cpu_id = ${id}`;
    } else if (type === 1) {
      return `C.process_id = ${id}`;
    } else if (type === 2) {
      return  `A.thread_id = ${id}`;
    } else {
      return '';
    }
  }
  queryCallData(data: Array<number>) {
    let condition = `${this.getQueryCallDataTypeCondition(data[0], data[1])} ${
      data[2] === -2 ? '' : `and event_type_id=${data[2]}`
    }`;
    const sql = `SELECT
    callchain_id,
    timestamp_trace - start_ts AS timeTip,
    event_count as eventCount,
    A.thread_id,
    cpu_id
  FROM
    perf_sample A,trace_range B
    left join perf_thread C on A.thread_id = C.thread_id
  where callchain_id != -1 and A.thread_id != 0 and ${condition}`;
    this.queryData(this.currentEventId!, 'perf-callstack-chart', sql, {});
  }
  initPerfFiles(): void {
    this.clearAll();
    this.queryData(
      this.currentEventId,
      'perf-queryPerfFiles',
      `select file_id as fileId, symbol, path
       from perf_files`,
      {}
    );
  }

  initPerfThreads(): void {
    this.queryData(
      this.currentEventId,
      'perf-queryPerfThread',
      `select a.thread_id as tid, a.thread_name as threadName, a.process_id as pid, b.thread_name as processName
       from perf_thread a
                left join (select * from perf_thread where thread_id = process_id) b on a.process_id = b.thread_id`,
      {}
    );
  }

  initPerfCalls(): void {
    this.queryData(
      this.currentEventId,
      'perf-queryPerfCalls',
      `select count(callchain_id) as depth, callchain_id as sampleId, name
       from perf_callchain
       where callchain_id != -1
       group by callchain_id`,
      {}
    );
  }

  initPerfCallchains(): void {
    this.queryData(
      this.currentEventId,
      'perf-queryPerfCallchains',
      `select c.name,
              c.callchain_id  as sampleId,
              c.vaddr_in_file as vaddrInFile,
              c.file_id       as fileId,
              c.depth,
              c.symbol_id     as symbolId
       from perf_callchain c
       where callchain_id != -1;`,
      {}
    );
  }

  /**
   *
   * @param selectionParam
   * @param sql 从饼图进程或者线程层点击进入Perf Profile时传入
   */
  private getCurrentDataFromDb(selectionParam: any, sql?: string): void {
    let filterSql = '';
    if (sql) {
      const cpus = selectionParam.perfAll ? [] : selectionParam.perfCpus;
      const cpuFilter = cpus.length > 0 ? ` and s.cpu_id in (${cpus.join(',')}) ` : '';
      let arg = `${sql}${cpuFilter}`.substring(3);
      filterSql = `and ${arg}`;
    } else {
      const cpus = selectionParam.perfAll ? [] : selectionParam.perfCpus;
      const processes = selectionParam.perfAll ? [] : selectionParam.perfProcess;
      const threads = selectionParam.perfAll ? [] : selectionParam.perfThread;

      if (cpus.length !== 0 || processes.length !== 0 || threads.length !== 0) {
        const cpuFilter = cpus.length > 0 ? `or s.cpu_id in (${cpus.join(',')}) ` : '';
        const processFilter = processes.length > 0 ? `or thread.process_id in (${processes.join(',')}) ` : '';
        const threadFilter = threads.length > 0 ? `or s.thread_id in (${threads.join(',')})` : '';
        let arg = `${cpuFilter}${processFilter}${threadFilter}`.substring(3);
        filterSql = ` and (${arg})`;
      }
    }
    let eventTypeId = selectionParam.perfEventTypeId;
    const eventTypeFilter = eventTypeId !== undefined ? ` and s.event_type_id = ${eventTypeId}` : '';
    filterSql += eventTypeFilter;
    this.queryData(
      this.currentEventId,
      'perf-queryCallchainsGroupSample',
      `select p.callchain_id as sampleId,
          p.thread_state as threadState,
          p.thread_id    as tid,
          p.count as count,
          p.process_id   as pid,
          p.event_count  as eventCount,
          p.ts as ts,
          p.event_type_id as eventTypeId
      from (
        select callchain_id, s.thread_id, s.event_type_id, thread_state, process_id, 
                count(callchain_id) as count,SUM(event_count) as event_count,
                group_concat(s.timestamp_trace - t.start_ts,',') as ts
        from perf_sample s, trace_range t
        left join perf_thread thread on s.thread_id = thread.thread_id
        where timestamp_trace between ${selectionParam.leftNs} + t.start_ts
            and ${selectionParam.rightNs} + t.start_ts
            and callchain_id != -1
            and s.thread_id != 0 ${filterSql}
        group by callchain_id, s.thread_id, thread_state, process_id) p`,
      {
        $startTime: selectionParam.leftNs,
        $endTime: selectionParam.rightNs,
        $sql: filterSql,
      }
    );
  }

  // 将perf_sample表的数据根据callchain_id分组并赋值startTime,endTime等等
  combinePerfSampleBycallChainId(sampleList: Array<HiPrefSample>) {
    let arr: any = new Array();
    let newPerfData = (sample: any) : HiPerfSymbol => {
      let perfSample = new HiPerfSymbol();
      perfSample.children = new Array<HiPerfSymbol>();
      perfSample.children[0] = new HiPerfSymbol();
      perfSample.depth = -1;
      perfSample.name = 'name';
      perfSample.callchain_id = sample.callchain_id;
      perfSample.thread_id = sample.thread_id;
      perfSample.id = sample.id;
      perfSample.startTime = sample.timeTip;
      perfSample.eventCount = sample.eventCount;
      return perfSample;
    };
    for (let i = 0; i < sampleList.length; i++) {
      if (arr.length > 0) {
        let last = arr[arr.length -1];
        last.endTime = sampleList[i].timeTip;
        last.totalTime = last.endTime - last.startTime;
        if (last.callchain_id === sampleList[i].callchain_id) {
          last.eventCount += sampleList[i].eventCount;
        } else {
          arr.push(newPerfData(sampleList[i]));
        }
      } else {
        arr.push(newPerfData(sampleList[i]));
      }
    }
    let last = arr[arr.length -1];
    if (last && last.endTime === 0) {
      last.endTime = this.perfCallData[3];
      last.totalTime = last.endTime - last.startTime;
    }
    return this.combineChartData(arr);
  }

  /**
   * 建立callChain每个函数的联系，设置depth跟children
   */
  private createCallChain(): void {
    this.dataCache.perfCallFireMap.clear();
    for (const item of this.dataCache.perfCallChain!) {
      this.dataCache.perfCallFireMap.set(`${item.callchain_id}-${item.depth}`, item);
      let parentSymbol = this.dataCache.perfCallFireMap.get(`${item.callchain_id}-${item.depth - 1}`);
      if (parentSymbol && parentSymbol.callchain_id === item.callchain_id && parentSymbol.depth === item.depth - 1) {
        parentSymbol.children = new Array<HiPerfSymbol>();
        parentSymbol.children.push(item);
      }
    }
  }

  combineChartData(samples: any): Array<HiPerfChartFrame> {
    let combineSample: any = [];
    // 遍历sample表查到的数据，并且为其匹配相应的callchain数据
    for (let sample of samples) {
      let stackTop = this.dataCache.perfCallFireMap.get(`${sample.callchain_id}-0`);
      if (stackTop) {
        let stackTopSymbol = JSON.parse(JSON.stringify(stackTop));
        stackTopSymbol.startTime = sample.startTime;
        stackTopSymbol.endTime = sample.endTime;
        stackTopSymbol.totalTime = sample.endTime - sample.startTime;
        stackTopSymbol.thread_id = sample.thread_id;
        stackTopSymbol.cpu_id = sample.thread_id;
        stackTopSymbol.eventCount = sample.eventCount;
        this.setDur(stackTopSymbol);
        sample.children = new Array<HiPerfSymbol>();
        sample.children.push(stackTopSymbol);
        // 每一项都和combineSample对比
        if (combineSample.length === 0) {
          combineSample.push(sample);
        } else {
          if (this.perfCallData[0] === 0) {
            if (combineSample[combineSample.length - 1].thread_id === sample.thread_id) {
              this.combinePerfCallData(combineSample[combineSample.length - 1], sample);
            } else {
              combineSample.push(sample);
            }
          } else {
            if (combineSample[combineSample.length - 1].cpu_id === sample.cpu_id) {
              this.combinePerfCallData(combineSample[combineSample.length - 1], sample);
            } else {
              combineSample.push(sample);
            }
          }
        }
      }
    }
    return combineSample;
  }

  // 递归设置dur,startTime,endTime
  setDur(data: any) {
    if (data.children && data.children.length > 0) {
      data.children[0].totalTime = data.totalTime;
      data.children[0].startTime = data.startTime;
      data.children[0].endTime = data.endTime;
      data.children[0].thread_id = data.thread_id;
      data.children[0].cpu_id = data.cpu_id;
      data.children[0].eventCount = data.eventCount;
      this.setDur(data.children[0]);
    } else {
      return;
    }
  }

  // hiperf火焰图合并逻辑
  combinePerfCallData(data1: any, data2: any) {
    if (data1.depth === data2.depth && data1.name === data2.name) {
      data1.endTime = data2.endTime;
      data1.eventCount += data2.eventCount;
      if (data1.children && data1.children.length > 0 && data2.children && data2.children.length > 0) {
        if (
          data1.children[data1.children.length - 1].depth === data2.children[0].depth &&
          data1.children[data1.children.length - 1].name !== data2.children[0].name
        ) {
          data1.children.push(data2.children[0]);
        } else {
          this.combinePerfCallData(data1.children[data1.children.length - 1], data2.children[0]);
        }
      } else if (data2.children && data2.children.length > 0 && (!data1.children || data1.children.length === 0)) {
        data1.endTime = data2.endTime;
        data1.totalTime = data1.endTime - data1.endTime;
        data1.children = new Array<HiPerfSymbol>();
        data1.children.push(data2.children[0]);
      } else {
      }
    }
    data1.totalTime = data1.endTime - data1.startTime;
    return;
  }
  clearAll(): void {
    this.filesData = {};
    this.samplesData = {};
    this.threadData = {};
    this.perfCallData = [];
    this.samplesCpu = [];
    this.callChainData = {};
    this.splitMapData = {};
    this.currentTreeMapData = {};
    this.currentTreeList = [];
    this.searchValue = '';
    this.dataSource = [];
    this.allProcess = [];
    this.dataCache.clearPerf();
  }

  initPerfCallChainTopDown(callChains: PerfCallChain[]): void {
    this.callChainData = {};
    callChains.forEach((callChain: PerfCallChain, index: number): void => {
      this.setPerfCallChainFrameName(callChain);
      this.addPerfGroupData(callChain);
      let callChainDatum = this.callChainData[callChain.sampleId];
      if (callChainDatum.length > 1) {
        PerfCallChain.setNextNode(callChainDatum[callChainDatum.length - 2], callChainDatum[callChainDatum.length - 1]);
      }
    });
  }

  setPerfCallChainFrameName(callChain: PerfCallChain): void {
    //设置调用栈的名称
    callChain.canCharge = true;
    if (callChain.symbolId === -1) {
      if (this.filesData[callChain.fileId] && this.filesData[callChain.fileId].length > 0) {
        callChain.fileName = this.filesData[callChain.fileId][0].fileName;
        callChain.path = this.filesData[callChain.fileId][0].path;
      } else {
        callChain.fileName = 'unknown';
      }
    } else {
      if (this.filesData[callChain.fileId] && this.filesData[callChain.fileId].length > callChain.symbolId) {
        callChain.fileName = this.filesData[callChain.fileId][callChain.symbolId].fileName;
        callChain.path = this.filesData[callChain.fileId][callChain.symbolId].path;
      } else {
        callChain.fileName = 'unknown';
      }
    }
  }

  addPerfGroupData(callChain: PerfCallChain): void {
    const currentCallChain = this.callChainData[callChain.sampleId] || [];
    this.callChainData[callChain.sampleId] = currentCallChain;
    if (currentCallChain.length > maxDepth) {
      currentCallChain.splice(0, 1);
    }
    currentCallChain.push(callChain);
  }

  addOtherCallchainsData(countSample: PerfCountSample, list: any[]): void {
    let threadCallChain = new PerfCallChain(); //新增的线程数据
    threadCallChain.tid = countSample.tid;
    threadCallChain.canCharge = false;
    threadCallChain.isThread = true;
    threadCallChain.name = `${this.threadData[countSample.tid].threadName || 'Thread'}(${countSample.tid})`;
    let threadStateCallChain = new PerfCallChain(); //新增的线程状态数据
    threadStateCallChain.tid = countSample.tid;
    threadStateCallChain.name = countSample.threadState || 'Unknown State';
    threadStateCallChain.fileName = threadStateCallChain.name === '-' ? 'Unknown Thread State' : '';
    threadStateCallChain.canCharge = false;
    if (!this.isHideThreadState) {
      list.unshift(threadStateCallChain);
    }
    if (!this.isHideThread) {
      list.unshift(threadCallChain);
    }
  }

  freshPerfCallchains(perfCountSamples: PerfCountSample[], isTopDown: boolean): void {
    this.currentTreeMapData = {};
    this.currentTreeList = [];
    let totalSamplesCount = 0;
    let totalEventCount = 0;

    perfCountSamples.forEach((perfSample): void => {
      totalSamplesCount += perfSample.count;
      totalEventCount += perfSample.eventCount;
      if (this.callChainData[perfSample.sampleId] && this.callChainData[perfSample.sampleId].length > 0) {
        let perfCallChains = [...this.callChainData[perfSample.sampleId]];
        this.addOtherCallchainsData(perfSample, perfCallChains);
        let topIndex = isTopDown ? 0 : perfCallChains.length - 1;
        if (perfCallChains.length > 0) {
          let perfRootNode = this.currentTreeMapData[perfCallChains[topIndex].name + perfSample.pid];
          if (perfRootNode === undefined) {
            perfRootNode = new PerfCallChainMerageData();
            this.currentTreeMapData[perfCallChains[topIndex].name + perfSample.pid] = perfRootNode;
            this.currentTreeList.push(perfRootNode);
          }

          PerfCallChainMerageData.merageCallChainSample(perfRootNode, perfCallChains[topIndex], perfSample, false);
          this.mergeChildrenByIndex(perfRootNode, perfCallChains, topIndex, perfSample, isTopDown);
        }
      }
    });
    let rootMerageMap: any = {};
    // @ts-ignore
    Object.values(this.currentTreeMapData).forEach((merageData: any): void => {
      if (rootMerageMap[merageData.pid] === undefined) {
        let perfProcessMerageData = new PerfCallChainMerageData(); //新增进程的节点数据
        perfProcessMerageData.canCharge = false;
        perfProcessMerageData.symbolName =
          (this.threadData[merageData.tid].processName || 'Process') + `(${merageData.pid})`;
        perfProcessMerageData.isProcess = true;
        perfProcessMerageData.symbol = perfProcessMerageData.symbolName;
        perfProcessMerageData.tid = merageData.tid;
        perfProcessMerageData.children.push(merageData);
        perfProcessMerageData.initChildren.push(merageData);
        perfProcessMerageData.dur = merageData.dur;
        perfProcessMerageData.count = merageData.dur;
        perfProcessMerageData.eventCount = merageData.eventCount;
        perfProcessMerageData.total = totalSamplesCount;
        perfProcessMerageData.totalEvent = totalEventCount;
        perfProcessMerageData.tsArray = [...merageData.tsArray];
        rootMerageMap[merageData.pid] = perfProcessMerageData;
      } else {
        rootMerageMap[merageData.pid].children.push(merageData);
        rootMerageMap[merageData.pid].initChildren.push(merageData);
        rootMerageMap[merageData.pid].dur += merageData.dur;
        rootMerageMap[merageData.pid].count += merageData.dur;
        rootMerageMap[merageData.pid].eventCount += merageData.eventCount;
        rootMerageMap[merageData.pid].total = totalSamplesCount;
        rootMerageMap[merageData.pid].totalEvent = totalEventCount;
        for (const ts of merageData.tsArray) {
          rootMerageMap[merageData.pid].tsArray.push(ts);
        }
      }
      merageData.parentNode = rootMerageMap[merageData.pid]; //子节点添加父节点的引用
    });
    let id = 0;
    this.currentTreeList.forEach((perfTreeNode: any): void => {
      perfTreeNode.total = totalSamplesCount;
      perfTreeNode.totalEvent = totalEventCount;
      if (perfTreeNode.id === '') {
        perfTreeNode.id = id + '';
        id++;
      }
      if (perfTreeNode.parentNode) {
        if (perfTreeNode.parentNode.id === '') {
          perfTreeNode.parentNode.id = id + '';
          id++;
        }
        perfTreeNode.parentId = perfTreeNode.parentNode.id;
      }
    });
    // @ts-ignore
    this.allProcess = Object.values(rootMerageMap);
  }

  mergeChildrenByIndex(
    currentNode: PerfCallChainMerageData,
    callChainDataList: any[],
    index: number,
    sample: PerfCountSample,
    isTopDown: boolean
  ): void {
    if ((isTopDown && index >= callChainDataList.length - 1) || (!isTopDown && index <= 0)) {
      return;
    }
    isTopDown ? index++ : index--;
    let isEnd = isTopDown ? callChainDataList.length === index + 1 : index === 0;
    let node: PerfCallChainMerageData;
    if (
      currentNode.initChildren.filter((child: PerfCallChainMerageData): boolean => {
        if (child.symbolName === callChainDataList[index]?.name) {
          node = child;
          PerfCallChainMerageData.merageCallChainSample(child, callChainDataList[index], sample, isEnd);
          return true;
        }
        return false;
      }).length === 0
    ) {
      node = new PerfCallChainMerageData();
      PerfCallChainMerageData.merageCallChainSample(node, callChainDataList[index], sample, isEnd);
      currentNode.children.push(node);
      currentNode.initChildren.push(node);
      this.currentTreeList.push(node);
      node.parentNode = currentNode;
    }
    if (node! && !isEnd) this.mergeChildrenByIndex(node, callChainDataList, index, sample, isTopDown);
  }

  //所有的操作都是针对整个树结构的 不区分特定的数据
  splitPerfTree(samples: PerfCallChainMerageData[], name: string, isCharge: boolean, isSymbol: boolean): void {
    samples.forEach((process: PerfCallChainMerageData): void => {
      process.children = [];
      if (isCharge) {
        this.recursionPerfChargeInitTree(process, name, isSymbol);
      } else {
        this.recursionPerfPruneInitTree(process, name, isSymbol);
      }
    });
    this.resetAllNode(samples);
  }

  recursionPerfChargeInitTree(sample: PerfCallChainMerageData, symbolName: string, isSymbol: boolean): void {
    if ((isSymbol && sample.symbolName === symbolName) || (!isSymbol && sample.libName === symbolName)) {
      (this.splitMapData[symbolName] = this.splitMapData[symbolName] || []).push(sample);
      sample.isStore++;
    }
    if (sample.initChildren.length > 0) {
      sample.initChildren.forEach((child: PerfCallChainMerageData): void => {
        this.recursionPerfChargeInitTree(child, symbolName, isSymbol);
      });
    }
  }

  recursionPerfPruneInitTree(node: PerfCallChainMerageData, symbolName: string, isSymbol: boolean): void {
    if ((isSymbol && node.symbolName === symbolName) || (!isSymbol && node.libName === symbolName)) {
      (this.splitMapData[symbolName] = this.splitMapData[symbolName] || []).push(node);
      node.isStore++;
      this.pruneChildren(node, symbolName);
    } else if (node.initChildren.length > 0) {
      node.initChildren.forEach((child): void => {
        this.recursionPerfPruneInitTree(child, symbolName, isSymbol);
      });
    }
  }

  //symbol lib prune
  recursionPruneTree(sample: PerfCallChainMerageData, symbolName: string, isSymbol: boolean): void {
    if ((isSymbol && sample.symbolName === symbolName) || (!isSymbol && sample.libName === symbolName)) {
      sample.parent && sample.parent.children.splice(sample.parent.children.indexOf(sample), 1);
    } else {
      sample.children.forEach((child: PerfCallChainMerageData): void => {
        this.recursionPruneTree(child, symbolName, isSymbol);
      });
    }
  }

  recursionChargeByRule(
    sample: PerfCallChainMerageData,
    ruleName: string,
    rule: (node: PerfCallChainMerageData) => boolean
  ): void {
    if (sample.initChildren.length > 0) {
      sample.initChildren.forEach((child): void => {
        if (rule(child)) {
          (this.splitMapData[ruleName] = this.splitMapData[ruleName] || []).push(child);
          child.isStore++;
        }
        this.recursionChargeByRule(child, ruleName, rule);
      });
    }
  }

  pruneChildren(sample: PerfCallChainMerageData, symbolName: string): void {
    if (sample.initChildren.length > 0) {
      sample.initChildren.forEach((child: PerfCallChainMerageData): void => {
        child.isStore++;
        (this.splitMapData[symbolName] = this.splitMapData[symbolName] || []).push(child);
        this.pruneChildren(child, symbolName);
      });
    }
  }

  hideSystemLibrary(): void {
    this.allProcess.forEach((item: PerfCallChainMerageData): void => {
      item.children = [];
      this.recursionChargeByRule(item, systemRuleName, (node: PerfCallChainMerageData): boolean => {
        return node.path.startsWith(systemRuleName);
      });
    });
  }

  hideNumMaxAndMin(startNum: number, endNum: string): void {
    let max = endNum === '∞' ? Number.POSITIVE_INFINITY : parseInt(endNum);
    this.allProcess.forEach((item: PerfCallChainMerageData): void => {
      item.children = [];
      this.recursionChargeByRule(item, numRuleName, (node: PerfCallChainMerageData): boolean => {
        return node.dur < startNum || node.dur > max;
      });
    });
  }

  clearSplitMapData(symbolName: string): void {
    delete this.splitMapData[symbolName];
  }

  resetAllSymbol(symbols: string[]): void {
    symbols.forEach((symbol: string): void => {
      let list = this.splitMapData[symbol];
      if (list !== undefined) {
        list.forEach((item: any): void => {
          item.isStore--;
        });
      }
    });
  }

  resetAllNode(sample: PerfCallChainMerageData[]): void {
    this.clearSearchNode();
    sample.forEach((process: PerfCallChainMerageData): void => {
      process.searchShow = true;
      process.isSearch = false;
    });
    this.resetNewAllNode(sample);
    if (this.searchValue !== '') {
      this.findSearchNode(sample, this.searchValue, false);
      this.resetNewAllNode(sample);
    }
  }

  resetNewAllNode(sampleArray: PerfCallChainMerageData[]): void {
    sampleArray.forEach((process: PerfCallChainMerageData): void => {
      process.children = [];
    });
    let values = this.currentTreeList.map((item: any): any => {
      item.children = [];
      return item;
    });
    values.forEach((sample: any): void => {
      if (sample.parentNode !== undefined) {
        if (sample.isStore === 0 && sample.searchShow) {
          let parentNode = sample.parentNode;
          while (parentNode !== undefined && !(parentNode.isStore === 0 && parentNode.searchShow)) {
            parentNode = parentNode.parentNode;
          }
          if (parentNode) {
            sample.currentTreeParentNode = parentNode;
            parentNode.children.push(sample);
          }
        }
      }
    });
  }

  findSearchNode(sampleArray: PerfCallChainMerageData[], search: string, parentSearch: boolean): void {
    search = search.toLocaleLowerCase();
    sampleArray.forEach((sample: PerfCallChainMerageData): void => {
      if ((sample.symbol && sample.symbol.toLocaleLowerCase().includes(search)) || parentSearch) {
        sample.searchShow = true;
        let parentNode = sample.parent;
        sample.isSearch = sample.symbol !== undefined && sample.symbol.toLocaleLowerCase().includes(search);
        while (parentNode !== undefined && !parentNode.searchShow) {
          parentNode.searchShow = true;
          parentNode = parentNode.parent;
        }
      } else {
        sample.searchShow = false;
        sample.isSearch = false;
      }
      if (sample.children.length > 0) {
        this.findSearchNode(sample.children, search, sample.searchShow);
      }
    });
  }

  clearSearchNode(): void {
    this.currentTreeList.forEach((sample: any): void => {
      sample.searchShow = true;
      sample.isSearch = false;
    });
  }

  splitAllProcess(processArray: any[]): void {
    processArray.forEach((item: any): void => {
      this.allProcess.forEach((process): void => {
        if (item.select === '0') {
          this.recursionPerfChargeInitTree(process, item.name, item.type === 'symbol');
        } else {
          this.recursionPerfPruneInitTree(process, item.name, item.type === 'symbol');
        }
      });
      if (!item.checked) {
        this.resetAllSymbol([item.name]);
      }
    });
  }

  resolvingAction(params: any[]): PerfCallChainMerageData[] | PerfAnalysisSample[] | PerfBottomUpStruct[] {
    if (params.length > 0) {
      for (let item of params) {
        if (item.funcName && item.funcArgs) {
          switch (item.funcName) {
            case 'getCallChainsBySampleIds':
              this.freshPerfCallchains(this.samplesData, item.funcArgs[0]);
              break;
            case 'getCurrentDataFromDb':
              if (item.funcArgs[1]) {
                let funcArgs = item.funcArgs[1];
                let sql = '';
                if (funcArgs.processId !== undefined) {
                  sql += `and thread.process_id = ${funcArgs.processId}`;
                }
                if (funcArgs.threadId !== undefined) {
                  sql += ` and s.thread_id = ${funcArgs.threadId}`;
                }
                this.getCurrentDataFromDb(item.funcArgs[0], sql);
              } else {
                this.getCurrentDataFromDb(item.funcArgs[0]);
              }
              break;
            case 'hideSystemLibrary':
              this.hideSystemLibrary();
              break;
            case 'hideThread':
              this.isHideThread = item.funcArgs[0];
              break;
            case 'hideThreadState':
              this.isHideThreadState = item.funcArgs[0];
              break;
            case 'hideNumMaxAndMin':
              this.hideNumMaxAndMin(item.funcArgs[0], item.funcArgs[1]);
              break;
            case 'splitAllProcess':
              this.splitAllProcess(item.funcArgs[0]);
              break;
            case 'resetAllNode':
              this.resetAllNode(this.allProcess);
              break;
            case 'resotreAllNode':
              this.resetAllSymbol(item.funcArgs[0]);
              break;
            case 'clearSplitMapData':
              this.clearSplitMapData(item.funcArgs[0]);
              break;
            case 'splitTree':
              this.splitPerfTree(this.allProcess, item.funcArgs[0], item.funcArgs[1], item.funcArgs[2]);
              break;
            case 'setSearchValue':
              this.searchValue = item.funcArgs[0];
              break;
            case 'setCombineCallChain':
              this.isAnalysis = true;
              break;
            case 'setPerfBottomUp':
              this.isPerfBottomUp = true;
              break;
            case 'combineAnalysisCallChain':
              return this.combineCallChainForAnalysis();
            case 'getBottomUp':
              return this.getBottomUp();
          }
        }
      }
      this.dataSource = this.allProcess.filter((process: PerfCallChainMerageData): boolean => {
        return process.children && process.children.length > 0;
      });
    }
    return this.dataSource;
  }

  combineCallChainForAnalysis(obj?: any): PerfAnalysisSample[] {
    let sampleCallChainList: Array<PerfAnalysisSample> = [];
    for (let sample of this.samplesData) {
      let callChains = [...this.callChainData[sample.sampleId]];
      const lastCallChain = callChains[callChains.length - 1];
      const threadName = this.threadData[sample.tid].threadName || 'Thread';
      const processName = this.threadData[sample.pid].threadName || 'Process';
      if (
        (obj && obj.libId === lastCallChain.fileId && obj.libName === lastCallChain.fileName) ||
        (obj && obj.symbolId === lastCallChain.symbolId && obj.symbolName === lastCallChain.name) ||
        !obj
      ) {
        let analysisSample = new PerfAnalysisSample(
          threadName,
          processName,
          lastCallChain.fileId,
          lastCallChain.fileName,
          lastCallChain.symbolId,
          lastCallChain.name
        );
        analysisSample.tid = sample.tid;
        analysisSample.pid = sample.pid;
        analysisSample.count = sample.count;
        analysisSample.threadState = sample.threadState;
        analysisSample.eventCount = sample.eventCount;
        analysisSample.sampleId = sample.sampleId;
        sampleCallChainList.push(analysisSample);
      }
    }
    if (this.isAnalysis) {
      this.isAnalysis = false;
    }
    return sampleCallChainList;
  }

  getBottomUp(): PerfBottomUpStruct[] {
    const topUp = new PerfBottomUpStruct('topUp');
    let perfTime = 1;
    for (let sample of this.samplesData) {
      let currentNode = topUp;
      let callChains = this.callChainData[sample.sampleId];
      for (let i = 0; i < callChains.length; i++) {
        if (i === 0) {
          currentNode = topUp;
        }
        let item = callChains[i];
        const existingNode = currentNode.children.find(
          (child) => child.symbolName === `${item.name}(${item.fileName})`
        );
        if (existingNode) {
          existingNode.tsArray.push(...sample.ts.split(',').map(Number));
          currentNode = existingNode;
          existingNode.totalTime += perfTime * sample.count;
          existingNode.eventCount += sample.eventCount;
          existingNode.calculateSelfTime();
          existingNode.notifyParentUpdateSelfTime();
        } else {
          let newNode = new PerfBottomUpStruct(`${item.name}(${item.fileName})`);
          newNode.totalTime = perfTime * sample.count;
          newNode.eventCount = sample.eventCount;
          newNode.tsArray = sample.ts.split(',').map(Number);
          currentNode.addChildren(newNode);
          newNode.calculateSelfTime();
          newNode.notifyParentUpdateSelfTime();
          currentNode = newNode;
        }
      }
    }
    topUp.children.forEach((child: PerfBottomUpStruct): void => {
      child.parentNode = undefined;
    });

    let date = this.topUpDataToBottomUpData(topUp.children);
    if (this.isPerfBottomUp) {
      this.isPerfBottomUp = false;
    }
    return date;
  }

  private topUpDataToBottomUpData(perfPositiveArray: Array<PerfBottomUpStruct>): Array<PerfBottomUpStruct> {
    let reverseTreeArray: Array<PerfBottomUpStruct> = [];
    const recursionTree = (perfBottomUpStruct: PerfBottomUpStruct): void => {
      if (perfBottomUpStruct.selfTime > 0) {
        const clonePerfBottomUpStruct = new PerfBottomUpStruct(perfBottomUpStruct.symbolName);
        clonePerfBottomUpStruct.selfTime = perfBottomUpStruct.selfTime;
        clonePerfBottomUpStruct.totalTime = perfBottomUpStruct.totalTime;
        clonePerfBottomUpStruct.eventCount = perfBottomUpStruct.eventCount;
        clonePerfBottomUpStruct.tsArray = [...perfBottomUpStruct.tsArray];
        reverseTreeArray.push(clonePerfBottomUpStruct);
        this.copyParentNode(clonePerfBottomUpStruct, perfBottomUpStruct);
      }
      if (perfBottomUpStruct.children.length > 0) {
        for (const children of perfBottomUpStruct.children) {
          children.parentNode = perfBottomUpStruct;
          recursionTree(children);
        }
      }
    };
    for (const perfBottomUpStruct of perfPositiveArray) {
      recursionTree(perfBottomUpStruct);
    }
    return this.mergeTreeBifurcation(reverseTreeArray, null);
  }

  private mergeTreeBifurcation(
    reverseTreeArray: Array<PerfBottomUpStruct> | null,
    parent: PerfBottomUpStruct | null
  ): Array<PerfBottomUpStruct> {
    const sameSymbolMap = new Map<string, PerfBottomUpStruct>();
    const currentLevelData: Array<PerfBottomUpStruct> = [];
    const dataArray = reverseTreeArray || parent?.frameChildren;
    if (!dataArray) {
      return [];
    }
    for (const perfBottomUpStruct of dataArray) {
      let symbolKey = perfBottomUpStruct.symbolName;
      let bottomUpStruct: PerfBottomUpStruct;
      if (sameSymbolMap.has(symbolKey)) {
        bottomUpStruct = sameSymbolMap.get(symbolKey)!;
        bottomUpStruct.totalTime += perfBottomUpStruct.totalTime;
        bottomUpStruct.selfTime += perfBottomUpStruct.selfTime;
        for (const ts of perfBottomUpStruct.tsArray) {
          bottomUpStruct.tsArray.push(ts);
        }
      } else {
        bottomUpStruct = perfBottomUpStruct;
        sameSymbolMap.set(symbolKey, bottomUpStruct);
        currentLevelData.push(bottomUpStruct);
        if (parent) {
          parent.addChildren(bottomUpStruct);
        }
      }
      bottomUpStruct.frameChildren?.push(...perfBottomUpStruct.children);
    }

    for (const data of currentLevelData) {
      this.mergeTreeBifurcation(null, data);
      data.frameChildren = [];
    }
    if (reverseTreeArray) {
      return currentLevelData;
    } else {
      return [];
    }
  }

  /**
   * copy整体调用链，从栈顶函数一直copy到栈底函数，
   * 给Parent设置selfTime，totalTime设置为children的selfTime,totalTime
   *  */
  private copyParentNode(perfBottomUpStruct: PerfBottomUpStruct, bottomUpStruct: PerfBottomUpStruct): void {
    if (bottomUpStruct.parentNode) {
      const copyParent = new PerfBottomUpStruct(bottomUpStruct.parentNode.symbolName);
      copyParent.selfTime = perfBottomUpStruct.selfTime;
      copyParent.totalTime = perfBottomUpStruct.totalTime;
      copyParent.eventCount = perfBottomUpStruct.eventCount;
      copyParent.tsArray = [...perfBottomUpStruct.tsArray];
      perfBottomUpStruct.addChildren(copyParent);
      this.copyParentNode(copyParent, bottomUpStruct.parentNode);
    }
  }
}

export class PerfFile {
  fileId: number = 0;
  symbol: string = '';
  path: string = '';
  fileName: string = '';

  static setFileName(data: PerfFile): void {
    if (data.path) {
      let number = data.path.lastIndexOf('/');
      if (number > 0) {
        data.fileName = data.path.substring(number + 1);
        return;
      }
    }
    data.fileName = data.path;
  }

  setFileName(): void {
    if (this.path) {
      let number = this.path.lastIndexOf('/');
      if (number > 0) {
        this.fileName = this.path.substring(number + 1);
        return;
      }
    }
    this.fileName = this.path;
  }
}

export class PerfThread {
  tid: number = 0;
  pid: number = 0;
  threadName: string = '';
  processName: string = '';
}

export class PerfCallChain {
  startNS: number = 0;
  dur: number = 0;
  sampleId: number = 0;
  callChainId: number = 0;
  vaddrInFile: number = 0;
  tid: number = 0;
  pid: number = 0;
  name: string = '';
  fileName: string = '';
  threadState: string = '';
  fileId: number = 0;
  symbolId: number = 0;
  path: string = '';
  count: number = 0;
  eventCount: number = 0;
  parentId: string = ''; //合并之后区分的id
  id: string = '';
  topDownMerageId: string = ''; //top down合并使用的id
  topDownMerageParentId: string = ''; //top down合并使用的id
  bottomUpMerageId: string = ''; //bottom up合并使用的id
  bottomUpMerageParentId: string = ''; //bottom up合并使用的id
  depth: number = 0;
  canCharge: boolean = true;
  previousNode: PerfCallChain | undefined = undefined; //将list转换为一个链表结构
  nextNode: PerfCallChain | undefined = undefined;
  isThread: boolean = false;
  isProcess: boolean = false;

  static setNextNode(currentNode: PerfCallChain, nextNode: PerfCallChain): void {
    currentNode.nextNode = nextNode;
    nextNode.previousNode = currentNode;
  }

  static setPreviousNode(currentNode: PerfCallChain, prevNode: PerfCallChain): void {
    currentNode.previousNode = prevNode;
    prevNode.nextNode = currentNode;
  }

  static merageCallChain(currentNode: PerfCallChain, callChain: PerfCallChain): void {
    currentNode.startNS = callChain.startNS;
    currentNode.tid = callChain.tid;
    currentNode.pid = callChain.pid;
    currentNode.sampleId = callChain.sampleId;
    currentNode.dur = callChain.dur;
    currentNode.count = callChain.count;
    currentNode.eventCount = callChain.eventCount;
  }
}

export class PerfCallChainMerageData extends ChartStruct {
  #parentNode: PerfCallChainMerageData | undefined = undefined;
  #total = 0;
  #totalEvent = 0;
  id: string = '';
  parentId: string = '';
  parent: PerfCallChainMerageData | undefined = undefined;
  symbolName: string = '';
  symbol: string = '';
  libName: string = '';
  path: string = '';
  weight: string = '';
  weightPercent: string = '';
  selfDur: number = 0;
  dur: number = 0;
  tid: number = 0;
  pid: number = 0;
  isStore = 0;
  canCharge: boolean = true;
  children: PerfCallChainMerageData[] = [];
  initChildren: PerfCallChainMerageData[] = [];
  type: number = 0;
  vaddrInFile: number = 0;
  isSelected: boolean = false;
  searchShow: boolean = true;
  isSearch: boolean = false;
  set parentNode(data: PerfCallChainMerageData | undefined) {
    this.parent = data;
    this.#parentNode = data;
  }

  get parentNode(): PerfCallChainMerageData | undefined {
    return this.#parentNode;
  }

  set total(data: number) {
    this.#total = data;
    this.weight = `${this.dur}`;
    this.weightPercent = `${((this.dur / data) * 100).toFixed(1)}%`;
  }

  get total(): number {
    return this.#total;
  }

  set totalEvent(data: number) {
    this.#totalEvent = data;
    this.eventPercent = `${((this.eventCount / data) * 100).toFixed(1)}%`;
  }

  get totalEvent(): number {
    return this.#totalEvent;
  }

  static merageCallChain(currentNode: PerfCallChainMerageData, callChain: PerfCallChain, isTopDown: boolean): void {
    if (currentNode.symbolName === '') {
      currentNode.symbol = `${callChain.name}  ${callChain.fileName ? `(${callChain.fileName})` : ''}`;
      currentNode.symbolName = callChain.name;
      currentNode.pid = callChain.pid;
      currentNode.tid = callChain.tid;
      currentNode.libName = callChain.fileName;
      currentNode.vaddrInFile = callChain.vaddrInFile;
      currentNode.addr = `${'0x'}${callChain.vaddrInFile.toString(16)}`;
      currentNode.lib = currentNode.libName;
      currentNode.canCharge = callChain.canCharge;
      if (callChain.path) {
        currentNode.path = callChain.path;
      }
    }
    if (callChain[isTopDown ? 'nextNode' : 'previousNode'] === undefined) {
      currentNode.selfDur += callChain.count;
    }
    currentNode.dur += callChain.count;
    currentNode.count += callChain.count;
    currentNode.eventCount += callChain.eventCount;
  }
  static merageCallChainSample(
    currentNode: PerfCallChainMerageData,
    callChain: PerfCallChain,
    sample: PerfCountSample,
    isEnd: boolean
  ): void {
    if (currentNode.symbolName === '') {
      currentNode.symbol = `${callChain.name}  ${callChain.fileName ? `(${callChain.fileName})` : ''}`;
      currentNode.symbolName = callChain.name;
      currentNode.pid = sample.pid;
      currentNode.tid = sample.tid;
      currentNode.libName = callChain.fileName;
      currentNode.vaddrInFile = callChain.vaddrInFile;
      currentNode.lib = callChain.fileName;
      currentNode.addr = `${'0x'}${callChain.vaddrInFile.toString(16)}`;
      currentNode.canCharge = callChain.canCharge;
      if (callChain.path) {
        currentNode.path = callChain.path;
      }
    }
    if (isEnd) {
      currentNode.selfDur += sample.count;
    }
    if (callChain.isThread && !currentNode.isThread) {
      currentNode.isThread = callChain.isThread;
    }
    currentNode.dur += sample.count;
    currentNode.count += sample.count;
    currentNode.eventCount += sample.eventCount;
    currentNode.tsArray.push(...sample.ts.split(',').map(Number));
  }
}

export class PerfCountSample {
  sampleId: number = 0;
  tid: number = 0;
  count: number = 0;
  threadState: string = '';
  pid: number = 0;
  eventCount: number = 0;
  ts: string = '';
}

export class PerfStack {
  symbol: string = '';
  path: string = '';
  fileId: number = 0;
  type: number = 0;
  vaddrInFile: number = 0;
}

export class PerfCmdLine {
  report_value: string = '';
}

class PerfAnalysisSample extends PerfCountSample {
  threadName: string;
  processName: string;
  libId: number;
  libName: string;
  symbolId: number;
  symbolName: string;

  constructor(
    threadName: string,
    processName: string,
    libId: number,
    libName: string,
    symbolId: number,
    symbolName: string
  ) {
    super();
    this.threadName = threadName;
    this.processName = processName;
    this.libId = libId;
    this.libName = libName;
    this.symbolId = symbolId;
    this.symbolName = symbolName;
  }
}

export function timeMsFormat2p(ns: number): string {
  let currentNs = ns;
  let hour1 = 3600_000;
  let minute1 = 60_000;
  let second1 = 1_000; // 1 second
  let perfResult = '';
  if (currentNs >= hour1) {
    perfResult += `${Math.floor(currentNs / hour1).toFixed(2)}h`;
    return perfResult;
  }
  if (currentNs >= minute1) {
    perfResult += `${Math.floor(currentNs / minute1).toFixed(2)}min`;
    return perfResult;
  }
  if (currentNs >= second1) {
    perfResult += `${Math.floor(currentNs / second1).toFixed(2)}s`;
    return perfResult;
  }
  if (currentNs > 0) {
    perfResult += `${currentNs.toFixed(2)}ms`;
    return perfResult;
  }
  if (perfResult === '') {
    perfResult = '0s';
  }
  return perfResult;
}

class HiPrefSample {
  name: string = '';
  depth: number = 0;
  callchain_id: number = 0;
  totalTime: number = 0;
  thread_id: number = 0;
  id: number = 0;
  eventCount: number = 0;
  startTime: number = 0;
  endTime: number = 0;
  timeTip: number = 0;
  cpu_id: number = 0;
  stack?: Array<HiPerfSymbol>;
}
