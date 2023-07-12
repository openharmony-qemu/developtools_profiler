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
import { TraceRow } from '../../../../../dist/trace/component/trace/base/TraceRow.js';
// @ts-ignore
import { Sptext } from '../../../../../dist/trace/component/Sptext.js';

describe('TraceRow Test', () => {
  beforeAll(() => {});
  const ctx = {
    lineWidth: 1,
    strokeStyle: true,
  };
  it('TraceRow Test01', () => {
    let traceRow = new TraceRow<any>({
      canvasNumber: 1,
      alpha: true,
      contextId: '2d',
      isOffScreen: true,
    });
    expect(traceRow).not.toBeUndefined();
  });

  it('TraceRow Test02', () => {
    let traceRow = new TraceRow<any>({
      canvasNumber: 1,
      alpha: true,
      contextId: '2d',
      isOffScreen: true,
    });
    expect(traceRow.sleeping).toBeFalsy();
  });

  it('TraceRow Test03', () => {
    let traceRow = new TraceRow<any>({
      canvasNumber: 1,
      alpha: true,
      contextId: '2d',
      isOffScreen: true,
    });
    traceRow.sleeping = true;
    expect(traceRow.sleeping).toBeTruthy();
  });

  it('TraceRow Test04', () => {
    let traceRow = new TraceRow<any>({
      canvasNumber: 1,
      alpha: true,
      contextId: '2d',
      isOffScreen: true,
    });
    traceRow.sleeping = false;
    expect(traceRow.sleeping).toBeFalsy();
  });

  it('TraceRow Test05', () => {
    let traceRow = new TraceRow<any>({
      canvasNumber: 1,
      alpha: true,
      contextId: '2d',
      isOffScreen: true,
    });
    expect(traceRow.rangeSelect).toBeFalsy();
  });

  it('TraceRow Test06', () => {
    let traceRow = new TraceRow<any>({
      canvasNumber: 1,
      alpha: true,
      contextId: '2d',
      isOffScreen: true,
    });
    traceRow.rangeSelect = true;
    expect(traceRow.rangeSelect).toBeTruthy();
  });

  it('TraceRow Test10', () => {
    const canvas = document.createElement('canvas');
    canvas.width = 1;
    canvas.height = 1;
    const ctx = canvas.getContext('2d');
    let traceRow = new TraceRow<any>({
      canvasNumber: 1,
      alpha: true,
      contextId: '2d',
      isOffScreen: true,
    });
    traceRow.dataList = {
      supplier: true,
      isLoading: false,
    };
    traceRow.supplier = true;
    traceRow.isLoading = false;
    traceRow.name = '111';
    traceRow.height = 20;
    traceRow.height = 30;
    expect(traceRow.clearCanvas(ctx)).toBeUndefined();
  });

  it('TraceRow Test11', () => {
    const canvas = document.createElement('canvas');
    canvas.width = 1;
    canvas.height = 1;
    const ctx = canvas.getContext('2d');
    let traceRow = new TraceRow<any>({
      canvasNumber: 1,
      alpha: true,
      contextId: '2d',
      isOffScreen: true,
    });
    traceRow.dataList = {
      supplier: true,
      isLoading: false,
    };
    traceRow.supplier = true;
    traceRow.isLoading = false;
    traceRow.name = '111';
    traceRow.height = 20;
    traceRow.height = 30;
    expect(traceRow.drawLines(ctx)).toBeUndefined();
  });

  it('TraceRow Test12', () => {
    const canvas = document.createElement('canvas');
    canvas.width = 1;
    canvas.height = 1;
    const ctx = canvas.getContext('2d');
    let traceRow = new TraceRow<any>({
      canvasNumber: 1,
      alpha: true,
      contextId: '2d',
      isOffScreen: true,
    });
    traceRow.dataList = {
      supplier: true,
      isLoading: false,
    };
    traceRow.supplier = true;
    traceRow.isLoading = false;
    traceRow.name = '111';
    traceRow.height = 20;
    traceRow.height = 30;
    expect(traceRow.drawSelection(ctx)).toBeUndefined();
  });

  it('TraceRow Test13', () => {
    let traceRow = new TraceRow<any>({
      canvasNumber: 1,
      alpha: true,
      contextId: '2d',
      isOffScreen: true,
    });
    expect(traceRow.collect).toBeFalsy();
  });

  it('TraceRow Test14', () => {
    let traceRow = new TraceRow<any>({
      canvasNumber: 1,
      alpha: true,
      contextId: '2d',
      isOffScreen: true,
    });
    traceRow.collect = true;
    expect(traceRow.collect).toBeTruthy();
  });

  it('TraceRow Test15', () => {
    let traceRow = new TraceRow<any>({
      canvasNumber: 1,
      alpha: true,
      contextId: '2d',
      isOffScreen: true,
    });
    expect(traceRow.rowType).toBeFalsy();
  });

  it('TraceRow Test16', () => {
    let traceRow = new TraceRow<any>({
      canvasNumber: 1,
      alpha: true,
      contextId: '2d',
      isOffScreen: true,
    });
    traceRow.rowType = true;
    expect(traceRow.rowType).toBeTruthy();
  });

  it('TraceRow Test17', () => {
    let traceRow = new TraceRow<any>({
      canvasNumber: 1,
      alpha: true,
      contextId: '2d',
      isOffScreen: true,
    });
    expect(traceRow.rowId).toBeFalsy();
  });

  it('TraceRow Test18', () => {
    let traceRow = new TraceRow<any>({
      canvasNumber: 1,
      alpha: true,
      contextId: '2d',
      isOffScreen: true,
    });
    traceRow.rowId = true;
    expect(traceRow.rowId).toBeTruthy();
  });

  it('TraceRow Test19', () => {
    let traceRow = new TraceRow<any>({
      canvasNumber: 1,
      alpha: true,
      contextId: '2d',
      isOffScreen: true,
    });
    expect(traceRow.rowParentId).toBeFalsy();
  });

  it('TraceRow Test20', () => {
    let traceRow = new TraceRow<any>({
      canvasNumber: 1,
      alpha: true,
      contextId: '2d',
      isOffScreen: true,
    });
    traceRow.rowParentId = true;
    expect(traceRow.rowParentId).toBeTruthy();
  });

  it('TraceRow Test21', () => {
    let traceRow = new TraceRow<any>({
      canvasNumber: 1,
      alpha: true,
      contextId: '2d',
      isOffScreen: true,
    });
    traceRow.rowHidden = true;
    expect(traceRow.rowHidden).toBeUndefined();
  });

  it('TraceRow Test22', () => {
    let traceRow = new TraceRow<any>({
      canvasNumber: 1,
      alpha: true,
      contextId: '2d',
      isOffScreen: true,
    });
    expect(traceRow.name).toBeFalsy();
  });

  it('TraceRow Test23', () => {
    let traceRow = new TraceRow<any>({
      canvasNumber: 1,
      alpha: true,
      contextId: '2d',
      isOffScreen: true,
    });
    traceRow.folder = false;
    expect(traceRow.folder).toBeFalsy();
  });

  it('TraceRow Test24', () => {
    let traceRow = new TraceRow<any>({
      canvasNumber: 1,
      alpha: true,
      contextId: '2d',
      isOffScreen: true,
    });
    traceRow.folder = true;
    expect(traceRow.folder).toBeTruthy();
  });

  it('TraceRow Test25', () => {
    // let traceRow = new TraceRow<any>({canvasNumber:1,alpha: true, contextId: '2d', isOffScreen: true});
    // traceRow.expansion = false;
    // expect(traceRow.expansion).toBeFalsy();
  });

  it('TraceRow Test26', () => {
    // let traceRow = new TraceRow<any>({canvasNumber:1,alpha: true, contextId: '2d', isOffScreen: true});
    // traceRow.parentNode = jest.fn(()=>true);
    // traceRow.parentNode.lastChild = jest.fn(()=>true);
    // traceRow.expansion = true;
    // expect(traceRow.expansion).toBeTruthy();
  });

  it('TraceRow Test27', () => {
    let traceRow = new TraceRow<any>({
      canvasNumber: 1,
      alpha: true,
      contextId: '2d',
      isOffScreen: true,
    });
    traceRow.tip = true;
    expect(traceRow.tip).toBeUndefined();
  });

  it('TraceRow Test28', () => {
    let traceRow = new TraceRow<any>({
      canvasNumber: 1,
      alpha: true,
      contextId: '2d',
      isOffScreen: true,
    });
    expect(traceRow.frame).not.toBeUndefined();
  });

  it('TraceRow Test29', () => {
    let traceRow = new TraceRow<any>({
      canvasNumber: 1,
      alpha: true,
      contextId: '2d',
      isOffScreen: true,
    });
    traceRow.frame = [0, 0, 0];
    expect(traceRow.frame).toBeTruthy();
  });

  it('TraceRow Test60', () => {
    let traceRow = new TraceRow<any>({
      canvasNumber: 1,
      alpha: true,
      contextId: '2d',
      isOffScreen: true,
    });
    expect(traceRow.disabledCheck).not.toBeUndefined();
  });

  it('TraceRow Test61', () => {
    let traceRow = new TraceRow<any>({
      canvasNumber: 1,
      alpha: true,
      contextId: '2d',
      isOffScreen: true,
    });
    traceRow.disabledCheck = true;
    expect(traceRow.disabledCheck).toBeTruthy();
  });

  it('TraceRow Test62', () => {
    let traceRow = new TraceRow<any>({
      canvasNumber: 1,
      alpha: true,
      contextId: '2d',
      isOffScreen: true,
    });
    expect(traceRow.folderPaddingLeft).toBeUndefined();
  });

  it('TraceRow Test30', () => {
    let traceRow = new TraceRow<any>({
      canvasNumber: 1,
      alpha: true,
      contextId: '2d',
      isOffScreen: true,
    });
    expect(traceRow.checkType).not.toBeUndefined();
  });

  it('TraceRow Test31', () => {
    let traceRow = new TraceRow<any>({
      canvasNumber: 1,
      alpha: true,
      contextId: '2d',
      isOffScreen: true,
    });
    traceRow.checkType = '-1';
    expect(traceRow.checkType).toBeTruthy();
  });

  it('TraceRow Test32', () => {
    let traceRow = new TraceRow<any>({
      canvasNumber: 1,
      alpha: true,
      contextId: '2d',
      isOffScreen: true,
    });
    expect(traceRow.drawType).toBe(0);
  });

  it('TraceRow Test33', () => {
    let traceRow = new TraceRow<any>({
      canvasNumber: 1,
      alpha: true,
      contextId: '2d',
      isOffScreen: true,
    });
    traceRow.drawType = true;
    expect(traceRow.drawType).toBeTruthy();
  });

  it('TraceRow Test34', () => {
    let traceRow = new TraceRow<any>({
      canvasNumber: 1,
      alpha: true,
      contextId: '2d',
      isOffScreen: true,
    });
    traceRow.args = jest.fn(() => true);
    traceRow.args.isOffScreen = jest.fn(() => null);
    expect(traceRow.updateWidth(1)).toBeUndefined();
  });

  it('TraceRow Test36', () => {
    let traceRow = new TraceRow<any>({
      canvasNumber: 1,
      alpha: true,
      contextId: '2d',
      isOffScreen: true,
    });
    expect(traceRow.onMouseHover()).toBeFalsy();
  });

  it('TraceRow Test37', () => {
    let traceRow = new TraceRow<any>({
      canvasNumber: 1,
      alpha: true,
      contextId: '2d',
      isOffScreen: true,
    });
    expect(traceRow.setTipLeft(1, null)).toBeFalsy();
  });

  it('TraceRow Test38', () => {
    let traceRow = new TraceRow<any>({
      canvasNumber: 1,
      alpha: true,
      contextId: '2d',
      isOffScreen: true,
    });
    expect(traceRow.onMouseLeave(1, 1)).toBeFalsy();
  });

  it('TraceRow Test39', () => {
    let traceRow = new TraceRow<any>({
      canvasNumber: 1,
      alpha: true,
      contextId: '2d',
      isOffScreen: true,
    });
    expect(traceRow.draw(false)).toBeFalsy();
  });

  it('TraceRow Test40', () => {
    let traceRow = new TraceRow<any>({
      canvasNumber: 1,
      alpha: true,
      contextId: '2d',
      isOffScreen: true,
    });
    traceRow.collect = 1;
    expect(traceRow.collect).toBeTruthy();
  });

  it('TraceRow Test41', () => {
    let traceRow = new TraceRow<any>({
      canvasNumber: 1,
      alpha: true,
      contextId: '2d',
      isOffScreen: true,
    });
    traceRow.collect = 0;
    expect(traceRow.collect).toBeFalsy();
  });

  it('TraceRow Test42', () => {
    let traceRow = new TraceRow<any>({
      canvasNumber: 1,
      alpha: true,
      contextId: '2d',
      isOffScreen: true,
    });
    traceRow.checkType = '0';
    expect(traceRow.checkType).toBe('0');
  });

  it('TraceRow Test43', () => {
    let traceRow = new TraceRow<any>({
      canvasNumber: 1,
      alpha: true,
      contextId: '2d',
      isOffScreen: true,
    });
    traceRow.checkType = '1';
    expect(traceRow.checkType).toBe('1');
  });

  it('TraceRow Test44', () => {
    let traceRow = new TraceRow<any>({
      canvasNumber: 1,
      alpha: true,
      contextId: '2d',
      isOffScreen: true,
    });
    traceRow.checkType = '2';
    expect(traceRow.checkType).toBe('2');
  });

  it('TraceRow Test45', () => {
    let traceRow = new TraceRow<any>({
      canvasNumber: 1,
      alpha: true,
      contextId: '2d',
      isOffScreen: true,
    });
    traceRow.checkType = 0;
    expect(traceRow.checkType).toBe('');
  });

  it('TraceRow Test46', () => {
    let traceRow = new TraceRow<any>({
      canvasNumber: 1,
      alpha: true,
      contextId: '2d',
      isOffScreen: true,
    });
    traceRow.rowHidden = false;
    expect(traceRow.rowHidden).toBeUndefined();
  });

  it('TraceRow Test47', () => {
    let traceRow = new TraceRow<any>({
      canvasNumber: 1,
      alpha: true,
      contextId: '2d',
      isOffScreen: true,
    });
    traceRow.highlight = false;
    expect(traceRow.highlight).toBeFalsy();
  });

  it('TraceRow Test48', () => {
    let traceRow = new TraceRow<any>({
      canvasNumber: 1,
      alpha: true,
      contextId: '2d',
      isOffScreen: true,
    });
    traceRow.highlight = true;
    expect(traceRow.highlight).toBeFalsy();
  });

  it('TraceRow Test49', () => {
    let traceRow = new TraceRow<any>({
      canvasNumber: 1,
      alpha: true,
      contextId: '2d',
      isOffScreen: true,
    });
    traceRow.setCheckBox = true;
    expect(traceRow.highlight).toBeFalsy();
  });

  it('TraceRow Test50', () => {
    let traceRow = new TraceRow<any>({
      canvasNumber: 1,
      alpha: true,
      contextId: '2d',
      isOffScreen: true,
    });
    traceRow.initCanvas = jest.fn(() => null);
    expect(traceRow.connectedCallback()).toBeUndefined();
  });

  it('TraceRow Test51', () => {
    let traceRow = new TraceRow<any>({
      canvasNumber: 1,
      alpha: true,
      contextId: '2d',
      isOffScreen: true,
    });
    expect(traceRow.isInTimeRange()).toBe(false);
  });

  it('TraceRow Test52', () => {
    let traceRow = new TraceRow<any>({
      canvasNumber: 1,
      alpha: true,
      contextId: '2d',
      isOffScreen: true,
    });
    expect(traceRow.getLineColor()).toBe('');
  });

  it('TraceRow Test53', () => {
    let traceRow = new TraceRow<any>({
      canvasNumber: 1,
      alpha: true,
      contextId: '2d',
      isOffScreen: true,
    });
    let value = traceRow.attributeChangedCallback('name');
    expect(value).toBe(undefined);
  });

  it('TraceRow Test54', () => {
    let traceRow = new TraceRow<any>({
      canvasNumber: 1,
      alpha: true,
      contextId: '2d',
      isOffScreen: true,
    });
    let value = traceRow.attributeChangedCallback('height', '1', '2');
    expect(value).toBe(undefined);
  });

  it('TraceRow Test55', () => {
    let traceRow = new TraceRow<any>({
      canvasNumber: 1,
      alpha: true,
      contextId: '2d',
      isOffScreen: true,
    });
    let value = traceRow.attributeChangedCallback('check-type', '1', 'check');
    expect(value).toBe(undefined);
  });

  it('TraceRow Test56', () => {
    let traceRow = new TraceRow<any>({
      canvasNumber: 1,
      alpha: true,
      contextId: '2d',
      isOffScreen: true,
    });
    expect(traceRow.initHtml()).toMatchInlineSnapshot(`
"
        <style>
        *{
            box-sizing: border-box;
        }
        :host(:not([row-hidden])){
            box-sizing: border-box;
            display: flex;
            flex-direction: column;
            width: 100%;
            height: min-content;
        }
        :host([row-hidden]){
            width: 100%;
            display: none;
        }
        .root{
            height: 100%;
            width: 100%;
            display: grid;
            grid-template-rows: 100%;
            grid-template-columns: 248px 1fr;
            border-bottom: 1px solid var(--dark-border1,#dadada);
            border-right: 15px solid var(--dark-border1,#ffffff);
            box-sizing: border-box;
        }
        .root .drag{
            background-color: var(--dark-background1,#eee);
            box-shadow: 0 4px 12px -4px #999 inset;
        }
        .root .line-top{
            box-shadow: 0 4px 2px -1px #4d7ab3 inset; 
            transition: all 0.2s;
        }
        .root .line-bottom{
            box-shadow: 0 -4px 2px -1px #4d7ab3 inset; 
            transition: all 0.2s;
        }
        .describe{
            box-sizing: border-box;
            border-right: 1px solid var(--dark-border1,#c9d0da);
            background-color: var(--dark-background5,#ffffff);
            align-items: center;
            position: relative;
        }
        .panel{
            width: 100%;
            height: 100%;
            overflow: visible;
            background-color: transparent;
            display: block;
        }
        .panel-container{
            width: 100%;
            position: relative;
            pointer-events: none;
        }
        .name{
            color: var(--dark-color1,#4b5766);
            margin-left: 10px;
            font-size: .9rem;
            font-weight: normal;
            width: 80%;
            max-height: 100%;
            text-align: left;
            overflow: hidden;
            user-select: none;
            text-overflow: ellipsis;
            white-space:nowrap
        }
        :host([highlight]) .name{
            color: #4b5766;
        }
        .icon{
            color: var(--dark-color1,#151515);
            margin-left: 10px;
        }
        .describe:hover {
            cursor: pointer;
        }
        :host([folder]) .describe:hover > .icon{
            color:#ecb93f;
            margin-left: 10px;
        }
        :host([folder]){
            /*background-color: var(--dark-background1,#f5fafb);*/
        }
        :host([folder]) .icon{
            display: flex;
        }
        :host(:not([folder])){
            /*background-color: var(--dark-background,#FFFFFF);*/
        }
        :host(:not([folder]):not([children])) {
        }
        :host(:not([folder]):not([children])) .icon{
            display: none;
        }
        :host(:not([folder])[children]) .icon{
            visibility: hidden;
            color:#fff
        }

        :host(:not([folder])[children]) .name{
        }
        :host([expansion]) {
            background-color: var(--bark-expansion,#0C65D1);
        }
        :host([expansion]) .name,:host([expansion]) .icon{
            color: #fff;
        }
        :host([expansion]) .describe{
            border-right: 0px;
            background-color: var(--bark-expansion,#0C65D1);
        }
        :host([expansion]:not(sleeping)) .panel-container{
            display: none;
        }
        :host([expansion]) .children{
            flex-direction: column;
            width: 100%;
        }
        :host([expansion]) .icon{
            transform: rotateZ(0deg);
        }
        :host(:not([expansion])) .children{
            display: none;
            flex-direction: column;
            width: 100%;
        }
        :host(:not([expansion])) .icon{
            transform: rotateZ(-90deg);
        }
        :host([sleeping]) .describe{
            display: none;
        }
        :host([sleeping]) .panel-container{
            display: none;
        }
        :host([sleeping]) .children{
            display: none;
        }
        :host(:not([sleeping])) .describe{
            display: flex;;
        }
        :host(:not([sleeping])) .panel-container{
            display: block;
        }
        :host(:not([sleeping])) .children{
            display: flex;
        }
        :host([folder]) .lit-check-box{
            display: none;
        }
        :host(:not([check-type])) .lit-check-box{
            display: none;
        }
        :host([collect-type]) {
            /*position:fixed;*/
            /*z-index:1000;*/
        }
        :host(:not([collect-type])) {
            /*position:static;*/
        }
        :host([collect-type]) .collect{
            display: block;
            color: #5291FF;
        }
        :host(:not([collect-type])) .collect{
            display: none;
            color: var(--dark-icon,#666666);
        }
        .collect{
            margin-right: 5px;
        }
        :host(:not([folder])) .describe:hover .collect{
            display: block;
        }
        :host([row-type="native-memory"]) #nativeRadioList{
            display: flex;
        }
        .popover{
            color: var(--dark-color1,#4b5766);
            display: none;
            justify-content: center;
            align-items: center;
            margin-right: 5px;
        }
        .radio{
            margin-right: 10px;

        }
        #setting{
            color: var(--dark-color1,#606060);
        }
        :host([expansion]) #setting{
            color: #FFFFFF;
        }
        :host([highlight]) .flash{
            background-color: #ffe263;
        }
        
        :host([row-type="energy"]) #appNameList{
            display: flex;
        }
        
         #listprocess::-webkit-scrollbar{
         width: 6px;
        }
        
        /*定义滑块 内阴影+圆角*/
        #listprocess::-webkit-scrollbar-thumb
        {
          border-radius: 6px;
          background-color: var(--dark-background7,#e7c9c9);
        }

        </style>
        <div class="root">
            <div class="describe flash" style="position: inherit">
                <lit-icon class="icon" name="caret-down" size="19"></lit-icon>
                <label class="name"></label>
                <lit-icon class="collect" name="star-fill" size="19"></lit-icon>
                <lit-popover placement="bottomLeft" trigger="click" id = "nativeRadioList" class="popover" haveRadio="true" style="z-index: 1;position: absolute;left: 230px">
                    <div style="display: block" slot="content">
                        <div id="first-radio" style="margin-bottom: 5px">
                        <input class="radio" name="status" type="radio" value="0" />Current Bytes</div>
                        <div id="second-radio" style="margin-bottom: 5px">
                        <input class="radio" name="status" type="radio" value="1" />Native Memory Density</div>
                    </div>
                    <lit-icon name="setting" size="19" id="setting"></lit-icon>
                </lit-popover>
                <lit-popover placement="bottomLeft" trigger="click" id="appNameList" class="popover" haveRadio="true" style="z-index: 1;position: absolute;left: 230px">
                    <div slot="content" id="listprocess" style="height:200px;overflow-y:auto">
                    </div>
                    <lit-icon name="setting" size="19" id="setting"></lit-icon>
                </lit-popover>
                <lit-check-box class="lit-check-box" style="margin-right: 10px;"></lit-check-box>
            </div>
        </div>
        "
`);
  });
  it('TraceRow Test57', () => {
    let traceRow = new TraceRow<any>({
      canvasNumber: 1,
      alpha: true,
      contextId: '2d',
      isOffScreen: true,
    });
    expect(traceRow.rowDiscard).toBeFalsy();
  });
  it('TraceRow Test58', () => {
    let traceRow = new TraceRow<any>({
      canvasNumber: 1,
      alpha: true,
      contextId: '2d',
      isOffScreen: true,
    });
    traceRow.rowDiscard = true;
    expect(traceRow.rowDiscard).toBeTruthy();
  });
  it('TraceRow Test58', () => {
    let traceRow = new TraceRow<any>({
      canvasNumber: 1,
      alpha: true,
      contextId: '2d',
      isOffScreen: true,
    });
    traceRow.rowDiscard = false;
    expect(traceRow.rowDiscard).toBeFalsy();
  });
  it('TraceRow Test59', () => {
    let traceRow = new TraceRow<any>({
      canvasNumber: 1,
      alpha: true,
      contextId: '2d',
      isOffScreen: true,
    });
    traceRow.disabledCheck = false;
    expect(traceRow.disabledCheck).toBeFalsy();
  });
  it('TraceRow Test64', () => {
    let traceRow = new TraceRow<any>({
      canvasNumber: 1,
      alpha: true,
      contextId: '2d',
      isOffScreen: true,
    });
    traceRow.folderPaddingLeft = 1;
    expect(traceRow.folderPaddingLeft).toBeUndefined();
  });
  it('TraceRow Test65', () => {
    let traceRow = new TraceRow<any>({
      canvasNumber: 1,
      alpha: true,
      contextId: '2d',
      isOffScreen: true,
    });
    expect(traceRow.getTransferArray()).toStrictEqual([undefined]);
  });
});
