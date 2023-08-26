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
import { getTabPaneIOTierStatisticsData } from '../../../../database/SqlLite.js';
import { Utils } from '../../base/Utils.js';
import { LitProgressBar } from '../../../../../base-ui/progress-bar/LitProgressBar.js';
import { TabPaneFilter } from '../TabPaneFilter.js';
import { SpSystemTrace } from '../../../SpSystemTrace.js';

@element('tabpane-io-tier-statistics')
export class TabPaneIOTierStatistics extends BaseElement {
  private ioTierStatisticsTbl: LitTable | null | undefined;
  private ioTierStatisticsSelectionParam: SelectionParam | null | undefined;
  private ioTierStatisticsProgressEL: LitProgressBar | null | undefined;
  private loadingPage: any;
  private loadingList: number[] = [];
  private ioTierStatisticsSource: Array<any> = [];
  private ioTierStatisticsSortKey: string = '';
  private ioTierStatisticsSortType: number = 0;
  private ioTierStatisticsResultData: Array<any> = [];

  set data(ioTierStatisticsSelection: SelectionParam | any) {
    if (ioTierStatisticsSelection == this.ioTierStatisticsSelectionParam) {
      return;
    }
    this.ioTierStatisticsProgressEL!.loading = true;
    this.loadingPage.style.visibility = 'visible';
    this.ioTierStatisticsSelectionParam = ioTierStatisticsSelection;
    // @ts-ignore
    this.ioTierStatisticsTbl!.shadowRoot!.querySelector('.table').style.height =
      this.parentElement!.clientHeight - 20 + 'px';
    this.queryDataByDB(ioTierStatisticsSelection);
  }

  initElements(): void {
    this.ioTierStatisticsProgressEL = this.shadowRoot!.querySelector<LitProgressBar>('.progress');
    this.loadingPage = this.shadowRoot!.querySelector('.loading');
    this.ioTierStatisticsTbl = this.shadowRoot!.querySelector<LitTable>('#tb-io-tier-statistics');
    this.ioTierStatisticsTbl!.addEventListener('column-click', (evt) => {
      // @ts-ignore
      this.ioTierStatisticsSortKey = evt.detail.key;
      // @ts-ignore
      this.ioTierStatisticsSortType = evt.detail.sort;

      let newSource = JSON.parse(JSON.stringify(this.ioTierStatisticsSource));
      if (this.ioTierStatisticsSortType != 0 && newSource.length > 0)
        this.sortTable(newSource[0], this.ioTierStatisticsSortKey);
      this.ioTierStatisticsTbl!.recycleDataSource = newSource;
    });
  }

  connectedCallback() {
    super.connectedCallback();
    new ResizeObserver((entries) => {
      if (this.parentElement!.clientHeight != 0) {
        // @ts-ignore
        this.ioTierStatisticsTbl!.shadowRoot!.querySelector('.table').style.height =
          this.parentElement!.clientHeight - 25 + 'px';
        this.ioTierStatisticsTbl!.reMeauseHeight();
        this.loadingPage.style.height = this.parentElement!.clientHeight - 24 + 'px';
      }
    }).observe(this.parentElement!);
  }

  getInitData(initIoTierItem: any, nameTitle: any = 'pname', subtitle: any = null) {
    if (nameTitle == 'path') {
      initIoTierItem.path =
        initIoTierItem.path != null ? SpSystemTrace.DATA_DICT.get(parseInt(initIoTierItem.path)) : '-';
    }
    return {
      ...initIoTierItem,
      title: initIoTierItem[nameTitle] + (subtitle ? '(' + initIoTierItem[subtitle] + ')' : ''),
      allDuration: Utils.getProbablyTime(initIoTierItem.allDuration),
      minDuration: Utils.getProbablyTime(initIoTierItem.minDuration),
      maxDuration: Utils.getProbablyTime(initIoTierItem.maxDuration),
      avgDuration: Utils.getProbablyTime(initIoTierItem.avgDuration),
      node: { ...initIoTierItem, children: [] },
    };
  }

  queryDataByDB(ioTierParam: SelectionParam | any) {
    this.loadingList.push(1);
    this.ioTierStatisticsProgressEL!.loading = true;
    this.loadingPage.style.visibility = 'visible';
    getTabPaneIOTierStatisticsData(
      ioTierParam.leftNs + ioTierParam.recordStartNs,
      ioTierParam.rightNs + ioTierParam.recordStartNs,
      ioTierParam.diskIOipids
    ).then((result) => {
      this.loadingList.splice(0, 1);
      if (this.loadingList.length == 0) {
        this.ioTierStatisticsProgressEL!.loading = false;
        this.loadingPage.style.visibility = 'hidden';
      }
      this.ioTierStatisticsResultData = JSON.parse(JSON.stringify(result));
      this.sortioTierStatisticsStatus(result, 'tier', 'ipid');
    });
  }

  sortioTierStatisticsStatus(result: Array<any>, firstLevel: string, secondLevel: string) {
    let ioTierFatherMap = new Map<any, any>();
    let ioTierChildMap = new Map<any, any>();
    let ioTierAllNode: any = {
      title: 'All',
      count: 0,
      allDuration: 0,
      minDuration: 0,
      maxDuration: 0,
      avgDuration: '',
      children: [],
    };
    result.forEach((resultItem, idx) => {
      if (ioTierChildMap.has(resultItem[firstLevel] + '_' + resultItem[secondLevel])) {
        let currentChildObject = ioTierChildMap.get(resultItem[firstLevel] + '_' + resultItem[secondLevel]);
        currentChildObject.count += resultItem.count;
        currentChildObject.allDuration += resultItem.allDuration;
        currentChildObject.minDuration =
          currentChildObject.minDuration <= resultItem.minDuration
            ? currentChildObject.minDuration
            : resultItem.minDuration;
        currentChildObject.maxDuration =
          currentChildObject.maxDuration >= resultItem.maxDuration
            ? currentChildObject.maxDuration
            : resultItem.maxDuration;
        currentChildObject.children.push(this.getInitData(resultItem, 'path', null));
      } else {
        ioTierChildMap.set(resultItem[firstLevel] + '_' + resultItem[secondLevel], {
          ...resultItem,
          children: [this.getInitData(resultItem, 'path', null)],
        });
      }

      if (ioTierFatherMap.has(resultItem[firstLevel])) {
        let currentFatherObject = ioTierFatherMap.get(resultItem[firstLevel]);
        currentFatherObject.count += resultItem.count;
        currentFatherObject.allDuration += resultItem.allDuration;
        currentFatherObject.minDuration =
          currentFatherObject.minDuration <= resultItem.minDuration
            ? currentFatherObject.minDuration
            : resultItem.minDuration;
        currentFatherObject.maxDuration =
          currentFatherObject.maxDuration >= resultItem.maxDuration
            ? currentFatherObject.maxDuration
            : resultItem.maxDuration;
        currentFatherObject.children.push(this.getInitData(resultItem));
      } else {
        ioTierFatherMap.set(resultItem[firstLevel], {
          ...resultItem,
          children: [this.getInitData(resultItem)],
        });
      }
      if (idx == 0) {
        ioTierAllNode.minDuration = resultItem.minDuration;
      } else {
        ioTierAllNode.minDuration =
          ioTierAllNode.minDuration <= resultItem.minDuration ? ioTierAllNode.minDuration : resultItem.minDuration;
      }
      ioTierAllNode.count += resultItem.count;
      ioTierAllNode.allDuration += resultItem.allDuration;
      ioTierAllNode.maxDuration =
        ioTierAllNode.maxDuration >= resultItem.maxDuration ? ioTierAllNode.maxDuration : resultItem.maxDuration;
    });

    for (let ks of ioTierFatherMap.keys()) {
      let sp = ioTierFatherMap.get(ks);
      sp!.children = [];
      sp.avgDuration = sp.allDuration / sp.count;
      let ioTierNode = this.getInitData(sp, 'tier', null);
      ioTierNode.path = {
        tier: ioTierNode.tier,
        pid: null,
        path: null,
        value: ioTierNode.title,
      };
      for (let kst of ioTierChildMap.keys()) {
        if (kst.startsWith(ks + '_')) {
          let spt = ioTierChildMap.get(kst);
          let data = this.getInitData(spt!, 'pname', 'pid');
          data.path = {
            tier: ioTierNode.tier,
            pid: data.pid,
            path: null,
            value: 'All-' + ioTierNode.title + '-' + data.title,
          };
          data.children.forEach((e: any) => {
            e.path = {
              tier: ioTierNode.tier,
              pid: data.pid,
              path: e.path,
              value: 'All-' + ioTierNode.title + '-' + data.title + '-' + e.title,
            };
          });
          sp!.children.push(data);
        }
      }
      ioTierAllNode.children.push(ioTierNode);
    }

    ioTierAllNode.avgDuration = ioTierAllNode.allDuration / ioTierAllNode.count;
    ioTierAllNode = this.getInitData(ioTierAllNode);
    ioTierAllNode.title = 'All';
    ioTierAllNode.path = { tier: null, pid: null, path: null, value: 'All' };
    this.ioTierStatisticsSource = result.length > 0 ? [ioTierAllNode] : [];
    let newSource = JSON.parse(JSON.stringify(this.ioTierStatisticsSource));
    if (this.ioTierStatisticsSortType != 0 && result.length > 0)
      this.sortTable(newSource[0], this.ioTierStatisticsSortKey);
    this.ioTierStatisticsTbl!.recycleDataSource = newSource;
  }

  sortTable(allNode: any, key: string) {
    allNode.children.sort((ioTierStatNodeA: any, ioTierStatNodeB: any) => {
      if (this.ioTierStatisticsSortType == 1) {
        return ioTierStatNodeA.node[key] - ioTierStatNodeB.node[key];
      } else if (this.ioTierStatisticsSortType == 2) {
        return ioTierStatNodeB.node[key] - ioTierStatNodeA.node[key];
      }
    });
    allNode.children.forEach((item: any) => {
      item.children.sort((ioTierStatItemA: any, ioTierStatItemB: any) => {
        if (this.ioTierStatisticsSortType == 1) {
          return ioTierStatItemA.node[key] - ioTierStatItemB.node[key];
        } else if (this.ioTierStatisticsSortType == 2) {
          return ioTierStatItemB.node[key] - ioTierStatItemA.node[key];
        }
      });
      item.children.forEach((ioTierStatItem: any) => {
        ioTierStatItem.children.sort((ioTierStatItemA: any, ioTierStatItemB: any) => {
          if (this.ioTierStatisticsSortType == 1) {
            return ioTierStatItemA.node[key] - ioTierStatItemB.node[key];
          } else if (this.ioTierStatisticsSortType == 2) {
            return ioTierStatItemB.node[key] - ioTierStatItemA.node[key];
          }
        });
      });
    });
  }

  initHtml(): string {
    return `
        <style>
        .io-tier-stat-progress{
            bottom: 5px;
            position: absolute;
            height: 1px;
            left: 0;
            right: 0;
        }
        :host{
            padding: 10px 10px 0 10px;
            display: flex;
            flex-direction: column;
        }
        .io-tier-stat-loading{
            bottom: 0;
            position: absolute;
            left: 0;
            right: 0;
            width:100%;
            background:transparent;
            z-index: 999999;
        }
        </style>
        <lit-table id="tb-io-tier-statistics" style="height: auto" tree>
            <lit-table-column class="io-tier-stat-column" width="20%" title="Tier/Process/Path" data-index="title" key="title" align="flex-start">
            </lit-table-column>
            <lit-table-column class="io-tier-stat-column" width="1fr" title="Count" data-index="count" key="count" align="flex-start" order>
            </lit-table-column>
            <lit-table-column class="io-tier-stat-column" width="1fr" title="Total Latency" data-index="allDuration" key="allDuration" align="flex-start" order>
            </lit-table-column>
            <lit-table-column class="io-tier-stat-column" width="1fr" title="Min Total Latency" data-index="minDuration" key="minDuration" align="flex-start" order>
            </lit-table-column>
            <lit-table-column class="io-tier-stat-column" width="1fr" title="Avg Total Latency" data-index="avgDuration" key="avgDuration" align="flex-start" order>
            </lit-table-column>
            <lit-table-column class="io-tier-stat-column" width="1fr" title="Max Total Latency" data-index="maxDuration" key="maxDuration" align="flex-start" order>
            </lit-table-column>
        </lit-table>
        <lit-progress-bar class="progress io-tier-stat-progress"></lit-progress-bar>
        <div class="loading io-tier-stat-loading"></div>
        `;
  }
}
