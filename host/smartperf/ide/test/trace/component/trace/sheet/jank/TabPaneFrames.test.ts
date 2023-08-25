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
import { TabPaneFrames } from '../../../../../../dist/trace/component/trace/sheet/jank/TabPaneFrames.js';

window.ResizeObserver =
  window.ResizeObserver ||
  jest.fn().mockImplementation(() => ({
    disconnect: jest.fn(),
    observe: jest.fn(),
    unobserve: jest.fn(),
  }));

describe('TabPaneFrames Test', () => {
  let tabPaneFrames = new TabPaneFrames();

  let frameData = {
    leftNs: 253,
    rightNs: 1252,
    jankFramesData: [
      [
        {
          id: 25,
          ts: 254151,
          dur: 1202,
          name: '1583',
          depth: 1,
          jank_tag: true,
          cmdline: 'render.test',
          type: '0',
          pid: 20,
          frame_type: 'frameTime',
          src_slice: '525',
          rs_ts: 2569,
          rs_vsync: '2569',
          rs_dur: 1528,
          rs_pid: 1252,
          rs_name: 'name',
          gpu_dur: 2568,
          app_dur: 110,
        },
        {
          id: 69,
          ts: 4244,
          dur: 245,
          name: '2454',
          depth: 1,
          jank_tag: true,
          cmdline: 'app.jank.test',
          type: '0',
          pid: 65,
          frame_type: 'app',
          src_slice: '525',
          app_dur: 2345,
        },
        {
          id: 516,
          ts: 2164,
          dur: 2153,
          name: '1234',
          depth: 1,
          jank_tag: true,
          cmdline: 'render.test',
          type: '0',
          pid: 135,
          frame_type: 'renderService',
          src_slice: '446',
          rs_ts: 4364,
          rs_vsync: '1242',
          rs_dur: 642,
          rs_pid: 1266,
          rs_name: 'name',
          gpu_dur: 15646,
        },
        {
          id: 58,
          ts: 2156,
          dur: 45,
          name: '1452',
          depth: 1,
          jank_tag: false,
          cmdline: 'app.test',
          type: '0',
          pid: 128,
          frame_type: 'app',
          src_slice: '213',
          app_dur: 4563,
        },
        {
          id: 56389,
          ts: 56,
          dur: 563,
          name: '4786',
          depth: 1,
          jank_tag: false,
          cmdline: 'render.test',
          type: '0',
          pid: 456,
          frame_type: 'renderService',
          src_slice: '4563',
          rs_ts: 6356,
          rs_vsync: '434',
          rs_dur: 5464,
          rs_pid: 565,
          rs_name: 'name',
          gpu_dur: 5465,
        },
        {
          id: 1635,
          ts: 2153,
          dur: 13,
          name: '3153',
          depth: 1,
          jank_tag: false,
          cmdline: 'render.test',
          type: '0',
          pid: 123,
          frame_type: 'frameTime',
          src_slice: '53',
          rs_ts: 2135,
          rs_vsync: '203',
          rs_dur: 516,
          rs_pid: 513,
          rs_name: 'name',
          gpu_dur: 153,
          app_dur: 32,
        },
      ],
    ],
  };

  it('TabPaneFramesTest01', function () {
    tabPaneFrames.data = frameData;
    expect(tabPaneFrames.data).toBeUndefined();
  });

  it('TabPaneFramesTest02', function () {
    expect(
      tabPaneFrames.sortByColumn({
        key: 'jankType',
      })
    ).toBeUndefined();
  });
});
