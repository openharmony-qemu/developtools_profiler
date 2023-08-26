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
import { getTabPaneVirtualMemoryStatisticsData } from '../../../../database/SqlLite.js';
import { Utils } from '../../base/Utils.js';
import { LitProgressBar } from '../../../../../base-ui/progress-bar/LitProgressBar.js';
import { TabPaneFilter } from '../TabPaneFilter.js';
import '../TabPaneFilter.js';
import { VM_TYPE_MAP } from '../../../../database/logic-worker/ProcedureLogicWorkerFileSystem.js';

@element('tabpane-virtual-memory-statistics')
export class TabPaneVirtualMemoryStatistics extends BaseElement {
  private vmStatisticsTbl: LitTable | null | undefined;
  private vmStatisticsSelectionParam: SelectionParam | null | undefined;
  private vmStatisticsProgressEL: LitProgressBar | null | undefined;
  private vmStatisticsFilter: TabPaneFilter | null | undefined;
  private loadingPage: any;
  private loadingList: number[] = [];
  private vmStatisticsSource: Array<any> = [];
  private vmStatisticsSortKey: string = '';
  private vmStatisticsSortType: number = 0;
  private vmStatisticsResultData: Array<any> = [];

  set data(vmStatisticsSelection: SelectionParam | any) {
    if (vmStatisticsSelection == this.vmStatisticsSelectionParam) {
      return;
    }
    this.vmStatisticsProgressEL!.loading = true;
    this.loadingPage.style.visibility = 'visible';
    this.vmStatisticsSelectionParam = vmStatisticsSelection;
    // @ts-ignore
    this.vmStatisticsTbl!.shadowRoot!.querySelector('.table').style.height =
      this.parentElement!.clientHeight - 20 + 'px';
    this.queryDataByDB(vmStatisticsSelection);
  }

  initElements(): void {
    this.vmStatisticsProgressEL = this.shadowRoot!.querySelector<LitProgressBar>('.progress');
    this.loadingPage = this.shadowRoot!.querySelector('.loading');
    this.vmStatisticsTbl = this.shadowRoot!.querySelector<LitTable>('#tb-vm-statistics');
    this.vmStatisticsTbl!.addEventListener('column-click', (evt) => {
      // @ts-ignore
      this.vmStatisticsSortKey = evt.detail.key;
      // @ts-ignore
      this.vmStatisticsSortType = evt.detail.sort;

      let newSource = JSON.parse(JSON.stringify(this.vmStatisticsSource));
      if (this.vmStatisticsSortType != 0 && newSource.length > 0)
        this.sortVmStatisticsTable(newSource[0], this.vmStatisticsSortKey);
      this.vmStatisticsTbl!.recycleDataSource = newSource;
    });
    this.vmStatisticsFilter = this.shadowRoot!.querySelector<TabPaneFilter>('#filter');
    this.vmStatisticsFilter!.getStatisticsTypeData((type) => {
      if (type == 'operation') {
        this.sortStatus(this.vmStatisticsResultData, 'ipid', 'itid');
      } else {
        this.sortStatus(this.vmStatisticsResultData, 'type', 'ipid');
      }
      this.vmStatisticsTbl!.shadowRoot!.querySelector('div > div.thead > div > div:nth-child(1) > label')!.textContent =
        type == 'operation' ? 'Process/Thread/Operation' : 'Operation/Process/Thread';
    });
  }

  connectedCallback() {
    super.connectedCallback();
    new ResizeObserver((entries) => {
      if (this.parentElement!.clientHeight != 0) {
        // @ts-ignore
        this.vmStatisticsTbl!.shadowRoot!.querySelector('.table').style.height =
          this.parentElement!.clientHeight - 10 - 32 + 'px';
        this.vmStatisticsTbl!.reMeauseHeight();
        this.loadingPage.style.height = this.parentElement!.clientHeight - 24 + 'px';
      }
    }).observe(this.parentElement!);
  }

  getInitData(initVmMemoryStatItem: any, nameTitle: any = 'pname', subtitle: any = null) {
    // @ts-ignore
    let title = nameTitle == 'type' ? VM_TYPE_MAP[initVmMemoryStatItem[nameTitle]] : initVmMemoryStatItem[nameTitle];
    return {
      ...initVmMemoryStatItem,
      title: title + (subtitle ? '(' + initVmMemoryStatItem[subtitle] + ')' : ''),
      allDuration: Utils.getProbablyTime(initVmMemoryStatItem.allDuration),
      minDuration: Utils.getProbablyTime(initVmMemoryStatItem.minDuration),
      maxDuration: Utils.getProbablyTime(initVmMemoryStatItem.maxDuration),
      avgDuration: Utils.getProbablyTime(initVmMemoryStatItem.avgDuration),
      node: { ...initVmMemoryStatItem, children: [] },
    };
  }

  queryDataByDB(vmMemoryStatParam: SelectionParam | any) {
    this.loadingList.push(1);
    this.vmStatisticsProgressEL!.loading = true;
    this.loadingPage.style.visibility = 'visible';
    getTabPaneVirtualMemoryStatisticsData(
      vmMemoryStatParam.leftNs + vmMemoryStatParam.recordStartNs,
      vmMemoryStatParam.rightNs + vmMemoryStatParam.recordStartNs
    ).then((result) => {
      this.loadingList.splice(0, 1);
      if (this.loadingList.length == 0) {
        this.vmStatisticsProgressEL!.loading = false;
        this.loadingPage.style.visibility = 'hidden';
      }
      this.vmStatisticsResultData = JSON.parse(JSON.stringify(result));
      this.sortStatus(result, 'type', 'ipid');
    });
  }

  sortStatus(result: Array<any>, firstLevel: string, secondLevel: string) {
    let vmMemoryStatFatherMap = new Map<any, any>();
    let vmMemoryStatChildMap = new Map<any, any>();
    let vmMemoryStatAllNode: any = {
      title: 'All',
      count: 0,
      allDuration: 0,
      minDuration: 0,
      maxDuration: 0,
      avgDuration: '',
      children: [],
    };
    result.forEach((item, idx) => {
      if (vmMemoryStatChildMap.has(item[firstLevel] + '_' + item[secondLevel])) {
        let vmMemoryStatChildObj = vmMemoryStatChildMap.get(item[firstLevel] + '_' + item[secondLevel]);
        vmMemoryStatChildObj.count += item.count;
        vmMemoryStatChildObj.allDuration += item.allDuration;
        vmMemoryStatChildObj.minDuration =
          vmMemoryStatChildObj.minDuration <= item.minDuration ? vmMemoryStatChildObj.minDuration : item.minDuration;
        vmMemoryStatChildObj.maxDuration =
          vmMemoryStatChildObj.maxDuration >= item.maxDuration ? vmMemoryStatChildObj.maxDuration : item.maxDuration;
        vmMemoryStatChildObj.children.push(
          this.getInitData(item, firstLevel == 'type' ? 'tname' : 'type', firstLevel == 'type' ? 'tid' : null)
        );
      } else {
        vmMemoryStatChildMap.set(item[firstLevel] + '_' + item[secondLevel], {
          ...item,
          children: [
            this.getInitData(item, firstLevel == 'type' ? 'tname' : 'type', firstLevel == 'type' ? 'tid' : null),
          ],
        });
      }

      if (vmMemoryStatFatherMap.has(item[firstLevel])) {
        let vmMemoryStatFatherObj = vmMemoryStatFatherMap.get(item[firstLevel]);
        vmMemoryStatFatherObj.count += item.count;
        vmMemoryStatFatherObj.allDuration += item.allDuration;
        vmMemoryStatFatherObj.minDuration =
          vmMemoryStatFatherObj.minDuration <= item.minDuration ? vmMemoryStatFatherObj.minDuration : item.minDuration;
        vmMemoryStatFatherObj.maxDuration =
          vmMemoryStatFatherObj.maxDuration >= item.maxDuration ? vmMemoryStatFatherObj.maxDuration : item.maxDuration;
        vmMemoryStatFatherObj.children.push(this.getInitData(item));
      } else {
        vmMemoryStatFatherMap.set(item[firstLevel], {
          ...item,
          children: [this.getInitData(item)],
        });
      }
      if (idx == 0) {
        vmMemoryStatAllNode.minDuration = item.minDuration;
      } else {
        vmMemoryStatAllNode.minDuration =
          vmMemoryStatAllNode.minDuration <= item.minDuration ? vmMemoryStatAllNode.minDuration : item.minDuration;
      }
      vmMemoryStatAllNode.count += item.count;
      vmMemoryStatAllNode.allDuration += item.allDuration;
      vmMemoryStatAllNode.maxDuration =
        vmMemoryStatAllNode.maxDuration >= item.maxDuration ? vmMemoryStatAllNode.maxDuration : item.maxDuration;
    });

    for (let ks of vmMemoryStatFatherMap.keys()) {
      let sp = vmMemoryStatFatherMap.get(ks);
      sp!.children = [];
      sp.avgDuration = sp.allDuration / sp.count;
      let vmMemoryStatNode = this.getInitData(
        sp,
        firstLevel == 'type' ? 'type' : 'pname',
        firstLevel == 'type' ? null : 'pid'
      );
      vmMemoryStatNode.path = { type: null, tid: null, pid: null, value: vmMemoryStatNode.title };
      vmMemoryStatNode.path[firstLevel == 'type' ? 'type' : 'pid'] =
        vmMemoryStatNode[firstLevel == 'type' ? 'type' : 'pid'];
      for (let kst of vmMemoryStatChildMap.keys()) {
        if (kst.startsWith(ks + '_')) {
          let spt = vmMemoryStatChildMap.get(kst);
          let data = this.getInitData(
            spt!,
            firstLevel == 'type' ? 'pname' : 'tname',
            firstLevel == 'type' ? 'pid' : 'tid'
          );
          data.path = {
            type: null,
            tid: null,
            pid: null,
            value: 'All-' + vmMemoryStatNode.title + '-' + data.title,
          };
          data.path[firstLevel == 'type' ? 'type' : 'pid'] = vmMemoryStatNode[firstLevel == 'type' ? 'type' : 'pid'];
          data.path[firstLevel == 'type' ? 'pid' : 'tid'] = data[firstLevel == 'type' ? 'pid' : 'tid'];
          data.children.forEach((e: any) => {
            e.path = {
              type: null,
              tid: null,
              pid: null,
              value: 'All-' + vmMemoryStatNode.title + '-' + data.title + '-' + e.title,
            };
            e.path[firstLevel == 'type' ? 'type' : 'pid'] = vmMemoryStatNode[firstLevel == 'type' ? 'type' : 'pid'];
            e.path[firstLevel == 'type' ? 'pid' : 'tid'] = data[firstLevel == 'type' ? 'pid' : 'tid'];
            e.path[firstLevel == 'type' ? 'tid' : 'type'] = e[firstLevel == 'type' ? 'tid' : 'type'];
          });
          sp!.children.push(data);
        }
      }
      vmMemoryStatAllNode.children.push(vmMemoryStatNode);
    }

    vmMemoryStatAllNode.avgDuration = vmMemoryStatAllNode.allDuration / vmMemoryStatAllNode.count;
    vmMemoryStatAllNode = this.getInitData(vmMemoryStatAllNode);
    vmMemoryStatAllNode.title = 'All';
    vmMemoryStatAllNode.path = { type: null, tid: null, pid: null, value: 'All' };
    this.vmStatisticsSource = result.length > 0 ? [vmMemoryStatAllNode] : [];
    let newSource = JSON.parse(JSON.stringify(this.vmStatisticsSource));
    if (this.vmStatisticsSortType != 0 && result.length > 0)
      this.sortVmStatisticsTable(newSource[0], this.vmStatisticsSortKey);
    this.vmStatisticsTbl!.recycleDataSource = newSource;
  }

  sortVmStatisticsTable(allNode: any, key: string) {
    allNode.children.sort((vmStatNodeA: any, vmStatNodeB: any) => {
      if (this.vmStatisticsSortType == 1) {
        return vmStatNodeA.node[key] - vmStatNodeB.node[key];
      } else if (this.vmStatisticsSortType == 2) {
        return vmStatNodeB.node[key] - vmStatNodeA.node[key];
      }
    });
    allNode.children.forEach((item: any) => {
      item.children.sort((vmStatNodeA: any, vmStatNodeB: any) => {
        if (this.vmStatisticsSortType == 1) {
          return vmStatNodeA.node[key] - vmStatNodeB.node[key];
        } else if (this.vmStatisticsSortType == 2) {
          return vmStatNodeB.node[key] - vmStatNodeA.node[key];
        }
      });
      item.children.forEach((vmStatItem: any) => {
        vmStatItem.children.sort((vmStatItemA: any, vmStatItemB: any) => {
          if (this.vmStatisticsSortType == 1) {
            return vmStatItemA.node[key] - vmStatItemB.node[key];
          } else if (this.vmStatisticsSortType == 2) {
            return vmStatItemB.node[key] - vmStatItemA.node[key];
          }
        });
      });
    });
  }

  initHtml(): string {
    return `
        <style>
        .vm-stat-progress{
            bottom: 5px;
            position: absolute;
            height: 1px;
            left: 0;
            right: 0;
        }
        :host{
            display: flex;
            flex-direction: column;
            padding: 10px 10px 0 10px;
        }
        .vm-stat-loading{
            bottom: 0;
            position: absolute;
            left: 0;
            right: 0;
            width:100%;
            background:transparent;
            z-index: 999999;
        }
        </style>
        <lit-table id="tb-vm-statistics" style="height: auto" tree>
            <lit-table-column class="vm-memory-stat-column" width="20%" title="Operation/Process/Thread" data-index="title" key="title" align="flex-start">
            </lit-table-column>
            <lit-table-column class="vm-memory-stat-column" width="1fr" title="Count" data-index="count" key="count" align="flex-start" order>
            </lit-table-column>
            <lit-table-column class="vm-memory-stat-column" width="1fr" title="Duration" data-index="allDuration" key="allDuration" align="flex-start" order>
            </lit-table-column>
            <lit-table-column class="vm-memory-stat-column" width="1fr" title="Min Duration" data-index="minDuration" key="minDuration" align="flex-start" order>
            </lit-table-column>
            <lit-table-column class="vm-memory-stat-column" width="1fr" title="Avg Duration" data-index="avgDuration" key="avgDuration" align="flex-start" order>
            </lit-table-column>
            <lit-table-column class="vm-memory-stat-column" width="1fr" title="Max Duration" data-index="maxDuration" key="maxDuration" align="flex-start" order>
            </lit-table-column>
        </lit-table>
        <lit-progress-bar class="progress vm-stat-progress"></lit-progress-bar>
        <tab-pane-filter id="filter" sort></tab-pane-filter>
        <div class="loading vm-stat-loading"></div>
        `;
  }
}
