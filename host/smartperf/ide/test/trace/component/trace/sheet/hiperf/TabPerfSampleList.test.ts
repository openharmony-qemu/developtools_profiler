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

//@ts-ignore
import { TabPerfSampleList } from '../../../../../../dist/trace/component/trace/sheet/hiperf/TabPerfSampleList.js';
import '../../../../../../dist/trace/component/trace/sheet/hiperf/TabPerfSampleList.js';

const sqlite = require('../../../../../../dist/trace/database/SqlLite.js');
jest.mock('../../../../../../dist/trace/database/SqlLite.js');

jest.mock('../../../../../../dist/trace/component/trace/base/TraceRow.js', () => {
  return {}
});

window.ResizeObserver =
  window.ResizeObserver ||
  jest.fn().mockImplementation(() => ({
    disconnect: jest.fn(),
    observe: jest.fn(),
    unobserve: jest.fn(),
  }));

describe('TabPerfSampleList Test', () => {
  document.body.innerHTML = `<tabpane-perf-sample id="sampleList"></tabpane-perf-sample>`;
  let sampleList = document.querySelector('#sampleList') as TabPerfSampleList;
  let sampleListData = {
    leftNs: 1222,
    rightNs: 5286,
  };

  let perfSampleList = [
    {
      sampleId: 1,
      tid: 1522,
      threadName: 'test',
      state: 'state',
      pid: 44147,
      time: 4744,
      core: 155,
    },
  ];
  let perfSampleListByTimeRange = sqlite.queryPerfSampleListByTimeRange;

  perfSampleListByTimeRange.mockResolvedValue(perfSampleList);
  let perfProcess = sqlite.queryPerfProcess;
  perfProcess.mockResolvedValue([
    {
      pid: 44147,
      processName: 'test1',
    },
  ]);
  let perfSampleCallChainDate = [
    {
      callChainId: 255,
      csampleId: 5525,
      fileId: 4585,
      symbolId: 4855,
      vaddrInFile: 0,
      symbol: 'name',
    },
  ];
  let perfSampleCallChain = sqlite.queryPerfSampleCallChain;
  perfSampleCallChain.mockResolvedValue(perfSampleCallChainDate);

  it('TabPerfSampleListTest01', function () {
    sampleList.data = sampleListData;
    expect(sampleList.data).toBeUndefined();
  });

  it('TabPerfSampleListTest02', function () {});

  it('TabPerfSampleListTest03', function () {
    expect(sampleList.sortPerfSampleTable('timeString', 2)).toBeUndefined();
  });
});
