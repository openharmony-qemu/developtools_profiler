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

import { BaseElement, element } from '../../../base-ui/BaseElement.js';
import { HdcDeviceManager } from '../../../hdc/HdcDeviceManager.js';
import { SpRecordTrace } from '../SpRecordTrace.js';
import { DataMessage } from '../../../hdc/message/DataMessage.js';

@element('sp-web-hdc-shell')
export class SpWebHdcShell extends BaseElement {
  private static MAX_DISPLAY_ROWS = 1000;
  private static MAX_SAVE_SIZE = 2097152;
  private shellDiv: HTMLDivElement | null | undefined;
  private shellCanvas: HTMLCanvasElement | null | undefined;
  private shellCanvasCtx: CanvasRenderingContext2D | null | undefined;
  private resultStr = '';
  private sendCallBack: ((keyboardEvent: KeyboardEvent | string) => void | undefined) | undefined;
  private startShellDevice = '';
  private intervalId: number | undefined;
  private skipFlag: number[] = [7];
  private clearFlag: number[] = [27, 91, 50, 74, 27, 91, 72];
  private CRLFFlag: number[] = [13, 13, 10];
  private startRealTimeFlag: number[] = [27, 91, 115];
  private endRealTimeFlag: number[] = [27, 91, 117];
  private clearRealTimeFlag: number[] = [27, 91, 72, 27, 91, 74];
  private ctrlCFlag: number[] = [13, 10, 35, 32];
  private points: Point | undefined;
  private forwardFlag: boolean = false;
  private cursorIndex: number = 3;
  private cursorRow: string = '';
  private textDecoder: TextDecoder = new TextDecoder();
  private isDragging: boolean = false;
  private static TOP_OFFSET = 48;
  private static FIRST_ROW_OFFSET = 32;
  private static LAST_ROW_OFFSET = 40;
  private static MULTI_LINE_FLAG = '<\b';
  private static LINE_BREAK_LENGTH = 2;
  private static LEFT_OFFSET = 48;
  private realTimeResult: string | null | undefined = '';
  private startRealTime: boolean = false;

  public initElements(): void {
    this.shellCanvas = this.shadowRoot!.querySelector<HTMLCanvasElement>('#shell_cmd');
    this.shellCanvasCtx = this.shellCanvas!.getContext('2d');
    this.shellCanvasCtx!.fillStyle = '#000';

    this.shellCanvasCtx!.fillRect(0, 0, this.shellCanvas!.width, this.shellCanvas!.height);
    this.shellDiv = this.shadowRoot!.querySelector<HTMLDivElement>('.shell_cmd_div');
    this.shellCanvasAddMouseListener();
    this.shellCanvas!.addEventListener('contextmenu', (event) => {
      event.preventDefault();
      event.stopPropagation();
    });
    let listenerThis = this;
    this.shellCanvas!.addEventListener('keydown', async (keyboardEvent) => {
      keyboardEvent.preventDefault();
      if (keyboardEvent.ctrlKey && keyboardEvent.code === 'KeyC' && listenerThis.points) {
        let rowText: string = listenerThis.getSelectedText();
        listenerThis.points = undefined;
        await navigator.clipboard.writeText(rowText);
      } else {
        if (this.sendCallBack) {
          this.sendCallBack(keyboardEvent);
        }
      }
    });
    window.subscribe(window.SmartEvent.UI.DeviceConnect, (deviceName: string) => {
      if (deviceName) {
        this.hdcShellFocus();
      }
    });
    window.subscribe(window.SmartEvent.UI.DeviceDisConnect, (deviceName: string) => {
      this.clear();
    });
    new ResizeObserver(() => {
      this.resizeCanvas();
      this.refreshShellPage(true);
    }).observe(this);
  }

  resizeCanvas(): void {
    if (this.shellCanvas !== null && this.shellCanvas !== undefined) {
      this.shellCanvas.width = this.shellCanvas.clientWidth;
      this.shellCanvas.height = this.shellCanvas.clientHeight;
    }
  }

  clear(): void {
    this.sendCallBack = undefined;
    this.resultStr = '';
    this.cursorRow = '';
    this.shellCanvasCtx!.clearRect(0, 0, this.shellCanvas!.width, this.shellCanvas!.height);
    this.shellCanvasCtx!.fillStyle = '#000';
    this.shellCanvasCtx!.fillRect(0, 0, this.shellCanvas!.width, this.shellCanvas!.height);
    window.clearInterval(this.intervalId);
  }

  public hdcShellFocus(): void {
    HdcDeviceManager.connect(SpRecordTrace.serialNumber).then((connected) => {
      if (connected) {
        if (this.sendCallBack && this.startShellDevice === SpRecordTrace.serialNumber) {
          this.shellCanvas!.focus();
          this.refreshShellPage(true);
        } else {
          this.clear();
          this.sendCallBack = HdcDeviceManager.startShell((result: DataMessage) => {
            if (result.channelClose) {
              this.clear();
              return;
            }
            this.startShellDevice = SpRecordTrace.serialNumber;
            this.handleHdcRecvData(result);
          });
          this.shellCanvas!.focus();
          this.refreshShellPage(true);
        }
      } else {
        this.clear();
      }
    });
  }

  arrayBufferCompare(compareA: ArrayBuffer, compareB: number[]): boolean {
    const arrayA = new Uint8Array(compareA);
    if (arrayA.length === compareB.length) {
      for (let i = 0; i < arrayA.length; i++) {
        const dd = arrayA[i];
        if (dd !== compareB[i]) {
          return false;
        }
      }
      return true;
    }
    return false;
  }

  getSelectedText(): string {
    let selectedText = '';
    let textLines = [...this.finalArr];
    let startX = this.points!.startX!;
    let startY = this.points!.startY!;
    let endX = this.points!.endX!;
    let endY = this.points!.endY!;
    let depth = Math.ceil((endY - startY) / 16);
    let index = 0;
    for (let i = 0; i < textLines.length; i++) {
      let line = textLines[i];
      let x = SpWebHdcShell.LEFT_OFFSET;
      let textFirstRowY = 16 * i + SpWebHdcShell.FIRST_ROW_OFFSET;
      let textLastRowY = 16 * i + SpWebHdcShell.LAST_ROW_OFFSET;
      let textEndY = 16 * i + SpWebHdcShell.TOP_OFFSET;
      let w = this.shellCanvasCtx!.measureText(line).width;
      if (
        (startY < textEndY && endY >= textEndY) ||
        (startY > textFirstRowY && startY < textEndY) ||
        (endY > textLastRowY && endY < textEndY)
      ) {
        index++;
        if (index === 1) {
          if (depth > 1) {
            selectedText +=
              line.slice(this.getCurrentLineBackSize(line, startX - x, true)) + (endX < x + w ? '\n' : '');
          } else {
            selectedText += `${line.slice(
              this.getCurrentLineBackSize(line, startX - x, true),
              this.getCurrentLineBackSize(line, endX - x, false)
            )}\n`;
          }
        } else if (index === depth) {
          selectedText += `${line.slice(0, this.getCurrentLineBackSize(line, endX - x, false))}\n`;
        } else {
          selectedText += `${line}\n`;
        }
      }
    }
    return selectedText.trim();
  }

  forwardSelected(startX: number, startY: number, endX: number, endY: number): void {
    //左边界x为SpWebHdcShell.LEFT_OFFSET，右边界为this.shellCanvas!.width
    let depth = Math.ceil((endY - startY) / 16);
    let startPointX = 0;
    let startPointY = 0;
    let endPointX = 0;
    let endPointY = 0;
    if (depth <= 1) {
      this.shellCanvasCtx!.fillRect(startX, startY, endX - startX, endY - startY);
      startPointX = startX;
      startPointY = startY;
      endPointX = endX;
      endPointY = endY;
    } else {
      //绘制多行
      for (let index = 1; index <= depth; index++) {
        //第一行，绘起始点到canvas右边界矩形
        if (index === 1) {
          this.shellCanvasCtx!.fillRect(startX, startY, this.shellCanvas!.width - startX, index * 16);
          startPointX = startX;
          startPointY = startY;
        } else if (index === depth) {
          //最后一行，canvas左边界到结束点矩形
          this.shellCanvasCtx!.fillRect(
            SpWebHdcShell.LEFT_OFFSET,
            startY + (index - 1) * 16,
            endX - SpWebHdcShell.LEFT_OFFSET,
            endY - (startY + (index - 1) * 16)
          );
          endPointX = endX;
          endPointY = endY;
        } else {
          //中间行，canvas的左边界到右边界的矩形
          this.shellCanvasCtx!.fillRect(
            SpWebHdcShell.LEFT_OFFSET,
            startY + (index - 1) * 16,
            this.shellCanvas!.width,
            16
          );
        }
      }
    }
    this.points = { startX: startPointX, startY: startPointY, endX: endPointX, endY: endPointY };
  }

  getCurrentLineBackSize(currentLine: string, maxBackSize: number, isStart: boolean): number {
    let fillText = '';
    let strings = currentLine.split('');
    for (let index = 0; index < strings.length; index++) {
      let text = strings[index];
      if (
        this.shellCanvasCtx!.measureText(fillText).width < maxBackSize &&
        this.shellCanvasCtx!.measureText(fillText + text).width >= maxBackSize
      ) {
        if (!isStart) {
          fillText += text;
        }
        break;
      }
      fillText += text;
    }
    return fillText.length;
  }

  reverseSelected(startX: number, startY: number, endX: number, endY: number): void {
    //左边界x为SpWebHdcShell.LEFT_OFFSET，右边界为this.shellCanvas!.width
    let depth = Math.ceil((startY - endY) / 16);
    let startPointX = 0;
    let startPointY = 0;
    let endPointX = 0;
    let endPointY = 0;
    if (depth <= 1) {
      this.shellCanvasCtx!.fillRect(endX, endY, startX - endX, startY - endY);
      startPointX = endX;
      startPointY = endY;
      endPointX = startX;
      endPointY = startY;
    } else {
      //绘制多行
      for (let index = 1; index <= depth; index++) {
        //第一行，绘起始点到canvas左边界矩形
        if (index === 1) {
          this.shellCanvasCtx!.fillRect(SpWebHdcShell.LEFT_OFFSET, startY - 16, startX - SpWebHdcShell.LEFT_OFFSET, 16);
          endPointX = startX;
          endPointY = startY;
        } else if (index === depth) {
          //最后一行，canvas右边界到结束点矩形
          this.shellCanvasCtx!.fillRect(endX, endY, this.shellCanvas!.width - endX, startY - (index - 1) * 16 - endY);
          startPointX = endX;
          startPointY = endY;
        } else {
          this.shellCanvasCtx!.fillRect(SpWebHdcShell.LEFT_OFFSET, startY - index * 16, this.shellCanvas!.width, 16);
          this.shellCanvasCtx!.textBaseline = 'middle';
        }
      }
    }
    this.points = { startX: startPointX, startY: startPointY, endX: endPointX, endY: endPointY };
  }

  private singleLineToMultiLine(shellStr: string, foundationWidth: number, maxWidth: number): string[] {
    let result = [];
    while (shellStr.length * foundationWidth > maxWidth) {
      let bfb = maxWidth / (shellStr.length * foundationWidth);
      let cutIndex = Math.floor(shellStr.length * bfb);
      let ss = shellStr.substring(0, cutIndex);
      result.push(ss);
      shellStr = shellStr.substring(cutIndex);
    }
    if (shellStr.length > 0) {
      result.push(shellStr);
    }
    return result;
  }

  private finalArr: Array<string> = [];

  refreshShellPage(scroller: boolean): void {
    try {
      if (this.resultStr.length === 0 && this.cursorRow.length === 0) {
        return;
      }
      this.shellCanvasCtx!.clearRect(0, 0, this.shellCanvas!.width, this.shellCanvas!.height);
      this.shellCanvasCtx!.fillStyle = '#000';
      this.shellCanvasCtx!.fillRect(0, 0, this.shellCanvas!.width, this.shellCanvas!.height);
      let resultStrArr = this.resultStr.split('\r\n');
      if (this.realTimeResult !== '') {
        resultStrArr = (this.resultStr + this.realTimeResult).split('\r\n');
      }
      this.finalArr = [];
      if (this.shellCanvas!.width > 0) {
        let maxWidth = this.shellCanvas!.width;
        let foundationWidth = Math.ceil(this.shellCanvasCtx!.measureText(' ').width);
        for (let i = 0; i < resultStrArr.length - 1; i++) {
          let shellStr = resultStrArr[i];
          let strWidth = this.shellCanvasCtx!.measureText(shellStr).width;
          if (strWidth > maxWidth) {
            let lines = this.singleLineToMultiLine(shellStr, foundationWidth, maxWidth - SpWebHdcShell.LEFT_OFFSET);
            this.finalArr.push(...lines);
          } else {
            this.finalArr.push(shellStr);
          }
        }
        let shellStrLength = 0;
        if (this.finalArr.length > SpWebHdcShell.MAX_DISPLAY_ROWS) {
          this.finalArr.splice(0, this.finalArr.length - SpWebHdcShell.MAX_DISPLAY_ROWS + 1);
        }
        let unitWidth: number = this.shellCanvasCtx!.measureText(' ').width;
        this.shellCanvasCtx!.fillStyle = '#fff';
        this.shellCanvasCtx!.font = '16px serif';
        let textY = SpWebHdcShell.TOP_OFFSET;
        this.finalArr.push(this.cursorRow);
        for (let index: number = 0; index < this.finalArr.length; index++) {
          let shellStr: string = this.finalArr[index];
          textY = SpWebHdcShell.TOP_OFFSET + index * 16;
          this.shellCanvasCtx!.fillText(shellStr, SpWebHdcShell.LEFT_OFFSET, textY);
        }
        shellStrLength =
          this.shellCanvasCtx!.measureText(this.cursorRow.slice(0, this.cursorIndex)).width + SpWebHdcShell.LEFT_OFFSET;
        if (scroller) {
          if (textY > this.shellDiv!.clientHeight) {
            this.shellDiv!.scrollTop = textY - this.shellDiv!.clientHeight + 3;
          }
        }
        if (this.intervalId) {
          window.clearInterval(this.intervalId);
        }
        let needClear = false;
        this.intervalId = window.setInterval(() => {
          if (needClear) {
            needClear = false;
            this.shellCanvasCtx!.fillStyle = '#000';
            this.shellCanvasCtx!.fillRect(shellStrLength, textY, 12, 3);
          } else {
            needClear = true;
            this.shellCanvasCtx!.fillStyle = '#fff';
            this.shellCanvasCtx!.fillRect(shellStrLength, textY, 12, 3);
          }
        }, 500);
      }
    } catch (e) {}
  }

  public initHtml(): string {
    return `
    <style>
    :host{
        display: block;
        border-radius: 0 16px 16px 0;
        width: 100%;
        position: relative;
    }
    .shell_cmd_div {
        width: 90%;
        margin-left: 5%;
        margin-top: 3%;
        overflow-y: scroll;
        height: 40rem;
        background: #000;
        border: 1px solid var(--dark-color1,#4D4D4D);
        border-radius: 16px;
    }
     ::-webkit-scrollbar{
       width: 13px;
       height: 10px;
       background-color: #FFFFFF;
     }
     ::-webkit-scrollbar-track{
       border-top-right-radius: 16px;
       border-bottom-right-radius: 16px;
       background-color: #000000;
     }
     ::-webkit-scrollbar-thumb{
       background: #5A5A5A;
       border-radius: 6px;
     }
    canvas {
         display: inline-block;
         outline: none;
    }

    </style>
    <div class="shell_cmd_div">
        <canvas id="shell_cmd" style="width: 100%;height:${16000 + SpWebHdcShell.TOP_OFFSET}px;" tabindex="0"></canvas>
    </div>
    `;
  }

  private refreshCurrentRow(): void {
    let lastRow: string = this.resultStr;
    if (this.resultStr.lastIndexOf('\r\n') !== -1) {
      lastRow = this.resultStr.substring(this.resultStr.lastIndexOf('\r\n') + 2);
    }
    let currentRow: string[] = [...lastRow];
    let result: string[] = [];
    this.cursorIndex = 0;
    for (let index: number = 0; index < currentRow.length; index++) {
      let currentResult: string = currentRow[index];
      if (currentResult === '\b') {
        this.cursorIndex--;
      } else {
        result[this.cursorIndex] = currentResult;
        this.cursorIndex++;
      }
    }
    this.cursorRow = result.join('');
  }

  private handleHdcRecvData(result: DataMessage): void {
    const resData = result.getData();
    if (resData) {
      if (this.arrayBufferCompare(resData, this.skipFlag)) {
        return;
      } else if (this.arrayBufferCompare(resData, this.clearFlag)) {
        this.resultStr = '';
        this.shellDiv!.scrollTop = 0;
        this.cursorIndex = 3;
      } else if (this.arrayBufferCompare(resData, this.CRLFFlag)) {
        if (this.resultStr.lastIndexOf('\r\n') !== -1) {
          this.resultStr = this.resultStr.substring(0, this.resultStr.lastIndexOf('\r\n') + 2) + this.cursorRow;
        } else {
          this.resultStr = this.cursorRow;
        }
        this.resultStr += result.getDataToString();
        this.cursorIndex = 3;
      } else {
        if (this.resultStr.length > SpWebHdcShell.MAX_SAVE_SIZE) {
          this.resultStr = this.resultStr.substring(this.resultStr.length / 2);
        }
        const arrayA = new Uint8Array(resData);
        if (arrayA[0] === 13 && arrayA[1] !== 10 && arrayA[1] !== 13) {
          const index = this.resultStr.lastIndexOf('\n');
          const resultStrLength = this.resultStr.length;
          if (index > -1 && resultStrLength > index) {
            this.resultStr =
              this.resultStr.substring(0, index + 1) + this.textDecoder.decode(arrayA.slice(1, arrayA.length));
          } else {
            if (this.resultStr.split('\n').length === 1) {
              const index = this.cursorRow.lastIndexOf('\n');
              this.cursorRow =
                this.cursorRow.substring(0, index + 1) + this.textDecoder.decode(arrayA.slice(1, arrayA.length));
              this.resultStr = this.cursorRow;
            } else {
              this.resultStr += result.getDataToString();
            }
          }
          this.realTimeResult = '';
        } else if (this.isStartWidthArrayBuffer(arrayA, this.startRealTimeFlag)) {
          let lastIndex = this.getLastRestorationIndex(arrayA, this.endRealTimeFlag);
          this.realTimeResult = this.removeTextAndColorSequenceStr(
            this.textDecoder.decode(arrayA.slice(lastIndex, arrayA.length))
          );
          this.startRealTime = true;
        } else if (this.isStartWidthArrayBuffer(arrayA, this.clearRealTimeFlag)) {
          this.realTimeResult = this.removeTextAndColorSequenceStr(
            this.textDecoder.decode(arrayA.slice(6, arrayA.length))
          );
          this.startRealTime = true;
        } else {
          if (this.isStartWidthArrayBuffer(arrayA, this.ctrlCFlag)) {
            this.resultStr += this.realTimeResult;
            this.startRealTime = false;
          }
          if (this.startRealTime) {
            if (result.getDataToString().includes(SpWebHdcShell.MULTI_LINE_FLAG)) {
              this.realTimeResult += result.getDataToString().substring(result.getDataToString().indexOf('\r'));
            } else {
              this.realTimeResult += result.getDataToString();
            }
            this.realTimeResult = this.removeTextAndColorSequenceStr(this.realTimeResult!);
          } else {
            this.realTimeResult = '';
            if (result.getDataToString().includes(SpWebHdcShell.MULTI_LINE_FLAG)) {
              // 获取所有内容，不包括最后一行数据
              this.resultStr = this.resultStr.substring(
                0,
                this.resultStr.lastIndexOf('\r\n') + SpWebHdcShell.LINE_BREAK_LENGTH
              );
              // 多行情况不能直接拼接返回数据
              this.resultStr += result.getDataToString().substring(result.getDataToString().indexOf('\r'));
            } else {
              this.resultStr += result.getDataToString();
            }
          }
        }
      }
      this.resultStr = this.removeTextAndColorSequenceStr(this.resultStr);
      this.refreshCurrentRow();
      this.refreshShellPage(true);
    }
  }

  private removeTextAndColorSequenceStr(currentStr: string): string {
    return currentStr.replace(new RegExp(/\x1B\[[0-9;]*[a-zA-Z]/g), '');
  }

  private isStartWidthArrayBuffer(sourceArray: Uint8Array, compareArray: number[]): boolean {
    for (let index = 0; index < compareArray.length; index++) {
      if (sourceArray[index] !== compareArray[index]) {
        return false;
      }
    }
    return true;
  }

  private getLastRestorationIndex(sourceArray: Uint8Array, compareArray: number[]): number {
    let lastIndex = -1;
    for (let index = sourceArray.length - 1; index >= 0; index--) {
      if (sourceArray[index] === compareArray[0]) {
        let isLast = true;
        for (let j = 1; j < compareArray.length; j++) {
          if (sourceArray[index + j] !== compareArray[j]) {
            isLast = false;
            break;
          }
        }
        if (isLast) {
          lastIndex = index;
          break;
        }
      }
    }
    return lastIndex + compareArray.length;
  }

  private shellCanvasAddMouseListener(): void {
    let startX: number;
    let startY: number;
    let endX: number;
    let endY: number;
    let that = this;
    this.shellCanvas!.addEventListener('mousedown', function (event) {
      if (that.resultStr.length === 0 && that.cursorRow.length === 0) {
        return;
      }
      that.isDragging = true;
      startX = event.offsetX;
      startY = event.offsetY;
      that.refreshShellPage(false);
    });
    this.shellCanvas!.addEventListener('mousemove', function (event) {
      if (!that.isDragging) {
        return;
      }
      if (that.resultStr.length === 0 && that.cursorRow.length === 0) {
        return;
      }
      endX = event.offsetX;
      endY = event.offsetY;
      that.refreshShellPage(false);
      that.points = undefined;
      that.shellCanvasCtx!.fillStyle = 'rgba(128, 128, 128, 0.5)';
      if (endY > startY) {
        that.forwardFlag = true;
        that.forwardSelected(startX, startY, endX, endY);
      } else {
        that.forwardFlag = false;
        that.reverseSelected(startX, startY, endX, endY);
      }
    });
    this.shellCanvasAddMouseUpListener();
  }

  private shellCanvasAddMouseUpListener(): void {
    let that = this;
    this.shellCanvas!.addEventListener('mouseup', async function (event) {
      if (!that.isDragging) {
        return;
      }
      if (that.resultStr.length === 0 && that.cursorRow.length === 0) {
        return;
      }
      that.isDragging = false;
      //右键
      if (event.button === 2) {
        let text: string = await navigator.clipboard.readText();
        if (text) {
          if (that.sendCallBack) {
            that.sendCallBack(text);
          }
          return;
        }
      }
    });
  }
}

export class Point {
  startX: number | undefined;
  startY: number | undefined;
  endX: number | undefined;
  endY: number | undefined;
}
