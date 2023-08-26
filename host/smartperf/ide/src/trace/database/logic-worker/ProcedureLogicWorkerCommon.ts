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

export class ChartStruct {
  depth: number = 0;
  symbol: string = '';
  lib: string = '';
  addr: string = '';
  size: number = 0;
  count: number = 0;
  dur: number = 0;
  parent: ChartStruct | undefined;
  children: Array<ChartStruct> = [];
  isSearch: boolean = false;
}

export class Msg {
  tag: string = '';
  index: number = 0;
  isSending: boolean = false;
  data: Array<any> = [];
}

export class MerageBean extends ChartStruct {
  #parentNode: MerageBean | undefined = undefined;
  #total = 0;
  currentTreeParentNode: MerageBean | undefined = undefined;
  id: string = '';
  parentId: string = '';
  symbolName: string = '';
  symbol: string = '';
  libName: string = '';
  path: string = '';
  self: string = '0s';
  weight: string = '';
  weightPercent: string = '';
  selfDur: number = 0;
  dur: number = 0;
  pid: number = 0;
  canCharge: boolean = true;
  isStore = 0;
  isSelected: boolean = false;
  searchShow: boolean = true;
  children: MerageBean[] = [];
  initChildren: MerageBean[] = [];
  type: number = 0;
  set parentNode(data: MerageBean | undefined) {
    this.currentTreeParentNode = data;
    this.#parentNode = data;
  }

  get parentNode(): MerageBean | undefined {
    return this.#parentNode;
  }

  set total(data: number) {
    this.#total = data;
    this.weight = `${ getProbablyTime(this.dur) }`;
    this.weightPercent = `${ ((this.dur / data) * 100).toFixed(1) }%`;
  }

  get total(): number {
    return this.#total;
  }
}

class MerageBeanDataSplit {
  systmeRuleName = '/system/';
  numRuleName = '/max/min/';

  //所有的操作都是针对整个树结构的, 不区分特定的数据
  splitTree(
    splitMapData: any,
    data: MerageBean[],
    name: string,
    isCharge: boolean,
    isSymbol: boolean,
    currentTreeList: any[],
    searchValue: string
  ): void {
    data.forEach((process) => {
      process.children = [];
      if (isCharge) {
        this.recursionChargeInitTree(splitMapData, process, name, isSymbol);
      } else {
        this.recursionPruneInitTree(splitMapData, process, name, isSymbol);
      }
    });
    this.resetAllNode(data, currentTreeList, searchValue);
  }

  recursionChargeInitTree(splitMapData: any, node: MerageBean, symbolName: string, isSymbol: boolean): void {
    if ((isSymbol && node.symbolName == symbolName) || (!isSymbol && node.libName == symbolName)) {
      (splitMapData[symbolName] = splitMapData[symbolName] || []).push(node);
      node.isStore++;
    }
    if (node.initChildren.length > 0) {
      node.initChildren.forEach((child) => {
        this.recursionChargeInitTree(splitMapData, child, symbolName, isSymbol);
      });
    }
  }

  recursionPruneInitTree(splitMapData: any, node: MerageBean, symbolName: string, isSymbol: boolean): void {
    if ((isSymbol && node.symbolName == symbolName) || (!isSymbol && node.libName == symbolName)) {
      (splitMapData[symbolName] = splitMapData[symbolName] || []).push(node);
      node.isStore++;
      this.pruneChildren(splitMapData, node, symbolName);
    } else if (node.initChildren.length > 0) {
      node.initChildren.forEach((child) => {
        this.recursionPruneInitTree(splitMapData, child, symbolName, isSymbol);
      });
    }
  }

  //symbol lib prune
  recursionPruneTree(node: MerageBean, symbolName: string, isSymbol: boolean): void {
    if ((isSymbol && node.symbolName == symbolName) || (!isSymbol && node.libName == symbolName)) {
      node.currentTreeParentNode &&
      node.currentTreeParentNode.children.splice(node.currentTreeParentNode.children.indexOf(node), 1);
    } else {
      node.children.forEach((child) => {
        this.recursionPruneTree(child, symbolName, isSymbol);
      });
    }
  }

  recursionChargeByRule(splitMapData: any, node: MerageBean, ruleName: string, rule: (node: MerageBean) => boolean): void {
    if (node.initChildren.length > 0) {
      node.initChildren.forEach((child) => {
        if (rule(child)) {
          (splitMapData[ruleName] = splitMapData[ruleName] || []).push(child);
          child.isStore++;
        }
        this.recursionChargeByRule(splitMapData, child, ruleName, rule);
      });
    }
  }

  pruneChildren(splitMapData: any, node: MerageBean, symbolName: string): void {
    if (node.initChildren.length > 0) {
      node.initChildren.forEach((child) => {
        child.isStore++;
        (splitMapData[symbolName] = splitMapData[symbolName] || []).push(child);
        this.pruneChildren(splitMapData, child, symbolName);
      });
    }
  }

  hideSystemLibrary(allProcess: MerageBean[], splitMapData: any): void {
    allProcess.forEach((item) => {
      item.children = [];
      this.recursionChargeByRule(splitMapData, item, this.systmeRuleName, (node) => {
        return node.path.startsWith(this.systmeRuleName);
      });
    });
  }

  hideNumMaxAndMin(allProcess: MerageBean[], splitMapData: any, startNum: number, endNum: string): void {
    let max = endNum == '∞' ? Number.POSITIVE_INFINITY : parseInt(endNum);
    allProcess.forEach((item) => {
      item.children = [];
      this.recursionChargeByRule(splitMapData, item, this.numRuleName, (node) => {
        return node.count < startNum || node.count > max;
      });
    });
  }

  resotreAllNode(splitMapData: any, symbols: string[]): void {
    symbols.forEach((symbol) => {
      let list = splitMapData[symbol];
      if (list != undefined) {
        list.forEach((item: any) => {
          item.isStore--;
        });
      }
    });
  }

  resetAllNode(data: MerageBean[], currentTreeList: any[], searchValue: string): void {
    this.clearSearchNode(currentTreeList);
    data.forEach((process) => {
      process.searchShow = true;
      process.isSearch = false;
    });
    this.resetNewAllNode(data, currentTreeList);
    if (searchValue != '') {
      this.findSearchNode(data, searchValue, false);
      this.resetNewAllNode(data, currentTreeList);
    }
  }

  resetNewAllNode(data: MerageBean[], currentTreeList: any[]): void {
    data.forEach((process) => {
      process.children = [];
    });
    let values = currentTreeList.map((item: any) => {
      item.children = [];
      return item;
    });
    values.forEach((item: any) => {
      if (item.parentNode != undefined) {
        if (item.isStore == 0 && item.searchShow) {
          let parentNode = item.parentNode;
          while (parentNode != undefined && !(parentNode.isStore == 0 && parentNode.searchShow)) {
            parentNode = parentNode.parentNode;
          }
          if (parentNode) {
            item.currentTreeParentNode = parentNode;
            parentNode.children.push(item);
          }
        }
      }
    });
  }

  findSearchNode(data: MerageBean[], search: string, parentSearch: boolean): void {
    search = search.toLocaleLowerCase();
    data.forEach((node) => {
      if ((node.symbolName && node.symbolName.toLocaleLowerCase().includes(search)) || parentSearch) {
        node.searchShow = true;
        node.isSearch = node.symbolName != undefined && node.symbolName.toLocaleLowerCase().includes(search);
        let parentNode = node.currentTreeParentNode;
        while (parentNode && !parentNode.searchShow) {
          parentNode.searchShow = true;
          parentNode = parentNode.currentTreeParentNode;
        }
      } else {
        node.searchShow = false;
        node.isSearch = false;
      }
      if (node.children.length > 0) {
        this.findSearchNode(node.children, search, node.searchShow);
      }
    });
  }

  clearSearchNode(currentTreeList: any[]): void {
    currentTreeList.forEach((node) => {
      node.searchShow = true;
      node.isSearch = false;
    });
  }

  splitAllProcess(allProcess: any[], splitMapData: any, list: any[]): void {
    list.forEach((item: any) => {
      allProcess.forEach((process) => {
        if (item.select == '0') {
          this.recursionChargeInitTree(splitMapData, process, item.name, item.type == 'symbol');
        } else {
          this.recursionPruneInitTree(splitMapData, process, item.name, item.type == 'symbol');
        }
      });
      if (!item.checked) {
        this.resotreAllNode(splitMapData, [item.name]);
      }
    });
  }
}

export let merageBeanDataSplit = new MerageBeanDataSplit();

export abstract class LogicHandler {
  abstract handle(data: any): void;
  queryData(eventId: string, queryName: string, sql: string, args: any): void {
    self.postMessage({
      id: eventId,
      type: queryName,
      isQuery: true,
      args: args,
      sql: sql,
    });
  }

  abstract clearAll(): void;
}

let dec = new TextDecoder();

export let setFileName = (path: string): string => {
  let fileName = '';
  if (path) {
    let number = path.lastIndexOf('/');
    if (number > 0) {
      fileName = path.substring(number + 1);
      return fileName;
    }
  }
  return path;
};

let pagination = (page: number, pageSize: number, source: Array<any>): any[] => {
  let offset = (page - 1) * pageSize;
  return offset + pageSize >= source.length
    ? source.slice(offset, source.length)
    : source.slice(offset, offset + pageSize);
};

const PAGE_SIZE: number = 50_0000;
export let postMessage = (id: any, action: string, results: Array<any>, pageSize: number = PAGE_SIZE): void => {
  if (results.length > pageSize) {
    let pageCount = Math.ceil(results.length / pageSize);
    for (let i = 1 ; i <= pageCount ; i++) {
      let tag = 'start';
      if (i == 1) {
        tag = 'start';
      } else if (i == pageCount) {
        tag = 'end';
      } else {
        tag = 'sending';
      }
      let msg = new Msg();
      msg.tag = tag;
      msg.index = i;
      msg.isSending = tag != 'end';
      msg.data = pagination(i, PAGE_SIZE, results);
      self.postMessage({
        id: id,
        action: action,
        isSending: msg.tag != 'end',
        results: msg,
      });
    }
    results.length = 0;
  } else {
    let msg = new Msg();
    msg.tag = 'end';
    msg.index = 0;
    msg.isSending = false;
    msg.data = results;
    self.postMessage({ id: id, action: action, results: msg });
    results.length = 0;
  }
};
export let translateJsonString = (str: string): string => {
  return str //   .padding
    .replace(/[\t|\r|\n]/g, '')
    .replace(/\\/g, '\\\\');
};

export let convertJSON = (arrBuf: ArrayBuffer | Array<any>): any[] => {
  if (arrBuf instanceof ArrayBuffer) {
    let string = dec.decode(arrBuf);
    let jsonArray = [];
    string = string.substring(string.indexOf('\n') + 1);
    if (!string) {
    } else {
      let parse;
      let tansStr = translateJsonString(string);
      try {
        parse = JSON.parse(translateJsonString(string));
      } catch {
        tansStr = tansStr.replace(/[^\x20-\x7E]/g, '?'); //匹配乱码字符，将其转换为？
        parse = JSON.parse(tansStr);
      }
      let columns = parse.columns;
      let values = parse.values;
      for (let i = 0 ; i < values.length ; i++) {
        let object: any = {};
        for (let j = 0 ; j < columns.length ; j++) {
          object[columns[j]] = values[i][j];
        }
        jsonArray.push(object);
      }
    }
    return jsonArray;
  } else {
    return arrBuf;
  }
};

export let getByteWithUnit = (bytes: number): string => {
  if (bytes < 0) {
    return '-' + getByteWithUnit(Math.abs(bytes));
  }
  let currentBytes = bytes;
  let kb1 = 1 << 10;
  let mb = (1 << 10) << 10;
  let gb = ((1 << 10) << 10) << 10; // 1 gb
  let res = '';
  if (currentBytes > gb) {
    res += (currentBytes / gb).toFixed(2) + ' Gb';
  } else if (currentBytes > mb) {
    res += (currentBytes / mb).toFixed(2) + ' Mb';
  } else if (currentBytes > kb1) {
    res += (currentBytes / kb1).toFixed(2) + ' Kb';
  } else {
    res += Math.round(currentBytes) + ' byte';
  }
  return res;
};

export let getTimeString = (ns: number): string => {
  let currentNs = ns;
  let hour1 = 3600_000_000_000;
  let minute1 = 60_000_000_000;
  let second1 = 1_000_000_000;
  let millisecond1 = 1_000_000;
  let microsecond1 = 1_000;
  let res = '';
  if (currentNs >= hour1) {
    res += Math.floor(currentNs / hour1) + 'h ';
    currentNs = currentNs - Math.floor(currentNs / hour1) * hour1;
  }
  if (currentNs >= minute1) {
    res += Math.floor(currentNs / minute1) + 'm ';
    currentNs = currentNs - Math.floor(ns / minute1) * minute1;
  }
  if (currentNs >= second1) {
    res += Math.floor(currentNs / second1) + 's ';
    currentNs = currentNs - Math.floor(currentNs / second1) * second1;
  }
  if (currentNs >= millisecond1) {
    res += Math.floor(currentNs / millisecond1) + 'ms ';
    currentNs = currentNs - Math.floor(currentNs / millisecond1) * millisecond1;
  }
  if (currentNs >= microsecond1) {
    res += Math.floor(currentNs / microsecond1) + 'μs ';
    currentNs = currentNs - Math.floor(currentNs / microsecond1) * microsecond1;
  }
  if (currentNs > 0) {
    res += currentNs + 'ns ';
  }
  if (res == '') {
    res = ns + '';
  }
  return res;
};

export function getProbablyTime(ns: number): string {
  let currentNs = ns;
  let hour1 = 3600_000_000_000;
  let minute1 = 60_000_000_000;
  let second1 = 1_000_000_000;
  let millisecond1 = 1_000_000;
  let microsecond1 = 1_000;
  let res = '';
  if (currentNs >= hour1) {
    res += (currentNs / hour1).toFixed(2) + 'h ';
  } else if (currentNs >= minute1) {
    res += (currentNs / minute1).toFixed(2) + 'm ';
  } else if (currentNs >= second1) {
    res += (currentNs / second1).toFixed(2) + 's ';
  } else if (currentNs >= millisecond1) {
    res += (currentNs / millisecond1).toFixed(2) + 'ms ';
  } else if (currentNs >= microsecond1) {
    res += (currentNs / microsecond1).toFixed(2) + 'μs ';
  } else if (currentNs > 0) {
    res += currentNs.toFixed(0) + 'ns ';
  } else if (res == '') {
    res = ns + '';
  }
  return res;
}

export function timeMsFormat2p(timeNs: number): string {
  let currentNs = timeNs;
  let oneHour = 3600_000;
  let oneMinute1 = 60_000;
  let oneSecond = 1_000; // 1 second
  let commonResult = '';
  if (currentNs >= oneHour) {
    commonResult += Math.floor(currentNs / oneHour).toFixed(2) + 'h';
    return commonResult;
  }
  if (currentNs >= oneMinute1) {
    commonResult += Math.floor(currentNs / oneMinute1).toFixed(2) + 'min';
    return commonResult;
  }
  if (currentNs >= oneSecond) {
    commonResult += Math.floor(currentNs / oneSecond).toFixed(2) + 's';
    return commonResult;
  }
  if (currentNs > 0) {
    commonResult += currentNs.toFixed(2) + 'ms';
    return commonResult;
  }
  if (commonResult == '') {
    commonResult = '0s';
  }
  return commonResult;
}

export function formatRealDate(date: Date, fmt: string): string {
  let obj = {
    'M+': date.getMonth() + 1,
    'd+': date.getDate(),
    'h+': date.getHours(),
    'm+': date.getMinutes(),
    's+': date.getSeconds(),
    'q+': Math.floor((date.getMonth() + 3) / 3),
    S: date.getMilliseconds(),
  };
  if (/(y+)/.test(fmt)) fmt = fmt.replace(RegExp.$1, (date.getFullYear() + '').substr(4 - RegExp.$1.length));
  for (let key in obj) {
    if (new RegExp('(' + key + ')').test(fmt)) {
      // @ts-ignore
      fmt = fmt.replace(
        RegExp.$1,
        // @ts-ignore
        RegExp.$1.length == 1 ? obj[key] : ('00' + obj[key]).substr(('' + obj[key]).length)
      );
    }
  }
  return fmt;
}

export function formatRealDateMs(timeNs: number): string {
  return formatRealDate(new Date(timeNs / 1000000), 'MM-dd hh:mm:ss.S');
}

export class JsProfilerSymbol {
  id: number = 0;
  nameId: number = 0;
  name?: string;
  scriptId: number = 0;
  urlId: number = 0;
  url: string = '';
  line: number = 0;
  column: number = 0;
  hitCount: number = 0;
  childrenString?: string;
  childrenIds?: Array<number>;
  children?: Array<JsProfilerSymbol>;
  parentId: number = 0;
  depth: number = -1;

  public clone(): JsProfilerSymbol {
    const cloneSymbol = new JsProfilerSymbol();
    cloneSymbol.name = this.name;
    cloneSymbol.url = this.url;
    cloneSymbol.hitCount = this.hitCount;
    cloneSymbol.children = new Array<JsProfilerSymbol>();
    cloneSymbol.parentId = this.parentId;
    cloneSymbol.depth = this.depth;
    return cloneSymbol;
  }
}

export class HeapTreeDataBean {
  MoudleName: string | undefined;
  AllocationFunction: string | undefined;
  symbolId: number = 0;
  fileId: number = 0;
  startTs: number = 0;
  endTs: number = 0;
  eventType: string | undefined;
  depth: number = 0;
  heapSize: number = 0;
  eventId: number = 0;
  addr: string = '';
  callChinId: number = 0;
}

export class PerfCall {
  sampleId: number = 0;
  depth: number = 0;
  name: string = '';
}

export class FileCallChain {
  callChainId: number = 0;
  depth: number = 0;
  symbolsId: number = 0;
  pathId: number = 0;
  ip: string = '';
}

export class DataCache {
  public static instance: DataCache | undefined;
  public dataDict = new Map<number, string>();
  public eBpfCallChainsMap = new Map<number, Array<FileCallChain>>();
  public nmFileDict = new Map<number, string>();
  public nmHeapFrameMap = new Map<number, Array<HeapTreeDataBean>>();
  public perfCountToMs = 1; // 1000 / freq
  public perfCallChainMap: Map<number, PerfCall> = new Map<number, PerfCall>();
  public jsCallChain: Array<JsProfilerSymbol> | undefined;
  public jsSymbolMap = new Map<number, JsProfilerSymbol>();

  public static getInstance(): DataCache {
    if (!this.instance) {
      this.instance = new DataCache();
    }
    return this.instance;
  }

  public clearAll(): void {
    if (this.dataDict) {
      this.dataDict.clear();
    }
    this.clearEBpf();
    this.clearNM();
    this.clearPerf();
    this.clearJsCache();
  }

  public clearNM(): void {
    this.nmFileDict.clear();
    this.nmHeapFrameMap.clear();
  }

  public clearEBpf(): void {
    this.eBpfCallChainsMap.clear();
  }

  public clearJsCache(): void {
    if (this.jsCallChain) {
      this.jsCallChain.length = 0;
    }
    this.jsSymbolMap.clear();
  }

  public clearPerf(): void {
    this.perfCallChainMap.clear();
  }
}
