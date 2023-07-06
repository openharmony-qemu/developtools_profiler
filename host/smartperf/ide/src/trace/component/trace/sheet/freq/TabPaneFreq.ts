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
import { ColorUtils } from '../../base/ColorUtils.js';
import { Utils } from '../../base/Utils.js';
import { resizeObserver } from '../SheetUtils.js';

@element('tabpane-freq')
export class TabPaneFreq extends BaseElement {
  private freqTbl: LitTable | null | undefined;

  set data(freqData: any) {
    if (freqData) {
      this.freqTbl!.dataSource = [
        {
          startNS: Utils.getTimeString(freqData.startNS >= 0 ? freqData.startNS : 0),
          absoluteTime: (freqData.startNS + (window as any).recordStartNS) / 1000000000,
          dur: Utils.getProbablyTime(freqData.dur),
          freq: `${ColorUtils.formatNumberComma(freqData.value!)} kHz`,
          cpu: `Cpu ${freqData.cpu}`,
        },
      ];
    }
  }

  initElements(): void {
    this.freqTbl = this.shadowRoot?.querySelector<LitTable>('#tb-freq');
  }

  connectedCallback() {
    super.connectedCallback();
    resizeObserver(this.parentElement!, this.freqTbl!);
  }

  initHtml(): string {
    return `
        <style>
        .freq-table{
            height: auto;
        }
        :host{
            padding: 10px 10px;
            display: flex;
            flex-direction: column;
        }
        </style>
        <lit-table id="tb-freq" class="freq-table">
            <lit-table-column class="freq-column" width="1fr" title="StartTime(Relative)" data-index="startNS" key="startNS" align="flex-start">
            </lit-table-column>
            <lit-table-column class="freq-column" width="1fr" title="StartTime(Absolute)" data-index="absoluteTime" key="absoluteTime" align="flex-start">
            </lit-table-column>
            <lit-table-column class="freq-column" width="1fr" title="Duration" data-index="dur" key="dur" align="flex-start" >
            </lit-table-column>
            <lit-table-column class="freq-column" width="1fr" title="Cpu" data-index="cpu" key="cpu" align="flex-start" >
            </lit-table-column>
            <lit-table-column class="freq-column" width="1fr" title="Freq" data-index="freq" key="freq" align="flex-start" >
            </lit-table-column>
        </lit-table>
        `;
  }
}
