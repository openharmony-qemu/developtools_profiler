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

import { BaseElement, element } from '../../BaseElement.js';
import { LitChartColumnConfig } from './LitChartColumnConfig.js';
import { resizeCanvas } from '../helper.js';
import { getProbablyTime } from '../../../trace/database/logic-worker/ProcedureLogicWorkerCommon.js';

class Pillar {
  obj?: any;
  xLabel?: string;
  yLabel?: string;
  type?: string;
  root?: boolean;
  bgFrame?: {
    x: number;
    y: number;
    w: number;
    h: number;
  };
  frame?: {
    x: number;
    y: number;
    w: number;
    h: number;
  };
  height?: number;
  process?: boolean;
  heightStep?: number;
  centerX?: number;
  centerY?: number;
  color?: string;
  hover?: boolean;
}

interface RLine {
  label: string;
  y: number;
}

@element('lit-chart-column')
export class LitChartColumn extends BaseElement {
  private litChartColumnTipEL: HTMLDivElement | null | undefined;
  litChartColumnCanvas: HTMLCanvasElement | undefined | null;
  litChartColumnCtx: CanvasRenderingContext2D | undefined | null;
  litChartColumnCfg: LitChartColumnConfig | null | undefined;
  offset?: { x: number | undefined; y: number | undefined };
  data: Pillar[] = [];
  rowLines: RLine[] = [];

  connectedCallback() {
    super.connectedCallback();
    this.litChartColumnTipEL = this.shadowRoot!.querySelector<HTMLDivElement>('#tip');
    this.litChartColumnCanvas = this.shadowRoot!.querySelector<HTMLCanvasElement>('#canvas');
    this.litChartColumnCtx = this.litChartColumnCanvas!.getContext('2d', { alpha: true });
    resizeCanvas(this.litChartColumnCanvas!);
    this.offset = { x: 60, y: 20 };
    this.litChartColumnCanvas!.onmouseout = (e) => {
      this.hideTip();
      this.data.forEach((it) => (it.hover = false));
      this.render();
    };
    this.litChartColumnCanvas!.onmousemove = (ev) => {
      let rect = this.getBoundingClientRect();
      let x = ev.pageX - rect.left;
      let y = ev.pageY - rect.top;
      this.data.forEach((it) => {
        if (contains(it.bgFrame!, x, y)) {
          it.hover = true;
          this.litChartColumnCfg?.hoverHandler?.(it.obj.no);
        } else {
          it.hover = false;
        }
      });
      let pillars = this.data.filter((it) => it.hover);
      if (this.litChartColumnCfg?.seriesField) {
        if (pillars.length > 0) {
          let titleEl = `<label>${this.litChartColumnCfg.xField}: ${pillars[0].xLabel}</label>`;
          let messageEl = pillars.map((it) => `<label>${it.type}: ${it.yLabel}</label>`).join('');
          let sumEl = `<label>Total: ${pillars
            .map((item) => item.obj[this.litChartColumnCfg?.yField!])
            .reduce((pre, current) => pre + current, 0)}</label>`;
          let innerHtml = `<div class="tip-content">${titleEl}${messageEl}${sumEl}</div>`;
          this.tipTypeShow(x, y, pillars, innerHtml);
        }
      } else {
        if (pillars.length > 0) {
          let title = `<label>${pillars[0].xLabel}:${pillars[0].yLabel}</label>`;
          let innerHtml = `<div class="tip-content">${title}</div>`;
          this.tipTypeShow(x, y, pillars, innerHtml);
        }
      }

      if (this.data.filter((it) => it.process).length == 0) {
        this.render();
      }
    };
    this.render();
  }

  private tipTypeShow(x: number, y: number, pillars: Pillar[], innerHtml: string) {
    if (x >= this.clientWidth - this.litChartColumnTipEL!.clientWidth) {
      this.showTip(
        x - this.litChartColumnTipEL!.clientWidth - 10,
        y - 20,
        this.litChartColumnCfg!.tip ? this.litChartColumnCfg!.tip(pillars) : innerHtml
      );
    } else {
      this.showTip(x + 10, y - 20, this.litChartColumnCfg!.tip ? this.litChartColumnCfg!.tip(pillars) : innerHtml);
    }
  }

  showHoverColumn(index: number) {
    this.data.forEach((it) => {
      if (it.obj.no === index) {
        it.hover = true;
      } else {
        it.hover = false;
      }
    });
    let pillars = this.data.filter((it) => it.hover);
    if (this.litChartColumnCfg?.seriesField) {
      if (pillars.length > 0) {
        let hoverData = pillars[0];
        let title = `<label>${this.litChartColumnCfg.xField}: ${pillars[0].xLabel}</label>`;
        let msg = pillars.map((it) => `<label>${it.type}: ${it.yLabel}</label>`).join('');
        let sum = `<label>Total: ${pillars
          .map((it) => it.obj[this.litChartColumnCfg?.yField!])
          .reduce((pre, current) => pre + current, 0)}</label>`;
        let innerHtml = `<div class="tip-content">${title}${msg}${sum}</div>`;
        this.showTip(
          this.clientWidth / 2,
          this.clientHeight / 2,
          this.litChartColumnCfg!.tip ? this.litChartColumnCfg!.tip(pillars) : innerHtml
        );
      }
    } else {
      if (pillars.length > 0) {
        let hoverData = pillars[0];
        let title = `<label>${pillars[0].xLabel}:${pillars[0].yLabel}</label>`;
        let innerHtml = `<div class="tip-content">${title}</div>`;
        this.showTip(
          this.clientWidth / 2,
          this.clientHeight / 2,
          this.litChartColumnCfg!.tip ? this.litChartColumnCfg!.tip(pillars) : innerHtml
        );
      }
    }

    if (this.data.filter((it) => it.process).length == 0) {
      this.render();
    }
  }

  initElements(): void {
    new ResizeObserver((entries, observer) => {
      entries.forEach((it) => {
        resizeCanvas(this.litChartColumnCanvas!);
        this.measure();
        this.render(false);
      });
    }).observe(this);
  }

  set config(litChartColumnConfig: LitChartColumnConfig | null | undefined) {
    if (!litChartColumnConfig) return;
    this.litChartColumnCfg = litChartColumnConfig;
    this.measure();
    this.render();
  }

  set dataSource(litChartColumnArr: any[]) {
    if (this.litChartColumnCfg) {
      this.litChartColumnCfg.data = litChartColumnArr;
      this.measure();
      this.render();
    }
  }

  get dataSource() {
    return this.litChartColumnCfg?.data || [];
  }

  measure() {
    if (!this.litChartColumnCfg) return;
    this.data = [];
    this.rowLines = [];
    if (!this.litChartColumnCfg.seriesField) {
      let maxValue = Math.max(...this.litChartColumnCfg.data.map((it) => it[this.litChartColumnCfg!.yField]));
      maxValue = Math.ceil(maxValue * 0.1) * 10;
      let partWidth = (this.clientWidth - this.offset!.x!) / this.litChartColumnCfg.data.length;
      let partHeight = this.clientHeight - this.offset!.y!;
      let gap = partHeight / 5;
      let valGap = maxValue / 5;
      for (let i = 0; i <= 5; i++) {
        this.rowLines.push({
          y: gap * i,
          label: `${getProbablyTime(maxValue - valGap * i)}`,
        });
      }
      this.litChartColumnCfg?.data
        .sort((a, b) => b[this.litChartColumnCfg!.yField] - a[this.litChartColumnCfg!.yField])
        .forEach((litChartColumnItem, litChartColumnIndex, array) => {
          this.data.push({
            color: this.litChartColumnCfg!.color(litChartColumnItem),
            obj: litChartColumnItem,
            root: true,
            xLabel: litChartColumnItem[this.litChartColumnCfg!.xField],
            yLabel: litChartColumnItem[this.litChartColumnCfg!.yField],
            bgFrame: {
              x: this.offset!.x! + partWidth * litChartColumnIndex,
              y: 0,
              w: partWidth,
              h: partHeight,
            },
            centerX: this.offset!.x! + partWidth * litChartColumnIndex + partWidth / 2,
            centerY:
              partHeight -
              (litChartColumnItem[this.litChartColumnCfg!.yField] * partHeight) / maxValue +
              (litChartColumnItem[this.litChartColumnCfg!.yField] * partHeight) / maxValue / 2,
            frame: {
              x: this.offset!.x! + partWidth * litChartColumnIndex + partWidth / 6,
              y: partHeight - (litChartColumnItem[this.litChartColumnCfg!.yField] * partHeight) / maxValue,
              w: partWidth - partWidth / 3,
              h: (litChartColumnItem[this.litChartColumnCfg!.yField] * partHeight) / maxValue,
            },
            height: 0,
            heightStep: Math.ceil((litChartColumnItem[this.litChartColumnCfg!.yField] * partHeight) / maxValue / 60),
            process: true,
          });
        });
    } else {
      let reduceGroup = this.litChartColumnCfg.data.reduce((pre, current, index, arr) => {
        (pre[current[this.litChartColumnCfg!.xField]] = pre[current[this.litChartColumnCfg!.xField]] || []).push(
          current
        );
        return pre;
      }, {});
      let sums = Reflect.ownKeys(reduceGroup).map((k) =>
        (reduceGroup[k] as any[]).reduce((pre, current) => pre + current[this.litChartColumnCfg!.yField], 0)
      );
      let maxValue = Math.ceil(Math.max(...sums) * 0.1) * 10;
      let partWidth = (this.clientWidth - this.offset!.x!) / Reflect.ownKeys(reduceGroup).length;
      let partHeight = this.clientHeight - this.offset!.y!;
      let gap = partHeight / 5;
      let valGap = maxValue / 5;
      for (let index = 0; index <= 5; index++) {
        this.rowLines.push({
          y: gap * index,
          label: `${getProbablyTime(maxValue - valGap * index)} `,
        });
      }
      Reflect.ownKeys(reduceGroup)
        .sort(
          (b, a) =>
            (reduceGroup[a] as any[]).reduce((pre, cur) => pre + (cur[this.litChartColumnCfg!.yField] as number), 0) -
            (reduceGroup[b] as any[]).reduce((pre, cur) => pre + (cur[this.litChartColumnCfg!.yField] as number), 0)
        )
        .forEach((reduceGroupKey, reduceGroupIndex) => {
          let elements = reduceGroup[reduceGroupKey];
          let initH = 0;
          elements.forEach((itemEl: any, y: number) => {
            this.data.push({
              color: this.litChartColumnCfg!.color(itemEl),
              obj: itemEl,
              root: y == 0,
              type: itemEl[this.litChartColumnCfg!.seriesField],
              xLabel: itemEl[this.litChartColumnCfg!.xField],
              yLabel: itemEl[this.litChartColumnCfg!.yField],
              bgFrame: {
                x: this.offset!.x! + partWidth * reduceGroupIndex,
                y: 0,
                w: partWidth,
                h: partHeight,
              },
              centerX: this.offset!.x! + partWidth * reduceGroupIndex + partWidth / 2,
              centerY:
                partHeight -
                initH -
                (itemEl[this.litChartColumnCfg!.yField] * partHeight) / maxValue +
                (itemEl[this.litChartColumnCfg!.yField] * partHeight) / maxValue / 2,
              frame: {
                x: this.offset!.x! + partWidth * reduceGroupIndex + partWidth / 6,
                y: partHeight - (itemEl[this.litChartColumnCfg!.yField] * partHeight) / maxValue - initH,
                w: partWidth - partWidth / 3,
                h: (itemEl[this.litChartColumnCfg!.yField] * partHeight) / maxValue,
              },
              height: 0,
              heightStep: Math.ceil((itemEl[this.litChartColumnCfg!.yField] * partHeight) / maxValue / 60),
              process: true,
            });
            initH += (itemEl[this.litChartColumnCfg!.yField] * partHeight) / maxValue;
          });
        });
    }
  }

  get config(): LitChartColumnConfig | null | undefined {
    return this.litChartColumnCfg;
  }

  render(ease: boolean = true) {
    if (!this.litChartColumnCanvas || !this.litChartColumnCfg) return;
    this.litChartColumnCtx!.clearRect(0, 0, this.clientWidth, this.clientHeight);
    this.drawLine(this.litChartColumnCtx!);
    this.data?.forEach((it) => this.drawColumn(this.litChartColumnCtx!, it, ease));
    if (ease) {
      if (this.data.filter((it) => it.process).length > 0) {
        requestAnimationFrame(() => this.render(ease));
      }
    }
  }

  drawLine(c: CanvasRenderingContext2D) {
    c.strokeStyle = '#dfdfdf';
    c.lineWidth = 1;
    c.beginPath();
    c.fillStyle = '#8c8c8c';
    this.rowLines.forEach((it, i) => {
      c.moveTo(this.offset!.x!, it.y);
      c.lineTo(this.clientWidth, it.y);
      if (i == 0) {
        c.fillText(it.label, this.offset!.x! - c.measureText(it.label).width - 2, it.y + 11);
      } else {
        c.fillText(it.label, this.offset!.x! - c.measureText(it.label).width - 2, it.y + 4);
      }
    });
    c.stroke();
    c.closePath();
  }

  drawColumn(c: CanvasRenderingContext2D, it: Pillar, ease: boolean) {
    if (it.hover) {
      c.globalAlpha = 0.2;
      c.fillStyle = '#999999';
      c.fillRect(it.bgFrame!.x, it.bgFrame!.y, it.bgFrame!.w, it.bgFrame!.h);
      c.globalAlpha = 1.0;
    }
    c.fillStyle = it.color || '#ff0000';
    if (ease) {
      if (it.height! < it.frame!.h) {
        it.process = true;
        c.fillRect(it.frame!.x, it.frame!.y + (it.frame!.h - it.height!), it.frame!.w, it.height!);
        it.height! += it.heightStep!;
      } else {
        c.fillRect(it.frame!.x, it.frame!.y, it.frame!.w, it.frame!.h);
        it.process = false;
      }
    } else {
      c.fillRect(it.frame!.x, it.frame!.y, it.frame!.w, it.frame!.h);
      it.process = false;
    }

    c.beginPath();
    c.strokeStyle = '#d8d8d8';
    c.moveTo(it.centerX!, it.frame!.y + it.frame!.h!);
    if (it.root) {
      c.lineTo(it.centerX!, it.frame!.y + it.frame!.h + 4);
    }
    let xMetrics = c.measureText(it.xLabel!);
    let xMetricsH = xMetrics.actualBoundingBoxAscent + xMetrics.actualBoundingBoxDescent;
    let yMetrics = c.measureText(it.yLabel!);
    let yMetricsH = yMetrics.fontBoundingBoxAscent + yMetrics.fontBoundingBoxDescent;
    c.fillStyle = '#8c8c8c';
    if (it.root) {
      c.fillText(it.xLabel!, it.centerX! - xMetrics.width / 2, it.frame!.y + it.frame!.h + 15);
    }
    c.fillStyle = '#fff';
    if (this.litChartColumnCfg?.label) {
      if (yMetricsH < it.frame!.h) {
        c.fillText(
          // @ts-ignore
          this.litChartColumnCfg!.label!.content ? this.litChartColumnCfg!.label!.content(it.obj) : it.yLabel!,
          it.centerX! - yMetrics.width / 2,
          it.centerY! + (it.frame!.h - it.height!) / 2
        );
      }
    }
    c.stroke();
    c.closePath();
  }

  beginPath(stroke: boolean, fill: boolean) {
    return (fn: (c: CanvasRenderingContext2D) => void) => {
      this.litChartColumnCtx!.beginPath();
      fn?.(this.litChartColumnCtx!);
      if (stroke) {
        this.litChartColumnCtx!.stroke();
      }
      if (fill) {
        this.litChartColumnCtx!.fill();
      }
      this.litChartColumnCtx!.closePath();
    };
  }

  showTip(x: number, y: number, msg: string) {
    this.litChartColumnTipEL!.style.display = 'flex';
    this.litChartColumnTipEL!.style.top = `${y}px`;
    this.litChartColumnTipEL!.style.left = `${x}px`;
    this.litChartColumnTipEL!.innerHTML = msg;
  }

  hideTip() {
    this.litChartColumnTipEL!.style.display = 'none';
  }

  initHtml(): string {
    return `
        <style>   
        :host {
            display: flex;
            flex-direction: column;
            width: 100%;
            height: 100%;
        }
        #tip{
            background-color: #f5f5f4;
            border: 1px solid #fff;
            border-radius: 5px;
            color: #333322;
            font-size: 8pt;
            position: absolute;
            min-width: max-content;
            display: none;
            top: 0;
            left: 0;
            pointer-events: none;
            user-select: none;
            padding: 5px 10px;
            box-shadow: 0 0 10px #22ffffff;
            /*transition: left;*/
            /*transition-duration: 0.3s;*/
        }
        #root{
            position:relative;
        }
        .tip-content{
            display: flex;
            flex-direction: column;
        }
        </style>
        <div id="root">
            <canvas id="canvas"></canvas>
            <div id="tip"></div>
        </div>`;
  }
}

function contains(rect: { x: number; y: number; w: number; h: number }, x: number, y: number): boolean {
  return rect.x <= x && x <= rect.x + rect.w && rect.y <= y && y <= rect.y + rect.h;
}
