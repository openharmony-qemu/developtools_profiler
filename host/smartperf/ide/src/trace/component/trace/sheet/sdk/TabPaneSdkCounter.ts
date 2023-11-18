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
import { SelectionData, SelectionParam } from '../../../../bean/BoxSelection.js';
import { getTabSdkCounterData, getTabSdkCounterLeftData, queryStartTime } from '../../../../database/SqlLite.js';
import { LitTableColumn } from '../../../../../base-ui/table/lit-table-column';
import { Utils } from '../../base/Utils.js';
import { SpSystemTrace } from '../../../SpSystemTrace.js';
import { TabUtil } from './TabUtil.js';
import { resizeObserver } from '../SheetUtils.js';

@element('tabpane-sdk-counter')
export class TabPaneSdkCounter extends BaseElement {
  private tblSdkCounter: LitTable | null | undefined;
  private sdkRange: HTMLLabelElement | null | undefined;
  private keyList: Array<string> | undefined;
  private statDataArray: any = [];
  private columnMap: any = {};
  private sqlMap: Map<number, any> = new Map<number, any>();

  set data(valSdkCounter: SelectionParam | any) {
    this.sdkRange!.textContent =
      'Selected range: ' + ((valSdkCounter.rightNs - valSdkCounter.leftNs) / 1000000.0).toFixed(5) + ' ms';
    this.queryDataByDB(valSdkCounter);
  }

  initElements(): void {
    this.tblSdkCounter = this.shadowRoot?.querySelector<LitTable>('#tb-counter');
    this.sdkRange = this.shadowRoot?.querySelector('#sdk-counter-time-range');
    this.tblSdkCounter!.addEventListener('column-click', (evt) => {
      // @ts-ignore
      this.sortByColumn(evt.detail);
    });
  }

  connectedCallback() {
    super.connectedCallback();
    resizeObserver(this.parentElement!, this.tblSdkCounter!);
  }

  queryDataByDB(sdkVal: SelectionParam | any) {
    queryStartTime().then((res) => {
      let startTime = res[0].start_ts;
      this.parseJson(SpSystemTrace.SDK_CONFIG_MAP);
      let counters: Array<string> = [];
      let componentId: number = -1;
      for (let index = 0; index < sdkVal.sdkCounterIds.length; index++) {
        let values = sdkVal.sdkCounterIds[index].split('-');
        let value = values[0];
        componentId = Number(values[1]);
        counters.push(value);
      }
      let sqlObj = this.sqlMap.get(componentId);
      let sql = sqlObj.TabCounterLeftData;
      getTabSdkCounterLeftData(sql, sdkVal.leftNs + startTime, counters, componentId).then((res) => {
        let leftTime = res[res.length - 1].max_value - startTime;
        let sql = sqlObj.TabCounterData;
        getTabSdkCounterData(sql, startTime, leftTime, sdkVal.rightNs, counters, componentId).then((counterItem) => {
          this.keyList = [];
          this.tblSdkCounter!.innerHTML = '';
          this.statDataArray = [];
          if (counterItem.length != null && counterItem.length > 0) {
            for (let counterItemIndex = 0; counterItemIndex < counterItem.length; counterItemIndex++) {
              const dataResult = counterItem[counterItemIndex];
              let keys = Object.keys(dataResult);
              // @ts-ignore
              let values = Object.values(dataResult);
              let counterJsonText = '{';
              for (let counterKeyIndex = 0; counterKeyIndex < keys.length; counterKeyIndex++) {
                let counterKey = keys[counterKeyIndex];
                if (this.keyList.indexOf(counterKey) <= -1) {
                  this.keyList.push(counterKey);
                }
                let counterValue = values[counterKeyIndex];
                if (this.columnMap[counterKey] == 'TimeStamp') {
                  counterValue = Utils.getTimeString(Number(counterValue));
                } else if (this.columnMap[counterKey] == 'ClockTime') {
                  counterValue = Utils.getTimeStampHMS(Number(counterValue));
                } else if (this.columnMap[counterKey] == 'RangTime') {
                  counterValue = Utils.getDurString(Number(counterValue));
                } else if (this.columnMap[counterKey] == 'PercentType') {
                  counterValue = counterValue + '%';
                } else if (this.columnMap[counterKey] == 'CurrencyType') {
                  // @ts-ignore
                  counterValue = counterValue.toString().replace(/\B(?=(\d{3})+$)/g, ',');
                }
                if (typeof counterValue == 'string') {
                  counterValue = counterValue.replace(/</gi, '&lt;').replace(/>/gi, '&gt;');
                }
                counterJsonText += '"' + counterKey + '"' + ': ' + '"' + counterValue + '"';
                if (counterKeyIndex != keys.length - 1) {
                  counterJsonText += ',';
                } else {
                  counterJsonText += '}';
                }
              }
              this.statDataArray.push(JSON.parse(counterJsonText));
            }
            this.tblSdkCounter!.recycleDataSource = this.statDataArray;
          } else {
            this.tblSdkCounter!.recycleDataSource = [];
          }
          this.initDataElement();

          setTimeout(() => {
            this.tblSdkCounter!.recycleDataSource = this.statDataArray;
            new ResizeObserver(() => {
              if (this.parentElement?.clientHeight != 0) {
                this.tblSdkCounter!.style.height = '100%';
                this.tblSdkCounter!.reMeauseHeight();
              }
            }).observe(this.parentElement!);
          }, 200);
        });
      });
    });
  }

  parseJson(configMap: Map<number, string>): string {
    let keys = configMap.keys();
    for (let key of keys) {
      let counterConfigObject: any = configMap.get(key);
      if (counterConfigObject != undefined) {
        let configStr = counterConfigObject.jsonConfig;
        let configJson = JSON.parse(configStr);
        let counterTableConfig = configJson.tableConfig;
        if (counterTableConfig != null) {
          let showTypes = counterTableConfig.showType;
          for (let counterTypesIndex = 0; counterTypesIndex < showTypes.length; counterTypesIndex++) {
            let showType = showTypes[counterTypesIndex];
            let type = TabUtil.getTableType(showType);
            if (type == 'counter') {
              let selectSql = 'select ';
              for (let counterColumnsIndex = 0; counterColumnsIndex < showType.columns.length; counterColumnsIndex++) {
                this.columnMap[showType.columns[counterColumnsIndex].column] =
                  showType.columns[counterColumnsIndex].displayName;
                if (showType.columns[counterColumnsIndex].showType.indexOf(3) > -1) {
                  selectSql += showType.columns[counterColumnsIndex].column + ',';
                }
              }
              let counterLeftSql =
                'select max(ts) as max_value,counter_id from ' +
                showType.tableName +
                ' where ts <= $leftNs and counter_id in' +
                ' ($counters) group by counter_id order by max_value desc';
              let tabCounterDataSql =
                selectSql.substring(0, selectSql.length - 1) +
                ' from ' +
                showType.tableName +
                ' where counter_id in ($counters) and (ts - $startTime) between $leftNs and $rightNs';
              this.sqlMap.set(key, {
                TabCounterData: tabCounterDataSql,
                TabCounterLeftData: counterLeftSql,
              });
            }
          }
        }
      }
    }
    return '';
  }

  initDataElement() {
    if (this.keyList) {
      this.keyList.forEach((sdkCounterItemKey) => {
        let sdkCounterEl = document.createElement('lit-table-column') as LitTableColumn;
        sdkCounterEl.setAttribute('title', sdkCounterItemKey);
        sdkCounterEl.setAttribute('data-index', sdkCounterItemKey);
        sdkCounterEl.setAttribute('key', sdkCounterItemKey);
        sdkCounterEl.setAttribute('align', 'flex-start');
        sdkCounterEl.setAttribute('width', '1fr');
        sdkCounterEl.setAttribute('order', '');
        this.tblSdkCounter!.appendChild(sdkCounterEl);
      });
    }
  }

  initHtml(): string {
    return `
<style>
.sdk-counter-table{
    display: flex;
    margin-bottom: 5px;
}
:host{
    padding: 10px 10px;
    display: flex;
    flex-direction: column;
}
</style>
<div class="sdk-counter-table" style="height: 20px;align-items: center;flex-direction: row;">
            <stack-bar id="sdk-counter-stack-bar" style="flex: 1"></stack-bar>
            <label id="sdk-counter-time-range"  style="width: auto;text-align: end;font-size: 10pt;">Selected range:0.0 ms</label>
        </div>
<lit-table id="tb-counter" class="sdk-counter-tbl" style="height: auto">
</lit-table>
        `;
  }

  sortByColumn(counterDetail: any) {
    // @ts-ignore
    function compare(property, countreSort, type) {
      return function (aSdkCounter: SelectionData, bSdkCounter: SelectionData) {
        if (aSdkCounter.process == ' ' || bSdkCounter.process == ' ') {
          return 0;
        }
        if (type === 'number') {
          // @ts-ignore
          return countreSort === 2 ? parseFloat(bSdkCounter[property]) - parseFloat(aSdkCounter[property]) : parseFloat(aSdkCounter[property]) - parseFloat(bSdkCounter[property]);
        }
        // @ts-ignore
        if (bSdkCounter[property] > aSdkCounter[property]) {
          return countreSort === 2 ? 1 : -1;
        } else {
          // @ts-ignore
          if (bSdkCounter[property] == aSdkCounter[property]) {
            return 0;
          } else {
            return countreSort === 2 ? -1 : 1;
          }
        }
      };
    }

    if (counterDetail.key.indexOf('name') != -1) {
      this.statDataArray.sort(compare(counterDetail.key, counterDetail.sort, 'string'));
    } else {
      this.statDataArray.sort(compare(counterDetail.key, counterDetail.sort, 'number'));
    }
    this.tblSdkCounter!.recycleDataSource = this.statDataArray;
  }
}
