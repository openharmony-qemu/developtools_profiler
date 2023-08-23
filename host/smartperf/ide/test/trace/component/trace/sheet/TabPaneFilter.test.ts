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
    expect(tabPaneFilter.icon).toBe('tree');
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

  it('TabPaneFilterTest07', function () {
    expect(tabPaneFilter.initHtml()).toMatchInlineSnapshot(`
"
        <style>
        :host{
            height: 30px;
            background: var(--dark-background4,#F2F2F2);
            border-top: 1px solid var(--dark-border1,#c9d0da);display: flex;align-items: center;z-index: 2;
            margin-left: -10px;
            width: calc(100% + 20px);
        }

        .chosen-single {
            position: relative;
            display: block;
            overflow: hidden;
            text-decoration: none;
            white-space: nowrap;
            height: 34px;
            padding: 3px 6px;
            font-size: 14px;
            line-height: 1.42857143;
            color: #555;
            background-color: #fff;
            background-image: none;
            border: 1px solid #ccc;
            border-radius: 4px;
            transition: border-color ease-in-out .15s,box-shadow ease-in-out .15s;
            box-shadow: inset 0 1px 1px rgba(0,0,0,.075);    
        }
        .disabled{
        color: rgba(0,0,0,0.4);
        }
        #filter-input{
        background: var(--dark-background4,#FFFFFF);
        border: 1px solid var(--dark-border,rgba(0,0,0,0.60));
        color: var(--dark-color2,#000000);
        border-radius: 8px;
        width: 200px;
        }
        #filter-input:focus{
            outline: none;
            box-shadow: 1px 1px 1px var(--dark-color,#bebebe);
        }
        #filter-input::-webkit-input-placeholder {
                color: var(--dark-color,#aab2bd);
            }
        .describe{
            /*display: inline-block*/
            font-size: 0.8rem;
            white-space: nowrap;
            overflow: hidden;
            text-overflow: ellipsis;
            min-width: 50px;
        }

        #mark{
            border: 1px solid var(--bark-prompt,#999999);
            border-radius: 1px;
            background: var(--dark-background4,#F2F2F2);
            color: var(--dark-color2,rgba(0,0,0,0.9));
            transition: all 0.1s;
        }
        #mark:hover{
            background: var(--dark-background1,#dfdfdf);
        }
        #mark:active{
            background: var(--dark-background4,#F2F2F2);
            transition: all 0.05s;
        }
        #first-select{
        width: 200px;
        }
        #second-select{
        width: 200px;
        }
        .spacing{
        margin-left: 10px;
        }
        .max-spacing{
        margin-left: 15px;
        }

        :host(:not([inputLeftText])) .left-text{
            display: none;
        }
        :host(:not([input])) #filter-input{
            display: none;
        }
        :host(:not([mark])) #mark{
            display: none;
        }
        :host(:not([first])) #first-select{
            display: none;
        }
        :host(:not([second])) #second-select{
            display: none;
        }
        :host(:not([third])) #third-select{
            display: none;
        }
        :host(:not([tree])) .tree{
            display: none;
        }
        :host([disabledMining]) #data-mining{
            display: none;
        }
        :host([disabledMining]) #data-library{
            display: none;
        }
        :host(:not([icon])) #icon{
            display: none;
        }
        #icon[name="statistics"]{
            margin-left: 12px;
        }
        
        .constraints-input{
            background: var(--dark-border,#ffffff);
            color: var(--dark-color1,rgba(0,0,0,0.86));
            border: 1px solid var(--dark-border,rgba(0,0,0,0.60));
            border-radius: 10px;
            width: 40px;
            margin-left: 10px;
            outline: none;
        }
        .constraints-input[disabled]{
            background: var(--dark-background5,#ededed);
        }
        .reset-button{
            opacity: 0.9;
            font-size: 13px;
            color: #0A59F7;
            text-align: center;
            line-height: 16px;
            background: var(--dark-background3,#F4F3F4);
            border: 1px solid var(--dark-background8,#F4F3F4);
            border-radius: 16px;
            padding: 2px 18px;
        }
        
        #call-tree-popover[visible="true"] #call-tree{
            color: #0A59F7;
        }
        #tree-constraints-popover[visible="true"] #tree-constraints{
            color: #0A59F7;
        }
        #data-mining-popover[visible="true"] #data-mining{
            color: #0A59F7;
        }
        
        .mining-checked[highlight]{
            color: #FFFFFF;
            background: #0C65D1;
        }
        #data-library-popover[visible="true"] #data-library{
            color: #0A59F7;
        }
        .library-checked[highlight]{
            color: #FFFFFF;
            background: #0C65D1;
        }
        #title{
            overflow: hidden;
            white-space: nowrap;
            text-overflow: ellipsis;
            flex: 1;
            text-align: left;
        }
        #mining-row{
            background: var(--dark-background4,#F2F2F2);
            border-radius: 2px;
            height: 135px;
            width: 250px;
            overflow-y: auto;
        }
        #library-row{
            background: var(--dark-background4,#F2F2F2);
            border-radius: 2px;
            height: 135px;
            width: 250px;
            overflow-y: auto;
        }
        .tree-check{
            margin-bottom: 5px;
            display: flex;
            align-content: center;
        }
        .sort{
            display: flex;
            align-items: center;
            cursor: pointer;
        }
        :host(:not([sort])) .sort{
            display: none;
        }
        .popover{
            display: flex;
        }
</style>
    <lit-icon name="statistics" class="spacing" id="icon" size="20"></lit-icon>
    <span class="describe left-text spacing">Input Filter</span>
    <input id="filter-input" class="spacing" placeholder="Detail Filter"/>
    <button id="mark" class="spacing">Mark Snapshot</button>
    <div id="load" style="display: flex">
    
    </div>
        <lit-popover placement="topLeft" class="popover" haveRadio="true" trigger="click" id="call-tree-popover">
             <div slot="content">
                 <div class="tree-check"><lit-check-box class="lit-check-box" not-close></lit-check-box><div>Invert</div></div>
                 <div class="tree-check"><lit-check-box class="lit-check-box" not-close></lit-check-box><div>Hide System so</div></div>
             </div>
             <span class="describe tree max-spacing" id="call-tree">Options</span>
        </lit-popover>
        <lit-popover placement="topLeft" class="popover" haveRadio="true" trigger="click" id="tree-constraints-popover">
             <div slot="content" style="display: flex; align-items: flex-end">
                 <lit-check-box id="constraints-check" not-close></lit-check-box>
                 <input class="constraints-input" disabled value="0" not-close/>
                 <lit-popover placement="topLeft" class="popover" haveRadio="true" not-close>
                     <div slot="content">
                         <div style="font-size: 0.7rem">Constraints：Only enabled with data and while stopped；</div>
                         <div style="font-size: 0.7rem">filters data to thresholds. </div>
                     </div>
                     <input class="constraints-input" disabled value="∞" not-close/>
                  </lit-popover>
             </div>
             <span class="describe tree max-spacing" id="tree-constraints">Sample Count Filter</span>
        </lit-popover>
         <lit-popover placement="topLeft" class="popover" haveRadio="true" trigger="click" id="data-mining-popover">
            <div slot="content">
                 <div id="mining-row">
                     
                 </div>
                 <div style="display: flex;justify-content: space-around; margin-top: 8px">
                     <div class="mining-button reset-button">Reset</div>
                 </div>
            </div>
            <span class="describe tree max-spacing" id="data-mining">Symbol Filter</span>
        </lit-popover>
        <lit-popover placement="topLeft" class="popover" haveRadio="true" trigger="click" id="data-library-popover">
            <div slot="content">
                 <div id="library-row">
                     
                 </div>
                 <div style="display: flex;justify-content: space-around; margin-top: 8px">
                     <div class="library-button reset-button">Reset</div>
                 </div>
            </div>
            <span class="describe tree max-spacing" id="data-library">Library Filter</span>
        </lit-popover>
        <div class="sort">
            <lit-icon name="swap" class="spacing" size="16"></lit-icon>
            <div style="margin-left: 5px" class="describe statistics-name">Statistics by Thread</div>
        </div>
        "
`);
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
});
