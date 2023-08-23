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
import { TabpanePerfProfile } from '../../../../../../dist/trace/component/trace/sheet/hiperf/TabPerfProfile.js';
//@ts-ignore
import {showButtonMenu} from "../../../../../../dist/trace/component/trace/sheet/SheetUtils.js";

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

describe('TabPerfProfile Test', () => {
  document.body.innerHTML = `<tabpane-perf-profile id="perfprofile"></tabpane-perf-profile>`;
  let tabpanePerfProfile = document.querySelector('#perfprofile') as TabpanePerfProfile;

  it('TabpanePerfProfileTest01 ', function () {
    TabpanePerfProfile.getParentTree = jest.fn(() => true);
    let call = {
      id: '1',
      dur: 1,
      children: [],
    };
    let id = '1';
    expect(tabpanePerfProfile.getParentTree([call], { id }, [])).not.toBeUndefined();
  });
  it('TabpanePerfProfileTest02 ', function () {
    let call = {
      id: '1',
      dur: 1,
      children: [],
    };
    expect(tabpanePerfProfile.getChildTree([call], '1', [])).not.toBeUndefined();
  });

  it('TabpanePerfProfileTest03 ', function () {
    let call = {
      id: '1',
      dur: 1,
      children: [],
    };
    expect(tabpanePerfProfile.setRightTableData(call)).toBeUndefined();
  });
  it('TabpanePerfProfileTest05 ', function () {
    let htmlDivElement = document.createElement('div');
    expect(showButtonMenu(htmlDivElement, true)).toBeUndefined();
  });
  it('TabpanePerfProfileTest06 ', function () {
    TabpanePerfProfile.getParentTree = jest.fn(() => true);
    let call = {
      id: '1',
      dur: 1,
      children: [],
    };
    let id = '2';

    expect(tabpanePerfProfile.getParentTree([call], { id }, [])).not.toBeUndefined();
  });
  it('TabpanePerfProfileTest08 ', function () {
    let data = {
      icon: 'tree',
    };
    tabpanePerfProfile.perfProfilerTbl!.reMeauseHeight = jest.fn(() => true);
    expect(tabpanePerfProfile.switchFlameChart(data)).toBeUndefined();
  });
  it('TabpanePerfProfileTest07 ', function () {
    let data = {
      icon: 1,
    };
    expect(tabpanePerfProfile.switchFlameChart(data)).toBeUndefined();
  });
  it('TabpanePerfProfileTest09 ', function () {
    tabpanePerfProfile.sortTree = jest.fn(() => true);
    tabpanePerfProfile.sortTree.sort = jest.fn(() => true);
    expect(tabpanePerfProfile.setPerfProfilerLeftTableData()).toBeUndefined();
  });
  it('TabpanePerfProfileTest10 ', function () {
    tabpanePerfProfile.getDataByWorker = jest.fn();
    tabpanePerfProfile.data = [
      {
        leftNs: 2565,
        rightNs: 2632,
      },
    ];
    expect(tabpanePerfProfile.data).toBeUndefined();
  });
});
