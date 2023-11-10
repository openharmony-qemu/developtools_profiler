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
import { LitSearch } from '../../../../../dist/trace/component/trace/search/Search.js';

describe(' SearchTest', () => {
  beforeAll(() => {});
  it('Search Test01', () => {
    let search = new LitSearch();
    expect(search).not.toBeUndefined();
  });

  it('Search Test02', () => {
    let search = new LitSearch();
    search.list = ['1'];
    expect(search.list[0]).toBe('1');
  });

  it('Search Test03', () => {
    let search = new LitSearch();
    search.index = 1;
    expect(search.index).toBe(1);
  });

  it('Search Test04', () => {
    let search = new LitSearch();
    search.index = 1;
    expect(search.total).toBe(0);
  });

  it('Search Test05', () => {
    let search = new LitSearch();
    search.index = 1;
    expect(search.setPercent('1', 2)).toBeUndefined();
  });

  it('Search Test06', () => {
    let search = new LitSearch();
    search.index = 1;
    expect(search.setPercent('1', 101)).toBeUndefined();
  });

  it('Search Test07', () => {
    let search = new LitSearch();
    search.index = 1;
    expect(search.setPercent('1', -1)).toBeUndefined();
  });

  it('Search Test08', () => {
    let search = new LitSearch();
    search.index = 1;
    expect(search.setPercent('1', -2)).toBeUndefined();
  });

  it('Search Test09', () => {
    let search = new LitSearch();
    expect(search.clear()).toBeUndefined();
  });

  it('Search Test11', function () {
    let search = new LitSearch();
    search.search = jest.fn(() => undefined);
    search.search.blur = jest.fn(() => true);
    expect(search.blur()).toBeUndefined();
  });

  it('Search Test12', () => {
    let search = new LitSearch();
    expect(search.searchValue).toBe('');
  });

  it('Search Test13', () => {
    let search = new LitSearch();
    expect(search.isLoading).toBeFalsy();
  });
});
