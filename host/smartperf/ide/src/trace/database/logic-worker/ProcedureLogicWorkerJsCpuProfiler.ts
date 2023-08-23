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
import { JsCpuProfilerChartFrame, JsCpuProfilerTabStruct, JsCpuProfilerUIStruct } from '../../bean/JsStruct.js';
import { DataCache, JsProfilerSymbol, LogicHandler, convertJSON } from './ProcedureLogicWorkerCommon.js';

const ROOT_ID = 1;
const LAMBDA_FUNCTION_NAME = '(anonymous)';
export class ProcedureLogicWorkerJsCpuProfiler extends LogicHandler {
  private currentEventId!: string;
  private dataCache = DataCache.getInstance();
  private samples = Array<JsCpuProfilerSample>(); // Array index equals id;
  private chartId = 0;
  private tabDataId = 0;

  public handle(msg: any): void {
    this.currentEventId = msg.id;

    if (msg && msg.type) {
      switch (msg.type) {
        case 'jsCpuProfiler-init':
          this.chartId = 0;
          if (!this.dataCache.dataDict || this.dataCache.dataDict.size === 0) {
            this.dataCache.dataDict = msg.params as Map<number, string>;
          }
          this.initCallChain();
          break;
        case 'jsCpuProfiler-call-chain':
          if (!this.dataCache.jsCallChain || this.dataCache.jsCallChain.length === 0) {
            this.dataCache.jsCallChain = convertJSON(msg.params.list) || [];
            this.createCallChain();
          }
          this.queryChartData();
          break;
        case 'jsCpuProfiler-samples':
          this.samples = convertJSON(msg.params.list) || [];
          self.postMessage({
            id: msg.id,
            action: msg.action,
            results: this.combineChartData(),
          });
          // 合并完泳道图数据之后，Tab页不再需要缓存数据
          this.dataCache.clearJsCache();
          break;
        case 'jsCpuProfiler-call-tree':
          this.tabDataId = 0;
          self.postMessage({
            id: msg.id,
            action: msg.action,
            results: this.combineTopDownData(msg.params, null),
          });
          break;
        case 'jsCpuProfiler-bottom-up':
          this.tabDataId = 0;
          self.postMessage({
            id: msg.id,
            action: msg.action,
            results: this.combineBottomUpData(msg.params),
          });
          break;
        case 'jsCpuProfiler-statistics':
          self.postMessage({
            id: msg.id,
            action: msg.action,
            results: this.calStatistic(msg.params.data, msg.params.leftNs, msg.params.rightNs),
          });
          break;
      }
    }
  }

  public clearAll(): void {
    this.dataCache.clearAll();
    this.samples.length = 0;
  }

  private calStatistic(
    chartData: Array<JsCpuProfilerChartFrame>,
    leftNs: number | undefined,
    rightNs: number | undefined
  ): Map<SampleType, number> {
    const typeMap = new Map<SampleType, number>();
    const samplesIdsArr: Array<any> = [];
    const samplesIds = this.findSamplesIds(chartData, [], []);
    for (const id of samplesIds) {
      const sample = this.samples[id];
      if (!sample) {
        continue;
      }
      let sampleTotalTime = sample.dur;
      if (leftNs && rightNs) {
        // 不在框选范围内的不做处理
        if (sample.startTime > rightNs || sample.endTime < leftNs){
          continue;
        }
        // 在框选范围内的被只框选到一部分的根据框选范围调整时间
        const startTime = sample.startTime < leftNs ? leftNs : sample.startTime;
        const endTime = sample.endTime > rightNs ? rightNs : sample.endTime;
        sampleTotalTime = endTime - startTime;
      }

      if (!samplesIdsArr.includes(sample)) {
        samplesIdsArr.push(sample);
        let typeDur = typeMap.get(sample.type);
        if (typeDur) {
          typeMap.set(sample.type, typeDur + sampleTotalTime);
        } else {
          typeMap.set(sample.type, sampleTotalTime);
        }
      }
    }
    return typeMap;
  }

  private findSamplesIds(
    chartData: Array<JsCpuProfilerChartFrame>,
    lastLayerData: Array<JsCpuProfilerChartFrame>,
    samplesIds: Array<number>
  ) {
    for (const data of chartData) {
      if (data.isSelect && data.selfTime > 0 && !lastLayerData.includes(data)) {
        lastLayerData.push(data);
        samplesIds.push(...data.samplesIds);
      } else if (data.children.length > 0) {
        this.findSamplesIds(data.children, lastLayerData, samplesIds);
      }
    }
    return samplesIds;
  }

  /**
   * 建立callChain每个函数的联系，设置depth跟children
   */
  private createCallChain(): void {
    const jsSymbolMap = this.dataCache.jsSymbolMap;
    for (const item of this.dataCache.jsCallChain!) {
      jsSymbolMap.set(item.id, item);
      //root不需要显示,depth为-1
      if (item.id === ROOT_ID) {
        item.depth = -1;
      }
      item.name = this.dataCache.dataDict?.get(item.nameId) || LAMBDA_FUNCTION_NAME;
      item.url = this.dataCache.dataDict?.get(item.urlId) || 'unknown';
      if (item.parentId > 0) {
        let parentSymbol = jsSymbolMap.get(item.parentId);
        if (parentSymbol) {
          if (!parentSymbol.children) {
            parentSymbol.children = new Array<JsProfilerSymbol>();
          }
          parentSymbol.children.push(item);
          item.depth = parentSymbol.depth + 1;
        }
      }
    }
  }

  private combineChartData(): Array<JsCpuProfilerChartFrame> {
    const combineSample = new Array<JsCpuProfilerChartFrame>();
    for (let sample of this.samples) {
      const stackTopSymbol = this.dataCache.jsSymbolMap.get(sample.functionId);
      // root 节点不需要显示
      if (stackTopSymbol?.id === ROOT_ID) {
        sample.type = SampleType.OTHER;
        continue;
      }
      if (stackTopSymbol) {
        let type: string;
        if (stackTopSymbol.name) {
          type = stackTopSymbol.name.substring(
            stackTopSymbol.name!.lastIndexOf('(') + 1,
            stackTopSymbol.name!.lastIndexOf(')')
          );
          switch (type) {
            case 'NAPI':
              sample.type = SampleType.NAPI;
              break;
            case 'ARKUI_ENGINE':
              sample.type = SampleType.ARKUI_ENGINE;
              break;
            case 'BUILTIN':
              sample.type = SampleType.BUILTIN;
              break;
            case 'GC':
              sample.type = SampleType.GC;
              break;
            case 'AINT':
              sample.type = SampleType.AINT;
              break;
            case 'CINT':
              sample.type = SampleType.CINT;
              break;
            case 'AOT':
              sample.type = SampleType.AOT;
              break;
            case 'RUNTIME':
              sample.type = SampleType.RUNTIME;
              break;
            default:
              sample.type = SampleType.OTHER;
              break;
          }
        }

        // 获取栈顶函数的整条调用栈为一个数组 下标0为触发的栈底函数
        sample.stack = this.getFullCallChainOfNode(stackTopSymbol);
        if (combineSample.length === 0) {
          // 首次combineSample没有数据时，用第一条数据创建一个调用树
          this.createNewChartFrame(sample, combineSample);
        } else {
          const lastCallChart = combineSample[combineSample.length - 1];
          if (this.isSymbolEqual(sample.stack[0], lastCallChart) && lastCallChart.endTime === sample.startTime) {
            this.combineCallChain(lastCallChart, sample);
          } else {
            // 一个调用链栈底函数与前一个不同时，需要新加入到combineSample
            this.createNewChartFrame(sample, combineSample);
          }
        }
      }
    }
    return combineSample;
  }

  /**
   * 同级使用广度优先算法，非同级使用深度优先算法，遍历泳道图树结构所有数据，
   * 将name,url,depth,parent相同的函数合并，构建成一个top down的树结构
   * @param combineSample 泳道图第一层数据，非第一层为null
   * @param parent 泳道图合并过的函数，第一层为null
   * @returns 返回第一层树结构(第一层数据通过children囊括了所有的函数)
   */
  private combineTopDownData(
    combineSample: Array<JsCpuProfilerChartFrame> | null,
    parent: JsCpuProfilerTabStruct | null
  ): Array<JsCpuProfilerTabStruct> {
    const sameSymbolMap = new Map<string, JsCpuProfilerTabStruct>();
    const currentLevelData = new Array<JsCpuProfilerTabStruct>();

    const chartArray = combineSample || parent?.chartFrameChildren;
    if (!chartArray) {
      return [];
    }
    // 同级广度优先 便于数据合并
    for (const chartFrame of chartArray) {
      if (!chartFrame.isSelect) {
        continue;
      }
      // 该递归函数已经保证depth跟parent相同，固只需要判断name跟url相同即可
      let symbolKey = chartFrame.name + ' ' + chartFrame.url;
      // lambda 表达式需要根据行列号区分是不是同一个函数
      if (chartFrame.name === LAMBDA_FUNCTION_NAME) {
        symbolKey += ' ' + chartFrame.line + ' ' + chartFrame.column;
      }
      let tabCallFrame: JsCpuProfilerTabStruct;
      if (sameSymbolMap.has(symbolKey)) {
        tabCallFrame = sameSymbolMap.get(symbolKey)!;
        tabCallFrame.totalTime += chartFrame.totalTime;
        tabCallFrame.selfTime += chartFrame.selfTime;
      } else {
        tabCallFrame = this.chartFrameToTabStruct(chartFrame);
        sameSymbolMap.set(symbolKey, tabCallFrame);
        currentLevelData.push(tabCallFrame);
        if (parent) {
          parent.children.push(tabCallFrame);
        }
      }
      tabCallFrame.chartFrameChildren?.push(...chartFrame.children);
    }

    // 非同级深度优先，便于设置children，同时保证下一级函数depth跟parent都相同
    for (const data of currentLevelData) {
      this.combineTopDownData(null, data);
      data.chartFrameChildren = [];
    }

    if (combineSample) {
      // 第一层为返回给Tab页的数据
      return currentLevelData;
    } else {
      return [];
    }
  }

  /**
   * copy整体调用链，从栈顶函数一直copy到栈底函数，
   * 给Parent设置selfTime，totalTime设置为children的selfTime,totalTime
   *  */
  private copyParent(frame: JsCpuProfilerChartFrame, chartFrame: JsCpuProfilerChartFrame): void {
    frame.children = [];
    if (chartFrame.parent) {
      const copyParent = this.cloneChartFrame(chartFrame.parent);
      copyParent.selfTime = frame.selfTime;
      copyParent.totalTime = frame.totalTime;
      frame.children.push(copyParent);
      this.copyParent(copyParent, chartFrame.parent);
    }
  }

  /**
   * 步骤1：框选/点选的chart树逆序
   * 步骤2：将name,url,parent，层级相同的函数合并
   * @param chartTreeArray ui传递的树结构
   * @returns 合并的Array<JsCpuProfilerChartFrame>树结构
   */
  private combineBottomUpData(chartTreeArray: Array<JsCpuProfilerChartFrame>): Array<JsCpuProfilerTabStruct> {
    const reverseTreeArray = new Array<JsCpuProfilerChartFrame>();
    // 将树结构逆序，parent变成children
    this.reverseChartFrameTree(chartTreeArray, reverseTreeArray);
    // 将逆序的树结构合并返回
    return this.combineTopDownData(reverseTreeArray, null);
  }

  /**
   * 树结构逆序
   * @param chartTreeArray 正序的树结构
   * @param reverseTreeArray 逆序的树结构
   */
  private reverseChartFrameTree(
    chartTreeArray: Array<JsCpuProfilerChartFrame>,
    reverseTreeArray: Array<JsCpuProfilerChartFrame>
  ) {
    const that = this;
    function recursionTree(chartFrame: JsCpuProfilerChartFrame) {
      // isSelect为框选/点选范围内的函数，其他都不需要处理
      if (!chartFrame.isSelect) {
        return;
      }
      //界面第一层只显示栈顶函数，只有栈顶函数的selfTime > 0
      if (chartFrame.selfTime > 0) {
        const copyFrame = that.cloneChartFrame(chartFrame);
        // 每个栈顶函数的parent的时间为栈顶函数的时间
        copyFrame.selfTime = chartFrame.selfTime;
        copyFrame.totalTime = chartFrame.totalTime;
        reverseTreeArray.push(copyFrame);
        // 递归处理parent的的totalTime selfTime
        that.copyParent(copyFrame, chartFrame);
      }

      if (chartFrame.children.length > 0) {
        for (const children of chartFrame.children) {
          children.parent = chartFrame;
          recursionTree(children);
        }
      }
    }

    //递归树结构
    for (const chartFrame of chartTreeArray) {
      recursionTree(chartFrame);
    }
  }

  private createNewChartFrame(sample: JsCpuProfilerSample, combineSample: Array<JsCpuProfilerChartFrame>): void {
    let lastSymbol: JsCpuProfilerChartFrame;
    for (const [idx, symbol] of sample.stack!.entries()) {
      if (idx === 0) {
        lastSymbol = this.symbolToChartFrame(sample, symbol);
        combineSample.push(lastSymbol);
      } else {
        const callFrame = this.symbolToChartFrame(sample, symbol);
        lastSymbol!.children.push(callFrame);
        callFrame.parentId = lastSymbol!.id;
        lastSymbol = callFrame;
      }
      if (idx + 1 === sample.stack?.length) {
        lastSymbol.selfTime = sample.dur;
      }
    }
  }

  /**
   * 相邻的两个sample的name,url，depth相同，且上一个的endTime等于下一个的startTime,
   * 则两个sample的调用栈合并
   * @param lastCallTree 上一个已经合并的树结构调用栈
   * @param sample 当前样本数据
   */
  private combineCallChain(lastCallTree: JsCpuProfilerChartFrame, sample: JsCpuProfilerSample): void {
    let lastCallTreeSymbol = lastCallTree;
    let parentCallFrame: JsCpuProfilerChartFrame;
    let isEqual = true;
    for (const [idx, symbol] of sample.stack!.entries()) {
      // 是否为每次采样的栈顶函数
      const isLastSymbol = idx + 1 === sample.stack?.length;
      if (
        isEqual &&
        this.isSymbolEqual(symbol, lastCallTreeSymbol) &&
        lastCallTreeSymbol.depth === idx &&
        lastCallTreeSymbol.endTime === sample.startTime
      ) {
        // 如果函数名跟depth匹配，则更新函数的持续时间
        lastCallTreeSymbol.endTime = sample.endTime;
        lastCallTreeSymbol.totalTime = sample.endTime - lastCallTreeSymbol.startTime;
        lastCallTreeSymbol.samplesIds.push(sample.id);
        let lastChildren = lastCallTreeSymbol.children;
        parentCallFrame = lastCallTreeSymbol;
        if (lastChildren && lastChildren.length > 0) {
          lastCallTreeSymbol = lastChildren[lastChildren.length - 1];
        }
        isEqual = true;
      } else {
        // 如果不匹配,则作为新的分支添加到lastCallTree
        const deltaFrame = this.symbolToChartFrame(sample, symbol);
        parentCallFrame!.children.push(deltaFrame);
        deltaFrame.parentId = parentCallFrame!.id;
        parentCallFrame = deltaFrame;
        isEqual = false;
      }
      // 每次采样的栈顶函数的selfTime为该次采样数据的时间
      if (isLastSymbol) {
        parentCallFrame.selfTime += sample.dur;
      }
    }
  }

  /**
   * 根据每个sample的栈顶函数，获取完整的调用栈
   * @param node 栈顶函数
   * @returns 完整的调用栈
   */
  private getFullCallChainOfNode(node: JsProfilerSymbol): Array<JsProfilerSymbol> {
    const callChain = new Array<JsProfilerSymbol>();
    callChain.push(node);
    while (node.parentId !== 0) {
      const parent = this.dataCache.jsSymbolMap.get(node.parentId);
      // id 1 is root Node
      if (!parent || parent.id <= ROOT_ID) {
        break;
      }
      callChain.push(parent);
      node = parent;
    }
    callChain.reverse();
    return callChain;
  }

  /**
   * 创建一个JsCpuProfilerChartFrame 作为绘制泳道图的结构
   * @param sample 数据库样本数据
   * @param symbol 样本的每一个函数
   * @returns JsCpuProfilerChartFrame
   */
  private symbolToChartFrame(sample: JsCpuProfilerSample, symbol: JsProfilerSymbol): JsCpuProfilerChartFrame {
    const chartFrame = new JsCpuProfilerChartFrame(
      this.chartId++,
      symbol.name || LAMBDA_FUNCTION_NAME,
      sample.startTime,
      sample.endTime,
      sample.dur,
      symbol.depth,
      symbol.url,
      symbol.line,
      symbol.column
    );
    chartFrame.samplesIds.push(sample.id);
    return chartFrame;
  }

  /**
   * 将泳道图数据JsCpuProfilerChartFrame转化为JsCpuProfilerTabStruct 作为绘制Ta页的结构
   * @param chartCallChain 泳道图函数信息
   * @returns JsCpuProfilerTabStruct
   */
  private chartFrameToTabStruct(chartCallChain: JsCpuProfilerChartFrame): JsCpuProfilerTabStruct {
    const tabData = new JsCpuProfilerTabStruct(
      chartCallChain.name,
      chartCallChain.selfTime,
      chartCallChain.totalTime,
      chartCallChain.depth,
      chartCallChain.url,
      chartCallChain.line,
      chartCallChain.column,
      this.tabDataId++
    );
    return tabData;
  }

  private cloneChartFrame(frame: JsCpuProfilerChartFrame): JsCpuProfilerChartFrame {
    const copyFrame = new JsCpuProfilerChartFrame(
      frame.id,
      frame.name,
      frame.startTime,
      frame.endTime,
      frame.totalTime,
      frame.depth,
      frame.url,
      frame.line,
      frame.column
    );
    copyFrame.parentId = frame.parentId;
    copyFrame.isSelect = true;
    return copyFrame;
  }

  private isSymbolEqual(symbol: JsProfilerSymbol, uiData: JsCpuProfilerUIStruct): boolean {
    return symbol.name === uiData.name && symbol.url === uiData.url;
  }

  private initCallChain(): void {
    const sql = `SELECT function_id AS id,
                        function_index AS nameId,
                        script_id AS scriptId,
                        url_index AS urlId,
                        line_number as line,
                        column_number as column,
                        hit_count AS hitCount,
                        children AS childrenString,
                        parent_id AS parentId
                    FROM
                        js_cpu_profiler_node`;
    this.queryData(this.currentEventId!, 'jsCpuProfiler-call-chain', sql, {});
  }

  private queryChartData(): void {
    const sql = `SELECT id,
                    function_id AS functionId,
                    start_time - start_ts AS startTime,
                    end_time - start_ts AS endTime,
                    dur
                  FROM
                    js_cpu_profiler_sample,trace_range`;
    this.queryData(this.currentEventId!, 'jsCpuProfiler-samples', sql, {});
  }
}

class JsCpuProfilerSample {
  id: number = 0;
  functionId: number = 0;
  startTime: number = 0;
  endTime: number = 0;
  dur: number = 0;
  type: SampleType = SampleType.OTHER;
  stack?: Array<JsProfilerSymbol>;
}

export enum SampleType {
  OTHER = 'OTHER',
  NAPI = 'NAPI',
  ARKUI_ENGINE = 'ARKUI_ENGINE',
  BUILTIN = 'BUILTIN',
  GC = 'GC',
  AINT = 'AINT',
  CINT = 'CINT',
  AOT = 'AOT',
  RUNTIME = 'RUNTIME',
}
