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
import { BaseStruct } from '../../../bean/BaseStruct.js';
import { Rect } from '../timer-shaft/Rect.js';
import { TraceRow } from './TraceRow.js';

export class TraceRowObject<T extends BaseStruct> {
  public rowId: string | undefined;
  public rowType: string | undefined;
  public rowParentId: string | undefined;
  public rowHidden: boolean = false;
  public rowHeight: number = 40;
  public name: string | undefined;
  public must: boolean = true;
  public folder: boolean = false;
  public isLoading: boolean = false;
  public children: boolean = false;
  public expansion: boolean = false;
  public dataList: Array<T> | undefined;
  public dataListCache: Array<T> = [];
  public color: string | undefined;
  public frame: Rect | undefined;
  public supplier: (() => Promise<Array<T>>) | undefined | null;
  public onThreadHandler: | ((
        row: TraceRow<T>,
        ctx: | ImageBitmapRenderingContext | CanvasRenderingContext2D | WebGLRenderingContext |
          WebGL2RenderingContext | null | undefined ) => void) | undefined | null;
  public onDrawHandler: | ((
        ctx: | ImageBitmapRenderingContext | CanvasRenderingContext2D | WebGLRenderingContext |
          WebGL2RenderingContext | null | undefined) => void) | undefined | null;
  public top: number = 0;
  public rowIndex: number = 0;
  public preObject: TraceRowObject<T> | undefined | null;
  public nextObject: TraceRowObject<T> | undefined | null;
}
