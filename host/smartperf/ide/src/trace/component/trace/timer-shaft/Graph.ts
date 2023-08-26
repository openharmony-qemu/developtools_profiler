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

import { Rect } from './Rect.js';

export abstract class Graph {
  frame: Rect;
  context2D: CanvasRenderingContext2D;
  canvas: HTMLCanvasElement | undefined | null;

  protected constructor(canvas: HTMLCanvasElement | undefined | null, c: CanvasRenderingContext2D, frame: Rect) {
    this.canvas = canvas;
    this.frame = frame;
    this.context2D = c;
  }

  abstract draw(): void;
}
