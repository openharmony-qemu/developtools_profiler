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
} from './ProcedureWorkerCommon.js';
import { TraceRow } from '../../component/trace/base/TraceRow.js';
import { ColorUtils } from '../../component/trace/base/ColorUtils.js';

export class AppStartupRender {
  renderMainThread(
    req: {
      useCache: boolean;
      appStartupContext: CanvasRenderingContext2D;
      type: string;
    },
    appStartUpRow: TraceRow<AppStartupStruct>
  ): void {
    let list = appStartUpRow.dataList;
    let filter = appStartUpRow.dataListCache;
    dataFilterHandler(list, filter, {
      startKey: 'startTs',
      durKey: 'dur',
      startNS: TraceRow.range?.startNS ?? 0,
      endNS: TraceRow.range?.endNS ?? 0,
      totalNS: TraceRow.range?.totalNS ?? 0,
      frame: appStartUpRow.frame,
      paddingTop: 5,
      useCache: req.useCache || !(TraceRow.range?.refresh ?? false),
    });
    req.appStartupContext.globalAlpha = 0.6;
    let find = false;
    let offset = 3;
    for (let re of filter) {
      AppStartupStruct.draw(req.appStartupContext, re);
      if (appStartUpRow.isHover) {
        if (re.frame && appStartUpRow.hoverX >= re.frame.x - offset &&
          appStartUpRow.hoverX <= re.frame.x + re.frame.width + offset
        ) {
          AppStartupStruct.hoverStartupStruct = re;
          find = true;
        }
      }
    }
    if (!find && appStartUpRow.isHover) {
      AppStartupStruct.hoverStartupStruct = undefined;
    }
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
  ];
  dur: number | undefined;
  value: string | undefined;
  startTs: number | undefined;
  pid: number | undefined;
  process: string | undefined;
  itid: number | undefined;
  endItid: number | undefined;
  tid: number | undefined;
  startName: number | undefined;
  stepName: string | undefined;

  static draw(ctx: CanvasRenderingContext2D, data: AppStartupStruct): void {
    if (data.frame) {
      ctx.globalAlpha = 1.0;
      ctx.fillStyle = ColorUtils.colorForTid(data.startName!);
      ctx.fillRect(data.frame.x, data.frame.y, data.frame.width, data.frame.height);
      if (data.frame.width > 7) {
        ctx.textBaseline = 'middle';
        ctx.lineWidth = 1;
        if (data.stepName === undefined) {
          data.stepName = `${ AppStartupStruct.getStartupName(data.startName) } (${ (data.dur! / 1000000).toFixed(2) }ms)`;
        }
        let textColor = ColorUtils.FUNC_COLOR[ColorUtils.hashFunc(data.stepName || '', 0, ColorUtils.FUNC_COLOR.length)];
        ctx.fillStyle = ColorUtils.funcTextColor(textColor);
        drawString(ctx, data.stepName, 2, data.frame, data);
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

  static getStartupName(step: number | undefined): string {
    if (step === undefined || step < 0 || step > 5) {
      return 'Unknown Start Step';
    } else {
      return AppStartupStruct.StartUpStep[step];
    }
  }

}
