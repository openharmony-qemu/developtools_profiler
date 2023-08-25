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

import { SystemDetailsEnergy } from '../../../../bean/EnergyStruct.js';
import { BaseElement, element } from '../../../../../base-ui/BaseElement.js';
import { LitTable } from '../../../../../base-ui/table/lit-table.js';
import { SelectionParam } from '../../../../bean/BoxSelection.js';
import {
  querySysLocationDetailsData,
  querySysLockDetailsData,
  querySystemWorkData,
} from '../../../../database/SqlLite.js';
import { SpHiSysEventChart } from '../../../chart/SpHiSysEventChart.js';
import { resizeObserver } from '../SheetUtils.js';
import { LitSlicerTrack } from '../../../../../base-ui/slicer/lit-slicer.js';

@element('tabpane-system-details')
export class TabPaneSystemDetails extends BaseElement {
  private tblSystemDetails: LitTable | null | undefined;
  private detailsTbl: LitTable | null | undefined;
  private eventSource: Array<any> = [];
  private detailsSource: Array<any> = [];
  private boxDetails: HTMLDivElement | null | undefined;
  private slicerTrack: LitSlicerTrack | null | undefined;

  set data(valSystemDetails: SelectionParam | any) {
    this.slicerTrack!.style.visibility = 'hidden';
    this.queryDataByDB(valSystemDetails);
  }

  connectedCallback() {
    super.connectedCallback();
    resizeObserver(this.parentElement!, this.tblSystemDetails!);
  }

  initElements(): void {
    this.boxDetails = this.shadowRoot?.querySelector<HTMLDivElement>('.box-details');
    this.tblSystemDetails = this.shadowRoot?.querySelector<LitTable>('#tb-system-data');
    this.detailsTbl = this.shadowRoot?.querySelector<LitTable>('#tb-system-details-data');
    this.slicerTrack = this.shadowRoot?.querySelector<LitSlicerTrack>('lit-slicer-track');
    this.tblSystemDetails!.addEventListener('row-click', (e) => {
      this.detailsSource = [];
      // @ts-ignore
      let data = e.detail.data as SystemDetailsEnergy;
      this.convertData(data);
    });
  }

  convertData(data: SystemDetailsEnergy) {
    if (data.eventName === 'Event Name') {
      this.slicerTrack!.style.visibility = 'hidden';
      this.detailsTbl!.dataSource = [];
      this.boxDetails!.style.width = '100%';
    } else {
      this.slicerTrack!.style.visibility = 'visible';
      this.detailsSource.push({
        key: 'EVENT_NAME : ',
        value: data.eventName,
      });
      this.detailsSource.push({ key: 'PID : ', value: data.pid });
      this.detailsSource.push({ key: 'UID : ', value: data.uid });
      if (data.eventName === 'GNSS_STATE') {
        this.detailsSource.push({ key: 'STATE : ', value: data.state });
      } else if (data.eventName === 'POWER_RUNNINGLOCK') {
        this.detailsSource.push({ key: 'TYPE : ', value: data.type });
        this.detailsSource.push({ key: 'STATE : ', value: data.state });
        this.detailsSource.push({
          key: 'LOG_LEVEL : ',
          value: data.log_level,
        });
        this.detailsSource.push({ key: 'NAME : ', value: data.name });
        this.detailsSource.push({
          key: 'MESSAGE : ',
          value: data.message,
        });
        this.detailsSource.push({ key: 'TAG : ', value: data.tag });
      } else {
        this.detailsSource.push({ key: 'TYPE : ', value: data.type });
        this.detailsSource.push({
          key: 'WORK_ID : ',
          value: data.workId,
        });
        this.detailsSource.push({ key: 'NAME : ', value: data.name });
        this.detailsSource.push({
          key: 'INTERVAL : ',
          value: data.interval,
        });
      }
      this.detailsTbl!.dataSource = this.detailsSource;
      this.boxDetails!.style.width = '65%';
    }
    this.detailsTbl!.shadowRoot?.querySelectorAll<HTMLDivElement>('.tr').forEach((tr) => {
      tr.style.gridTemplateColumns = '120px 1fr';
    });
    this.detailsTbl!.shadowRoot?.querySelectorAll<HTMLDivElement>('.td').forEach((td) => {
      let item = td.getAttribute('title');
      td.style.fontSize = '14px';
      td.style.fontWeight = '400';
      if (item != null && item.indexOf(':') > -1) {
        td.style.opacity = '0.9';
        td.style.lineHeight = '16px';
      } else {
        td.style.opacity = '0.6';
        td.style.lineHeight = '20px';
      }
    });
  }

  queryDataByDB(val: SelectionParam | any) {
    Promise.all([
      querySystemWorkData(val.rightNs),
      querySysLockDetailsData(val.rightNs, 'POWER_RUNNINGLOCK'),
      querySysLocationDetailsData(val.rightNs, 'GNSS_STATE'),
    ]).then((result) => {
      let itemList: Array<any> = [];
      let systemWorkData = this.getSystemWorkData(result[0], val.leftNs, val.rightNs);
      if (systemWorkData.length > 0) {
        systemWorkData.forEach((item) => {
          itemList.push(item);
        });
      }
      let systemLockData = this.getSystemLockData(result[1], val.leftNs);
      if (systemLockData.length > 0) {
        systemLockData.forEach((item) => {
          itemList.push(item);
        });
      }
      let systemLocationData = this.getSystemLocationData(result[2], val.leftNs);
      if (systemLocationData.length > 0) {
        systemLocationData.forEach((item) => {
          itemList.push(item);
        });
      }
      itemList.sort((leftData: any, rightData: any) => {
        return leftData.ts - rightData.ts;
      });
      this.eventSource = [];
      this.eventSource.push({
        ts: 'Time',
        interval: 0,
        level: 0,
        name: '',
        state: 0,
        tag: '',
        type: '',
        uid: 0,
        pid: 0,
        workId: '',
        message: '',
        log_level: '',
        eventName: 'Event Name',
      });

      this.tblSystemDetails!.recycleDataSource = this.eventSource.concat(itemList);
      this.detailsTbl!.dataSource = [];
      this.boxDetails!.style.width = '100%';
      this.tblSystemDetails?.shadowRoot?.querySelectorAll<HTMLDivElement>('.td').forEach((td) => {
        td.style.fontSize = '14px';
        if (td.getAttribute('title') === 'Event Name' || td.getAttribute('title') === 'Time') {
          td.style.fontWeight = '700';
        } else {
          td.style.fontWeight = '400';
          td.style.opacity = '0.9';
          td.style.lineHeight = '16px';
        }
      });
    });
  }

  private getSystemWorkData(data: Array<any>, leftNs: number, rightNs: number) {
    let values = this.getConvertData(data);
    let lifeCycleData: Array<any> = [];
    let watchIndex: Array<string> = [];
    for (let index = 0; index < values.length; index++) {
      let filterData: any = values[index];
      if (filterData.name == SpHiSysEventChart.app_name) {
        if (filterData.eventName.indexOf('WORK_ADD') > -1) {
          watchIndex.push(filterData.workId);
          let number = watchIndex.indexOf(filterData.workId);
          lifeCycleData[number] = {
            startData: {},
            endData: {},
            rangeData: [],
          };
          lifeCycleData[number].startData = filterData;
          let virtualEndData = JSON.parse(JSON.stringify(filterData));
          virtualEndData.ts = rightNs;
          virtualEndData.eventName = 'WORK_REMOVE';
          lifeCycleData[number].endData = virtualEndData;
        } else if (filterData.eventName.indexOf('WORK_REMOVE') > -1) {
          let number = watchIndex.indexOf(filterData.workId);
          if (number > -1) {
            lifeCycleData[number].endData = filterData;
            watchIndex[number] = number + filterData.ts;
          }
        } else {
          let number = watchIndex.indexOf(filterData.workId);
          if (number > -1) {
            lifeCycleData[number].rangeData.push(filterData);
            let virtualEndData = JSON.parse(JSON.stringify(filterData));
            virtualEndData.ts = rightNs;
            virtualEndData.eventName = 'WORK_REMOVE';
            lifeCycleData[number].endData = virtualEndData;
          } else {
            if (filterData.eventName.indexOf('WORK_START') > -1) {
              lifeCycleData.push({
                startData: {},
                endData: {},
                rangeData: [],
              });
              watchIndex.push(filterData.workId);
              number = watchIndex.indexOf(filterData.workId);
              let virtualData = JSON.parse(JSON.stringify(filterData));
              if (filterData.ts > 0) {
                virtualData.ts = 0;
              } else {
                virtualData.ts = filterData.ts - 1;
              }
              virtualData.eventName = 'WORK_ADD';
              lifeCycleData[number].startData = virtualData;
              lifeCycleData[number].rangeData.push(filterData);
              let virtualEndData = JSON.parse(JSON.stringify(filterData));
              virtualEndData.ts = rightNs;
              virtualEndData.eventName = 'WORK_REMOVE';
              lifeCycleData[number].endData = virtualEndData;
            }
          }
        }
      }
    }
    let resultData: Array<any> = [];
    lifeCycleData.forEach((life: any) => {
      if (life.endData.ts >= leftNs) {
        let midData = life.rangeData;
        midData.forEach((rang: any, index: number) => {
          if (rang.eventName.indexOf('WORK_STOP') > -1 && rang.ts >= leftNs) {
            resultData.push(life.startData);
            if (index - 1 >= 0 && midData[index - 1].eventName.indexOf('WORK_START') > -1) {
              resultData.push(midData[index - 1]);
            }
            resultData.push(rang);
          }
        });
      }
    });
    return resultData;
  }

  private getSystemLocationData(data: Array<any>, leftNs: number) {
    let values = this.getConvertData(data);
    let fillMap: Map<any, any> = new Map<any, any>();
    let leftMap: Map<any, any> = new Map<any, any>();
    let watchIndex: Array<string> = [];
    for (let index = 0; index < values.length; index++) {
      let filterData: any = values[index];
      if (filterData.state.indexOf('start') > -1) {
        leftMap.set(filterData.pid, filterData);
        watchIndex.push(filterData.pid);
      } else {
        let i = watchIndex.indexOf(filterData.pid);
        if (i > -1) {
          fillMap.set(leftMap.get(filterData.pid), filterData);
          delete watchIndex[i];
          leftMap.delete(filterData.pid);
        }
      }
    }

    let locationData: Array<any> = [];
    fillMap.forEach((value, key) => {
      if (value.ts >= leftNs) {
        locationData.push(key);
        locationData.push(value);
      }
    });
    leftMap.forEach((value, key) => {
      locationData.push(value);
    });
    return locationData;
  }

  private getSystemLockData(data: Array<any>, leftNs: number) {
    let values = this.getConvertData(data);
    let watchIndex: Array<string> = [];
    let fillMap: Map<any, any> = new Map<any, any>();
    let leftMap: Map<any, any> = new Map<any, any>();
    for (let index = 0; index < values.length; index++) {
      let filterData: any = values[index];
      if (filterData.tag.indexOf('ADD') > -1) {
        leftMap.set(filterData.message, filterData);
        watchIndex.push(filterData.message);
      } else {
        let i = watchIndex.indexOf(filterData.message);
        if (i > -1) {
          fillMap.set(leftMap.get(filterData.message), filterData);
          delete watchIndex[i];
          leftMap.delete(filterData.message);
        }
      }
    }
    let lockData: Array<any> = [];
    fillMap.forEach((value, key) => {
      if (value.ts >= leftNs) {
        lockData.push(key);
        lockData.push(value);
      }
    });
    leftMap.forEach((value, key) => {
      lockData.push(value);
    });
    return lockData;
  }

  private getConvertData(data: Array<any>) {
    let convertItem: any = {};
    data.forEach((item: any) => {
      if (convertItem[item.ts + item.eventName] == undefined) {
        convertItem[item.ts + item.eventName] = {};
        convertItem[item.ts + item.eventName]['ts'] = item.ts;
        convertItem[item.ts + item.eventName]['eventName'] = item.eventName;
        convertItem[item.ts + item.eventName][item.appKey.toLocaleLowerCase()] = item.appValue;
      } else {
        convertItem[item.ts + item.eventName][item.appKey.toLocaleLowerCase()] = item.appValue;
      }
    });
    // @ts-ignore
    return Object.values(convertItem);
  }

  initHtml(): string {
    return `
        <style>
        :host{
            display: flex;
            flex-direction: column;
            padding: 10px 10px 0 10px;
        }
        .sys-detail-progress{
            bottom: 33px;
            position: absolute;
            height: 1px;
            left: 0;
            right: 0;
        }
        </style>
        <div class="sys-detail-content" style="display: flex;flex-direction: column">
            <div style="display: flex;flex-direction: row">
                <lit-slicer style="width:100%">
                    <div class="box-details" style="width: 100%">
                        <lit-table id="tb-system-data" style="height: auto">
                            <lit-table-column class="sys-detail-column" width="300px" title="" data-index="eventName" key="eventName"  align="flex-start" order>
                            </lit-table-column>
                            <lit-table-column class="sys-detail-column" width="300px" title="" data-index="ts" key="ts"  align="flex-start" order>
                            </lit-table-column>
                        </lit-table>
                    </div>
                    <lit-slicer-track ></lit-slicer-track>
                    <lit-table id="tb-system-details-data" no-head hideDownload style="height: auto;border-left: 1px solid var(--dark-border1,#e2e2e2)">
                        <lit-table-column class="sys-detail-column" width="100px" title="" data-index="key" key="key"  align="flex-start" >
                        </lit-table-column>
                        <lit-table-column class="sys-detail-column" width="1fr" title="" data-index="value" key="value"  align="flex-start">
                        </lit-table-column>
                    </lit-table>
                </lit-slicer>
            </div>
            <lit-progress-bar class="progress sys-detail-progress"></lit-progress-bar>
        </div>
        `;
  }
}
