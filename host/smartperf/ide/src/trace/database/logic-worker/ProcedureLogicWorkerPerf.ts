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

import { LogicHandler, ChartStruct, convertJSON, DataCache, PerfCall } from './ProcedureLogicWorkerCommon.js';
import { PerfBottomUpStruct } from '../../bean/PerfBottomUpStruct.js';

const systemRuleName = '/system/';
const numRuleName = '/max/min/';

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

  private dataCache = DataCache.getInstance();

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
          threads.forEach((thread: any) => {
            this.threadData[thread.tid] = thread;
          });
          this.initPerfCalls();
          break;
        case 'perf-queryPerfCalls':
          let perfCalls = convertJSON(data.params.list) || [];
          if (perfCalls.length !== 0) {
            perfCalls.forEach((perfCall: any) => {
              this.dataCache.perfCallChainMap.set(perfCall.sampleId, perfCall);
            });
          }
          this.initPerfCallchains();
          break;
        case 'perf-queryPerfCallchains':
          let arr = convertJSON(data.params.list) || [];
          this.initPerfCallChainTopDown(arr);
          // @ts-ignore
          self.postMessage({
            id: data.id,
            action: data.action,
            results: this.dataCache.perfCallChainMap,
          });
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
            result = this.resolvingAction([
              {
                funcName: 'getCallChainsBySampleIds',
                funcArgs: [true],
              },
            ]);
          }
          self.postMessage({
            id: data.id,
            action: data.action,
            results: result,
          });
          break;
        case 'perf-action':
          if (data.params) {
            let filter = data.params.filter((item: any) => item.funcName == 'getCurrentDataFromDb');
            if (filter.length == 0) {
              // @ts-ignore
              self.postMessage({
                id: data.id,
                action: data.action,
                results: this.resolvingAction(data.params),
              });
            } else {
              this.resolvingAction(data.params);
            }
          }
          break;
      }
    }
  }

  initPerfFiles() {
    this.clearAll();
    this.queryData(
      this.currentEventId,
      'perf-queryPerfFiles',
      `select file_id as fileId, symbol, path
       from perf_files`,
      {}
    );
  }

  initPerfThreads() {
    this.queryData(
      this.currentEventId,
      'perf-queryPerfThread',
      `select a.thread_id as tid, a.thread_name as threadName, a.process_id as pid, b.thread_name as processName
       from perf_thread a
                left join (select * from perf_thread where thread_id = process_id) b on a.process_id = b.thread_id`,
      {}
    );
  }

  initPerfCalls() {
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

  initPerfCallchains() {
    this.queryData(
      this.currentEventId,
      'perf-queryPerfCallchains',
      `select c.name,
              c.callchain_id  as sampleId,
              c.vaddr_in_file as vaddrInFile,
              c.file_id       as fileId,
              c.symbol_id     as symbolId
       from perf_callchain c
       where callchain_id != -1;`,
      {}
    );
  }

  getCurrentDataFromDb(selectionParam: any) {
    let cpus = selectionParam.perfAll ? [] : selectionParam.perfCpus;
    let processes = selectionParam.perfAll ? [] : selectionParam.perfProcess;
    let threads = selectionParam.perfAll ? [] : selectionParam.perfThread;
    let sql = '';
    if (cpus.length != 0 || processes.length != 0 || threads.length != 0) {
      let arg1 = cpus.length > 0 ? `or s.cpu_id in (${cpus.join(',')}) ` : '';
      let arg2 = processes.length > 0 ? `or thread.process_id in (${processes.join(',')}) ` : '';
      let arg3 = threads.length > 0 ? `or s.thread_id in (${threads.join(',')})` : '';
      let arg = `${arg1}${arg2}${arg3}`.substring(3);
      sql = ` and (${arg})`;
    }
    this.queryData(
      this.currentEventId,
      'perf-queryCallchainsGroupSample',
      `
          select p.callchain_id as sampleId,
                 p.thread_state as threadState,
                 p.thread_id    as tid,
                 p.count,
                 p.process_id   as pid,
                 p.event_count  as eventCount
          from (select callchain_id, s.thread_id, thread_state, process_id, count(callchain_id) as count,event_count
                from perf_sample s, trace_range t
                    left join perf_thread thread
                on s.thread_id = thread.thread_id
                where timestamp_trace between $startTime + t.start_ts
                  and $endTime + t.start_ts
                  and callchain_id != -1
                  and s.thread_id != 0 ${sql}
                group by callchain_id, s.thread_id, thread_state, process_id) p`,
      {
        $startTime: selectionParam.leftNs,
        $endTime: selectionParam.rightNs,
        $sql: sql,
      }
    );
  }

  clearAll() {
    this.filesData = {};
    this.samplesData = {};
    this.threadData = {};
    this.callChainData = {};
    this.splitMapData = {};
    this.currentTreeMapData = {};
    this.currentTreeList = [];
    this.searchValue = '';
    this.dataSource = [];
    this.allProcess = [];
    this.dataCache.clearPerf();
  }

  initPerfCallChainBottomUp(callChains: PerfCallChain[]) {
    callChains.forEach((callChain, index) => {
      if (this.threadData[callChain.tid] == undefined) {
        return;
      }
      this.setPerfCallChainFrameName(callChain);
      this.addPerfGroupData(callChain);
      if (index + 1 < callChains.length && callChains[index + 1].sampleId == callChain.sampleId) {
        PerfCallChain.setPreviousNode(callChain, callChains[index + 1]);
      }
      if (callChains.length == index + 1 || callChains[index + 1].sampleId != callChain.sampleId) {
        this.addProcessThreadStateData(callChain);
      }
    });
  }

  initPerfCallChainTopDown(callChains: PerfCallChain[]) {
    this.callChainData = {};
    callChains.forEach((callChain, index) => {
      this.setPerfCallChainFrameName(callChain);
      this.addPerfGroupData(callChain);
      let callChainDatum = this.callChainData[callChain.sampleId];
      if (callChainDatum.length > 1) {
        PerfCallChain.setNextNode(callChainDatum[callChainDatum.length - 2], callChainDatum[callChainDatum.length - 1]);
      }
    });
  }

  setPerfCallChainFrameName(callChain: PerfCallChain) {
    //设置调用栈的名称
    callChain.canCharge = true;
    if (callChain.symbolId == -1) {
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

  addProcessThreadStateData(callChain: PerfCallChain) {
    //当调用栈为调用的根节点时
    this.addPerfCallData(callChain);
    let threadCallChain = new PerfCallChain(); //新增的线程数据
    threadCallChain.depth = 0;
    PerfCallChain.merageCallChain(threadCallChain, callChain);
    threadCallChain.canCharge = false;
    threadCallChain.name = (this.threadData[callChain.tid].threadName || 'Thread') + '(' + callChain.tid + ')';
    let threadStateCallChain = new PerfCallChain(); //新增的线程状态数据
    PerfCallChain.merageCallChain(threadStateCallChain, callChain);
    threadStateCallChain.name = callChain.threadState || 'Unknown State';
    threadStateCallChain.fileName = threadStateCallChain.name === '-' ? 'Unknown Thread State' : '';
    threadStateCallChain.canCharge = false;
    this.addPerfGroupData(threadCallChain);
    this.addPerfGroupData(threadStateCallChain);
    PerfCallChain.setNextNode(threadCallChain, threadStateCallChain);
    PerfCallChain.setNextNode(threadStateCallChain, callChain);
  }

  addPerfCallData(callChain: PerfCallChain) {
    let perfCall = new PerfCall();
    perfCall.depth = this.callChainData[callChain.sampleId]?.length || 0;
    perfCall.sampleId = callChain.sampleId;
    perfCall.name = callChain.name;
    this.dataCache.perfCallChainMap.set(callChain.sampleId, perfCall);
  }

  addPerfGroupData(callChain: PerfCallChain) {
    this.callChainData[callChain.sampleId] = this.callChainData[callChain.sampleId] || [];
    this.callChainData[callChain.sampleId].push(callChain);
  }

  getPerfCallChainsBySampleIds(sampleIds: string[], isTopDown: boolean) {
    this.allProcess = this.groupNewTreeNoId(sampleIds, isTopDown);
    return this.allProcess;
  }

  addOtherCallchainsData(countSample: PerfCountSample, list: any[]) {
    let threadCallChain = new PerfCallChain(); //新增的线程数据
    threadCallChain.tid = countSample.tid;
    threadCallChain.canCharge = false;
    threadCallChain.name = this.threadData[countSample.tid].threadName || 'Thread' + '(' + countSample.tid + ')';
    let threadStateCallChain = new PerfCallChain(); //新增的线程状态数据
    threadStateCallChain.tid = countSample.tid;
    threadStateCallChain.name = countSample.threadState || 'Unknown State';
    threadStateCallChain.fileName = threadStateCallChain.name == '-' ? 'Unknown Thread State' : '';
    threadStateCallChain.canCharge = false;
    list.unshift(threadCallChain, threadStateCallChain);
  }

  freshPerfCallchains(perfCountSamples: PerfCountSample[], isTopDown: boolean) {
    this.currentTreeMapData = {};
    this.currentTreeList = [];
    let totalSamplesCount = 0;
    perfCountSamples.forEach((perfSample) => {
      totalSamplesCount += perfSample.count;
      if (this.callChainData[perfSample.sampleId] && this.callChainData[perfSample.sampleId].length > 0) {
        let perfCallChains = [...this.callChainData[perfSample.sampleId]];
        this.addOtherCallchainsData(perfSample, perfCallChains);
        let topIndex = isTopDown ? 0 : perfCallChains.length - 1;
        if (perfCallChains.length > 0) {
          let perfRootNode = this.currentTreeMapData[perfCallChains[topIndex].name + perfSample.pid];
          if (perfRootNode == undefined) {
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
    Object.values(this.currentTreeMapData).forEach((merageData: any) => {
      if (rootMerageMap[merageData.pid] == undefined) {
        let perfProcessMerageData = new PerfCallChainMerageData(); //新增进程的节点数据
        perfProcessMerageData.canCharge = false;
        perfProcessMerageData.symbolName =
          (this.threadData[merageData.tid].processName || 'Process') + `(${merageData.pid})`;
        perfProcessMerageData.symbol = perfProcessMerageData.symbolName;
        perfProcessMerageData.tid = merageData.tid;
        perfProcessMerageData.children.push(merageData);
        perfProcessMerageData.initChildren.push(merageData);
        perfProcessMerageData.dur = merageData.dur;
        perfProcessMerageData.count = merageData.dur;
        perfProcessMerageData.total = totalSamplesCount;
        rootMerageMap[merageData.pid] = perfProcessMerageData;
      } else {
        rootMerageMap[merageData.pid].children.push(merageData);
        rootMerageMap[merageData.pid].initChildren.push(merageData);
        rootMerageMap[merageData.pid].dur += merageData.dur;
        rootMerageMap[merageData.pid].count += merageData.dur;
        rootMerageMap[merageData.pid].total = totalSamplesCount;
      }
      merageData.parentNode = rootMerageMap[merageData.pid]; //子节点添加父节点的引用
    });
    let id = 0;
    this.currentTreeList.forEach((perfTreeNode) => {
      perfTreeNode.total = totalSamplesCount;
      if (perfTreeNode.id == '') {
        perfTreeNode.id = id + '';
        id++;
      }
      if (perfTreeNode.parentNode) {
        if (perfTreeNode.parentNode.id == '') {
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
  ) {
    isTopDown ? index++ : index--;
    let isEnd = isTopDown ? callChainDataList.length == index + 1 : index == 0;
    let node;
    if (
      currentNode.initChildren.filter((child: PerfCallChainMerageData) => {
        if (child.symbolName == callChainDataList[index]?.name) {
          node = child;
          PerfCallChainMerageData.merageCallChainSample(child, callChainDataList[index], sample, isEnd);
          return true;
        }
        return false;
      }).length == 0
    ) {
      node = new PerfCallChainMerageData();
      PerfCallChainMerageData.merageCallChainSample(node, callChainDataList[index], sample, isEnd);
      currentNode.children.push(node);
      currentNode.initChildren.push(node);
      this.currentTreeList.push(node);
      node.parentNode = currentNode;
    }
    if (node && !isEnd) this.mergeChildrenByIndex(node, callChainDataList, index, sample, isTopDown);
  }

  groupNewTreeNoId(sampleIds: string[], isTopDown: boolean): any[] {
    this.currentTreeMapData = {};
    this.currentTreeList = [];
    for (let i = 0; i < sampleIds.length; i++) {
      let callChains = this.callChainData[sampleIds[i]];
      if (callChains == undefined) continue;
      let topIndex = isTopDown ? 0 : callChains.length - 1;
      if (callChains.length > 0) {
        let root = this.currentTreeMapData[callChains[topIndex].name + callChains[topIndex].pid];
        if (root == undefined) {
          root = new PerfCallChainMerageData();
          this.currentTreeMapData[callChains[topIndex].name + callChains[topIndex].pid] = root;
          this.currentTreeList.push(root);
        }
        PerfCallChainMerageData.merageCallChain(root, callChains[topIndex], isTopDown);
        this.merageChildren(root, callChains[topIndex], isTopDown);
      }
    }
    let rootMerageMap: any = {};
    // @ts-ignore
    Object.values(this.currentTreeMapData).forEach((merageData: any) => {
      if (rootMerageMap[merageData.pid] == undefined) {
        let processMerageData = new PerfCallChainMerageData(); //新增进程的节点数据
        processMerageData.canCharge = false;
        processMerageData.symbolName = this.threadData[merageData.tid].processName || `Process(${merageData.pid})`;
        processMerageData.symbol = processMerageData.symbolName;
        processMerageData.tid = merageData.tid;
        processMerageData.children.push(merageData);
        processMerageData.initChildren.push(merageData);
        processMerageData.dur = merageData.dur;
        processMerageData.count = merageData.dur;
        processMerageData.total = sampleIds.length;
        rootMerageMap[merageData.pid] = processMerageData;
      } else {
        rootMerageMap[merageData.pid].children.push(merageData);
        rootMerageMap[merageData.pid].initChildren.push(merageData);
        rootMerageMap[merageData.pid].dur += merageData.dur;
        rootMerageMap[merageData.pid].count += merageData.dur;
        rootMerageMap[merageData.pid].total = sampleIds.length;
      }
      merageData.parentNode = rootMerageMap[merageData.pid]; //子节点添加父节点的引用
    });
    let id = 0;
    this.currentTreeList.forEach((node) => {
      node.total = sampleIds.length;
      if (node.id == '') {
        node.id = id + '';
        id++;
      }
      if (node.parentNode) {
        if (node.parentNode.id == '') {
          node.parentNode.id = id + '';
          id++;
        }
        node.parentId = node.parentNode.id;
      }
    });
    // @ts-ignore
    return Object.values(rootMerageMap);
  }

  merageChildren(currentNode: PerfCallChainMerageData, callChain: any, isTopDown: boolean) {
    let nextNodeKey = isTopDown ? 'nextNode' : 'previousNode';
    if (callChain[nextNodeKey] == undefined) return;
    let node;
    if (
      currentNode.initChildren.filter((child: PerfCallChainMerageData) => {
        if (child.symbolName == callChain[nextNodeKey]?.name) {
          node = child;
          PerfCallChainMerageData.merageCallChain(child, callChain[nextNodeKey], isTopDown);
          return true;
        }
        return false;
      }).length == 0
    ) {
      node = new PerfCallChainMerageData();
      PerfCallChainMerageData.merageCallChain(node, callChain[nextNodeKey], isTopDown);
      currentNode.children.push(node);
      currentNode.initChildren.push(node);
      this.currentTreeList.push(node);
      node.parentNode = currentNode;
    }
    if (node) this.merageChildren(node, callChain[nextNodeKey], isTopDown);
  }

  //所有的操作都是针对整个树结构的 不区分特定的数据
  splitPerfTree(samples: PerfCallChainMerageData[], name: string, isCharge: boolean, isSymbol: boolean) {
    samples.forEach((process) => {
      process.children = [];
      if (isCharge) {
        this.recursionPerfChargeInitTree(process, name, isSymbol);
      } else {
        this.recursionPerfPruneInitTree(process, name, isSymbol);
      }
    });
    this.resetAllNode(samples);
  }

  recursionPerfChargeInitTree(sample: PerfCallChainMerageData, symbolName: string, isSymbol: boolean) {
    if ((isSymbol && sample.symbolName == symbolName) || (!isSymbol && sample.libName == symbolName)) {
      (this.splitMapData[symbolName] = this.splitMapData[symbolName] || []).push(sample);
      sample.isStore++;
    }
    if (sample.initChildren.length > 0) {
      sample.initChildren.forEach((child) => {
        this.recursionPerfChargeInitTree(child, symbolName, isSymbol);
      });
    }
  }

  recursionPerfPruneInitTree(node: PerfCallChainMerageData, symbolName: string, isSymbol: boolean) {
    if ((isSymbol && node.symbolName == symbolName) || (!isSymbol && node.libName == symbolName)) {
      (this.splitMapData[symbolName] = this.splitMapData[symbolName] || []).push(node);
      node.isStore++;
      this.pruneChildren(node, symbolName);
    } else if (node.initChildren.length > 0) {
      node.initChildren.forEach((child) => {
        this.recursionPerfPruneInitTree(child, symbolName, isSymbol);
      });
    }
  }

  //symbol lib prune
  recursionPruneTree(sample: PerfCallChainMerageData, symbolName: string, isSymbol: boolean) {
    if ((isSymbol && sample.symbolName == symbolName) || (!isSymbol && sample.libName == symbolName)) {
      sample.currentTreeParentNode &&
        sample.currentTreeParentNode.children.splice(sample.currentTreeParentNode.children.indexOf(sample), 1);
    } else {
      sample.children.forEach((child) => {
        this.recursionPruneTree(child, symbolName, isSymbol);
      });
    }
  }

  recursionChargeByRule(
    sample: PerfCallChainMerageData,
    ruleName: string,
    rule: (node: PerfCallChainMerageData) => boolean
  ) {
    if (sample.initChildren.length > 0) {
      sample.initChildren.forEach((child) => {
        if (rule(child)) {
          (this.splitMapData[ruleName] = this.splitMapData[ruleName] || []).push(child);
          child.isStore++;
        }
        this.recursionChargeByRule(child, ruleName, rule);
      });
    }
  }

  pruneChildren(sample: PerfCallChainMerageData, symbolName: string) {
    if (sample.initChildren.length > 0) {
      sample.initChildren.forEach((child) => {
        child.isStore++;
        (this.splitMapData[symbolName] = this.splitMapData[symbolName] || []).push(child);
        this.pruneChildren(child, symbolName);
      });
    }
  }

  hideSystemLibrary() {
    this.allProcess.forEach((item) => {
      item.children = [];
      this.recursionChargeByRule(item, systemRuleName, (node) => {
        return node.path.startsWith(systemRuleName);
      });
    });
  }

  hideNumMaxAndMin(startNum: number, endNum: string) {
    let max = endNum == '∞' ? Number.POSITIVE_INFINITY : parseInt(endNum);
    this.allProcess.forEach((item) => {
      item.children = [];
      this.recursionChargeByRule(item, numRuleName, (node) => {
        return node.dur < startNum || node.dur > max;
      });
    });
  }

  clearSplitMapData(symbolName: string) {
    delete this.splitMapData[symbolName];
  }

  resetAllSymbol(symbols: string[]) {
    symbols.forEach((symbol) => {
      let list = this.splitMapData[symbol];
      if (list != undefined) {
        list.forEach((item: any) => {
          item.isStore--;
        });
      }
    });
  }

  resetAllNode(sample: PerfCallChainMerageData[]) {
    this.clearSearchNode();
    sample.forEach((process) => {
      process.searchShow = true;
      process.isSearch = false;
    });
    this.resetNewAllNode(sample);
    if (this.searchValue != '') {
      this.findSearchNode(sample, this.searchValue, false);
      this.resetNewAllNode(sample);
    }
  }

  resetNewAllNode(sampleArray: PerfCallChainMerageData[]) {
    sampleArray.forEach((process) => {
      process.children = [];
    });
    let values = this.currentTreeList.map((item: any) => {
      item.children = [];
      return item;
    });
    values.forEach((sample: any) => {
      if (sample.parentNode != undefined) {
        if (sample.isStore == 0 && sample.searchShow) {
          let parentNode = sample.parentNode;
          while (parentNode != undefined && !(parentNode.isStore == 0 && parentNode.searchShow)) {
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

  findSearchNode(sampleArray: PerfCallChainMerageData[], search: string, parentSearch: boolean) {
    search = search.toLocaleLowerCase();
    sampleArray.forEach((sample) => {
      if ((sample.symbol && sample.symbol.toLocaleLowerCase().includes(search)) || parentSearch) {
        sample.searchShow = true;
        let parentNode = sample.currentTreeParentNode;
        sample.isSearch = sample.symbol != undefined && sample.symbol.toLocaleLowerCase().includes(search);
        while (parentNode != undefined && !parentNode.searchShow) {
          parentNode.searchShow = true;
          parentNode = parentNode.currentTreeParentNode;
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

  clearSearchNode() {
    this.currentTreeList.forEach((sample) => {
      sample.searchShow = true;
      sample.isSearch = false;
    });
  }

  splitAllProcess(processArray: any[]) {
    processArray.forEach((item: any) => {
      this.allProcess.forEach((process) => {
        if (item.select == '0') {
          this.recursionPerfChargeInitTree(process, item.name, item.type == 'symbol');
        } else {
          this.recursionPerfPruneInitTree(process, item.name, item.type == 'symbol');
        }
      });
      if (!item.checked) {
        this.resetAllSymbol([item.name]);
      }
    });
  }

  resolvingAction(params: any[]) {
    if (params.length > 0) {
      for (let item of params) {
        if (item.funcName && item.funcArgs) {
          switch (item.funcName) {
            case 'getCallChainsBySampleIds':
              this.freshPerfCallchains(this.samplesData, item.funcArgs[0]);
              break;
            case 'getCurrentDataFromDb':
              this.getCurrentDataFromDb(item.funcArgs[0]);
              break;
            case 'hideSystemLibrary':
              this.hideSystemLibrary();
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
      this.dataSource = this.allProcess.filter((process) => {
        return process.children && process.children.length > 0;
      });
    }
    return this.dataSource;
  }

  combineCallChainForAnalysis() {
    let sampleCallChainList = new Array<PerfAnalysisSample>();
    for (let sample of this.samplesData) {
      let callChains = [...this.callChainData[sample.sampleId]];
      const lastCallChain = callChains[callChains.length - 1];
      const threadName = this.threadData[sample.tid].threadName || 'Thread';
      const processName = this.threadData[sample.pid].threadName || 'Process';
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
      sampleCallChainList.push(analysisSample);
    }
    if (this.isAnalysis) {
      this.isAnalysis = false;
    }
    return sampleCallChainList;
  }

  getBottomUp() {
    const topUp = new PerfBottomUpStruct('topUp');
    let perfTime = this.dataCache.perfCountToMs;
    for (let sample of this.samplesData) {
      let currentNode = topUp;
      let callChains = this.callChainData[sample.sampleId];
      for (let i = 0; i < callChains.length; i++) {
        if (i === 0) {
          currentNode = topUp;
        }
        let item = callChains[i];
        const existingNode = currentNode.children.find(
          (child) => child.symbolName === item.name + '(' + item.fileName + ')'
        );
        if (existingNode) {
          currentNode = existingNode;
          existingNode.totalTime += perfTime * sample.count;
          existingNode.calculateSelfTime();
          existingNode.notifyParentUpdateSelfTime();
        } else {
          let newNode = new PerfBottomUpStruct(item.name + '(' + item.fileName + ')');
          newNode.totalTime = perfTime * sample.count;
          currentNode.addChildren(newNode);
          newNode.calculateSelfTime();
          newNode.notifyParentUpdateSelfTime();
          currentNode = newNode;
        }
      }
    }
    topUp.children.forEach((child) => {
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
    const recursionTree = (perfBottomUpStruct: PerfBottomUpStruct) => {
      if (perfBottomUpStruct.selfTime > 0) {
        const clonePerfBottomUpStruct = new PerfBottomUpStruct(perfBottomUpStruct.symbolName);
        clonePerfBottomUpStruct.selfTime = perfBottomUpStruct.selfTime;
        clonePerfBottomUpStruct.totalTime = perfBottomUpStruct.totalTime;
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
    const currentLevelData = new Array<PerfBottomUpStruct>();
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

  static setFileName(data: PerfFile) {
    if (data.path) {
      let number = data.path.lastIndexOf('/');
      if (number > 0) {
        data.fileName = data.path.substring(number + 1);
        return;
      }
    }
    data.fileName = data.path;
  }

  setFileName() {
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

  static setNextNode(currentNode: PerfCallChain, nextNode: PerfCallChain) {
    currentNode.nextNode = nextNode;
    nextNode.previousNode = currentNode;
  }

  static setPreviousNode(currentNode: PerfCallChain, prevNode: PerfCallChain) {
    currentNode.previousNode = prevNode;
    prevNode.nextNode = currentNode;
  }

  static merageCallChain(currentNode: PerfCallChain, callChain: PerfCallChain) {
    currentNode.startNS = callChain.startNS;
    currentNode.tid = callChain.tid;
    currentNode.pid = callChain.pid;
    currentNode.sampleId = callChain.sampleId;
    currentNode.dur = callChain.dur;
    currentNode.count = callChain.count;
  }
}

export class PerfCallChainMerageData extends ChartStruct {
  #parentNode: PerfCallChainMerageData | undefined = undefined;
  #total = 0;
  id: string = '';
  parentId: string = '';
  currentTreeParentNode: PerfCallChainMerageData | undefined = undefined;
  symbolName: string = '';
  symbol: string = '';
  libName: string = '';
  path: string = '';
  self: string = '0s';
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
    this.currentTreeParentNode = data;
    this.#parentNode = data;
  }

  get parentNode() {
    return this.#parentNode;
  }

  set total(data: number) {
    this.#total = data;
    this.weight = `${timeMsFormat2p(this.dur * (DataCache.getInstance().perfCountToMs || 1))}`;
    this.weightPercent = `${((this.dur / data) * 100).toFixed(1)}%`;
  }

  get total() {
    return this.#total;
  }

  static merageCallChain(currentNode: PerfCallChainMerageData, callChain: PerfCallChain, isTopDown: boolean) {
    if (currentNode.symbolName == '') {
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
    if (callChain[isTopDown ? 'nextNode' : 'previousNode'] == undefined) {
      currentNode.selfDur += callChain.count;
      currentNode.self = timeMsFormat2p(currentNode.selfDur * (DataCache.getInstance().perfCountToMs || 1));
    }
    currentNode.dur += callChain.count;
    currentNode.count += callChain.count;
  }

  static merageCallChainSample(
    currentNode: PerfCallChainMerageData,
    callChain: PerfCallChain,
    sample: PerfCountSample,
    isEnd: boolean
  ) {
    if (currentNode.symbolName == '') {
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
      currentNode.self = timeMsFormat2p(currentNode.selfDur * (DataCache.getInstance().perfCountToMs || 1));
    }
    currentNode.dur += sample.count;
    currentNode.count += sample.count;
  }
}

export class PerfCountSample {
  sampleId: number = 0;
  tid: number = 0;
  count: number = 0;
  threadState: string = '';
  pid: number = 0;
  eventCount: number = 0;
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

export function timeMsFormat2p(ns: number) {
  let currentNs = ns;
  let hour1 = 3600_000;
  let minute1 = 60_000;
  let second1 = 1_000; // 1 second
  let perfResult = '';
  if (currentNs >= hour1) {
    perfResult += Math.floor(currentNs / hour1).toFixed(2) + 'h';
    return perfResult;
  }
  if (currentNs >= minute1) {
    perfResult += Math.floor(currentNs / minute1).toFixed(2) + 'min';
    return perfResult;
  }
  if (currentNs >= second1) {
    perfResult += Math.floor(currentNs / second1).toFixed(2) + 's';
    return perfResult;
  }
  if (currentNs > 0) {
    perfResult += currentNs.toFixed(2) + 'ms';
    return perfResult;
  }
  if (perfResult == '') {
    perfResult = '0s';
  }
  return perfResult;
}
