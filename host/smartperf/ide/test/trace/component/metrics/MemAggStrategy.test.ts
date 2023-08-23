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
import { initMemoryAggStrategy } from '../../../../dist/trace/component/metrics/MemAggStrategy.js';

describe('MemAggStrategy Test', () => {
  it('initMemoryAggStrategyTest01', () => {
    const metricData = [
      {
        length: 1,
        processName: 'name',
        name: 'oom_score_adj',
        value: '',
        ts: '',
      },
    ];
    expect(initMemoryAggStrategy(metricData)).toBeTruthy();
  });

  it('initMemoryAggStrategyTest02', () => {
    const metricData = [
      {
        length: 1,
        processName: 'name',
        name: 'mem.rss.anon',
        value: '',
        ts: '',
      },
    ];
    expect(initMemoryAggStrategy(metricData)).toBeTruthy();
  });

  it('initMemoryAggStrategyTest03', () => {
    const metricData = [
      {
        length: 1,
        processName: 'name',
        name: 'mem.swap',
        value: '',
        ts: '',
      },
    ];
    expect(initMemoryAggStrategy(metricData)).toBeTruthy();
  });

  it('initMemoryAggStrategyTest04', () => {
    const metricData = [
      {
        length: 1,
        processName: 'name',
        name: 'mem.rss.file',
        value: '',
        ts: '',
      },
    ];
    expect(initMemoryAggStrategy(metricData)).toBeTruthy();
  });

  it('initMemoryAggStrategyTest05', () => {
    const metricData = [
      {
        length: 1,
        processName: 'name',
        name: 'oom_score_adj',
        value: '',
        ts: '',
      },
    ];
    expect(initMemoryAggStrategy(metricData)).toBeTruthy();
  });
});
