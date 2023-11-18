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

import { BaseElement, element } from '../../../../base-ui/BaseElement.js';
import { LitIcon } from '../../../../base-ui/icon/LitIcon.js';

const LOCAL_STORAGE_SEARCH_KEY = 'search_key';

@element('lit-search')
export class LitSearch extends BaseElement {
  valueChangeHandler: ((str: string) => void) | undefined | null;
  private search: HTMLInputElement | undefined | null;
  private _total: number = 0;
  private _index: number = 0;
  private _list: Array<any> = [];
  private totalEL: HTMLSpanElement | null | undefined;
  private indexEL: HTMLSpanElement | null | undefined;
  private searchHistoryListEL: HTMLUListElement | null | undefined;
  private historyMaxCount = 100;
  private lastSearch = '';
  private searchList: Array<SearchInfo> = [];
  private searchELList: Array<HTMLElement> = [];

  get list(): Array<any> {
    return this._list;
  }

  set list(value: Array<any>) {
    this._list = value;
    this.total = value.length;
  }

  get index(): number {
    return this._index;
  }

  set index(value: number) {
    this._index = value;
    this.indexEL!.textContent = `${value + 1}`;
  }

  get searchValue() {
    return this.search?.value;
  }

  get total(): number {
    return this._total;
  }

  set total(value: number) {
    if (value > 0) {
      this.setAttribute('show-search-info', '');
    } else {
      this.removeAttribute('show-search-info');
    }
    this._total = value;
    this.totalEL!.textContent = value.toString();
  }

  get isLoading(): boolean {
    return this.hasAttribute('isLoading');
  }

  set isLoading(va) {
    if (va) {
      this.setAttribute('isLoading', '');
    } else {
      this.removeAttribute('isLoading');
      window.localStorage.setItem(LOCAL_STORAGE_SEARCH_KEY, '');
    }
  }

  setPercent(name: string = '', value: number) {
    let searchHide = this.shadowRoot!.querySelector<HTMLElement>('.root');
    let searchIcon = this.shadowRoot!.querySelector<HTMLElement>('#search-icon');
    if (this.hasAttribute('textRoll')) {
      this.removeAttribute('textRoll');
    }
    this.isLoading = false;
    if (value > 0 && value <= 100) {
      searchHide!.style.display = 'flex';
      searchHide!.style.backgroundColor = 'var(--dark-background5,#e3e3e3)';
      searchIcon?.setAttribute('name', 'cloud-sync');
      this.search!.setAttribute('placeholder', `${name}${value}%`);
      this.search!.setAttribute('readonly', '');
      this.search!.className = 'readonly';
      this.isLoading = true;
    } else if (value > 100) {
      searchHide!.style.display = 'flex';
      searchHide!.style.backgroundColor = 'var(--dark-background5,#fff)';
      searchIcon?.setAttribute('name', 'search');
      this.search?.setAttribute('placeholder', `search`);
      this.search?.removeAttribute('readonly');
      this.search!.className = 'write';
    } else if (value == -1) {
      searchHide!.style.display = 'flex';
      searchHide!.style.backgroundColor = 'var(--dark-background5,#e3e3e3)';
      searchIcon?.setAttribute('name', 'cloud-sync');
      this.search!.setAttribute('placeholder', `${name}`);
      this.search!.setAttribute('readonly', '');
      this.search!.className = 'readonly';
    } else if (value == -2) {
      searchHide!.style.display = 'flex';
      searchHide!.style.backgroundColor = 'var(--dark-background5,#e3e3e3)';
      searchIcon?.setAttribute('name', 'cloud-sync');
      this.search!.setAttribute('placeholder', `${name}`);
      this.search!.setAttribute('readonly', '');
      this.search!.className = 'text-Roll';
      setTimeout(() => {
        this.setAttribute('textRoll', '');
      }, 200);
    } else {
      searchHide!.style.display = 'none';
    }
  }

  clear() {
    this.search = this.shadowRoot!.querySelector<HTMLInputElement>('input');
    this.search!.value = '';
    this.list = [];
  }

  blur() {
    this.search?.blur();
  }

  updateSearchList(searchStr: string | null) {
    if (searchStr === null || searchStr.length === 0 || searchStr.trim().length === 0) {
      return;
    }
    if (this.lastSearch === searchStr) {
      return;
    }
    this.lastSearch = searchStr;
    let searchInfo = this.searchList.find((searchInfo) => searchInfo.searchContent === searchStr);
    if (searchInfo != undefined) {
      searchInfo.useCount += 1;
    } else {
      this.searchList.push({ searchContent: searchStr, useCount: 1 });
    }
  }

  getSearchHistory(): Array<SearchInfo> {
    let searchString = window.localStorage.getItem(LOCAL_STORAGE_SEARCH_KEY);
    if (searchString) {
      let searHistory = JSON.parse(searchString);
      if (Array.isArray(searHistory)) {
        this.searchList = searHistory;
        return searHistory;
      }
    }
    return [];
  }

  private searchFocusListener() {
    if (!this.search?.hasAttribute('readonly')) {
      this.showSearchHistoryList();
    }
    this.dispatchEvent(
      new CustomEvent('focus', {
        detail: {
          value: this.search!.value,
        },
      })
    );
  }

  private searchBlurListener() {
    this.dispatchEvent(
      new CustomEvent('blur', {
        detail: {
          value: this.search!.value,
        },
      })
    );
    setTimeout(() => {
      this.hideSearchHistoryList();
    }, 200);
  }

  private searchKeyupListener(e: KeyboardEvent) {
    if (e.code === 'Enter') {
      this.updateSearchList(this.search!.value);
      if (e.shiftKey) {
        this.dispatchEvent(
          new CustomEvent('previous-data', {
            detail: {
              value: this.search!.value,
            },
            composed: false,
          })
        );
      } else {
        this.dispatchEvent(
          new CustomEvent('next-data', {
            detail: {
              value: this.search!.value,
            },
            composed: false,
          })
        );
      }
    } else {
      this.updateSearchHistoryList(this.search!.value);
      this.valueChangeHandler?.(this.search!.value);
    }
    e.stopPropagation();
  }

  initElements(): void {
    this.search = this.shadowRoot!.querySelector<HTMLInputElement>('input');
    this.totalEL = this.shadowRoot!.querySelector<HTMLSpanElement>('#total');
    this.indexEL = this.shadowRoot!.querySelector<HTMLSpanElement>('#index');
    this.searchHistoryListEL = this.shadowRoot!.querySelector<HTMLUListElement>('.search-history-list');
    this.search!.addEventListener('focus', () => {
      this.searchFocusListener();
    });
    this.search!.addEventListener('blur', (e) => {
      this.searchBlurListener();
    });
    this.search!.addEventListener('change', (event) => {
      this.index = -1;
    });
    this.search!.addEventListener('keyup', (e: KeyboardEvent) => {
      this.searchKeyupListener(e);
    });
    this.shadowRoot?.querySelector('#arrow-left')?.addEventListener('click', (e) => {
      this.dispatchEvent(
        new CustomEvent('previous-data', {
          detail: {
            value: this.search!.value,
          },
        })
      );
    });
    this.shadowRoot?.querySelector('#arrow-right')?.addEventListener('click', (e) => {
      this.dispatchEvent(
        new CustomEvent('next-data', {
          detail: {
            value: this.search!.value,
          },
        })
      );
    });
  }

  initHtml(): string {
    return `
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
            width: max-content;
            border: 0px;
            background-color: transparent;
            font-size: inherit;
            color: var(--dark-color,#666666);
            flex: 1;
            height: auto;
            vertical-align:middle;
            line-height:inherit;
            height:inherit;
            padding: 6px 6px 6px 6px;
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
         .search-history {
            position: relative;
         }
        .search-history-list {
            list-style-type: none;
            margin: 0;
            position: absolute;
            width: 35vw;
            top: 100%;
            background-color: #FFFFFF;
            border: 1px solid #ddd;
            max-height: 200px;
            overflow-y: auto;
            display: none;
            border-radius: 0 0 20px 20px;
        }     
        .search-history-list:hover{
             cursor: pointer;
        }  
        .search-history-list-item {
            cursor: pointer;
            width: 100%;
        }
        .search-list:hover {
            background-color: #e9e9e9;
        }
        .search-list {
            display: flex;
            justify-content: space-between;
            padding-right: 20px;
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
        <div class="search-history">
              <ul class="search-history-list"></ul>
        </div>
        `;
  }

  showSearchHistoryList() {
    this.searchHistoryListEL!.innerHTML = '';
    let historyInfos = this.getSearchHistory();
    let fragment = document.createElement('div');
    historyInfos.forEach((historyInfo) => {
      let searchContainer = document.createElement('div');
      searchContainer.className = 'search-list';
      let searchInfoOption = document.createElement('li');
      let closeOption = document.createElement('lit-icon');
      closeOption.setAttribute('name', 'close');
      closeOption.className = 'close-option';
      closeOption.setAttribute('size', '20');
      searchInfoOption.className = 'search-history-list-item';
      searchInfoOption.textContent = historyInfo.searchContent;
      searchInfoOption.addEventListener('click', () => {
        if (searchInfoOption.textContent) {
          this.search!.value = searchInfoOption.textContent;
          this.valueChangeHandler?.(this.search!.value);
        }
      });
      searchContainer.append(searchInfoOption);
      searchContainer.append(closeOption);
      this.searchELList.push(searchInfoOption);
      this.searchELList.push(closeOption);
      fragment.append(searchContainer);
    });
    this.searchHistoryListEL?.append(fragment);
    this.searchHistoryListEL!.style.display = 'block';
    let closeOptionList = this.searchHistoryListEL!.querySelectorAll<LitIcon>('.close-option');
    closeOptionList.forEach((item) => {
      item.addEventListener('click', () => {
        let currentHistory = item.previousSibling!.textContent;
        let index = this.searchList.findIndex((element) => element.searchContent === currentHistory);
        if (index !== -1) {
          this.searchList.splice(index, 1);
        }
        let historyStr = JSON.stringify(this.searchList);
        window.localStorage.setItem(LOCAL_STORAGE_SEARCH_KEY, historyStr);
      });
    });
  }

  hideSearchHistoryList() {
    this.searchHistoryListEL!.style.display = 'none';
    this.searchList = this.searchList.sort((a, b) => {
      return b.useCount - a.useCount;
    });
    if (this.searchList.length > this.historyMaxCount) {
      this.searchList = this.searchList.slice(0, this.historyMaxCount);
    }
    if (this.searchList.length === 0) {
      return;
    }
    let historyStr = JSON.stringify(this.searchList);
    window.localStorage.setItem(LOCAL_STORAGE_SEARCH_KEY, historyStr);
    this.searchList = [];
    this.searchELList = [];
  }

  updateSearchHistoryList(searchValue: string) {
    const keyword = searchValue.toLowerCase();
    this.searchELList.forEach((item) => {
      if (item.textContent!.toLowerCase().includes(keyword)) {
        item.style.display = 'block';
      } else {
        item.style.display = 'none';
      }
    });
  }
}

export interface SearchInfo {
  searchContent: string;
  useCount: number;
}
