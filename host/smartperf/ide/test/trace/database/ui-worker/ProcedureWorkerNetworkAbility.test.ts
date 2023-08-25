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

// @ts-ignore
import { TraceRow } from '../../../../dist/trace/component/trace/base/TraceRow.js';

jest.mock('../../../../dist/trace/database/ui-worker/ProcedureWorker.js', () => {
  return {};
});

//@ts-ignore
import {
  NetworkAbilityMonitorStruct,
  NetworkAbilityRender,
} from '../../../../dist/trace/database/ui-worker/ProcedureWorkerNetworkAbility.js';

describe('ProcedureWorkerNetworkAbility Test', () => {
  const canvas = document.createElement('canvas');
  canvas.width = 1;
  canvas.height = 1;
  const ctx = canvas.getContext('2d');

  const data = {
    frame: {
      x: 2,
      y: 2,
      width: 10,
      height: 10,
    },
    startNS: 11,
    value: 15,
  };
  let res = [
    {
      startNS: 12,
      dur: 13,
      frame: {
        x: 440,
        y: 94,
        width: 40,
        height: 140,
      },
    },
  ];

  it('ProcedureWorkerNetworkAbilityTest01', function () {
    expect(NetworkAbilityMonitorStruct.draw(ctx, data)).toBeUndefined();
  });

  it('ProcedureWorkerNetworkAbilityTest02', function () {
    let networkAbilityRender = new NetworkAbilityRender();
    let networkAbility = {
      lazyRefresh: true,
      type: '',
      startNS: 32,
      endNS: 33,
      totalNS: 1,
      frame: {
        x: 23,
        y: 23,
        width: 603,
        height: 103,
      },
      useCache: false,
      range: {
        refresh: '',
      },
      canvas: '',
      context: {
        font: '11px sans-serif',
        fillStyle: '#272822',
        globalAlpha: 0.6,
      },
      lineColor: '#120f82',
      isHover: '',
      hoverX: 10,
      params: '',
      wakeupBean: undefined,
      flagMoveInfo: '',
      flagSelectedInfo: '',
      slicesTime: 13,
      id: 1,
      x: 60,
      y: 60,
      width: 100,
      height: 106,
    };
    window.postMessage = jest.fn(() => true);
    expect(networkAbilityRender.render(networkAbility, [], [])).toBeUndefined();
  });
  it('ProcedureWorkerNetworkAbilityTest03', function () {
    let networkAbilityRender = new NetworkAbilityRender();
    let canvas = document.createElement('canvas') as HTMLCanvasElement;
    let context = canvas.getContext('2d');
    const data = {
      context: context!,
      useCache: true,
      type: '',
      traceRange: [],
    };
    window.postMessage = jest.fn(() => true);
    expect(networkAbilityRender.renderMainThread(data, new TraceRow())).toBeUndefined();
  });
});
