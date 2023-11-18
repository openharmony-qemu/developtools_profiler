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
import { getTabPaneFilesystemStatistics } from '../../../../database/SqlLite.js';
import { Utils } from '../../base/Utils.js';
import { LitProgressBar } from '../../../../../base-ui/progress-bar/LitProgressBar.js';

@element('tabpane-file-statistics')
export class TabPaneFileStatistics extends BaseElement {
  private fileStatisticsTbl: LitTable | null | undefined;
  private selectionParam: SelectionParam | null | undefined;
  private fileStatisticsProgressEL: LitProgressBar | null | undefined;
  private fileStatisticsLoadingPage: any;
  private fileStatisticsLoadingList: number[] = [];
  private fileStatisticsSource: Array<any> = [];
  private typeList: Array<string> = ['OPEN', 'CLOSE', 'READ', 'WRITE'];
  private fileStatisticsSortKey: string = '';
  private fileStatisticsSortType: number = 0;

  set data(fileStatisticsSelection: SelectionParam | any) {
    if (fileStatisticsSelection == this.selectionParam) {
      return;
    }
    this.fileStatisticsProgressEL!.loading = true;
    this.fileStatisticsLoadingPage.style.visibility = 'visible';
    this.selectionParam = fileStatisticsSelection;
    // @ts-ignore
    this.fileStatisticsTbl!.shadowRoot!.querySelector('.table').style.height =
      this.parentElement!.clientHeight - 25 + 'px';
    this.queryDataByDB(fileStatisticsSelection);
  }

  initElements(): void {
    this.fileStatisticsProgressEL = this.shadowRoot!.querySelector<LitProgressBar>('.file-statistics-progress');
    this.fileStatisticsLoadingPage = this.shadowRoot!.querySelector('.file-statistics-loading');
    this.fileStatisticsTbl = this.shadowRoot!.querySelector<LitTable>('#tb-file-statistics');
    this.fileStatisticsTbl!.addEventListener('column-click', (evt) => {
      // @ts-ignore
      this.fileStatisticsSortKey = evt.detail.key;
      // @ts-ignore
      this.fileStatisticsSortType = evt.detail.sort;

      let newSource = JSON.parse(JSON.stringify(this.fileStatisticsSource));
      if (this.fileStatisticsSortType != 0 && newSource.length > 0)
        this.sortTable(newSource[0], this.fileStatisticsSortKey);
      this.fileStatisticsTbl!.recycleDataSource = newSource;
    });
  }

  connectedCallback() {
    super.connectedCallback();
    new ResizeObserver((entries) => {
      if (this.parentElement!.clientHeight != 0) {
        // @ts-ignore
        this.fileStatisticsTbl!.shadowRoot!.querySelector('.table').style.height =
          this.parentElement!.clientHeight - 25 + 'px';
        this.fileStatisticsTbl!.reMeauseHeight();
        this.fileStatisticsLoadingPage.style.height = this.parentElement!.clientHeight - 24 + 'px';
      }
    }).observe(this.parentElement!);
  }

  getInitData(item: any) {
    return {
      ...item,
      title: item.name + '(' + item.pid + ')',
      logicalWrites: Utils.getBinaryByteWithUnit(item.logicalWrites),
      logicalReads: Utils.getBinaryByteWithUnit(item.logicalReads),
      otherFile: Utils.getBinaryByteWithUnit(item.otherFile),
      allDuration: Utils.getProbablyTime(item.allDuration),
      minDuration: Utils.getProbablyTime(item.minDuration),
      maxDuration: Utils.getProbablyTime(item.maxDuration),
      avgDuration: Utils.getProbablyTime(item.avgDuration),
      node: { ...item, children: [] },
    };
  }

  queryDataByDB(val: SelectionParam | any) {
    this.fileStatisticsLoadingList.push(1);
    this.fileStatisticsProgressEL!.loading = true;
    this.fileStatisticsLoadingPage.style.visibility = 'visible';
    getTabPaneFilesystemStatistics(
      val.leftNs + val.recordStartNs,
      val.rightNs + val.recordStartNs,
      val.fileSystemType
    ).then((result) => {
      this.fileStatisticsLoadingList.splice(0, 1);
      if (this.fileStatisticsLoadingList.length == 0) {
        this.fileStatisticsProgressEL!.loading = false;
        this.fileStatisticsLoadingPage.style.visibility = 'hidden';
      }
      let fileStatisticsFatherMap = new Map<any, any>();
      let fileStatisticsAllNode: any = {
        title: 'All',
        count: 0,
        logicalReads: 0,
        logicalWrites: 0,
        otherFile: 0,
        allDuration: 0,
        minDuration: 0,
        maxDuration: 0,
        avgDuration: '',
        children: [],
      };
      result.forEach((item, idx) => {
        if (fileStatisticsFatherMap.has(item.type)) {
          let fileStatisticsObj = fileStatisticsFatherMap.get(item.type);
          fileStatisticsObj.count += item.count;
          fileStatisticsObj.logicalReads += item.logicalReads;
          fileStatisticsObj.logicalWrites += item.logicalWrites;
          fileStatisticsObj.otherFile += item.otherFile;
          fileStatisticsObj.allDuration += item.allDuration;
          fileStatisticsObj.minDuration =
            fileStatisticsObj.minDuration <= item.minDuration ? fileStatisticsObj.minDuration : item.minDuration;
          fileStatisticsObj.maxDuration =
            fileStatisticsObj.maxDuration >= item.maxDuration ? fileStatisticsObj.maxDuration : item.maxDuration;
          fileStatisticsObj.children.push(this.getInitData(item));
        } else {
          fileStatisticsFatherMap.set(item.type, {
            type: item.type,
            count: item.count,
            logicalReads: item.logicalReads,
            logicalWrites: item.logicalWrites,
            otherFile: item.otherFile,
            allDuration: item.allDuration,
            minDuration: item.minDuration,
            maxDuration: item.maxDuration,
            children: [this.getInitData(item)],
          });
        }
        if (idx == 0) {
          fileStatisticsAllNode.minDuration = item.minDuration;
        } else {
          fileStatisticsAllNode.minDuration =
            fileStatisticsAllNode.minDuration <= item.minDuration
              ? fileStatisticsAllNode.minDuration
              : item.minDuration;
        }
        fileStatisticsAllNode.count += item.count;
        fileStatisticsAllNode.logicalReads += item.logicalReads;
        fileStatisticsAllNode.logicalWrites += item.logicalWrites;
        fileStatisticsAllNode.otherFile += item.otherFile;
        fileStatisticsAllNode.allDuration += item.allDuration;
        fileStatisticsAllNode.maxDuration =
          fileStatisticsAllNode.maxDuration >= item.maxDuration ? fileStatisticsAllNode.maxDuration : item.maxDuration;
      });
      fileStatisticsFatherMap.forEach((item) => {
        item.avgDuration = item.allDuration / item.count;
        let node = this.getInitData(item);
        if (item.type < 4) {
          node.title = this.typeList[item.type];
        } else {
          node.title = item.type;
        }
        fileStatisticsAllNode.children.push(node);
      });
      fileStatisticsAllNode.avgDuration = fileStatisticsAllNode.allDuration / fileStatisticsAllNode.count;
      fileStatisticsAllNode = this.getInitData(fileStatisticsAllNode);
      fileStatisticsAllNode.title = 'All';
      this.fileStatisticsSource = result.length > 0 ? [fileStatisticsAllNode] : [];
      let newSource = JSON.parse(JSON.stringify(this.fileStatisticsSource));
      if (this.fileStatisticsSortType != 0 && result.length > 0)
        this.sortTable(newSource[0], this.fileStatisticsSortKey);
      this.fileStatisticsTbl!.recycleDataSource = newSource;
    });
  }

  sortTable(fileStatisticsAllNode: any, key: string) {
    fileStatisticsAllNode.children.sort((fileStatisticsA: any, fileStatisticsB: any) => {
      if (this.fileStatisticsSortType == 1) {
        return fileStatisticsA.node[key] - fileStatisticsB.node[key];
      } else if (this.fileStatisticsSortType == 2) {
        return fileStatisticsB.node[key] - fileStatisticsA.node[key];
      }
    });
    fileStatisticsAllNode.children.forEach((item: any) => {
      item.children.sort((fileStatisticsA: any, fileStatisticsB: any) => {
        if (this.fileStatisticsSortType == 1) {
          return fileStatisticsA.node[key] - fileStatisticsB.node[key];
        } else if (this.fileStatisticsSortType == 2) {
          return fileStatisticsB.node[key] - fileStatisticsA.node[key];
        }
      });
    });
  }

  initHtml(): string {
    return `
        <style>
        :host{
            display: flex;
            flex-direction: column;
            padding: 10px 10px;
        }
        .file-statistics-progress{
            bottom: 5px;
            position: absolute;
            height: 1px;
            left: 0;
            right: 0;
        }
        .file-statistics-loading{
            bottom: 0;
            position: absolute;
            left: 0;
            right: 0;
            width:100%;
            background:transparent;
            z-index: 999999;
        }
        </style>
        <lit-table id="tb-file-statistics" style="height: auto" tree>
            <lit-table-column class="fs-stat-column" width="20%" title="Syscall/Process" data-index="title" key="title" align="flex-start"retract>
            </lit-table-column>
            <lit-table-column class="fs-stat-column" width="1fr" title="Count" data-index="count" key="count" align="flex-start" order>
            </lit-table-column>
            <lit-table-column class="fs-stat-column" width="1fr" title="Logical Writes" data-index="logicalWrites" key="logicalWrites" align="flex-start" order>
            </lit-table-column>
            <lit-table-column class="fs-stat-column" width="1fr" title="Logical Reads" data-index="logicalReads" key="logicalReads" align="flex-start" order>
            </lit-table-column>
            <lit-table-column class="fs-stat-column" width="1fr" title="Other Filesystem Bytes" data-index="otherFile" key="otherFile" align="flex-start" order>
            </lit-table-column>
            <lit-table-column class="fs-stat-column" width="1fr" title="Duration" data-index="allDuration" key="allDuration" align="flex-start" order>
            </lit-table-column>
            <lit-table-column class="fs-stat-column" width="1fr" title="Min Duration" data-index="minDuration" key="minDuration" align="flex-start" order>
            </lit-table-column>
            <lit-table-column class="fs-stat-column" width="1fr" title="Avg Duration" data-index="avgDuration" key="avgDuration" align="flex-start" order>
            </lit-table-column>
            <lit-table-column class="fs-stat-column" width="1fr" title="Max Duration" data-index="maxDuration" key="maxDuration" align="flex-start" order>
            </lit-table-column>
        </lit-table>
        <lit-progress-bar class="file-statistics-progress"></lit-progress-bar>
        <div class="file-statistics-loading"></div>
        `;
  }
}
