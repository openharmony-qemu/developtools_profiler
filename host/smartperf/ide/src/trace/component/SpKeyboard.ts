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

import { BaseElement, element } from '../../base-ui/BaseElement.js';
@element('sp-keyboard')
export class SpKeyboard extends BaseElement {
  initElements(): void {
    let parentElement = this.parentNode as HTMLElement;
    parentElement.style.overflow = 'hidden';
    let closeWindow = this.shadowRoot?.querySelector('.close-icon');
    let keyboardDiv = document
      .querySelector('body > sp-application')!
      .shadowRoot!.querySelector<SpKeyboard>('#sp-keyboard')!;
    let welcomeDiv = document
      .querySelector('body > sp-application')!
      .shadowRoot!.querySelector<SpKeyboard>('#sp-welcome')!;
    let shadow_box = this.shadowRoot?.querySelector('.shadow-box')!;
    closeWindow!.addEventListener('click', () => {
      keyboardDiv.style.visibility = 'hidden';
      welcomeDiv.style.visibility = 'visible';
    });
    shadow_box!.addEventListener('click', () => {
      keyboardDiv.style.visibility = 'hidden';
      welcomeDiv.style.visibility = 'visible';
    });
  }
  initHtml(): string {
    return `
            ${this.initHtmlStyle()}
            <div class="sp-keyboard-container">
            <div class='shadow-box'></div>
             <div class="body">
              <header style="display">
                <h2 class="title">SmartPerf help</h3>
                <a class="close-icon"> ✕ </a>
              </header>
              <main>
                <div class="help">
                  <h2>Navigation</h2>
                  <table>
                    <tr>
                      <td>
                        <div class="keycap">w</div> /
                        <div class="keycap">s</div>
                      </td>
                      <td class="describe-td">放大/缩小</td>
                    </tr>
                    <tr>
                    <td>
                      <div class="keycap">a</div> /
                      <div class="keycap">d</div>
                    </td>
                    <td class="describe-td">左移/右移</td>
                  </tr>
                  </table>
                  <h2>Mouse Controls</h2>
                  <table class="mouse-ctr">
                    <tr>
                      <td>Click</td>
                      <td class="describe-td">点选</td>
                    </tr> 
                    <tr>
                      <td>Click + Drag</td>
                      <td class="describe-td">框选</td>
                    </tr> 
                    <tr>
                    <tr>
                      <td>Ctrl + Scroll wheel</td>
                      <td class="describe-td">放大/缩小</td>
                    </tr> 
                      <td>Ctrl + Click + Drag</td>
                      <td class="describe-td">拖拽左移/右移</td>
                    </tr> 
                  </table>
                  <h2>Making SQL queries from the query page</h2>
                  <table>
                    <tr>
                      <td>
                        <div class="keycap">Ctrl</div> +
                        <div class="keycap">Enter</div>
                      </td>
                      <td class="describe-td">在数据库查询界面写好查询语句后进行查询</td>
                    </tr>
                  </table>
                  <h2>Other</h2>
                  <table>
                    <tr>
                      <td>
                        <div class="keycap">f</div> (with event selected)
                      </td>
                      <td class="describe-td">聚焦</td>
                    </tr>
                    <tr>
                      <td>
                        <div class="keycap">m</div> (with event or area selected)
                      </td>
                      <td class="describe-td">临时框选标记</td>
                    </tr>
                    <tr>
                      <td>
                        <div class="keycap">Shift</div> +
                        <div class="keycap">m</div> (with event or area selected)
                      </td>
                      <td class="describe-td">永久框选标记</td>
                    </tr>
                    <tr>
                      <td>
                        <div class="keycap">Ctrl</div> +
                        <div class="keycap">,</div> /
                        <div class="keycap">.</div>
                      </td>
                      <td class="describe-td">定位到上一个/下一个旗子标记的时间点</td>
                    </tr>
                    <tr>
                      <td>
                        <div class="keycap">Ctrl</div> +
                        <div class="keycap">[</div> /
                        <div class="keycap">]</div>
                      </td>
                      <td class="describe-td">定位到上一个/下一个用shift + m 框选的位置</td>
                    </tr>
                    <tr>
                      <td>
                        <div class="keycap">?</div>
                      </td>
                      <td class="describe-td">展示快捷方式</td>
                    </tr>
                  </table>
                </div>
              </main>
             </div>
            </div>
            `;
  }
  private initHtmlStyle(): string {
    return `
          <style>
            .root{
              position: relative;
            }
            .shadow-box{
              position:absolute;
              width: 100%;
              height : 100%;
              background-color: rgba(0, 0, 0, 0.8);
            }
            .sp-keyboard-container {
              width: 100%;
              height: 100%;
              display: flex;
              justify-content: center;
              align-items: center;
            }
            .body{
              width: 50%;
              background-color: #fff;
              padding: 30px;
              z-index: 9000;
              max-height: 600px;
              overflow-y: scroll;
            }
            .title{
              margin: 0;
              position:absolute;
            }
            header {
              position: relative;
              width: 100%;
              height: 31px;
              line-height: 31px;
            }
            .close-icon{
              cursor: pointer;
              position: absolute;
              right: 19px;
            }
            main{
              font-size: 12px;
              font-weight: 700;
              margin: 50px 0 30px 0;
            }
            table{
              margin-bottom: 15px;
            }
            .help table tr{
              font-size: 16px;
            }
            .help table td{
              min-width: 250px;
              font-weight: 500;
            }
            .keycap{
              background-color: #fafbfc;
              border:1px solid #d1d5da;
              border-bottom-color: #c6cbd1;
              border-radius:3px;
              box-shadow: inset 0 -1px 0 #c6cbd1;
              color: #444d56;
              display: inline-block;
              vertical-align: middle;
              line-height: 20px;
              padding:3px 7px;
              font-weight: 500;
            }
            .mouse-ctr tr{
              line-height: 27px;
            }
            .describe-td{
              font-weight: 200!important;
            }
          </style>
        `;
  }
}
