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
import { DbPool, threadPool } from './database/SqlLite.js';
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

@element('sp-application')
export class SpApplication extends BaseElement {
  private static loadingProgress: number = 0;
  private static progressStep: number = 2;
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
  colorTransiton: any;

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
        .search input{
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
        .search input::placeholder {
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
        .filter-config {
            opacity: 1;
            visibility: hidden;
        }
        .filter-config:hover {
            opacity: 0.7;
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
                </div>
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
                <sp-help style="width:100%;height:100%;overflow:auto;visibility:hidden;top:0px;left:0px;right:0;bottom:0px;position:absolute;z-index: 103" id="sp-help">
                </sp-help>
                <sp-flags style="width:100%;height:100%;overflow:auto;visibility:hidden;top:0px;left:0px;right:0;bottom:0px;position:absolute;z-index: 104" id="sp-flags">
                </sp-flags>
                <trace-row-config class="chart-filter"></trace-row-config>
            </div>
        </div>
        `;
  }

  initElements() {
    SpStatisticsHttpUtil.initStatisticsServerConfig();
    SpStatisticsHttpUtil.addUserVisitAction('visit');
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
    chartFilter!.setAttribute('mode', '');
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
    let sideColor = mainMenu.shadowRoot?.querySelector('.color') as HTMLDivElement;
    //修改侧边导航栏配色
    sideColor!.onclick = (e) => {
      let backgroundColor = sessionStorage.getItem('backgroundColor');
      let menu: HTMLDivElement | undefined | null = this.shadowRoot?.querySelector('#main-menu');
      let menuGroup = mainMenu.shadowRoot?.querySelectorAll<LitMainMenuGroup>('lit-main-menu-group');
      let menuItem = menu!.shadowRoot?.querySelectorAll<LitMainMenuItem>('lit-main-menu-item');
      if (backgroundColor == 'white' || !backgroundColor) {
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
        ColorUtils.MD_PALETTE = ColorUtils.FUNC_COLOR_A;
        ColorUtils.FUNC_COLOR = ColorUtils.FUNC_COLOR_A;
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
          item.style.color = 'var(--dark-color,rgba(0,0,0,0.6))';
        });
        ColorUtils.MD_PALETTE = ColorUtils.FUNC_COLOR_B;
        ColorUtils.FUNC_COLOR = ColorUtils.FUNC_COLOR_B;
      }

      sessionStorage.setItem('backgroundColor', menu!.style.backgroundColor);
      if (this.colorTransiton) {
        clearTimeout(this.colorTransiton);
      }
      this.colorTransiton = setTimeout(() => {
        menu!.style.transition = '0s';
      }, 1000);
    };

    window.subscribe(window.SmartEvent.UI.MenuTrace, () => showContent(spSystemTrace!));
    window.subscribe(window.SmartEvent.UI.Error, (err) => {
      litSearch.setPercent(err, -1);
      progressEL.loading = false;
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
    litSearch.valueChangeHandler = (value: string) => {
      if (value.length > 0) {
        let list = spSystemTrace!.searchCPU(value);
        spSystemTrace!.searchFunction(list, value).then((mixedResults) => {
          if (litSearch.searchValue != '') {
            litSearch.list = spSystemTrace!.searchSdk(mixedResults, value);
          }
        });
      } else {
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
      } else {
        this!.setAttribute('chart_filter', '');
      }
    });
    configClose!.addEventListener('click', (ev) => {
      if (this.hasAttribute('chart_filter')) {
        this!.removeAttribute('chart_filter');
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
        let uploadPath = `https://${window.location.host.split(':')[0]}:9000/upload`;
        if (that.vs) {
          uploadPath = `http://${window.location.host.split(':')[0]}:${window.location.port}/upload`;
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
          spSystemTrace!.loadDatabaseArrayBuffer(
            this.result as ArrayBuffer,
            wasmUrl,
            (command: string, percent: number) => {
              setProgress(command);
            },
            (res) => {
              mainMenu.menus!.splice(2, 1, {
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
      if (mainMenu.menus!.length > 2) {
        mainMenu.menus!.splice(1, 1);
        mainMenu.menus = mainMenu.menus!;
      }
    };

    function openTraceFile(ev: any, isClickHandle?: boolean) {
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
            title: 'Record new trace',
            icon: 'copyhovered',
            clickHandler: function (item: MenuItem) {
              if (that.vs) {
                spRecordTrace!.vs = true;
                spRecordTrace!.startRefreshDeviceList();
              }
              spRecordTrace!.synchronizeDeviceList();
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

    let urlParams = this.getUrlParams(window.location.href);
    if (urlParams && urlParams.trace && urlParams.link) {
      openFileInit();
      openMenu(false);
      litSearch.clear();
      showContent(spSystemTrace!);
      that.search = true;
      progressEL.loading = true;
      let downloadLineFile = false;
      setProgress(downloadLineFile ? 'download trace file' : 'open trace file');
      this.downloadOnLineFile(urlParams.trace, downloadLineFile, (localPath) => {
        let path = urlParams.trace as string;
        let fileName = path.split('/').reverse()[0];
        let showFileName =
          fileName.lastIndexOf('.') == -1 ? fileName : fileName.substring(0, fileName.lastIndexOf('.'));
        TraceRow.rangeSelectObject = undefined;
        let localUrl = downloadLineFile ? `${window.location.origin}${localPath}` : urlParams.trace;
        fetch(localUrl).then((res) => {
          res.arrayBuffer().then((arrayBuf) => {
            let fileSize = (arrayBuf.byteLength / 1048576).toFixed(1);
            postLog(fileName, fileSize);
            document.title = `${showFileName} (${fileSize}M)`;
            info('Parse trace using wasm mode ');
            handleWasmMode(new File([arrayBuf], fileName), showFileName, fileSize, fileName);
          });
        });
      });
    } else {
      openMenu(true);
    }
  }

  private downloadOnLineFile(url: string, download: boolean, openFileHandler: (path: string) => void) {
    if (download) {
      let api = `${window.location.origin}/download-file`;
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
    } else {
      openFileHandler(url);
    }
  }

  private getUrlParams(url: string) {
    const _url = url || window.location.href;
    const _urlParams = _url.match(/([?&])(.+?=[^&]+)/gim);
    return _urlParams ? _urlParams.reduce((a: any, b) => {
          const value = b.slice(1).split('=');
          a[`${value[0]}`] = decodeURIComponent(value[1]);
          return a;
        }, {})
      : {};
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
