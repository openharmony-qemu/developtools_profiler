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
import { SelectionData, SelectionParam } from '../../../../bean/BoxSelection';
import { LitTable } from '../../../../../base-ui/table/lit-table';
import { JankFramesStruct } from '../../../../bean/JankFramesStruct.js';
import { JanksStruct } from '../../../../bean/JanksStruct.js';
import { resizeObserver } from '../SheetUtils.js';

@element('tabpane-frames')
export class TabPaneFrames extends BaseElement {
  private framesTbl: LitTable | null | undefined;
  private range: HTMLLabelElement | null | undefined;
  private framesSource: Array<any> = [];
  set data(framesParam: SelectionParam | any) {
    this.range!.textContent =
      'Selected range: ' + parseFloat(((framesParam.rightNs - framesParam.leftNs) / 1000000.0).toFixed(5)) + '  ms';
    this.queryDataByDB(framesParam);
  }

  queryDataByDB(framesParam: SelectionParam | any): void {
    let tablelist = new Array<JankFramesStruct>();
    let sumRes: JankFramesStruct = new JankFramesStruct();
    let appJank: JankFramesStruct = new JankFramesStruct();
    let rsJank: JankFramesStruct = new JankFramesStruct();
    let noJank: JankFramesStruct = new JankFramesStruct();
    framesParam.jankFramesData.forEach((data: Array<JanksStruct>) => {
      sumRes.occurrences += data.length;
      data.forEach((structValue: JanksStruct) => {
        if (structValue.frame_type === 'app') {
          this.appJankDataHandle(structValue, appJank, noJank);
        } else if (structValue.frame_type === 'renderService') {
          this.rsJankDataHandle(structValue, rsJank, noJank);
        } else {
          // frameTime
          this.frameTimelineJankDataHandle(structValue, appJank, noJank);
        }
      });
    });
    tablelist.push(sumRes);
    if (appJank.occurrences > 0) {
      appJank.maxDurationStr = appJank.maxDuration + '';
      appJank.minDurationStr = appJank.minDuration + '';
      appJank.meanDurationStr = appJank.meanDuration + '';
      tablelist.push(appJank);
    }
    if (rsJank.occurrences > 0) {
      rsJank.maxDurationStr = rsJank.maxDuration + '';
      rsJank.minDurationStr = rsJank.minDuration + '';
      rsJank.meanDurationStr = rsJank.meanDuration + '';
      tablelist.push(rsJank);
    }
    if (noJank.occurrences > 0) {
      noJank.maxDurationStr = noJank.maxDuration + '';
      noJank.minDurationStr = noJank.minDuration + '';
      noJank.meanDurationStr = noJank.meanDuration + '';
      tablelist.push(noJank);
    }
    this.framesSource = tablelist;
    this.framesTbl!.recycleDataSource = tablelist;
  }

  private frameTimelineJankDataHandle(structValue: JanksStruct, appJank: JankFramesStruct, noJank: JankFramesStruct): void {
    if (structValue.dur === null || structValue.dur === undefined) {
      structValue.dur = 0;
    }
    if (structValue && structValue.jank_tag && structValue.jank_tag > 0) {
      appJank.flag = structValue.jank_tag;
      appJank.jankType = 'Deadline Missed';
      appJank.occurrences += 1;
      appJank.maxDuration = Math.max(structValue.dur, appJank.maxDuration);
      appJank.minDuration = Math.min(structValue.dur, appJank.minDuration);
      if (appJank.minDuration === -1) {
        appJank.minDuration = structValue.dur;
      } else {
        appJank.minDuration = Math.min(structValue.dur, appJank.minDuration!);
      }
      if (appJank.meanDuration === -1) {
        appJank.meanDuration = structValue.dur;
      } else {
        appJank.meanDuration = Number(((structValue.dur + appJank.meanDuration) / 2).toFixed(2));
      }
    } else {
      noJank.flag = structValue.jank_tag;
      noJank.jankType = 'None';
      noJank.occurrences += 1;
      noJank.maxDuration = Math.max(structValue.dur, noJank.maxDuration);
      if (noJank.minDuration === -1) {
        noJank.minDuration = structValue.dur;
      } else {
        noJank.minDuration = Math.min(structValue.dur, noJank.minDuration!);
      }
      if (noJank.meanDuration === -1) {
        noJank.meanDuration = structValue.dur;
      } else {
        noJank.meanDuration = Number(((structValue.dur + noJank.meanDuration) / 2).toFixed(2));
      }
    }
  }
  private rsJankDataHandle(structValue: JanksStruct, rsJank: JankFramesStruct, noJank: JankFramesStruct): void {
    if (structValue.dur === null || structValue.dur === undefined) {
      structValue.dur = 0;
    }
    if (structValue.jank_tag && structValue.jank_tag > 0) {
      rsJank.flag = structValue.jank_tag;
      rsJank.jankType = 'RenderService Deadline Missed';
      rsJank.occurrences += 1;
      rsJank.maxDuration = Math.max(structValue.dur, rsJank.maxDuration!);
      if (rsJank.minDuration === -1) {
        rsJank.minDuration = structValue.dur;
      } else {
        rsJank.minDuration = Math.min(structValue.dur, rsJank.minDuration!);
      }
      if (rsJank.meanDuration === -1) {
        rsJank.meanDuration = structValue.dur;
      } else {
        rsJank.meanDuration = Number(((structValue.dur + rsJank.meanDuration!) / 2).toFixed(2));
      }
    } else {
      this.refreshNoJankData(noJank, structValue);
    }
  }
  private appJankDataHandle(structValue: JanksStruct, appJank: JankFramesStruct, noJank: JankFramesStruct): void {
    if (structValue.dur === null || structValue.dur === undefined) {
      structValue.dur = 0;
    }
    if (structValue.jank_tag && structValue.jank_tag > 0) {
      appJank.flag = structValue.jank_tag;
      appJank.jankType = 'APP Deadline Missed';
      appJank.occurrences += 1;
      appJank.maxDuration = Math.max(structValue.dur, appJank.maxDuration!);
      if (appJank.minDuration === -1) {
        appJank.minDuration = structValue.dur;
      } else {
        appJank.minDuration = Math.min(structValue.dur, appJank.minDuration!);
      }
      if (appJank.meanDuration === -1) {
        appJank.meanDuration = structValue.dur;
      } else {
        appJank.meanDuration = Number(((structValue.dur + appJank.meanDuration!) / 2).toFixed(2));
      }
    } else {
      this.refreshNoJankData(noJank, structValue);
    }
  }
  private refreshNoJankData(noJank: JankFramesStruct, structValue: JanksStruct): void {
    noJank.flag = structValue.jank_tag;
    noJank.jankType = 'None';
    noJank.occurrences += 1;
    noJank.maxDuration = Math.max(structValue.dur!, noJank.maxDuration!);
    if (noJank.minDuration === -1) {
      noJank.minDuration = structValue.dur!;
    } else {
      noJank.minDuration = Math.min(structValue.dur!, noJank.minDuration!);
    }
    if (noJank.meanDuration === -1) {
      noJank.meanDuration = structValue.dur!;
    } else {
      noJank.meanDuration = Number(((structValue.dur! + noJank.meanDuration!) / 2).toFixed(2));
    }
  }

  initElements(): void {
    this.framesTbl = this.shadowRoot?.querySelector<LitTable>('#tb-frames');
    this.range = this.shadowRoot?.querySelector('#jank-frames-time-range');
    this.framesTbl!.addEventListener('column-click', (evt) => {
      // @ts-ignore
      this.sortByColumn(evt.detail);
    });
  }

  connectedCallback(): void {
    super.connectedCallback();
    resizeObserver(this.parentElement!, this.framesTbl!);
  }

  initHtml(): string {
    return `
        <style>
        .frames-label{
          height: 20px;
          text-align: end;
        }
        :host{
            padding: 10px 10px;
            display: flex;
            flex-direction: column;
        }
        </style>
        <label id="jank-frames-time-range" class="frames-label" style="width: 100%;font-size: 10pt;margin-bottom: 5px">Selected range:0.0 ms</label>
        <lit-table id="tb-frames" style="height: auto">
            <lit-table-column class="jank-frames-column" title="Jank Type" width="1fr" data-index="jankType" key="jankType"  align="flex-start" order>
            </lit-table-column>
            <lit-table-column class="jank-frames-column" title="Min duration" width="1fr" data-index="minDurationStr" key="minDurationStr"  align="flex-start" order >
            </lit-table-column>
            <lit-table-column class="jank-frames-column" title="Max duration" width="1fr" data-index="maxDurationStr" key="maxDurationStr"  align="flex-start" order >
            </lit-table-column>
            <lit-table-column class="jank-frames-column" title="Mean duration" width="1fr" data-index="meanDurationStr" key="meanDurationStr"  align="flex-start" order >
            </lit-table-column>
            <lit-table-column class="jank-frames-column" title="Occurrences" width="1fr" data-index="occurrences" key="occurrences"  align="flex-start" order >
            </lit-table-column>
        </lit-table>
        `;
  }

  sortByColumn(framesDetail: any): void {
    // @ts-ignore
    function compare(property, sort, type) {
      return function (framesLeftData: SelectionData, framesRightData: SelectionData) {
        if (framesLeftData.process === ' ' || framesRightData.process === ' ') {
          return 0;
        }
        if (type === 'number') {
          // @ts-ignore
          return sort === 2 ? parseFloat(framesRightData[property]) - parseFloat(framesLeftData[property]) : parseFloat(framesLeftData[property]) - parseFloat(framesRightData[property]);
        } else {
          // @ts-ignore
          if (framesRightData[property] > framesLeftData[property]) {
            return sort === 2 ? 1 : -1;
          } else {
            // @ts-ignore
            if (framesRightData[property] === framesLeftData[property]) {
              return 0;
            } else {
              return sort === 2 ? -1 : 1;
            }
          }
        }
      };
    }

    if (framesDetail.key === 'jankType') {
      this.framesSource.sort(compare(framesDetail.key, framesDetail.sort, 'string'));
    } else {
      this.framesSource.sort(compare(framesDetail.key, framesDetail.sort, 'number'));
    }
    this.framesTbl!.recycleDataSource = this.framesSource;
  }
}
