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
import { TabpanePerfBottomUp } from '../../../../../../dist/trace/component/trace/sheet/hiperf/TabPerfBottomUp.js';
//@ts-ignore
import { showButtonMenu } from '../../../../../../dist/trace/component/trace/sheet/SheetUtils.js';

jest.mock('../../../../../../dist/trace/component/trace/base/TraceRow.js', () => {
  return {};
});
jest.mock('../../../../../../dist/base-ui/table/lit-table.js', () => {
  return {
    snapshotDataSource: () => {},
    removeAttribute: () => {},
  };
});
jest.mock('../../../../../../dist/js-heap/model/DatabaseStruct.js', () => {});
window.ResizeObserver =
  window.ResizeObserver ||
  jest.fn().mockImplementation(() => ({
    disconnect: jest.fn(),
    observe: jest.fn(),
    unobserve: jest.fn(),
  }));
describe('TabPanePerfBottomUp Test', () => {
  let tabPanePerfBottomUp = new TabpanePerfBottomUp();
  let data = {
    leftNs: 1222,
    rightNs: 5286,
  };
  it('TabPanePerfBottomUp02 ', function () {
    tabPanePerfBottomUp.data = data;
    expect(tabPanePerfBottomUp.data).toBeUndefined();
  });
  it('TabPanePerfBottomUp03 ', function () {
    expect(tabPanePerfBottomUp.setBottomUpTableData([])).toBeUndefined();
  });
});
