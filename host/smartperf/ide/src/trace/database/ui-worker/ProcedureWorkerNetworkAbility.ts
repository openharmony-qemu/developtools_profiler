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

import { ColorUtils } from '../../component/trace/base/ColorUtils.js';
import { dataFilterHandler, isFrameContainPoint, Render, RequestMessage, BaseStruct } from './ProcedureWorkerCommon.js';
import { TraceRow } from '../../component/trace/base/TraceRow.js';
export class NetworkAbilityRender extends Render {
  renderMainThread(
    req: {
      context: CanvasRenderingContext2D;
      useCache: boolean;
      type: string;
      maxNetworkRate: number;
      maxNetworkRateName: string;
    },
    networkAbilityRow: TraceRow<NetworkAbilityMonitorStruct>
  ): void {
    let networkAbilityList = networkAbilityRow.dataList;
    let networkAbilityFilter = networkAbilityRow.dataListCache;
    dataFilterHandler(networkAbilityList, networkAbilityFilter, {
      startKey: 'startNS',
      durKey: 'dur',
      startNS: TraceRow.range?.startNS ?? 0,
      endNS: TraceRow.range?.endNS ?? 0,
      totalNS: TraceRow.range?.totalNS ?? 0,
      frame: networkAbilityRow.frame,
      paddingTop: 5,
      useCache: req.useCache || !(TraceRow.range?.refresh ?? false),
    });
    req.context.beginPath();
    let find = false;
    for (let re of networkAbilityFilter) {
      NetworkAbilityMonitorStruct.draw(req.context, re, req.maxNetworkRate, networkAbilityRow.isHover);
      if (networkAbilityRow.isHover && re.frame &&
        isFrameContainPoint(re.frame, networkAbilityRow.hoverX, networkAbilityRow.hoverY)
      ) {
        NetworkAbilityMonitorStruct.hoverNetworkAbilityStruct = re;
        find = true;
      }
    }
    if (!find && networkAbilityRow.isHover) {
      NetworkAbilityMonitorStruct.hoverNetworkAbilityStruct = undefined;
    }
    req.context.closePath();
    let textMetrics = req.context.measureText(req.maxNetworkRateName);
    req.context.globalAlpha = 0.8;
    req.context.fillStyle = '#f0f0f0';
    req.context.fillRect(0, 5, textMetrics.width + 8, 18);
    req.context.globalAlpha = 1;
    req.context.fillStyle = '#333';
    req.context.textBaseline = 'middle';
    req.context.fillText(req.maxNetworkRateName, 4, 5 + 9);
  }

  render(req: RequestMessage, list: Array<any>, filter: Array<any>): void {}
}

export class NetworkAbilityMonitorStruct extends BaseStruct {
  static maxNetworkRate: number = 0;
  static maxNetworkRateName: string = '0 KB/S';
  static hoverNetworkAbilityStruct: NetworkAbilityMonitorStruct | undefined;
  static selectNetworkAbilityStruct: NetworkAbilityMonitorStruct | undefined;
  value: number | undefined;
  startNS: number | undefined;

  static draw(
    networkAbilityContext2D: CanvasRenderingContext2D,
    networkAbilityData: NetworkAbilityMonitorStruct,
    maxNetworkRate: number,
    isHover: boolean
  ): void {
    if (networkAbilityData.frame) {
      let width = networkAbilityData.frame.width || 0;
      let index = 2;
      networkAbilityContext2D.fillStyle = ColorUtils.colorForTid(index);
      networkAbilityContext2D.strokeStyle = ColorUtils.colorForTid(index);
      if (networkAbilityData.startNS === NetworkAbilityMonitorStruct.hoverNetworkAbilityStruct?.startNS && isHover) {
        networkAbilityContext2D.lineWidth = 1;
        networkAbilityContext2D.globalAlpha = 0.6;
        let drawHeight: number = Math.floor(((networkAbilityData.value || 0) * (networkAbilityData.frame.height || 0) * 1.0) / maxNetworkRate);
        networkAbilityContext2D.fillRect(
          networkAbilityData.frame.x,
          networkAbilityData.frame.y + networkAbilityData.frame.height - drawHeight + 4,
          width,
          drawHeight
        );
        networkAbilityContext2D.beginPath();
        networkAbilityContext2D.arc(
          networkAbilityData.frame.x,
          networkAbilityData.frame.y + networkAbilityData.frame.height - drawHeight + 4,
          3,
          0,
          2 * Math.PI,
          true
        );
        networkAbilityContext2D.fill();
        networkAbilityContext2D.globalAlpha = 1.0;
        networkAbilityContext2D.stroke();
        networkAbilityContext2D.beginPath();
        networkAbilityContext2D.moveTo(networkAbilityData.frame.x + 3, networkAbilityData.frame.y + networkAbilityData.frame.height - drawHeight + 4);
        networkAbilityContext2D.lineWidth = 3;
        networkAbilityContext2D.lineTo(networkAbilityData.frame.x + width, networkAbilityData.frame.y + networkAbilityData.frame.height - drawHeight + 4);
        networkAbilityContext2D.stroke();
      } else {
        networkAbilityContext2D.globalAlpha = 0.6;
        networkAbilityContext2D.lineWidth = 1;
        let drawHeight: number = Math.floor(((networkAbilityData.value || 0) * (networkAbilityData.frame.height || 0)) / maxNetworkRate);
        networkAbilityContext2D.fillRect(
          networkAbilityData.frame.x,
          networkAbilityData.frame.y + networkAbilityData.frame.height - drawHeight + 4,
          width,
          drawHeight
        );
      }
    }
    networkAbilityContext2D.globalAlpha = 1.0;
    networkAbilityContext2D.lineWidth = 1;
  }
}
