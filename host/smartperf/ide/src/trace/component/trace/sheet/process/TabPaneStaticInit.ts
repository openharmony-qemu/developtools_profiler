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
import { getTabStaticInit } from '../../../../database/SqlLite.js';
import { log } from '../../../../../log/Log.js';
import { getProbablyTime } from '../../../../database/logic-worker/ProcedureLogicWorkerCommon.js';
import { resizeObserver } from '../SheetUtils.js';
import { SoStruct } from '../../../../database/ui-worker/ProcedureWorkerSoInit.js';

interface SoTreeItem {
  name: string;
  dur: number;
  durStr: string;
  ratio: string;
  children: SoTreeItem[] | undefined;
}

@element('tabpane-static-init')
export class TabPaneStaticInit extends BaseElement {
  private staticinitTbl: LitTable | null | undefined;
  private range: HTMLLabelElement | null | undefined;
  private staticinitSource: Array<SoTreeItem> = [];
  private currentSelectionParam: SelectionParam | undefined;

  set data(staticinitParam: SelectionParam | any) {
    if (this.currentSelectionParam === staticinitParam) {
      return;
    }
    this.currentSelectionParam = staticinitParam;
    //@ts-ignore
    this.staticinitTbl?.shadowRoot?.querySelector('.table')?.style?.height =
      this.parentElement!.clientHeight - 45 + 'px';
    this.range!.textContent =
      'Selected range: ' + ((staticinitParam.rightNs - staticinitParam.leftNs) / 1000000.0).toFixed(5) + ' ms';
    this.staticinitTbl!.loading = true;
    getTabStaticInit(staticinitParam.processIds, staticinitParam.leftNs, staticinitParam.rightNs).then(
      (result: SoStruct[]) => {
        this.staticinitTbl!.loading = false;
        if (result != null && result.length > 0) {
          log('getTabStaticInit  result size : ' + result.length);
          let map: Map<number, SoTreeItem> = new Map<number, SoTreeItem>();
          result.forEach((item) => {
            let so: SoTreeItem = {
              name: (item.soName || '[NULL]').replace('dlopen: ', ''),
              dur: item.dur || 0,
              durStr: getProbablyTime(item.dur || 0),
              ratio: `0%`,
              children: [],
            };
            if (map.has(item.pid!)) {
              let ps = map.get(item.pid!);
              if (ps && ps.children) {
                ps.dur += item.dur || 0;
                ps.children!.push(so);
              }
            } else {
              map.set(item.pid!, {
                name: item.process || `Process ${ item.pid }`,
                dur: item.dur || 0,
                durStr: '',
                ratio: `100%`,
                children: [so],
              });
            }
          });
          let soArr = Array.from(map.values());
          soArr.forEach((it) => {
            it.durStr = getProbablyTime(it.dur);
            it.children!.forEach((child) => {
              child.ratio = ((child.dur * 100) / it.dur).toFixed(2) + '%';
            });
          });
          this.staticinitSource = soArr;
          this.staticinitTbl!.recycleDataSource = this.staticinitSource;
        } else {
          this.staticinitSource = [];
          this.staticinitTbl!.recycleDataSource = [];
        }
      }
    );
  }

  initElements(): void {
    this.staticinitTbl = this.shadowRoot?.querySelector<LitTable>('#tb-staticinit');
    this.range = this.shadowRoot?.querySelector('#staticinit-time-range');
    this.staticinitTbl!.addEventListener('column-click', (evt: any) => {
      this.sortByColumn(evt.detail);
    });
  }

  connectedCallback(): void {
    super.connectedCallback();
    resizeObserver(this.parentElement!, this.staticinitTbl!);
  }

  initHtml(): string {
    return `
        <style>
        .staticinit-table{
          flex-direction: row;
          margin-bottom: 5px;
        }
        :host{
            display: flex;
            flex-direction: column;
            padding: 10px 10px;
        }
        </style>
        <div class="staticinit-table" style="display: flex;height: 20px;align-items: center;flex-direction: row;margin-bottom: 5px">
            <div style="flex: 1"></div>
            <label id="staticinit-time-range"  style="width: auto;text-align: end;font-size: 10pt;">Selected range:0.0 ms</label>
        </div>
        <div style="overflow: auto">
            <lit-table id="tb-staticinit" style="height: auto" tree>
                <lit-table-column width="700px" title="Process / Lib"  data-index="name" key="name"  align="flex-start" order>
                </lit-table-column>
                <lit-table-column width="200px" title="Duration" data-index="durStr" key="durStr"  align="flex-start" order >
                </lit-table-column>
            </lit-table>
        </div>
        `;
  }

  sortByColumn(soDetail: any): void {
    let compare = (soA: SoTreeItem, soB: SoTreeItem): number => (soDetail.sort === 1 ? soA.dur - soB.dur : soB.dur - soA.dur);
    this.staticinitSource.forEach((it) => it.children?.sort(compare));
    this.staticinitSource.sort(compare);
    this.staticinitTbl!.recycleDataSource = this.staticinitSource;
  }
}
