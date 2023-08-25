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
import {
  convertJSON,
  DataCache,
  getByteWithUnit,
  HeapTreeDataBean,
  LogicHandler,
  MerageBean,
  merageBeanDataSplit,
  postMessage,
  setFileName,
} from './ProcedureLogicWorkerCommon.js';

export class ProcedureLogicWorkerNativeMemory extends LogicHandler {
  selectTotalSize = 0;
  selectTotalCount = 0;
  stackCount = 0;
  NATIVE_MEMORY_DATA: Array<NativeEvent> = [];
  currentTreeMapData: any = {};
  currentTreeList: any[] = [];
  queryAllCallchainsSamples: NativeHookStatistics[] = [];
  currentSamples: NativeHookStatistics[] = [];
  allThreads: NativeHookCallInfo[] = [];
  splitMapData: any = {};
  searchValue: string = '';
  currentEventId: string = '';
  chartComplete: Map<number, boolean> = new Map<number, boolean>();
  realTimeDif: number = 0;
  responseTypes: { key: number; value: string }[] = [];
  totalNS: number = 0;
  isAnalysis: boolean = false;
  isStatistic: boolean = false;
  boxRangeNativeHook: Array<NativeMemory> = [];
  clearBoxSelectionData: boolean = false;
  nativeMemoryArgs?: Map<string, any>
  private dataCache = DataCache.getInstance();

  handle(data: any): void {
    this.currentEventId = data.id;
    if (data && data.type) {
      switch (data.type) {
        case 'native-memory-init':
          this.clearAll();
          if (data.params.isRealtime) {
            this.realTimeDif = data.params.realTimeDif;
          }
          this.dataCache.dataDict = data.params.dataDict;
          this.initNMChartData();
          break;
        case 'native-memory-queryNMChartData':
          this.NATIVE_MEMORY_DATA = convertJSON(data.params.list) || [];
          this.initNMFrameData();
          break;
        case 'native-memory-queryNMFrameData':
          let arr = convertJSON(data.params.list) || [];
          this.initNMStack(arr);
          arr = [];
          self.postMessage({
            id: data.id,
            action: 'native-memory-init',
            results: [],
          });
          break;
        case 'native-memory-queryCallchainsSamples':
          this.searchValue = '';
          if (data.params.list) {
            let callchainsSamples = convertJSON(data.params.list) || [];
            this.queryAllCallchainsSamples = callchainsSamples;
            this.freshCurrentCallchains(callchainsSamples, true);
            // @ts-ignore
            self.postMessage({
              id: data.id,
              action: data.action,
              results: this.allThreads,
            });
          } else {
            this.queryCallchainsSamples(
              'native-memory-queryCallchainsSamples',
              data.params.leftNs,
              data.params.rightNs,
              data.params.types
            );
          }
          break;
        case 'native-memory-queryStatisticCallchainsSamples':
          this.searchValue = '';
          if (data.params.list) {
            let samples = convertJSON(data.params.list) || [];
            this.queryAllCallchainsSamples = samples;
            this.freshCurrentCallchains(samples, true);
            // @ts-ignore
            self.postMessage({
              id: data.id,
              action: data.action,
              results: this.allThreads,
            });
          } else {
            this.queryStatisticCallchainsSamples(
              'native-memory-queryStatisticCallchainsSamples',
              data.params.leftNs,
              data.params.rightNs,
              data.params.types
            );
          }
          break;
        case 'native-memory-queryAnalysis':
          if (data.params.list) {
            let samples = convertJSON(data.params.list) || [];
            this.queryAllCallchainsSamples = samples;
            self.postMessage({
              id: data.id,
              action: data.action,
              results: this.combineStatisticAndCallChain(samples),
            });
          } else {
            if (data.params.isStatistic) {
              this.isStatistic = true;
              this.queryStatisticCallchainsSamples(
                'native-memory-queryAnalysis',
                data.params.leftNs,
                data.params.rightNs,
                data.params.types
              );
            } else {
              this.isStatistic = false;
              this.queryCallchainsSamples(
                'native-memory-queryAnalysis',
                data.params.leftNs,
                data.params.rightNs,
                data.params.types
              );
            }
          }
          break;
        case 'native-memory-queryNativeHookEvent':
          if (data.params) {
            if (data.params.list) {
              this.boxRangeNativeHook = convertJSON(data.params.list);
              if (this.nativeMemoryArgs?.get('refresh')) {
                this.clearBoxSelectionData = this.boxRangeNativeHook.length > 100_0000;
              }
              this.supplementNativeHoodData();
              postMessage(data.id, data.action, this.resolvingActionNativeMemory(this.nativeMemoryArgs!), 100_0000);
              if (this.clearBoxSelectionData) {
                this.boxRangeNativeHook = [];
              }
            } else if (data.params.get('refresh') || this.boxRangeNativeHook.length === 0) {
              this.nativeMemoryArgs = data.params;
              let leftNs = data.params.get('leftNs');
              let rightNs = data.params.get('rightNs');
              let types = data.params.get('types');
              this.boxRangeNativeHook = [];
              this.queryNativeHookEvent(leftNs, rightNs, types);
            } else {
              this.nativeMemoryArgs = data.params;
              postMessage(data.id, data.action, this.resolvingActionNativeMemory(this.nativeMemoryArgs!), 100_0000);
              if (this.clearBoxSelectionData) {
                this.boxRangeNativeHook = [];
              }
            }
          }
          break;
        case 'native-memory-action':
          if (data.params) {
            self.postMessage({
              id: data.id,
              action: data.action,
              results: this.resolvingAction(data.params),
            });
          }
          break;
        case 'native-memory-chart-action':
          if (data.params) {
            postMessage(data.id, data.action, this.resolvingActionNativeMemoryChartData(data.params));
          }
          break;
        case 'native-memory-calltree-action':
          if (data.params) {
            self.postMessage({
              id: data.id,
              action: data.action,
              results: this.resolvingNMCallAction(data.params),
            });
          }
          break;
        case 'native-memory-init-responseType':
          this.initResponseTypeList(data.params);
          self.postMessage({
            id: data.id,
            action: data.action,
            results: [],
          });
          break;
        case 'native-memory-get-responseType':
          self.postMessage({
            id: data.id,
            action: data.action,
            results: this.responseTypes,
          });
          break;
        case 'native-memory-queryNativeHookStatistic':
          if (data.params.list) {
            let arr = this.statisticDataHandler(convertJSON(data.params.list));
            postMessage(data.id, data.action, this.handleNativeHookStatisticData(arr));
          } else {
            this.totalNS = data.params.totalNS;
            this.queryNativeHookStatistic(data.params.type);
          }
          break;
      }
    }
  }

  initNMChartData() {
    this.queryData(
      this.currentEventId,
      'native-memory-queryNMChartData',
      `
            select * from (
                select 
                    h.start_ts - t.start_ts as startTime,
                    h.heap_size as heapSize,
                    (case when h.event_type = 'AllocEvent' then 0 else 1 end) as eventType
                from native_hook h ,trace_range t
                where h.start_ts between t.start_ts and t.end_ts
                    and (h.event_type = 'AllocEvent' or h.event_type = 'MmapEvent')
                union all
                select 
                    h.end_ts - t.start_ts as startTime,
                    h.heap_size as heapSize,
                    (case when h.event_type = 'AllocEvent' then 2 else 3 end) as eventType
                from native_hook h ,trace_range t
                where 
                  h.start_ts between t.start_ts and t.end_ts
                  and h.end_ts between t.start_ts and t.end_ts
                  and (h.event_type = 'AllocEvent' or h.event_type = 'MmapEvent')
            )
            order by startTime;
        `,
      {}
    );
  }

  queryNativeHookStatistic(type: number) {
    let condition: string;
    if (type === 0) {
      condition = 'and type = 0';
    } else if (type === 1) {
      condition = 'and type > 0';
    } else {
      condition = '';
    }
    let sql = `
select callchain_id callchainId,
       ts - start_ts as ts,
       apply_count applyCount,
       apply_size applySize,
       release_count releaseCount,
       release_size releaseSize
from native_hook_statistic,trace_range
where ts between start_ts and end_ts ${condition};
        `;
    this.queryData(this.currentEventId, 'native-memory-queryNativeHookStatistic', sql, {});
  }

  queryNativeHookEvent(leftNs: number, rightNs: number, types: Array<string>) {
    let condition = types.length === 1 ? `and A.event_type = ${types[0]}` : `and (A.event_type = 'AllocEvent' or A.event_type = 'MmapEvent')`;
    let libId = this.nativeMemoryArgs?.get('filterResponseType');
    let allocType = this.nativeMemoryArgs?.get('filterAllocType');
    let eventType = this.nativeMemoryArgs?.get('filterEventType');
    if (libId !== undefined && libId !== -1) {
      condition = `${condition} and last_lib_id = ${libId}`;// filter lib
    }
    if (eventType === '1') {
      condition = `${condition} and event_type = 'AllocEvent'`;
    }
    if (eventType === '2') {
      condition = `${condition} and event_type = 'MmapEvent'`;
    }
    if (allocType === '1') {
      condition = `${condition} and ((A.end_ts - B.start_ts) > ${rightNs} or A.end_ts is null)`;
    }
    if (allocType === '2') {
      condition = `${condition} and (A.end_ts - B.start_ts) <= ${rightNs}`;
    }
    let sql = `
    select
      callchain_id as eventId,
      event_type as eventType,
      heap_size as heapSize,
      ('0x' || addr) as addr,
      (A.start_ts - B.start_ts) as startTs,
      (A.end_ts - B.start_ts) as endTs,
      tid as threadId,
      sub_type_id as subTypeId,
      ifnull(last_lib_id,0) as lastLibId
    from
      native_hook A,
      trace_range B
    left join
      thread t
    on
      A.itid = t.id
    where
    A.start_ts - B.start_ts between ${leftNs} and ${rightNs} ${condition}
    `;
    this.queryData(this.currentEventId, 'native-memory-queryNativeHookEvent', sql, {});
  }

  supplementNativeHoodData() {
    let len = this.boxRangeNativeHook.length;
    for(let i = 0, j = len -1; i <= j; i++, j--){
      this.fillNativeHook(this.boxRangeNativeHook[i], i);
      if (i !== j) {
        this.fillNativeHook(this.boxRangeNativeHook[j], j);
      }
    }
  }

  fillNativeHook(memory: NativeMemory, index: number) {
    if (memory.subTypeId !== null && memory.subType === undefined) {
      memory.subType = this.dataCache.dataDict.get(memory.subTypeId) || '-';
    }
    memory.index = index;
    let arr = this.dataCache.nmHeapFrameMap.get(memory.eventId) || [];
    let frame = Array.from(arr)
      .reverse()
      .find((item) => {
        let fileName = this.dataCache.dataDict.get(item.fileId);
        return !((fileName ?? '').includes('libc++') || (fileName ?? '').includes('musl'));
      });
    if (frame === null || frame === undefined) {
      if (arr.length > 0) {
        frame = arr[0];
      }
    }
    if (frame !== null && frame !== undefined) {
      memory.symbol = this.groupCutFilePath(frame.symbolId, this.dataCache.dataDict.get(frame.symbolId) || '');
      memory.library = this.groupCutFilePath(
        frame.fileId,
        this.dataCache.dataDict.get(frame.fileId) || 'Unknown Path'
      );
    } else {
      memory.symbol = '-';
      memory.library = '-';
    }
  }

  statisticDataHandler(arr: Array<any>) {
    let callGroupMap: Map<number, any[]> = new Map<number, any[]>();
    let obj = {};
    for (let hook of arr) {
      if ((obj as any)[hook.ts]) {
        let data = (obj as any)[hook.ts] as any;
        data.startTime = hook.ts;
        data.dur = 0;
        if (callGroupMap.has(hook.callchainId)) {
          let calls = callGroupMap.get(hook.callchainId);
          let last = calls![calls!.length - 1];
          data.heapsize += hook.applySize - last.applySize - (hook.releaseSize - last.releaseSize);
          data.density += hook.applyCount - last.applyCount - (hook.releaseCount - last.releaseCount);
          calls!.push(hook);
        } else {
          data.heapsize += hook.applySize - hook.releaseSize;
          data.density += hook.applyCount - hook.releaseCount;
          callGroupMap.set(hook.callchainId, [hook]);
        }
      } else {
        let data: any = {};
        data.startTime = hook.ts;
        data.dur = 0;
        if (callGroupMap.has(hook.callchainId)) {
          let calls = callGroupMap.get(hook.callchainId);
          let last = calls![calls!.length - 1];
          data.heapsize = hook.applySize - last.applySize - (hook.releaseSize - last.releaseSize);
          data.density = hook.applyCount - last.applyCount - (hook.releaseCount - last.releaseCount);
          calls!.push(hook);
        } else {
          data.heapsize = hook.applySize - hook.releaseSize;
          data.density = hook.applyCount - hook.releaseCount;
          callGroupMap.set(hook.callchainId, [hook]);
        }
        (obj as any)[hook.ts] = data;
      }
    }
    return Object.values(obj) as {
      startTime: number;
      heapsize: number;
      density: number;
      dur: number;
    }[];
  }

  handleNativeHookStatisticData(
    arr: {
      startTime: number;
      heapsize: number;
      density: number;
      dur: number;
    }[]
  ) {
    let maxSize = 0,
      maxDensity = 0,
      minSize = 0,
      minDensity = 0;
    for (let i = 0, len = arr.length; i < len; i++) {
      if (i == len - 1) {
        arr[i].dur = this.totalNS - arr[i].startTime;
      } else {
        arr[i + 1].heapsize = arr[i].heapsize + arr[i + 1].heapsize;
        arr[i + 1].density = arr[i].density + arr[i + 1].density;
        arr[i].dur = arr[i + 1].startTime - arr[i].startTime;
      }
      maxSize = Math.max(maxSize, arr[i].heapsize);
      maxDensity = Math.max(maxDensity, arr[i].density);
      minSize = Math.min(minSize, arr[i].heapsize);
      minDensity = Math.min(minDensity, arr[i].density);
    }
    return arr.map((it) => {
      (it as any).maxHeapSize = maxSize;
      (it as any).maxDensity = maxDensity;
      (it as any).minHeapSize = minSize;
      (it as any).minDensity = minDensity;
      return it;
    });
  }
  initResponseTypeList(list: any[]) {
    this.responseTypes = [
      {
        key: -1,
        value: 'ALL',
      },
    ];
    list.forEach((item) => {
      if (item.lastLibId == null) {
        this.responseTypes.push({
          key: 0,
          value: '-',
        });
      } else {
        this.responseTypes.push({
          key: item.lastLibId,
          value: this.groupCutFilePath(item.lastLibId, item.value) || '-',
        });
      }
    });
  }
  initNMFrameData() {
    this.queryData(
      this.currentEventId,
      'native-memory-queryNMFrameData',
      `
            select h.symbol_id as symbolId, h.file_id as fileId, h.depth, h.callchain_id as eventId, h.vaddr as addr
                    from native_hook_frame h
        `,
      {}
    );
  }
  initNMStack(frameArr: Array<HeapTreeDataBean>) {
    frameArr.map((frame) => {
      let frameEventId = frame.eventId;
      if (this.dataCache.nmHeapFrameMap.has(frameEventId)) {
        this.dataCache.nmHeapFrameMap.get(frameEventId)!.push(frame);
      } else {
        this.dataCache.nmHeapFrameMap.set(frameEventId, [frame]);
      }
    });
  }
  resolvingAction(paramMap: Map<string, any>): Array<NativeHookCallInfo | NativeMemory | HeapStruct> {
    let actionType = paramMap.get('actionType');
    if (actionType === 'memory-stack') {
      return this.resolvingActionNativeMemoryStack(paramMap);
    } else if (actionType === 'native-memory-state-change') {
      let startTs = paramMap.get('startTs');
      let currentSelection = this.boxRangeNativeHook.filter((item) => {
        return item.startTs === startTs;
      });
      if (currentSelection.length > 0) {
        currentSelection[0].isSelected = true;
      }
      return [];
    } else {
      return [];
    }
  }
  resolvingActionNativeMemoryChartData(paramMap: Map<string, any>): Array<HeapStruct> {
    let nativeMemoryType: number = paramMap.get('nativeMemoryType') as number;
    let totalNS: number = paramMap.get('totalNS') as number;
    let arr: Array<HeapStruct> = [];
    let nmMaxSize = 0;
    let nmMaxDensity = 0;
    let nmMinSize = 0;
    let nmMinDensity = 0;
    let nmTempSize = 0;
    let nmTempDensity = 0;
    let nmFilterLen = 0;
    let nmFilterLevel = 0;
    let putArr = (ne: NativeEvent, filterLevel: number, finish: boolean) => {
      let nmHeapStruct = new HeapStruct();
      nmHeapStruct.startTime = ne.startTime;
      if (arr.length == 0) {
        if (ne.eventType == 0 || ne.eventType == 1) {
          nmHeapStruct.density = 1;
          nmHeapStruct.heapsize = ne.heapSize;
        } else {
          nmHeapStruct.density = -1;
          nmHeapStruct.heapsize = 0 - ne.heapSize;
        }
        nmMaxSize = nmHeapStruct.heapsize;
        nmMaxDensity = nmHeapStruct.density;
        nmMinSize = nmHeapStruct.heapsize;
        nmMinDensity = nmHeapStruct.density;
        arr.push(nmHeapStruct);
      } else {
        let last = arr[arr.length - 1];
        last.dur = nmHeapStruct.startTime! - last.startTime!;
        if (last.dur > filterLevel || finish) {
          if (ne.eventType == 0 || ne.eventType == 1) {
            nmHeapStruct.density = last.density! + nmTempDensity + 1;
            nmHeapStruct.heapsize = last.heapsize! + nmTempSize + ne.heapSize;
          } else {
            nmHeapStruct.density = last.density! + nmTempDensity - 1;
            nmHeapStruct.heapsize = last.heapsize! + nmTempSize - ne.heapSize;
          }
          nmTempDensity = 0;
          nmTempSize = 0;
          if (nmHeapStruct.density > nmMaxDensity) {
            nmMaxDensity = nmHeapStruct.density;
          }
          if (nmHeapStruct.density < nmMinDensity) {
            nmMinDensity = nmHeapStruct.density;
          }
          if (nmHeapStruct.heapsize > nmMaxSize) {
            nmMaxSize = nmHeapStruct.heapsize;
          }
          if (nmHeapStruct.heapsize < nmMinSize) {
            nmMinSize = nmHeapStruct.heapsize;
          }
          arr.push(nmHeapStruct);
        } else {
          if (ne.eventType == 0 || ne.eventType == 1) {
            nmTempDensity = nmTempDensity + 1;
            nmTempSize = nmTempSize + ne.heapSize;
          } else {
            nmTempDensity = nmTempDensity - 1;
            nmTempSize = nmTempSize - ne.heapSize;
          }
        }
      }
    };
    if (nativeMemoryType == 1) {
      let temp = this.NATIVE_MEMORY_DATA.filter((ne) => ne.eventType === 0 || ne.eventType === 2);
      nmFilterLen = temp.length;
      nmFilterLevel = this.getFilterLevel(nmFilterLen);
      temp.map((ne, index) => putArr(ne, nmFilterLevel, index === nmFilterLen - 1));
      temp.length = 0;
    } else if (nativeMemoryType == 2) {
      let temp = this.NATIVE_MEMORY_DATA.filter((ne) => ne.eventType === 1 || ne.eventType === 3);
      nmFilterLen = temp.length;
      nmFilterLevel = this.getFilterLevel(nmFilterLen);
      temp.map((ne, index) => putArr(ne, nmFilterLevel, index === nmFilterLen - 1));
      temp.length = 0;
    } else {
      nmFilterLen = this.NATIVE_MEMORY_DATA.length;
      let filterLevel = this.getFilterLevel(nmFilterLen);
      this.NATIVE_MEMORY_DATA.map((ne, index) => putArr(ne, filterLevel, index === nmFilterLen - 1));
    }
    if (arr.length > 0) {
      arr[arr.length - 1].dur = totalNS - arr[arr.length - 1].startTime!;
    }
    arr.map((heapStruct) => {
      heapStruct.maxHeapSize = nmMaxSize;
      heapStruct.maxDensity = nmMaxDensity;
      heapStruct.minHeapSize = nmMinSize;
      heapStruct.minDensity = nmMinDensity;
    });
    this.chartComplete.set(nativeMemoryType, true);
    if (this.chartComplete.has(0) && this.chartComplete.has(1) && this.chartComplete.has(2)) {
      this.NATIVE_MEMORY_DATA = [];
    }
    return arr;
  }
  resolvingActionNativeMemoryStack(paramMap: Map<string, any>) {
    let eventId = paramMap.get('eventId');
    let frameArr = this.dataCache.nmHeapFrameMap.get(eventId) || [];
    let arr: Array<NativeHookCallInfo> = [];
    frameArr.map((frame) => {
      let target = new NativeHookCallInfo();
      target.eventId = frame.eventId;
      target.depth = frame.depth;
      target.addr = frame.addr;
      target.symbol = this.groupCutFilePath(frame.symbolId, this.dataCache.dataDict.get(frame.symbolId) || '') ?? '';
      target.library = this.groupCutFilePath(frame.fileId, this.dataCache.dataDict.get(frame.fileId) || '') ?? '';
      target.title = `[ ${target.symbol} ]  ${target.library}`;
      target.type =
        target.library.endsWith('.so.1') || target.library.endsWith('.dll') || target.library.endsWith('.so') ? 0 : 1;
      arr.push(target);
    });
    return arr;
  }

  resolvingActionNativeMemory(paramMap: Map<string, any>): Array<NativeMemory> {
    let filterAllocType = paramMap.get('filterAllocType');
    let filterEventType = paramMap.get('filterEventType');
    let filterResponseType = paramMap.get('filterResponseType');
    let leftNs = paramMap.get('leftNs');
    let rightNs = paramMap.get('rightNs');
    let sortColumn = paramMap.get('sortColumn');
    let sortType = paramMap.get('sortType');
    let statisticsSelection = paramMap.get('statisticsSelection');
    let filter = this.boxRangeNativeHook;
    if ((filterAllocType !== undefined && filterAllocType !== 0) ||
      (filterEventType !== undefined && filterEventType !== 0) ||
      (filterResponseType !== undefined && filterResponseType !== -1)
    ) {
      filter = this.boxRangeNativeHook.filter((item) => {
        let filterAllocation = true;
        let freed = item.endTs > leftNs &&
          item.endTs <= rightNs &&
          item.endTs !== 0 &&
          item.endTs !== null;
        if (filterAllocType === '1') {
          filterAllocation = !freed;
        } else if (filterAllocType == '2') {
          filterAllocation = freed;
        }
        let filterNative = this.getTypeFromIndex(parseInt(filterEventType), item, statisticsSelection);
        let filterLastLib = filterResponseType == -1 ? true : filterResponseType == item.lastLibId;
        return filterAllocation && filterNative && filterLastLib;
      });
    }
    if (sortColumn !== undefined && sortType !== undefined && sortColumn !== '' && sortType !== 0) {
      return this.sortByNativeMemoryColumn(sortColumn, sortType, filter);
    } else {
      return filter;
    }
  }

  sortByNativeMemoryColumn(nmMemoryColumn: string, nmMemorySort: number, list: Array<NativeMemory>) {
    if (nmMemorySort === 0) {
      return list;
    } else {
      return list.sort((memoryLeftData: any, memoryRightData: any) => {
        if (nmMemoryColumn === 'index' || nmMemoryColumn === 'startTs' || nmMemoryColumn === 'heapSize') {
          return nmMemorySort == 1
            ? memoryLeftData[nmMemoryColumn] - memoryRightData[nmMemoryColumn]
            : memoryRightData[nmMemoryColumn] - memoryLeftData[nmMemoryColumn];
        } else {
          if (nmMemorySort == 1) {
            if (memoryLeftData[nmMemoryColumn] > memoryRightData[nmMemoryColumn]) {
              return 1;
            } else if (memoryLeftData[nmMemoryColumn] === memoryRightData[nmMemoryColumn]) {
              return 0;
            } else {
              return -1;
            }
          } else {
            if (memoryRightData[nmMemoryColumn] > memoryLeftData[nmMemoryColumn]) {
              return 1;
            } else if (memoryLeftData[nmMemoryColumn] == memoryRightData[nmMemoryColumn]) {
              return 0;
            } else {
              return -1;
            }
          }
        }
      });
    }
  }

  groupCutFilePath(fileId: number, path: string): string {
    let name: string;
    if (this.dataCache.nmFileDict.has(fileId)) {
      name = this.dataCache.nmFileDict.get(fileId) ?? '';
    } else {
      let currentPath = path.substring(path.lastIndexOf('/') + 1);
      this.dataCache.nmFileDict.set(fileId, currentPath);
      name = currentPath;
    }
    return name == '' ? '-' : name;
  }

  traverseSampleTree(stack: NativeHookCallInfo, hook: NativeHookStatistics) {
    stack.count += 1;
    stack.countValue = `${stack.count}`;
    stack.countPercent = `${((stack.count / this.selectTotalCount) * 100).toFixed(1)}%`;
    stack.size += hook.heapSize;
    stack.tid = hook.tid;
    stack.threadName = hook.threadName;
    stack.heapSizeStr = `${getByteWithUnit(stack.size)}`;
    stack.heapPercent = `${((stack.size / this.selectTotalSize) * 100).toFixed(1)}%`;
    if (stack.children.length > 0) {
      stack.children.map((child) => {
        this.traverseSampleTree(child as NativeHookCallInfo, hook);
      });
    }
  }
  traverseTree(stack: NativeHookCallInfo, hook: NativeHookStatistics) {
    stack.count = 1;
    stack.countValue = `${stack.count}`;
    stack.countPercent = `${((stack!.count / this.selectTotalCount) * 100).toFixed(1)}%`;
    stack.size = hook.heapSize;
    stack.tid = hook.tid;
    stack.threadName = hook.threadName;
    stack.heapSizeStr = `${getByteWithUnit(stack!.size)}`;
    stack.heapPercent = `${((stack!.size / this.selectTotalSize) * 100).toFixed(1)}%`;
    if (stack.children.length > 0) {
      stack.children.map((child) => {
        this.traverseTree(child as NativeHookCallInfo, hook);
      });
    }
  }
  getTypeFromIndex(
    indexOf: number,
    item: NativeHookStatistics | NativeMemory,
    statisticsSelection: Array<StatisticsSelection>
  ): boolean {
    if (indexOf == -1) {
      return false;
    }
    if (indexOf < 3) {
      if (indexOf == 0) {
        return true;
      } else if (indexOf == 1) {
        return item.eventType == 'AllocEvent';
      } else if (indexOf == 2) {
        return item.eventType == 'MmapEvent';
      }
    } else if (indexOf - 3 < statisticsSelection.length) {
      let selectionElement = statisticsSelection[indexOf - 3];
      if (selectionElement.memoryTap != undefined && selectionElement.max != undefined) {
        if (selectionElement.memoryTap.indexOf('Malloc') != -1) {
          return item.eventType == 'AllocEvent' && item.heapSize == selectionElement.max;
        } else if (selectionElement.memoryTap.indexOf('Mmap') != -1) {
          return item.eventType == 'MmapEvent' && item.heapSize == selectionElement.max && item.subTypeId === null;
        } else {
          return item.subType == selectionElement.memoryTap;
        }
      }
      if (selectionElement.max === undefined && typeof selectionElement.memoryTap === 'number') {
        return item.subTypeId === selectionElement.memoryTap;
      }
    }
    return false;
  }
  clearAll() {
    this.dataCache.clearNM();
    this.splitMapData = {};
    this.currentSamples = [];
    this.allThreads = [];
    this.queryAllCallchainsSamples = [];
    this.NATIVE_MEMORY_DATA = [];
    this.chartComplete.clear();
    this.realTimeDif = 0;
    this.currentTreeMapData = {};
    this.currentTreeList.length = 0;
    this.responseTypes.length = 0;
    this.boxRangeNativeHook = [];
    this.nativeMemoryArgs?.clear();
  }

  queryCallchainsSamples(action: string, leftNs: number, rightNs: number, types: Array<string>) {
    this.queryData(
      this.currentEventId,
      action,
      `select A.id,
                callchain_id as eventId,
                event_type as eventType,
                heap_size as heapSize,
                (A.start_ts - B.start_ts) as startTs,
                (A.end_ts - B.start_ts) as endTs,
                tid,
                ifnull(last_lib_id,0) as lastLibId,
                t.name as threadName,
                A.addr,
                A.sub_type_id as subTypeId
            from
                native_hook A,
                trace_range B
                left join
                thread t
                on
                A.itid = t.id
            where
                A.start_ts - B.start_ts
                between ${leftNs} and ${rightNs} and A.event_type in (${types.join(',')})
        `,
      {}
    );
  }
  queryStatisticCallchainsSamples(action: string, leftNs: number, rightNs: number, types: Array<number>) {
    let condition = '';
    if (types.length === 1) {
      if (types[0] === 0) {
        condition = 'and type = 0';
      } else {
        condition = 'and type != 0';
      }
    }
    this.queryData(
      this.currentEventId,
      action,
      `select A.id,
                0 as tid,
                callchain_id as eventId,
                (case when type = 0 then 'AllocEvent' else 'MmapEvent' end) as eventType,
                (case when sub_type_id not null then sub_type_id else type end) as subTypeId,
                apply_size as heapSize,
                release_size as freeSize,
                apply_count as count,
                release_count as freeCount,
                (max(A.ts) - B.start_ts) as startTs
            from
                native_hook_statistic A,
                trace_range B
            where
                A.ts - B.start_ts
                between ${leftNs} and ${rightNs}
                ${condition}
            group by callchain_id;
        `,
      {}
    );
  }

  combineStatisticAndCallChain(samples: NativeHookStatistics[]) {
    samples.sort((a, b) => a.id - b.id);
    const analysisSampleList = new Array<AnalysisSample>();
    const applyAllocSamples = new Array<AnalysisSample>();
    const applyMmapSamples = new Array<AnalysisSample>();

    for (const sample of samples) {
      const count = this.isStatistic ? sample.count : 1;
      const analysisSample = new AnalysisSample(sample.id, sample.heapSize, count, sample.eventType, sample.startTs);

      if (this.isStatistic) {
        analysisSample.releaseCount = sample.freeCount;
        analysisSample.releaseSize = sample.freeSize;
        switch (sample.subTypeId) {
          case 1:
            analysisSample.subType = 'MmapEvent';
            break;
          case 2:
            analysisSample.subType = 'FILE_PAGE_MSG';
            break;
          case 3:
            analysisSample.subType = 'MEMORY_USING_MSG';
            break;
          default:
            analysisSample.subType = this.dataCache.dataDict.get(sample.subTypeId);
        }
      } else {
        let subType = undefined;
        if (sample.subTypeId) {
          subType = this.dataCache.dataDict.get(sample.subTypeId);
        }
        analysisSample.endTs = sample.endTs;
        analysisSample.addr = sample.addr;
        analysisSample.tid = sample.tid;
        analysisSample.subType = subType;
      }

      if (['FreeEvent', 'MunmapEvent'].includes(sample.eventType)) {
        if (sample.eventType === 'FreeEvent') {
          this.setApplyIsRelease(analysisSample, applyAllocSamples);
        } else {
          this.setApplyIsRelease(analysisSample, applyMmapSamples);
        }
        continue;
      } else {
        if (sample.eventType === 'AllocEvent') {
          applyAllocSamples.push(analysisSample);
        } else {
          applyMmapSamples.push(analysisSample);
        }
      }

      const callChains = this.dataCache.nmHeapFrameMap.get(sample.eventId) || [];
      if (!callChains || callChains.length === 0) {
        return;
      }
      let index = callChains.length - 1;
      let lastFilterCallChain: HeapTreeDataBean | undefined | null;
      while (true) {
        // if all call stack is musl or libc++. use stack top lib
        if (index < 0) {
          lastFilterCallChain = callChains[callChains.length - 1];
          break;
        }

        lastFilterCallChain = callChains[index];
        const libPath = this.dataCache.dataDict.get(lastFilterCallChain.fileId);
        //ignore musl and libc++ so
        if (libPath?.includes('musl') || libPath?.includes('libc++')) {
          index--;
        } else {
          lastFilterCallChain = lastFilterCallChain;
          break;
        }
      }

      const filePath = this.dataCache.dataDict.get(lastFilterCallChain.fileId)!;
      let libName = '';
      if (filePath) {
        const path = filePath.split('/');
        libName = path[path.length - 1];
      }
      const symbolName =
        this.dataCache.dataDict.get(lastFilterCallChain.symbolId) || libName + ' (' + sample.addr + ')';

      analysisSample.libId = lastFilterCallChain.fileId;
      analysisSample.libName = libName;
      analysisSample.symbolId = lastFilterCallChain.symbolId;
      analysisSample.symbolName = symbolName;

      analysisSampleList.push(analysisSample);
    }
    return analysisSampleList;
  }

  setApplyIsRelease(sample: AnalysisSample, arr: Array<AnalysisSample>) {
    let idx = arr.length - 1;
    for (idx; idx >= 0; idx--) {
      let item = arr[idx];
      if (item.endTs === sample.startTs && item.addr === sample.addr) {
        arr.splice(idx, 1);
        item.isRelease = true;
        return;
      }
    }
  }

  freshCurrentCallchains(samples: NativeHookStatistics[], isTopDown: boolean) {
    this.currentTreeMapData = {};
    this.currentTreeList = [];
    let totalSize = 0;
    let totalCount = 0;
    samples.forEach((nativeHookSample) => {
      if (nativeHookSample.eventId == -1) {
        return;
      }
      totalSize += nativeHookSample.heapSize;
      totalCount += nativeHookSample.count || 1;
      let callChains = this.createThreadSample(nativeHookSample);
      let topIndex = isTopDown ? 0 : callChains.length - 1;
      if (callChains.length > 0) {
        let root =
          this.currentTreeMapData[
            nativeHookSample.tid + '-' + (callChains[topIndex].symbolId || '') + '-' + (callChains[topIndex].fileId || '')
          ];
        if (root == undefined) {
          root = new NativeHookCallInfo();
          root.threadName = nativeHookSample.threadName;
          this.currentTreeMapData[
            nativeHookSample.tid + '-' + (callChains[topIndex].symbolId || '') + '-' + (callChains[topIndex].fileId || '')
          ] = root;
          this.currentTreeList.push(root);
        }
        NativeHookCallInfo.merageCallChainSample(root, callChains[topIndex], nativeHookSample);
        if (callChains.length > 1) {
          this.merageChildrenByIndex(root, callChains, topIndex, nativeHookSample, isTopDown);
        }
      }
    });
    let rootMerageMap: any = {};
    // @ts-ignore
    let threads = Object.values(this.currentTreeMapData);
    threads.forEach((merageData: any) => {
      if (rootMerageMap[merageData.tid] == undefined) {
        let threadMerageData = new NativeHookCallInfo(); //新增进程的节点数据
        threadMerageData.canCharge = false;
        threadMerageData.type = -1;
        threadMerageData.symbolName = `${merageData.threadName || 'Thread'} [${merageData.tid}]`;
        threadMerageData.symbol = threadMerageData.symbolName;
        threadMerageData.children.push(merageData);
        threadMerageData.initChildren.push(merageData);
        threadMerageData.count = merageData.count || 1;
        threadMerageData.heapSize = merageData.heapSize;
        threadMerageData.totalCount = totalCount;
        threadMerageData.totalSize = totalSize;
        rootMerageMap[merageData.tid] = threadMerageData;
      } else {
        rootMerageMap[merageData.tid].children.push(merageData);
        rootMerageMap[merageData.tid].initChildren.push(merageData);
        rootMerageMap[merageData.tid].count += merageData.count || 1;
        rootMerageMap[merageData.tid].heapSize += merageData.heapSize;
        rootMerageMap[merageData.tid].totalCount = totalCount;
        rootMerageMap[merageData.tid].totalSize = totalSize;
      }
      merageData.parentNode = rootMerageMap[merageData.tid]; //子节点添加父节点的引用
    });
    let id = 0;
    this.currentTreeList.forEach((nmTreeNode) => {
      nmTreeNode.totalCount = totalCount;
      nmTreeNode.totalSize = totalSize;
      this.setMerageName(nmTreeNode);
      if (nmTreeNode.id == '') {
        nmTreeNode.id = id + '';
        id++;
      }
      if (nmTreeNode.parentNode) {
        if (nmTreeNode.parentNode.id == '') {
          nmTreeNode.parentNode.id = id + '';
          id++;
        }
        nmTreeNode.parentId = nmTreeNode.parentNode.id;
      }
    });
    // @ts-ignore
    this.allThreads = Object.values(rootMerageMap) as NativeHookCallInfo[];
  }
  groupCallchainSample(paramMap: Map<string, any>) {
    let groupMap: any = {};
    let filterAllocType = paramMap.get('filterAllocType');
    let filterEventType = paramMap.get('filterEventType');
    let filterResponseType = paramMap.get('filterResponseType');
    let leftNs = paramMap.get('leftNs');
    let rightNs = paramMap.get('rightNs');
    let nativeHookType = paramMap.get('nativeHookType');
    let statisticsSelection = paramMap.get('statisticsSelection');
    if (filterAllocType == '0' && filterEventType == '0' && filterResponseType == -1) {
      this.currentSamples = this.queryAllCallchainsSamples;
      return;
    }
    let filter = this.queryAllCallchainsSamples.filter((item) => {
      let filterAllocation = true;
      if (nativeHookType === 'native-hook') {
        if (filterAllocType == '1') {
          filterAllocation =
            item.startTs >= leftNs &&
            item.startTs <= rightNs &&
            (item.endTs > rightNs || item.endTs == 0 || item.endTs == null);
        } else if (filterAllocType == '2') {
          filterAllocation =
            item.startTs >= leftNs &&
            item.startTs <= rightNs &&
            item.endTs <= rightNs &&
            item.endTs != 0 &&
            item.endTs != null;
        }
      } else {
        if (filterAllocType == '1') {
          filterAllocation = item.heapSize > item.freeSize;
        } else if (filterAllocType == '2') {
          filterAllocation = item.heapSize === item.freeSize;
        }
      }
      let filterLastLib = filterResponseType == -1 ? true : filterResponseType == item.lastLibId;
      let filterNative = this.getTypeFromIndex(parseInt(filterEventType), item, statisticsSelection);
      return filterAllocation && filterNative && filterLastLib;
    });
    filter.forEach((sample) => {
      let currentNode = groupMap[sample.tid + '-' + sample.eventId] || new NativeHookStatistics();
      if (currentNode.count == 0) {
        Object.assign(currentNode, sample);
        if (filterAllocType == '1' && nativeHookType !== 'native-hook') {
          currentNode.heapSize = sample.heapSize - sample.freeSize;
          currentNode.count = sample.count - sample.freeCount;
        }
        if (currentNode.count === 0) {
          currentNode.count++;
        }
      } else {
        currentNode.count++;
        currentNode.heapSize += sample.heapSize;
      }
      groupMap[sample.tid + '-' + sample.eventId] = currentNode;
    });
    // @ts-ignore
    this.currentSamples = Object.values(groupMap);
  }
  createThreadSample(sample: NativeHookStatistics) {
    return this.dataCache.nmHeapFrameMap.get(sample.eventId) || [];
  }
  merageChildrenByIndex(
    currentNode: NativeHookCallInfo,
    callChainDataList: any[],
    index: number,
    sample: NativeHookStatistics,
    isTopDown: boolean
  ) {
    isTopDown ? index++ : index--;
    let isEnd = isTopDown ? callChainDataList.length == index + 1 : index == 0;
    let node;
    if (
      currentNode.initChildren.filter((child: any) => {
        if (child.symbolId == callChainDataList[index]?.symbolId && child.fileId == callChainDataList[index]?.fileId) {
          node = child;
          NativeHookCallInfo.merageCallChainSample(child, callChainDataList[index], sample);
          return true;
        }
        return false;
      }).length == 0
    ) {
      node = new NativeHookCallInfo();
      NativeHookCallInfo.merageCallChainSample(node, callChainDataList[index], sample);
      currentNode.children.push(node);
      currentNode.initChildren.push(node);
      this.currentTreeList.push(node);
      node.parentNode = currentNode;
    }
    if (node && !isEnd) this.merageChildrenByIndex(node, callChainDataList, index, sample, isTopDown);
  }
  setMerageName(currentNode: NativeHookCallInfo) {
    currentNode.symbol =
      this.groupCutFilePath(currentNode.symbolId, this.dataCache.dataDict.get(currentNode.symbolId) || '') ?? 'unknown';
    currentNode.path = this.dataCache.dataDict.get(currentNode.fileId) || 'unknown';
    currentNode.libName = setFileName(currentNode.path);
    currentNode.lib = currentNode.path;
    currentNode.symbolName = `[${currentNode.symbol}] ${currentNode.libName}`;
    currentNode.type =
      currentNode.libName.endsWith('.so.1') ||
      currentNode.libName.endsWith('.dll') ||
      currentNode.libName.endsWith('.so')
        ? 0
        : 1;
  }
  clearSplitMapData(symbolName: string) {
    delete this.splitMapData[symbolName];
  }
  resolvingNMCallAction(params: any[]) {
    if (params.length > 0) {
      params.forEach((item) => {
        if (item.funcName && item.funcArgs) {
          switch (item.funcName) {
            case 'groupCallchainSample':
              this.groupCallchainSample(item.funcArgs[0] as Map<string, any>);
              break;
            case 'getCallChainsBySampleIds':
              this.freshCurrentCallchains(this.currentSamples, item.funcArgs[0]);
              break;
            case 'hideSystemLibrary':
              merageBeanDataSplit.hideSystemLibrary(this.allThreads, this.splitMapData);
              break;
            case 'hideNumMaxAndMin':
              merageBeanDataSplit.hideNumMaxAndMin(
                this.allThreads,
                this.splitMapData,
                item.funcArgs[0],
                item.funcArgs[1]
              );
              break;
            case 'splitAllProcess':
              merageBeanDataSplit.splitAllProcess(this.allThreads, this.splitMapData, item.funcArgs[0]);
              break;
            case 'resetAllNode':
              merageBeanDataSplit.resetAllNode(this.allThreads, this.currentTreeList, this.searchValue);
              break;
            case 'resotreAllNode':
              merageBeanDataSplit.resotreAllNode(this.splitMapData, item.funcArgs[0]);
              break;
            case 'splitTree':
              merageBeanDataSplit.splitTree(
                this.splitMapData,
                this.allThreads,
                item.funcArgs[0],
                item.funcArgs[1],
                item.funcArgs[2],
                this.currentTreeList,
                this.searchValue
              );
              break;
            case 'setSearchValue':
              this.searchValue = item.funcArgs[0];
              break;
            case 'clearSplitMapData':
              this.clearSplitMapData(item.funcArgs[0]);
              break;
          }
        }
      });
    }
    return this.allThreads.filter((thread) => {
      return thread.children && thread.children.length > 0;
    });
  }
  getFilterLevel(len: number): number {
    if (len > 300_0000) {
      return 50_0000;
    } else if (len > 200_0000) {
      return 30_0000;
    } else if (len > 100_0000) {
      return 10_0000;
    } else if (len > 50_0000) {
      return 5_0000;
    } else if (len > 30_0000) {
      return 2_0000;
    } else if (len > 15_0000) {
      return 1_0000;
    } else {
      return 0;
    }
  }
}

export class NativeHookStatistics {
  id: number = 0;
  eventId: number = 0;
  eventType: string = '';
  subType: string = '';
  subTypeId: number = 0;
  heapSize: number = 0;
  freeSize: number = 0;
  addr: string = '';
  startTs: number = 0;
  endTs: number = 0;
  sumHeapSize: number = 0;
  max: number = 0;
  count: number = 0;
  freeCount: number = 0;
  tid: number = 0;
  threadName: string = '';
  lastLibId: number = 0;
  isSelected: boolean = false;
}
export class NativeHookCallInfo extends MerageBean {
  #totalCount: number = 0;
  #totalSize: number = 0;
  library: string = '';
  symbolId: number = 0;
  fileId: number = 0;
  title: string = '';
  count: number = 0;
  countValue: string = '';
  countPercent: string = '';
  type: number = 0;
  heapSize: number = 0;
  heapPercent: string = '';
  heapSizeStr: string = '';
  eventId: number = 0;
  tid: number = 0;
  threadName: string = '';
  eventType: string = '';
  isSelected: boolean = false;
  set totalCount(total: number) {
    this.#totalCount = total;
    this.countValue = this.count + '';
    this.size = this.heapSize;
    this.countPercent = `${((this.count / total) * 100).toFixed(1)}%`;
  }
  get totalCount() {
    return this.#totalCount;
  }
  set totalSize(total: number) {
    this.#totalSize = total;
    this.heapSizeStr = `${getByteWithUnit(this.heapSize)}`;
    this.heapPercent = `${((this.heapSize / total) * 100).toFixed(1)}%`;
  }
  get totalSize() {
    return this.#totalSize;
  }
  static merageCallChainSample(
    currentNode: NativeHookCallInfo,
    callChain: HeapTreeDataBean,
    sample: NativeHookStatistics
  ) {
    if (currentNode.symbol == undefined || currentNode.symbol == '') {
      currentNode.symbol = callChain.AllocationFunction || '';
      currentNode.addr = callChain.addr;
      currentNode.eventId = sample.eventId;
      currentNode.eventType = sample.eventType;
      currentNode.symbolId = callChain.symbolId;
      currentNode.fileId = callChain.fileId;
      currentNode.tid = sample.tid;
    }
    currentNode.count += sample.count || 1;
    currentNode.heapSize += sample.heapSize;
  }
}
export class NativeMemory {
  index: number = 0;
  eventId: number = 0;
  eventType: string = '';
  subType: string = '';
  subTypeId: number = 0;
  addr: string = '';
  startTs: number = 0;
  endTs: number = 0;
  heapSize: number = 0;
  symbol: string = '';
  library: string = '';
  lastLibId: number = 0;
  isSelected: boolean = false;
  threadId: number = 0;
}

export class HeapStruct {
  startTime: number | undefined;
  endTime: number | undefined;
  dur: number | undefined;
  density: number | undefined;
  heapsize: number | undefined;
  maxHeapSize: number = 0;
  maxDensity: number = 0;
  minHeapSize: number = 0;
  minDensity: number = 0;
}
export class NativeEvent {
  startTime: number = 0;
  heapSize: number = 0;
  eventType: number = 0;
}
export class StatisticsSelection {
  memoryTap: string = '';
  max: number = 0;
}

class AnalysisSample {
  id: number;
  count: number;
  size: number;
  type: number;
  startTs: number;

  isRelease: boolean;
  releaseCount?: number;
  releaseSize?: number;

  endTs?: number;
  subType?: string;
  tid?: number;
  addr?: string;

  libId!: number;
  libName!: string;
  symbolId!: number;
  symbolName!: string;

  constructor(id: number, size: number, count: number, type: number | string, startTs: number) {
    this.id = id;
    this.size = size;
    this.count = count;
    this.startTs = startTs;
    switch (type) {
      case 'AllocEvent':
      case '0':
        this.type = 0;
        this.isRelease = false;
        break;
      case 'MmapEvent':
      case '1':
        this.isRelease = false;
        this.type = 1;
        break;
      case 'FreeEvent':
        this.isRelease = true;
        this.type = 2;
        break;
      case 'MunmapEvent':
        this.isRelease = true;
        this.type = 3;
        break;
      default:
        this.isRelease = false;
        this.type = -1;
    }
  }
}
