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
import '../../../../base-ui/icon/LitIcon.js';
import { LitIcon } from '../../../../base-ui/icon/LitIcon.js';
import '../../../../base-ui/popover/LitPopoverV.js';
import { LitCheckBox } from '../../../../base-ui/checkbox/LitCheckBox.js';
import { LitSelect } from '../../../../base-ui/select/LitSelect.js';
import '../../../../base-ui/select/LitSelect.js';
import { LitSelectOption } from '../../../../base-ui/select/LitSelectOption.js';
@element('tab-pane-js-memory-filter')
export class TabPaneJsMemoryFilter extends BaseElement {
  initElements(): void {}
  initHtml(): string {
    return `
        <style>
        :host{
            height: 30px;
            background: var(--dark-background4,#F2F2F2);
            border-top: 1px solid var(--dark-border1,#c9d0da);display: flex;align-items: center;z-index: 2;
            margin-left: -10px;
            width: calc(100% + 20px);
            position: fixed;
            bottom: 0px;
        }
        #js-memory-filter-input{
            background: var(--dark-background4,#FFFFFF);
            border: 1px solid var(--dark-border,rgba(0,0,0,0.60));
            color: var(--dark-color2,#000000);
            border-radius: 8px;
            margin-left: 10px;
            width: 200px;
        }
        #js-memory-filter-input:focus{
            outline: none;
            box-shadow: 1px 1px 1px var(--dark-color,#bebebe);
        }
        #js-memory-filter-input::-webkit-input-placeholder {
            color: var(--dark-color,#aab2bd);
        }
        .describe{
            margin-left: 10px;
            font-size: 0.8rem;
            white-space: nowrap;
            overflow: hidden;
            text-overflow: ellipsis;
            min-width: 50px;
        }
        :host(:not([first])) #comparison-select{
            display: none;
        }
        :host(:not([input])) .filter-wrap{
            display: none;
        }
        #comparison-select{
            width:150px;
            margin-left: 10px;
        }
        </style>
        <div class='filter-wrap'>
            <span class="describe left-text spacing">Class Filter</span>
            <input id="js-memory-filter-input" class="spacing" placeholder="Class filter" value=""/>
        </div>
        <div class='select-wrap'>
            <lit-select id='comparison-select' title='Select View'></lit-select>
        </div>
        `;
  }
}
