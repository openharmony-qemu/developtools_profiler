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
import { TabPaneFilter } from '../../../../../dist/trace/component/trace/sheet/TabPaneFilter.js';

describe('TabPaneFilter Test', () => {
  let tabPaneFilter = new TabPaneFilter();

  it('TabPaneFilterTest01', function () {
    expect(tabPaneFilter.firstSelect).toBe('');
  });

  it('TabPaneFilterTest02', function () {
    expect(tabPaneFilter.secondSelect).toBe('');
  });

  it('TabPaneFilterTest03', function () {
    expect(tabPaneFilter.filterValue).toBe('');
  });

  it('TabPaneFilterTest04', function () {
    tabPaneFilter.filterValue = true;
    expect(tabPaneFilter.filterValue).toBeTruthy();
  });

  it('TabPaneFilterTest05', function () {
    expect(tabPaneFilter.icon).toBe('block');
  });

  it('TabPaneFilterTest08', function () {
    tabPaneFilter.iconEL.name = 'menu';
    expect(tabPaneFilter.icon).toBe('block');
  });

  it('TabPaneFilterTest09', function () {
    tabPaneFilter.iconEL.name = '';
    expect(tabPaneFilter.icon).toBe('');
  });

  it('TabPaneFilterTest06', function () {
    tabPaneFilter.icon = true;
    expect(tabPaneFilter.icon).toBe('');
  });

  it('TabPaneFilterTest010', function () {
    tabPaneFilter.icon = 'block';
    expect(tabPaneFilter.icon).toBe('block');
  });

  it('TabPaneFilterTest011', function () {
    tabPaneFilter.icon = 'tree';
    expect(tabPaneFilter.icon).toBe('tree');
  });

  it('TabPaneFilterTest10', function () {
    expect(tabPaneFilter.addDataMining({ name: '' }, '')).toBe(-1);
  });

  it('TabPaneFilterTest11', function () {
    expect(tabPaneFilter.getFilterTreeData()).not.toBeUndefined();
  });

  it('TabPaneFilterTest12', function () {
    expect(tabPaneFilter.initializeFilterTree(true, true, true)).toBeUndefined();
  });

  it('TabPaneFilterTest13', function () {
    expect(tabPaneFilter.disabledMining).toBeFalsy();
  });

  it('TabPaneFilterTest14', function () {
    tabPaneFilter.disabledMining = true;
    expect(tabPaneFilter.disabledMining).toBeTruthy();
  });
  it('TabPaneFilterTest15', function () {
    let mouseClickEvent: MouseEvent = new MouseEvent('click', <MouseEventInit>{ clientX: 1, clientY: 2 });
    tabPaneFilter.iconEL.name = 'statistics';
    tabPaneFilter.iconEL.dispatchEvent(mouseClickEvent);
  });
  it('TabPaneFilterTest16', function () {
    let mouseClickEvent: MouseEvent = new MouseEvent('click', <MouseEventInit>{ clientX: 1, clientY: 2 });
    tabPaneFilter.iconEL.name = 'menu';
    tabPaneFilter.iconEL.dispatchEvent(mouseClickEvent);
  });
  it('TabPaneFilterTest17', function () {
    let mouseClickEvent: MouseEvent = new MouseEvent('click', <MouseEventInit>{ clientX: 1, clientY: 2 });
    tabPaneFilter.getFilter = jest.fn(()=>true);
    tabPaneFilter.markButtonEL.dispatchEvent(mouseClickEvent);
  });
  it('TabPaneFilterTest18', function () {
    let mouseChangeEvent: MouseEvent = new MouseEvent('change', <MouseEventInit>{ clientX: 1, clientY: 2 });
    tabPaneFilter.firstSelectEL.dispatchEvent(mouseChangeEvent);
  });
  it('TabPaneFilterTest19', function () {
    let mouseChangeEvent: MouseEvent = new MouseEvent('change', <MouseEventInit>{ clientX: 1, clientY: 2 });
    tabPaneFilter.secondSelectEL.dispatchEvent(mouseChangeEvent);
  });
  it('TabPaneFilterTest20', function () {
    let mouseChangeEvent: MouseEvent = new MouseEvent('change', <MouseEventInit>{ clientX: 1, clientY: 2 });
    tabPaneFilter.thirdSelectEL.dispatchEvent(mouseChangeEvent);
  });
});
