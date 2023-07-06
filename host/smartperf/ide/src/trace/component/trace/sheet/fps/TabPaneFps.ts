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
import { getTabFps } from '../../../../database/SqlLite.js';
import { Utils } from '../../base/Utils.js';
import { log } from '../../../../../log/Log.js';
import { resizeObserver } from '../SheetUtils.js';

@element('tabpane-fps')
export class TabPaneFps extends BaseElement {
  private fpsTbl: LitTable | null | undefined;
  private fpsRange: HTMLLabelElement | null | undefined;

  set data(fpsSelection: SelectionParam | any) {
    this.fpsRange!.textContent =
      'Selected range: ' + parseFloat(((fpsSelection.rightNs - fpsSelection.leftNs) / 1000000.0).toFixed(5)) + ' ms';
    getTabFps(fpsSelection.leftNs, fpsSelection.rightNs).then((fpsResult) => {
      if (fpsResult != null && fpsResult.length > 0) {
        log('getTabFps result size : ' + fpsResult.length);

        let index = fpsResult.findIndex((d) => d.startNS >= fpsSelection.leftNs);
        if (index != -1) {
          let arr = fpsResult.splice(index > 0 ? index - 1 : index);
          arr.map((e) => (e.timeStr = Utils.getTimeString(e.startNS)));
          this.fpsTbl!.recycleDataSource = arr;
        } else {
          let last = fpsResult[fpsResult.length - 1];
          last.timeStr = Utils.getTimeString(last.startNS);
          this.fpsTbl!.recycleDataSource = [last];
        }
      } else {
        this.fpsTbl!.recycleDataSource = [];
      }
    });
  }

  initElements(): void {
    this.fpsTbl = this.shadowRoot?.querySelector<LitTable>('#tb-fps');
    this.fpsRange = this.shadowRoot?.querySelector('#fps-time-range');
  }

  connectedCallback() {
    super.connectedCallback();
    resizeObserver(this.parentElement!, this.fpsTbl!);
  }

  initHtml(): string {
    return `
        <style>
        .fps-label{
            text-align: end;
        }
        :host{
            display: flex;
            flex-direction: column;
            padding: 10px 10px;
        }
        </style>
        <label id="fps-time-range" class="fps-label" style="width: 100%;height: 20px;font-size: 10pt;margin-bottom: 5px">Selected range:0.0 ms</label>
        <lit-table id="tb-fps" style="height: auto">
            <lit-table-column class="fps-column" width="1fr" title="Time" data-index="timeStr" key="timeStr" align="flex-start">
            </lit-table-column>
            <lit-table-column class="fps-column" width="1fr" title="FPS" data-index="fps" key="fps" align="flex-start" >
            </lit-table-column>
        </lit-table>
        `;
  }
}
