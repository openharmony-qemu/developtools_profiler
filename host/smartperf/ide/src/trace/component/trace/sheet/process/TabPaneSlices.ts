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
import { LitTable } from '../../../../../base-ui/table/lit-table.js';
import { SelectionData, SelectionParam } from '../../../../bean/BoxSelection.js';
import { getTabSlices, getTabSlicesAsyncFunc } from '../../../../database/SqlLite.js';
import { resizeObserver } from '../SheetUtils.js';

@element('tabpane-slices')
export class TabPaneSlices extends BaseElement {
  private slicesTbl: LitTable | null | undefined;
  private slicesRange: HTMLLabelElement | null | undefined;
  private slicesSource: Array<SelectionData> = [];
  private currentSelectionParam: SelectionParam | undefined;

  set data(slicesParam: SelectionParam | any) {
    if (this.currentSelectionParam === slicesParam) {
      return;
    }
    this.currentSelectionParam = slicesParam;
    this.slicesRange!.textContent =
      'Selected range: ' + parseFloat(((slicesParam.rightNs - slicesParam.leftNs) / 1000000.0).toFixed(5)) + ' ms';
    let asyncNames: Array<string> = [];
    let asyncPid: Array<number> = [];
    slicesParam.funAsync.forEach((it: any) => {
      asyncNames.push(it.name);
      asyncPid.push(it.pid);
    });
    this.slicesTbl!.loading = true;
    getTabSlicesAsyncFunc(asyncNames, asyncPid, slicesParam.leftNs, slicesParam.rightNs).then((res) => {
      getTabSlices(slicesParam.funTids, slicesParam.processIds, slicesParam.leftNs, slicesParam.rightNs).then((res2) => {
        this.slicesTbl!.loading = false;
        let processSlicesResult = (res || []).concat(res2 || []);
        if (processSlicesResult != null && processSlicesResult.length > 0) {
          let sumWall = 0.0;
          let sumOcc = 0;
          for (let processSliceItem of processSlicesResult) {
            processSliceItem.name = processSliceItem.name == null ? '' : processSliceItem.name;
            sumWall += processSliceItem.wallDuration;
            sumOcc += processSliceItem.occurrences;
            processSliceItem.wallDuration = parseFloat((processSliceItem.wallDuration / 1000000.0).toFixed(5));
            processSliceItem.avgDuration = parseFloat((processSliceItem.avgDuration / 1000000.0).toFixed(5));
          }
          let count = new SelectionData();
          count.process = ' ';
          count.wallDuration = parseFloat((sumWall / 1000000.0).toFixed(5));
          count.occurrences = sumOcc;
          processSlicesResult.splice(0, 0, count);
          this.slicesSource = processSlicesResult;
          this.slicesTbl!.recycleDataSource = processSlicesResult;
        } else {
          this.slicesSource = [];
          this.slicesTbl!.recycleDataSource = this.slicesSource;
        }
      });
    });
  }

  initElements(): void {
    this.slicesTbl = this.shadowRoot?.querySelector<LitTable>('#tb-slices');
    this.slicesRange = this.shadowRoot?.querySelector('#time-range');
    this.slicesTbl!.addEventListener('column-click', (evt) => {
      // @ts-ignore
      this.sortByColumn(evt.detail);
    });
  }

  connectedCallback() {
    super.connectedCallback();
    resizeObserver(this.parentElement!, this.slicesTbl!);
  }

  initHtml(): string {
    return `
        <style>
        .slice-label{
            height: 20px;
        }
        :host{
            display: flex;
            padding: 10px 10px;
            flex-direction: column;
        }
        </style>
        <label id="time-range" class="slice-label" style="width: 100%;text-align: end;font-size: 10pt;margin-bottom: 5px">Selected range:0.0 ms</label>
        <lit-table id="tb-slices" style="height: auto">
            <lit-table-column class="slices-column" title="Name" width="500px" data-index="name" key="name"  align="flex-start" order>
            </lit-table-column>
            <lit-table-column class="slices-column" title="Wall duration(ms)" width="1fr" data-index="wallDuration" key="wallDuration"  align="flex-start" order >
            </lit-table-column>
            <lit-table-column class="slices-column" title="Avg Wall duration(ms)" width="1fr" data-index="avgDuration" key="avgDuration"  align="flex-start" order >
            </lit-table-column>
            <lit-table-column class="slices-column" title="Occurrences" width="1fr" data-index="occurrences" key="occurrences"  align="flex-start" order >
            </lit-table-column>
        </lit-table>
        `;
  }

  sortByColumn(slicesDetail: any) {
    // @ts-ignore
    function compare(property, slicesSort, type) {
      return function (slicesLeftData: SelectionData, slicesRightData: SelectionData) {
        if (slicesLeftData.process == ' ' || slicesRightData.process == ' ') {
          return 0;
        }
        if (type === 'number') {
          // @ts-ignore
          return slicesSort === 2 ? parseFloat(slicesRightData[property]) - parseFloat(slicesLeftData[property]) : parseFloat(slicesLeftData[property]) - parseFloat(slicesRightData[property]);
        } else {
          // @ts-ignore
          if (slicesRightData[property] > slicesLeftData[property]) {
            return slicesSort === 2 ? 1 : -1;
          } else {
            // @ts-ignore
            if (slicesRightData[property] == slicesLeftData[property]) {
              return 0;
            } else {
              return slicesSort === 2 ? -1 : 1;
            }
          }
        }
      };
    }

    if (slicesDetail.key === 'name') {
      this.slicesSource.sort(compare(slicesDetail.key, slicesDetail.sort, 'string'));
    } else {
      this.slicesSource.sort(compare(slicesDetail.key, slicesDetail.sort, 'number'));
    }
    this.slicesTbl!.recycleDataSource = this.slicesSource;
  }
}
