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
import { SelectionParam } from '../../../../bean/BoxSelection';
import { queryVmTrackerShmSizeData } from '../../../../database/SqlLite.js';
import { Utils } from '../../base/Utils.js';
import { MemoryConfig } from '../../../../bean/MemoryConfig.js';
import { ns2s } from '../../../../database/ui-worker/ProcedureWorkerCommon.js';

@element('tabpane-vmtracker-shm')
export class TabPaneVmTrackerShm extends BaseElement {
  private TableEl: LitTable | undefined | null;
  private shmData: Array<number | string> = [];
  private sortArray: Array<any> = [];
  private memoryConfig: MemoryConfig = MemoryConfig.getInstance();
  private tabTitle: HTMLDivElement | undefined | null;
  private range: HTMLLabelElement | null | undefined;

  set data(valVmTrackerShm: SelectionParam | any) {
    if (valVmTrackerShm.vmtrackershm.length > 0) {
      this.init();
      this.clear();
      this.range!.textContent =
          'Selected range: ' +
          parseFloat(((valVmTrackerShm.rightNs - valVmTrackerShm.leftNs) / 1000000.0).toFixed(5)) +
          '  ms';
      this.queryDataByDB(valVmTrackerShm);
    }
  }

  initElements(): void {
    this.TableEl = this.shadowRoot!.querySelector<LitTable>('#tb-shm') as LitTable;
    this.range = this.shadowRoot?.querySelector('#time-range');
    this.tabTitle = this.TableEl!.shadowRoot?.querySelector('.thead') as HTMLDivElement;
    this.TableEl!.addEventListener('column-click', (evt) => {
      // @ts-ignore
      this.sortByColumn(evt.detail.key, evt.detail.sort);
    });
  }

  queryDataByDB(srVal: SelectionParam | any): void {
    queryVmTrackerShmSizeData(
      srVal.leftNs,
      srVal.rightNs,
      this.memoryConfig.iPid,
      (MemoryConfig.getInstance().interval * 1000_000) / 5
    ).then((result) => {
      if (result.length > 0) {
        for (let filter of result) {
          filter.time = ns2s(filter.startNS);
          filter.sumSizeStr = Utils.getBinaryByteWithUnit(filter.sum);
          filter.avgSizeStr = Utils.getBinaryByteWithUnit(filter.avg);
          filter.minSizeStr = Utils.getBinaryByteWithUnit(filter.min);
          filter.maxSizeStr = Utils.getBinaryByteWithUnit(filter.max);
        }
        this.shmData = result.sort((a, b) => b.avg - a.avg);
        this.TableEl!.recycleDataSource = this.shmData;
      }
    });
  }

  clear(): void {
    this.TableEl!.recycleDataSource = [];
  }

  private init(): void {
    const thTable = this.tabTitle!.querySelector('.th');
    const vmTrackerShmTblNodes = thTable!.querySelectorAll('div');
    if (this.tabTitle!.hasAttribute('sort')) {
      this.tabTitle!.removeAttribute('sort');
      vmTrackerShmTblNodes.forEach((item) => {
        item.querySelectorAll('svg').forEach((svg) => {
          svg.style.display = 'none';
        });
      });
    }
  }

  sortByColumn(column: string, sort: number): void {
    switch (sort) {
      case 0:
        this.TableEl!.snapshotDataSource = this.shmData;
        break;
      default:
        this.sortArray = [...this.shmData];
        switch (column) {
          case 'time':
            this.TableEl!.snapshotDataSource = this.sortArray.sort((leftStartNS, rightStartNS) => {
              return sort === 1
                ? leftStartNS.startNS - rightStartNS.startNS
                : rightStartNS.startNS - leftStartNS.startNS;
            });
            break;
          case 'flag':
            this.TableEl!.snapshotDataSource = this.sortArray.sort((leftFlag, rightFlag) => {
              return sort === 1 ? leftFlag.flag - rightFlag.flag : rightFlag.flag - leftFlag.flag;
            });
            break;
          case 'minSizeStr':
            this.TableEl!.snapshotDataSource = this.sortArray.sort((leftMin, rightMin) => {
              return sort === 1 ? leftMin.min - rightMin.min : rightMin.min - leftMin.min;
            });
            break;
          case 'avgSizeStr':
            this.TableEl!.snapshotDataSource = this.sortArray.sort((leftAvg, rightAvg) => {
              return sort === 1 ? leftAvg.avg - rightAvg.avg : rightAvg.avg - leftAvg.avg;
            });
            break;
          case 'maxSizeStr':
            this.TableEl!.snapshotDataSource = this.sortArray.sort((leftMax, rightMax) => {
              return sort === 1 ? leftMax.max - rightMax.max : rightMax.max - leftMax.max;
            });
            break;
        }
        break;
    }
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
        tab-pane-filter {
            border: solid rgb(216,216,216) 1px;
            float: left;
            position: fixed;
            bottom: 0;
            width: 100%;
        }
        selector{
            display: none;
        }
        .show{
            display: flex;
            flex: 1;
        }
        .progress{
            bottom: 33px;
            position: absolute;
            height: 1px;
            left: 0;
            right: 0;
        }
        .loading{
            bottom: 0;
            position: absolute;
            left: 0;
            right: 0;
            width:100%;
            background:transparent;
            z-index: 999999;
        }
        </style>
         <label id="time-range" style="width: 100%;height: 20px;text-align: end;font-size: 10pt;margin-bottom: 5px">Selected range:0.0 ms</label>
                        <lit-table id="tb-shm" style="height: auto">
                            <lit-table-column width="1fr" title="AvgSize" data-index="avgSizeStr" key="avgSizeStr" align="flex-start"  order>
                            </lit-table-column>
                            <lit-table-column width="1fr" title="MinSize" data-index="minSizeStr" key="minSizeStr" align="flex-start" order>
                            </lit-table-column>
                            <lit-table-column width="1fr" title="MaxSize" data-index="maxSizeStr" key="maxSizeStr" align="flex-start"  order>
                            </lit-table-column>
                        </lit-table>
        `;
  }
}
