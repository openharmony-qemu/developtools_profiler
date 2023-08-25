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
import '../../../../base-ui/checkbox/LitCheckBox.js';
import { LitCheckBox } from '../../../../base-ui/checkbox/LitCheckBox.js';
import { TraceRow } from './TraceRow.js';
import { SpSystemTrace } from '../../SpSystemTrace.js';
import { LitSearch } from '../search/Search.js';
import { TraceSheet } from './TraceSheet.js';
import { CpuStruct } from '../../../database/ui-worker/ProcedureWorkerCPU.js';
import { BaseStruct } from '../../../bean/BaseStruct.js';

@element('trace-row-config')
export class TraceRowConfig extends BaseElement {
  static allTraceRowList: Array<TraceRow<BaseStruct>> = [];
  selectTypeList: Array<string> | undefined = [];
  private spSystemTrace: SpSystemTrace | null | undefined;
  private sceneTable: HTMLDivElement | null | undefined;
  private chartTable: HTMLDivElement | null | undefined;
  private inputElement: HTMLInputElement | null | undefined;
  private traceRowList: NodeListOf<TraceRow<BaseStruct>> | undefined;

  get value(): string {
    return this.getAttribute('value') || '';
  }

  set value(value: string) {
    this.setAttribute('value', value);
  }

  static get observedAttributes(): string[] {
    return ['mode'];
  }

  init(): void {
    let sceneList = ['FrameTimeline', 'Task Pool', 'Animation Effect', 'Ark Ts', 'AppStartup'];
    this.selectTypeList = [];
    this.sceneTable!.innerHTML = '';
    this.chartTable!.innerHTML = '';
    this.inputElement!.value = '';
    this.spSystemTrace = this.parentElement!.querySelector<SpSystemTrace>('sp-system-trace');
    this.traceRowList =
      this.spSystemTrace!.shadowRoot?.querySelector('div[class=rows-pane]')!.querySelectorAll<TraceRow<BaseStruct>>(
        'trace-row[row-parent-id=\'\']'
      );
    let allowSceneList: Array<string> = [];
    TraceRowConfig.allTraceRowList.push(...this.traceRowList!);
    this.traceRowList!.forEach((traceRow: TraceRow<BaseStruct>) => {
      traceRow.setAttribute('scene', '');
      if (traceRow.templateType.length > 0) {
        traceRow.templateType.forEach((type) => {
          if (sceneList.indexOf(type) >= 0 && allowSceneList.indexOf(type) < 0) {
            allowSceneList.push(type);
            this.initConfigSceneTable(type);
          }
        });
      }
      this.initConfigChartTable(traceRow);
    });
  }

  initConfigSceneTable(item: string): void {
    let spliceIndex = 1;
    let div = document.createElement('div');
    div.className = 'scene-option-div';
    div.textContent = item;
    let optionCheckBox: LitCheckBox = new LitCheckBox();
    optionCheckBox.checked = false;
    optionCheckBox.style.justifySelf = 'center';
    optionCheckBox.style.height = '100%';
    optionCheckBox.title = item;
    optionCheckBox.addEventListener('change', () => {
      if (optionCheckBox.checked) {
        this.selectTypeList!.push(item);
      } else {
        if (this.selectTypeList!.length > 0) {
          let indexNum = this.selectTypeList!.indexOf(item);
          this.selectTypeList!.splice(indexNum, spliceIndex);
        }
      }
      this.resetChartOption();
      this.resetChartTable();
    });
    let htmlDivElement = document.createElement('div');
    htmlDivElement.style.display = 'grid';
    htmlDivElement.style.gridTemplateColumns = '1fr 1fr';
    htmlDivElement.appendChild(div);
    htmlDivElement.appendChild(optionCheckBox);
    this.sceneTable?.appendChild(htmlDivElement);
  }

  initConfigChartTable(row: TraceRow<BaseStruct>): void {
    let templateType = '';
    if (row.templateType.length > 0) {
      templateType = row.templateType.reduce((pre, cur) => `${pre  }:${  cur}`);
    }
    let div = document.createElement('div');
    div.className = 'chart-option-div chart-item';
    div.textContent = row.name;
    div.title = row.name;
    div.setAttribute('search_text', row.name);
    let optionCheckBox: LitCheckBox = new LitCheckBox();
    optionCheckBox.checked = true;
    optionCheckBox.className = 'chart-config-check chart-item';
    optionCheckBox.style.height = '100%';
    optionCheckBox.style.justifySelf = 'center';
    optionCheckBox.title = templateType;
    optionCheckBox.setAttribute('search_text', row.name);
    optionCheckBox.addEventListener('change', () => {
      if (row.folder) {
        TraceRowConfig.allTraceRowList.forEach((chartRow): void => {
          let upParentRow = chartRow;
          while (upParentRow.hasParentRowEl) {
            if (!upParentRow.parentRowEl) {
              break;
            }
            upParentRow = upParentRow.parentRowEl;
          }
          if (upParentRow === row) {
            if (optionCheckBox.checked) {
              chartRow.removeAttribute('row-hidden');
              chartRow.setAttribute('scene', '');
            } else {
              row.expansion = true;
              chartRow.removeAttribute('scene');
              chartRow.setAttribute('row-hidden', '');
            }
          }
        });
      }
      if (optionCheckBox.checked) {
        row.removeAttribute('row-hidden');
        row.setAttribute('scene', '');
      } else {
        row.removeAttribute('scene');
        row.setAttribute('row-hidden', '');
      }
      this.refreshSystemPanel();
    });
    this.chartTable!.append(...[div, optionCheckBox]);
  }

  resetChartOption(): void {
    this.shadowRoot!.querySelectorAll<LitCheckBox>('.chart-item').forEach((litCheckBox: LitCheckBox) => {
      let isShowCheck: boolean = false;
      if (this.selectTypeList!.length === 0) {
        isShowCheck = true;
      } else {
        if (litCheckBox.title !== '') {
          let divTemplateTypeList = litCheckBox.title.split(':');
          for (let index = 0; index < divTemplateTypeList.length; index++) {
            let type = divTemplateTypeList[index];
            if (this.selectTypeList!.indexOf(type) >= 0) {
              isShowCheck = true;
              break;
            }
          }
        }
      }
      litCheckBox.checked = isShowCheck;
    });
  }

  resetChartTable(): void {
    if (this.traceRowList && this.traceRowList.length > 0) {
      this.traceRowList.forEach((traceRow: TraceRow<BaseStruct>) => {
        let isShowRow: boolean = false;
        if (this.selectTypeList!.length === 0) {
          traceRow.removeAttribute('row-hidden');
          traceRow.setAttribute('scene', '');
          this.refreshChildRow(traceRow.childrenList, true);
        } else {
          for (let index = 0; index < traceRow.templateType!.length; index++) {
            let type = traceRow.templateType![index];
            if (this.selectTypeList!.indexOf(type) >= 0) {
              isShowRow = true;
              break;
            }
          }
          if (isShowRow) {
            if (traceRow.templateType.length > 0) {
              traceRow.removeAttribute('row-hidden');
              traceRow.setAttribute('scene', '');
              if (traceRow.childrenList && traceRow.childrenList.length > 0) {
                this.refreshChildRow(traceRow.childrenList, isShowRow);
              }
              traceRow.expansion = false;
            }
          } else {
            traceRow.removeAttribute('scene');
            traceRow.setAttribute('row-hidden', '');
            this.refreshChildRow(traceRow.childrenList);
          }
        }
      });
      this.spSystemTrace?.collectRows.forEach(favoriteRow => {
        let isShowRow: boolean = false;
        if (favoriteRow.parentRowEl) {
          favoriteRow.parentRowEl.expansion = false;
        }
        if (this.selectTypeList!.length === 0) {
          favoriteRow.removeAttribute('row-hidden');
          favoriteRow.setAttribute('scene', '');
        } else {
          for (let index = 0; index < favoriteRow.templateType!.length; index++) {
            if (this.selectTypeList!.indexOf(favoriteRow.templateType![index]) >= 0) {
              isShowRow = true;
              break;
            }
          }
          if (isShowRow) {
            if (favoriteRow.templateType.length > 0) {
              favoriteRow.removeAttribute('row-hidden');
              favoriteRow.setAttribute('scene', '');
            }
          } else {
            favoriteRow.removeAttribute('scene');
            favoriteRow.setAttribute('row-hidden', '');
          }
        }
      });
      this.refreshSystemPanel();
    }
  }

  refreshChildRow(childRows: Array<TraceRow<BaseStruct>>, isShowScene: boolean = false): void{
    childRows.forEach(row => {
      if (isShowScene) {
        row.setAttribute('scene', '');
        if (row.childrenList && row.childrenList.length > 0) {
          this.refreshChildRow(row.childrenList, isShowScene);
        }
        row.expansion = false;
      } else {
        row.removeAttribute('scene');
        row.setAttribute('row-hidden', '');
        if (row.childrenList && row.childrenList.length > 0) {
          this.refreshChildRow(row.childrenList);
        }
      }
    });
  }

  refreshSystemPanel(): void {
    this.clearSearchAndFlag();
    this.spSystemTrace!.rowsPaneEL!.scroll({
      top: 0 - this.spSystemTrace!.canvasPanel!.offsetHeight,
      left: 0,
      behavior: 'smooth',
    });
    this.spSystemTrace!.refreshFavoriteCanvas();
    this.spSystemTrace!.refreshCanvas(true);
  }

  clearSearchAndFlag(): void {
    let traceSheet = this.spSystemTrace!.shadowRoot?.querySelector('.trace-sheet') as TraceSheet;
    if (traceSheet) {
      traceSheet!.setAttribute('mode', 'hidden');
    }
    let search = document.querySelector('sp-application')!.shadowRoot?.querySelector('#lit-search') as LitSearch;
    if (search) {
      search.clear();
    }
    let highlightRow = this.spSystemTrace!.shadowRoot?.querySelector<TraceRow<BaseStruct>>('trace-row[highlight]');
    if (highlightRow) {
      highlightRow.highlight = false;
    }
    this.spSystemTrace!.timerShaftEL?.removeTriangle('inverted');
    CpuStruct.wakeupBean = undefined;
    this.spSystemTrace!.hoverFlag = undefined;
    this.spSystemTrace!.selectFlag = undefined;
  }

  initElements(): void {}

  connectedCallback(): void {
    this.sceneTable = this.shadowRoot!.querySelector<HTMLDivElement>('#scene-select');
    this.chartTable = this.shadowRoot!.querySelector<HTMLDivElement>('#chart-select');
    this.inputElement = this.shadowRoot!.querySelector('input');
    this.inputElement?.addEventListener('keyup', () => {
      this.shadowRoot!.querySelectorAll<HTMLElement>('.chart-item').forEach((elementOption: HTMLElement) => {
        let searchText = elementOption.getAttribute('search_text') || '';
        if (searchText!.indexOf(this.inputElement!.value) < 0) {
          elementOption.style.display = 'none';
        } else {
          elementOption.style.display = 'block';
        }
      });
      this.value = this.inputElement!.value;
    });
  }

  initHtml(): string {
    return `
            <style>
                :host([mode='hidden']){
                    visibility: hidden;
                }
                :host{
                    visibility: visible;
                    background-color: #F6F6F6;
                }
                .config-title {
                    height: 100px;
                    border-top: 1px solid #D5D5D5;
                    background-color: #0A59F7;
                    display: flex;
                    align-items: center;
                    padding: 0 20px;
                }
                .title-text {
                    font-family: Helvetica-Bold;
                    font-size: 16px;
                    color: #FFFFFF;
                    text-align: left;
                    font-weight: 700;
                    margin-right: auto;
                }
                .config-close {
                    text-align: right;
                    cursor: pointer;
                    opacity: 1;
                }
                .config-close:hover {
                    opacity: 0.7;
                }
                .title_div{
                  display: flex;
                  flex-direction: row;
                  align-items: center;
                  padding-left: 15px;
                  padding-right: 15px;
                  background-color: #F6F6F6;
                  height: 3.2em;
                }
                .config-scene-select {
                  height: auto;
                  max-height: 120px;
                  overflow-y: auto;
                  background: #FFFFFF;
                  overflow-x: hidden;
                  border-radius: 5px;
                  border: solid 1px #e0e0e0;
                }
                .config-chart-select {
                  display: grid;
                  height: inherit;
                  padding: 10px 30px;
                  background: #FFFFFF;
                  overflow-y: scroll; 
                  overflow-x: hidden;
                  border-radius: 5px;
                  border: solid 1px #e0e0e0;
                  grid-template-columns: auto auto;
                  grid-template-rows: repeat(auto-fit, 35px);
                }
                .config-img {
                    margin-right: 12px;
                } 
                .chart-option-div {
                    height: 35px;
                    line-height: 35px;
                    overflow: hidden;
                    text-overflow: ellipsis;
                }
                .scene-option-div {
                    height: 35px;
                    line-height: 35px;
                    margin-left: 28px;
                }
                input{
                    border: 0;
                    outline: none;
                    background-color: transparent;
                    cursor: pointer;
                    -webkit-user-select:none;
                    -moz-user-select:none;
                    user-select:none;
                    display: inline-flex;
                    width:100%;
                    color: rgba(0,0,0,0.6);
                }
                .multipleSelect{
                    outline: none;
                    font-size: 1rem;
                    -webkit-user-select:none;
                    -moz-user-select:none;
                    position: relative;
                    display: flex;
                    align-items: center;
                    justify-content: space-between;
                    transition: all 0.3s;
                    user-select:none;
                    width: 250px;
                    color: #ffffff;
                    cursor: pointer;
                    line-height: 40px;
                    text-align: center;
                    border:1px solid #dcdcdc;
                    border-radius:16px;
                    background-color: #FFFFFF;
                    height: 70%;
                    margin: auto 4.2em auto auto;
                }
            </style>
            <div class="config-title">
               <span class="title-text">Display Template</span>
               <lit-icon class="config-close" name="config-close" title="Config Close"></lit-icon>
            </div>
            <div class="config-scene" style="display: contents;">
                <div class="title_div">
                    <img class="config-img" title="Template Select" src="img/config_scene.png">
                    <div>Template Select</div>
                </div>
            </div>
            <div class="config-select config-scene-select" id="scene-select"></div>
            <div class="config-chart" style="display: contents;">
                 <div class="title_div">
                    <img class="config-img" title="Timeline Details" src="img/config_chart.png">
                    <div>Timeline Details</div>
                    <div class="multipleSelect" tabindex="0">
                        <div class="multipleRoot" id="select" style="width:100%">
                            <input id="singleInput"/> 
                        </div>
                        <lit-icon class="icon" name='search' color="#c3c3c3"></lit-icon>
                    </div>
                </div>
            </div>
            <div class="config-select config-chart-select" id="chart-select">
            </div>
`;
  }

  attributeChangedCallback(name: string, oldValue: string, newValue: string): void {
    if (name === 'mode' && newValue === '') {
      this.init();
    }
  }
}
