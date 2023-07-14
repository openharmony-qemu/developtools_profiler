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

const TAG: string = 'FrameChart';
const scaleHeight = 30;
const depthHeight = 20;
const filterPixel = 2;
const sideLength = 8;

class Module {
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
  private canvas: HTMLCanvasElement | undefined | null;
  private canvasContext: CanvasRenderingContext2D | undefined | null;
  private floatHint: HTMLDivElement | undefined | null;

  private rect: Rect = new Rect(0, 0, 0, 0);
  private _mode = ChartMode.Byte;
  private startX = 0; // canvas start x coord
  private startY = 0; // canvas start y coord
  private canvasX = -1; // canvas current x
  private canvasY = -1; // canvas current y
  private hintContent = ''; // float hint inner html content

  private historyList: Array<Array<ChartStruct>> = [];
  private currentSize = 0;
  private currentCount = 0;
  private currentDuration = 0;

  private currentData: Array<ChartStruct> = [];
  private xPoint = 0; // x in rect
  private isFocusing = false;
  private canvasScrollTop = 0;
  private _maxDepth = 0;
  private chartClickListenerList: Array<Function> = [];
  private isUpdateCanvas = false;

  static get observedAttributes() {
    return [];
  }

  /**
   * set chart mode
   * @param mode chart format for data mode
   */
  set mode(mode: ChartMode) {
    this._mode = mode;
  }

  set data(val: Array<ChartStruct> | any) {
    this.historyList = [];
    ChartStruct.lastSelectFuncStruct = undefined;
    this.currentData = val;
    this.resetTrans();
    this.calDrawArgs(true);
    for (let callback of this.chartClickListenerList) {
      callback(true);
    }
  }

  set tabPaneScrollTop(scrollTop: number) {
    this.canvasScrollTop = scrollTop;
    this.hideFloatHint();
  }

  /**
   * add callback of chart click
   * @param callback function of chart click
   */
  public addChartClickListener(callback: Function) {
    if (this.chartClickListenerList.indexOf(callback) < 0) {
      this.chartClickListenerList.push(callback);
    }
  }

  /**
   * remove callback of chart click
   * @param callback function of chart click
   */
  public removeChartClickListener(callback: Function) {
    let index = this.chartClickListenerList.indexOf(callback);
    if (index > -1) {
      this.chartClickListenerList.splice(index, 1);
    }
  }

  /**
   * cal total count size and max Depth
   * @param isCalRoot use depth 1 node to cal depth 0 node size/count/dur
   */
  private calDrawArgs(isCalRoot: boolean): void {
    this.currentCount = 0;
    this.currentSize = 0;
    this.currentDuration = 0;
    this._maxDepth = 0;
    for (let rootNode of this.currentData!) {
      let depth = 0;
      this.calMaxDepth(rootNode, depth, isCalRoot, true);
      this.currentCount += rootNode.drawCount || rootNode.count;
      this.currentSize += rootNode.drawSize || rootNode.size;
      this.currentDuration += rootNode.drawDur || rootNode.dur;
    }
    this.rect.width = this.canvas!.width;
    this.rect.height = (this._maxDepth + 1) * 20 + scaleHeight; // 20px/depth and 30 is scale height
    this.canvas!.style.height = this.rect!.height + 'px';
    this.canvas!.height = Math.ceil(this.rect!.height);
  }

  /**
   * cal max Depth
   * @param node every child node
   * @param depth current depth
   * @param isCalRoot use depth 1 node to cal depth 0 node size/count/dur
   */
  private calMaxDepth(node: ChartStruct, depth: number, isCalRoot: boolean, isCalDisplay: boolean): void {
    node.depth = depth;
    this.clearDisplayInfo(node);
    if (node.isSearch && isCalDisplay) {
      let module = new Module();
      module.count = node.drawCount = node.count;
      module.dur = node.drawDur = node.dur;
      module.size = node.drawSize = node.size;
      this.setParentDisplayInfo(node, module);
      isCalDisplay = false;
    }

    depth++;
    if (node.children && node.children.length > 0) {
      let parentSize, parentCount, parentDuration;
      parentSize = parentCount = parentDuration = 0;
      for (let children of node.children) {
        children.parent = node;
        if (node.depth == 0 && isCalRoot) {
          parentSize += children.size;
          parentCount += children.count;
          parentDuration += children.dur;
        }
        this.calMaxDepth(children, depth, isCalRoot, isCalDisplay);
      }
      if (node.depth == 0 && isCalRoot) {
        node.size = parentSize;
        node.count = parentCount;
        node.dur = parentDuration;
      }
    } else {
      this._maxDepth = Math.max(depth, this._maxDepth);
    }
  }

  private setParentDisplayInfo(node: ChartStruct, module: Module): void {
    if (node.parent) {
      node.parent.drawCount += module.count;
      node.parent.drawDur += module.dur;
      node.parent.drawSize += module.size;
      this.setParentDisplayInfo(node.parent, module);
    }
  }

  private clearDisplayInfo(node: ChartStruct): void {
    node.drawCount = 0;
    node.drawDur = 0;
    node.drawSize = 0;
  }

  /**
   * calculate Data and draw chart
   */
  public async calculateChartData() {
    this.clearCanvas();
    this.canvasContext?.beginPath();
    this.drawScale();
    let x = this.xPoint;
    switch (this._mode) {
      case ChartMode.Byte:
        for (let node of this.currentData!) {
          let nodeSize = node.drawSize || node.size;
          let width = Math.round((nodeSize / this.currentSize) * this.rect!.width);
          let nmHeight = depthHeight; // 20px / depth
          // ensure the data for first depth frame
          if (!node.frame) {
            node.frame = new Rect(x, scaleHeight, width, nmHeight);
          } else {
            node.frame!.x = x;
            node.frame!.y = scaleHeight;
            node.frame!.width = width;
            node.frame!.height = nmHeight;
          }
          // not draw when rect not in canvas
          if (x + width >= 0 && x < this.canvas!.width) {
            node.percent = nodeSize / this.currentSize;
            draw(this.canvasContext!, node);
          }
          this.setStructFuncFrame(node);
          this.drawFrameChart(node);
          x += width;
        }
        break;
      case ChartMode.Count:
        for (let node of this.currentData!) {
          let nodeCount = node.drawCount || node.count;
          let width = Math.round((nodeCount / this.currentCount) * this.rect!.width);
          let perfHeight = depthHeight; // 20px / depth
          // ensure the data for first depth frame
          if (!node.frame) {
            node.frame = new Rect(x, scaleHeight, width, perfHeight);
          } else {
            node.frame!.x = x;
            node.frame!.y = scaleHeight;
            node.frame!.width = width;
            node.frame!.height = perfHeight;
          }
          // not draw when rect not in canvas
          if (x + width >= 0 && x < this.canvas!.width) {
            node.percent = nodeCount / this.currentCount;
            draw(this.canvasContext!, node);
          }
          this.setStructFuncFrame(node);
          this.drawFrameChart(node);
          x += width;
        }
        break;
      case ChartMode.Duration:
        for (let node of this.currentData!) {
          let nodeDur = node.drawDur || node.dur;
          let width = Math.round((nodeDur / this.currentDuration) * this.rect!.width);
          let ebpfHeight = depthHeight; // 20px / depth
          // ensure the data for first depth frame
          if (!node.frame) {
            node.frame = new Rect(x, scaleHeight, width, ebpfHeight);
          } else {
            node.frame!.x = x;
            node.frame!.y = scaleHeight;
            node.frame!.width = width;
            node.frame!.height = ebpfHeight;
          }
          // not draw when rect not in canvas
          if (x + width >= 0 && x < this.canvas!.width) {
            node.percent = nodeDur / this.currentDuration;
            draw(this.canvasContext!, node);
          }
          this.setStructFuncFrame(node);
          this.drawFrameChart(node);
          x += width;
        }
        break;
    }
    this.drawTriangleOnScale();
    this.canvasContext?.closePath();
  }

  /**
   * draw last selected reset position on scale
   */
  private drawTriangleOnScale(): void {
    if (ChartStruct.lastSelectFuncStruct) {
      this.canvasContext!.fillStyle = `rgba(${82}, ${145}, ${255})`;
      let x = Math.ceil(ChartStruct.lastSelectFuncStruct.frame!.x + ChartStruct.lastSelectFuncStruct.frame!.width / 2);
      if (x < 0) x = sideLength / 2;
      if (x > this.canvas!.width) x = this.canvas!.width - sideLength;
      this.canvasContext!.moveTo(x - sideLength / 2, scaleHeight - sideLength);
      this.canvasContext!.lineTo(x + sideLength / 2, scaleHeight - sideLength);
      this.canvasContext!.lineTo(x, scaleHeight);
      this.canvasContext!.lineTo(x - sideLength / 2, scaleHeight - sideLength);
      this.canvasContext?.fill();
    }
  }

  /**
   * clear canvas all data
   */
  public clearCanvas(): void {
    this.canvasContext?.clearRect(0, 0, this.canvas!.width, this.canvas!.height);
  }

  /**
   * update canvas size
   */
  public updateCanvas(updateWidth: boolean, newWidth?: number): void {
    if (this.canvas instanceof HTMLCanvasElement) {
      this.canvas.style.width = 100 + '%';
      this.canvas.style.height = this.rect!.height + 'px';
      if (this.canvas.clientWidth == 0 && newWidth) {
        this.canvas.width = newWidth - 40;
      } else {
        this.canvas.width = this.canvas.clientWidth;
      }
      this.canvas.height = Math.ceil(this.rect!.height);
      this.updateCanvasCoord();
    }
    if (
      this.rect.width == 0 ||
      updateWidth ||
      Math.round(newWidth!) != this.canvas!.width + 40 ||
      newWidth! > this.rect.width
    ) {
      this.rect.width = this.canvas!.width;
    }
  }

  /**
   * updateCanvasCoord
   */
  private updateCanvasCoord(): void {
    if (this.canvas instanceof HTMLCanvasElement) {
      this.isUpdateCanvas = this.canvas.clientWidth != 0;
      if (this.canvas.getBoundingClientRect()) {
        let box = this.canvas.getBoundingClientRect();
        let D = document.documentElement;
        this.startX = box.left + Math.max(D.scrollLeft, document.body.scrollLeft) - D.clientLeft;
        this.startY = box.top + Math.max(D.scrollTop, document.body.scrollTop) - D.clientTop + this.canvasScrollTop;
      }
    }
  }

  /**
   * draw top Scale Into 100 piece
   */
  private drawScale(): void {
    let spApplication = <SpApplication>document.getElementsByTagName('sp-application')[0];
    // line
    this.canvasContext!.lineWidth = 0.5;
    this.canvasContext?.moveTo(0, 0);
    this.canvasContext?.lineTo(this.canvas!.width, 0);

    for (let i = 0; i <= 10; i++) {
      let startX = Math.floor((this.canvas!.width / 10) * i);
      for (let j = 0; j < 10; j++) {
        // children scale
        this.canvasContext!.lineWidth = 0.5;
        let startItemX = startX + Math.floor((this.canvas!.width / 100) * j);
        this.canvasContext?.moveTo(startItemX, 0);
        this.canvasContext?.lineTo(startItemX, 10);
      }
      if (i == 0) continue; // skip first Size is 0
      // long line every 10 count
      this.canvasContext!.lineWidth = 1;
      let sizeRatio = this.canvas!.width / this.rect.width; // scale ratio
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
      let scale = '';
      switch (this._mode) {
        case ChartMode.Byte:
          scale = Utils.getByteWithUnit(((this.currentSize * sizeRatio) / 10) * i);
          break;
        case ChartMode.Count:
          scale = Utils.timeMsFormat2p(
            (((this.currentCount * (SpHiPerf.stringResult?.fValue || 1)) * sizeRatio) / 10) * i
          );
          break;
        case ChartMode.Duration:
          scale = Utils.getProbablyTime(((this.currentDuration * sizeRatio) / 10) * i);
          break;
      }
      let size = this.canvasContext!.measureText(scale).width;
      this.canvasContext?.fillText(scale, startX - size - 5, depthHeight, 50); // 50 is Text max Length
      this.canvasContext?.stroke();
    }
  }

  private setStructFuncFrame(node: ChartStruct) {
    if (node.children && node.children.length > 0) {
      for (let children of node.children) {
        node.isDraw = false;
        children.parent = node;
        switch (this._mode) {
          case ChartMode.Byte:
            let childrenSize = children.drawSize || children.size;
            setFuncFrame(children, this.rect, this.currentSize, this._mode);
            children.percent = childrenSize / this.currentSize;
            break;
          case ChartMode.Count:
            let childrenCount = children.drawCount || children.count;
            setFuncFrame(children, this.rect, this.currentCount, this._mode);
            children.percent = childrenCount / this.currentCount;
            break;
          case ChartMode.Duration:
            let childrenDur = children.drawDur || children.dur;
            setFuncFrame(children, this.rect, this.currentDuration, this._mode);
            children.percent = childrenDur / this.currentDuration;
            break;
        }
        this.setStructFuncFrame(children);
      }
    }
  }

  /**
   * draw chart
   * @param node draw chart by every piece
   */
  private drawFrameChart(node: ChartStruct) {
    let effectChildList = [];
    let nodeSize = node.drawSize || node.size;
    let nodeCount = node.drawCount || node.count;
    let nodeDur = node.drawDur || node.dur;
    let ignoreSize, ignoreCount, ignoreDur;
    ignoreSize = ignoreCount = ignoreDur = 0;

    if (node.children && node.children.length > 0) {
      for (let children of node.children) {
        // not draw when rect not in canvas
        if (
          (children.frame!.x + children.frame!.width >= 0 && //less than canvas left
            children.frame!.x < this.canvas!.width && // more than canvas right
            children.frame!.width > filterPixel) || // filter px
          children.needShow
        ) {
          // click and back
          effectChildList.push(children);
        } else {
          ignoreSize += children.drawSize || children.size;
          ignoreCount += children.drawCount || children.count;
          ignoreDur += children.drawDur || children.dur;
        }
      }
      let x = node.frame!.x;
      if (effectChildList.length > 0) {
        for (let children of effectChildList) {
          children.frame!.x = x;
          switch (this._mode) {
            case ChartMode.Byte:
              let childSize = children.drawSize || children.size;
              children.frame!.width = (childSize / (nodeSize - ignoreSize)) * node.frame!.width;
              break;
            case ChartMode.Count:
              let childCount = children.drawCount || children.count;
              children.frame!.width = (childCount / (nodeCount - ignoreCount)) * node.frame!.width;
              break;
            case ChartMode.Duration:
              let childDur = children.drawDur || children.dur;
              children.frame!.width = (childDur / (nodeDur - ignoreDur)) * node.frame!.width;
              break;
          }
          x += children.frame!.width;
          draw(this.canvasContext!, children);
          this.drawFrameChart(children);
        }
      } else {
        let firstChildren = node.children[0];
        firstChildren.frame!.x = node.frame!.x;
        firstChildren.frame!.width = node.frame!.width;
        draw(this.canvasContext!, firstChildren);
        this.drawFrameChart(firstChildren);
      }
    }
  }

  /**
   * find target node from tree by mouse position
   *
   * @param nodes tree nodes
   * @param canvasX x coord of canvas
   * @param canvasY y coord of canvas
   * @returns target node
   */
  private searchData(nodes: Array<ChartStruct>, canvasX: number, canvasY: number): any {
    for (let node of nodes) {
      if (node.frame?.contains(canvasX, canvasY)) {
        return node;
      } else {
        let result = this.searchData(node.children, canvasX, canvasY);
        if (!result) continue; // if not found in this branch;search another branch
        return result;
      }
    }
    return null;
  }

  /**
   * show float hint and update position
   */
  private updateFloatHint(): void {
    this.floatHint!.innerHTML = this.hintContent;
    this.floatHint!.style.display = 'block';
    let x = this.canvasX;
    let y = this.canvasY - this.canvasScrollTop;
    //right rect hint show left
    if (this.canvasX + this.floatHint!.clientWidth > (this.canvas?.clientWidth || 0)) {
      x -= this.floatHint!.clientWidth - 1;
    } else {
      x += scaleHeight;
    }
    //bottom rect hint show top
    y -= this.floatHint!.clientHeight - 1;

    this.floatHint!.style.transform = `translate(${x}px,${y}px)`;
  }

  /**
   * redraw Chart while click to scale chart
   * @param selectData select Rect data as array
   */
  private redrawChart(selectData: Array<ChartStruct>): void {
    this.currentData = selectData;
    if (selectData.length == 0) return;
    this.calDrawArgs(false);
    this.calculateChartData();
  }

  /**
   * press w to zoom in, s to zoom out
   * @param index < 0 zoom out , > 0 zoom in
   */
  private scale(index: number): void {
    let newWidth = 0;
    // zoom in
    let deltaWidth = this.rect!.width * 0.2;
    if (index > 0) {
      newWidth = this.rect!.width + deltaWidth;
      // max scale
      let sizeRatio = this.canvas!.width / this.rect.width;
      switch (this._mode) {
        case ChartMode.Byte:
          //limit 10 byte
          if (Math.round((this.currentSize * sizeRatio) / 1.2) <= 10) {
            if (this.xPoint == 0) {
              return;
            }
            newWidth = this.canvas!.width / (10 / this.currentSize);
          }
          break;
        case ChartMode.Count:
          //limit 10 counts
          if (Math.round((this.currentCount * sizeRatio) / 1.2) <= 10) {
            if (this.xPoint == 0) {
              return;
            }
            newWidth = this.canvas!.width / (10 / this.currentCount);
          }
          break;
        case ChartMode.Duration:
          //limit 10ms
          if (Math.round((this.currentDuration * sizeRatio) / 1.2) <= 10_000_000) {
            if (this.xPoint == 0) {
              return;
            }
            newWidth = this.canvas!.width / (10_000_000 / this.currentDuration);
          }
          break;
      }
      deltaWidth = newWidth - this.rect!.width;
    } else {
      // zoom out
      newWidth = this.rect!.width - deltaWidth;
      // min scale
      if (newWidth < this.canvas!.width) {
        newWidth = this.canvas!.width;
        this.resetTrans();
      }
      deltaWidth = this.rect!.width - newWidth;
    }
    // width not change
    if (newWidth == this.rect.width) return;
    this.translationByScale(index, deltaWidth, newWidth);
  }

  private resetTrans() {
    this.xPoint = 0;
  }

  /**
   * translation after scale
   * @param index is zoom in
   * @param deltaWidth scale delta width
   * @param newWidth rect width after scale
   */
  private translationByScale(index: number, deltaWidth: number, newWidth: number): void {
    let translationValue = (deltaWidth * (this.canvasX - this.xPoint)) / this.rect.width;
    if (index > 0) {
      this.xPoint -= translationValue;
    } else {
      this.xPoint += translationValue;
    }
    this.rect!.width = newWidth;
    this.translationDraw();
  }

  /**
   * press a/d to translate rect
   * @param index left or right
   */
  private translation(index: number): void {
    let offset = this.canvas!.width / 10;
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
    this.calculateChartData();
  }

  /**
   * canvas click
   * @param e MouseEvent
   */
  private onMouseClick(e: MouseEvent): void {
    if (e.button == 0) {
      // mouse left button
      if (ChartStruct.hoverFuncStruct && ChartStruct.hoverFuncStruct != ChartStruct.selectFuncStruct) {
        this.drawDataSet(ChartStruct.lastSelectFuncStruct!, false);
        ChartStruct.lastSelectFuncStruct = undefined;
        ChartStruct.selectFuncStruct = ChartStruct.hoverFuncStruct;
        this.historyList.push(this.currentData!);
        let selectData = new Array<ChartStruct>();
        selectData.push(ChartStruct.selectFuncStruct!);
        // reset scale and translation
        this.rect.width = this.canvas!.clientWidth;
        this.resetTrans();
        this.redrawChart(selectData);
        for (let callback of this.chartClickListenerList) {
          callback(false);
        }
      }
    } else if (e.button == 2) {
      // mouse right button
      ChartStruct.selectFuncStruct = undefined;
      ChartStruct.hoverFuncStruct = undefined;
      if (this.currentData.length == 1 && this.historyList.length > 0) {
        ChartStruct.lastSelectFuncStruct = this.currentData[0];
        this.drawDataSet(ChartStruct.lastSelectFuncStruct, true);
      }
      if (this.historyList.length > 0) {
        // reset scale and translation
        this.rect.width = this.canvas!.clientWidth;
        this.resetTrans();
        this.redrawChart(this.historyList.pop()!);
      }
      if (this.historyList.length === 0) {
        for (let callback of this.chartClickListenerList) {
          callback(true);
        }
      }
    }
    this.hideFloatHint();
  }

  private hideFloatHint() {
    if (this.floatHint) {
      this.floatHint.style.display = 'none';
    }
  }

  /**
   * set current select rect parents will show
   * @param data current node
   * @param isShow is show in chart
   */
  private drawDataSet(data: ChartStruct, isShow: boolean): void {
    if (data) {
      data.needShow = isShow;
      if (data.parent) {
        this.drawDataSet(data.parent, isShow);
      }
    }
  }

  /**
   * mouse on canvas move event
   */
  private onMouseMove(): void {
    let lastNode = ChartStruct.hoverFuncStruct;
    let searchResult = this.searchData(this.currentData!, this.canvasX, this.canvasY);
    if (searchResult && (searchResult.isDraw || searchResult.needShow || searchResult.depth == 0)) {
      ChartStruct.hoverFuncStruct = searchResult;
      // judge current node is hover redraw chart
      if (searchResult != lastNode) {
        let name = ChartStruct.hoverFuncStruct?.symbol;
        switch (this._mode) {
          case ChartMode.Byte:
            let size = Utils.getByteWithUnit(
              ChartStruct.hoverFuncStruct!.drawSize || ChartStruct.hoverFuncStruct!.size
            );
            this.hintContent = `
                        <span class="bold">Symbol: </span> <span class="text">${name} </span> <br>
                        <span class="bold">Lib: </span> <span class="text">${ChartStruct.hoverFuncStruct?.lib}</span> <br>
                        <span class="bold">Addr: </span> <span>${ChartStruct.hoverFuncStruct?.addr}</span> <br>
                        <span class="bold">Size: </span> <span>${size}</span> <br>
                        <span class="bold">Count: </span> <span>${ChartStruct.hoverFuncStruct?.count}</span>`;
            break;
          case ChartMode.Count:
            let count = ChartStruct.hoverFuncStruct!.count;
            const dur = Utils.timeMsFormat2p(count * (SpHiPerf.stringResult?.fValue || 1));
            this.hintContent = `
                        <span class="bold">Name: </span> <span class="text">${name} </span> <br>
                        <span class="bold">Lib: </span> <span class="text">${ChartStruct.hoverFuncStruct?.lib}</span>
                        <br>
                        <span class="bold">Addr: </span> <span>${ChartStruct.hoverFuncStruct?.addr}</span> 
                        <br>
                        <span class="bold">Dur: </span> <span>${dur}</span> 
                        <br>
                        <span class="bold">Count: </span> <span> ${count}</span>`;
            break;
          case ChartMode.Duration:
            let duration = Utils.getProbablyTime(ChartStruct.hoverFuncStruct!.dur);
            this.hintContent = `
                        <span class="bold">Name: </span> <span class="text">${name} </span> <br>
                        <span class="bold">Lib: </span> <span class="text">${ChartStruct.hoverFuncStruct?.lib}</span>
                        <br>
                        <span class="bold">Addr: </span> <span>${ChartStruct.hoverFuncStruct?.addr}</span> <br>
                        <span class="bold">Duration: </span> <span>${duration}</span>`;
            break;
        }
        this.calculateChartData();
      }
      // prevent float hint trigger onmousemove event
      this.updateFloatHint();
    } else {
      this.hideFloatHint();
      ChartStruct.hoverFuncStruct = undefined;
    }
  }

  initElements(): void {
    this.canvas = this.shadowRoot?.querySelector('#canvas');
    this.canvasContext = this.canvas?.getContext('2d');
    this.floatHint = this.shadowRoot?.querySelector('#float_hint');

    this.canvas!.oncontextmenu = () => {
      return false;
    };
    this.canvas!.onmouseup = (e) => {
      this.onMouseClick(e);
    };

    this.canvas!.onmousemove = (e) => {
      if (!this.isUpdateCanvas) {
        this.updateCanvasCoord();
      }
      this.canvasX = e.clientX - this.startX;
      this.canvasY = e.clientY - this.startY + this.canvasScrollTop;
      this.isFocusing = true;
      this.onMouseMove();
    };

    this.canvas!.onmouseleave = () => {
      ChartStruct.selectFuncStruct = undefined;
      this.isFocusing = false;
      this.hideFloatHint();
    };

    document.addEventListener('keydown', (e) => {
      if (!this.isFocusing) return;
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
    new ResizeObserver((entries) => {
      if (this.canvas!.getBoundingClientRect()) {
        let box = this.canvas!.getBoundingClientRect();
        let element = document.documentElement;
        this.startX = box.left + Math.max(element.scrollLeft, document.body.scrollLeft) - element.clientLeft;
        this.startY =
          box.top + Math.max(element.scrollTop, document.body.scrollTop) - element.clientTop + this.canvasScrollTop;
      }
    }).observe(document.documentElement);
  }

  initHtml(): string {
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
