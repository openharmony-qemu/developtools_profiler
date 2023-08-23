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
import { FrameDynamicStruct } from '../../../../database/ui-worker/ProcedureWorkerFrameDynamic.js';
import { FrameAnimationSelect } from '../../../../bean/FrameComponentBean.js';
import { Utils } from '../../base/Utils.js';

@element('tabpane-frame-dynamic')
export class TabPaneFrameDynamic extends BaseElement {
  private frameDynamicTbl: LitTable | null | undefined;
  private range: HTMLLabelElement | null | undefined;
  private frameDynamicSource: Array<FrameAnimationSelect> = [];

  set data(frameDynamicParam: SelectionParam) {
    let secondToMillisecond: number = 1000_000.0;
    let fixedNumber: number = 5;
    this.range!.textContent = `Selected range:  ${parseFloat(((frameDynamicParam.rightNs -
      frameDynamicParam.leftNs) / secondToMillisecond).toFixed(fixedNumber))} ms`;
    this.buildDynamicTable(frameDynamicParam.frameDynamic);
  }

  buildDynamicTable(dynamicDataList: FrameDynamicStruct[], isClickSelect: boolean = false): void {
    if (isClickSelect) {
      this.range!.style.visibility = 'hidden';
    } else {
      this.range!.style.visibility = 'visible';
    }
    let result: FrameAnimationSelect[] = [];
    dynamicDataList.forEach(dynamic => {
      result.push({
        id: dynamic.id,
        value: dynamic.typeValue,
        timestamp: Utils.getTimeString(dynamic.ts)
      });
    });
    this.frameDynamicSource = result;
    this.frameDynamicTbl!.recycleDataSource = result;
  }

  initElements(): void {
    this.frameDynamicTbl = this.shadowRoot?.querySelector<LitTable>('#tb-frame-dynamic');
    this.range = this.shadowRoot?.querySelector('#frame-dynamic-time-range');
    this.frameDynamicTbl!.addEventListener('column-click', (evt) => {
      // @ts-ignore
      this.sortByColumn(evt.detail);
    });
  }

  connectedCallback(): void {
    super.connectedCallback();
    resizeObserver(this.parentElement!, this.frameDynamicTbl!);
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
        <label id="frame-dynamic-time-range" class="frame-dynamic-label" style="width: 100%;font-size: 10pt;
        margin-bottom: 5px;height: 20px;text-align: end;">Selected range:0.0 ms</label>
        <lit-table id="tb-frame-dynamic" style="height: auto">
            <lit-table-column class="frame-dynamic-column" title="Timestamp" width="1fr" 
            data-index="timestamp" key="timestamp"  align="flex-start" order >
            </lit-table-column>
            <lit-table-column class="frame-dynamic-column" title="Index" width="1fr" 
            data-index="id" key="id"  align="flex-start" order>
            </lit-table-column>
            <lit-table-column class="frame-dynamic-column" title="Value" width="1fr" 
            data-index="value" key="value"  align="flex-start" order >
            </lit-table-column>

        </lit-table>
        `;
  }

  private sortByColumn(framesDetail: {
    sort: number,
    key: string,
  }): void {
    let compare = function (property: string, sort: number, type: string) {
      return function (frameDynamicLeft: FrameAnimationSelect, frameDynamicRight: FrameAnimationSelect): number {
        let firstSortNumber: number = -1;
        let SecondSortNumber: number = 1;
        let thirdSortNumber: number = 2;
        // @ts-ignore
        let rightData = frameDynamicRight[property];
        // @ts-ignore
        let leftData = frameDynamicLeft[property];
        if (type === 'number') {
          return sort === thirdSortNumber ? parseFloat(rightData) - parseFloat(leftData) :
            parseFloat(leftData) - parseFloat(rightData);
        } else {
          if (rightData > leftData) {
            return sort === thirdSortNumber ? SecondSortNumber : firstSortNumber;
          } else {
            if (rightData === leftData) {
              return 0;
            } else {
              return sort === thirdSortNumber ? firstSortNumber : SecondSortNumber;
            }
          }
        }
      };
    };

    if (framesDetail.key === 'timestamp') {
      this.frameDynamicSource.sort(compare(framesDetail.key, framesDetail.sort, 'string'));
    } else {
      this.frameDynamicSource.sort(compare(framesDetail.key, framesDetail.sort, 'number'));
    }
    this.frameDynamicTbl!.recycleDataSource = this.frameDynamicSource;
  }
}
