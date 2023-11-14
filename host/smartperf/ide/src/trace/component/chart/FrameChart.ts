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

import { BaseElement, element } from '../../../base-ui/BaseElement.js';
import { Rect } from '../trace/timer-shaft/Rect.js';
import { ChartMode, ChartStruct, draw, setFuncFrame } from '../../bean/FrameChartStruct.js';
import { SpApplication } from '../../SpApplication.js';
import { Utils } from '../trace/base/Utils.js';
import { SpHiPerf } from './SpHiPerf.js';

const scaleHeight = 30; // 刻度尺高度
const depthHeight = 20; // 调用栈高度
const filterPixel = 2; // 过滤像素
const textMaxWidth = 50;
const scaleRatio = 0.2; // 缩放比例
const ms10 = 10_000_000;

class NodeValue {
  size: number;
  count: number;
  dur: number;

  constructor() {
    this.size = 0;
    this.count = 0;
    this.dur = 0;
  }
}

@element('tab-framechart')
export class FrameChart extends BaseElement {
  private canvas!: HTMLCanvasElement;
  private canvasContext!: CanvasRenderingContext2D;
  private floatHint!: HTMLDivElement | undefined | null; // 悬浮框

  private rect: Rect = new Rect(0, 0, 0, 0);
  private _mode = ChartMode.Byte;
  private startX = 0; // 画布相对于整个界面的x坐标
  private startY = 0; // 画布相对于整个界面的y坐标
  private canvasX = -1; // 鼠标当前所在画布位置x坐标
  private canvasY = -1; // 鼠标当前所在画布位置y坐标
  private hintContent = ''; // 悬浮框内容。 html格式字符串
  private rootNode!: ChartStruct;
  private currentData: Array<ChartStruct> = [];
  private xPoint = 0; // x in rect
  private isFocusing = false; // 鼠标是否在画布范围内
  private canvasScrollTop = 0; // Tab页上下滚动位置
  private _maxDepth = 0;
  private chartClickListenerList: Array<Function> = [];
  private isUpdateCanvas = false;
  private isClickMode = false; //是否为点选模式

  /**
   * set chart mode
   * @param mode chart format for data mode
   */
  set mode(mode: ChartMode) {
    this._mode = mode;
  }

  set data(val: Array<ChartStruct>) {
    ChartStruct.lastSelectFuncStruct = undefined;
    this.setSelectStatusRecursive(ChartStruct.selectFuncStruct, true);
    ChartStruct.selectFuncStruct = undefined;
    this.isClickMode = false;
    this.currentData = val;
    this.resetTrans();
    this.calDrawArgs(true);
  }

  set tabPaneScrollTop(scrollTop: number) {
    this.canvasScrollTop = scrollTop;
    this.hideTip();
  }

  private get total(): number {
    return this.getNodeValue(this.rootNode);
  }

  private getNodeValue(node: ChartStruct): number {
    switch (this._mode) {
      case ChartMode.Byte:
        return node.drawSize || node.size;
      case ChartMode.Count:
        return node.drawCount || node.count;
      case ChartMode.Duration:
        return node.drawDur || node.dur;
    }
  }

  /**
   * add callback of chart click
   * @param callback function of chart click
   */
  public addChartClickListener(callback: Function): void {
    if (this.chartClickListenerList.indexOf(callback) < 0) {
      this.chartClickListenerList.push(callback);
    }
  }

  /**
   * remove callback of chart click
   * @param callback function of chart click
   */
  public removeChartClickListener(callback: Function): void {
    const index = this.chartClickListenerList.indexOf(callback);
    if (index > -1) {
      this.chartClickListenerList.splice(index, 1);
    }
  }

  private createRootNode(): void {
    // 初始化root
    this.rootNode = new ChartStruct();
    this.rootNode.symbol = 'root';
    this.rootNode.depth = 0;
    this.rootNode.percent = 1;
    this.rootNode.frame = new Rect(0, scaleHeight, this.canvas!.width, depthHeight);
    for (const node of this.currentData!) {
      this.rootNode.children.push(node);
      this.rootNode.count += node.drawCount || node.count;
      this.rootNode.size += node.drawSize || node.size;
      this.rootNode.dur += node.drawDur || node.dur;
      node.parent = this.rootNode;
    }
  }

  /**
   * 1.计算调用栈最大深度
   * 2.计算搜索情况下每个函数块显示的大小(非实际大小)
   * 3.计算点选情况下每个函数块的显示大小(非实际大小)
   * @param initRoot 是否初始化root节点
   */
  private calDrawArgs(initRoot: boolean): void {
    this._maxDepth = 0;
    if (initRoot) {
      this.createRootNode();
    }
    this.initData(this.rootNode, 0, true);
    this.selectInit();
    this.setRootValue();
    this.rect.width = this.canvas!.width;
    this.rect.height = (this._maxDepth + 1) * depthHeight + scaleHeight;
    this.canvas!.style.height = `${this.rect!.height}px`;
    this.canvas!.height = Math.ceil(this.rect!.height);
  }

  /**
   * 点选情况下由点选来设置每个函数的显示Size
   */
  private selectInit(): void {
    const node = ChartStruct.selectFuncStruct;
    if (node) {
      const module = new NodeValue();
      node.drawCount = 0;
      node.drawDur = 0;
      node.drawSize = 0;
      for (let child of node.children) {
        node.drawCount += child.searchCount;
        node.drawDur += child.searchDur;
        node.drawSize += child.searchSize;
      }
      module.count = node.drawCount = node.drawCount || node.count;
      module.dur = node.drawDur = node.drawDur || node.dur;
      module.size = node.drawSize = node.drawSize || node.size;

      this.setParentDisplayInfo(node, module, true);
      this.setChildrenDisplayInfo(node);
    }
  }

  // 设置root显示区域value 以及占真实value的百分比
  private setRootValue(): void {
    let currentValue = '';
    let currentValuePercent = 1;
    switch (this._mode) {
      case ChartMode.Byte:
        currentValue = Utils.getBinaryByteWithUnit(this.total);
        currentValuePercent = this.total / this.rootNode.size;
        break;
      case ChartMode.Count:
        currentValue = Utils.timeMsFormat2p(this.total * (SpHiPerf.stringResult?.fValue || 1));
        currentValuePercent = this.total / this.rootNode.count;
        break;
      case ChartMode.Duration:
        currentValue = Utils.getProbablyTime(this.total);
        currentValuePercent = this.total / this.rootNode.dur;
        break;
    }
    this.rootNode.symbol = `Root : ${currentValue} (${(currentValuePercent * 100).toFixed(2)}%)`;
  }

  /**
   * 计算调用栈最大深度，计算每个node显示大小
   * @param node 函数块
   * @param depth 当前递归深度
   * @param calDisplay 该层深度是否需要计算显示大小
   */
  private initData(node: ChartStruct, depth: number, calDisplay: boolean): void {
    node.depth = depth;
    depth++;
    //设置搜索以及点选的显示值，将点击/搜索的值设置为父节点的显示值
    this.clearDisplayInfo(node);
    if (node.isSearch && calDisplay) {
      const module = new NodeValue();
      module.size = node.drawSize = node.searchSize = node.size;
      module.count = node.drawCount = node.searchCount = node.count;
      module.dur = node.drawDur = node.searchDur = node.dur;
      this.setParentDisplayInfo(node, module, false);
      calDisplay = false;
    }

    // 设置parent以及计算最大的深度
    if (node.children && node.children.length > 0) {
      for (const children of node.children) {
        children.parent = node;
        this.initData(children, depth, calDisplay);
      }
    } else {
      this._maxDepth = Math.max(depth, this._maxDepth);
    }
  }

  // 递归设置node parent的显示大小
  private setParentDisplayInfo(node: ChartStruct, module: NodeValue, isSelect?: boolean): void {
    const parent = node.parent;
    if (parent) {
      if (isSelect) {
        parent.isChartSelect = true;
        parent.isChartSelectParent = true;
        parent.drawCount = module.count;
        parent.drawDur = module.dur;
        parent.drawSize = module.size;
      } else {
        parent.searchCount += module.count;
        parent.searchDur += module.dur;
        parent.searchSize += module.size;
        // 点击模式下不需要赋值draw value，由点击去
        if (!this.isClickMode) {
          parent.drawDur = parent.searchDur;
          parent.drawCount = parent.searchCount;
          parent.drawSize = parent.searchSize;
        }
      }
      this.setParentDisplayInfo(parent, module, isSelect);
    }
  }

  /**
   * 点击与搜索同时触发情况下，由点击去设置绘制大小
   * @param node 当前点选的函数
   * @returns void
   */
  private setChildrenDisplayInfo(node: ChartStruct): void {
    if (node.children.length < 0) {
      return;
    }
    for (const children of node.children) {
      children.drawCount = children.searchCount || children.count;
      children.drawDur = children.searchDur || children.dur;
      children.drawSize = children.searchSize || children.size;
      this.setChildrenDisplayInfo(children);
    }
  }

  private clearDisplayInfo(node: ChartStruct): void {
    node.drawCount = 0;
    node.drawDur = 0;
    node.drawSize = 0;
    node.searchCount = 0;
    node.searchDur = 0;
    node.searchSize = 0;
  }

  /**
   * 计算每个函数块的坐标信息以及绘制火焰图
   */
  public async calculateChartData(): Promise<void> {
    this.clearCanvas();
    this.canvasContext?.beginPath();
    // 绘制刻度线
    this.drawCalibrationTails();
    // 绘制root节点
    draw(this.canvasContext, this.rootNode);
    // 设置子节点的位置以及宽高
    this.setFrameData(this.rootNode);
    // 绘制子节点
    this.drawFrameChart(this.rootNode);
    this.canvasContext?.closePath();
  }

  /**
   * 清空画布
   */
  public clearCanvas(): void {
    this.canvasContext?.clearRect(0, 0, this.canvas!.width, this.canvas!.height);
  }

  /**
   * 在窗口大小变化时调整画布大小
   */
  public updateCanvas(updateWidth: boolean, newWidth?: number): void {
    if (this.canvas instanceof HTMLCanvasElement) {
      this.canvas.style.width = `${100}%`;
      this.canvas.style.height = `${this.rect!.height}px`;
      if (this.canvas.clientWidth === 0 && newWidth) {
        this.canvas.width = newWidth - depthHeight * 2;
      } else {
        this.canvas.width = this.canvas.clientWidth;
      }
      this.canvas.height = Math.ceil(this.rect!.height);
      this.updateCanvasCoord();
    }
    if (
      this.rect.width === 0 ||
      updateWidth ||
      Math.round(newWidth!) !== this.canvas!.width + depthHeight * 2 ||
      newWidth! > this.rect.width
    ) {
      this.rect.width = this.canvas!.width;
    }
  }

  /**
   * 更新画布坐标
   */
  private updateCanvasCoord(): void {
    if (this.canvas instanceof HTMLCanvasElement) {
      this.isUpdateCanvas = this.canvas.clientWidth !== 0;
      if (this.canvas.getBoundingClientRect()) {
        const box = this.canvas.getBoundingClientRect();
        const D = document.documentElement;
        this.startX = box.left + Math.max(D.scrollLeft, document.body.scrollLeft) - D.clientLeft;
        this.startY = box.top + Math.max(D.scrollTop, document.body.scrollTop) - D.clientTop + this.canvasScrollTop;
      }
    }
  }

  /**
   * 绘制刻度尺，分为100段，每10段画一条长线
   */
  private drawCalibrationTails(): void {
    const spApplication = <SpApplication>document.getElementsByTagName('sp-application')[0];
    this.canvasContext!.lineWidth = 0.5;
    this.canvasContext?.moveTo(0, 0);
    this.canvasContext?.lineTo(this.canvas!.width, 0);
    for (let i = 0; i <= 10; i++) {
      let startX = Math.floor((this.canvas!.width / 10) * i);
      for (let j = 0; j < 10; j++) {
        this.canvasContext!.lineWidth = 0.5;
        const startItemX = startX + Math.floor((this.canvas!.width / 100) * j);
        this.canvasContext?.moveTo(startItemX, 0);
        this.canvasContext?.lineTo(startItemX, 10);
      }
      if (i === 0) {
        continue;
      }
      this.canvasContext!.lineWidth = 1;
      const sizeRatio = this.canvas!.width / this.rect.width; // scale ratio
      if (spApplication.dark) {
        this.canvasContext!.strokeStyle = '#888';
      } else {
        this.canvasContext!.strokeStyle = '#ddd';
      }
      this.canvasContext?.moveTo(startX, 0);
      this.canvasContext?.lineTo(startX, this.canvas!.height);
      if (spApplication.dark) {
        this.canvasContext!.fillStyle = '#fff';
      } else {
        this.canvasContext!.fillStyle = '#000';
      }
      let calibration = '';
      switch (this._mode) {
        case ChartMode.Byte:
          calibration = Utils.getByteWithUnit(((this.total * sizeRatio) / 10) * i);
          break;
        case ChartMode.Count:
          //count 转化为时间
          calibration = Utils.timeMsFormat2p(
            ((this.total * (SpHiPerf.stringResult?.fValue || 1) * sizeRatio) / 10) * i
          );
          break;
        case ChartMode.Duration:
          calibration = Utils.getProbablyTime(((this.total * sizeRatio) / 10) * i);
          break;
      }
      const size = this.canvasContext!.measureText(calibration).width;
      this.canvasContext?.fillText(calibration, startX - size - 5, depthHeight, textMaxWidth);
      this.canvasContext?.stroke();
    }
  }

  /**
   * 设置每个node的宽高，开始坐标
   * @param node 函数块
   */
  private setFrameData(node: ChartStruct): void {
    if (node.children.length > 0) {
      for (const children of node.children) {
        node.isDraw = false;
        if (this.isClickMode && ChartStruct.selectFuncStruct) {
          //处理点击逻辑，当前node为点选调用栈，children不是点选调用栈，width置为0
          if (!children.isChartSelect) {
            if (children.frame) {
              children.frame.x = this.rootNode.frame?.x || 0;
              children.frame.width = 0;
              children.percent = 0;
            } else {
              children.frame = new Rect(0, 0, 0, 0);
            }
            this.setFrameData(children);
            continue;
          }
        }
        const childrenValue = this.getNodeValue(children);
        setFuncFrame(children, this.rect, this.total, this._mode);
        children.percent = childrenValue / this.total;
        this.setFrameData(children);
      }
    }
  }

  /**
   * 计算有效数据，当node的宽度太小不足以绘制时
   * 计算忽略node的size
   * 忽略的size将转换成width，按照比例平摊到显示的node上
   * @param node 当前node
   * @param effectChildList 生效的node
   */
  private calEffectNode(node: ChartStruct, effectChildList: Array<ChartStruct>): number {
    const ignore = new NodeValue();
    for (const children of node.children) {
      // 小于1px的不绘制,并将其size平均赋值给>1px的
      if (children.frame!.width >= filterPixel) {
        effectChildList.push(children);
      } else {
        if (node.isChartSelect || this.isSearch(node)) {
          ignore.size += children.drawSize;
          ignore.count += children.drawCount;
          ignore.dur += children.drawDur;
        } else {
          ignore.size += children.size;
          ignore.count += children.count;
          ignore.dur += children.dur;
        }
      }
    }
    switch (this._mode) {
      case ChartMode.Byte:
        return ignore.size;
      case ChartMode.Count:
        return ignore.count;
      case ChartMode.Duration:
        return ignore.dur;
    }
  }

  private isSearch(node: ChartStruct): boolean {
    switch (this._mode) {
      case ChartMode.Byte:
        return node.searchSize > 0;
      case ChartMode.Count:
        return node.searchCount > 0;
      case ChartMode.Duration:
        return node.searchDur > 0;
    }
  }

  /**
   * 绘制每个函数色块
   * @param node 函数块
   */
  private drawFrameChart(node: ChartStruct): void {
    const effectChildList: Array<ChartStruct> = [];
    const nodeValue = this.getNodeValue(node);

    if (node.children && node.children.length > 0) {
      const ignoreValue = this.calEffectNode(node, effectChildList);
      let x = node.frame!.x;
      if (effectChildList.length > 0) {
        for (let children of effectChildList) {
          children.frame!.x = x;
          const childrenValue = this.getNodeValue(children);
          children.frame!.width = (childrenValue / (nodeValue - ignoreValue)) * node.frame!.width;
          x += children.frame!.width;
          if (this.nodeInCanvas(children)) {
            draw(this.canvasContext!, children);
            this.drawFrameChart(children);
          }
        }
      } else {
        const firstChildren = node.children[0];
        firstChildren.frame!.x = node.frame!.x;
        // perf parent有selfTime 需要所有children的count跟
        firstChildren.frame!.width = node.frame!.width * (ignoreValue / nodeValue);
        draw(this.canvasContext!, firstChildren);
        this.drawFrameChart(firstChildren);
      }
    }
  }

  /**
   * 根据鼠标当前的坐标递归查找对应的函数块
   *
   * @param nodes
   * @param canvasX 鼠标相对于画布开始点的x坐标
   * @param canvasY 鼠标相对于画布开始点的y坐标
   * @returns 当前鼠标位置的函数块
   */
  private searchDataByCoord(nodes: Array<ChartStruct>, canvasX: number, canvasY: number): ChartStruct | null {
    for (const node of nodes) {
      if (node.frame?.contains(canvasX, canvasY)) {
        return node;
      } else {
        const result = this.searchDataByCoord(node.children, canvasX, canvasY);
        // if not found in this branch;search another branch
        if (!result) {
          continue;
        }
        return result;
      }
    }
    return null;
  }

  /**
   * 显示悬浮框信息，更新位置
   */
  private showTip(): void {
    this.floatHint!.innerHTML = this.hintContent;
    this.floatHint!.style.display = 'block';
    let x = this.canvasX;
    let y = this.canvasY - this.canvasScrollTop;
    //右边的函数块悬浮框显示在函数左边
    if (this.canvasX + this.floatHint!.clientWidth > (this.canvas?.clientWidth || 0)) {
      x -= this.floatHint!.clientWidth - 1;
    } else {
      x += scaleHeight;
    }
    //最下边函数块悬浮框显示在函数上边
    y -= this.floatHint!.clientHeight - 1;

    this.floatHint!.style.transform = `translate(${x}px,${y}px)`;
  }

  /**
   * 递归设置传入node的parent以及children的isSelect
   * 将上次点选的整条树的isSelect置为false
   * 将本次点击的整条树的isSelect置为true
   * @param node 点击的node
   * @param isSelect 点选
   */
  private setSelectStatusRecursive(node: ChartStruct | undefined, isSelect: boolean): void {
    if (!node) {
      return;
    }
    node.isChartSelect = isSelect;

    // 处理子节点及其子节点的子节点
    const stack: ChartStruct[] = [node]; // 使用栈来实现循环处理
    while (stack.length > 0) {
      const currentNode = stack.pop();
      if (currentNode) {
        currentNode.children.forEach((child) => {
          child.isChartSelect = isSelect;
          stack.push(child);
        });
      }
    }

    // 处理父节点
    while (node?.parent) {
      node.parent.isChartSelect = isSelect;
      node.parent.isChartSelectParent = isSelect;
      node = node.parent;
    }
  }

  /**
   * 点选后重绘火焰图
   */
  private clickRedraw(): void {
    //将上次点选的isSelect置为false
    if (ChartStruct.lastSelectFuncStruct) {
      this.setSelectStatusRecursive(ChartStruct.lastSelectFuncStruct!, false);
    }
    // 递归设置点选的parent，children为点选状态
    this.setSelectStatusRecursive(ChartStruct.selectFuncStruct!, true);

    this.calDrawArgs(false);
    this.calculateChartData();
  }

  /**
   * 点击w s的放缩算法
   * @param index < 0 缩小 , > 0 放大
   */
  private scale(index: number): void {
    let newWidth = 0;
    let deltaWidth = this.rect!.width * scaleRatio;
    const ratio = 1 + scaleRatio;
    if (index > 0) {
      // zoom in
      newWidth = this.rect!.width + deltaWidth;
      const sizeRatio = this.canvas!.width / this.rect.width; // max scale
      switch (this._mode) {
        case ChartMode.Byte:
        case ChartMode.Count:
          if (Math.round((this.total * sizeRatio) / ratio) <= 10) {
            if (this.xPoint === 0) {
              return;
            }
            newWidth = this.canvas!.width / (10 / this.total);
          }
          break;
        case ChartMode.Duration:
          if (Math.round((this.total * sizeRatio) / ratio) <= ms10) {
            if (this.xPoint === 0) {
              return;
            }
            newWidth = this.canvas!.width / (ms10 / this.total);
          }
          break;
      }
      deltaWidth = newWidth - this.rect!.width;
    } else {
      // zoom out
      newWidth = this.rect!.width - deltaWidth;
      if (newWidth < this.canvas!.width) {
        newWidth = this.canvas!.width;
        this.resetTrans();
      }
      deltaWidth = this.rect!.width - newWidth;
    }
    // width not change
    if (newWidth === this.rect.width) {
      return;
    }
    this.translationByScale(index, deltaWidth, newWidth);
  }

  private resetTrans(): void {
    this.xPoint = 0;
  }

  /**
   * 放缩之后的平移算法
   * @param index  < 0 缩小 , > 0 放大
   * @param deltaWidth 放缩增量
   * @param newWidth 放缩后的宽度
   */
  private translationByScale(index: number, deltaWidth: number, newWidth: number): void {
    const translationValue = (deltaWidth * (this.canvasX - this.xPoint)) / this.rect.width;
    if (index > 0) {
      this.xPoint -= translationValue;
    } else {
      this.xPoint += translationValue;
    }
    this.rect!.width = newWidth;

    this.translationDraw();
  }

  /**
   * 点击a d 平移
   * @param index < 0 左移； >0 右移
   */
  private translation(index: number): void {
    const offset = this.canvas!.width / 10;
    if (index < 0) {
      this.xPoint += offset;
    } else {
      this.xPoint -= offset;
    }
    this.translationDraw();
  }

  /**
   * judge position ro fit canvas and draw
   */
  private translationDraw(): void {
    // right trans limit
    if (this.xPoint > 0) {
      this.xPoint = 0;
    }
    // left trans limit
    if (this.rect.width + this.xPoint < this.canvas!.width) {
      this.xPoint = this.canvas!.width - this.rect.width;
    }
    this.rootNode.frame!.width = this.rect.width;
    this.rootNode.frame!.x = this.xPoint;
    this.calculateChartData();
  }

  private nodeInCanvas(node: ChartStruct): boolean {
    if (!node.frame) {
      return false;
    }
    return node.frame.x + node.frame.width >= 0 && node.frame.x < this.canvas.clientWidth;
  }
  private onMouseClick(e: MouseEvent): void {
    if (e.button === 0) {
      // mouse left button
      if (ChartStruct.hoverFuncStruct && ChartStruct.hoverFuncStruct !== ChartStruct.selectFuncStruct) {
        ChartStruct.lastSelectFuncStruct = ChartStruct.selectFuncStruct;
        ChartStruct.selectFuncStruct = ChartStruct.hoverFuncStruct;
        this.isClickMode = ChartStruct.selectFuncStruct !== this.rootNode;
        this.rect.width = this.canvas!.clientWidth;
        // 重置缩放
        this.resetTrans();
        this.rootNode.frame!.x = this.xPoint;
        this.rootNode.frame!.width = this.rect.width = this.canvas.clientWidth;
        // 重新绘图
        this.clickRedraw();
        document.dispatchEvent(
          new CustomEvent('number_calibration', {
            detail: {
              time: ChartStruct.selectFuncStruct.tsArray,
              counts: ChartStruct.selectFuncStruct.countArray,
              durations: ChartStruct.selectFuncStruct.durArray,
            },
          })
        );
      }
    }
    this.hideTip();
  }

  private hideTip(): void {
    if (this.floatHint) {
      this.floatHint.style.display = 'none';
    }
  }

  /**
   * 更新悬浮框内容
   */
  private updateTipContent(): void {
    const hoverNode = ChartStruct.hoverFuncStruct;
    if (!hoverNode) {
      return;
    }
    const name = hoverNode?.symbol.replace(/</g, '&lt;').replace(/>/g, '&gt;');
    const percent = ((hoverNode?.percent || 0) * 100).toFixed(2);
    switch (this._mode) {
      case ChartMode.Byte:
        const size = Utils.getByteWithUnit(this.getNodeValue(hoverNode));
        const countPercent = ((this.getNodeValue(hoverNode) / this.total) * 100).toFixed(2);
        this.hintContent = `
                    <span class="bold">Symbol: </span> <span class="text">${name} </span> <br>
                    <span class="bold">Lib: </span> <span class="text">${hoverNode?.lib}</span> <br>
                    <span class="bold">Addr: </span> <span>${hoverNode?.addr}</span> <br>
                    <span class="bold">Size: </span> <span>${size} (${percent}%) </span> <br>
                    <span class="bold">Count: </span> <span>${hoverNode?.count} (${countPercent}%)</span>`;
        break;
      case ChartMode.Count:
        const count = this.getNodeValue(hoverNode);
        const dur = Utils.timeMsFormat2p(count * (SpHiPerf.stringResult?.fValue || 1));
        this.hintContent = `
                    <span class="bold">Name: </span> <span class="text">${name} </span> <br>
                    <span class="bold">Lib: </span> <span class="text">${hoverNode?.lib}</span>
                    <br>
                    <span class="bold">Addr: </span> <span>${hoverNode?.addr}</span>
                    <br>
                    <span class="bold">Dur: </span> <span>${dur} (${percent}%)</span>
                    <br>
                    <span class="bold">Count: </span> <span> ${count} (${percent}%)</span>`;
        break;
      case ChartMode.Duration:
        const duration = Utils.getProbablyTime(this.getNodeValue(hoverNode));
        this.hintContent = `
                    <span class="bold">Name: </span> <span class="text">${name} </span> <br>
                    <span class="bold">Lib: </span> <span class="text">${hoverNode?.lib}</span>
                    <br>
                    <span class="bold">Addr: </span> <span>${hoverNode?.addr}</span> <br>
                    <span class="bold">Duration: </span> <span>${duration} (${percent}%)</span>`;
        break;
    }
  }

  /**
   * mouse on canvas move event
   */
  private onMouseMove(): void {
    const lastNode = ChartStruct.hoverFuncStruct;
    // 鼠标移动到root节点不作显示
    const hoverRootNode = this.rootNode.frame?.contains(this.canvasX, this.canvasY);
    if (hoverRootNode) {
      ChartStruct.hoverFuncStruct = this.rootNode;
      return;
    }
    // 查找鼠标所在那个node上
    const searchResult = this.searchDataByCoord(this.currentData!, this.canvasX, this.canvasY);
    if (searchResult && (searchResult.isDraw || searchResult.depth === 0)) {
      ChartStruct.hoverFuncStruct = searchResult;
      // 悬浮的node未改变，不需要更新悬浮框文字信息，不绘图
      if (searchResult !== lastNode) {
        this.updateTipContent();
        this.calculateChartData();
      }
      this.showTip();
    } else {
      this.hideTip();
      ChartStruct.hoverFuncStruct = undefined;
    }
  }

  /**
   * 监听页面Size变化
   */
  private listenerResize(): void {
    new ResizeObserver(() => {
      if (this.canvas!.getBoundingClientRect()) {
        const box = this.canvas!.getBoundingClientRect();
        const element = document.documentElement;
        this.startX = box.left + Math.max(element.scrollLeft, document.body.scrollLeft) - element.clientLeft;
        this.startY =
          box.top + Math.max(element.scrollTop, document.body.scrollTop) - element.clientTop + this.canvasScrollTop;
      }
    }).observe(document.documentElement);
  }

  public initElements(): void {
    this.canvas = this.shadowRoot!.querySelector('#canvas')!;
    this.canvasContext = this.canvas.getContext('2d')!;
    this.floatHint = this.shadowRoot?.querySelector('#float_hint');

    this.canvas!.oncontextmenu = (): boolean => {
      return false;
    };
    this.canvas!.onmouseup = (e): void => {
      this.onMouseClick(e);
    };

    this.canvas!.onmousemove = (e): void => {
      if (!this.isUpdateCanvas) {
        this.updateCanvasCoord();
      }
      this.canvasX = e.clientX - this.startX;
      this.canvasY = e.clientY - this.startY + this.canvasScrollTop;
      this.isFocusing = true;
      this.onMouseMove();
    };

    this.canvas!.onmouseleave = (): void => {
      this.isFocusing = false;
      this.hideTip();
    };

    document.addEventListener('keydown', (e) => {
      if (!this.isFocusing) {
        return;
      }
      switch (e.key.toLocaleLowerCase()) {
        case 'w':
          this.scale(1);
          break;
        case 's':
          this.scale(-1);
          break;
        case 'a':
          this.translation(-1);
          break;
        case 'd':
          this.translation(1);
          break;
      }
    });
    this.listenerResize();
  }

  public initHtml(): string {
    return `
            <style>
            .frame-tip{
                position:absolute;
                left: 0;
                background-color: white;
                border: 1px solid #f9f9f9;
                width: auto;
                font-size: 8px;
                color: #50809e;
                padding: 2px 10px;
                display: none;
                max-width:400px;
            }
            .bold{
                font-weight: bold;
            }
            .text{
                max-width:350px;
                word-break: break-all;
            }
            :host{
                display: flex;
                padding: 10px 10px;
            }
            </style>
            <canvas id="canvas"></canvas>
            <div id ="float_hint" class="frame-tip"></div>`;
  }
}
