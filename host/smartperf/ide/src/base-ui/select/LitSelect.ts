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

import { BaseElement, element } from '../BaseElement.js';
import { selectHtmlStr } from './LitSelectHtml.js';

@element('lit-select')
export class LitSelect extends BaseElement {
  private focused: any;
  private selectInputEl: any;
  private selectClearEl: any;
  private selectIconEl: any;
  private bodyEl: any;
  private selectSearchEl: any;
  private selectMultipleRootEl: any;

  static get observedAttributes() {
    return [
      'value',
      'default-value',
      'placeholder',
      'disabled',
      'loading',
      'allow-clear',
      'show-search',
      'list-height',
      'border',
      'mode',
    ];
  }

  get value() {
    return this.getAttribute('value') || this.defaultValue;
  }

  set value(selectValue) {
    this.setAttribute('value', selectValue);
  }

  get rounded() {
    return this.hasAttribute('rounded');
  }

  set rounded(selectRounded: boolean) {
    if (selectRounded) {
      this.setAttribute('rounded', '');
    } else {
      this.removeAttribute('rounded');
    }
  }

  get placement(): string {
    return this.getAttribute('placement') || '';
  }

  set placement(selectPlacement: string) {
    if (selectPlacement) {
      this.setAttribute('placement', selectPlacement);
    } else {
      this.removeAttribute('placement');
    }
  }

  get border() {
    return this.getAttribute('border') || 'true';
  }

  set border(selectBorder) {
    if (selectBorder) {
      this.setAttribute('border', 'true');
    } else {
      this.setAttribute('border', 'false');
    }
  }

  get listHeight() {
    return this.getAttribute('list-height') || '256px';
  }

  set listHeight(selectListHeight) {
    this.setAttribute('list-height', selectListHeight);
  }

  get defaultPlaceholder() {
    return this.getAttribute('placeholder') || '请选择';
  }

  set canInsert(can: boolean) {
    if (can) {
      this.setAttribute('canInsert', '');
    } else {
      this.removeAttribute('canInsert');
    }
  }

  get canInsert() {
    return this.hasAttribute('canInsert');
  }
  get showSearch() {
    return this.hasAttribute('show-search');
  }

  get defaultValue() {
    return this.getAttribute('default-value') || '';
  }

  set defaultValue(selectDefaultValue) {
    this.setAttribute('default-value', selectDefaultValue);
  }

  get placeholder() {
    return this.getAttribute('placeholder') || this.defaultPlaceholder;
  }

  set placeholder(selectPlaceHolder) {
    this.setAttribute('placeholder', selectPlaceHolder);
  }

  get loading() {
    return this.hasAttribute('loading');
  }

  set loading(selectLoading) {
    if (selectLoading) {
      this.setAttribute('loading', '');
    } else {
      this.removeAttribute('loading');
    }
  }

  set dataSource(selectDataSource: any) {
    selectDataSource.forEach((dateSourceBean: any) => {
      let selectOption = document.createElement('lit-select-option');
      if (dateSourceBean.name) {
        selectOption.textContent = dateSourceBean.name;
        selectOption.setAttribute('value', dateSourceBean.name);
      }
      this.append(selectOption);
    });
    this.initOptions();
  }

  initElements(): void {}

  initHtml() {
    return `
        <style>
        ${selectHtmlStr()}
        :host(:not([mode]))  input{
            width: 100%;
        }
        .body{
            max-height: ${this.listHeight};
            overflow: auto;
            border-radius: 2px;
            box-shadow: 0 5px 15px 0px #00000033;
        }
        .multipleRoot input::-webkit-input-placeholder {
            color: var(--dark-color,#aab2bd);
        }
        :host(:not([loading])) .loading{
            display: none;
        }
        :host([loading]) .loading{
            display: flex;
        }
        :host(:not([allow-clear])) .clear{
            display: none;
        }
        :host([loading]) .icon{
            display: none;
        }
        :host(:not([loading])) .icon{
            display: flex;
        }
        .clear:hover{
            color: #8c8c8c;
        }
        .clear{
            color: #bfbfbf;
            display: none;
        }
        .multipleRoot{
            display: flex;
            align-items: center;
            flex-flow: wrap;
            flex-wrap: wrap;
            flex-direction: column;
        }
        .search{
            color: #bfbfbf;
            display: none;
        }
        .tag{
            overflow: auto;
            height: auto;
            display: inline-flex;
            position: relative;
            align-items: center;
            font-size: .75rem;
            font-weight: bold;
            padding: 1px 4px;
            margin-right: 4px;
            margin-top: 1px;
            margin-bottom: 1px;
            color: #242424;
            background-color: #f5f5f5;
        }
        .tag-close:hover{
            color: #333;
        }
        .tag-close{
            padding: 2px;
            font-size: .8rem;
            color: #999999;
            margin-left: 0px;
        }
        </style>
        <div class="root noSelect" tabindex="0" hidefocus="true">
            <div class="multipleRoot">
            <input placeholder="${this.placeholder}" autocomplete="off" ${
      this.showSearch || this.canInsert ? '' : 'readonly'
    } tabindex="0"></div>
            <lit-loading class="loading" size="12"></lit-loading>
            <lit-icon class="icon" name='down' color="#c3c3c3"></lit-icon>
            <lit-icon class="clear" name='close-circle-fill'></lit-icon>
            <lit-icon class="search" name='search'></lit-icon>
        </div>
        <div class="body">
            <slot></slot>
            <slot name="footer"></slot>
        </div>
        `;
  }

  isMultiple() {
    return this.hasAttribute('mode') && this.getAttribute('mode') === 'multiple';
  }

  newTag(value: any, text: any) {
    let tag: any = document.createElement('div');
    let icon: any = document.createElement('lit-icon');
    icon.classList.add('tag-close');
    icon.name = 'close';
    let span = document.createElement('span');
    tag.classList.add('tag');
    span.dataset['value'] = value;
    span.textContent = text;
    tag.append(span);
    tag.append(icon);
    icon.onclick = (ev: any) => {
      tag.parentElement.removeChild(tag);
      this.querySelector(`lit-select-option[value=${value}]`)!.removeAttribute('selected');
      if (this.shadowRoot!.querySelectorAll('.tag').length == 0) {
        this.selectInputEl.style.width = 'auto';
        this.selectInputEl.placeholder = this.defaultPlaceholder;
      }
      ev.stopPropagation();
    };
    tag.value = value;
    tag.dataset['value'] = value;
    tag.text = text;
    tag.dataset['text'] = text;
    return tag;
  }

  connectedCallback() {
    this.tabIndex = 0;
    this.focused = false;
    this.bodyEl = this.shadowRoot!.querySelector('.body');
    this.selectInputEl = this.shadowRoot!.querySelector('input');
    this.selectClearEl = this.shadowRoot!.querySelector('.clear');
    this.selectIconEl = this.shadowRoot!.querySelector('.icon');
    this.selectSearchEl = this.shadowRoot!.querySelector('.search');
    this.selectMultipleRootEl = this.shadowRoot!.querySelector('.multipleRoot');
    this.selectClearEl.onclick = (ev: any) => {
      if (this.isMultiple()) {
        let delNodes: Array<any> = [];
        this.selectMultipleRootEl.childNodes.forEach((a: any) => {
          if (a.tagName === 'DIV') {
            delNodes.push(a);
          }
        });
        for (let i = 0; i < delNodes.length; i++) {
          delNodes[i].remove();
        }
        if (this.shadowRoot!.querySelectorAll('.tag').length == 0) {
          this.selectInputEl.style.width = 'auto';
          this.selectInputEl.placeholder = this.defaultPlaceholder;
        }
      }
      this.querySelectorAll('lit-select-option').forEach((a) => a.removeAttribute('selected'));
      this.selectInputEl.value = '';
      this.selectClearEl.style.display = 'none';
      this.selectIconEl.style.display = 'flex';
      this.blur();
      ev.stopPropagation();
      this.dispatchEvent(new CustomEvent('onClear', { detail: ev }));
    };
    this.initOptions();
    this.onclick = (ev: any) => {
      if (ev.target.tagName === 'LIT-SELECT') {
        if (this.focused === false) {
          this.selectInputEl.focus();
          this.focused = true;
          this.bodyEl!.style.display = 'block';
        } else {
          this.blur();
          this.bodyEl!.style.display = 'none';
          this.focused = false;
        }
      }
    };
    this.onmouseover = this.onfocus = (ev) => {
      if (this.focused === false && this.hasAttribute('adaptive-expansion')) {
        if (this.parentElement!.offsetTop < this.bodyEl!.clientHeight) {
          this.bodyEl!.classList.add('body-bottom');
        } else {
          this.bodyEl!.classList.remove('body-bottom');
        }
      }
      if (this.hasAttribute('allow-clear')) {
        if (this.selectInputEl.value.length > 0 || this.selectInputEl.placeholder !== this.defaultPlaceholder) {
          this.selectClearEl.style.display = 'flex';
          this.selectIconEl.style.display = 'none';
        } else {
          this.selectClearEl.style.display = 'none';
          this.selectIconEl.style.display = 'flex';
        }
      }
    };
    this.onmouseout = this.onblur = (ev) => {
      if (this.hasAttribute('allow-clear')) {
        this.selectClearEl.style.display = 'none';
        this.selectIconEl.style.display = 'flex';
      }
      this.focused = false;
    };
    this.selectInputEl.onfocus = (ev: any) => {
      if (this.hasAttribute('disabled')) return;
      if (this.selectInputEl.value.length > 0) {
        this.selectInputEl.placeholder = this.selectInputEl.value;
        this.selectInputEl.value = '';
      }
      if (this.hasAttribute('show-search')) {
        this.selectSearchEl.style.display = 'flex';
        this.selectIconEl.style.display = 'none';
      }
      this.querySelectorAll('lit-select-option').forEach((a) => {
        // @ts-ignore
        a.style.display = 'flex';
      });
    };
    this.selectInputEl.onblur = (ev: any) => {
      if (this.hasAttribute('disabled')) return;
      if (this.isMultiple()) {
        if (this.hasAttribute('show-search')) {
          this.selectSearchEl.style.display = 'none';
          this.selectIconEl.style.display = 'flex';
        }
      } else {
        if (this.selectInputEl.placeholder !== this.defaultPlaceholder) {
          this.selectInputEl.value = this.selectInputEl.placeholder;
          this.selectInputEl.placeholder = this.defaultPlaceholder;
        }
        if (this.hasAttribute('show-search')) {
          this.selectSearchEl.style.display = 'none';
          this.selectIconEl.style.display = 'flex';
        }
      }
    };
    this.selectInputEl.oninput = (ev: any) => {
      let els = [...this.querySelectorAll('lit-select-option')];
      if (this.hasAttribute('show-search')) {
        if (!ev.target.value) {
          els.forEach((a: any) => (a.style.display = 'flex'));
        } else {
          els.forEach((a: any) => {
            let value = a.getAttribute('value');
            if (
              value.toLowerCase().indexOf(ev.target.value.toLowerCase()) !== -1 ||
              a.textContent.toLowerCase().indexOf(ev.target.value.toLowerCase()) !== -1
            ) {
              a.style.display = 'flex';
            } else {
              a.style.display = 'none';
            }
          });
        }
      } else {
        this.value = ev.target.value;
      }
    };
    this.selectInputEl.onkeydown = (ev: any) => {
      if (ev.key === 'Backspace') {
        if (this.isMultiple()) {
          let tag = this.selectMultipleRootEl.lastElementChild.previousElementSibling;
          if (tag) {
            this.querySelector(`lit-select-option[value=${tag.value}]`)?.removeAttribute('selected');
            tag.remove();
            if (this.shadowRoot!.querySelectorAll('.tag').length == 0) {
              this.selectInputEl.style.width = 'auto';
              this.selectInputEl.placeholder = this.defaultPlaceholder;
            }
          }
        } else {
          this.clear();
          this.dispatchEvent(new CustomEvent('onClear', { detail: ev })); //向外派发清理事件
        }
      } else if (ev.key === 'Enter') {
        if (!this.canInsert) {
          let filter = [...this.querySelectorAll('lit-select-option')].filter((a: any) => a.style.display !== 'none');
          if (filter.length > 0) {
            this.selectInputEl.value = filter[0].textContent;
            this.selectInputEl.placeholder = filter[0].textContent;
            this.blur();
            // @ts-ignore
            this.value = filter[0].getAttribute('value');
            this.dispatchEvent(
              new CustomEvent('change', {
                detail: {
                  selected: true,
                  value: filter[0].getAttribute('value'),
                  text: filter[0].textContent,
                },
              })
            );
          }
        }
      }
    };
  }

  initOptions() {
    this.querySelectorAll('lit-select-option').forEach((a) => {
      if (this.isMultiple()) {
        a.setAttribute('check', '');
        if (a.getAttribute('value') === this.defaultValue) {
          let tag = this.newTag(a.getAttribute('value'), a.textContent);
          this.selectMultipleRootEl.insertBefore(tag, this.selectInputEl);
          this.selectInputEl.placeholder = '';
          this.selectInputEl.value = '';
          this.selectInputEl.style.width = '1px';
          a.setAttribute('selected', '');
        }
      } else {
        if (a.getAttribute('value') === this.defaultValue) {
          this.selectInputEl.value = a.textContent;
          a.setAttribute('selected', '');
        }
      }
      a.addEventListener('mouseup', (e) => {
        e.stopPropagation();
      });
      a.addEventListener('mousedown', (e) => {
        e.stopPropagation();
      });
      a.addEventListener('onSelected', (e: any) => {
        if (this.isMultiple()) {
          if (a.hasAttribute('selected')) {
            let tag = this.shadowRoot!.querySelector(`div[data-value=${e.detail.value}]`) as HTMLElement;
            if (tag) {
              tag.parentElement!.removeChild(tag);
            }
            e.detail.selected = false;
          } else {
            let tag = this.newTag(e.detail.value, e.detail.text);
            this.selectMultipleRootEl.insertBefore(tag, this.selectInputEl);
            this.selectInputEl.placeholder = '';
            this.selectInputEl.value = '';
            this.selectInputEl.style.width = '1px';
          }
          if (this.shadowRoot!.querySelectorAll('.tag').length == 0) {
            this.selectInputEl.style.width = 'auto';
            this.selectInputEl.placeholder = this.defaultPlaceholder;
          }
          this.selectInputEl.focus();
        } else {
          [...this.querySelectorAll('lit-select-option')].forEach((a) => a.removeAttribute('selected'));
          this.blur();
          this.bodyEl!.style.display = 'none';
          // @ts-ignore
          this.selectInputEl.value = e.detail.text;
        }
        if (a.hasAttribute('selected')) {
          a.removeAttribute('selected');
        } else {
          a.setAttribute('selected', '');
        }
        // @ts-ignore
        this.value = e.detail.value;
        this.dispatchEvent(new CustomEvent('change', { detail: e.detail })); //向外层派发change事件，返回当前选中项
      });
    });
  }

  clear() {
    this.selectInputEl.value = '';
    this.selectInputEl.placeholder = this.defaultPlaceholder;
  }

  reset() {
    this.querySelectorAll('lit-select-option').forEach((a) => {
      [...this.querySelectorAll('lit-select-option')].forEach((a) => a.removeAttribute('selected'));
      if (a.getAttribute('value') === this.defaultValue) {
        this.selectInputEl.value = a.textContent;
        a.setAttribute('selected', '');
      }
    });
  }

  disconnectedCallback() {}

  adoptedCallback() {}

  attributeChangedCallback(name: any, oldValue: any, newValue: any) {
    if (name === 'value' && this.selectInputEl) {
      if (newValue) {
        [...this.querySelectorAll('lit-select-option')].forEach((a) => {
          if (a.getAttribute('value') === newValue) {
            a.setAttribute('selected', '');
            this.selectInputEl.value = a.textContent;
          } else {
            a.removeAttribute('selected');
          }
        });
      } else {
        this.clear();
      }
    }
  }
}
