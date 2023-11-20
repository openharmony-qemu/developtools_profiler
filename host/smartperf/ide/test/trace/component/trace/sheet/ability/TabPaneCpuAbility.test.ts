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
import { TabPaneCpuAbility } from '../../../../../../dist/trace/component/trace/sheet/ability/TabPaneCpuAbility.js';
const sqlite = require('../../../../../../dist/trace/database/SqlLite.js');
jest.mock('../../../../../../dist/trace/database/SqlLite.js');

jest.mock('../../../../../../dist/trace/component/trace/sheet/SheetUtils.js', () => {
  return {};
});

window.ResizeObserver =
  window.ResizeObserver ||
  jest.fn().mockImplementation(() => ({
    disconnect: jest.fn(),
    observe: jest.fn(),
    unobserve: jest.fn(),
  }));

describe('TabPaneCpuAbility Test', () => {
  let tabPaneCpuAbility = new TabPaneCpuAbility();
  let val = [
    {
      startNs: 0,
      rightNs: 1000,
      leftNs:0,
    }
  ];
  let getTabCpuData = sqlite.getTabCpuAbilityData;
  let cpuData = [
    {
      startTime: 0,
      duration: 1000,
      totalLoad:2,
      userLoad:2,
      systemLoad:3,
      threads:1,
    }
  ];
  getTabCpuData.mockResolvedValue(cpuData);
  it('TabPaneCpuAbilityTest01', function () {
    tabPaneCpuAbility.queryCpuResult.length = 2;
    expect(tabPaneCpuAbility.filterData()).toBeUndefined();
  });

  it('TabPaneCpuAbilityTest02', function () {
    const systemCpuSummary = {
      startTimeStr: '',
      durationStr: '',
      totalLoadStr: '',
      userLoadStr: '',
      systemLoadStr: '',
      threadsStr: '',
    };
    expect(tabPaneCpuAbility.toCpuAbilityArray(systemCpuSummary)).not.toBeUndefined();
  });

  it('TabPaneCpuAbilityTest03 ', function () {
    expect(
      tabPaneCpuAbility.sortByColumn({
        key: 'startTime',
      })
    ).toBeUndefined();
  });

  it('TabPaneCpuAbilityTest04 ', function () {
    expect(
      tabPaneCpuAbility.sortByColumn({
        key: !'startTime',
      })
    ).toBeUndefined();
  });

  it('TabPaneCpuAbilityTest05 ', function () {
    expect(
      tabPaneCpuAbility.sortByColumn({
        key: 'totalLoadStr',
      })
    ).toBeUndefined();
  });

  it('TabPaneCpuAbilityTest06 ', function () {
    expect(
      tabPaneCpuAbility.sortByColumn({
        key: 'userLoadStr',
      })
    ).toBeUndefined();
  });

  it('TabPaneCpuAbilityTest07 ', function () {
    expect(
      tabPaneCpuAbility.sortByColumn({
        key: 'systemLoadStr',
      })
    ).toBeUndefined();
  });

  it('TabPaneCpuAbilityTest08 ', function () {
    expect(
      tabPaneCpuAbility.sortByColumn({
        key: 'durationStr',
      })
    ).toBeUndefined();
  });
  it('TabPaneCpuAbilityTest09 ', function () {
    expect(
        tabPaneCpuAbility.queryDataByDB({val
        })
    ).toBeUndefined();
  });
});
