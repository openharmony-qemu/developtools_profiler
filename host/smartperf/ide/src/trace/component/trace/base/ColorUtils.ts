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

import { CpuStruct } from '../../../database/ui-worker/ProcedureWorkerCPU.js';

export class ColorUtils {
  public static GREY_COLOR: string = '#f0f0f0';

  public static FUNC_COLOR_A: Array<string> = [
    '#40b3e7',
    '#23b0e7',
    '#8d9171',
    '#FF0066',
    '#7a9160',
    '#9fafc4',
    '#8a8a8b',
    '#e05b52',
    '#9bb87a',
    '#ebc247',
    '#c2cc66',
    '#a16a40',
    '#a94eb9',
    '#aa4fba',
  ];
  public static FUNC_COLOR_B: Array<string> = [
    '#9785D3',
    '#A27F7E',
    '#00bdd6',
    '#94B5F4',
    '#B282F6',
    '#E97978',
    '#7AD7E6',
    '#A1C38A',
    '#DB8E86',
    '#42B7A4',
    '#AACEA0',
    '#E69553',
    '#7EC6BB',
    '#8d9171',

  ];

  public static ANIMATION_COLOR: Array<string> = [
    '#ECECEC',
    '#FE3000',
    '#61CFBE',
    '#000',
    '#FFFFFF',
    '#C6D9F2',
    '#BFEBE5',
    '#0A59F7',
    '#25ACF5',
    '#FFFFFF'
  ];

  public static JANK_COLOR: Array<string> = ['#42A14D', '#C0CE85', '#FF651D', '#E8BE44', '#009DFA', '#E97978', '#A8D1F4'];
  public static MD_PALETTE: Array<string> = ColorUtils.FUNC_COLOR_B;
  public static FUNC_COLOR: Array<string> = ColorUtils.FUNC_COLOR_B;

  public static hash(str: string, max: number): number {
    let colorA: number = 0x811c9dc5;
    let colorB: number = 0xfffffff;
    let colorC: number = 16777619;
    let colorD: number = 0xffffffff;
    let hash: number = colorA & colorB;

    for (let index: number = 0; index < str.length; index++) {
      hash ^= str.charCodeAt(index);
      hash = (hash * colorC) & colorD;
    }
    return Math.abs(hash) % max;
  }

  public static colorForThread(thread: CpuStruct): string {
    if (thread == null) {
      return ColorUtils.GREY_COLOR;
    }
    let tid: number | undefined | null = (thread.processId || -1) >= 0 ? thread.processId : thread.tid;
    return ColorUtils.colorForTid(tid || 0);
  }

  public static colorForTid(tid: number): string {
    let colorIdx: number = ColorUtils.hash(`${tid}`, ColorUtils.MD_PALETTE.length);
    return ColorUtils.MD_PALETTE[colorIdx];
  }

  public static colorForName(name: string): string {
    let colorIdx: number = ColorUtils.hash(name, ColorUtils.MD_PALETTE.length);
    return ColorUtils.MD_PALETTE[colorIdx];
  }

  public static formatNumberComma(str: number): string {
    if (str === undefined || str === null) return '';
    let unit = str >= 0 ? '' : '-';
    let l = Math.abs(str).toString().split('').reverse();
    let t: string = '';
    for (let i = 0; i < l.length; i++) {
      t += l[i] + ((i + 1) % 3 == 0 && i + 1 != l.length ? ',' : '');
    }
    return unit + t.split('').reverse().join('');
  }

  public static hashFunc(str: string, depth: number, max: number): number {
    let colorA: number = 0x811c9dc5;
    let colorB: number = 0xfffffff;
    let colorC: number = 16777619;
    let colorD: number = 0xffffffff;
    let hash: number = colorA & colorB;
    let st = str.replace(/[0-9]+/g, '');
    for (let index: number = 0; index < st.length; index++) {
      hash ^= st.charCodeAt(index);
      hash = (hash * colorC) & colorD;
    }
    return (Math.abs(hash) + depth) % max;
  }

  public static funcTextColor(val: string) {
    var reg = /^#([0-9a-fA-f]{3}|[0-9a-fA-f]{6})$/;
    // 把颜色值变成小写
    var color = val.toLowerCase();
    var result = '';
    if (reg.test(color)) {
      if (color.length === 4) {
        var colorNew = '#';
        for (var i = 1; i < 4; i += 1) {
          colorNew += color.slice(i, i + 1).concat(color.slice(i, i + 1));
        }
        color = colorNew;
      }
      var colorChange = [];
      for (var i = 1; i < 7; i += 2) {
        colorChange.push(parseInt('0x' + color.slice(i, i + 2)));
      }
      var grayLevel = colorChange[0] * 0.299 + colorChange[1] * 0.587 + colorChange[2] * 0.114;
      if (grayLevel >= 150) {
        //浅色模式
        return '#000';
      } else {
        return '#fff';
      }
    } else {
      result = '无效';
      return result;
    }
  }
}
