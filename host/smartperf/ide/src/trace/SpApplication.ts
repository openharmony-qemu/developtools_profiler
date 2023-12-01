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
import { type SpKeyboard } from './component/SpKeyboard.js';
import './component/SpKeyboard.js';

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
        <div class="root" style="position: relative;">
            <lit-main-menu id="main-menu" class="menu" data=''></lit-main-menu>
            <sp-keyboard style="width:100%;height:100%;overflow:auto;visibility:hidden;top:0px;left:0px;right:0;bottom:0px;position:absolute;z-index: 8888" id="sp-keyboard">
            </sp-keyboard>
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
