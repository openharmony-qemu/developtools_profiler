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

// @ts-ignore
import {
  fileSysChart,
  FileSysChartStruct,
} from '../../../../dist/trace/database/ui-worker/ProcedureWorkerFileSystem.js';

describe('ProcedureWorkerFileSystem Test', () => {
  it('ProcedureWorkerFileSystemTest01', function () {
    let frame = {
      x: 20,
      y: 20,
      width: 100,
      height: 100,
    };
    expect(fileSysChart([], [{ length: 1 }], '', [{ length: 1 }], 1, 2, 1, frame, true, true, true)).toBeUndefined();
  });

  it('ProcedureWorkerFileSystemTest02', function () {
    let frame = {
      x: 20,
      y: 20,
      width: 100,
      height: 100,
    };
    expect(fileSysChart([], [{ length: 1 }], '', [{ length: 0 }], 1, 2, 1, frame, false, false, false)).toBeUndefined();
  });

  it('ProcedureWorkerFileSystemTest03', function () {
    let frame = {
      x: 40,
      y: 24,
      width: 440,
      height: 500,
    };
    let fileSystemDataList = new Array();
    fileSystemDataList.push({
      startTime: 60,
      dur: 110,
      frame: { x: 40, y: 59, width: 140, height: 142 },
    });
    fileSystemDataList.push({ startTime: 14, dur: 41 });
    fileSysChart(fileSystemDataList, fileSystemDataList, '', [{ length: 0 }], 1, 2, 1, frame, true, false, false);
  });

  it('ProcedureWorkerFileSystemTest03', function () {
    let frame = {
      x: 24,
      y: 21,
      width: 121,
      height: 230,
    };
    let fileSystemDataList = new Array();
    fileSystemDataList.push({
      startTime: 20,
      dur: 11,
      frame: { x: 10, y: 91, width: 40, height: 540 },
    });
    fileSystemDataList.push({ startTime: 41, dur: 141 });
    fileSysChart(fileSystemDataList, fileSystemDataList, '', [{ length: 0 }], 1, 2, 1, frame, true, false, false);
  });

  it('ProcedureWorkerFileSystemTest04', function () {
    let node = {
      frame: {
        x: 60,
        y: 24,
        width: 440,
        height: 460,
      },
      startNS: 100,
      value: 980,
      startTs: 63,
      dur: 21,
      height: 222,
    };
    let frame = {
      x: 2,
      y: 21,
      width: 15,
      height: 84,
    };
    expect(FileSysChartStruct.setFrame(node, 2, 1, 2, frame)).toBeUndefined();
  });

  it('ProcedureWorkerFileSystemTest05', function () {
    let node = {
      frame: {
        x: 80,
        y: 20,
        width: 330,
        height: 330,
      },
      startNS: 2,
      value: 61,
      startTs: 32,
      dur: 8,
      height: 10,
    };
    let frame = {
      x: 66,
      y: 2,
      width: 600,
      height: 188,
    };
    expect(FileSysChartStruct.setFrame(node, 2, 1, 2, frame)).toBeUndefined();
  });

  it('ProcedureWorkerFileSystemTest06', function () {
    expect(FileSysChartStruct.computeHeightNoGroup([{ length: 1 }], 10)).toEqual([
      {
        dur: 10,
        group10Ms: false,
        height: 18,
        size: 1,
        startNS: 0,
      },
      { dur: 0, group10Ms: false, height: 0, size: 0, startNS: 10 },
    ]);
  });

  it('ProcedureWorkerFileSystemTest07', function () {
    expect(FileSysChartStruct.groupBy10MSWithMaxLatency([{ id: 1, NS: 3 }, { copy: '1' }])).toEqual([
      {
        dur: 10000000,
        height: NaN,
        startNS: NaN,
        group10Ms: true,
        size: undefined,
      },
    ]);
  });

  it('ProcedureWorkerFileSystemTest08', function () {
    expect(FileSysChartStruct.groupBy10MSWithCount([{ id: 1, NS: 3 }, { copy: '1' }])).toEqual([
      {
        dur: 10000000,
        height: 36,
        startNS: NaN,
        group10Ms: true,
        size: 2,
      },
    ]);
  });
});
