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
import { queryVmTrackerShmSelectionData } from '../../../../database/SqlLite.js';
import { SpSystemTrace } from '../../../SpSystemTrace.js';
import { Utils } from '../../base/Utils.js';
import { SnapshotStruct } from '../../../../database/ui-worker/ProcedureWorkerSnapshot';
import { MemoryConfig } from '../../../../bean/MemoryConfig.js';
import { ns2s } from '../../../../database/ui-worker/ProcedureWorkerCommon.js';

@element('tabpane-vmtracker-shm-selection')
export class TabPaneVmTrackerShmSelection extends BaseElement {
  private TableEl: LitTable | undefined | null;
  private shmData: Array<any> = [];
  private memoryConfig: MemoryConfig = MemoryConfig.getInstance();
  private tabTitle: HTMLDivElement | undefined | null;

  setShmData(data: SnapshotStruct, dataList: Array<SnapshotStruct>): void {
    this.init();
    this.clear();
    this.queryDataByDB(data);
  }

  initElements(): void {
    this.TableEl = this.shadowRoot!.querySelector<LitTable>('#tb-shm-selection') as LitTable;
    this.tabTitle = this.TableEl!.shadowRoot?.querySelector('.thead') as HTMLDivElement;
    this.TableEl!.addEventListener('column-click', (evt) => {
      // @ts-ignore
      this.sortByColumn(evt.detail.key, evt.detail.sort);
    });
  }

  queryDataByDB(data: SnapshotStruct): void {
    queryVmTrackerShmSelectionData(data.startNs, this.memoryConfig.iPid).then((result) => {
      if (result.length > 0) {
        for (let filter of result) {
          filter.name = SpSystemTrace.DATA_DICT.get(filter.name)?.split('/');
          filter.ts = ns2s(filter.startNS);
          filter.sizeStr = Utils.getBinaryByteWithUnit(filter.size);
          this.TableEl!.getItemTextColor = (filter): any => {
            if (filter.flag === 1) {
              return '#d4b550';
            } else if (filter.flag === 2) {
              return '#f86b6b';
            } else {
              return '#000000';
            }
          };
        }
        this.shmData = result.sort((a, b) => b.size - a.size);
        this.TableEl!.recycleDataSource = this.shmData;
      }
    });
  }

  clear(): void {
    this.TableEl!.recycleDataSource = [];
  }

  private init(): void {
    const thTable = this.tabTitle!.querySelector('.th');
    const list = thTable!.querySelectorAll('div');
    if (this.tabTitle!.hasAttribute('sort')) {
      this.tabTitle!.removeAttribute('sort');
      list.forEach((item) => {
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
        let arr = [...this.shmData];
        switch (column) {
          case 'ts':
            this.TableEl!.snapshotDataSource = arr.sort((leftTs, rightTs) => {
              return sort === 1 ? leftTs.startNS - rightTs.startNS : rightTs.startNS - leftTs.startNS;
            });
            break;
          case 'fd':
            this.TableEl!.snapshotDataSource = arr.sort((leftFd, rightFd) => {
              return sort === 1 ? leftFd.fd - rightFd.fd : rightFd.fd - leftFd.fd;
            });
            break;
          case 'sizeStr':
            this.TableEl!.snapshotDataSource = arr.sort((leftSize, rightSize) => {
              return sort === 1 ? leftSize.size - rightSize.size : rightSize.size - leftSize.size;
            });
            break;
          case 'adj':
            this.TableEl!.snapshotDataSource = arr.sort((leftAdj, rightAdj) => {
              return sort === 1 ? leftAdj.adj - rightAdj.adj : rightAdj.adj - leftAdj.adj;
            });
            break;
          case 'name':
            this.TableEl!.snapshotDataSource = arr.sort((leftName, rightName) => {
              if (sort === 1) {
                if (leftName.name > rightName.name) {
                  return 1;
                } else if (leftName.name === rightName.name) {
                  return 0;
                } else {
                  return -1;
                }
              } else {
                if (rightName.name > leftName.name) {
                  return 1;
                } else if (leftName.name === rightName.name) {
                  return 0;
                } else {
                  return -1;
                }
              }
            });
            break;
          case 'id':
            this.TableEl!.snapshotDataSource = arr.sort((leftId, rightId) => {
              return sort === 1 ? leftId.id - rightId.id : rightId.id - leftId.id;
            });
            break;
          case 'time':
            this.TableEl!.snapshotDataSource = arr.sort((leftTime, rightTime) => {
              return sort === 1 ? leftTime.time - rightTime.time : rightTime.time - leftTime.time;
            });
            break;
          case 'count':
            this.TableEl!.snapshotDataSource = arr.sort((leftCount, rightCount) => {
              return sort === 1 ? leftCount.count - rightCount.count : rightCount.count - leftCount.count;
            });
            break;
          case 'purged':
            this.TableEl!.snapshotDataSource = arr.sort((leftPurged, rightPurged) => {
              return sort === 1 ? leftPurged.purged - rightPurged.purged : rightPurged.purged - leftPurged.purged;
            });
            break;
          case 'flag':
            this.TableEl!.snapshotDataSource = arr.sort((leftFlag, rightFlag) => {
              return sort === 1 ? leftFlag.flag - rightFlag.flag : rightFlag.flag - leftFlag.flag;
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
                        <lit-table id="tb-shm-selection" style="height: auto">
                            <lit-table-column width="100px" title="TimeStamp" data-index="ts" key="ts"  align="flex-start" order>
                            </lit-table-column>
                            <lit-table-column width="1fr" title="Fd" data-index="fd" key="fd" align="flex-start"  order>
                            </lit-table-column>
                            <lit-table-column width="1fr" title="Size" data-index="sizeStr" key="sizeStr" align="flex-start"  order>
                            </lit-table-column>
                            <lit-table-column width="1fr" title="Adj" data-index="adj" key="adj" align="flex-start" order>
                            </lit-table-column>
                            <lit-table-column width="250px" title="AshmName" data-index="name" key="name" align="flex-start"  order>
                            </lit-table-column>
                            <lit-table-column width="1fr" title="Id" data-index="id" key="id" align="flex-start"  order>
                            </lit-table-column>
                            <lit-table-column width="200px" title="Time" data-index="time" key="time" align="flex-start"  order>
                            </lit-table-column>
                            <lit-table-column width="1fr" title="Refcount" data-index="count" key="count" align="flex-start"  order>
                            </lit-table-column>
                            <lit-table-column width="1fr" title="Purged" data-index="purged" key="purged" align="flex-start"  order>
                            </lit-table-column>
                            <lit-table-column width="1fr" title="Flag" data-index="flag" key="flag" align="flex-start"  order>
                            </lit-table-column>
                        </lit-table>
        `;
  }
}
