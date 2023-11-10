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

import { LitTable } from '../../../../base-ui/table/lit-table.js';
import { NativeMemoryExpression } from '../../../bean/NativeHook.js';

export function resizeObserver(
  parentEl: HTMLElement,
  tableEl: LitTable,
  tblOffsetHeight: number = 50,
  loadingPage?: HTMLElement,
  loadingPageOffsetHeight: number = 24
): void {
  new ResizeObserver((entries) => {
    if (parentEl.clientHeight !== 0) {
      // @ts-ignore
      tableEl?.shadowRoot.querySelector('.table').style.height = parentEl.clientHeight - tblOffsetHeight + 'px';
      tableEl?.reMeauseHeight();
      if (loadingPage) {
        loadingPage.style.height = parentEl.clientHeight - loadingPageOffsetHeight + 'px';
      }
    }
  }).observe(parentEl);
}

export function resizeObserverFromMemory(
  parentElement: HTMLElement,
  tableEl: LitTable,
  filterEl: HTMLElement,
  tblOffsetHeight: number = 45
): void {
  new ResizeObserver((entries) => {
    let filterHeight = 0;
    if (parentElement.clientHeight !== 0) {
      // @ts-ignore
      tableEl?.shadowRoot.querySelector('.table').style.height = parentElement.clientHeight - tblOffsetHeight + 'px';
      tableEl?.reMeauseHeight();
    }
    if (filterEl!.clientHeight > 0) {
      filterHeight = filterEl!.clientHeight;
    }
    if (parentElement!.clientHeight > filterHeight) {
      filterEl!.style.display = 'flex';
    } else {
      filterEl!.style.display = 'none';
    }
  }).observe(parentElement);
}

export function showButtonMenu(filter: any, isShow: boolean): void {
  if (isShow) {
    filter.setAttribute('tree', '');
    filter.setAttribute('input', '');
    filter.setAttribute('inputLeftText', '');
  } else {
    filter.removeAttribute('tree');
    filter.removeAttribute('input');
    filter.removeAttribute('inputLeftText');
  }
}

export function compare<T extends CompareStruct>(base: T[], target: T[]): T[] {
  const diffMap = new Map<string, T>();

  for (const item of base) {
    diffMap.set(item.key, item.clone(true) as T);
  }

  for (const item of target) {
    if (diffMap.has(item.key)) {
      const diffItem = diffMap.get(item.key);
      diffItem!.value = diffItem!.value - item.value;
    } else {
      diffMap.set(item.key, item.clone() as T);
    }
  }
  return [...diffMap.values()];
}

export class CompareStruct {
  key: string;
  value: number;

  constructor(key: string, value: number) {
    this.key = key;
    this.value = value;
  }

  clone(isBase?: boolean): CompareStruct {
    const value = isBase ? this.value : -this.value;
    return new CompareStruct(this.key, value);
  }
}

export class ParseExpression {
  private expression: string; //输入的表达式
  private libTreeMap: Map<string, string[]> = new Map<string, string[]>();
  private expressionStruct: NativeMemoryExpression;
  constructor(expression: string) {
    this.expressionStruct = new NativeMemoryExpression();
    this.expression = expression.trim();
  }

  /**
   * 解析用户输入的表达式
   * @returns string：sql/ null: 非法表达式
   */
  public parse(): NativeMemoryExpression | null {
    // 表达式必须以@开头
    if (!this.expression.startsWith('@')) {
      return null;
    }

    const expressions: string[] = [];
    // 包含- 表示可能有两组表达式
    if (this.expression.includes('-')) {
      const multiExpression = this.expression.split('-');
      if (multiExpression.length === 0 || multiExpression.length > 2) {
        return null;
      }
      expressions.push(...multiExpression);
    } else {
      expressions.push(this.expression);
    }
    let include = true;
    for (let expression of expressions) {
      this.paseSingleExpression(expression, include);
      include = false;
    }
    return this.expressionStruct;
  }

  private paseSingleExpression(expression: string, includes: boolean): void {
    const regex = /\((.*?)\)/; // 匹配括号内的内容
    const match = expression.match(regex);
    if (match && match.length > 1) {
      expression = match[1];

      const libs = expression.split(','); // 逗号拆分lib
      for (let lib of libs) {
        lib = lib.trim();
        const items = lib.split(' '); // 空格拆分函数
        if (items.length > 0) {
          const path = items[0];
          items.splice(0, 1);
          if (this.libTreeMap.has(path)) {
            continue;
          }
          if (includes) {
            this.expressionStruct.includeLib.set(`${path}`, items);
          } else {
            this.expressionStruct.abandonLib.set(`${path}`, items);
          }
        }
      }
    }
  }
}
