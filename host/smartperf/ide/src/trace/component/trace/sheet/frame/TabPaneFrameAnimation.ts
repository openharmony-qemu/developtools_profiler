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
import { SelectionParam } from '../../../../bean/BoxSelection.js';
import { LitTable } from '../../../../../base-ui/table/lit-table.js';
import { resizeObserver } from '../SheetUtils.js';
import { FrameAnimationStruct } from '../../../../database/ui-worker/ProcedureWorkerFrameAnimation.js';

@element('tabpane-frames-animation')
export class TabPaneFrameAnimation extends BaseElement {
  private frameAnimationTbl: LitTable | null | undefined;
  private range: HTMLLabelElement | null | undefined;
  private frameAnimationSource: Array<FrameAnimationStruct> = [];

  set data(frameAnimationParam: SelectionParam) {
    let secondToMillisecond: number = 1000_000.0;
    let fixedNumber: number = 5;
    this.range!.textContent = `Selected range: ${parseFloat(((frameAnimationParam.rightNs -
      frameAnimationParam.leftNs) / secondToMillisecond).toFixed(fixedNumber))} ms`;
    this.queryDataByDB(frameAnimationParam);
  }

  queryDataByDB(frameAnimationParam: SelectionParam): void {
    let tableData: FrameAnimationStruct[] = [];
    this.frameAnimationSource = tableData;
    this.frameAnimationTbl!.recycleDataSource = tableData;
  }

  initElements(): void {
    this.frameAnimationTbl = this.shadowRoot?.querySelector<LitTable>('#tb-frame-animation');
    this.range = this.shadowRoot?.querySelector('#frame-animation-time-range');
    this.frameAnimationTbl!.addEventListener('column-click', (evt: CustomEventInit) => {
      this.sortByColumn(evt.detail);
    });
  }

  connectedCallback(): void {
    super.connectedCallback();
    resizeObserver(this.parentElement!, this.frameAnimationTbl!);
  }

  initHtml(): string {
    return `
        <style>
        .frames-label{
          height: 20px;
          text-align: end;
        }
        :host{
            padding: 10px 10px;
            display: flex;
            flex-direction: column;
        }
        </style>
        <label id="frame-animation-time-range" class="frame-animation-label" 
        style="width: 100%;font-size: 10pt;margin-bottom: 5px">Selected range:0.0 ms</label>
        <lit-table id="tb-frame-animation" style="height: auto">
            <lit-table-column class="frame-animation-column" title="Id" width="1fr" 
            data-index="id" key="id"  align="flex-start" order>
            </lit-table-column>
            <lit-table-column class="frame-animation-column" title="Input Time" width="1fr" 
            data-index="ts" key="ts"  align="flex-start" order >
            </lit-table-column>
            <lit-table-column class="frame-animation-column" title="Dynamic Start Time" width="1fr" 
            data-index="dynamicStartTs" key="dynamicStartTs"  align="flex-start" order >
            </lit-table-column>
            <lit-table-column class="frame-animation-column" title="Dynamic End Time" width="1fr" 
            data-index="dynamicEndTs" key="dynamicEndTs"  align="flex-start" order >
            </lit-table-column>
            <lit-table-column class="frame-animation-column" title="Dynamic Dur" width="1fr" 
            data-index="dur" key="dur"  align="flex-start" order >
            </lit-table-column>
        </lit-table>
        `;
  }

  private sortByColumn(framesDetail: {
    sort: number,
    key: string,
  }): void {
    let compare = function (dateKey: string, sort: number, type: string) {
      return function (leftAnimation: FrameAnimationStruct, rightAnimation: FrameAnimationStruct): number {
        let firstSortNumber: number = -1;
        let SecondSortNumber: number = 1;
        let thirdSortNumber: number = 2;
        // @ts-ignore
        let rightData = rightAnimation[dateKey];
        // @ts-ignore
        let leftData = leftAnimation[dateKey];
        if (type === 'number') {
          return sort === thirdSortNumber ? parseFloat(rightData) - parseFloat(leftData) :
            parseFloat(leftData) - parseFloat(rightData);
        } else {
          if (leftData > leftData) {
            return sort === thirdSortNumber ? SecondSortNumber : firstSortNumber;
          } else {
            if (leftData === leftData) {
              return 0;
            } else {
              return sort === thirdSortNumber ? firstSortNumber : SecondSortNumber;
            }
          }
        }
      };
    };

    if (framesDetail.key === 'animation') {
      this.frameAnimationSource.sort(compare(framesDetail.key, framesDetail.sort, 'string'));
    } else {
      this.frameAnimationSource.sort(compare(framesDetail.key, framesDetail.sort, 'number'));
    }
    this.frameAnimationTbl!.recycleDataSource = this.frameAnimationSource;
  }
}
