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
import { LitTabs } from '../../../dist/base-ui/tabs/lit-tabs.js';

window.ResizeObserver =
  window.ResizeObserver ||
  jest.fn().mockImplementation(() => ({
    disconnect: jest.fn(),
    observe: jest.fn(),
    unobserve: jest.fn(),
  }));

describe('LitSwitch Test', () => {
  let litTabs = new LitTabs();

  litTabs.position = 'position';
  litTabs.mode = 'mode';
  litTabs.activekey = 'activekey';

  litTabs.nav = jest.fn(() => {
    let el = document.createElement('div');
    let htmlDivElement = document.createElement('div');
    htmlDivElement.setAttribute('class', "nav-item[data-key='${key}']");

    el.appendChild(htmlDivElement);
    return el;
  });

  LitTabs.nav = jest.fn(() => {
    return document.createElement('div') as HTMLDivElement;
  });

  LitTabs.nav.querySelectorAll = jest.fn(() => {
    return ['fd'];
  });

  it('litTabsTest1', () => {
    expect(litTabs.activekey).toBe('activekey');
  });

  it('litTabsTest01', () => {
    expect(litTabs.onTabClick).toBeUndefined();
  });

  it('litTabsTest02', () => {
    litTabs.nav = jest.fn(() => true);
    litTabs.nav.querySelector = jest.fn(() => {
      return document.createElement('div') as HTMLDivElement;
    });
    litTabs.nav.querySelectorAll = jest.fn(() => true);
    expect(litTabs.updateDisabled('key', 'value')).toBeUndefined();
  });

  it('litTabsTest03', () => {
    litTabs.nav = jest.fn(() => true);
    litTabs.nav.querySelector = jest.fn(() => {
      return document.createElement('div') as HTMLDivElement;
    });
    litTabs.nav.querySelectorAll = jest.fn(() => true);
    expect(litTabs.updateCloseable('key', 'value')).toBeUndefined();
  });

  it('litTabsTest04', () => {
    litTabs.nav = jest.fn(() => true);
    litTabs.nav.querySelector = jest.fn(() => {
      return document.createElement('div') as HTMLDivElement;
    });
    litTabs.nav.querySelectorAll = jest.fn(() => true);

    expect(litTabs.updateHidden('key', 'true')).toBeUndefined();
  });

  it('litTabsTest13', () => {
    litTabs.nav = jest.fn(() => true);
    litTabs.nav.querySelector = jest.fn(() => {
      return document.createElement('div') as HTMLDivElement;
    });
    litTabs.nav.querySelectorAll = jest.fn(() => true);

    expect(litTabs.updateHidden('key', !'true')).toBeUndefined();
  });

  it('litTabsTest05', () => {
    expect(litTabs.initTabPos()).toBeUndefined();
  });

  it('litTabsTest07', () => {
    expect(litTabs.activeByKey(null || undefined)).toBeUndefined();
  });

  it('litTabsTest06', () => {
    expect(litTabs.activePane('Key')).toBeFalsy();
  });

  it('litTabsTest007', () => {
    expect(litTabs.connectedCallback()).toBeUndefined();
  });
  it('litTabsTest8', () => {
    expect(litTabs.attributeChangedCallback('activekey', 'disabled', 'activekey')).toBeUndefined();
  });

  it('litTabsTest9', () => {
    expect(litTabs.adoptedCallback()).toBeUndefined();
  });

  it('litTabsTest09', () => {
    expect(litTabs.disconnectedCallback()).toBeUndefined();
  });
  it('litTabsTest10', () => {
    expect(litTabs.position).toBe('position');
  });

  it('litTabsTest11', () => {
    expect(litTabs.mode).toBe('mode');
  });

  it('litTabsTest12', () => {
    expect(litTabs.shadowRoot.innerHTML).toMatchInlineSnapshot(`
"
        <style>
        :host{ 
            display: block;
            text-align: unset;
            color: var(--dark-color1,#252525);
            background-color: var(--dark-background,#FFFFFF);
            box-shadow: #00000033 0 0 10px ;
        }
        ::slotted(lit-tabpane){
            box-sizing:border-box;
            width:100%;
            height:100%;
            flex-shrink:0;
            overflow:auto;
        }
        .nav-item{
            display: inline-flex;
            justify-content: center;
            align-items: center;
            /*padding: 6px 0px 6px 12px;*/
            padding-left: 12px;
            font-size: .9rem;
            font-weight: normal;
            cursor: pointer;
            transition: all 0.3s;
            flex-shrink: 0;
        }
        .nav-item lit-icon{
            margin-right: 2px;
            font-size: inherit;
        }

        .nav-item[data-disabled]{
            pointer-events: all;
            cursor: not-allowed;
            color: #bfbfbf;
        }
        .nav-item[data-hidden]{
            pointer-events: all;
            cursor: not-allowed;
            color: #bfbfbf;
            display: none;
        }

        .tab-content{
            display: block;
            background-color: #fff;
            flex:1;
        }
        
        /*
         *   top  top-left top-center top-right
         */
        :host(:not([position])) .nav-root,
        :host([position^='top']) .nav-root{
            display: flex;
            position: relative;
            height: 38px;
            z-index: auto;
            /*justify-content: center;*/
            /*align-items: center;*/
        }
        :host(:not([mode]):not([position])) .tab-line,/*移动的线条*/
        :host([mode='flat'][position^='top']) .tab-line{
            position:absolute;
        }
        
        :host(:not([position])) .tab-nav-container,
        :host([position^='top']) .tab-nav-container{
            display: flex;
            /*position: relative;*/
            /*flex-direction: column;*/
            /*overflow-y: hidden;*/
            /*overflow-x: auto;*/
            /*overflow: -moz-scrollbars-none; */
            /*-ms-overflow-style: none;*/
            /*transition: all 0.3s;*/
            
            position: absolute;
            overflow: auto;
            height: 850px;
            transform: rotateZ(-90deg) rotateY(180deg);
            transform-origin: left top;
            overflow-x: hidden;
            width: 38px;
            
            cursor: row-resize;
            user-select: none;
        }
        :host([position='top']) .tab-nav,
        :host([position='top-left']) .tab-nav{
            display: flex;
            position: relative;
            justify-content: flex-start;
            cursor: row-resize;
            user-select: none;
            margin-top: 6px;
            margin-left: 5px;
            
            transform: translateY(-38px) rotateZ(90deg) rotateX(180deg) translateY(38px);
            transform-origin: left bottom;
            flex-wrap: nowrap;
            height: 38px;
        }
        :host([position='top-center']) .tab-nav{
            display: flex;
            justify-content: center;
        }
        :host([position='top-right']) .tab-nav{
            display: flex;
            justify-content: flex-end;
        }
        
        :host([position^='top'][mode='card']) .nav-item{
            border-top: 1px solid var(--dark-border1,#f0f0f0);
            border-left: 1px solid var(--dark-border1,#f0f0f0);
            border-right: 1px solid var(--dark-border1,#f0f0f0);
            bottom: 0px;
            margin-right: 2px;
            position: relative;
            height: 100%;
        }
        :host([position^='top']) .tab-nav-bg-line{
            position: absolute;bottom: 0;height: 1px;
            width: 100%
        }
        :host([position^='top'][mode='card']) .nav-item:not([data-selected]){
            border-top-left-radius: 5px;
            border-top-right-radius: 5px;
            background-color: var(--dark-border1,#D8D8D8);
            color: var(--dark-color1,#212121);
        }
        :host([position^='top'][mode='card']) .nav-item[data-selected]{
            background-color: var(--dark-background,#ffffff);
            bottom: 0px;
            color: var(--dark-color1,#212121);
            border-top: 1px solid var(--dark-border1,#bababa);
            border-top-left-radius: 5px;
            border-top-right-radius: 5px;
            border-left: 1px solid var(--dark-border1,#bababa);
            border-right: 1px solid var(--dark-border1,#bababa);
        }
        /*
            bottom bottom-left bottom-center bottom-right
        */
        :host([position^='bottom']) .tab{
            display: flex;
            flex-direction: column-reverse;
        }
        :host([mode='flat'][position^='bottom']) .tab-line{
            position:absolute;
            top: -3px;
            background-color: #42b983;
            height: 2px;
            transform: translateY(-100%);
            transition: all 0.3s;
        }
        :host([position^='bottom']) .tab-nav-container{
            display: flex;
            position: relative;
            flex-direction: column;
            overflow-x: auto;
            overflow-y: visible;
            overflow: -moz-scrollbars-none; 
            -ms-overflow-style: none;
            transition: all 0.3s;
            flex: 1;
            border-top: #f0f0f0 1px solid;
        }
        :host([position^='bottom']) .nav-root{
            display: flex;
            justify-content: center;
            align-items: center;
        }
        :host([position='bottom']) .tab-nav,
        :host([position='bottom-left']) .tab-nav{
            display: flex;
            position: relative;
            justify-content: flex-start;
        }
        :host([position='bottom-center']) .tab-nav{
            display: flex;
            justify-content: center;
        }
        :host([position='bottom-right']) .tab-nav{
            display: flex;
            justify-content: flex-end;
        }
        :host([position^='bottom'][mode='card']) .nav-item{
            border-top: 1px solid #ffffff;
            border-left: 1px solid #f0f0f0;
            border-right: 1px solid #f0f0f0;
            border-bottom: 1px solid #f0f0f0;
            top: -1px;
            margin-right: 2px;
            position: relative;
        }
        :host([position^='bottom']) .tab-nav-bg-line{
            position: absolute;top: 0;height: 1px;background-color: #f0f0f0;width: 100%
        }
        :host([position^='bottom'][mode='card']) .nav-item:not([data-selected]){
            background-color: #f5f5f5;
            border-top: 1px solid #f0f0f0;
        }
        :host([position^='bottom'][mode='card']) .nav-item[data-selected]{
            background-color: #ffffff;
            border-top: 1px solid #fff;
            top: -1px;
        }
        /*
        left left-top left-center left-bottom
        */
        :host([position^='left']) .tab{
            display: flex;
            flex-direction: row;
        }
        :host([mode='flat'][position^='left']) .tab-line{
            position:absolute;
            right: 1px;
            background-color: #42b983;
            width: 3px;
            transform: translateX(100%);
            transition: all 0.3s;
        }
        :host([position^='left']) .tab-nav-container{
            display: flex;
            position: relative;
            flex-direction: row;
            overflow-x: auto;
            overflow-y: visible;
            overflow: -moz-scrollbars-none; 
            -ms-overflow-style: none;
            transition: all 0.3s;
            flex: 1;
            border-right: #f0f0f0 1px solid;
        }
        :host([position^='left']) .nav-root{
            display: flex;
            flex-direction: column;
            justify-content: center;
            align-items: center;
        }
        :host([position='left']) .tab-nav,
        :host([position='left-top']) .tab-nav{
            display: flex;
            position: relative;
            flex-direction: column;
            justify-content: flex-start;
        }
        :host([position='left-center']) .tab-nav{
            display: flex;
            position: relative;
            flex-direction: column;
            justify-content: center;
        }
        :host([position='left-bottom']) .tab-nav{
            display: flex;
            position: relative;
            flex-direction: column;
            justify-content: flex-end;
        }
        :host([position^='left'][mode='card']) .nav-item{
            border-top: 1px solid #f0f0f0;
            border-left: 1px solid #f0f0f0;
            border-right: 1px solid #ffffff;
            border-bottom: 1px solid #f0f0f0;
            right: -1px;
            margin-bottom: 2px;
            position: relative;
        }
        :host([position^='left']) .tab-nav-bg-line{
            position: absolute;right: 0;width: 1px;background-color: #f0f0f0;width: 100%
        }
        :host([position^='left'][mode='card']) .nav-item:not([data-selected]){
            background-color: #f5f5f5;
            border-right: 1px solid #f0f0f0;
        }
        :host([position^='left'][mode='card']) .nav-item[data-selected]{
            background-color: #ffffff;
            border-bottom: 1px solid #fff;
            right: -1px;
        }
        /*
        right right-top right-center right-bottom
        */
        :host([position^='right']) .tab{
            display: flex;
            flex-direction: row-reverse;
        }
        :host([mode='flat'][position^='right']) .tab-line{
            position:absolute;
            left: 1px;
            background-color: #42b983;
            width: 3px;
            transform: translateX(-100%);
            transition: all 0.3s;
        }
        :host([position^='right']) .tab-nav-container{
            display: flex;
            position: relative;
            flex-direction: row-reverse;
            overflow-x: auto;
            overflow-y: visible;
            overflow: -moz-scrollbars-none; 
            -ms-overflow-style: none;
            transition: all 0.3s;
            flex: 1;
            border-left: #f0f0f0 1px solid;
        }
        :host([position^='right']) .nav-root{
            display: flex;
            flex-direction: column;
            justify-content: center;
            align-items: center;
        }
        :host([position='right']) .tab-nav,
        :host([position='right-top']) .tab-nav{
            display: flex;
            position: relative;
            flex-direction: column;
            justify-content: flex-start;
        }
        :host([position='right-center']) .tab-nav{
            display: flex;
            position: relative;
            flex-direction: column;
            justify-content: center;
        }
        :host([position='right-bottom']) .tab-nav{
            display: flex;
            position: relative;
            flex-direction: column;
            justify-content: flex-end;
        }
        :host([position^='right'][mode='card']) .nav-item{
            border-top: 1px solid #f0f0f0;
            border-left: 1px solid #ffffff;
            border-right: 1px solid #f0f0f0;
            border-bottom: 1px solid #f0f0f0;
            left: -1px;
            margin-top: 2px;
            position: relative;
        }
        :host([position^='right']) .tab-nav-bg-line{
            position: absolute;left: 0;width: 1px;background-color: #f0f0f0;width: 100%
        }
        :host([position^='right'][mode='card']) .nav-item:not([data-selected]){
            background-color: #f5f5f5;
            border-left: 1px solid #f0f0f0;
        }
        :host([position^='right'][mode='card']) .nav-item[data-selected]{
            background-color: #ffffff;
            left: -1px;
        }
        
        
        .tab-nav-container::-webkit-scrollbar {
            display: none;
        }
        
        .close-icon:hover{
            color: #000;
        }
        .nav-item[data-closeable] .close-icon{
            display: block;
            padding: 2px 5px;
            color: var(--dark-icon,#606060)
        }
        .nav-item[data-closeable] .no-close-icon{
            display: none;
        }
        .nav-item:not([data-closeable]) .no-close-icon{
            display: block;
        }
        .nav-item:not([data-closeable]) .close-icon{
            display: none;
        }
        
        </style>
        <style id="filter"></style>
        <div class="tab">
            <div class="nav-root" style="background-color: var(--dark-background4,#f2f2f2);">
                <slot name="left" style="flex:1"></slot>
                <div class="tab-nav-container">
                    <div class="tab-nav-bg-line"></div>
                    <div class="tab-nav" id="nav"></div>
                    <div class="tab-line" id="tab-line"></div>
                </div>
                <div id="tab-filling" style="flex: 1"></div>
                <slot name="right" style="flex:1"></slot>
            </div>
            <div class="tab-content">
                <slot id="slot">NEED CONTENT</slot>
            </div>
        </div>
        "
`);
  });

  it('litTabsTest14', () => {
    litTabs.nav = jest.fn(() => true);
    litTabs.nav.querySelector = jest.fn(() => {
      return document.createElement('div') as HTMLDivElement;
    });
    litTabs.nav.querySelectorAll = jest.fn(() => true);
    expect(litTabs.updateDisabled('key', undefined)).toBeUndefined();
  });

  it('litTabsTest15', () => {
    litTabs.nav = jest.fn(() => true);
    litTabs.nav.querySelector = jest.fn(() => {
      return document.createElement('div') as HTMLDivElement;
    });
    litTabs.nav.querySelectorAll = jest.fn(() => true);
    expect(litTabs.updateCloseable('key', undefined)).toBeUndefined();
  });

  it('litTabsTest19', () => {
    expect(litTabs.activePane(null)).toBe(false);
  });
});
