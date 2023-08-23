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

import { TraceRow } from '../trace/base/TraceRow.js';
import { renders } from '../../database/ui-worker/ProcedureWorker.js';
import { JankRender, JankStruct } from '../../database/ui-worker/ProcedureWorkerJank.js';
import { SpSystemTrace } from '../SpSystemTrace.js';
import {
  queryActualFrameDate,
  queryExpectedFrameDate,
  queryFrameAnimationData, queryFrameApp, queryFrameDynamicData, queryFrameSpacing,
  queryFrameTimeData, queryPhysicalData
} from '../../database/SqlLite.js';
import { JanksStruct } from '../../bean/JanksStruct.js';
import { ns2xByTimeShaft, PairPoint } from '../../database/ui-worker/ProcedureWorkerCommon.js';
import { LitPopover } from '../../../base-ui/popover/LitPopoverV.js';
import { FrameDynamicRender, FrameDynamicStruct } from '../../database/ui-worker/ProcedureWorkerFrameDynamic.js';
import { FrameAnimationRender, FrameAnimationStruct } from '../../database/ui-worker/ProcedureWorkerFrameAnimation.js';
import { BaseStruct } from '../../bean/BaseStruct.js';
import { FrameSpacingRender, FrameSpacingStruct } from '../../database/ui-worker/ProcedureWorkerFrameSpacing.js';
import { FlagsConfig, Params } from '../SpFlags.js';
import { AnimationRanges, DeviceStruct } from '../../bean/FrameComponentBean.js';

export class SpFrameTimeChart {
  private trace: SpSystemTrace;
  private flagConfig: Params | undefined;

  constructor(trace: SpSystemTrace) {
    this.trace = trace;
  }

  async init(): Promise<void> {
    let frameTimeData = await queryFrameTimeData();
    if (frameTimeData.length > 0) {
      let frameTimeLineRow: TraceRow<JanksStruct> = await this.initFrameTimeLine();
      await this.initExpectedChart(frameTimeLineRow);
      await this.initActualChart(frameTimeLineRow);
    }
  }

  async initFrameTimeLine(): Promise<TraceRow<JanksStruct>> {
    let frameTimeLineRow: TraceRow<JanksStruct> = TraceRow.skeleton<JanksStruct>();
    frameTimeLineRow.rowId = 'frameTime';
    frameTimeLineRow.rowType = TraceRow.ROW_TYPE_JANK;
    frameTimeLineRow.rowParentId = '';
    frameTimeLineRow.style.width = '100%';
    frameTimeLineRow.style.height = '40px';
    frameTimeLineRow.folder = true;
    frameTimeLineRow.name = 'FrameTimeline';
    frameTimeLineRow.setAttribute('children', '');
    frameTimeLineRow.supplier = (): Promise<JanksStruct[]> =>
      new Promise((resolve) => {
        resolve([]);
      });
    frameTimeLineRow.favoriteChangeHandler = this.trace.favoriteChangeHandler;
    frameTimeLineRow.selectChangeHandler = this.trace.selectChangeHandler;
    frameTimeLineRow.onThreadHandler = (useCache: boolean): void => {
      let context: CanvasRenderingContext2D = frameTimeLineRow!.collect ?
        this.trace.canvasFavoritePanelCtx! : this.trace.canvasPanelCtx!;
      frameTimeLineRow!.canvasSave(context);
      (renders.jank as JankRender).renderMainThread(
        {
          context: context,
          useCache: useCache,
          type: 'expected_frame_timeline_slice'
        },
        frameTimeLineRow!
      );
      frameTimeLineRow!.canvasRestore(context);
    };
    this.trace.rowsEL?.appendChild(frameTimeLineRow);
    return frameTimeLineRow;
  }

  async initExpectedChart(frameTimeLineRow: TraceRow<JanksStruct>): Promise<void> {
    let frameExpectedData = await this.getExpectedFrameDate();
    let unitIndex: number = 1;
    let unitHeight: number = 20;
    let max: number = Math.max(...frameExpectedData.map((it) => it.depth || 0)) + unitIndex;
    let maxHeight: number = max * unitHeight;
    let expectedTimeLineRow = TraceRow.skeleton<JanksStruct>();
    expectedTimeLineRow.rowId = 'expected frameTime';
    expectedTimeLineRow.rowType = TraceRow.ROW_TYPE_JANK;
    expectedTimeLineRow.rowHidden = !frameTimeLineRow.expansion;
    expectedTimeLineRow.rowParentId = 'frameTime';
    expectedTimeLineRow.style.width = '100%';
    expectedTimeLineRow.style.height = '40px';
    expectedTimeLineRow.style.height = `${maxHeight}px`;
    expectedTimeLineRow.name = 'Expected Timeline';
    expectedTimeLineRow.addTemplateTypes('FrameTimeline');
    expectedTimeLineRow.setAttribute('height', `${maxHeight}`);
    expectedTimeLineRow.setAttribute('children', '');
    expectedTimeLineRow.supplier = (): Promise<JanksStruct[]> =>
      new Promise((resolve): void => {
        resolve(frameExpectedData);
      });
    expectedTimeLineRow.favoriteChangeHandler = this.trace.favoriteChangeHandler;
    expectedTimeLineRow.selectChangeHandler = this.trace.selectChangeHandler;
    expectedTimeLineRow.onThreadHandler = (useCache: boolean): void => {
      let context: CanvasRenderingContext2D = expectedTimeLineRow!.collect ?
        this.trace.canvasFavoritePanelCtx! : this.trace.canvasPanelCtx!;
      expectedTimeLineRow!.canvasSave(context);
      (renders.jank as JankRender).renderMainThread(
        {
          context: context,
          useCache: useCache,
          type: 'expected_frame_timeline_slice'
        },
        expectedTimeLineRow!
      );
      expectedTimeLineRow!.canvasRestore(context);
    };
    frameTimeLineRow.addChildTraceRow(expectedTimeLineRow);
  }

  async initActualChart(frameTimeLineRow: TraceRow<JanksStruct>): Promise<void> {
    let frameActualData = await this.getActualFrameDate();
    let unitIndex: number = 1;
    let unitHeight: number = 20;
    let maxHeight: number = (Math.max(...frameActualData.map((it) => it.depth || 0)) + unitIndex) * unitHeight;
    let actualTimeLineRow = TraceRow.skeleton<JanksStruct>();
    actualTimeLineRow.rowId = 'actual frameTime';
    actualTimeLineRow.rowType = TraceRow.ROW_TYPE_JANK;
    actualTimeLineRow.rowHidden = !frameTimeLineRow.expansion;
    actualTimeLineRow.rowParentId = 'frameTime';
    actualTimeLineRow.style.width = '100%';
    actualTimeLineRow.style.height = `${maxHeight}px`;
    actualTimeLineRow.name = 'Actual Timeline';
    actualTimeLineRow.addTemplateTypes('FrameTimeline');
    actualTimeLineRow.setAttribute('height', `${maxHeight}`);
    actualTimeLineRow.setAttribute('children', '');
    actualTimeLineRow.dataList = frameActualData;
    actualTimeLineRow.supplier = (): Promise<JanksStruct[]> => new Promise((resolve): void => {
      resolve(frameActualData);
    });
    actualTimeLineRow.favoriteChangeHandler = this.trace.favoriteChangeHandler;
    actualTimeLineRow.selectChangeHandler = this.trace.selectChangeHandler;
    actualTimeLineRow.onThreadHandler = (useCache: boolean): void => {
      let context: CanvasRenderingContext2D = actualTimeLineRow!.collect ? this.trace.canvasFavoritePanelCtx! :
        this.trace.canvasPanelCtx!;
      actualTimeLineRow!.canvasSave(context);
      (renders.jank as JankRender).renderMainThread(
        {
          context: context,
          useCache: useCache,
          type: 'expected_frame_timeline_slice'
        },
        actualTimeLineRow!
      );
      actualTimeLineRow!.canvasRestore(context);
    };
    frameTimeLineRow.addChildTraceRow(actualTimeLineRow);
    let offsetYTimeOut: number = 0;
    frameTimeLineRow.addEventListener('expansion-change', (customEventInit: CustomEventInit) => {
      JankStruct.delJankLineFlag = false;
      if (offsetYTimeOut) {
        clearTimeout(offsetYTimeOut);
      }
      if (customEventInit.detail?.expansion) {
        offsetYTimeOut = this.frameExpandTimeOut(customEventInit, actualTimeLineRow);
      } else {
        offsetYTimeOut = this.frameNoExpandTimeOut(customEventInit, frameTimeLineRow);
      }
    });
  }

  async initAnimatedScenesChart(
    processRow: TraceRow<BaseStruct>,
    process: {
      pid: number | null;
      processName: string | null;
    },
    firstRow: TraceRow<BaseStruct>
  ): Promise<void> {
    this.flagConfig = FlagsConfig.getFlagsConfig('AnimationAnalysis');
    if (this.flagConfig?.AnimationAnalysis === 'Enabled') {
      if (process.processName?.startsWith('render_service')) {
        let targetRowList = processRow.childrenList.filter(childRow =>
          childRow.rowType === 'thread' && childRow.name.startsWith('render_service')
        );
        let animationRanges = await this.initAnimationChart(processRow, firstRow);
        let nameArr = await queryFrameApp();
        let addRowList: Array<TraceRow<BaseStruct>> = [];
        for (let index = 0; index < nameArr.length; index++) {
          let name = nameArr[index].appName;
          let dynamicChart = await this.initDynamicCurveChart(processRow, name, animationRanges);
          let frameSpacingChart = await this.initFrameSpacing(processRow, name, animationRanges);
          addRowList.push(dynamicChart);
          addRowList.push(frameSpacingChart);
        }
        let resultRow = addRowList.reverse();
        resultRow.forEach(row => {
          processRow.addChildTraceRowAfter(row, targetRowList[0]);
        });
      }
    }
  }

  async initAnimationChart(
    processRow: TraceRow<BaseStruct>,
    firstRow: TraceRow<BaseStruct>
  ): Promise<AnimationRanges[]> {
    let frameAnimationData: FrameAnimationStruct[] = await queryFrameAnimationData();
    let animationRanges: AnimationRanges[] = [];
    if (frameAnimationData.length > 0) {
      frameAnimationData.forEach(data => {
        let range = {
          start: data.dynamicStartTs,
          end: data.dynamicEndTs
        };
        animationRanges.push(range);
      });
    }
    let frameAnimationRow = TraceRow.skeleton<FrameAnimationStruct>();
    frameAnimationRow.rowId = 'Animation';
    frameAnimationRow.rowType = TraceRow.ROW_TYPE_FRAME_ANIMATION;
    frameAnimationRow.rowHidden = !processRow.expansion;
    frameAnimationRow.rowParentId = processRow.rowId;
    frameAnimationRow.style.width = '100%';
    frameAnimationRow.style.height = '40px';
    frameAnimationRow.name = 'Animation';
    frameAnimationRow.addTemplateTypes('Animation Effect');
    frameAnimationRow.setAttribute('children', '');
    frameAnimationRow.supplier = (): Promise<FrameAnimationStruct[]> =>
      new Promise((resolve) => {
        resolve(frameAnimationData);
      });
    frameAnimationRow.favoriteChangeHandler = this.trace.favoriteChangeHandler;
    frameAnimationRow.selectChangeHandler = this.trace.selectChangeHandler;
    frameAnimationRow.onThreadHandler = (useCache): void => {
      let context: CanvasRenderingContext2D = frameAnimationRow!.collect ?
        this.trace.canvasFavoritePanelCtx! : this.trace.canvasPanelCtx!;
      frameAnimationRow!.canvasSave(context);
      (renders.frameAnimation as FrameAnimationRender).renderMainThread(
        {
          context: context,
          useCache: useCache,
          type: 'frameAnimation'
        },
        frameAnimationRow!
      );
      frameAnimationRow!.canvasRestore(context);
    };
    processRow.addChildTraceRowBefore(frameAnimationRow, firstRow);
    return animationRanges;
  }

  async initDynamicCurveChart(
    processRow: TraceRow<BaseStruct>,
    componentName: string,
    animationRanges: AnimationRanges[]
  ): Promise<TraceRow<FrameDynamicStruct>> {
    let frameDynamicCurveData: FrameDynamicStruct[] = await queryFrameDynamicData(componentName);
    let systemConfigList: string[] = [' '];
    let dynamicCurveRow: TraceRow<FrameDynamicStruct> = TraceRow.skeleton<FrameDynamicStruct>();
    let systemPopover = this.addSystemConfigButton(dynamicCurveRow, systemConfigList);
    dynamicCurveRow.rowId = `animation Effect Curve-${componentName}`;
    dynamicCurveRow.rowType = TraceRow.ROW_TYPE_FRAME_DYNAMIC;
    dynamicCurveRow.rowHidden = !processRow.expansion;
    dynamicCurveRow.rowParentId = processRow.rowId;
    dynamicCurveRow.style.width = '100%';
    dynamicCurveRow.style.height = '40px';
    dynamicCurveRow.style.height = '100px';
    let labelName = dynamicCurveRow.shadowRoot?.querySelector('.name') as HTMLLabelElement;
    labelName.style.marginRight = '77px';
    dynamicCurveRow.name = `Animation Effect Curve (${componentName})`;
    dynamicCurveRow.addTemplateTypes('Animation Effect');
    dynamicCurveRow.setAttribute('height', '100px');
    dynamicCurveRow.setAttribute('children', '');
    dynamicCurveRow.supplier = (): Promise<FrameDynamicStruct[]> =>
      new Promise((resolve): void => {
        resolve(frameDynamicCurveData);
      });
    dynamicCurveRow.favoriteChangeHandler = () : void => {
      this.favoriteSelect(systemPopover, dynamicCurveRow);
    };
    dynamicCurveRow.selectChangeHandler = this.trace.selectChangeHandler;
    dynamicCurveRow.onThreadHandler = (useCache: boolean): void => {
      let context: CanvasRenderingContext2D = dynamicCurveRow!.collect ? this.trace.canvasFavoritePanelCtx! :
        this.trace.canvasPanelCtx!;
      dynamicCurveRow!.canvasSave(context);
      (renders.frameDynamicCurve as FrameDynamicRender).renderMainThread(
        {
          context: context,
          useCache: useCache,
          type: 'dynamicEffectCurve',
          animationRanges: animationRanges
        },
        dynamicCurveRow!
      );
      dynamicCurveRow!.canvasRestore(context);
    };
    return dynamicCurveRow;
  }

  private favoriteSelect(systemPopover: LitPopover, dynamicCurveRow: TraceRow<FrameDynamicStruct>) : void {
    let popover = systemPopover.querySelector('.dynamicPopover') as HTMLDivElement;
    if (dynamicCurveRow.collect) {
      systemPopover.setAttribute('placement', 'right');
      popover.style.display = 'flex';
    } else {
      systemPopover.setAttribute('placement', 'bottomLeft');
      popover.style.display = 'block';
    }
  }

  async initFrameSpacing(
    processRow: TraceRow<BaseStruct>,
    componentName: string,
    animationRanges: AnimationRanges[]
  ): Promise<TraceRow<FrameSpacingStruct>> {
    let frameData: FrameSpacingStruct[] = await queryFrameSpacing(componentName);
    let deviceStructArray = await queryPhysicalData();
    let deviceStruct: DeviceStruct = deviceStructArray[0];
    this.dataProcessing(frameData, deviceStruct);
    let frameSpacingRow = TraceRow.skeleton<FrameSpacingStruct>();
    frameSpacingRow.rowId = `frame spacing-${componentName}`;
    frameSpacingRow.rowType = TraceRow.ROW_TYPE_FRAME_SPACING;
    frameSpacingRow.rowHidden = !processRow.expansion;
    frameSpacingRow.rowParentId = processRow.rowId;
    frameSpacingRow.style.width = '100%';
    frameSpacingRow.style.height = '140px';
    frameSpacingRow.name = `Frame spacing (${componentName})`;
    frameSpacingRow.addTemplateTypes('Animation Effect');
    frameSpacingRow.setAttribute('height', '140');
    frameSpacingRow.setAttribute('children', '');
    frameSpacingRow.supplier = (): Promise<FrameSpacingStruct[]> =>
      new Promise((resolve): void => {
        resolve(frameData);
      });
    frameSpacingRow.favoriteChangeHandler = this.trace.favoriteChangeHandler;
    frameSpacingRow.selectChangeHandler = this.trace.selectChangeHandler;
    frameSpacingRow.onThreadHandler = (useCache: boolean): void => {
      let context = frameSpacingRow!.collect ?
        this.trace.canvasFavoritePanelCtx! : this.trace.canvasPanelCtx!;
      frameSpacingRow!.canvasSave(context);
      (renders.frameSpacing as FrameSpacingRender).renderMainThread(
        {
          context: context,
          useCache: useCache,
          type: 'frame_spacing_slice',
          frameRate: deviceStruct.physicalFrameRate,
          animationRanges: animationRanges
        },
        frameSpacingRow!
      );
      frameSpacingRow!.canvasRestore(context);
    };
    return frameSpacingRow;
  }

  dataProcessing(frameData: FrameSpacingStruct[], deviceStruct: DeviceStruct): void {
    let unitIndex: number = 1;
    let secondToNanosecond: number = 1000_000_000;
    let physicalWidth = Number(this.flagConfig!.physicalWidth);
    let physicalHeight = Number(this.flagConfig!.physicalHeight);
    FrameSpacingStruct.physicalWidth = physicalWidth !== 0 ? physicalWidth : deviceStruct.physicalWidth;
    FrameSpacingStruct.physicalHeight = physicalHeight !== 0 ? physicalHeight : deviceStruct.physicalHeight;
    for (let index = 0; index < frameData.length; index++) {
      if (index > 0) {
        let intervalTime = (frameData[index].currentTs - frameData[index - unitIndex].currentTs) / secondToNanosecond;
        let widthDifference = frameData[index].currentFrameWidth! - frameData[index - unitIndex].currentFrameWidth!;
        let heightDifference = frameData[index].currentFrameHeight! - frameData[index - unitIndex].currentFrameHeight!;
        let xDifference = frameData[index].x! - frameData[index - unitIndex].x!;
        let yDifference = frameData[index].y! - frameData[index - unitIndex].y!;
        let frameWidth = Math.abs(widthDifference / FrameSpacingStruct.physicalWidth / intervalTime);
        let frameHeight = Math.abs(heightDifference / FrameSpacingStruct.physicalHeight / intervalTime);
        let frameX = Math.abs(xDifference / FrameSpacingStruct.physicalHeight / intervalTime);
        let frameY = Math.abs(yDifference / FrameSpacingStruct.physicalHeight / intervalTime);
        let result = Math.max(frameWidth, frameHeight, frameX, frameY);
        frameData[index].frameSpacingResult = Number(result.toFixed(unitIndex));
        frameData[index].preTs = frameData[index - unitIndex].currentTs;
        frameData[index].preFrameWidth = frameData[index - unitIndex].currentFrameWidth;
        frameData[index].preFrameHeight = frameData[index - unitIndex].currentFrameHeight;
      } else {
        frameData[index].frameSpacingResult = 0;
        frameData[index].preTs = 0;
        frameData[index].preFrameWidth = 0;
        frameData[index].preFrameHeight = 0;
      }
    }
  }

  addSystemConfigButton(dynamicCurveRow: TraceRow<FrameDynamicStruct>, systemConfigList: Array<string>): LitPopover {
    let rowContent: HTMLDivElement = dynamicCurveRow.shadowRoot?.querySelector('.describe') as HTMLDivElement;
    let systemPopover: LitPopover = document.createElement('lit-popover') as LitPopover;
    systemPopover.style.zIndex = '100';
    systemPopover.style.position = 'absolute';
    systemPopover.style.left = '165px';
    systemPopover.style.display = 'flex';
    systemPopover.setAttribute('placement', 'bottomLeft');
    systemPopover.setAttribute('trigger', 'click');
    systemPopover.setAttribute('haveRadio', 'true');
    systemPopover.innerHTML = `
    <div style="display: block; overflow: auto" slot="content" class="dynamicPopover">
      ${systemConfigList.map((): string => `
              <div class="option" style="margin-bottom: 5px; color: black;">
                <input class="radio" name="status" type="radio" value="x" style="margin-right: 10px;" checked/>X
              </div>
              <div class="option" style="margin-bottom: 5px; color: black;">
                <input class="radio" name="status" type="radio" value="y" style="margin-right: 10px;"/>Y
              </div>
              <div class="option" style="margin-bottom: 5px; color: black;">
                <input class="radio" name="status" type="radio" value="width" style="margin-right: 10px;"/>Width
              </div>
              <div class="option" style="margin-bottom: 5px; color: black;">
                <input class="radio" name="status" type="radio" value="height" style="margin-right: 10px;"/>Height
              </div>
              <div class="option" style="margin-bottom: 5px; color: black;">
                <input class="radio" name="status" type="radio" value="alpha" style="margin-right: 10px;"/>Alpha
              </div>`).join('')}
    </div>
    <lit-icon name="setting" size="19" id="setting"></lit-icon>`;
    rowContent.appendChild(systemPopover);
    let radioList = systemPopover.querySelectorAll<HTMLInputElement>('.radio');
    let divElement = systemPopover.querySelectorAll<HTMLDivElement>('.option');
    divElement.forEach((divEl, index) => {
      divEl.addEventListener('click', () => {
        if (radioList[index]) {
          radioList[index].checked = true;
          dynamicCurveRow.setAttribute('modelType', `${radioList[index].value}`);
          systemPopover.blur();
          this.trace.refreshCanvas(false);
          this.trace.clickEmptyArea();
        }
      });
    });
    return systemPopover;
  }

  private frameNoExpandTimeOut(event: CustomEventInit<{
    expansion: boolean,
    rowType: string,
    rowId: string,
    rowParentId: string,
  }>, frameTimeLineRow: TraceRow<JanksStruct>): number {
    if (JankStruct!.selectJankStruct) {
      JankStruct.selectJankStructList?.push(<JankStruct>JankStruct!.selectJankStruct);
    }
    let topPadding: number = 195;
    let halfNumber: number = 2;
    let offsetYTime: number = 300;
    let refreshTime: number = 360;
    let offsetYTimeOut: number = window.setTimeout(() => {
      this.trace.linkNodes.forEach((linkNode: PairPoint[]) => {
        if (linkNode[0].rowEL.collect) {
          linkNode[0].rowEL.translateY = linkNode[0].rowEL.getBoundingClientRect().top - topPadding;
        } else {
          linkNode[0].rowEL.translateY = linkNode[0].rowEL.offsetTop - this.trace.rowsPaneEL!.scrollTop;
        }
        linkNode[0].y = linkNode[0].rowEL!.translateY! + linkNode[0].offsetY;
        if (linkNode[1].rowEL.collect) {
          linkNode[1].rowEL.translateY = linkNode[1].rowEL.getBoundingClientRect().top - topPadding;
        } else {
          linkNode[1].rowEL.translateY = linkNode[1].rowEL.offsetTop - this.trace.rowsPaneEL!.scrollTop;
        }
        linkNode[1].y = linkNode[1].rowEL!.translateY! + linkNode[1].offsetY;
        if (linkNode[0].rowEL.rowParentId === event.detail?.rowId) {
          linkNode[0].x = ns2xByTimeShaft(linkNode[0].ns, this.trace.timerShaftEL!);
          linkNode[0].y = frameTimeLineRow!.translateY! + linkNode[0].offsetY / halfNumber;
          linkNode[0].offsetY = linkNode[0].offsetY / halfNumber;
          linkNode[0].rowEL = frameTimeLineRow;
        } else if (linkNode[1].rowEL.rowParentId === event.detail?.rowId) {
          linkNode[1].x = ns2xByTimeShaft(linkNode[1].ns, this.trace.timerShaftEL!);
          linkNode[1].y = frameTimeLineRow!.translateY! + linkNode[1].offsetY / halfNumber;
          linkNode[1].offsetY = linkNode[1].offsetY / halfNumber;
          linkNode[1].rowEL = frameTimeLineRow!;
        }
      });
    }, offsetYTime);
    let refreshTimeOut: number = window.setTimeout(() => {
      this.trace.refreshCanvas(true);
      clearTimeout(refreshTimeOut);
    }, refreshTime);
    return offsetYTimeOut;
  }

  private frameExpandTimeOut(event: CustomEventInit<{
    expansion: boolean,
    rowType: string,
    rowId: string,
    rowParentId: string,
  }>, actualTimeLineRow: TraceRow<JanksStruct>): number {
    let topPadding: number = 195;
    let halfNumber: number = 2;
    let offsetYTime: number = 300;
    let refreshTime: number = 360;
    let offsetYTimeOut: number = window.setTimeout(() => {
      this.trace.linkNodes.forEach((linkFrameNode: PairPoint[]) => {
        JankStruct.selectJankStructList?.forEach((dat: JankStruct) => {
          if (event.detail?.rowId === dat.pid) {
            JankStruct.selectJankStruct = dat;
            JankStruct.hoverJankStruct = dat;
          }
        });
        if (linkFrameNode[0].rowEL.collect) {
          linkFrameNode[0].rowEL.translateY = linkFrameNode[0].rowEL.getBoundingClientRect().top - topPadding;
        } else {
          linkFrameNode[0].rowEL.translateY = linkFrameNode[0].rowEL.offsetTop - this.trace.rowsPaneEL!.scrollTop;
        }
        linkFrameNode[0].y = linkFrameNode[0].rowEL!.translateY! + linkFrameNode[0].offsetY;
        if (linkFrameNode[1].rowEL.collect) {
          linkFrameNode[1].rowEL.translateY = linkFrameNode[1].rowEL.getBoundingClientRect().top - topPadding;
        } else {
          linkFrameNode[1].rowEL.translateY = linkFrameNode[1].rowEL.offsetTop - this.trace.rowsPaneEL!.scrollTop;
        }
        linkFrameNode[1].y = linkFrameNode[1].rowEL!.translateY! + linkFrameNode[1].offsetY;
        if (linkFrameNode[0].rowEL.rowId === event.detail?.rowId) {
          linkFrameNode[0].x = ns2xByTimeShaft(linkFrameNode[0].ns, this.trace.timerShaftEL!);
          linkFrameNode[0].y = actualTimeLineRow!.translateY! + linkFrameNode[0].offsetY * halfNumber;
          linkFrameNode[0].offsetY = linkFrameNode[0].offsetY * halfNumber;
          linkFrameNode[0].rowEL = actualTimeLineRow;
        } else if (linkFrameNode[1].rowEL.rowId === event.detail?.rowId) {
          linkFrameNode[1].x = ns2xByTimeShaft(linkFrameNode[1].ns, this.trace.timerShaftEL!);
          linkFrameNode[1].y = actualTimeLineRow!.translateY! + linkFrameNode[1].offsetY * halfNumber;
          linkFrameNode[1].offsetY = linkFrameNode[1].offsetY * halfNumber;
          linkFrameNode[1].rowEL = actualTimeLineRow!;
        }
      });
    }, offsetYTime);
    let refreshTimeOut: number = window.setTimeout(() => {
      this.trace.refreshCanvas(true);
      clearTimeout(refreshTimeOut);
    }, refreshTime);
    return offsetYTimeOut;
  }

  private async getExpectedFrameDate(): Promise<JanksStruct[]> {
    let frameExpectedData = await queryExpectedFrameDate();
    if (frameExpectedData.length > 0) {
      let unitIndex: number = 1;
      let isIntersect = (a: JanksStruct, b: JanksStruct): boolean =>
        Math.max(a.ts! + a.dur!, b.ts! + b.dur!) - Math.min(a.ts!, b.ts!) < a.dur! + b.dur!;
      let depths = [];
      for (let i: number = 0; i < frameExpectedData.length; i++) {
        let it = frameExpectedData[i];
        if (!it.dur || it.dur < 0) {
          continue;
        }
        if (depths.length === 0) {
          it.depth = 0;
          depths[0] = it;
        } else {
          let index: number = 0;
          let isContinue: boolean = true;
          while (isContinue) {
            if (isIntersect(depths[index], it)) {
              if (depths[index + unitIndex] === undefined || !depths[index + unitIndex]) {
                it.depth = index + unitIndex;
                depths[index + unitIndex] = it;
                isContinue = false;
              }
            } else {
              it.depth = index;
              depths[index] = it;
              isContinue = false;
            }
            index++;
          }
        }
      }
    }
    return frameExpectedData;
  }

  private async getActualFrameDate(): Promise<JanksStruct[]> {
    let frameActualData = await queryActualFrameDate();
    if (frameActualData.length > 0) {
      let unitIndex: number = 1;
      let isIntersect = (leftStruct: JanksStruct, rightStruct: JanksStruct): boolean =>
        Math.max(leftStruct.ts! + leftStruct.dur!, rightStruct.ts! + rightStruct.dur!) -
        Math.min(leftStruct.ts!, rightStruct.ts!) < leftStruct.dur! + rightStruct.dur!;
      let depthArray = [];
      for (let index: number = 0; index < frameActualData.length; index++) {
        let it = frameActualData[index];
        if (!it.dur || it.dur < 0) {
          continue;
        }
        if (depthArray.length === 0) {
          it.depth = 0;
          depthArray[0] = it;
        } else {
          let index: number = 0;
          let isContinue: boolean = true;
          while (isContinue) {
            if (isIntersect(depthArray[index], it)) {
              if (depthArray[index + unitIndex] === undefined || !depthArray[index + unitIndex]) {
                it.depth = index + unitIndex;
                depthArray[index + unitIndex] = it;
                isContinue = false;
              }
            } else {
              it.depth = index;
              depthArray[index] = it;
              isContinue = false;
            }
            index++;
          }
        }
      }
    }
    return frameActualData;
  }

  private dynamicHoverTip(dynamicCurveRow: TraceRow<FrameDynamicStruct>): void {
    if (FrameDynamicStruct.hoverFrameDynamicStruct) {
      this.trace?.displayTip(
        dynamicCurveRow,
        FrameDynamicStruct.hoverFrameDynamicStruct,
        `<div style="width: 250px;z-index: 100;">
                   <div style="display: flex">
                      <div style="width: 80%;text-align: left">prev </div>
                      <div style="width: 20%;text-align: left"></div>
                   </div>
                   <div style="display: flex">
                      <div style="width: 70%;padding-left: 10%; text-align: left">ts: </div>
                      <div style="width: 20%;text-align: left">
        ${(FrameDynamicStruct.hoverFrameDynamicStruct.ts) || ''}
                      </div>
                   </div>
                   <div style="display: flex"><div style="width: 70%;padding-left: 10%; text-align: left">x: 
                   </div>
                   <div style="width: 20%;text-align: left">
        ${(FrameDynamicStruct.hoverFrameDynamicStruct.x) || ''}</div>
                   </div>
                   <div style="display: flex"><div style="width: 70%;padding-left: 10%; text-align: left">y: 
                   </div>
                   <div style="width: 20%;text-align: left">
        ${(FrameDynamicStruct.hoverFrameDynamicStruct.y) || ''}</div>
                   </div>
                   <div style="display: flex"><div style="width: 70%;padding-left: 10%; text-align: left">width: 
                   </div>
                   <div style="width: 20%;text-align: left">
        ${(FrameDynamicStruct.hoverFrameDynamicStruct.width) || ''}</div>
                   </div>
                   <div style="display: flex"><div style="width: 70%;padding-left: 10%; text-align: left">height: 
                   </div>
                   <div style="width: 20%;text-align: left">
        ${(FrameDynamicStruct.hoverFrameDynamicStruct.height) || ''}</div>
                   </div>
                   <div style="display: flex"><div style="width: 70%;padding-left: 10%; text-align: left">alpha: 
                   </div>
                </div>`
      );
    }
  }
}
