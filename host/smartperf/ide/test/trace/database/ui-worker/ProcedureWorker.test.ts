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
import { ProcedureWorker, drawSelection } from '../../../dist/trace/database/ProcedureWorker.js';

describe('ProcedureWorker Test', () => {
  it('ProcedureWorkerTest01', function () {
    const context = {
      globalAlpha: 0.5,
      fillStyle: '#666666',
      fillRect: '',
    };
    const params = {
      isRangeSelect: {},
      rangeSelectObject: {
        startX: '',
        endX: '',
        startNS: '',
        endNS: '',
      },
      startNS: '',
      endNS: '',
      totalNS: 1,
      frame: {
        x: '',
        y: '',
        height: 1,
        width: 1,
      },
    };
    let drawSelection = jest.fn(() => true);
    // @ts-ignore
    expect(drawSelection(context, params)).toBeTruthy();
  });
});
