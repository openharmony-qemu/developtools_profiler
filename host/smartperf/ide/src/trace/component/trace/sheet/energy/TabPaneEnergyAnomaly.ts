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
import { SpHiSysEventChart } from '../../../chart/SpHiSysEventChart.js';
import { LitTable } from '../../../../../base-ui/table/lit-table.js';

import { queryAnomalyDetailedData } from '../../../../database/SqlLite.js';
import { SelectionParam } from '../../../../bean/BoxSelection.js';
import { EnergyAnomalyStruct } from '../../../../database/ui-worker/ProcedureWorkerEnergyAnomaly.js';
import { resizeObserver } from '../SheetUtils.js';

@element('tabpane-anomaly-details')
export class TabPaneEnergyAnomaly extends BaseElement {
  private tblAnomaly: LitTable | null | undefined;
  private static KEY_INDEX: number = 2;
  private static VALUE_INDEX: number = 3;
  set data(selectionAnomaly: SelectionParam) {
    let div: HTMLElement | null | undefined = this?.shadowRoot?.querySelector('#anomaly-details');
    let htmlText = '';
    if (selectionAnomaly) {
      this.queryAnomalyTableData(selectionAnomaly.leftNs, selectionAnomaly.rightNs).then((bean) => {
        let filterAppMap = new Map();
        for (let index = 0; index < bean.length; index++) {
          let findAppNameIndex = -1;
          // @ts-ignore
          let values = Object.values(bean[index]);
          if (values[TabPaneEnergyAnomaly.VALUE_INDEX]) {
            let apps = values[TabPaneEnergyAnomaly.VALUE_INDEX].split(',');
            for (let appIndex = 0; appIndex < apps.length; appIndex++) {
              if (apps.indexOf(SpHiSysEventChart.app_name) != -1) {
                findAppNameIndex = apps.indexOf(SpHiSysEventChart.app_name);
                filterAppMap.set(values[0] + values[1], findAppNameIndex);
                break;
              }
            }
            if (values[TabPaneEnergyAnomaly.KEY_INDEX] == 'APPNAME') {
              //ts+eventName : appNameIndex
              filterAppMap.set(values[0] + values[1], findAppNameIndex);
            }
          }
        }
        let set = new Set();
        for (let index = 0; index < bean.length; index++) {
          // @ts-ignore
          let values = Object.values(bean[index]);
          let findAppNameIndex = -1;
          if (filterAppMap.get(values[0] + values[1]) == -1) {
            continue;
          } else {
            findAppNameIndex = filterAppMap.get(values[0] + values[1]);
          }
          if (!set.has(values[0])) {
            set.add(values[0]);
            htmlText +=
              '<div><table' +
              ' style=\'border:none;table-layout:fixed;word-break:break-all\' cellspacing="5"; cellpadding="5"><tbody>' +
              '<tr><td colspan="5" style=\'font-weight: 700;font-size: 14px\'>' +
              values[1] +
              '</td></tr>';
          }
          // @ts-ignore
          if (set.has(Object.values(bean[index])[0])) {
            let appValues = values[TabPaneEnergyAnomaly.VALUE_INDEX].split(',');
            htmlText +=
              "<tr><td style='font-weight: 400;font-size: 14px;opacity:0.9;width:150px;'>" +
              values[TabPaneEnergyAnomaly.KEY_INDEX] +
              "</td><td style='font-weight: 400;font-size: 14px;opacity:0.6;width:250px;'>" +
              (findAppNameIndex >= 0
                ? appValues.length > 1
                  ? appValues[findAppNameIndex]
                  : values[TabPaneEnergyAnomaly.VALUE_INDEX]
                : values[TabPaneEnergyAnomaly.VALUE_INDEX]) +
              TabPaneEnergyAnomaly.getUnit(values[TabPaneEnergyAnomaly.KEY_INDEX]) +
              "</td><td style='width:100px'></td>";
          }
          if (index + 1 < bean.length) {
            // @ts-ignore
            let nextValues = Object.values(bean[index + 1]);
            let appValues = nextValues[TabPaneEnergyAnomaly.VALUE_INDEX].split(',');
            if (set.has(nextValues[0])) {
              htmlText +=
                "<td style='font-weight: 400;font-size: 14px;opacity:0.9;width:150px;'>" +
                nextValues[TabPaneEnergyAnomaly.KEY_INDEX] +
                "</td><td style='font-weight: 400;font-size: 14px;opacity:0.6;width:250px;'>" +
                (findAppNameIndex >= 0
                  ? appValues.length > 1
                    ? appValues[findAppNameIndex]
                    : nextValues[TabPaneEnergyAnomaly.VALUE_INDEX]
                  : nextValues[TabPaneEnergyAnomaly.VALUE_INDEX]) +
                TabPaneEnergyAnomaly.getUnit(nextValues[TabPaneEnergyAnomaly.KEY_INDEX]) +
                '</td></tr>';
            } else {
              htmlText += '</tr>';
              htmlText += '</tbody></table></div>';
              continue;
            }
            index++;
          }
        }
        div!.innerHTML = htmlText;
      });
    }
  }

  static getUnit(value: any) {
    if (value == 'DURATION') {
      return ' ms';
    } else if (value == 'ENERGY' || value == 'BGENERGY' || value == 'BATTERY_GAS_GUAGE') {
      return ' mAh';
    } else if (value == 'BGUSAGE') {
      return ' s';
    }
    return '';
  }

  /**
   * 查询出 异常详细信息
   * @param data
   */
  async queryAnomalyTableData(startTime: number, endTime: number): Promise<Array<EnergyAnomalyStruct>> {
    let anomalyTableData = await queryAnomalyDetailedData(startTime, endTime);
    return anomalyTableData;
  }

  initElements(): void {
    this.tblAnomaly = this.shadowRoot?.querySelector<LitTable>('#anomalyselectionTbl');
    this.tblAnomaly?.addEventListener('column-click', (ev: any) => {});
  }

  connectedCallback() {
    super.connectedCallback();
    resizeObserver(this.parentElement!, this.tblAnomaly!);
  }

  initHtml(): string {
    return `
        <style>
            .anomaly-title{
                display: flex;
                width: 95%;
                background: var(--dark-background,#ffffff);
                top: 0;
                position: sticky;
            }
            .anomaly-title h2{
                padding: 0 10px;
                font-size: 16px;
                font-weight: 400;
                width: 50%;
                visibility: visible;
            }
            .scroll-area{
                overflow-y: auto;
                height: auto;
                display: flex;
                
            }
            .left-table{
                padding: 0 10px;
                width: 50%;
            }
            .anomaly-table{
                height: auto;
            }
        </style>
        <div style="width: 100%;height: auto;position: relative">
            <div id="anomaly-details" class="anomaly-title" style="margin-left: 12px;display: block">
                <h2 id="leftTitle"></h2>
            </div>
            <div class="scroll-area">
                <div class="left-table">
                    <lit-table id="anomalyselectionTbl" no-head class="anomaly-table">
                        <lit-table-column key="name" align="flex-start"  width="180px" title="name" data-index="name" >
                        </lit-table-column>
                        <lit-table-column key="value" align="flex-start" title="value" data-index="value">
                        </lit-table-column>
                    </lit-table>
                </div>
            </div>
        </div>
        `;
  }
}
