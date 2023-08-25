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

import {
  BaseStruct,
  drawFlagLine,
  drawLines,
  drawLoading,
  drawSelection,
  PerfRender,
  RequestMessage,
} from './ProcedureWorkerCommon.js';
import { TraceRow } from '../../component/trace/base/TraceRow.js';

export class FileSystemRender extends PerfRender {
  renderMainThread(
    req: {
      context: CanvasRenderingContext2D;
      useCache: boolean;
      type: string;
      chartColor: string;
    },
    fileSystemRow: TraceRow<FileSysChartStruct>
  ): void {
    let list = fileSystemRow.dataList;
    let filter = fileSystemRow.dataListCache;
    let groupBy10MS = (TraceRow.range?.scale || 50) > 40_000_000;
    let isDiskIO: boolean = req.type.includes('disk-io');
    if (list && fileSystemRow.dataList2.length === 0) {
      fileSystemRow.dataList2 = isDiskIO ?
        FileSysChartStruct.groupBy10MSWithMaxLatency(list) : FileSysChartStruct.groupBy10MSWithCount(list);
    }
    fileSysChart(
      list,
      fileSystemRow.dataList2,
      req.type,
      filter,
      TraceRow.range?.startNS ?? 0,
      TraceRow.range?.endNS ?? 0,
      TraceRow.range?.totalNS ?? 0,
      fileSystemRow.frame,
      groupBy10MS,
      isDiskIO,
      req.useCache || (TraceRow.range?.refresh ?? false)
    );
    req.context.beginPath();
    let find = false;
    let hoverRect: FileSysChartStruct | undefined = undefined;
    for (let re of filter) {
      if (fileSystemRow.isHover && re.frame && fileSystemRow.hoverX >= re.frame.x &&
        fileSystemRow.hoverX <= re.frame.x + re.frame.width
      ) {
        if (hoverRect == undefined || re.size! > hoverRect.size!) {
          hoverRect = re;
          find = true;
        }
      }
      if (re.frame && re.frame!.x > fileSystemRow.hoverX + 3) {
        break;
      }
    }
    if (hoverRect) {
      FileSysChartStruct.hoverFileSysStruct = hoverRect;
    }
    for (let re of filter) {
      FileSysChartStruct.draw(req.context, re, req.chartColor);
    }
    if (!find && fileSystemRow.isHover) {
      FileSysChartStruct.hoverFileSysStruct = undefined;
    }
    req.context.closePath();
  }

  render(fileSysRequest: RequestMessage, list: Array<any>, filter: Array<any>, dataList2: Array<any>): void {
    let groupBy10MS = fileSysRequest.scale > 20_000_000;
    let isDiskIO: boolean = fileSysRequest.type!.includes('disk-io');
    if (isDiskIO) {
      groupBy10MS = true;
    }
    if (fileSysRequest.lazyRefresh || !fileSysRequest.useCache) {
      let use = false;
      if (fileSysRequest.lazyRefresh) {
        use = fileSysRequest.useCache || !fileSysRequest.range.refresh;
      }
      fileSysChart(
        list,
        dataList2,
        fileSysRequest.type!,
        filter,
        fileSysRequest.startNS,
        fileSysRequest.endNS,
        fileSysRequest.totalNS,
        fileSysRequest.frame,
        groupBy10MS,
        isDiskIO,
        use
      );
    }
    let hoverStruct: FileSysChartStruct | undefined;
    if (fileSysRequest.canvas) {
      fileSysRequest.context.clearRect(0, 0, fileSysRequest.frame.width, fileSysRequest.frame.height);
      let arr = filter;
      if (arr.length > 0 && !fileSysRequest.range.refresh && !fileSysRequest.useCache && fileSysRequest.lazyRefresh) {
        drawLoading(
          fileSysRequest.context,
          fileSysRequest.startNS,
          fileSysRequest.endNS,
          fileSysRequest.totalNS,
          fileSysRequest.frame,
          arr[0].startNS,
          arr[arr.length - 1].startNS + arr[arr.length - 1].dur
        );
      }
      drawLines(fileSysRequest.context, fileSysRequest.xs, fileSysRequest.frame.height, fileSysRequest.lineColor);
      fileSysRequest.context.stroke();
      fileSysRequest.context.beginPath();
      if (fileSysRequest.isHover) {
        let offset = groupBy10MS ? 0 : 3;
        for (let re of filter) {
          if (
            re.frame &&
            fileSysRequest.hoverX >= re.frame.x - offset &&
            fileSysRequest.hoverX <= re.frame.x + re.frame.width + offset
          ) {
            hoverStruct = re;
            break;
          }
        }
      }
      for (let re of filter) {
        FileSysChartStruct.draw(fileSysRequest.context, re, fileSysRequest.chartColor);
      }
      drawSelection(fileSysRequest.context, fileSysRequest.params);
      fileSysRequest.context.closePath();
      drawFlagLine(
        fileSysRequest.context,
        fileSysRequest.flagMoveInfo,
        fileSysRequest.flagSelectedInfo,
        fileSysRequest.startNS,
        fileSysRequest.endNS,
        fileSysRequest.totalNS,
        fileSysRequest.frame,
        fileSysRequest.slicesTime
      );
    }
    let msg = {
      id: fileSysRequest.id,
      type: fileSysRequest.type,
      results: fileSysRequest.canvas ? undefined : filter,
      hover: hoverStruct,
    };
    self.postMessage(msg);
  }
}

export function fileSysChart(
  arr: Array<any>,
  arr2: Array<any>,
  type: string,
  fileSysFilters: Array<any>,
  startNS: number,
  endNS: number,
  totalNS: number,
  frame: any,
  groupBy10MS: boolean,
  isDiskIO: boolean,
  use: boolean
) {
  if (use && fileSysFilters.length > 0) {
    //&& !groupBy10MS
    let pns = (endNS - startNS) / frame.width;
    let y = frame.y;
    for (let i = 0; i < fileSysFilters.length; i++) {
      let fileSysData = fileSysFilters[i];
      if ((fileSysData.startNS || 0) + (fileSysData.dur || 0) > startNS && (fileSysData.startNS || 0) < endNS) {
        if (!fileSysData.frame) {
          fileSysData.frame = {};
          fileSysData.frame.y = y;
        }
        fileSysData.frame.height = fileSysData.height;
        FileSysChartStruct.setFrame(fileSysData, pns, startNS, endNS, frame);
      } else {
        fileSysData.frame = null;
      }
    }
    return;
  }
  fileSysFilters.length = 0;
  if (arr) {
    let list: Array<any> = [];
    let pns = (endNS - startNS) / frame.width;
    let y = frame.y;
    if (groupBy10MS) {
      list = arr2.filter((it) => (it.startNS || 0) + (it.dur || 0) > startNS && (it.startNS || 0) < endNS);
      let groups = list
        .map((it) => {
          if (!it.frame) {
            it.frame = {};
            it.frame.y = y;
          }
          it.frame.height = it.height;
          FileSysChartStruct.setFrame(it, pns, startNS, endNS, frame);
          return it;
        })
        .reduce((pre, current, index, arr) => {
          (pre[`${current.frame.x}`] = pre[`${current.frame.x}`] || []).push(current);
          return pre;
        }, {});
      Reflect.ownKeys(groups).map((kv) => {
        fileSysFilters.push(groups[kv][0]);
      });
    } else {
      let filter = arr.filter((it) => (it.startNS || 0) + (it.dur || 0) > startNS && (it.startNS || 0) < endNS);
      list = isDiskIO
        ? FileSysChartStruct.computeHeightNoGroupLatency(filter, totalNS)
        : FileSysChartStruct.computeHeightNoGroup(filter, totalNS);
      list.map((it) => {
        if (!it.frame) {
          it.frame = {};
          it.frame.y = y;
        }
        it.frame.height = it.height;
        FileSysChartStruct.setFrame(it, pns, startNS, endNS, frame);
        fileSysFilters.push(it);
      });
    }
  }
}

export class FileSysChartStruct extends BaseStruct {
  static hoverFileSysStruct: FileSysChartStruct | undefined;
  startNS: number | undefined;
  endNS: number | undefined;
  dur: number | undefined;
  size: number | undefined;
  height: number | undefined;
  group10Ms: boolean | undefined;

  static draw(ctx: CanvasRenderingContext2D, data: FileSysChartStruct, chartColor: string) {
    if (data.frame) {
      ctx.fillStyle = chartColor;
      ctx.strokeStyle = chartColor;
      ctx.fillRect(data.frame.x, 40 - (data.height || 0), data.frame.width, data.height || 0);
    }
  }

  static setFrame(fileSystemNode: any, pns: number, startNS: number, endNS: number, frame: any) {
    if ((fileSystemNode.startNS || 0) < startNS) {
      fileSystemNode.frame.x = 0;
    } else {
      fileSystemNode.frame.x = Math.floor(((fileSystemNode.startNS || 0) - startNS) / pns);
    }
    if ((fileSystemNode.startNS || 0) + (fileSystemNode.dur || 0) > endNS) {
      fileSystemNode.frame.width = frame.width - fileSystemNode.frame.x;
    } else {
      fileSystemNode.frame.width = Math.ceil(
        ((fileSystemNode.startNS || 0) + (fileSystemNode.dur || 0) - startNS) / pns - fileSystemNode.frame.x
      );
    }
    if (fileSystemNode.frame.width < 1) {
      fileSystemNode.frame.width = 1;
    }
  }

  static computeHeightNoGroup(array: Array<any>, totalNS: number): Array<any> {
    if (array.length > 0) {
      let time: Array<{ time: number; type: number }> = [];
      array.map((item) => {
        time.push({ time: item.startNS, type: 1 });
        time.push({ time: item.endNS || totalNS, type: -1 });
      });
      time = time.sort((a, b) => a.time - b.time);
      let arr: Array<any> = [];
      let first = {
        startNS: time[0].time ?? 0,
        dur: 0,
        size: 1,
        group10Ms: false,
        height: 1,
      };
      arr.push(first);
      let max = 2;
      for (let i = 1, len = time.length; i < len; i++) {
        let heap = {
          startNS: time[i].time,
          dur: 0,
          size: 0,
          group10Ms: false,
          height: 0,
        };
        arr[i - 1].dur = heap.startNS - arr[i - 1].startNS;
        if (i == len - 1) {
          heap.dur = totalNS - heap.startNS;
        }
        heap.size = arr[i - 1].size + time[i].type;
        heap.height = Math.floor((heap.size / 6) * 36);
        max = max > heap.size ? max : heap.size;
        arr.push(heap);
      }
      arr.map((it) => (it.height = Math.floor((it.size / max) * 36)));
      return arr;
    } else {
      return [];
    }
  }

  static groupBy10MSWithCount(array: Array<any>): Array<any> {
    let obj = array
      .map((dataItem) => {
        dataItem.timestamp_group = Math.trunc(dataItem.startNS / 1_000_000_0) * 1_000_000_0;
        return dataItem;
      })
      .reduce((pre, current) => {
        (pre[current['timestamp_group']] = pre[current['timestamp_group']] || []).push(current);
        return pre;
      }, {});
    let arr: any[] = [];
    let max = 1;
    for (let aKey in obj) {
      max = obj[aKey].length > max ? obj[aKey].length : max;
    }
    for (let aKey in obj) {
      let ns = parseInt(aKey);
      let height: number = Math.floor((obj[aKey].length / max) * 36);
      arr.push({
        startNS: ns,
        dur: 1_000_000_0,
        group10Ms: true,
        size: obj[aKey].length,
        height: height < 1 ? 1 : height,
      });
    }
    return arr;
  }

  static computeHeightNoGroupLatency(array: Array<any>, totalNS: number): Array<any> {
    if (array.length > 0) {
      let max = 0;
      let arr: Array<any> = [];
      for (let io of array) {
        let ioItem = {
          startNS: io.startNS,
          dur: io.endNS > totalNS ? totalNS - io.startNS : io.endNS - io.startNS,
          size: io.dur,
          group10Ms: false,
          height: 0,
        };
        max = max > ioItem.size ? max : ioItem.size;
        arr.push(ioItem);
      }
      arr.map((it) => {
        let height = Math.floor((it.size / max) * 36);
        it.height = height < 1 ? 1 : height;
      });
      return arr;
    } else {
      return [];
    }
  }

  static groupBy10MSWithMaxLatency(array: Array<any>): Array<any> {
    let obj = array
      .map((it) => {
        it.timestamp_group = Math.trunc(it.startNS / 1_000_000_0) * 1_000_000_0;
        return it;
      })
      .reduce((pre, current) => {
        if (pre[current['timestamp_group']] == undefined || pre[current['timestamp_group']] == null) {
          pre[current['timestamp_group']] = [];
        }
        if (pre[current['timestamp_group']].length > 0) {
          let p = pre[current['timestamp_group']][0];
          if (p.dur < current.dur) {
            pre[current['timestamp_group']][0] = current;
          }
        } else {
          pre[current['timestamp_group']][0] = current;
        }
        return pre;
      }, {});
    let arr: any[] = [];
    let max = 1;
    for (let aKey in obj) {
      max = obj[aKey][0].dur > max ? obj[aKey][0].dur : max;
    }
    for (let aKey in obj) {
      let ns = parseInt(aKey);
      let height: number = Math.floor((obj[aKey][0].dur / max) * 36);
      arr.push({
        startNS: ns,
        dur: 1_000_000_0,
        group10Ms: true,
        size: obj[aKey][0].dur,
        height: height < 1 ? 1 : height,
      });
    }
    return arr;
  }
}
