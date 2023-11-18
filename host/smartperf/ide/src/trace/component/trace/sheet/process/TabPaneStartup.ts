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
import { getTabStartups } from '../../../../database/SqlLite.js';
import { log } from '../../../../../log/Log.js';
import { getProbablyTime } from '../../../../database/logic-worker/ProcedureLogicWorkerCommon.js';
import { resizeObserver } from '../SheetUtils.js';
import { AppStartupStruct } from '../../../../database/ui-worker/ProcedureWorkerAppStartup.js';

interface StartupTreeItem {
  name: string;
  step: number;
  dur: number;
  durStr: string;
  ratio: string;
  children: StartupTreeItem[] | undefined;
}

@element('tabpane-startup')
export class TabPaneStartup extends BaseElement {
  private startupTbl: LitTable | null | undefined;
  private range: HTMLLabelElement | null | undefined;
  private startupSource: Array<StartupTreeItem> = [];
  private currentSelectionParam: SelectionParam | undefined;

  set data(startupParam: SelectionParam | any) {
    if (this.currentSelectionParam === startupParam) {
      return;
    }
    this.currentSelectionParam = startupParam;
    //@ts-ignore
    this.startupTbl?.shadowRoot?.querySelector('.table')?.style?.height = this.parentElement!.clientHeight - 45 + 'px';
    this.range!.textContent =
      'Selected range: ' + ((startupParam.rightNs - startupParam.leftNs) / 1000000.0).toFixed(5) + ' ms';
    this.startupTbl!.loading = true;
    getTabStartups(startupParam.processIds, startupParam.leftNs, startupParam.rightNs).then(
      (result: AppStartupStruct[]) => {
        this.startupTbl!.loading = false;
        if (result != null && result.length > 0) {
          log('getTabStartups result  size : ' + result.length);
          let map: Map<number, StartupTreeItem> = new Map<number, StartupTreeItem>();
          result.forEach((item) => {
            let startup = {
              name: AppStartupStruct.getStartupName(item.startName),
              dur: item.dur || 0,
              durStr: getProbablyTime(item.dur || 0),
              ratio: `0%`,
              step: item.startName || 0,
              children: [],
            };
            if (map.has(item.pid!)) {
              let ps = map.get(item.pid!);
              if (ps && ps.children) {
                ps.dur += item.dur || 0;
                ps.children!.push(startup);
              }
            } else {
              map.set(item.pid!, {
                name: item.process || `Process ${ item.pid }`,
                dur: item.dur || 0,
                durStr: '',
                ratio: `100%`,
                step: 0,
                children: [startup],
              });
            }
          });
          let startups = Array.from(map.values());
          startups.forEach((it) => {
            it.durStr = getProbablyTime(it.dur);
            if (it.dur === 0) {
              it.ratio = '0%';
            }
            it.children!.forEach((child) => {
              if (it.dur === 0) {
                child.ratio = '0%';
              } else {
                child.ratio = ((child.dur * 100) / (it.dur)).toFixed(2) + '%';
              }
            });
          });
          this.startupSource = startups;
          this.startupTbl!.recycleDataSource = this.startupSource;
        } else {
          this.startupSource = [];
          this.startupTbl!.recycleDataSource = [];
        }
      }
    );
  }

  initElements(): void {
    this.startupTbl = this.shadowRoot?.querySelector<LitTable>('#tb-startup');
    this.range = this.shadowRoot?.querySelector('#startup-time-range');
    this.startupTbl!.addEventListener('column-click', (evt: any) => {
      this.sortByColumn(evt.detail);
    });
  }

  connectedCallback(): void {
    super.connectedCallback();
    resizeObserver(this.parentElement!, this.startupTbl!);
  }

  initHtml(): string {
    return `
        <style>
        .startup-table{
          flex-direction: row;
          margin-bottom: 5px;
        }
        :host{
            display: flex;
            flex-direction: column;
            padding: 10px 10px;
        }
        </style>
        <div class="startup-table" style="display: flex;height: 20px;align-items: center;flex-direction: row;margin-bottom: 5px">
            <div style="flex: 1"></div>
            <label id="startup-time-range"  style="width: auto;text-align: end;font-size: 10pt;">Selected range:0.0 ms</label>
        </div>
        <div style="overflow: auto">
            <lit-table id="tb-startup" style="height: auto" tree>
                <lit-table-column width="600px" title="Process / Startup"  data-index="name" key="name"  align="flex-start" >
                </lit-table-column>
                <lit-table-column width="200px" title="Duration" data-index="durStr" key="durStr"  align="flex-start" order >
                </lit-table-column>
                <lit-table-column width="200px" title="%" data-index="ratio" key="ratio"  align="flex-start" order >
                </lit-table-column>
            </lit-table>
        </div>
        `;
  }

  sortByColumn(startupDetail: any): void {
    let compare = (startupA: StartupTreeItem, startupB: StartupTreeItem): number => {
      if (startupDetail.sort === 0) {
        return startupA.step - startupB.step;
      } else if (startupDetail.sort === 1) {
        return startupA.dur - startupB.dur;
      } else {
        return startupB.dur - startupA.dur;
      }
    };
    this.startupSource.forEach((startup) => {
      startup.children?.sort(compare);
    });
    this.startupSource.sort(compare);
    this.startupTbl!.recycleDataSource = this.startupSource;
  }
}
