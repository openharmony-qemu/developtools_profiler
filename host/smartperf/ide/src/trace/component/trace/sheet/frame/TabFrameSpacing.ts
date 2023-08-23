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
import { FrameSpacingStruct } from '../../../../database/ui-worker/ProcedureWorkerFrameSpacing.js';
import { SelectionParam } from '../../../../bean/BoxSelection.js';
import { LitTable } from '../../../../../base-ui/table/lit-table.js';
import { resizeObserver } from '../SheetUtils.js';
import { Utils } from '../../base/Utils.js';

@element('tabpane-frames-spacing')
export class TabFrameSpacing extends BaseElement {
  private range: HTMLLabelElement | null | undefined;
  private framesTbl: LitTable | null | undefined;
  private framesSource: Array<FrameSpacingTableStruct> = [];

  set data(frameSpacingParam: SelectionParam) {
    let secondToMillisecond: number = 1000_000.0;
    let fixedNumber: number = 5;
    this.range!.textContent = `Selected range: ${parseFloat(((frameSpacingParam.rightNs - frameSpacingParam.leftNs) /
      secondToMillisecond).toFixed(fixedNumber))} ms`;
    let tableList: FrameSpacingTableStruct[] = [];
    for (let index = 0; index < frameSpacingParam.frameSpacing.length; index++) {
      this.constructTable(frameSpacingParam.frameSpacing[index], tableList);
    }
    this.framesSource = tableList;
    this.framesTbl!.recycleDataSource = tableList;
  }

  connectedCallback(): void {
    super.connectedCallback();
    resizeObserver(this.parentElement!, this.framesTbl!);
  }

  setFrameSpacingData(data: FrameSpacingStruct): void {
    let tableList: FrameSpacingTableStruct[] = [];
    this.range!.textContent = '';
    this.constructTable(data, tableList);
    this.framesSource = tableList;
    this.framesTbl!.recycleDataSource = tableList;
  }

  private constructTable(structValue: FrameSpacingStruct, tableList: FrameSpacingTableStruct[]): void {
    // @ts-ignore
    let startNS = window.recordStartNS;
    tableList.push(this.getSplitSpacingData(structValue, startNS, 'W'));
    tableList.push(this.getSplitSpacingData(structValue, startNS, 'H'));
  }

  getSplitSpacingData(structValue: FrameSpacingStruct, startNS: number, propertyStr: string): FrameSpacingTableStruct{
    let frameSpacing: FrameSpacingTableStruct = new FrameSpacingTableStruct();
    let secondToNanosecond: number = 1000_000_000;
    frameSpacing.index = structValue.id;
    frameSpacing.timestamp = Utils.getTimeString(Number(structValue.currentTs));
    frameSpacing.property = propertyStr;
    frameSpacing.value2 = propertyStr === 'W' ? structValue.currentFrameWidth! : structValue.currentFrameHeight!;
    frameSpacing.value1 = propertyStr === 'W' ? structValue.preFrameWidth! : structValue.preFrameHeight!;
    frameSpacing.screen = propertyStr === 'W' ? FrameSpacingStruct.physicalWidth : FrameSpacingStruct.physicalHeight;
    frameSpacing.currentTs = ((structValue.currentTs + startNS) / secondToNanosecond).toString();
    frameSpacing.preTs = (((structValue.preTs || 0) + startNS) / secondToNanosecond).toString();
    if (structValue.preTs === 0) {
      frameSpacing.preTs = '-';
    }
    frameSpacing.frameSpacingResult = structValue.frameSpacingResult;
    return frameSpacing;
  }

  initElements(): void {
    this.framesTbl = this.shadowRoot?.querySelector<LitTable>('#tb-frame-spacing');
    this.range = this.shadowRoot?.querySelector('#frames-spacing-time-range');
    this.framesTbl!.addEventListener('column-click', (evt) => {
      // @ts-ignore
      this.sortByColumn(evt.detail);
    });
  }

  sortByColumn(framesDetail: {
    sort: number,
    key: string,
  }): void {
    let compare = function (property: string, sort: number, type: string) {
      return function (
        frameSpacingLeftData: FrameSpacingTableStruct,
        frameSpacingRightData: FrameSpacingTableStruct
      ): number {
        let firstSortNumber: number = -1;
        let SecondSortNumber: number = 1;
        let thirdSortNumber: number = 2;
        // @ts-ignore
        let rightSpacingData = frameSpacingRightData[property];
        // @ts-ignore
        let leftSpacingData = frameSpacingLeftData[property];
        if (type === 'number') {
          return sort === thirdSortNumber ? parseFloat(rightSpacingData) - parseFloat(leftSpacingData) :
            parseFloat(leftSpacingData) - parseFloat(rightSpacingData);
        } else {
          if (rightSpacingData > leftSpacingData) {
            return sort === thirdSortNumber ? SecondSortNumber : firstSortNumber;
          } else {
            if (rightSpacingData === leftSpacingData) {
              return 0;
            } else {
              return sort === thirdSortNumber ? firstSortNumber : SecondSortNumber;
            }
          }
        }
      };
    };
    if (framesDetail.key === 'property') {
      this.framesSource.sort(compare(framesDetail.key, framesDetail.sort, 'string'));
    } else {
      this.framesSource.sort(compare(framesDetail.key, framesDetail.sort, 'number'));
    }
    this.framesTbl!.recycleDataSource = this.framesSource;
  }

  initHtml(): string {
    return `
        <style>
        :host{
            display: flex;
            flex-direction: column;
            padding: 10px 10px 0 10px;
            height: calc(100% - 10px - 31px);
        }
        .frames-label{
          height: 20px;
          text-align: end;
          width: 100%;
          font-size: 10pt;
          margin-bottom: 5px;
        }
        </style>
        <label id="frames-spacing-time-range" class="frames-label">Selected range:0.0 ms</label>
        <lit-table id="tb-frame-spacing" style="height: auto">
            <lit-table-column title="Timestamp" width="1fr" data-index="timestamp" key="timestamp" 
            align="flex-start" order>
            </lit-table-column>
            <lit-table-column title="Index" width="1fr" data-index="index" key="index"  align="flex-start" order>
            </lit-table-column>
            <lit-table-column title="property" width="1fr" data-index="property" key="property" 
            align="flex-start" order>
            </lit-table-column>
            <lit-table-column title="Value2" width="1fr" data-index="value2" key="value2" align="flex-start" order>
            </lit-table-column>
            <lit-table-column title="Value1" width="1fr" data-index="value1" key="value1" align="flex-start" order>
            </lit-table-column>
            <lit-table-column title="Screen" width="1fr" data-index="screen" key="screen" align="flex-start" order>
            </lit-table-column>
            <lit-table-column title="T2(s)" width="1fr" data-index="currentTs" key="currentTs" align="flex-start" order>
            </lit-table-column>
            <lit-table-column title="T1(s)" width="1fr" data-index="preTs" key="preTs" align="flex-start" order>
            </lit-table-column>
            <lit-table-column title="result" width="1fr" data-index="frameSpacingResult" 
            key="frameSpacingResult"  align="flex-start" order>
            </lit-table-column>
        </lit-table>
        `;
  }
}

export class FrameSpacingTableStruct {
  index: number | undefined;
  timestamp: string | undefined;
  property: string | undefined;
  value2: number | undefined;
  value1: number | undefined;
  screen: number | undefined;
  currentTs: string | undefined;
  preTs: string | undefined;
  frameSpacingResult: number | undefined;
}
