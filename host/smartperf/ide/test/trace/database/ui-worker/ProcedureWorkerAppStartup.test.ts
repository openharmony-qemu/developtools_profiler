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
// @ts-ignore
import { Rect } from '../../../../dist/trace/component/trace/timer-shaft/Rect.js';
// @ts-ignore
import {
  AppStartupRender,
  AppStartupStruct,
} from '../../../../dist/trace/database/ui-worker/ProcedureWorkerAppStartup.js';
jest.mock('../../../../dist/trace/database/ui-worker/ProcedureWorker.js', () => {
  return {};
});

describe('ProcedureWorkerAppStartup Test', () => {
  it('AppStartupStructTest01', () => {
    const data = {
      frame: {
        x: 20,
        y: 20,
        width: 9,
        height: 3,
      },
      dur: 1,
      value: 'aa',
      startTs: 12,
      pid: 1,
      process: 'null',
      itid: 12,
      endItid: 13,
      tid: 1,
      startName: '23',
      stepName: 'st',
    };
    const canvas = document.createElement('canvas');
    canvas.width = 1;
    canvas.height = 1;
    const ctx = canvas.getContext('2d');
    expect(AppStartupStruct.draw(ctx, data)).toBeUndefined();
  });

  it('AppStartupStructTest02', () => {
    expect(AppStartupStruct.getStartupName(12)).toBe('Unknown Start Step');
  });
  it('AppStartupStructTest03', () => {
    expect(AppStartupStruct).not.toBeUndefined();
  });
  it('AppStartupStructTest04', () => {
    let canvas = document.createElement('canvas') as HTMLCanvasElement;
    let context = canvas.getContext('2d');
    const data = {
      useCache: true,
      appStartupContext: context,
      type: '',
    };
    let appStartupRender = new AppStartupRender();
    expect(appStartupRender.renderMainThread(data, new TraceRow())).toBeUndefined();
  });
});
