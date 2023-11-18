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
import { resizeObserver } from '../SheetUtils.js';
import { queryGpuDataByTs } from '../../../../database/SqlLite.js';
import { VmTrackerChart } from '../../../chart/SpVmTrackerChart.js';
import { log } from '../../../../../log/Log.js';
import { SpSystemTrace } from '../../../SpSystemTrace.js';
import { Utils } from '../../base/Utils.js';
interface GpuTreeItem {
  name: string;
  id: number;
  size: number;
  sizeStr: string;
  children?: GpuTreeItem[] | undefined;
}
@element('tabpane-gpu-click-select')
export class TabPaneGpuClickSelect extends BaseElement {
  private gpuTbl: LitTable | null | undefined;
  private gpuSource: Array<GpuTreeItem> = [];
  gpuClickData(gpu: { type: string; startTs: number }) {
    let label = this.gpuTbl!.shadowRoot!.querySelector('.thead')?.firstChild?.firstChild?.firstChild;
    if (label) {
      (label as HTMLLabelElement).innerHTML = gpu.type === 'total' ? 'Module / Category' : 'Window / Module / Category';
    }
    //@ts-ignore
    this.gpuTbl?.shadowRoot?.querySelector('.table')?.style?.height = this.parentElement!.clientHeight - 45 + 'px';
    this.gpuTbl!.loading = true;
    let window = gpu.type === 'total' ? 0 : VmTrackerChart.gpuWindow;
    let module = gpu.type === 'total' ? VmTrackerChart.gpuTotalModule : VmTrackerChart.gpuWindowModule;
    queryGpuDataByTs(gpu.startTs, window || 0, module).then((result) => {
      this.gpuTbl!.loading = false;
      if (result != null && result.length > 0) {
        log('queryGpuDataByTs result size : ' + result.length);
        let items = this.createTreeData(result);
        this.gpuSource = (gpu.type === 'total' ? items[0].children : items) || [];
        this.gpuTbl!.recycleDataSource = this.gpuSource;
      } else {
        this.gpuSource = [];
        this.gpuTbl!.recycleDataSource = [];
      }
    });
  }
  protected createTreeData(result: any): Array<any> {
    let gpuDataObj = result.reduce(
      (
        group: any,
        item: { categoryId: number; size: number; windowNameId: number; moduleId: number; windowId: any }
      ) => {
        let categoryItem: GpuTreeItem = {
          name: SpSystemTrace.DATA_DICT.get(item.categoryId) || 'null',
          id: item.categoryId,
          size: item.size,
          sizeStr: Utils.getBinaryByteWithUnit(item.size),
        };
        if (group[`${item.windowNameId}(${item.windowId})`]) {
          let windowGroup = group[`${item.windowNameId}(${item.windowId})`] as GpuTreeItem;
          windowGroup.size += item.size;
          windowGroup.sizeStr = Utils.getBinaryByteWithUnit(windowGroup.size);
          let moduleGroup = windowGroup.children!.find((it) => it.id === item.moduleId);
          if (moduleGroup) {
            moduleGroup.size += item.size;
            moduleGroup.sizeStr = Utils.getBinaryByteWithUnit(moduleGroup.size);
            moduleGroup.children?.push(categoryItem);
          } else {
            windowGroup.children?.push({
              name: SpSystemTrace.DATA_DICT.get(item.moduleId) || 'null',
              id: item.moduleId,
              size: item.size,
              sizeStr: Utils.getBinaryByteWithUnit(item.size),
              children: [categoryItem],
            });
          }
        } else {
          group[`${item.windowNameId}(${item.windowId})`] = {
            name: SpSystemTrace.DATA_DICT.get(item.windowNameId) + `(${item.windowId})`,
            id: item.windowNameId,
            size: item.size,
            sizeStr: Utils.getBinaryByteWithUnit(item.size),
            children: [
              {
                name: SpSystemTrace.DATA_DICT.get(item.moduleId),
                id: item.moduleId,
                size: item.size,
                sizeStr: Utils.getBinaryByteWithUnit(item.size),
                children: [categoryItem],
              },
            ],
          };
        }
        return group;
      },
      {}
    );
    let items = Object.values(gpuDataObj) as GpuTreeItem[];
    return items;
  }
  initElements(): void {
    this.gpuTbl = this.shadowRoot?.querySelector<LitTable>('#tb-gpu');
    this.gpuTbl!.addEventListener('column-click', (evt: any) => {
      this.sortByColumn(evt.detail);
    });
  }
  connectedCallback(): void {
    super.connectedCallback();
    this.parentElement!.style.overflow = 'hidden';
    resizeObserver(this.parentElement!, this.gpuTbl!, 18);
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
        <lit-table id="tb-gpu" style="height: auto" tree>
                <lit-table-column width="50%" title="Window / Module / Category" data-index="name" key="name" align="flex-start">
                </lit-table-column>
                <lit-table-column width="1fr" title="Size" data-index="sizeStr" key="sizeStr"  align="flex-start" order >
                </lit-table-column>
        </lit-table>
        `;
  }
  sortByColumn(gpuDetail: any): void {
    let compare = (gpuA: GpuTreeItem, gpuB: GpuTreeItem): number => {
      if (gpuDetail.sort === 0) {
        return gpuA.size - gpuB.size;
      } else if (gpuDetail.sort === 1) {
        return gpuA.size - gpuB.size;
      } else {
        return gpuB.size - gpuA.size;
      }
    };
    let deepCompare = (arr: GpuTreeItem[]) => {
      arr.forEach((it) => {
        if (it.children) {
          deepCompare(it.children);
        }
      });
      arr.sort(compare);
    };
    deepCompare(this.gpuSource);
    this.gpuTbl!.recycleDataSource = this.gpuSource;
  }
}
