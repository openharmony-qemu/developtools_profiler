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
import { HiSysEventStruct } from '../../../../database/ui-worker/ProcedureWorkerHiSysEvent.js';
import { ns2x, Rect } from '../../../../database/ui-worker/ProcedureWorkerCommon.js';
import { LitPageTable } from '../../../../../base-ui/table/LitPageTable.js';
import { LitTable } from '../../../../../base-ui/table/lit-table.js';
import { LitSlicerTrack } from '../../../../../base-ui/slicer/lit-slicer.js';
import { TraceRow } from '../../base/TraceRow.js';
import { Flag } from '../../timer-shaft/Flag.js';
import { TraceSheet } from '../../base/TraceSheet.js';
import { SpSystemTrace } from '../../../SpSystemTrace.js';
import { ColorUtils } from '../../base/ColorUtils.js';
import { queryRealTime } from '../../../../database/SqlLite.js';

@element('tab-hisysevents')
export class TabPaneHisysEvents extends BaseElement {
  private hisysEventSource: Array<HiSysEventStruct> = [];
  private filterDataList: HiSysEventStruct[] = [];
  private hiSysEventTable: LitPageTable | undefined | null;
  private currentSelection: SelectionParam | undefined;
  private domainFilterInput: HTMLInputElement | undefined | null;
  private eventNameFilterInput: HTMLInputElement | undefined | null;
  private levelFilter: HTMLSelectElement | undefined | null;
  private contentFilterInput: HTMLInputElement | undefined | null;
  private domainTag: Set<string> = new Set();
  private domainTagDiv: HTMLDivElement | undefined | null;
  private eventNameTag: Set<string> = new Set();
  private eventNameTagDiv: HTMLDivElement | undefined | null;
  private traceSheetEl: TraceSheet | undefined | null;
  private spSystemTrace: SpSystemTrace | undefined | null;
  private detailsTbl: LitTable | null | undefined;
  private boxDetails: HTMLDivElement | null | undefined;
  private slicerTrack: LitSlicerTrack | null | undefined;
  private tableElement: HTMLDivElement | undefined | null;
  private detailbox: HTMLDivElement | null | undefined;
  private changeInput: HTMLInputElement | null | undefined;
  private currentDetailList: Array<{ key: string, value: string }> = [];
  private realTime: number = 0;
  private baseTime: string = '';

  set data(systemEventParam: SelectionParam) {
    if (systemEventParam === this.currentSelection) {
      return;
    }
    if (this.hiSysEventTable) {
      this.hiSysEventTable.recycleDataSource = [];
    }
    this.initTabSheetEl();
    queryRealTime().then((result) => {
      if (result && result.length > 0) {
        this.realTime = Math.floor(result[0].ts / millisecond);
      }
      this.currentSelection = systemEventParam;
      this.hiSysEventTable!.recycleDataSource = systemEventParam.hiSysEvents;
      this.hisysEventSource = systemEventParam.hiSysEvents;
      this.detailsTbl!.recycleDataSource = [];
    });
  }

  initElements(): void {
    this.boxDetails = this.shadowRoot?.querySelector<HTMLDivElement>('.box-details');
    this.hiSysEventTable = this.shadowRoot?.querySelector<LitPageTable>('#tb-hisysevent');
    this.hiSysEventTable!.getItemTextColor = (data) => {
      return ColorUtils.getHisysEventColor(data.level);
    };
    this.domainTagDiv = this.shadowRoot?.querySelector<HTMLDivElement>('#domainTagFilter');
    this.eventNameTagDiv = this.shadowRoot?.querySelector<HTMLDivElement>('#eventNameTagFilter');
    this.domainFilterInput = this.shadowRoot?.querySelector<HTMLInputElement>('#domain-filter');
    this.eventNameFilterInput = this.shadowRoot?.querySelector<HTMLInputElement>('#event-name-filter');
    this.levelFilter = this.shadowRoot?.querySelector<HTMLSelectElement>('#level-filter');
    this.spSystemTrace = document.querySelector('body > sp-application')
      ?.shadowRoot?.querySelector<SpSystemTrace>('#sp-system-trace');
    this.traceSheetEl = this.spSystemTrace!.shadowRoot?.querySelector('.trace-sheet');
    this.contentFilterInput = this.shadowRoot?.querySelector<HTMLInputElement>('#contents-filter');
    this.changeInput = this.shadowRoot?.querySelector<HTMLInputElement>('#contents-change');
    this.detailsTbl = this.shadowRoot?.querySelector<LitTable>('#tb-hisysevent-data');
    this.slicerTrack = this.shadowRoot?.querySelector<LitSlicerTrack>('lit-slicer-track');
    this.detailbox = this.shadowRoot?.querySelector<HTMLDivElement>('.detail-content');
    this.tableElement = this.hiSysEventTable?.shadowRoot?.querySelector('.table') as HTMLDivElement;
    this.hiSysEventTable!.addEventListener('row-click', (e) => {
      this.changeInput!.value = '';
      // @ts-ignore
      this.convertData(e.detail.data);
      this.updateDetail(this.baseTime);
    });
    this.hiSysEventTable!.addEventListener('row-hover', (e) => {
      // @ts-ignore
      let data = e.detail.data;
      if (data) {
        this.drawFlag(data.ts, '#999999');
      }
    });
    this.hiSysEventTable!.addEventListener('column-click', (evt) => {
      // @ts-ignore
      this.sortByColumn(evt.detail);
    });
    this.tableElement.addEventListener('mouseout', () => {
      this.traceSheetEl!.systemLogFlag = undefined;
      this.spSystemTrace?.refreshCanvas(false);
    });
    this.detailsTbl!.addEventListener('row-hover', (e) => {
      // @ts-ignore
      let data = e.detail.data;
      if (data && data.key) {
        if (data.key.endsWith('_TIME') || data.key.endsWith('_LATENCY')) {
          this.drawFlag(data.value, '#999999');
          return;
        }
      }
      this.traceSheetEl!.systemLogFlag = undefined;
      this.spSystemTrace?.refreshCanvas(false);
    });
  }

  initTabSheetEl(): void {
    this.levelFilter!.selectedIndex = 0;
    this.domainFilterInput!.value = '';
    this.domainTagDiv!.innerHTML = '';
    this.domainTag.clear();
    this.eventNameFilterInput!.value = '';
    this.eventNameTagDiv!.innerHTML = '';
    this.eventNameTag.clear();
    this.contentFilterInput!.value = '';
    this.boxDetails!.style.width = '100%';
    this.detailbox!.style.display = 'none';
    this.slicerTrack!.style.visibility = 'hidden';
    this.detailsTbl!.style.paddingLeft = '0px';
  }

  initHtml(): string {
    return `
        <style>
        :host{
            padding: 10px 10px;
            display: flex;
            flex-direction: column;
        }
        .sys-detail-progress{
            bottom: 33px;
            position: absolute;
            height: 1px;
            left: 0;
            right: 0;
        }
        .title-content {
          display: flex;
          flex-wrap: wrap;
          width: 100%;
          align-items: center;
          justify-content: right;
          border-bottom: 1px solid #D5D5D5;
          padding-bottom: 10px;
        }
        .detail-content {
          display: flex;
          flex-wrap: wrap;
          width: 100%;
          padding-left: 20px;
          align-items: center;
          justify-content: right;
          border-bottom: 1px solid #D5D5D5;
          padding-bottom: 10px;
        }
        .level-content {
          margin-right: 20px;
        }
        #level-filter {
          padding: 1px 12px;
          opacity: 0.6;
          font-size: 14px;
          line-height: 20px;
          font-weight: 400;
        }
        select {
          border: 1px solid rgba(0,0,0,0.60);
          border-radius: 10px;
          margin-bottom: 4px;
        }
        option {
          font-weight: 400;
          font-size: 14px;
        }
        .change-input {
          width: 40%;
          line-height: 16px;
          margin-right: 20px;
          padding: 3px 12px;
          height: 16px;
        }
        .filter-input {
          line-height: 16px;
          margin-right: 20px;
          padding: 3px 12px;
          height: 16px;
          margin-bottom: 4px;
        }
        input {
          background: #FFFFFF;
          font-size: 14px;
          color: #212121;
          text-align: left;
          line-height: 16px;
          font-weight: 400;
          text-indent: 2%;
          border: 1px solid #979797;
          border-radius: 10px;
        }
        .tagElement {
          display: flex;
          background-color: #0A59F7;
          align-items: center;
          margin-right: 5px;
          border-radius: 10px;
          font-size: 14px;
          height: 22px;
          margin-bottom: 4px;
        }
        .tag {
          line-height: 14px;
          padding: 4px 8px;
          color: #FFFFFF;
        }
        #tb-hisysevent-data {
          height: auto;
          border-left: 1px solid var(--dark-border1,#e2e2e2);
          display:flex;
        }
        </style>
        <div style="display: flex;flex-direction: row">
          <div class="box-details" style="width: auto">
            <div class="title-content">
                <div style="display: flex;flex-wrap: wrap;">
                   <div style="display: flex;">
                     <div id="domainTagFilter" style='display: flex;width: auto; height: 100%;flex-wrap: wrap;'>
                     </div>
                     <input type="text" id="domain-filter" class="filter-input" placeholder="Filter by Domain…">
                  </div>
                   <div style="display: flex;">
                    <div id="eventNameTagFilter" style='display: flex;width: auto; height: 100%;flex-wrap: wrap;'>
                    </div>
                    <input type="text" id="event-name-filter" class="filter-input" placeholder="Filter by eventName…">
                  </div>
                   <div class="level-content">
                       <select id="level-filter">
                        <option>ALL</option>
                        <option>MINOR</option>
                        <option>CRITICAL</option>
                       </select>
                    </div>
                   <input type="text" id="contents-filter" class="filter-input" placeholder="Filter by contents…">
                </div>
            </div>
        <lit-page-table id="tb-hisysevent" style="height: auto">
           <lit-table-column title="id" width="0.5fr" data-index="id" key="id"  align="flex-start" order>
           </lit-table-column>
           <lit-table-column title="domain" width="1.5fr" data-index="domain" key="domain"  align="flex-start" >
           </lit-table-column>
           <lit-table-column title="eventName" width="3fr" data-index="eventName" key="eventName" align="flex-start" >
           </lit-table-column>
           <lit-table-column title="type" width="0.5fr" data-index="eventType" key="eventType"  align="flex-start" >
           </lit-table-column>
           <lit-table-column title="time" width="1.5fr" data-index="ts" key="ts"  align="flex-start" order>
           </lit-table-column>
           <lit-table-column title="pid" width="1fr" data-index="pid" key="pid"  align="flex-start" order >
           </lit-table-column>
           <lit-table-column title="tid" width="1fr" data-index="tid" key="tid"  align="flex-start" order >
           </lit-table-column>
           <lit-table-column title="uid" width="1fr" data-index="uid" key="uid"  align="flex-start" order >
           </lit-table-column>
            <lit-table-column title="info" width="1fr" data-index="info" key="info"  align="flex-start" >
           </lit-table-column>
           <lit-table-column title="level" width="1fr" data-index="level" key="level"  align="flex-start" >
           </lit-table-column>
           <lit-table-column title="seq" width="1fr" data-index="seq" key="seq"  align="flex-start" >
           </lit-table-column>
           <lit-table-column title="contents" width="5fr" data-index="contents" key="contents"  align="flex-start">
           </lit-table-column>
        </lit-page-table>
     </div>
     <lit-slicer-track></lit-slicer-track>
     <div class="detail-box" style="flex-grow: 1;" > 
        <div class="detail-content">
          <input type="text" id="contents-change" class="change-input" placeholder=" ">
        </div>
        <lit-table id="tb-hisysevent-data" no-head hideDownload>
          <lit-table-column class="sys-detail-column" width="50%" title="key" 
          data-index="key" key="key" align="flex-start" style="flex: 1">
          </lit-table-column>
          <lit-table-column class="sys-detail-column" width="50%" 
          title="value" data-index="value" key="value"  align="flex-start" style="flex: 1">
          </lit-table-column> 
        </lit-table> 
     </div>
    </div>`;
  }

  connectedCallback(): void {
    this.domainFilterInput?.addEventListener('keyup', this.domainKeyEvent);
    this.eventNameFilterInput?.addEventListener('keyup', this.eventNameKeyEvent);
    this.contentFilterInput?.addEventListener('input', this.filterInputEvent);
    this.levelFilter?.addEventListener('change', this.filterInputEvent);
    this.domainTagDiv?.addEventListener('click', this.domainDivClickEvent);
    this.eventNameTagDiv?.addEventListener('click', this.eventNameDivClickEvent);
    this.changeInput?.addEventListener('input', this.changeInputEvent);
    new ResizeObserver(() => {
      this.tableElement!.style.height = `${this.parentElement!.clientHeight - 20 - 35}px`;
      this.hiSysEventTable?.reMeauseHeight();
      // @ts-ignore
      this.detailsTbl?.shadowRoot?.querySelector('.table').style.height =
        `${this.parentElement!.clientHeight - 20 - 35}px`;
      // @ts-ignore
      this.detailsTbl?.shadowRoot?.querySelector('.table').style = 'hidden';
      this.parentElement!.style.overflow = 'hidden';
      this.detailsTbl?.reMeauseHeight();
      this.updateData();
    }).observe(this.parentElement!);
  }

  disconnectedCallback(): void {
    super.disconnectedCallback();
    this.domainFilterInput?.removeEventListener('keyup', this.domainKeyEvent);
    this.eventNameFilterInput?.removeEventListener('keyup', this.eventNameKeyEvent);
    this.contentFilterInput?.removeEventListener('input', this.filterInputEvent);
    this.levelFilter?.addEventListener('change', this.filterInputEvent);
    this.changeInput?.addEventListener('input', this.changeInputEvent);
    this.domainTagDiv?.removeEventListener('click', this.domainDivClickEvent);
    this.eventNameTagDiv?.removeEventListener('click', this.eventNameDivClickEvent);
  }

  filterInputEvent = (): void => {
    this.updateData();
  };

  domainDivClickEvent = (ev: Event): void => {
    // @ts-ignore
    let parentNode = ev.target.parentNode;
    if (parentNode && this.domainTagDiv!.contains(parentNode)) {
      this.domainTagDiv!.removeChild(parentNode);
      this.domainTag['delete'](parentNode.textContent.trim().toLowerCase());
    }
    this.updateData();
  };

  eventNameDivClickEvent = (ev: Event): void => {
    // @ts-ignore
    let parentNode = ev.target.parentNode;
    if (parentNode && this.eventNameTagDiv!.contains(parentNode)) {
      this.eventNameTagDiv!.removeChild(parentNode);
      this.eventNameTag['delete'](parentNode.textContent.trim().toLowerCase());
    }
    this.updateData();
  };

  domainKeyEvent = (e: KeyboardEvent): void => {
    let domainValue = this.domainFilterInput!.value.trim();
    if (e.code === 'Enter') {
      if (domainValue !== '' && !this.domainTag.has(domainValue.toLowerCase())) {
        let tagElement = this.buildTag(domainValue);
        this.domainTag.add(domainValue.toLowerCase());
        this.domainTagDiv!.append(tagElement);
        this.domainFilterInput!.value = '';
      }
    } else if (e.code === 'Backspace') {
      let index = this.domainTagDiv!.childNodes.length - 1;
      if (index >= 0 && domainValue === '') {
        let childNode = this.domainTagDiv!.childNodes[index];
        this.domainTagDiv!.removeChild(childNode);
        this.domainTag['delete'](childNode.textContent!.trim().toLowerCase());
      }
    }
    this.updateData();
  };

  private buildTag(domainValue: string): HTMLDivElement {
    let tagElement = document.createElement('div');
    tagElement.className = 'tagElement';
    tagElement.id = domainValue;
    let tag = document.createElement('div');
    tag.className = 'tag';
    tag.innerHTML = domainValue;
    let closeButton = document.createElement('lit-icon');
    closeButton.setAttribute('name', 'close-light');
    closeButton.style.color = '#FFFFFF';
    tagElement.append(tag);
    tagElement.append(closeButton);
    return tagElement;
  }

  eventNameKeyEvent = (e: KeyboardEvent): void => {
    let eventNameValue = this.eventNameFilterInput!.value.trim();
    if (e.code === 'Enter') {
      if (eventNameValue !== '' && !this.eventNameTag.has(eventNameValue.toLowerCase())) {
        let tagElement = this.buildTag(eventNameValue);
        this.eventNameTag!.add(eventNameValue.toLowerCase());
        this.eventNameTagDiv!.append(tagElement);
        this.eventNameFilterInput!.value = '';
      }
    } else if (e.code === 'Backspace') {
      let index = this.eventNameTagDiv!.childNodes.length - 1;
      if (index >= 0 && eventNameValue === '') {
        let childNode = this.eventNameTagDiv!.childNodes[index];
        this.eventNameTagDiv!.removeChild(childNode);
        this.eventNameTag['delete'](childNode.textContent!.trim().toLowerCase());
      }
    }
    this.updateData();
  };

  updateData(): void {
    if (this.hisysEventSource.length > 0) {
      this.filterDataList = this.hisysEventSource.filter((data) => this.filterData(data));
    }
    if (this.filterDataList.length > 0) {
      this.hiSysEventTable!.recycleDataSource = this.filterDataList;
    } else {
      this.hiSysEventTable!.recycleDataSource = [];
    }
  }

  filterData(data: HiSysEventStruct): boolean {
    let level = this.levelFilter?.value;
    let contentsValue = this.contentFilterInput?.value.toLowerCase() || '';
    contentsValue = contentsValue.replace(/\s/g, '');
    return (
      (level === 'ALL' || data.level! === level) &&
      (this.domainTag.size === 0 || this.domainTag.has(data.domain!.toLowerCase())) &&
      (this.eventNameTag.size === 0 || this.eventNameTag.has(data.eventName!.toLowerCase())) &&
      (contentsValue === '' || data.contents!.toLowerCase().replace(/\s/g, '').indexOf(contentsValue) >= 0)
    );
  }

  changeInputEvent = (): void => {
    const changeValue = this.changeInput!.value;
    const currentValue = changeValue;
    if (!/^[0-9]*$/.test(changeValue) || isNaN(Number(currentValue))) {
      this.changeInput!.value = '';
      this.updateDetail(this.baseTime);
    } else {
      this.updateDetail(this.changeInput!.value);
    }
  }

  updateDetail(baseTime: string): void {
    const latencySuffix = '_LATENCY';
    let detailList: Array<{ key: string, value: string }> = [];
    this.currentDetailList.forEach(item => {
      const latencyValue = item.key && item.key.endsWith(latencySuffix) ? item.value + Number(baseTime) : item.value;
      detailList.push({
        key: item.key,
        value: latencyValue
      });
    });
    this.detailsTbl!.recycleDataSource = detailList;
    console.log(this.detailsTbl!.recycleDataSource)
    const tr = this.detailsTbl!.shadowRoot?.querySelector<HTMLDivElement>('.tr:nth-of-type(1)');
    if (tr) {
      tr.style.fontWeight = 'bolder';
    }
  }

  convertData = (data: HiSysEventStruct): void => {
    this.baseTime = '';
    this.currentDetailList = [{
      key: 'key',
      value: 'value'}];
    const content = JSON.parse(data.contents ?? '{}');
    if (content && typeof content === 'object') {
      let isFirstTime = false;
      let inputTimeTs = '';
      let keyList = Object.keys(content);
      keyList.forEach(key => {
        const value: string = content[key];
        let contentValue = value;
        if (key.endsWith('_TIME')) {
          if (this.realTime === 0) {
            contentValue = ((Number(value) - this.realTime) > 0) ? String(Number(value) - this.realTime) : value;
          }
          if (!isNaN(Number(value))) {
            let diffTime = (Number(value) - this.realTime) * millisecond;
            // @ts-ignore
            contentValue = String(diffTime - (window.recordStartNS || 0));
            if (!isFirstTime) {
              this.baseTime = contentValue;
              isFirstTime = true;
            }
          }
          if (key === 'INPUT_TIME') {
            inputTimeTs = contentValue;
          }
        }
        this.currentDetailList.push({
          key: key,
          value: contentValue
        });
      });
      if (keyList.indexOf('INPUT_TIME') >= 0) {
        this.baseTime = inputTimeTs;
      }
    }
    this.changeInput!.value = `${this.baseTime}`;
    this.detailsTbl!.recycleDataSource = this.currentDetailList;
    this.slicerTrack!.style.visibility = 'visible';
    this.detailsTbl!.style.paddingLeft = '20px';
    this.boxDetails!.style.width = '65%';
    this.detailbox!.style.display = 'block';
  }

  sortByColumn(framesDetail: { sort: number; key: string }): void {
    let compare = function (property: string, sort: number, type: string) {
      return function (
        eventLeftData: HiSysEventStruct,
        eventRightData: HiSysEventStruct
      ): number {
        let firstSortNumber: number = -1;
        let SecondSortNumber: number = 1;
        let thirdSortNumber: number = 2;
        // @ts-ignore
        let rightEventData = eventRightData[property];
        // @ts-ignore
        let leftEventData = eventLeftData[property];
        if (type === 'number') {
          return sort === thirdSortNumber ?
            parseFloat(rightEventData) - parseFloat(leftEventData) :
            parseFloat(leftEventData) - parseFloat(rightEventData);
        } else {
          if (rightEventData > leftEventData) {
            return sort === thirdSortNumber ? SecondSortNumber : firstSortNumber;
          } else {
            if (rightEventData === leftEventData) {
              return 0;
            } else {
              return sort === thirdSortNumber ? firstSortNumber : SecondSortNumber;
            }
          }
        }
      };
    };
    this.hisysEventSource.sort(compare(framesDetail.key, framesDetail.sort, 'number'));
    this.hiSysEventTable!.recycleDataSource = this.hisysEventSource;
  }

  drawFlag(value: number, color: string): void {
    let pointX: number = ns2x(
      value || 0,
      TraceRow.range!.startNS,
      TraceRow.range!.endNS,
      TraceRow.range!.totalNS,
      new Rect(0, 0, TraceRow.FRAME_WIDTH, 0)
    );
    this.traceSheetEl!.systemLogFlag = new Flag(Math.floor(pointX), 0, 0, 0, value!, color, true, '');
    this.spSystemTrace?.refreshCanvas(false);
  }
}
const  millisecond = 1000_000;