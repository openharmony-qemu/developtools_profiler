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

jest.mock('../../../../dist/trace/component/trace/base/TraceRow.js', () => {
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
    startNS: 21,
    value: 5,
  };
  let res = [
    {
      startNS: 0,
      dur: 10,
      frame: {
        x: 0,
        y: 9,
        width: 10,
        height: 10,
      },
    },
  ];

  it('ProcedureWorkerNetworkAbilityTest01', function () {
    expect(NetworkAbilityMonitorStruct.draw(ctx, data)).toBeUndefined();
  });

  it('ProcedureWorkerNetworkAbilityTest02', function () {
    let networkAbilityRender = new NetworkAbilityRender();
    let req = {
      lazyRefresh: true,
      type: '',
      startNS: 1,
      endNS: 1,
      totalNS: 1,
      frame: {
        x: 20,
        y: 20,
        width: 100,
        height: 100,
      },
      useCache: false,
      range: {
        refresh: '',
      },
      canvas: '',
      context: {
        font: '11px sans-serif',
        fillStyle: '#ec407a',
        globalAlpha: 0.6,
      },
      lineColor: '',
      isHover: '',
      hoverX: 1,
      params: '',
      wakeupBean: undefined,
      flagMoveInfo: '',
      flagSelectedInfo: '',
      slicesTime: 3,
      id: 1,
      x: 20,
      y: 20,
      width: 100,
      height: 100,
    };
    window.postMessage = jest.fn(() => true);
    expect(networkAbilityRender.render(req, [], [])).toBeUndefined();
  });
});
