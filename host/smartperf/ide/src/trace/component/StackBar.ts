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
import { SelectionData } from '../bean/BoxSelection.js';
import { Utils } from './trace/base/Utils.js';

@element('stack-bar')
export class StackBar extends BaseElement {
  private container: HTMLDivElement | undefined | null;

  static get observedAttributes() {
    return ['mode']; // max min hidden show 三种状态
  }

  set data(val: Array<SelectionData>) {
    let map = new Map<string, StackValue>();
    for (let v of val) {
      if (map.has(v.state)) {
        let sv = map.get(v.state);
        sv!.value = sv!.value + v.wallDuration;
        sv!.state = v.state + ' : ' + sv!.value.toFixed(7) + 'ms';
      } else {
        let sv = new StackValue();
        sv.value = v.wallDuration;
        sv.state = v.state + ' : ' + sv.value.toFixed(7) + 'ms';
        sv.color = Utils.getStateColor(v.stateJX);
        map.set(v.state, sv);
      }
    }
    let totalDuration = 0;
    let arr: Array<StackValue> = [];
    for (let key of map.keys()) {
      if (key == ' ') {
        totalDuration = map.get(key)!.value;
      } else {
        arr.push(map.get(key)!);
      }
    }
    arr.sort((a, b) => a.value - b.value);
    this.container!.innerHTML = '';
    for (let stackValue of arr) {
      this.container!.appendChild(this.createBarElement(stackValue, totalDuration));
    }
  }

  initElements(): void {
    this.container = this.shadowRoot?.querySelector('#container');
  }

  initHtml(): string {
    return `
        <style>
            :host([mode='hidden']){
                display: none;
            }
            :host{
                display: block;
                /*background-color: rebeccapurple;*/
            }
            .state-text{
                width: 10%;display: inline-block;overflow: hidden;white-space: nowrap;padding: 5px; margin-right: 2px;font-size: 9pt;
            }
            </style>
            <div style="display: flex;flex-direction: row;" id="container">
            </div>
        `;
  }

  getStateWidth(state: string): number {
    let canvas = document.createElement('canvas');
    let context = canvas.getContext('2d');
    context!.font = '9pt';
    let metrics = context!.measureText(state);
    return metrics.width;
  }

  createBarElement(sv: StackValue, total: number): HTMLDivElement {
    let bar = document.createElement('div');
    bar.setAttribute('class', 'state-text');
    bar.setAttribute('need-width', this.getStateWidth(sv.state) + '');
    bar.style.backgroundColor = sv.color;
    bar.textContent = sv.state;
    if (sv.state.startsWith('Sleeping')) {
      bar.style.color = '#555555';
    } else {
      bar.style.color = '#ffffff';
    }
    let weight = ((sv.value * 1.0) / total) * 100.0;
    if (weight < 1) {
      weight = 1;
    }
    bar.style.width = weight + '%';
    bar.addEventListener('mouseover', (event) => {
      let needWidth = parseFloat(bar.getAttribute('need-width')!);
      let trueWidth = parseFloat(window.getComputedStyle(bar).width);
      if (trueWidth < needWidth) {
        bar.style.width = needWidth + 100 + 'px';
      }
    });
    bar.addEventListener('mouseleave', (event) => {
      let weight = ((sv.value * 1.0) / total) * 100.0;
      if (weight < 1) {
        weight = 1;
      }
      bar.style.width = weight + '%';
    });
    return bar;
  }
}

export class StackValue {
  state: string = '';
  color: string = '';
  value: number = 0;
}
