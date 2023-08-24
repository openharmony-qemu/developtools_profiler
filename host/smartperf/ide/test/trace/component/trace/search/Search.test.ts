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

  it('Search Test10', () => {
    let search = new LitSearch();
    expect(search.initHtml()).toMatchInlineSnapshot(`
"
        <style>
        :host{
        }
        .root{
            background-color: var(--dark-background5,#fff);
            border-radius: 40px;
            padding: 3px 20px;
            display: flex;
            justify-content: center;
            align-items: center;
            border: 1px solid var(--dark-border,#c5c5c5);
            width: 35vw;
            overflow: hidden;
            }
        .root input{
            outline: none;
            border: 0px;
            background-color: transparent;
            font-size: inherit;
            color: var(--dark-color,#666666);
            flex: 1;
            height: auto;
            vertical-align:middle;
            line-height:inherit;
            height:inherit;
            padding: 6px 6px 6px 6px};
            max-height: inherit;
            box-sizing: border-box;
        }
        ::placeholder {
          color: #b5b7ba;
          font-size: 1em;
        }
        .write::placeholder {
          color: #b5b7ba;
          font-size: 1em;
        }
        .readonly::placeholder {
          color: #4f7ab3;
          font-size: 1em;
        }
        .text-Roll::placeholder {
          font-weight: 700;
          color: #DB5860;
          font-size: 1em;
        }
        :host([show-search-info]) .search-info{
            display: inline-flex;
        }
        :host(:not([show-search-info])) .search-info{
            display: none;
        }
        .search-info span{
            color:#ABABAB;
        }
        .search-info lit-icon{
            font-weight: bold;
        }
        
        :host([textRoll]) input {
            position: relative;
            animation: textRoll 5s ease-in-out 0s backwards;
            white-space: nowrap;
            overflow: hidden;
            display: block;
            text-overflow: ellipsis;
        }
      
        @keyframes textRoll {
            0% {
                left: 0;
            }
            100% {
                left: 100%;
            }
        }       
        
        </style>
        <div class="root" style="display: none">
            <lit-icon id="search-icon" name="search" size="22" color="#aaaaaa">
            </lit-icon>
            <input class="readonly" placeholder="Search" readonly/>
            <div class="search-info">
                <span id="index">0</span><span>/</span><span id="total">0</span>
                <lit-icon class="icon" id="arrow-left" name="caret-left" color="#AAAAAA" size="26">
                </lit-icon>
                <span>|</span>
                <lit-icon class="icon" id="arrow-right"  name="caret-right" color="#AAAAAA" size="26">
                </lit-icon>
            </div>
        </div>
        "
`);
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
