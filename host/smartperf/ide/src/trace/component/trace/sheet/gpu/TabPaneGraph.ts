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
import { queryGpuDataTab } from '../../../../database/SqlLite.js';
import { log } from '../../../../../log/Log.js';
import { getProbablyTime } from '../../../../database/logic-worker/ProcedureLogicWorkerCommon.js';
import { resizeObserver } from '../SheetUtils.js';
import { Utils } from '../../base/Utils.js';
import { MemoryConfig } from '../../../../bean/MemoryConfig.js';

interface Graph {
  startTs: number;
  startTsStr?: string;
  size: number;
  sizeStr?: string;
}

@element('tabpane-gpu-graph')
export class TabPaneGpuGraph extends BaseElement {
  private graphTbl: LitTable | null | undefined;
  private range: HTMLLabelElement | null | undefined;
  private graphSource: Array<Graph> = [];
  private currentSelectionParam: SelectionParam | undefined;

  set data(graphParam: SelectionParam | any) {
    if (this.currentSelectionParam === graphParam) {
      return;
    }
    this.currentSelectionParam = graphParam;
    //@ts-ignore
    this.graphTbl?.shadowRoot?.querySelector('.table')?.style?.height = this.parentElement!.clientHeight - 45 + 'px';
    this.range!.textContent =
      'Selected range: ' + ((graphParam.rightNs - graphParam.leftNs) / 1000000.0).toFixed(5) + ' ms';
    this.graphTbl!.loading = true;
    queryGpuDataTab(
      MemoryConfig.getInstance().iPid,
      graphParam.leftNs,
      graphParam.rightNs,
      MemoryConfig.getInstance().snapshotDur,
      "'mem.graph_pss'"
    ).then((result) => {
      this.graphTbl!.loading = false;
      log('queryGpuDataTab result size : ' + result.length);
      if (result.length > 0) {
        result.forEach((it: Graph) => {
          it.startTsStr = getProbablyTime(it.startTs);
          it.sizeStr = Utils.getBinaryByteWithUnit(it.size);
        });
        this.graphSource = result;
        this.graphTbl!.recycleDataSource = this.graphSource;
      } else {
        this.graphSource = [];
        this.graphTbl!.recycleDataSource = [];
      }
    });
  }

  initElements(): void {
    this.graphTbl = this.shadowRoot?.querySelector<LitTable>('#tb-graph');
    this.range = this.shadowRoot?.querySelector('#graph-time-range');
  }

  connectedCallback(): void {
    super.connectedCallback();
    resizeObserver(this.parentElement!, this.graphTbl!);
  }

  initHtml(): string {
    return `
        <style>
        .graph-table{
          flex-direction: row;
          margin-bottom: 5px;
        }
        :host{
            display: flex;
            flex-direction: column;
            padding: 10px 10px;
        }
        </style>
        <div class="graph-table" style="display: flex;height: 20px;align-items: center;flex-direction: row;margin-bottom: 5px">
            <div style="flex: 1"></div>
            <label id="graph-time-range"  style="width: auto;text-align: end;font-size: 10pt;">Selected range:0.0 ms</label>
        </div>
        <div style="overflow: auto">
            <lit-table id="tb-graph" style="height: auto" tree>
                <lit-table-column width="600px" title="Timestamp"  data-index="startTsStr" key="startTsStr" align="flex-start" >
                </lit-table-column>
                <lit-table-column width="200px" title="GraphPSS" data-index="sizeStr" key="sizeStr"  align="flex-start">
                </lit-table-column>
            </lit-table>
        </div>
        `;
  }
}
