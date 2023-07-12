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
  dataFilterHandler, drawString,
  ns2x,
} from './ProcedureWorkerCommon.js';
import { TraceRow } from '../../component/trace/base/TraceRow.js';
import { ColorUtils } from '../../component/trace/base/ColorUtils.js';

export class AppStartupRender {
  renderMainThread(
    req: {
      useCache: boolean;
      context: CanvasRenderingContext2D;
      type: string;
    },
    row: TraceRow<AppStartupStruct>
  ) {
    let list = row.dataList;
    let filter = row.dataListCache;
    dataFilterHandler(list, filter, {
      startKey: 'startTs',
      durKey: 'dur',
      startNS: TraceRow.range?.startNS ?? 0,
      endNS: TraceRow.range?.endNS ?? 0,
      totalNS: TraceRow.range?.totalNS ?? 0,
      frame: row.frame,
      paddingTop: 5,
      useCache: req.useCache || !(TraceRow.range?.refresh ?? false),
    });
    req.context.font = '11px sans-serif';
    req.context.globalAlpha = 0.6;
    let find = false;
    let offset = 3;
    for (let re of filter) {
      AppStartupStruct.draw(req.context, re);
      if (row.isHover) {
        if (re.frame && row.hoverX >= re.frame.x - offset && row.hoverX <= re.frame.x + re.frame.width + offset) {
          AppStartupStruct.hoverStartupStruct = re;
          find = true;
        }
      }
    }
    if (!find && row.isHover) AppStartupStruct.hoverStartupStruct = undefined;
  }

}
const padding = 3;

export class AppStartupStruct extends BaseStruct {
  static hoverStartupStruct: AppStartupStruct | undefined;
  static selectStartupStruct: AppStartupStruct | undefined;
  static StartUpStep: string[] = [
    'Process Creating',
    'Application Launching',
    'UI Ability Launching',
    'UI Ability OnForeground',
    'First Frame - APP Phase',
    'First Frame - Render Phase'
  ]
  dur: number | undefined;
  value: string | undefined;
  startTs: number | undefined;
  pid: number | undefined;
  process: string | undefined;
  itid: number | undefined;
  endItid: number | undefined;
  tid: number | undefined;
  startName: number | undefined;

  static draw(ctx: CanvasRenderingContext2D, data: AppStartupStruct) {
    if (data.frame) {
      ctx.globalAlpha = 1.0;
      ctx.fillStyle = ColorUtils.colorForTid(data.startName!);
      ctx.fillRect(data.frame.x, data.frame.y, data.frame.width, data.frame.height);
      if(data.frame.width > 7) {
        ctx.fillStyle = '#fff';
        ctx.textBaseline = 'middle';
        ctx.font = '8px sans-serif';
        drawString(ctx, AppStartupStruct.getStartupName(data.startName), 2, data.frame, data);
      }
      if (data === AppStartupStruct.selectStartupStruct) {
        ctx.strokeStyle = '#232c5d';
        ctx.lineWidth = 2;
        ctx.strokeRect(
          data.frame.x,
          data.frame.y,
          data.frame.width,
          data.frame.height
        );
      }
    }
  }

  static getStartupName(step: number | undefined) {
    if (step === undefined || step < 0 || step > 5) {
      return 'Unknown Start Step';
    } else {
      return AppStartupStruct.StartUpStep[step];
    }
  }

}
