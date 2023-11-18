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
import { LogStruct } from '../../../../database/ui-worker/ProcedureWorkerLog.js';
import { ColorUtils } from '../../base/ColorUtils.js';
import { LitTable } from '../../../../../base-ui/table/lit-table.js';
import { LitIcon } from '../../../../../base-ui/icon/LitIcon.js';

@element('tab-hi-log-summary')
export class TabPaneHiLogSummary extends BaseElement {
  private logSummaryTable: HTMLDivElement | undefined | null;
  private summaryDownLoadTbl: LitTable | undefined | null;
  private parentTabEl: HTMLElement | undefined | null;
  private systemLogSource: LogStruct[] = [];
  private logTreeNodes: LogTreeNode[] = [];
  private expansionDiv: HTMLDivElement | undefined | null;
  private expansionUpIcon: LitIcon | undefined | null;
  private expansionDownIcon: LitIcon | undefined | null;
  private expandedNodeList: Set<number> = new Set();
  private logLevel: string[] = ['Debug', 'Info', 'Warn', 'Error', 'Fatal'];
  private selectTreeDepth: number = 0;
  private currentSelection: SelectionParam | undefined;

  set data(systemLogDetailParam: SelectionParam) {
    if (systemLogDetailParam === this.currentSelection) {
      return;
    }
    this.currentSelection = systemLogDetailParam;
    this.systemLogSource = [];
    this.expandedNodeList.clear();
    this.expansionUpIcon!.name = 'up';
    this.expansionDownIcon!.name = 'down';
    this.logSummaryTable!.innerHTML = '';
    this.summaryDownLoadTbl!.recycleDataSource = [];
    if (this.summaryDownLoadTbl) {
      this.summaryDownLoadTbl.recycleDataSource = [];
    }
    this.systemLogSource = systemLogDetailParam.hiLogSummary;
    if (this.systemLogSource?.length !== 0 && systemLogDetailParam) {
      this.refreshRowNodeTable();
    }
  }

  initElements(): void {
    this.logSummaryTable = this.shadowRoot?.querySelector<HTMLDivElement>('#tab-summary');
    this.summaryDownLoadTbl = this.shadowRoot?.querySelector<LitTable>('#tb-hilog-summary');
    this.expansionDiv = this.shadowRoot?.querySelector<HTMLDivElement>('.expansion-div');
    this.expansionUpIcon = this.shadowRoot?.querySelector<LitIcon>('.expansion-up-icon');
    this.expansionDownIcon = this.shadowRoot?.querySelector<LitIcon>('.expansion-down-icon');
    let summaryTreeLevel: string[] = ['Level', '/Process', '/Tag', '/Message'];
    this.shadowRoot?.querySelectorAll<HTMLLabelElement>('.head-label').forEach((summaryTreeHead) => {
      summaryTreeHead.addEventListener('click', () => {
        this.selectTreeDepth = summaryTreeLevel.indexOf(summaryTreeHead.textContent!);
        this.expandedNodeList.clear();
        this.refreshSelectDepth(this.logTreeNodes);
        this.refreshRowNodeTable(true);
      });
    });
  }

  initHtml(): string {
    return `<style>
        :host{
          padding: 10px 10px;
          display: flex;
          flex-direction: column;
        }
        .tree-row-tr, .tab-summary-head {
          display: flex;
          height: 30px;
          line-height: 30px;
          align-items: center;
          background-color: white;
        }
        .tree-row-tr:hover {
          background-color: #DEEDFF;
        }
        td, .head-label, .head-count {
          white-space: nowrap;
          overflow: hidden;
        }
        .head-label, .head-count {
          font-weight: bold;
        }
        .count-column-td, .head-count {
          margin-left: auto;
          margin-right: 40%;
        }
        .row-name-td {
          white-space: nowrap;
          overflow-x: scroll;
          overflow-y: hidden;
          display: inline-block;
          margin-right: 15px;
        }
        .row-name-td::-webkit-scrollbar {
          display: none;
        }
        </style>
        <div class="tab-summary-head">
          <div style="justify-content: flex-start; display: flex">
            <div class="expansion-div" style="display: grid;">
              <lit-icon class="expansion-up-icon" name="up"></lit-icon>
              <lit-icon class="expansion-down-icon" name="down"></lit-icon>
            </div>
            <label class="head-label" style="cursor: pointer;">Level</label>
            <label class="head-label" style="cursor: pointer;">/Process</label>
            <label class="head-label" style="cursor: pointer;">/Tag</label>
            <label class="head-label" style="cursor: pointer;">/Message</label>
          </div>
          <label class="head-count">Count</label> 
        </div>
        <div id="tab-summary" style="overflow: auto"></div>
        <lit-table id="tb-hilog-summary" style="display: none" tree>
          <lit-table-column title="Level/Process/Tag/Message" data-index="logName" key="logName"></lit-table-column>
          <lit-table-column title="Count" data-index="count" key="count"></lit-table-column>
        </lit-table>
        `;
  }

  connectedCallback() {
    super.connectedCallback();
    new ResizeObserver(() => {
      this.refreshRowNodeTable();
    }).observe(this.parentElement!);
    this.expansionDiv?.addEventListener('click', this.expansionClickEvent);
  }

  disconnectedCallback() {
    super.disconnectedCallback();
    this.expansionDiv?.removeEventListener('click', this.expansionClickEvent);
  }

  expansionClickEvent = () => {
    this.expandedNodeList.clear();
    if (this.expansionUpIcon?.name === 'down') {
      this.selectTreeDepth = 0;
      this.expansionUpIcon!.name = 'up';
      this.expansionDownIcon!.name = 'down';
    } else {
      this.selectTreeDepth = 4;
      this.expansionUpIcon!.name = 'down';
      this.expansionDownIcon!.name = 'up';
    }
    this.refreshSelectDepth(this.logTreeNodes);
    this.refreshRowNodeTable(true);
  };

  private refreshSelectDepth(logTreeNodes: LogTreeNode[]) {
    logTreeNodes.forEach((item) => {
      if (item.depth < this.selectTreeDepth) {
        this.expandedNodeList.add(item.id);
        if (item.children.length > 0) {
          this.refreshSelectDepth(item.children);
        }
      }
    });
  }

  initTabSheetEl(parentTabEl: HTMLElement) {
    this.parentTabEl = parentTabEl;
  }

  private getLevelName(level: string): string {
    switch (level) {
      case 'D':
        return 'Debug';
      case 'I':
        return 'Info';
      case 'W':
        return 'Warn';
      case 'E':
        return 'Error';
      case 'F':
        return 'Fatal';
      default:
        return 'Other';
    }
  }

  private createRowNodeTableEL(rowNodeList: LogTreeNode[], rowColor: string = ''): DocumentFragment {
    let unitPadding: number = 20;
    let leftPadding: number = 5;
    let tableFragmentEl: DocumentFragment = document.createDocumentFragment();
    rowNodeList.forEach((rowNode) => {
      let tableRowEl: HTMLElement = document.createElement('tr');
      tableRowEl.className = 'tree-row-tr';
      let leftSpacingEl: HTMLElement = document.createElement('td');
      leftSpacingEl.style.paddingLeft = `${rowNode.depth * unitPadding + leftPadding}px`;
      tableRowEl.appendChild(leftSpacingEl);
      this.addToggleIconEl(rowNode, tableRowEl);
      let rowNodeTextEL: HTMLElement = document.createElement('td');
      rowNodeTextEL.textContent = rowNode.logName!;
      rowNodeTextEL.className = 'row-name-td';
      tableRowEl.appendChild(rowNodeTextEL);
      let countEL: HTMLElement = document.createElement('td');
      countEL.textContent = rowNode.count.toString();
      countEL.className = 'count-column-td';
      if (rowNode.depth === 0) {
        rowNodeTextEL.style.color = ColorUtils.getHilogColor(rowNode.logName!);
        countEL.style.color = ColorUtils.getHilogColor(rowNode.logName!);
      } else {
        rowNodeTextEL.style.color = rowColor;
        countEL.style.color = rowColor;
      }
      tableRowEl.appendChild(countEL);
      tableFragmentEl.appendChild(tableRowEl);
      if (rowNode.children && this.expandedNodeList.has(rowNode.id)) {
        let documentFragment = this.createRowNodeTableEL(rowNode.children, countEL.style.color);
        tableFragmentEl.appendChild(documentFragment);
      }
    });
    return tableFragmentEl;
  }

  private addToggleIconEl(rowNode: LogTreeNode, tableRowEl: HTMLElement): void {
    let toggleIconEl: HTMLElement = document.createElement('td');
    let expandIcon = document.createElement('lit-icon');
    expandIcon.classList.add('tree-icon');
    if (rowNode.children && rowNode.children.length > 0) {
      toggleIconEl.appendChild(expandIcon);
      // @ts-ignore
      expandIcon.name = this.expandedNodeList.has(rowNode.id) ? 'minus-square' : 'plus-square';
      toggleIconEl.classList.add('expand-icon');
      toggleIconEl.addEventListener('click', () => {
        let scrollTop = this.logSummaryTable?.scrollTop ?? 0;
        this.changeNode(rowNode.id);
        this.logSummaryTable!.scrollTop = scrollTop;
      });
    }
    tableRowEl.appendChild(toggleIconEl);
  }

  private changeNode(currentNode: number): void {
    if (this.expandedNodeList.has(currentNode)) {
      this.expandedNodeList['delete'](currentNode);
    } else {
      this.expandedNodeList.add(currentNode);
    }
    this.refreshRowNodeTable();
  }

  private refreshRowNodeTable(useCacheRefresh: boolean = false): void {
    this.logSummaryTable!.innerHTML = '';
    if (this.logSummaryTable && this.parentTabEl) {
      this.logSummaryTable.style.height = `${this.parentTabEl!.clientHeight - 30}px`;
    }
    if (!useCacheRefresh) {
      this.logTreeNodes = this.buildTreeTblNodes(this.systemLogSource);
      if (this.logTreeNodes.length > 0) {
        this.summaryDownLoadTbl!.recycleDataSource = this.logTreeNodes;
      } else {
        this.summaryDownLoadTbl!.recycleDataSource = [];
      }
    }
    let fragment = this.createRowNodeTableEL(this.logTreeNodes);
    this.logSummaryTable!.appendChild(fragment);
  }

  private buildTreeTblNodes(logTreeNodes: LogStruct[]): LogTreeNode[] {
    let id = 0;
    let root: LogTreeNode = { id: id, depth: 0, children: [], logName: 'All', count: 0 };
    logTreeNodes.forEach((item) => {
      id++;
      let levelName = this.getLevelName(item.level!);
      let levelNode = root.children.find((node) => node.logName === levelName);
      if (!levelNode) {
        id++;
        levelNode = { id: id, depth: 0, children: [], logName: levelName, count: 0 };
        root.children.push(levelNode);
      }
      let processNode = levelNode.children.find((node) => node.logName === item.processName);
      if (!processNode) {
        id++;
        processNode = { id: id, depth: 1, children: [], logName: item.processName, count: 0 };
        levelNode.children.push(processNode);
      }
      let tagNode = processNode.children.find((node) => node.logName === item.tag);
      if (!tagNode) {
        id++;
        tagNode = { id: id, depth: 2, children: [], logName: item.tag, count: 0 };
        processNode.children.push(tagNode);
      }
      id++;
      tagNode.children.push({ id: id, depth: 3, children: [], logName: item.context, count: 1 });
      tagNode.count++;
      processNode.count++;
      levelNode.count++;
      root.count++;
    });
    return root.children.sort((leftData, rightData) => {
      return this.logLevel.indexOf(leftData.logName!) - this.logLevel.indexOf(rightData.logName!);
    });
  }
}

export interface LogTreeNode {
  id: number;
  depth: number;
  children: LogTreeNode[];
  logName: string | undefined;
  count: number;
}
