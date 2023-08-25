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

import { Graph } from './Graph.js';
import { Rect } from './Rect.js';
import { ns2s, TimerShaftElement } from '../TimerShaftElement.js';

export class TimeRuler extends Graph {
  totalNS: number;
  private stepSmall: number;
  private step: number;
  private stepNS: number;

  constructor(timerShaftEL: TimerShaftElement, frame: Rect, totalNS: number = 10_000_000_000) {
    super(timerShaftEL.canvas, timerShaftEL.ctx!, frame);
    this.totalNS = totalNS;
    this.step = this.frame.width / 10;
    this.stepSmall = this.frame.width / 100;
    this.stepNS = this.totalNS / 10;
  }

  draw() {
    this.stepSmall = this.frame.width / 100;
    this.step = this.frame.width / 10;
    this.stepNS = this.totalNS / 10;
    this.context2D.clearRect(this.frame.x, this.frame.y, this.frame.width, this.frame.height);
    this.context2D.beginPath();
    this.context2D.lineWidth = 1;
    this.context2D.strokeStyle = '#999';
    for (let index = 0; index <= 10; index++) {
      let x = Math.floor(index * this.step) + this.frame.x;
      this.context2D.moveTo(x, 0);
      this.context2D.lineTo(x, this.frame.height);
      if (index == 10) break;
      for (let inner_index = 1; inner_index < 10; inner_index++) {
        this.context2D.moveTo(x + Math.floor(inner_index * this.stepSmall), 0);
        this.context2D.lineTo(x + Math.floor(inner_index * this.stepSmall), this.frame.height / 4);
      }
      this.context2D.fillStyle = '#999';
      this.context2D.fillText(`${ns2s(index * this.stepNS)}`, x + 5, this.frame.height - 1);
    }
    this.context2D.stroke();
    this.context2D.closePath();
  }
}
