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
import { TabPaneDiskAbility } from '../../../../../../dist/trace/component/trace/sheet/ability/TabPaneDiskAbility.js';
jest.mock('../../../../../../dist/trace/component/trace/sheet/SheetUtils.js', () => {
  return {};
});
const sqlite = require('../../../../../../dist/trace/database/SqlLite.js');
jest.mock('../../../../../../dist/trace/database/SqlLite.js');
window.ResizeObserver =
  window.ResizeObserver ||
  jest.fn().mockImplementation(() => ({
    disconnect: jest.fn(),
    observe: jest.fn(),
    unobserve: jest.fn(),
  }));

describe('TabPaneDiskAbility Test', () => {
  let tabPaneDiskAbility = new TabPaneDiskAbility();
  let val = [
    {
      startNs: 0,
      rightNs: 1000,
      leftNs:0,
    }
  ];
  let getTabDiskAbilityData = sqlite.getTabDiskAbilityData;
  let diskAbilityData = [
    {
      startTime: 0,
      duration: 1000,
      dataRead:2,
      dataReadSec:2,
      dataWrite:3,
      dataWriteSec:1,
      readsIn:2,
      readsInSec:4,
      writeOut:2,
      writeOutSec:5
    }
  ];
  getTabDiskAbilityData.mockResolvedValue(diskAbilityData);
  it('TabPaneDiskAbilityTest01', () => {
    tabPaneDiskAbility.queryDiskResult.length = 1;
    expect(tabPaneDiskAbility.filterData()).toBeUndefined();
  });

  it('TabPaneDiskAbilityTest02 ', function () {
    const val = {
      startTimeStr: '',
      durationStr: '',
      dataReadStr: '',
      dataReadSecStr: '',
      dataWriteStr: '',
      readsInStr: '',
      readsInSecStr: '',
      writeOutStr: '',
      writeOutSecStr: '',
    };
    expect(tabPaneDiskAbility.toDiskAbilityArray(val)).not.toBeUndefined();
  });

  it('TabPaneDiskAbilityTest03 ', function () {
    expect(
      tabPaneDiskAbility.sortByColumn({
        key: 'startTime',
      })
    ).toBeUndefined();
  });

  it('TabPaneDiskAbilityTest04 ', function () {
    expect(
      tabPaneDiskAbility.sortByColumn({
        key: !'startTime',
      })
    ).toBeUndefined();
  });

  it('TabPaneDiskAbilityTest05 ', function () {
    expect(
      tabPaneDiskAbility.sortByColumn({
        key: 'durationStr',
      })
    ).toBeUndefined();
  });

  it('TabPaneDiskAbilityTest06 ', function () {
    expect(
      tabPaneDiskAbility.sortByColumn({
        key: 'dataReadStr',
      })
    ).toBeUndefined();
  });

  it('TabPaneDiskAbilityTest07 ', function () {
    expect(
      tabPaneDiskAbility.sortByColumn({
        key: 'dataReadSecStr',
      })
    ).toBeUndefined();
  });

  it('TabPaneDiskAbilityTest08 ', function () {
    expect(
      tabPaneDiskAbility.sortByColumn({
        key: 'dataWriteStr',
      })
    ).toBeUndefined();
  });

  it('TabPaneDiskAbilityTest09 ', function () {
    expect(
      tabPaneDiskAbility.sortByColumn({
        key: 'dataWriteSecStr',
      })
    ).toBeUndefined();
  });

  it('TabPaneDiskAbilityTest10 ', function () {
    expect(
      tabPaneDiskAbility.sortByColumn({
        key: 'readsInStr',
      })
    ).toBeUndefined();
  });

  it('TabPaneDiskAbilityTest11 ', function () {
    expect(
      tabPaneDiskAbility.sortByColumn({
        key: 'readsInSecStr',
      })
    ).toBeUndefined();
  });

  it('TabPaneDiskAbilityTest12', function () {
    expect(
      tabPaneDiskAbility.sortByColumn({
        key: 'writeOutStr',
      })
    ).toBeUndefined();
  });

  it('TabPaneDiskAbilityTest13', function () {
    expect(
      tabPaneDiskAbility.sortByColumn({
        key: 'writeOutSecStr',
      })
    ).toBeUndefined();
  });
  it('TabPaneDiskAbilityTest14', function () {
    expect(
        tabPaneDiskAbility.queryDataByDB({
         val
        })
    ).toBeUndefined();
  });
});
