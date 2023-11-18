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

import { BaseElement, element } from '../../../../base-ui/BaseElement.js';
import { LitTable } from '../../../../base-ui/table/lit-table.js';
import { MarkStruct } from '../../../bean/MarkStruct.js';
import { SpSystemTrace } from '../../SpSystemTrace.js';
import { ns2s } from '../TimerShaftElement.js';
import { SlicesTime, StType } from '../timer-shaft/SportRuler.js';

@element('tabpane-current')
export class TabPaneCurrent extends BaseElement {
  private slicestimeList: Array<SlicesTime> = [];
  private slicestime: SlicesTime | null = null;
  private systemTrace: SpSystemTrace | undefined | null;
  private tableDataSource: Array<MarkStruct | any> = [];
  private panelTable: LitTable | undefined | null;

  initElements(): void {
    this.systemTrace = document
      .querySelector('body > sp-application')?.
      shadowRoot!.querySelector<SpSystemTrace>('#sp-system-trace');
    this.panelTable = this.shadowRoot!.querySelector<LitTable>('.notes-editor-panel');
    this.panelTable!.addEventListener('row-click', (evt: any) => {
      // 点击表格某一行后，背景变色
      // @ts-ignore
      let data = evt.detail.data;
      this.systemTrace!.slicesList = this.slicestimeList || [];
      //   页面上对应的slice变为实心
      this.slicestimeList.forEach((slicesTime, index) => {
        if (data.startTime === slicesTime.startTime && data.endTime === slicesTime.endTime) {
          slicesTime.selected = true;
          this.setTableSelection(index + 1);
        } else {
          slicesTime.selected = false;
        }
        this.systemTrace?.timerShaftEL!.sportRuler!.draw();
      });
      // 如果点击RemoveAll，隐藏所有卡尺，清空数组，隐藏tab页
      if (data.operate.innerHTML === 'RemoveAll') {
        this.systemTrace!.slicesList = [];
        for (let slice of this.slicestimeList) {
          slice.hidden = true;
          document.dispatchEvent(new CustomEvent('slices-change', { detail: slice }));
        }
        this.slicestimeList = [];
      }
    });
  }

  public setCurrentSlicesTime(slicestime: SlicesTime): void {
    this.slicestime = slicestime;
    // 判断当前传入的卡尺是否已经存在
    let findSlicesTime = this.slicestimeList.find(
      (it) => it.startTime === slicestime.startTime && it.endTime === slicestime.endTime
    );
    // m键生成的临时卡尺只能同时出现最后一个，所以将永久卡尺过滤出来，并加上最后一个临时卡尺
    if (this.slicestime.type === StType.TEMP) {
      this.slicestimeList = this.slicestimeList.filter(
        (item: SlicesTime) =>
          item.type === StType.PERM ||
          (item.type === StType.TEMP &&
            item.startTime === this.slicestime!.startTime &&
            item.endTime === this.slicestime!.endTime)
      );
    }
    // 如果this.slicestimeList为空，或者没有在同一位置绘制过，就将当前的框选的范围画上线
    if (!findSlicesTime || this.slicestimeList.length === 0) {
      this.slicestimeList!.push(this.slicestime);
    }
    this.setTableData();
  }

  /**
   * 根据this.slicestimeList设置表格数据
   */
  private setTableData(): void {
    this.tableDataSource = [];
    // 按照开始时间进行排序，保证泳道图上的卡尺（shift+m键）和表格的顺序一致
    this.slicestimeList.sort(function (a, b) {
      return a.startTime - b.startTime;
    });
    for (let slice of this.slicestimeList) {
      let btn = document.createElement('button');
      btn.className = 'remove';
      let color = document.createElement('input');
      color.type = 'color';
      let sliceData = new MarkStruct(
        btn,
        color,
        ns2s(slice.startTime),
        slice.startTime,
        ns2s(slice.endTime),
        slice.endTime
      );
      color!.value = slice.color;
      slice.selected === true ? (sliceData.isSelected = true) : (sliceData.isSelected = false);
      this.tableDataSource.push(sliceData);
    }
    // 表格第一行只添加一个RemoveAll按钮
    let removeAll = document.createElement('button');
    removeAll.className = 'removeAll';
    removeAll.innerHTML = 'RemoveAll';
    let sliceData = new MarkStruct(removeAll);
    this.tableDataSource.unshift(sliceData);

    // 当前点击了哪个卡尺，就将对应的表格中的那行的背景变色
    this.tableDataSource.forEach((data, index) => {
      if (data.startTime === this.slicestime?.startTime && data.endTime === this.slicestime?.endTime) {
        this.setTableSelection(index);
      }
    });
    this.panelTable!.recycleDataSource = this.tableDataSource;
    this.eventHandler();
    this.systemTrace!.slicesList = this.slicestimeList || [];
  }

  /**
   * 修改卡尺颜色事件和移除卡尺的事件处理
   */
  private eventHandler(): void {
    let tr = this.panelTable!.shadowRoot!.querySelectorAll('.tr') as NodeListOf<HTMLDivElement>;
    //   第一个tr是移除全部，所以跳过，从第二个tr开始，和this.slicestimeList数组的第一个对应……，所以i从1开始，在this.slicestimeList数组中取值时用i-1
    for (let i = 1; i < tr.length; i++) {
      tr[i].querySelector('input')!.value = this.slicestimeList[i - 1].color;
      //  点击色块修改颜色
      tr[i].querySelector('input')?.addEventListener('change', (event: any) => {
        if (
          this.tableDataSource[i].startTime === this.slicestimeList[i - 1].startTime &&
          this.tableDataSource[i].endTime === this.slicestimeList[i - 1].endTime
        ) {
          this.systemTrace!.slicesList = this.slicestimeList || [];
          this.slicestimeList[i - 1].color = event?.target.value;
          document.dispatchEvent(new CustomEvent('slices-change', { detail: this.slicestimeList[i - 1] }));
          //   卡尺颜色改变时，重绘泳道图
          this.systemTrace?.refreshCanvas(true);
        }
        event.stopPropagation();
      });
      // 点击remove按钮移除
      tr[i]!.querySelector('.remove')?.addEventListener('click', (event: any) => {
        if (
          this.tableDataSource[i].startTime === this.slicestimeList[i - 1].startTime &&
          this.tableDataSource[i].endTime === this.slicestimeList[i - 1].endTime
        ) {
          this.slicestimeList[i - 1].hidden = true;
          this.systemTrace!.slicesList = this.slicestimeList || [];
          document.dispatchEvent(new CustomEvent('slices-change', { detail: this.slicestimeList[i - 1] }));
          this.slicestimeList.splice(this.slicestimeList.indexOf(this.slicestimeList[i - 1]), 1);
          //   移除时更新表格内容
          this.setTableData();
        }
        event.stopPropagation();
      });
    }
  }

  /**
   * 修改表格指定行数的背景颜色
   * @param line 要改变的表格行数
   */
  public setTableSelection(line: any): void {
    this.tableDataSource[line].isSelected = true;
    this.panelTable?.clearAllSelection(this.tableDataSource[line]);
    this.panelTable?.setCurrentSelection(this.tableDataSource[line]);
  }

  initHtml(): string {
    return `
        <style>
        :host{
            display: flex;
            flex-direction: column;
            padding: 10px 10px;
        }
        </style>
        <lit-table class="notes-editor-panel" style="height: auto">
            <lit-table-column width="1fr" data-index="startTimeStr" key="startTimeStr" align="flex-start" title="StartTime">
            </lit-table-column>
            <lit-table-column width="1fr" data-index="endTimeStr" key="endTimeStr" align="flex-start" title="EndTime">
            </lit-table-column>
            <lit-table-column width="1fr" data-index="color" key="color" align="flex-start" title="Color">
                <template>
                    <div style='width:50px; height: 21px; position: relative;overflow: hidden;'>
                        <input type="color" id="color-input" style='
                            background: var(--dark-background5,#FFFFFF);
                            padding: 0px;
                            border: none;
                            width: 60px;
                            height: 31px;
                            position: absolute;
                            top: -5px;
                            left: -5px;'/>
                    </div>
                </template>
            </lit-table-column>
            <lit-table-column width="1fr" data-index="operate" key="operate" align="flex-start" title="Operate">
                <template>
                    <button class="remove" style='
                        background: var(--dark-border1,#262f3c);
                        color: white;
                        border-radius: 10px;
                        font-size: 10px;
                        height: 21px;
                        line-height: 21px;
                        min-width: 7em;
                        border: none;
                        cursor: pointer;
                        outline: inherit;
                    '>Remove</button>
                </template>
            </lit-table-column>
        </lit-table>
        `;
  }
}
