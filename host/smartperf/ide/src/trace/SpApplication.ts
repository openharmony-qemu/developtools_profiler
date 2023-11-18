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

import { BaseElement, element } from '../base-ui/BaseElement.js';
import '../base-ui/menu/LitMainMenu.js';
import '../base-ui/icon/LitIcon.js';
import { SpMetrics } from './component/SpMetrics.js';
import { SpHelp } from './component/SpHelp.js';
import './component/SpHelp.js';
import { SpQuerySQL } from './component/SpQuerySQL.js';
import './component/SpQuerySQL.js';
import { SpSystemTrace } from './component/SpSystemTrace.js';
import { LitMainMenu, MenuItem } from '../base-ui/menu/LitMainMenu.js';
import { SpInfoAndStats } from './component/SpInfoAndStas.js';
import '../base-ui/progress-bar/LitProgressBar.js';
import { LitProgressBar } from '../base-ui/progress-bar/LitProgressBar.js';
import { SpRecordTrace } from './component/SpRecordTrace.js';
import { SpWelcomePage } from './component/SpWelcomePage.js';
import { LitSearch } from './component/trace/search/Search.js';
import { DbPool, queryExistFtrace, queryTraceType, threadPool } from './database/SqlLite.js';
import './component/trace/search/Search.js';
import './component/SpWelcomePage.js';
import './component/SpSystemTrace.js';
import './component/SpRecordTrace.js';
import './component/SpMetrics.js';
import './component/SpInfoAndStas.js';
import './component/trace/base/TraceRow.js';
import './component/schedulingAnalysis/SpSchedulingAnalysis.js';
import { info, log } from '../log/Log.js';
import { LitMainMenuGroup } from '../base-ui/menu/LitMainMenuGroup.js';
import { LitMainMenuItem } from '../base-ui/menu/LitMainMenuItem.js';
import { LitIcon } from '../base-ui/icon/LitIcon.js';
import { Cmd } from '../command/Cmd.js';
import { TraceRow } from './component/trace/base/TraceRow.js';
import { SpSchedulingAnalysis } from './component/schedulingAnalysis/SpSchedulingAnalysis.js';
import './component/trace/base/TraceRowConfig.js';
import { TraceRowConfig } from './component/trace/base/TraceRowConfig.js';
import { ColorUtils } from './component/trace/base/ColorUtils.js';
import { SpStatisticsHttpUtil } from '../statistics/util/SpStatisticsHttpUtil.js';
import { FlagsConfig, SpFlags } from './component/SpFlags.js';
import './component/SpFlags.js';
import './component/trace/base/CustomThemeColor.js';
import { CustomThemeColor, Theme } from './component/trace/base/CustomThemeColor.js';
import { convertPool } from './database/Convert.js';
import { LongTraceDBUtils } from './database/LongTraceDBUtils.js';

@element('sp-application')
export class SpApplication extends BaseElement {
  private static loadingProgress: number = 0;
  private static progressStep: number = 2;
  longTraceHeadMessageList: Array<{
    pageNum: number;
    data: ArrayBuffer;
  }> = [];

  longTraceDataList: Array<{
    fileType: string;
    index: number;
    pageNum: number;
    startOffsetSize: number;
    endOffsetSize: number;
  }> = [];

  longTraceTypeMessageMap:
    | Map<
        number,
        Array<{
          fileType: string;
          startIndex: number;
          endIndex: number;
          size: number;
        }>
      >
    | undefined
    | null;
  static skinChange: Function | null | undefined = null;
  static skinChange2: Function | null | undefined = null;
  skinChangeArray: Array<Function> = [];
  private icon: HTMLDivElement | undefined | null;
  private rootEL: HTMLDivElement | undefined | null;
  private spHelp: SpHelp | undefined | null;
  private keyCodeMap = {
    61: true,
    107: true,
    109: true,
    173: true,
    187: true,
    189: true,
  };
  private traceFileName: string | undefined;
  colorTransiton: any;
  static isLongTrace: boolean = false;
  fileTypeList: string[] = ['ebpf', 'arkts', 'hiperf'];
  private pageTimStamp: number = 0;
  private currentPageNum: number = 1;

  static get observedAttributes() {
    return ['server', 'sqlite', 'wasm', 'dark', 'vs', 'query-sql', 'subsection'];
  }

  get dark() {
    return this.hasAttribute('dark');
  }

  set dark(value) {
    if (value) {
      this.rootEL!.classList.add('dark');
      this.setAttribute('dark', '');
    } else {
      this.rootEL!.classList.remove('dark');
      this.removeAttribute('dark');
    }
    if (this.skinChangeArray.length > 0) {
      this.skinChangeArray.forEach((item) => item(value));
    }
    if (SpApplication.skinChange) {
      SpApplication.skinChange(value);
    }
    if (SpApplication.skinChange2) {
      SpApplication.skinChange2(value);
    }

    if (this.spHelp) {
      this.spHelp.dark = value;
    }
  }

  get vs(): boolean {
    return this.hasAttribute('vs');
  }

  set vs(isVs: boolean) {
    if (isVs) {
      this.setAttribute('vs', '');
    }
  }

  get sqlite(): boolean {
    return this.hasAttribute('sqlite');
  }

  get wasm(): boolean {
    return this.hasAttribute('wasm');
  }

  get server(): boolean {
    return this.hasAttribute('server');
  }

  set server(s: boolean) {
    if (s) {
      this.setAttribute('server', '');
    } else {
      this.removeAttribute('server');
    }
  }

  get querySql(): boolean {
    return this.hasAttribute('query-sql');
  }

  set querySql(isShowMetric) {
    if (isShowMetric) {
      this.setAttribute('query-sql', '');
    } else {
      this.removeAttribute('query-sql');
    }
  }

  set search(search: boolean) {
    if (search) {
      this.setAttribute('search', '');
    } else {
      this.removeAttribute('search');
    }
  }

  get search(): boolean {
    return this.hasAttribute('search');
  }

  addSkinListener(handler: Function) {
    this.skinChangeArray.push(handler);
  }

  removeSkinListener(handler: Function) {
    this.skinChangeArray.splice(this.skinChangeArray.indexOf(handler), 1);
  }

  initHtml(): string {
    return `
        <style>
        :host{

        }
        .dark{
        --dark-background: #272C34;
        --dark-background1: #424851;
        --dark-background2: #262f3c;
        --dark-background3: #292D33;
        --dark-background4: #323841;
        --dark-background5: #333840;
        --dark-background6: rgba(82,145,255,0.2);
        --dark-background7: #494d52;
        --dark-background8: #5291FF;
        --dark-color: rgba(255,255,255,0.6);
        --dark-color1: rgba(255,255,255,0.86);
        --dark-color2: rgba(255,255,255,0.9);
        --dark-border: #474F59;
        --dark-color3:#4694C2;
        --dark-color4:#5AADA0;
        --dark-border1: #454E5A;
        --bark-expansion:#0076FF;
        --bark-prompt:#9e9e9e;
        --dark-icon:#adafb3;
        --dark-img: url('img/dark_pic.png');
            background: #272C34;
            color: #FFFFFF;
        }
        .root{
            display: grid;
            grid-template-rows: min-content 1fr;
            grid-template-columns: min-content 1fr;
            grid-template-areas: 'm s'
                                 'm b';
            height: 100vh;
            width: 100vw;
        }
        .filedrag::after {
             content: 'Drop the trace file to open it';
             position: fixed;
             z-index: 2001;
             top: 0;
             left: 0;
             right: 0;
             bottom: 0;
             border: 5px dashed var(--dark-color1,#404854);
             text-align: center;
             font-size: 3rem;
             line-height: 100vh;
             background: rgba(255, 255, 255, 0.5);
        }
        .menu{
            grid-area: m;
            /*transition: all 0.2s;*/
            box-shadow: 4px 0px 20px rgba(0,0,0,0.05);
            z-index: 2000;
        }
        .search-container{
            z-index: 10;
            position: relative;
            cursor: default;
        }
        .progress{
            bottom: 0;
            position: absolute;
            height: 1px;
            left: 0;
            right: 0;
        }
        :host(:not([search])) .search-container  {
           display: none;
        }
        :host(:not([search])) .search-container .search  {
            background-color: var(--dark-background5,#F6F6F6);
        }
        .search{
            grid-area: s;
            background-color: var(--dark-background,#FFFFFF);
            height: 48px;
            display: flex;
            justify-content: center;
            align-items: center;

        }
        .search .search-bg{
            background-color: var(--dark-background5,#fff);
            border-radius: 40px;
            padding: 3px 20px;
            display: flex;
            justify-content: center;
            align-items: center;
            border: 1px solid var(--dark-border,#c5c5c5);
        }
        lit-search input{
            outline: none;
            border: 0px;
            background-color: transparent;
            font-size: inherit;
            color: var(--dark-color,#666666);
            width: 30vw;
            height: auto;
            vertical-align:middle;
            line-height:inherit;
            height:inherit;
            padding: 6px 6px 6px 6px};
            max-height: inherit;
            box-sizing: border-box;

        }
        ::placeholder { /* CSS 3 標準 */
          color: #b5b7ba;
          font-size: 1em;
        }
        lit-search input::placeholder {
          color: #b5b7ba;
          font-size: 1em;
        }
        .content{
            grid-area: b;
            background-color: #ffffff;
            height: 100%;
            overflow: auto;
            position:relative;
        }
        .sheet{

        }
        .sidebar-button{
            position: absolute;
            top: 0;
            left: 0;
            background-color: var(--dark-background1,#FFFFFF);
            height: 100%;
            border-radius: 0 5px 5px 0;
            width: 48px;
            display: flex;
            align-content: center;
            justify-content: center;
            cursor: pointer;
        }
        :host{
            font-size: inherit;
            display: inline-block;
            transition: .3s;
         }
         :host([spin]){
            animation: rotate 1.75s linear infinite;
         }
         @keyframes rotate {
            to{
                transform: rotate(360deg);
            }
         }
         .icon{
            display: block;
            width: 1em;
            height: 1em;
            margin: auto;
            fill: currentColor;
            overflow: hidden;
            font-size: 20px;
            color: var(--dark-color1,#47A7E0);
         }
         .chart-filter {
            visibility: hidden;
            z-index: -1;
        }
        :host([chart_filter]) .chart-filter {
            display: grid;
            grid-template-rows: min-content min-content min-content max-content auto;
            overflow-y: clip;
            height: 99%;
            visibility: visible;
            position: absolute;
            width: 40%;
            right: 0;
            z-index: 1001;
            top: 0;
        }
        :host([custom-color]) .custom-color {
            display: grid;
            grid-template-rows: min-content min-content min-content max-content auto;
            overflow-y: auto;
            height: 100%;
            visibility: visible;
            position: absolute;
            width: 50%;
            right: 0;
            z-index: 1002;
            top: 0;
        }
        .filter-config {
            opacity: 1;
            visibility: hidden;
        }
        .filter-config:hover {
            opacity: 0.7;
        }
        .page-button[prohibit] {
          cursor: none;
        }
        .page-button {
            background: #D8D8D8;
            border-radius: 12px;
            width: 24px;
            height: 24px;
            margin-right: 12px;
            display: flex;
            justify-content: center;
            align-items: center;
        }
        #preview-button:hover {
          cursor: pointer;
          background: #0A59F7;
          color: #FFFFFF;
          opacity: 1;
        }
        #next-button:hover {
          cursor: pointer;
          background: #0A59F7;
          color: #FFFFFF;
          opacity: 1;
        }
        .pagination:hover {
          cursor: pointer;
          background: #0A59F7;
          color: #FFFFFF;
          opacity: 1;
        }
        .confirm-button:hover {
          cursor: pointer;
          background: #0A59F7;
          color: #FFFFFF;
          opacity: 1;
        }
        .pagination {
            background: #D8D8D8;
            color: #000000;
            border-radius: 12px;
            width: 24px;
            height: 24px;
            margin-right: 12px;
            display: flex;
            justify-content: center;
            align-items: center;
            font-family: Helvetica;
            font-size: 12px;
            text-align: center;
            line-height: 20px;
            font-weight: 400;
            opacity: 0.6;
        }
        .pagination[selected] {
            background: #0A59F7;
            color: #FFFFFF;
            opacity: 1;
        }
        .page-jump-font {
            opacity: 0.6;
            font-family: Helvetica;
            font-size: 12px;
            color: #000000;
            text-align: center;
            line-height: 20px;
            font-weight: 400;
        }
        .page-input {
            background: #D8D8D8;
            border-radius: 10px;
            width: 40px;
            height: 24px;
            justify-content: center;
            align-items: center;
            text-align: center;
            margin-right: 8px;
            border: none;
        }
        .confirm-button {
            font-family: Helvetica;
            font-size: 12px;
            color: #0A59F7;
            text-align: center;
            font-weight: 400;
            border: 1px solid #0A59F7;
            border-radius: 10px;
            width: 64px;
            height: 24px;
            line-height: 24px;
        }
        .long_trace_page {
            justify-content: flex-end;
            width: -webkit-fill-available;
            margin-right: 80px;
            align-items: center;
            display: none;
        }
        .page-number-list {
            display: flex;
        }
        </style>
        <div class="root">
            <lit-main-menu id="main-menu" class="menu" data=''></lit-main-menu>
            <div class="search-container">
                <div class="search" style="position: relative;">
                    <div class="sidebar-button" style="width: 0">
                        <svg class="icon" id="icon" aria-hidden="true" viewBox="0 0 1024 1024">
                             <use id="use" xlink:href="./base-ui/icon.svg#icon-menu"></use>
                        </svg>
                    </div>
                    <lit-search id="lit-search"></lit-search>
                    <lit-search id="lit-record-search"></lit-search>
                    <div class="long_trace_page" style="display: none;">
                      <div class="page-button" id="preview-button">
                         <img title="preview" src="img/preview.png"/>
                         </div>
                      <div class="page-number-list"></div>
                      <div class="page-button" id="next-button" style="margin-right: 8px;">
                         <img title="next" src="img/next.png"/>
                      </div>
                      <div class="page-jump-font" style="margin-right: 8px;">To</div>
                      <input class="page-input" />
                      <div class="confirm-button">Confirm</div>
                    </div>
                </div>
                <img class="cut-trace-file" title="Cut Trace File" src="img/menu-cut.svg" style="display: block;text-align: right;position: absolute;right: 3.2em;cursor: pointer;top: 20px">
                <img class="filter-config" title="Display Template" src="img/config_filter.png" style="display: block;text-align: right;position: absolute;right: 1.2em;cursor: pointer;top: 20px">
                <lit-progress-bar class="progress"></lit-progress-bar>
            </div>
            <div id="app-content" class="content">
                <sp-welcome style="visibility:visible;top:0px;left:0px;position:absolute;z-index: 100" id="sp-welcome">
                </sp-welcome>
                <sp-system-trace style="visibility:visible;" id="sp-system-trace">
                </sp-system-trace>
                <sp-record-trace style="width:100%;height:100%;overflow:auto;visibility:hidden;top:0px;left:0px;right:0;bottom:0px;position:absolute;z-index: 102" id="sp-record-trace">
                </sp-record-trace>
                <sp-record-trace record_template='' style="width:100%;height:100%;overflow:auto;visibility:hidden;top:0px;left:0px;right:0;bottom:0px;position:absolute;z-index: 102" id="sp-record-template">
                </sp-record-trace>
                <sp-scheduling-analysis style="width:100%;height:100%;overflow:auto;visibility:hidden;top:0;left:0;right:0;bottom:0;position:absolute;" id="sp-scheduling-analysis"></sp-scheduling-analysis>
                <sp-metrics style="width:100%;height:100%;overflow:auto;visibility:hidden;top:0;left:0;right:0;bottom:0;position:absolute;z-index: 97" id="sp-metrics">
                </sp-metrics>
                <sp-query-sql style="width:100%;height:100%;overflow:auto;visibility:hidden;top:0;left:0;right:0;bottom:0;position:absolute;z-index: 98" id="sp-query-sql">
                </sp-query-sql>
                <sp-info-and-stats style="width:100%;height:100%;overflow:auto;visibility:hidden;top:0;left:0;right:0;bottom:0;position:absolute;z-index: 99" id="sp-info-and-stats">
                </sp-info-and-stats>
                <sp-convert-trace style="width:100%;height:100%;overflow:auto;visibility:hidden;top:0;left:0;right:0;bottom:0;position:absolute;z-index: 99" id="sp-convert-trace">
                </sp-convert-trace>
                <sp-help style="width:100%;height:100%;overflow:auto;visibility:hidden;top:0px;left:0px;right:0;bottom:0px;position:absolute;z-index: 103" id="sp-help">
                </sp-help>
                <sp-flags style="width:100%;height:100%;overflow:auto;visibility:hidden;top:0px;left:0px;right:0;bottom:0px;position:absolute;z-index: 104" id="sp-flags">
                </sp-flags>
                <trace-row-config class="chart-filter" style="height:100%;top:0px;right:0;bottom:0px;position:absolute;z-index: 105"></trace-row-config>
                <custom-theme-color class="custom-color" style="height:100%;top:0px;right:0;bottom:0px;position:absolute;z-index: 106"></custom-theme-color>
            </div>
        </div>
        `;
  }

  initElements() {
    SpStatisticsHttpUtil.initStatisticsServerConfig();
    SpStatisticsHttpUtil.addUserVisitAction('visit');
    LongTraceDBUtils.getInstance().createDBAndTable().then();
    let that = this;
    this.querySql = true;
    this.rootEL = this.shadowRoot!.querySelector<HTMLDivElement>('.root');
    let spWelcomePage = this.shadowRoot!.querySelector('#sp-welcome') as SpWelcomePage;
    let spMetrics = this.shadowRoot!.querySelector<SpMetrics>('#sp-metrics') as SpMetrics; // new SpMetrics();
    let spQuerySQL = this.shadowRoot!.querySelector<SpQuerySQL>('#sp-query-sql') as SpQuerySQL; // new SpQuerySQL();
    let spInfoAndStats = this.shadowRoot!.querySelector<SpInfoAndStats>('#sp-info-and-stats') as SpInfoAndStats; // new SpInfoAndStats();
    let spSystemTrace = this.shadowRoot!.querySelector<SpSystemTrace>('#sp-system-trace');
    this.spHelp = this.shadowRoot!.querySelector<SpHelp>('#sp-help');
    let spFlags = this.shadowRoot!.querySelector<SpFlags>('#sp-flags') as SpFlags;
    let spRecordTrace = this.shadowRoot!.querySelector<SpRecordTrace>('#sp-record-trace');
    let spRecordTemplate = this.shadowRoot!.querySelector<SpRecordTrace>('#sp-record-template');
    let spSchedulingAnalysis = this.shadowRoot!.querySelector<SpSchedulingAnalysis>(
      '#sp-scheduling-analysis'
    ) as SpSchedulingAnalysis;
    let appContent = this.shadowRoot?.querySelector('#app-content') as HTMLDivElement;
    let mainMenu = this.shadowRoot?.querySelector('#main-menu') as LitMainMenu;
    let menu = mainMenu.shadowRoot?.querySelector('.menu-button') as HTMLDivElement;
    let progressEL = this.shadowRoot?.querySelector('.progress') as LitProgressBar;
    let litSearch = this.shadowRoot?.querySelector('#lit-search') as LitSearch;
    let litRecordSearch = this.shadowRoot?.querySelector('#lit-record-search') as LitSearch;
    let search = this.shadowRoot?.querySelector('.search-container') as HTMLElement;
    let sidebarButton: HTMLDivElement | undefined | null = this.shadowRoot?.querySelector('.sidebar-button');
    let chartFilter = this.shadowRoot?.querySelector('.chart-filter') as TraceRowConfig;
    let cutTraceFile = this.shadowRoot?.querySelector('.cut-trace-file') as HTMLImageElement;
    let longTracePage = that.shadowRoot!.querySelector('.long_trace_page') as HTMLDivElement;
    cutTraceFile.addEventListener('click', () => {
      this.croppingFile(progressEL, litSearch);
    });
    let customColor = this.shadowRoot?.querySelector('.custom-color') as CustomThemeColor;
    mainMenu!.setAttribute('main_menu', '1');
    chartFilter!.setAttribute('mode', '');
    chartFilter!.setAttribute('hidden', '');
    customColor!.setAttribute('mode', '');
    customColor!.setAttribute('hidden', '');
    let childNodes = [
      spSystemTrace,
      spRecordTrace,
      spWelcomePage,
      spMetrics,
      spQuerySQL,
      spSchedulingAnalysis,
      spInfoAndStats,
      this.spHelp,
      spRecordTemplate,
      spFlags,
    ];
    document.addEventListener('visibilitychange', function () {
      if (document.visibilityState === 'visible') {
        if (window.localStorage.getItem('Theme') == 'dark') {
          that.changeTheme(Theme.DARK);
        } else {
          that.changeTheme(Theme.LIGHT);
        }
      }
    });
    window.subscribe(window.SmartEvent.UI.MenuTrace, () => showContent(spSystemTrace!));
    window.subscribe(window.SmartEvent.UI.Error, (err) => {
      litSearch.setPercent(err, -1);
      progressEL.loading = false;
      that.freshMenuDisable(false);
    });
    window.subscribe(window.SmartEvent.UI.Loading, (loading) => {
      litSearch.setPercent(loading ? 'Import So File' : '', loading ? -1 : 101);
      window.publish(window.SmartEvent.UI.MouseEventEnable, {
        mouseEnable: !loading,
      });
      progressEL.loading = loading;
    });
    litSearch.addEventListener('focus', () => {
      window.publish(window.SmartEvent.UI.KeyboardEnable, {
        enable: false,
      });
    });
    litSearch.addEventListener('blur', () => {
      window.publish(window.SmartEvent.UI.KeyboardEnable, {
        enable: true,
      });
    });
    litSearch.addEventListener('previous-data', (ev: any) => {
      litSearch.index = spSystemTrace!.showStruct(true, litSearch.index, litSearch.list);
      litSearch.blur();
    });
    litSearch.addEventListener('next-data', (ev: any) => {
      litSearch.index = spSystemTrace!.showStruct(false, litSearch.index, litSearch.list);
      litSearch.blur();
    });
    // 翻页事件
    litSearch.addEventListener('retarget-data', (ev: any) => {
      litSearch.index = spSystemTrace!.showStruct(true, ev.detail.value, litSearch.list, ev.detail.value);
      litSearch.blur();
    });
    litSearch.valueChangeHandler = (value: string) => {
      if (value.length > 0) {
        let list = spSystemTrace!.searchCPU(value);
        spSystemTrace!.searchFunction(list, value).then((mixedResults) => {
          if (litSearch.searchValue != '') {
            litSearch.list = spSystemTrace!.searchSdk(mixedResults, value);
          }
        });
      } else {
        let indexEL = litSearch.shadowRoot!.querySelector<HTMLSpanElement>('#index');
        indexEL!.textContent = '0';
        litSearch.list = [];
        spSystemTrace?.visibleRows.forEach((it) => {
          it.highlight = false;
          it.draw();
        });
        spSystemTrace?.timerShaftEL?.removeTriangle('inverted');
      }
    };
    spSystemTrace?.addEventListener('previous-data', (ev: any) => {
      litSearch.index = spSystemTrace!.showStruct(true, litSearch.index, litSearch.list);
    });
    spSystemTrace?.addEventListener('next-data', (ev: any) => {
      litSearch.index = spSystemTrace!.showStruct(false, litSearch.index, litSearch.list);
    });

    let filterConfig = this.shadowRoot?.querySelector('.filter-config') as LitIcon;
    let configClose = this.shadowRoot
      ?.querySelector<HTMLElement>('.chart-filter')!
      .shadowRoot?.querySelector<LitIcon>('.config-close');
    filterConfig.addEventListener('click', (ev) => {
      if (this!.hasAttribute('chart_filter')) {
        this!.removeAttribute('chart_filter');
        chartFilter!.setAttribute('hidden', '');
      } else {
        this!.removeAttribute('custom-color');
        customColor!.setAttribute('hidden', '');
        customColor.cancelOperate();
        this!.setAttribute('chart_filter', '');
        chartFilter!.removeAttribute('hidden');
      }
    });
    configClose!.addEventListener('click', (ev) => {
      if (this.hasAttribute('chart_filter')) {
        this!.removeAttribute('chart_filter');
      }
    });

    let customColorShow = this.shadowRoot
      ?.querySelector('lit-main-menu')!
      .shadowRoot!.querySelector('.customColor') as HTMLDivElement;
    customColorShow.addEventListener('click', (ev) => {
      if (this!.hasAttribute('custom-color')) {
        this!.removeAttribute('custom-color');
        customColor!.setAttribute('hidden', '');
        customColor.cancelOperate();
      } else {
        this!.removeAttribute('chart_filter');
        chartFilter!.setAttribute('hidden', '');
        this!.setAttribute('custom-color', '');
        customColor!.removeAttribute('hidden');
      }
    });

    //打开侧边栏
    sidebarButton!.onclick = (e) => {
      let menu: HTMLDivElement | undefined | null = this.shadowRoot?.querySelector('#main-menu');
      let menuButton: HTMLElement | undefined | null = this.shadowRoot?.querySelector('.sidebar-button');
      if (menu) {
        menu.style.width = `248px`;
        // @ts-ignore
        menu.style.zIndex = 2000;
        menu.style.display = `flex`;
      }
      if (menuButton) {
        menuButton.style.width = `0px`;
      }
    };
    let icon: HTMLDivElement | undefined | null = this.shadowRoot
      ?.querySelector('#main-menu')
      ?.shadowRoot?.querySelector('div.header > div');
    icon!.style.pointerEvents = 'none';
    icon!.onclick = (e) => {
      let menu: HTMLElement | undefined | null = this.shadowRoot?.querySelector('#main-menu');
      let menuButton: HTMLElement | undefined | null = this.shadowRoot?.querySelector('.sidebar-button');
      if (menu) {
        menu.style.width = `0px`;
        menu.style.display = `flex`;
        // @ts-ignore
        menu.style.zIndex = 0;
      }
      if (menuButton) {
        menuButton.style.width = `48px`;
      }
    };

    function showContent(showNode: HTMLElement) {
      if (showNode === spSystemTrace) {
        menu!.style.pointerEvents = 'auto';
        sidebarButton!.style.pointerEvents = 'auto';
        that.search = true;
        litRecordSearch.style.display = 'none';
        litSearch.style.display = 'block';
        window.publish(window.SmartEvent.UI.KeyboardEnable, {
          enable: true,
        });
        filterConfig.style.visibility = 'visible';
      } else {
        that.removeAttribute('custom-color');
        customColor.cancelOperate();
        menu!.style.pointerEvents = 'none';
        sidebarButton!.style.pointerEvents = 'none';
        that.search = litSearch.isLoading;
        if (!that.search) {
          litSearch.style.display = 'none';
          litRecordSearch.style.display = 'block';
        }
        window.publish(window.SmartEvent.UI.KeyboardEnable, {
          enable: false,
        });
        filterConfig.style.visibility = 'hidden';
      }
      log('show pages' + showNode.id);
      childNodes.forEach((node) => {
        if (that.hasAttribute('chart_filter')) {
          that!.removeAttribute('chart_filter');
        }
        if (that!.hasAttribute('custom-color')) {
          that!.removeAttribute('custom-color');
          customColor!.setAttribute('hidden', '');
          customColor.cancelOperate();
        } else {
          that!.setAttribute('custom-color', '');
          customColor!.removeAttribute('hidden');
        }
        if (node === showNode) {
          showNode.style.visibility = 'visible';
        } else {
          node!.style.visibility = 'hidden';
        }
      });
    }

    function postLog(filename: string, fileSize: string) {
      log('postLog filename is: ' + filename + ' fileSize: ' + fileSize);
      fetch(`https://${window.location.host.split(':')[0]}:9000/logger`, {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json',
        },
        body: JSON.stringify({
          fileName: filename,
          fileSize: fileSize,
        }),
      })
        .then((response) => response.json())
        .then((data) => {})
        .catch((error) => {});
    }

    function getTraceOptionMenus(
      showFileName: string,
      fileSize: string,
      fileName: string,
      isServer: boolean,
      dbName?: string
    ) {
      let menus = [
        {
          title: `${showFileName} (${fileSize}M)`,
          icon: 'file-fill',
          clickHandler: function () {
            that.search = true;
            showContent(spSystemTrace!);
          },
        },
        {
          title: 'Scheduling Analysis',
          icon: 'piechart-circle-fil',
          clickHandler: function () {
            SpStatisticsHttpUtil.addOrdinaryVisitAction({
              event: 'Scheduling Analysis',
              action: 'scheduling_analysis',
            });
            showContent(spSchedulingAnalysis!);
            spSchedulingAnalysis.init();
          },
        },
        {
          title: 'Download File',
          icon: 'download',
          clickHandler: function () {
            if (that.vs) {
              that.vsDownload(mainMenu, fileName, isServer, dbName);
            } else {
              that.download(mainMenu, fileName, isServer, dbName);
              SpStatisticsHttpUtil.addOrdinaryVisitAction({
                event: 'download',
                action: 'download',
              });
            }
          },
        },
        {
          title: 'Download Database',
          icon: 'download',
          clickHandler: function () {
            if (that.vs) {
              that.vsDownloadDB(mainMenu, fileName);
            } else {
              that.downloadDB(mainMenu, fileName);
              SpStatisticsHttpUtil.addOrdinaryVisitAction({
                event: 'download_db',
                action: 'download',
              });
            }
          },
        },
      ];
      if (that.querySql) {
        if (spQuerySQL) {
          spQuerySQL.reset();
          menus.push({
            title: 'Query (SQL)',
            icon: 'filesearch',
            clickHandler: () => {
              showContent(spQuerySQL);
            },
          });
        }

        if (spMetrics) {
          spMetrics.reset();
          menus.push({
            title: 'Metrics',
            icon: 'metric',
            clickHandler: () => {
              showContent(spMetrics);
            },
          });
        }

        if (spInfoAndStats) {
          menus.push({
            title: 'Info and stats',
            icon: 'info',
            clickHandler: () => {
              SpStatisticsHttpUtil.addOrdinaryVisitAction({
                event: 'info',
                action: 'info_stats',
              });
              showContent(spInfoAndStats);
            },
          });
        }
      }
      if ((window as any).cpuCount === 0 || !FlagsConfig.getFlagsConfigEnableStatus('SchedulingAnalysis')) {
        //if cpu count > 1 or SchedulingAnalysis config 'enable'  then show Scheduling-Analysis menu else hide it
        menus.splice(1, 1);
      }
      return menus;
    }

    function restoreDownLoadIcons() {
      let querySelectorAll = mainMenu.shadowRoot?.querySelectorAll<LitMainMenuGroup>('lit-main-menu-group');
      querySelectorAll!.forEach((menuGroup) => {
        let attribute = menuGroup.getAttribute('title');
        if (attribute === 'Convert trace') {
          let querySelectors = menuGroup.querySelectorAll<LitMainMenuItem>('lit-main-menu-item');
          querySelectors.forEach((item) => {
            if (item.getAttribute('title') === 'Convert to .systrace') {
              item!.setAttribute('icon', 'download');
              let querySelector = item!.shadowRoot?.querySelector('.icon') as LitIcon;
              querySelector.removeAttribute('spin');
            }
          });
        }
      });
    }

    function postConvert(fileName: string): void {
      let newFileName = fileName.substring(0, fileName.lastIndexOf('.')) + '.systrace';
      let aElement = document.createElement('a');
      convertPool.submitWithName('getConvertData', (status: boolean, msg: string, results: Blob) => {
        aElement.href = URL.createObjectURL(results);
        aElement.download = newFileName;
        let timeoutId = 0;
        aElement.addEventListener('click', (ev) => {
          clearTimeout(timeoutId);
          timeoutId = window.setTimeout(() => {
            restoreDownLoadIcons();
          }, 2000);
        });
        aElement.click();
        window.URL.revokeObjectURL(aElement.href);
      });
    }

    function pushConvertTrace(fileName: string): Array<any> {
      let menus = [];
      menus.push({
        title: 'Convert to .systrace',
        icon: 'download',
        clickHandler: function () {
          convertPool.init('convert').then((item) => {
            let querySelectorAll = mainMenu.shadowRoot?.querySelectorAll<LitMainMenuGroup>('lit-main-menu-group');
            querySelectorAll!.forEach((menuGroup) => {
              let attribute = menuGroup.getAttribute('title');
              if (attribute === 'Convert trace') {
                let querySelectors = menuGroup.querySelectorAll<LitMainMenuItem>('lit-main-menu-item');
                querySelectors.forEach((item) => {
                  if (item.getAttribute('title') === 'Convert to .systrace') {
                    item!.setAttribute('icon', 'convert-loading');
                    item!.classList.add('pending');
                    item!.style.fontKerning = '';
                    let querySelector = item!.shadowRoot?.querySelector('.icon') as LitIcon;
                    querySelector.setAttribute('spin', '');
                  }
                });
              }
            });
            postConvert(fileName);
          });
        },
      });
      return menus;
    }

    function setProgress(command: string) {
      if (command == 'database ready' && SpApplication.loadingProgress < 50) {
        SpApplication.progressStep = 6;
      }
      if (command == 'process' && SpApplication.loadingProgress < 92) {
        SpApplication.loadingProgress = 92 + Math.round(Math.random() * SpApplication.progressStep);
      } else {
        SpApplication.loadingProgress += Math.round(Math.random() * SpApplication.progressStep + Math.random());
      }
      if (SpApplication.loadingProgress > 99) {
        SpApplication.loadingProgress = 99;
      }
      info('setPercent ：' + command + 'percent :' + SpApplication.loadingProgress);
      litSearch.setPercent(command + '  ', SpApplication.loadingProgress);
    }

    function handleServerMode(
      ev: any,
      showFileName: string,
      fileSize: string,
      fileName: string,
      isClickHandle?: boolean
    ) {
      threadPool.init('server').then(() => {
        info('init server ok');
        litSearch.setPercent('parse trace', 1);
        // Load the trace file and send it to the background parse to return the db file path
        const fd = new FormData();
        if (that.vs && isClickHandle) {
          fd.append('convertType', 'vsUpload');
          fd.append('filePath', ev as any);
        } else {
          fd.append('file', ev as any);
        }
        let uploadPath = `https://${window.location.host.split(':')[0]}:9000/application/upload`;
        if (that.vs) {
          uploadPath = `http://${window.location.host.split(':')[0]}:${window.location.port}/application/upload`;
        }
        info('upload trace');
        let dbName = '';
        fetch(uploadPath, {
          method: 'POST',
          body: fd,
        })
          .then((res) => {
            litSearch.setPercent('load database', 5);
            if (res.ok) {
              info(' server Parse trace file success');
              return res.text();
            } else {
              if (res.status == 404) {
                info(' server Parse trace file failed');
                litSearch.setPercent('This File is not supported!', -1);
                progressEL.loading = false;
                that.freshMenuDisable(false);
                return Promise.reject();
              }
            }
          })
          .then((res) => {
            if (res != undefined) {
              dbName = res;
              info('get trace db');
              let loadPath = `https://${window.location.host.split(':')[0]}:9000`;
              if (that.vs) {
                loadPath = `http://${window.location.host.split(':')[0]}:${window.location.port}`;
              }
              SpApplication.loadingProgress = 0;
              SpApplication.progressStep = 3;
              spSystemTrace!.loadDatabaseUrl(
                loadPath + res,
                (command: string, percent: number) => {
                  setProgress(command);
                },
                (res) => {
                  info('loadDatabaseUrl success');
                  mainMenu.menus!.splice(1, mainMenu.menus!.length > 2 ? 1 : 0, {
                    collapsed: false,
                    title: 'Current Trace',
                    describe: 'Actions on the current trace',
                    children: getTraceOptionMenus(showFileName, fileSize, fileName, true, dbName),
                  });
                  litSearch.setPercent('', 101);
                  chartFilter!.setAttribute('mode', '');
                  progressEL.loading = false;
                  that.freshMenuDisable(false);
                }
              );
            } else {
              litSearch.setPercent('', 101);
              progressEL.loading = false;
              that.freshMenuDisable(false);
            }
            spInfoAndStats.initInfoAndStatsData();
          });
      });
    }

    function sendCutFileMessage(timStamp: number) {
      that.pageTimStamp = timStamp;
      threadPool.init('wasm').then(() => {
        let headUintArray = new Uint8Array(that.longTraceHeadMessageList.length * 1024);
        let headOffset = 0;
        that.longTraceHeadMessageList = that.longTraceHeadMessageList.sort(
          (leftMessage, rightMessage) => leftMessage.pageNum - rightMessage.pageNum
        );
        for (let index = 0; index < that.longTraceHeadMessageList.length; index++) {
          let currentUintArray = new Uint8Array(that.longTraceHeadMessageList[index].data);
          headUintArray.set(currentUintArray, headOffset);
          headOffset += currentUintArray.length;
        }
        threadPool.submit(
          'ts-cut-file',
          '',
          {
            headArray: headUintArray,
            timeStamp: timStamp,
            splitFileInfo: that.longTraceTypeMessageMap?.get(0),
            splitDataList: that.longTraceDataList,
          },
          (res: Array<any>) => {
            litSearch.setPercent('Cut in file ', 100);
            if (that.longTraceHeadMessageList.length > 0) {
              getTraceFileByPage(that.currentPageNum);
              litSearch.style.marginLeft = '80px';
              longTracePage.style.display = 'flex';
              let pageListDiv = that.shadowRoot?.querySelector('.page-number-list') as HTMLDivElement;
              let previewButton: HTMLDivElement | null | undefined =
                that.shadowRoot?.querySelector<HTMLDivElement>('#preview-button');
              let nextButton: HTMLDivElement | null | undefined =
                that.shadowRoot?.querySelector<HTMLDivElement>('#next-button');
              let pageInput = that.shadowRoot?.querySelector<HTMLInputElement>('.page-input');
              pageListDiv.innerHTML = '';
              that.refreshPageList(
                pageListDiv,
                previewButton!,
                nextButton!,
                pageInput!,
                that.currentPageNum,
                that.longTraceHeadMessageList.length
              );
              if (previewButton) {
                previewButton.addEventListener('click', () => {
                  if (progressEL.loading || that.currentPageNum === 1) {
                    return;
                  }
                  if (that.currentPageNum > 1) {
                    that.currentPageNum--;
                    progressEL.loading = true;
                    that.refreshPageList(
                      pageListDiv,
                      previewButton!,
                      nextButton!,
                      pageInput!,
                      that.currentPageNum,
                      that.longTraceHeadMessageList.length
                    );
                    getTraceFileByPage(that.currentPageNum);
                  }
                });
              }
              nextButton!.addEventListener('click', () => {
                if (progressEL.loading || that.currentPageNum === that.longTraceHeadMessageList.length) {
                  return;
                }
                if (that.currentPageNum < that.longTraceHeadMessageList.length) {
                  that.currentPageNum++;
                  progressEL.loading = true;
                  that.refreshPageList(
                    pageListDiv,
                    previewButton!,
                    nextButton!,
                    pageInput!,
                    that.currentPageNum,
                    that.longTraceHeadMessageList.length
                  );
                  getTraceFileByPage(that.currentPageNum);
                }
              });
              let nodeListOf = pageListDiv.querySelectorAll<HTMLDivElement>('div');
              nodeListOf.forEach((divEL, index) => {
                divEL.addEventListener('click', () => {
                  if (progressEL.loading) {
                    return;
                  }
                  if (divEL.textContent === '...') {
                    let freeSize =
                      Number(nodeListOf[index + 1].textContent) - Number(nodeListOf[index - 1].textContent);
                    that.currentPageNum = Math.floor(freeSize / 2 + Number(nodeListOf[index - 1].textContent));
                  } else {
                    that.currentPageNum = Number(divEL.textContent);
                  }
                  progressEL.loading = true;
                  that.refreshPageList(
                    pageListDiv,
                    previewButton!,
                    nextButton!,
                    pageInput!,
                    that.currentPageNum,
                    that.longTraceHeadMessageList.length
                  );
                  getTraceFileByPage(that.currentPageNum);
                });
              });
              pageInput!.addEventListener('input', () => {
                let value = pageInput!.value;
                value = value.replace(/\D/g, '');
                if (value) {
                  value = Math.min(that.longTraceHeadMessageList.length, parseInt(value)).toString();
                }
                pageInput!.value = value;
              });
              let pageConfirmEl = that.shadowRoot?.querySelector<HTMLDivElement>('.confirm-button');
              pageConfirmEl!.addEventListener('click', () => {
                if (progressEL.loading) {
                  return;
                }
                that.currentPageNum = Number(pageInput!.value);
                progressEL.loading = true;
                that.refreshPageList(
                  pageListDiv,
                  previewButton!,
                  nextButton!,
                  pageInput!,
                  that.currentPageNum,
                  that.longTraceHeadMessageList.length
                );
                getTraceFileByPage(that.currentPageNum);
              });
            }
          },
          'long_trace'
        );
      });
    }

    function getTraceFileByPage(pageNumber: number): void {
      openFileInit();
      litSearch.clear();
      showContent(spSystemTrace!);
      that.search = true;
      progressEL.loading = true;
      if (!that.wasm) {
        progressEL.loading = false;
        return;
      }
      if (that.pageTimStamp === 0) {
        return;
      }
      let indexedDbPageNum = pageNumber - 1;
      let maxTraceFileLength = 400 * 1024 * 1024;
      let traceRange = IDBKeyRange.bound(
        [that.pageTimStamp, 'trace', indexedDbPageNum],
        [that.pageTimStamp, 'trace', indexedDbPageNum],
        false,
        false
      );
      LongTraceDBUtils.getInstance()
        .indexedDBHelp.get(LongTraceDBUtils.getInstance().tableName, traceRange, 'QueryFileByPage')
        .then((result) => {
          let traceData = indexedDataToBufferData(result);
          let traceLength = traceData.byteLength;
          let ebpfRange = IDBKeyRange.bound(
            [that.pageTimStamp, 'ebpf_new', indexedDbPageNum],
            [that.pageTimStamp, 'ebpf_new', indexedDbPageNum],
            false,
            false
          );
          let arkTsRange = IDBKeyRange.bound(
            [that.pageTimStamp, 'arkts_new', indexedDbPageNum],
            [that.pageTimStamp, 'arkts_new', indexedDbPageNum],
            false,
            false
          );
          let hiperfRange = IDBKeyRange.bound(
            [that.pageTimStamp, 'hiperf_new', indexedDbPageNum],
            [that.pageTimStamp, 'hiperf_new', indexedDbPageNum],
            false,
            false
          );
          Promise.all([
            LongTraceDBUtils.getInstance().indexedDBHelp.get(
              LongTraceDBUtils.getInstance().tableName,
              ebpfRange,
              'QueryFileByPage'
            ),
            LongTraceDBUtils.getInstance().indexedDBHelp.get(
              LongTraceDBUtils.getInstance().tableName,
              arkTsRange,
              'QueryFileByPage'
            ),
            LongTraceDBUtils.getInstance().indexedDBHelp.get(
              LongTraceDBUtils.getInstance().tableName,
              hiperfRange,
              'QueryFileByPage'
            ),
          ]).then((otherResult) => {
            let ebpfData = indexedDataToBufferData(otherResult[0]);
            let arkTsData = indexedDataToBufferData(otherResult[1]);
            let hiperfData = indexedDataToBufferData(otherResult[2]);
            let traceArray = new Uint8Array(traceData);
            let ebpfArray = new Uint8Array(ebpfData);
            let arkTsArray = new Uint8Array(arkTsData);
            let hiPerfArray = new Uint8Array(hiperfData);
            let allOtherData = [ebpfData, arkTsData, hiperfData];
            let otherDataLength = traceLength + ebpfData.byteLength + arkTsData.byteLength + hiperfData.byteLength;
            let fileName = `hiprofiler_long_trace_${indexedDbPageNum}.htrace`;
            if (otherDataLength > maxTraceFileLength) {
              if (traceLength > maxTraceFileLength) {
                litSearch.isLoading = false;
                litSearch.setPercent('hitrace file too big!', -1);
                progressEL.loading = false;
                that.freshMenuDisable(false);
              } else {
                let freeDataLength = maxTraceFileLength - traceLength;
                let freeDataIndex = findFreeSizeAlgorithm(
                  [ebpfData.byteLength, arkTsData.byteLength, hiperfData.byteLength],
                  freeDataLength
                );
                let finalData = [traceData];
                freeDataIndex.forEach((dataIndex) => {
                  finalData.push(allOtherData[dataIndex]);
                });
                let fileBlob = new Blob(finalData);
                const file = new File([fileBlob], fileName);
                let fileSize = (file.size / 1048576).toFixed(1);
                document.title = `${fileName}(${fileSize}M)`;
                handleWasmMode(file, file.name, `${fileSize}M`, fileName);
              }
            } else {
              let fileBlob = new Blob([traceArray, ebpfArray, arkTsArray, hiPerfArray]);
              const file = new File([fileBlob], fileName);
              let fileSize = (file.size / 1048576).toFixed(1);
              document.title = `${fileName}(${fileSize}M)`;
              handleWasmMode(file, file.name, `${fileSize}M`, file.name);
            }
            that.traceFileName = fileName;
          });
        });
    }

    function indexedDataToBufferData(sourceData: any): ArrayBuffer {
      let uintArrayLength = 0;
      let uintDataList = sourceData.map((item: any) => {
        let currentBufData = new Uint8Array(item.buf);
        uintArrayLength += currentBufData.length;
        return currentBufData;
      });
      let resultArrayBuffer = new ArrayBuffer(uintArrayLength);
      let resultUintArray = new Uint8Array(resultArrayBuffer);
      let offset = 0;
      uintDataList.forEach((currentArray: Uint8Array) => {
        resultUintArray.set(currentArray, offset);
        offset += currentArray.length;
      });
      return resultArrayBuffer;
    }

    function findFreeSizeAlgorithm(numbers: Array<number>, freeSize: number): Array<number> {
      let closestSize = 0;
      let currentSize = 0;
      let finalIndex: Array<number> = [];
      let currentSelectIndex: Array<number> = [];

      function reBackFind(index: number): void {
        if (index === numbers.length) {
          const sumDifference = Math.abs(currentSize - freeSize);
          if (currentSize <= freeSize && sumDifference < Math.abs(closestSize - freeSize)) {
            closestSize = currentSize;
            finalIndex = [...currentSelectIndex];
          }
          return;
        }
        currentSize += numbers[index];
        currentSelectIndex.push(index);
        reBackFind(index + 1);
        currentSize -= numbers[index];
        currentSelectIndex.pop();
        reBackFind(index + 1);
      }

      reBackFind(0);
      return finalIndex;
    }

    function handleWasmMode(ev: any, showFileName: string, fileSize: string, fileName: string) {
      litSearch.setPercent('', 1);
      threadPool.init('wasm').then((res) => {
        let reader: FileReader | null = new FileReader();
        reader.readAsArrayBuffer(ev as any);
        reader.onloadend = function (ev) {
          info('read file onloadend');
          litSearch.setPercent('ArrayBuffer loaded  ', 2);
          let wasmUrl = `https://${window.location.host.split(':')[0]}:${window.location.port}/application/wasm.json`;
          if (that.vs) {
            wasmUrl = `http://${window.location.host.split(':')[0]}:${window.location.port}/wasm.json`;
          }
          SpApplication.loadingProgress = 0;
          SpApplication.progressStep = 3;
          let data = this.result as ArrayBuffer;
          spSystemTrace!.loadDatabaseArrayBuffer(
            data,
            wasmUrl,
            (command: string, percent: number) => {
              setProgress(command);
            },
            async (res) => {
              let existFtrace = await queryExistFtrace();
              let isAllowTrace = true;
              if (DbPool.sharedBuffer) {
                let traceHeadData = new Uint8Array(DbPool.sharedBuffer!.slice(0, 10));
                let enc = new TextDecoder();
                let headerStr = enc.decode(traceHeadData);
                let rowTraceStr = Array.from(new Uint8Array(DbPool.sharedBuffer!.slice(0, 2)))
                .map((byte) => byte.toString(16).padStart(2, '0'))
                .join('');
                if (headerStr.indexOf('OHOSPROF') !== 0 && rowTraceStr.indexOf('49df') !== 0) {
                  isAllowTrace = false;
                }
              }
              let index = 2;
              if (existFtrace.length > 0 && isAllowTrace) {
                mainMenu.menus!.splice(2, 1, {
                  collapsed: false,
                  title: 'Convert trace',
                  describe: 'Convert to other formats',
                  children: pushConvertTrace(fileName),
                });
                index = 3;
              }
              mainMenu.menus!.splice(index, 1, {
                collapsed: false,
                title: 'Support',
                describe: 'Support',
                children: [
                  {
                    title: 'Help Documents',
                    icon: 'smart-help',
                    clickHandler: function (item: MenuItem) {
                      SpStatisticsHttpUtil.addOrdinaryVisitAction({
                        event: 'help_page',
                        action: 'help_doc',
                      });
                      that.search = false;
                      that.spHelp!.dark = that.dark;
                      showContent(that.spHelp!);
                    },
                  },
                  {
                    title: 'Flags',
                    icon: 'menu',
                    clickHandler: function (item: MenuItem) {
                      SpStatisticsHttpUtil.addOrdinaryVisitAction({
                        event: 'flags',
                        action: 'flags',
                      });
                      that.search = false;
                      showContent(spFlags);
                    },
                  },
                ],
              });
              if (res.status) {
                info('loadDatabaseArrayBuffer success');
                mainMenu.menus!.splice(1, mainMenu.menus!.length > 2 ? 1 : 0, {
                  collapsed: false,
                  title: 'Current Trace',
                  describe: 'Actions on the current trace',
                  children: getTraceOptionMenus(showFileName, fileSize, fileName, false),
                });
                showContent(spSystemTrace!);
                litSearch.setPercent('', 101);
                chartFilter!.setAttribute('mode', '');
                progressEL.loading = false;
                that.freshMenuDisable(false);
              } else {
                info('loadDatabaseArrayBuffer failed');
                litSearch.setPercent(res.msg || 'This File is not supported!', -1);
                progressEL.loading = false;
                that.freshMenuDisable(false);
                mainMenu.menus!.splice(1, 1);
                mainMenu.menus = mainMenu.menus!;
              }
              spInfoAndStats.initInfoAndStatsData();
              reader = null;
            }
          );
        };
      });
    }

    let openFileInit = () => {
      SpStatisticsHttpUtil.addOrdinaryVisitAction({
        event: 'open_trace',
        action: 'open_trace',
      });
      info('openTraceFile');
      spSystemTrace!.clearPointPair();
      spSystemTrace!.reset((command: string, percent: number) => {
        setProgress(command);
      });
      window.publish(window.SmartEvent.UI.MouseEventEnable, {
        mouseEnable: false,
      });
      window.clearTraceRowComplete();
      that.freshMenuDisable(true);
      SpSchedulingAnalysis.resetCpu();
      if (mainMenu.menus!.length > 3) {
        mainMenu.menus!.splice(1, 2);
        mainMenu.menus = mainMenu.menus!;
      } else if (mainMenu.menus!.length > 2) {
        mainMenu.menus!.splice(1, 1);
        mainMenu.menus = mainMenu.menus!;
      }
    };

    function openLongTraceFile(ev: any, isRecordTrace: boolean = false) {
      openFileInit();
      litSearch.clear();
      showContent(spSystemTrace!);
      that.search = true;
      progressEL.loading = true;
      if (!that.wasm) {
        progressEL.loading = false;
        return;
      }
      if (longTracePage) {
        longTracePage.style.display = 'none';
        litSearch.style.marginLeft = '0px';
        let pageListDiv = that.shadowRoot?.querySelector('.page-number-list') as HTMLDivElement;
        pageListDiv.innerHTML = '';
      }
      that.currentPageNum = 1;
      if (isRecordTrace) {
        let detail = (ev as any).detail;
        sendCutFileMessage(detail.timeStamp);
      } else {
        that.longTraceHeadMessageList = [];
        that.longTraceTypeMessageMap = undefined;
        that.longTraceDataList = [];
        let detail = (ev as any).detail;
        let timStamp = new Date().getTime();
        let traceTypePage: Array<number> = [];
        let allFileSize = 0;
        let readSize = 0;
        for (let index = 0; index < detail.length; index++) {
          let file = detail[index];
          let fileName = file.name as string;
          allFileSize += file.size;
          if (that.fileTypeList.some((fileType) => fileName.toLowerCase().includes(fileType))) {
            continue;
          }
          let firstLastIndexOf = fileName.lastIndexOf('.');
          let firstText = fileName.slice(0, firstLastIndexOf);
          let resultLastIndexOf = firstText.lastIndexOf('_');
          traceTypePage.push(Number(firstText.slice(resultLastIndexOf + 1, firstText.length)) - 1);
        }
        traceTypePage.sort((leftNum: number, rightNum: number) => leftNum - rightNum);
        const readFiles = async (files: FileList, traceTypePage: Array<number>) => {
          const promises = Array.from(files).map((file) => {
            let types = that.fileTypeList.filter((type) => file.name.toLowerCase().includes(type.toLowerCase()));
            return readFile(file, types, traceTypePage);
          });
          return Promise.all(promises);
        };
        const readFile = async (file: any, types: Array<string>, traceTypePage: Array<number>) => {
          return new Promise((resolve, reject) => {
            let fileName = file.name;
            let fr = new FileReader();
            let message = {
              fileType: '',
              startIndex: 0,
              endIndex: 0,
              size: 0,
            };
            info('Parse long trace using wasm mode ');
            let maxSize = 48 * 1024 * 1024;
            let fileType = 'trace';
            let pageNumber = 0;
            if (types.length > 0) {
              fileType = types[0];
            } else {
              let firstLastIndexOf = fileName.lastIndexOf('.');
              let firstText = fileName.slice(0, firstLastIndexOf);
              let resultLastIndexOf = firstText.lastIndexOf('_');
              let searchNumber = Number(firstText.slice(resultLastIndexOf + 1, firstText.length)) - 1;
              pageNumber = traceTypePage.lastIndexOf(searchNumber);
            }
            let chunk = maxSize;
            let offset = 0;
            let sliceLen = 0;
            let index = 1;
            fr.onload = function () {
              let data = fr.result as ArrayBuffer;
              LongTraceDBUtils.getInstance()
                .indexedDBHelp.add(LongTraceDBUtils.getInstance().tableName, {
                  buf: data,
                  id: `${fileType}_${timStamp}_${pageNumber}_${index}`,
                  fileType: fileType,
                  pageNum: pageNumber,
                  startOffset: offset,
                  endOffset: offset + sliceLen,
                  index: index,
                  timStamp: timStamp,
                })
                .then(() => {
                  if (index === 1 && types.length === 0) {
                    that.longTraceHeadMessageList.push({
                      pageNum: pageNumber,
                      data: data.slice(offset, 1024),
                    });
                  }
                  that.longTraceDataList.push({
                    index: index,
                    fileType: fileType,
                    pageNum: pageNumber,
                    startOffsetSize: offset,
                    endOffsetSize: offset + sliceLen,
                  });
                  offset += sliceLen;
                  if (offset < file.size) {
                    index++;
                  }
                  continue_reading();
                });
            };

            function continue_reading() {
              if (offset >= file.size) {
                message.endIndex = index;
                message.size = file.size;
                if (that.longTraceTypeMessageMap) {
                  if (that.longTraceTypeMessageMap?.has(pageNumber)) {
                    let oldTypeList = that.longTraceTypeMessageMap?.get(pageNumber);
                    oldTypeList?.push(message);
                    that.longTraceTypeMessageMap?.set(pageNumber, oldTypeList!);
                  } else {
                    that.longTraceTypeMessageMap?.set(pageNumber, [message]);
                  }
                } else {
                  that.longTraceTypeMessageMap = new Map();
                  that.longTraceTypeMessageMap.set(pageNumber, [message]);
                }
                resolve(true);
                return;
              }
              if (index === 1) {
                message.fileType = fileType;
                message.startIndex = index;
              }
              sliceLen = Math.min(file.size - offset, chunk);
              let slice = file.slice(offset, offset + sliceLen);
              readSize += slice.size;
              let percentValue = ((readSize * 100) / allFileSize).toFixed(2);
              litSearch.setPercent('Read in file: ', Number(percentValue));
              fr.readAsArrayBuffer(slice);
            }
            continue_reading();
            fr.onerror = function () {
              reject(false);
            };
          });
        };
        litSearch.setPercent('Read in file: ', 1);
        readFiles(detail, traceTypePage).then(() => {
          litSearch.setPercent('Cut in file: ', 1);
          sendCutFileMessage(timStamp);
        });
      }
    }

    function openTraceFile(ev: any, isClickHandle?: boolean) {
      that.removeAttribute('custom-color');
      longTracePage.style.display = 'none';
      litSearch.style.marginLeft = '0px';
      let pageListDiv = that.shadowRoot?.querySelector('.page-number-list') as HTMLDivElement;
      pageListDiv.innerHTML = '';
      openFileInit();
      if (that.vs && isClickHandle) {
        Cmd.openFileDialog().then((res: string) => {
          if (res != '') {
            litSearch.clear();
            showContent(spSystemTrace!);
            that.search = true;
            progressEL.loading = true;
            let openResult = JSON.parse(res);
            let fileName = openResult.fileName;
            let fileSize = (openResult.fileSize / 1048576).toFixed(1);
            let showFileName =
              fileName.lastIndexOf('.') == -1 ? fileName : fileName.substring(0, fileName.lastIndexOf('.'));
            document.title = `${showFileName} (${fileSize}M)`;
            TraceRow.rangeSelectObject = undefined;
            if (that.server) {
              info('Parse trace using server mode ');
              handleServerMode(openResult.filePath, showFileName, fileSize, fileName, isClickHandle);
              return;
            }
            if (that.wasm) {
              info('Parse trace using wasm mode ');
              const vsUpload = new FormData();
              vsUpload.append('convertType', 'vsUpload');
              vsUpload.append('isTransform', '');
              vsUpload.append('filePath', openResult.filePath);
              info('openResult.filePath   ', openResult.filePath);
              litSearch.setPercent('upload file ', 1);
              Cmd.uploadFile(vsUpload, (response: Response) => {
                if (response.ok) {
                  response.text().then((traceFile) => {
                    let traceFilePath =
                      `http://${window.location.host.split(':')[0]}:${window.location.port}` + traceFile;
                    fetch(traceFilePath).then((res) => {
                      res.arrayBuffer().then((arrayBuf) => {
                        handleWasmMode(new File([arrayBuf], fileName), showFileName, fileSize, fileName);
                      });
                    });
                  });
                }
              });
              return;
            }
          } else {
            return;
          }
        });
      } else {
        litSearch.clear();
        showContent(spSystemTrace!);
        that.search = true;
        progressEL.loading = true;
        let fileName = (ev as any).name;
        that.traceFileName = fileName;
        let fileSize = ((ev as any).size / 1048576).toFixed(1);
        postLog(fileName, fileSize);
        let showFileName =
          fileName.lastIndexOf('.') == -1 ? fileName : fileName.substring(0, fileName.lastIndexOf('.'));
        document.title = `${showFileName} (${fileSize}M)`;
        TraceRow.rangeSelectObject = undefined;
        if (that.server) {
          info('Parse trace using server mode ');
          handleServerMode(ev, showFileName, fileSize, fileName);
          return;
        }
        if (that.sqlite) {
          info('Parse trace using sql mode');
          litSearch.setPercent('', 0);
          threadPool.init('sqlite').then((res) => {
            let reader = new FileReader();
            reader.readAsArrayBuffer(ev as any);
            reader.onloadend = function (ev) {
              SpApplication.loadingProgress = 0;
              SpApplication.progressStep = 3;
              spSystemTrace!.loadDatabaseArrayBuffer(
                this.result as ArrayBuffer,
                '',
                (command: string, percent: number) => {
                  setProgress(command);
                },
                () => {
                  mainMenu.menus!.splice(1, mainMenu.menus!.length > 2 ? 1 : 0, {
                    collapsed: false,
                    title: 'Current Trace',
                    describe: 'Actions on the current trace',
                    children: getTraceOptionMenus(showFileName, fileSize, fileName, false),
                  });
                  litSearch.setPercent('', 101);
                  chartFilter!.setAttribute('mode', '');
                  progressEL.loading = false;
                  that.freshMenuDisable(false);
                }
              );
            };
          });
          return;
        }
        if (that.wasm) {
          info('Parse trace using wasm mode ');
          handleWasmMode(ev, showFileName, fileSize, fileName);
          return;
        }
      }
    }

    mainMenu.menus = [
      {
        collapsed: false,
        title: 'Navigation',
        describe: 'Open or record a new trace',
        children: [
          {
            title: 'Open trace file',
            icon: 'folder',
            fileChoose: !that.vs,
            fileHandler: function (ev: InputEvent) {
              openTraceFile(ev.detail as any);
            },
            clickHandler: function (hand: any) {
              openTraceFile(hand, true);
            },
          },
          {
            title: 'Open long trace file',
            icon: 'folder',
            fileChoose: !that.vs,
            fileHandler: function (ev: InputEvent) {
              openLongTraceFile(ev);
            },
            clickHandler: function (hand: any) {
              openLongTraceFile(hand, true);
            },
          },
          {
            title: 'Record new trace',
            icon: 'copyhovered',
            clickHandler: function (item: MenuItem) {
              if (that.vs) {
                spRecordTrace!.vs = true;
                spRecordTrace!.startRefreshDeviceList();
              }
              spRecordTrace!.synchronizeDeviceList();
              spRecordTemplate!.record_template = false;
              spRecordTrace!.refreshConfig(true);
              showContent(spRecordTrace!);
            },
          },
          {
            title: 'Record template',
            icon: 'copyhovered',
            clickHandler: function (item: MenuItem) {
              if (that.vs) {
                spRecordTemplate!.vs = true;
                spRecordTemplate!.startRefreshDeviceList();
              }
              spRecordTemplate!.refreshHint();
              spRecordTemplate!.record_template = true;
              spRecordTemplate!.refreshConfig(false);
              spRecordTemplate!.synchronizeDeviceList();
              showContent(spRecordTemplate!);
            },
          },
        ],
      },
      {
        collapsed: false,
        title: 'Support',
        describe: 'Support',
        children: [
          {
            title: 'Help Documents',
            icon: 'smart-help',
            clickHandler: function (item: MenuItem) {
              that.spHelp!.dark = that.dark;
              that.search = false;
              showContent(that.spHelp!);
              SpStatisticsHttpUtil.addOrdinaryVisitAction({
                event: 'help_page',
                action: 'help_doc',
              });
            },
          },
          {
            title: 'Flags',
            icon: 'menu',
            clickHandler: function (item: MenuItem) {
              that.search = false;
              showContent(spFlags);
              SpStatisticsHttpUtil.addOrdinaryVisitAction({
                event: 'flags',
                action: 'flags',
              });
            },
          },
        ],
      },
    ];

    let body = document.querySelector('body');
    body!.addEventListener(
      'dragover',
      (e: any) => {
        e.preventDefault();
        e.stopPropagation();
        if (e.dataTransfer.items.length > 0 && e.dataTransfer.items[0].kind === 'file') {
          e.dataTransfer.dropEffect = 'copy';
          if (!this.rootEL!.classList.contains('filedrag')) {
            this.rootEL!.classList.add('filedrag');
          }
        }
      },
      false
    );
    body!.addEventListener(
      'dragleave',
      (e) => {
        e.stopPropagation();
        e.preventDefault();
        if (this.rootEL!.classList.contains('filedrag')) {
          this.rootEL!.classList.remove('filedrag');
        }
      },
      false
    );
    body!.addEventListener(
      'drop',
      (e: any) => {
        e.preventDefault();
        e.stopPropagation();
        if (this.rootEL!.classList.contains('filedrag')) {
          this.rootEL!.classList.remove('filedrag');
        }
        if (e.dataTransfer.items !== undefined && e.dataTransfer.items.length > 0) {
          let item = e.dataTransfer.items[0];
          if (item.webkitGetAsEntry()?.isFile) {
            openTraceFile(item.getAsFile());
          } else if (item.webkitGetAsEntry()?.isDirectory) {
            litSearch.setPercent('This File is not supported!', -1);
            progressEL.loading = false;
            that.freshMenuDisable(false);
            mainMenu.menus!.splice(1, 1);
            mainMenu.menus = mainMenu.menus!;
            spSystemTrace!.reset(null);
          }
        }
      },
      false
    );
    document.addEventListener('keydown', (event) => {
      const e = event || window.event;
      const ctrlKey = e.ctrlKey || e.metaKey;
      if (ctrlKey && (this.keyCodeMap as any)[e.keyCode]) {
        e.preventDefault();
      } else if (e.detail) {
        // Firefox
        event.returnValue = false;
      }
    });
    document.body.addEventListener(
      'wheel',
      (e) => {
        if (e.ctrlKey) {
          if (e.deltaY < 0) {
            e.preventDefault();
            return false;
          }
          if (e.deltaY > 0) {
            e.preventDefault();
            return false;
          }
        }
      },
      { passive: false }
    );

    const openMenu = (open: boolean) => {
      if (mainMenu) {
        mainMenu.style.width = open ? `248px` : '0px';
        mainMenu.style.zIndex = open ? '2000' : '0';
      }
      if (sidebarButton) {
        sidebarButton.style.width = open ? `0px` : '48px';
      }
    };
    let urlParams = new URL(window.location.href).searchParams;
    if (urlParams && urlParams.get('trace') && urlParams.get('link')) {
      openFileInit();
      openMenu(false);
      litSearch.clear();
      showContent(spSystemTrace!);
      that.search = true;
      progressEL.loading = true;
      let downloadLineFile = false;
      if (urlParams.get('local')) {
        downloadLineFile = false;
      } else {
        downloadLineFile = true;
      }
      setProgress(downloadLineFile ? 'download trace file' : 'open trace file');
      this.downloadOnLineFile(
        urlParams.get('trace') as string,
        downloadLineFile,
        (arrayBuf, fileName, showFileName, fileSize) => {
          handleWasmMode(new File([arrayBuf], fileName), showFileName, fileSize, fileName);
        },
        (localPath) => {
          let path = urlParams.get('trace') as string;
          let fileName: string = '';
          let showFileName: string = '';
          if (urlParams.get('local')) {
            openMenu(true);
            fileName = urlParams.get('traceName') as string;
          } else {
            fileName = path.split('/').reverse()[0];
          }
          that.traceFileName = fileName;
          showFileName = fileName.lastIndexOf('.') == -1 ? fileName : fileName.substring(0, fileName.lastIndexOf('.'));
          TraceRow.rangeSelectObject = undefined;
          let localUrl = downloadLineFile ? `${window.location.origin}${localPath}` : urlParams.get('trace')!;
          fetch(localUrl)
            .then((res) => {
              res.arrayBuffer().then((arrayBuf) => {
                if (urlParams.get('local')) {
                  URL.revokeObjectURL(localUrl);
                }
                let fileSize = (arrayBuf.byteLength / 1048576).toFixed(1);
                postLog(fileName, fileSize);
                document.title = `${showFileName} (${fileSize}M)`;
                info('Parse trace using wasm mode ');
                handleWasmMode(new File([arrayBuf], fileName), showFileName, fileSize, fileName);
              });
            })
            .catch((e) => {
              if (!downloadLineFile) {
                const firstQuestionMarkIndex = window.location.href.indexOf('?');
                location.replace(window.location.href.substring(0, firstQuestionMarkIndex));
              }
            });
        }
      );
    } else {
      openMenu(true);
    }
  }

  private refreshPageList(
    pageListDiv: HTMLDivElement,
    previewButton: HTMLDivElement,
    nextButton: HTMLDivElement,
    pageInput: HTMLInputElement,
    currentPageNum: number,
    maxPageNumber: number
  ): void {
    if (pageInput) {
      pageInput.textContent = currentPageNum.toString();
      pageInput.value = currentPageNum.toString();
    }
    let pageText: string[] = [];
    if (maxPageNumber > 7) {
      switch (currentPageNum) {
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
          pageText = ['1', '2', '3', '4', '5', '...', maxPageNumber.toString()];
          break;
        case maxPageNumber:
        case maxPageNumber - 1:
        case maxPageNumber - 2:
        case maxPageNumber - 3:
        case maxPageNumber - 4:
          pageText = [
            '1',
            '...',
            (maxPageNumber - 4).toString(),
            (maxPageNumber - 3).toString(),
            (maxPageNumber - 2).toString(),
            (maxPageNumber - 1).toString(),
            maxPageNumber.toString(),
          ];
          break;
        default:
          nextButton.style.pointerEvents = 'auto';
          previewButton!.style.pointerEvents = 'auto';
          nextButton.style.opacity = '1';
          previewButton!.style.opacity = '1';
          pageText = [
            '1',
            '...',
            (currentPageNum - 1).toString(),
            currentPageNum.toString(),
            (currentPageNum + 1).toString(),
            '...',
            maxPageNumber.toString(),
          ];
          break;
      }
    } else {
      pageText = [];
      for (let index = 0; index < maxPageNumber; index++) {
        pageText.push((index + 1).toString());
      }
    }
    let pageNodeList = pageListDiv.querySelectorAll<HTMLDivElement>('div');
    if (pageNodeList.length > 0) {
      pageNodeList.forEach((page, index) => {
        page.textContent = pageText[index].toString();
        page.title = pageText[index];
        if (currentPageNum.toString() === pageText[index]) {
          page.setAttribute('selected', '');
        } else {
          if (page.hasAttribute('selected')) {
            page.removeAttribute('selected');
          }
        }
      });
    } else {
      pageListDiv.innerHTML = '';
      pageText.forEach((page) => {
        let element = document.createElement('div');
        element.className = 'page-number pagination';
        element.textContent = page.toString();
        element.title = page.toString();
        if (currentPageNum.toString() === page.toString()) {
          element.setAttribute('selected', '');
        }
        pageListDiv.appendChild(element);
      });
    }
    nextButton.style.pointerEvents = 'auto';
    nextButton.style.opacity = '1';
    previewButton.style.pointerEvents = 'auto';
    previewButton.style.opacity = '1';
    if (currentPageNum === 1) {
      previewButton.style.pointerEvents = 'none';
      previewButton.style.opacity = '0.7';
    } else if (currentPageNum === maxPageNumber) {
      nextButton.style.pointerEvents = 'none';
      nextButton.style.opacity = '0.7';
    }
  }

  /**
   * 修改颜色或者主题，重新绘制侧边栏和泳道图
   * @param theme 当前主题（深色和浅色）
   * @param colorsArray 预览的情况下传入
   */
  changeTheme(theme: Theme, colorsArray?: Array<string>) {
    let systemTrace = this.shadowRoot!.querySelector<SpSystemTrace>('#sp-system-trace');
    let menu: HTMLDivElement | undefined | null = this.shadowRoot?.querySelector('#main-menu');
    let menuGroup = menu!.shadowRoot?.querySelectorAll<LitMainMenuGroup>('lit-main-menu-group');
    let menuItem = menu!.shadowRoot?.querySelectorAll<LitMainMenuItem>('lit-main-menu-item');
    let customColor = this.shadowRoot?.querySelector('.custom-color') as CustomThemeColor;
    if (!colorsArray) {
      customColor.setRadioChecked(theme);
    }
    if (theme === Theme.DARK) {
      menu!.style.backgroundColor = '#262f3c';
      menu!.style.transition = '1s';
      menuGroup!.forEach((item) => {
        let groupName = item!.shadowRoot!.querySelector('.group-name') as LitMainMenuGroup;
        let groupDescribe = item!.shadowRoot!.querySelector('.group-describe') as LitMainMenuGroup;
        groupName.style.color = 'white';
        groupDescribe.style.color = 'white';
      });
      menuItem!.forEach((item) => {
        item.style.color = 'white';
      });
      if (
        !colorsArray &&
        window.localStorage.getItem('DarkThemeColors') &&
        ColorUtils.FUNC_COLOR_B !== JSON.parse(window.localStorage.getItem('DarkThemeColors')!)
      ) {
        ColorUtils.MD_PALETTE = JSON.parse(window.localStorage.getItem('DarkThemeColors')!);
        ColorUtils.FUNC_COLOR = JSON.parse(window.localStorage.getItem('DarkThemeColors')!);
      } else if (colorsArray) {
        ColorUtils.MD_PALETTE = colorsArray;
        ColorUtils.FUNC_COLOR = colorsArray;
      } else {
        ColorUtils.MD_PALETTE = ColorUtils.FUNC_COLOR_B;
        ColorUtils.FUNC_COLOR = ColorUtils.FUNC_COLOR_B;
      }
    } else {
      menu!.style.backgroundColor = 'white';
      menu!.style.transition = '1s';
      menuGroup!.forEach((item) => {
        let groupName = item!.shadowRoot!.querySelector('.group-name') as LitMainMenuGroup;
        let groupDescribe = item!.shadowRoot!.querySelector('.group-describe') as LitMainMenuGroup;
        groupName.style.color = 'black';
        groupDescribe.style.color = '#92959b';
      });
      menuItem!.forEach((item) => {
        item.style.color = 'black';
      });
      if (
        !colorsArray &&
        window.localStorage.getItem('LightThemeColors') &&
        ColorUtils.FUNC_COLOR_A !== JSON.parse(window.localStorage.getItem('LightThemeColors')!)
      ) {
        ColorUtils.MD_PALETTE = JSON.parse(window.localStorage.getItem('LightThemeColors')!);
        ColorUtils.FUNC_COLOR = JSON.parse(window.localStorage.getItem('LightThemeColors')!);
      } else if (colorsArray) {
        ColorUtils.MD_PALETTE = colorsArray;
        ColorUtils.FUNC_COLOR = colorsArray;
      } else {
        ColorUtils.MD_PALETTE = ColorUtils.FUNC_COLOR_A;
        ColorUtils.FUNC_COLOR = ColorUtils.FUNC_COLOR_A;
      }
    }
    systemTrace!.timerShaftEL!.rangeRuler!.draw();
    if (this.colorTransiton) {
      clearTimeout(this.colorTransiton);
    }
    this.colorTransiton = setTimeout(() => {
      menu!.style.transition = '0s';
    }, 1000);
  }

  private downloadOnLineFile(
    url: string,
    download: boolean,
    openUrl: (buffer: ArrayBuffer, fileName: string, showFileName: string, fileSize: string) => void,
    openFileHandler: (path: string) => void
  ) {
    if (download) {
      fetch(url)
        .then((res) => {
          res.arrayBuffer().then((arrayBuf) => {
            let fileSize = (arrayBuf.byteLength / 1048576).toFixed(1);
            let fileName = url.split('/').reverse()[0];
            document.title = `${fileName} (${fileSize}M)`;
            info('Parse trace using wasm mode ');
            let showFileName =
              fileName.lastIndexOf('.') == -1 ? fileName : fileName.substring(0, fileName.lastIndexOf('.'));
            openUrl(arrayBuf, fileName, showFileName, fileSize);
          });
        })
        .catch((e) => {
          let api = `${window.location.origin}/application/download-file`;
          fetch(api, {
            method: 'POST',
            headers: {
              'Content-Type': 'application/x-www-form-urlencoded',
            },
            body: new URLSearchParams({
              url: url,
            }),
          })
            .then((response) => response.json())
            .then((res) => {
              if (res.code === 0 && res.success) {
                let resultUrl = res.data.url;
                if (resultUrl) {
                  openFileHandler(resultUrl.toString().replace(/\\/g, '/'));
                }
              }
            });
        });
    } else {
      openFileHandler(url);
    }
  }

  private croppingFile(progressEL: LitProgressBar, litSearch: LitSearch) {
    let cutLeftNs = TraceRow.rangeSelectObject?.startNS || 0;
    let cutRightNs = TraceRow.rangeSelectObject?.endNS || 0;
    if (cutRightNs === cutLeftNs) {
      return;
    }
    let recordStartNS = (window as any).recordStartNS;
    let offset = Math.floor((cutRightNs - cutLeftNs) * 0.1);
    let cutLeftTs = recordStartNS + cutLeftNs - offset;
    if (cutLeftNs - offset < 0) {
      cutLeftTs = recordStartNS;
    }
    let recordEndNS = (window as any).recordEndNS;
    let cutRightTs = recordStartNS + cutRightNs + offset;
    if (cutRightTs > recordEndNS) {
      cutRightTs = recordEndNS;
    }
    let minCutDur = 1_000_000;
    if (cutRightTs - cutLeftTs < minCutDur) {
      let unitTs = (cutRightTs - cutLeftTs) / 2;
      let midTs = cutLeftTs + unitTs;
      cutLeftTs = midTs - minCutDur / 2;
      cutRightTs = midTs + minCutDur / 2;
    }
    progressEL.loading = true;
    threadPool.cutFile(cutLeftTs, cutRightTs, (status: boolean, msg: string, cutBuffer?: ArrayBuffer) => {
      progressEL.loading = false;
      if (status) {
        let traceFileName = this.traceFileName as string;
        let cutIndex = traceFileName.indexOf('_cut_');
        let fileType = traceFileName.substring(traceFileName.lastIndexOf('.'));
        let traceName = document.title.replace(/\s*\([^)]*\)/g, '').trim();
        if (cutIndex != -1) {
          traceName = traceName.substring(0, cutIndex);
        } else {
          traceName = traceName;
        }
        let blobUrl = URL.createObjectURL(new Blob([cutBuffer!]));
        window.open(
          `index.html?link=true&local=true&traceName=${traceName}_cut_${cutLeftTs}${fileType}&trace=${encodeURIComponent(
            blobUrl
          )}`
        );
      } else {
        litSearch.setPercent(msg, -1);
        window.setTimeout(() => {
          litSearch.setPercent(msg, 101);
        }, 1000);
      }
    });
  }

  private downloadDB(mainMenu: LitMainMenu, fileDbName: string) {
    let fileName = fileDbName?.substring(0, fileDbName?.lastIndexOf('.')) + '.db';
    threadPool.submit(
      'download-db',
      '',
      {},
      (reqBufferDB: any) => {
        let a = document.createElement('a');
        a.href = URL.createObjectURL(new Blob([reqBufferDB]));
        a.download = fileName;
        a.click();
        this.itemIconLoading(mainMenu, 'Current Trace', 'Download Database', true);
        let that = this;
        let timer = setInterval(function () {
          that.itemIconLoading(mainMenu, 'Current Trace', 'Download Database', false);
          clearInterval(timer);
        }, 4000);
      },
      'download-db'
    );
  }

  private async download(mainMenu: LitMainMenu, fileName: string, isServer: boolean, dbName?: string) {
    let a = document.createElement('a');
    if (isServer) {
      if (dbName != '') {
        let file = dbName?.substring(0, dbName?.lastIndexOf('.')) + fileName.substring(fileName.lastIndexOf('.'));
        a.href = `https://${window.location.host.split(':')[0]}:9000` + file;
      } else {
        return;
      }
    } else {
      a.href = URL.createObjectURL(new Blob([DbPool.sharedBuffer!]));
    }
    a.download = fileName;
    a.click();
    window.URL.revokeObjectURL(a.href);
    let that = this;
    this.itemIconLoading(mainMenu, 'Current Trace', 'Download File', true);
    let timer = setInterval(function () {
      that.itemIconLoading(mainMenu, 'Current Trace', 'Download File', false);
      clearInterval(timer);
    }, 4000);
  }

  private itemIconLoading(mainMenu: LitMainMenu, groupName: string, itemName: string, start: boolean) {
    let currentTraceGroup = mainMenu.shadowRoot?.querySelector<LitMainMenuGroup>(
      `lit-main-menu-group[title='${groupName}']`
    );
    let downloadItem = currentTraceGroup!.querySelector<LitMainMenuItem>(`lit-main-menu-item[title='${itemName}']`);
    let downloadIcon = downloadItem!.shadowRoot?.querySelector('.icon') as LitIcon;
    if (start) {
      downloadItem!.setAttribute('icon', 'convert-loading');
      downloadIcon.setAttribute('spin', '');
    } else {
      downloadItem!.setAttribute('icon', 'download');
      downloadIcon.removeAttribute('spin');
    }
  }

  private vsDownloadDB(mainMenu: LitMainMenu, fileDbName: string) {
    let fileName = fileDbName?.substring(0, fileDbName?.lastIndexOf('.')) + '.db';
    threadPool.submit(
      'download-db',
      '',
      {},
      (reqBufferDB: any) => {
        Cmd.showSaveFile((filePath: string) => {
          if (filePath != '') {
            this.itemIconLoading(mainMenu, 'Current Trace', 'Download Database', true);
            const fd = new FormData();
            fd.append('convertType', 'download');
            fd.append('filePath', filePath);
            fd.append('file', new File([reqBufferDB], fileName));
            Cmd.uploadFile(fd, (res: Response) => {
              if (res.ok) {
                this.itemIconLoading(mainMenu, 'Current Trace', 'Download Database', false);
              }
            });
          }
        });
      },
      'download-db'
    );
  }

  private vsDownload(mainMenu: LitMainMenu, fileName: string, isServer: boolean, dbName?: string) {
    Cmd.showSaveFile((filePath: string) => {
      if (filePath != '') {
        this.itemIconLoading(mainMenu, 'Current Trace', 'Download File', true);
        if (isServer) {
          if (dbName != '') {
            let file = dbName?.substring(0, dbName?.lastIndexOf('.')) + fileName.substring(fileName.lastIndexOf('.'));
            Cmd.copyFile(file, filePath, (res: Response) => {
              this.itemIconLoading(mainMenu, 'Current Trace', 'Download File', false);
            });
          }
        } else {
          const fd = new FormData();
          fd.append('convertType', 'download');
          fd.append('filePath', filePath);
          fd.append('file', new File([DbPool.sharedBuffer!], fileName));
          Cmd.uploadFile(fd, (res: Response) => {
            if (res.ok) {
              this.itemIconLoading(mainMenu, 'Current Trace', 'Download File', false);
            }
          });
        }
      }
    });
  }

  private stopDownLoading(mainMenu: LitMainMenu, title: string = 'Download File') {
    let querySelectorAll = mainMenu.shadowRoot?.querySelectorAll<LitMainMenuGroup>('lit-main-menu-group');
    querySelectorAll!.forEach((menuGroup) => {
      let attribute = menuGroup.getAttribute('title');
      if (attribute === 'Current Trace') {
        let querySelectors = menuGroup.querySelectorAll<LitMainMenuItem>('lit-main-menu-item');
        querySelectors.forEach((item) => {
          if (item.getAttribute('title') == title) {
            item!.setAttribute('icon', 'download');
            let querySelector1 = item!.shadowRoot?.querySelector('.icon') as LitIcon;
            querySelector1.removeAttribute('spin');
          }
        });
      }
    });
  }

  freshMenuDisable(disable: boolean) {
    let mainMenu = this.shadowRoot?.querySelector('#main-menu') as LitMainMenu;
    // @ts-ignore
    mainMenu.menus[0].children[0].disabled = disable;
    // @ts-ignore
    mainMenu.menus[0].children[1].disabled = disable;
    if (mainMenu.menus!.length > 2) {
      // @ts-ignore
      mainMenu.menus[1].children.map((it) => (it.disabled = disable));
    }
    mainMenu.menus = mainMenu.menus;
    let litIcon = this.shadowRoot?.querySelector('.filter-config') as LitIcon;
    if (disable) {
      litIcon.style.visibility = 'hidden';
    } else {
      litIcon.style.visibility = 'visible';
    }
  }
}
