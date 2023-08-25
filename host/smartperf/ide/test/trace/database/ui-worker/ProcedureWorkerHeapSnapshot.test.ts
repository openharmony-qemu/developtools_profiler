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
  HeapSnapshot,
  HeapSnapshotRender,
  HeapSnapshotStruct,
} from '../../../../dist/trace/database/ui-worker/ProcedureWorkerHeapSnapshot.js';

jest.mock('../../../../dist/trace/database/ui-worker/ProcedureWorker.js', () => {
  return {};
});

describe('ProcedureWorkerHeapTimeline Test', () => {
  it('HeapSnapshotTest', () => {
    const canvas = document.createElement('canvas');
    canvas.width = 1;
    canvas.height = 1;
    const ctx = canvas.getContext('2d');
    let dataList = new Array();
    dataList.push({
      startTime: 0,
      dur: 10,
      frame: { x: 0, y: 9, width: 10, height: 10 },
    });
    dataList.push({ startTime: 1, dur: 111 });
    let rect = new Rect(0, 10, 10, 10);
    let filter = [
      {
        end_time: 50,
        end_ts: 1520000,
        file_name: 'Snapshot0',
        frame: { x: 0, y: 0, width: 25, height: 40 },
        id: 0,
        pid: 4243,
        start_time: 0,
        start_ts: 88473061693464,
        textMetricsWidth: 50.5810546875,
      },
    ];
    let list = [
      {
        end_time: 50,
        end_ts: 1520000,
        pid: 4243,
        start_time: 0,
        start_ts: 88473061693464,
        textMetricsWidth: 50.5810546875,
        file_name: 'Snapshot0',
        frame: { x: 0, y: 0, width: 6222, height: 62222 },
        id: 0,
      },
    ];
    HeapSnapshot(list, filter, 100254, 100254, rect, { height: 40, width: 1407, x: 0, y: 0 });
  });

  it('HeapSnapshotStructTest01', () => {
    const data = {
      cpu: 3,
      startNs: 31,
      value: 91,
      frame: {
        x: 23,
        y: 22,
        width: 200,
        height: 120,
      },
      maxValue: undefined,
      startTime: 31,
      filterID: 23,
      size: 106,
    };
    const canvas = document.createElement('canvas');
    canvas.width = 1;
    canvas.height = 1;
    const ctx = canvas.getContext('2d');
    expect(HeapSnapshotStruct.draw(ctx, data)).toBeUndefined();
  });

  it('HeapSnapshotStructTest02', () => {
    const data = {
      cpu: 0,
      startNs: 71,
      value: 17,
      frame: {
        x: 270,
        y: 250,
        width: 500,
        height: 606,
      },
      maxValue: undefined,
      startTime: 61,
      filterID: 21,
    };
    let node = {
      start_time: 3,
      end_time: 4,
      frame: null,
    };
    expect(HeapSnapshotStruct.setFrame(node, 0, 1, 2, data)).toBeUndefined();
  });

  it('HeapSnapshotRenderTest03', () => {
    let canvas = document.createElement('canvas') as HTMLCanvasElement;
    let context = canvas.getContext('2d');
    const data = {
      context: context!,
      useCache: true,
      type: '',
      traceRange: [],
    };
    let heapSnapshotRender = new HeapSnapshotRender();
    expect(heapSnapshotRender.renderMainThread(data, new TraceRow())).toBeUndefined();
  });
  it('HeapSnapshotStructTest04', () => {
    expect(HeapSnapshotStruct).not.toBeUndefined();
  });
});
