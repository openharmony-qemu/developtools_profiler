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
import { SelectionParam } from '../../../../bean/BoxSelection.js';
import { getTabPaneFrequencySampleData } from '../../../../database/SqlLite.js';
import { LitProgressBar } from '../../../../../base-ui/progress-bar/LitProgressBar.js';
import { Utils } from '../../base/Utils.js';
import { ColorUtils } from '../../base/ColorUtils.js';
import { resizeObserver } from '../SheetUtils.js';

@element('tabpane-frequency-sample')
export class TabPaneFrequencySample extends BaseElement {
  private frequencySampleTbl: LitTable | null | undefined;
  private range: HTMLLabelElement | null | undefined;
  private loadDataInCache: boolean = true;
  private selectionParam: SelectionParam | null | undefined;
  private frequencyProgressEL: LitProgressBar | null | undefined;
  private frequencyLoadingPage: any;
  private frequencyLoadingList: number[] = [];
  private frequencySampleSource: any[] = [];
  private frequencySampleSortKey: string = 'counter';
  private frequencySampleSortType: number = 0;

  set data(frequencySampleValue: SelectionParam | any) {
    if (frequencySampleValue == this.selectionParam) {
      return;
    }
    this.frequencyProgressEL!.loading = true;
    this.frequencyLoadingPage.style.visibility = 'visible';
    this.selectionParam = frequencySampleValue;
    // @ts-ignore
    this.frequencySampleTbl!.shadowRoot?.querySelector('.table').style.height =
      this.parentElement!.clientHeight - 25 + 'px';
    this.queryDataByDB(frequencySampleValue);
  }

  initElements(): void {
    this.frequencyProgressEL = this.shadowRoot!.querySelector<LitProgressBar>('.progressFre');
    this.frequencyLoadingPage = this.shadowRoot!.querySelector('.loadingFre');
    this.frequencySampleTbl = this.shadowRoot!.querySelector<LitTable>('#tb-states');
    this.frequencySampleTbl!.addEventListener('column-click', (evt) => {
      // @ts-ignore
      this.frequencySampleSortKey = evt.detail.key;
      // @ts-ignore
      this.frequencySampleSortType = evt.detail.sort;
      // @ts-ignore
      this.sortTable(evt.detail.key, evt.detail.sort);
    });
  }

  connectedCallback() {
    super.connectedCallback();
    resizeObserver(this.parentElement!, this.frequencySampleTbl!, 25, this.frequencyLoadingPage, 24);
  }

  queryDataByDB(frqSampleParam: SelectionParam | any) {
    this.frequencyLoadingList.push(1);
    this.frequencyProgressEL!.loading = true;
    this.frequencyLoadingPage.style.visibility = 'visible';

    getTabPaneFrequencySampleData(
      frqSampleParam.leftNs + frqSampleParam.recordStartNs,
      frqSampleParam.rightNs + frqSampleParam.recordStartNs,
      frqSampleParam.cpuFreqFilterIds
    ).then((result) => {
      this.frequencyLoadingList.splice(0, 1);
      if (this.frequencyLoadingList.length == 0) {
        this.frequencyProgressEL!.loading = false;
        this.frequencyLoadingPage.style.visibility = 'hidden';
      }
      let sampleMap = new Map<any, any>();
      frqSampleParam.cpuFreqFilterIds.forEach((a: number) => {
        this.getInitTime(
          result.filter((f) => f.filterId == a),
          sampleMap,
          frqSampleParam
        );
      });

      let frqSampleList: Array<any> = [];
      sampleMap.forEach((a) => {
        a.timeStr = parseFloat((a.time / 1000000.0).toFixed(6));
        frqSampleList.push(a);
      });
      this.frequencySampleSource = frqSampleList;
      this.sortTable(this.frequencySampleSortKey, this.frequencySampleSortType);
    });
  }

  getInitTime(initFreqResult: Array<any>, sampleMap: Map<any, any>, selectionParam: SelectionParam) {
    let leftStartNs = selectionParam.leftNs + selectionParam.recordStartNs;
    let rightEndNs = selectionParam.rightNs + selectionParam.recordStartNs;
    if (initFreqResult.length == 0) return;
    let includeData = initFreqResult.findIndex((a) => a.ts >= leftStartNs);
    if (includeData !== 0) {
      initFreqResult = initFreqResult.slice(
        includeData == -1 ? initFreqResult.length - 1 : includeData - 1,
        initFreqResult.length
      );
    }
    if (initFreqResult[0].ts < leftStartNs && includeData !== 0) initFreqResult[0].ts = leftStartNs;
    initFreqResult.forEach((item, idx) => {
      if (idx + 1 == initFreqResult.length) {
        item.time = rightEndNs - item.ts;
      } else {
        item.time = initFreqResult[idx + 1].ts - item.ts;
      }
      if (sampleMap.has(item.filterId + '-' + item.value)) {
        let obj = sampleMap.get(item.filterId + '-' + item.value);
        obj.time += item.time;
      } else {
        sampleMap.set(item.filterId + '-' + item.value, {
          ...item,
          counter: 'Cpu ' + item.cpu,
          valueStr: ColorUtils.formatNumberComma(item.value),
        });
      }
    });
  }

  sortTable(key: string, type: number) {
    if (type == 0) {
      this.frequencySampleTbl!.recycleDataSource = this.frequencySampleSource;
    } else {
      let arr = Array.from(this.frequencySampleSource);
      arr.sort((frequencySampleLeftData, frequencySampleRightData): number => {
        if (key == 'timeStr') {
          if (type == 1) {
            return frequencySampleLeftData.time - frequencySampleRightData.time;
          } else {
            return frequencySampleRightData.time - frequencySampleLeftData.time;
          }
        } else if (key == 'counter') {
          if (frequencySampleLeftData.counter > frequencySampleRightData.counter) {
            return type === 2 ? -1 : 1;
          } else if (frequencySampleLeftData.counter == frequencySampleRightData.counter) {
            return 0;
          } else {
            return type === 2 ? 1 : -1;
          }
        } else if (key == 'valueStr') {
          if (type == 1) {
            return frequencySampleLeftData.value - frequencySampleRightData.value;
          } else {
            return frequencySampleRightData.value - frequencySampleLeftData.value;
          }
        } else {
          return 0;
        }
      });
      this.frequencySampleTbl!.recycleDataSource = arr;
    }
  }

  initHtml(): string {
    return `
        <style>
        .progressFre{
            bottom: 5px;
            position: absolute;
            height: 1px;
            left: 0;
            right: 0;
        }
        :host{
            padding: 10px 10px;
            display: flex;
            flex-direction: column;
        }
        .loadingFre{
            bottom: 0;
            position: absolute;
            left: 0;
            right: 0;
            width:100%;
            background:transparent;
            z-index: 999999;
        }
        </style>
        <lit-table id="tb-states" style="height: auto" >
            <lit-table-column class="freq-sample-column" width="20%" title="Cpu" data-index="counter" key="counter" align="flex-start" order>
            </lit-table-column>
            <lit-table-column class="freq-sample-column" width="1fr" title="Time(ms)" data-index="timeStr" key="timeStr" align="flex-start" order>
            </lit-table-column>
            <lit-table-column class="freq-sample-column" width="1fr" title="Value(kHz)" data-index="valueStr" key="valueStr" align="flex-start" order>
            </lit-table-column>
        </lit-table>
        <lit-progress-bar class="progressFre"></lit-progress-bar>
        <div class="loadingFre"></div>
        `;
  }
}
