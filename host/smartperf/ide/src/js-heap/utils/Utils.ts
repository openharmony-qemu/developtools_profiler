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

import { HeapNode } from '../model/DatabaseStruct.js';
import { ConstructorComparison } from '../model/UiStruct.js';

export function HeapNodeToConstructorItem(node: HeapNode): ConstructorComparison {
  let constructor = new ConstructorComparison();
  constructor.fileId = node.fileId;
  constructor.id = node.id;
  constructor.index = node.nodeIndex;
  constructor.nodeName = node.nodeName();
  constructor.edgeCount = node.edgeCount;
  constructor.distance = node.distance;
  constructor.shallowSize = node.selfSize;
  constructor.retainedSize = node.retainedSize;
  constructor.nodeType = node.type;
  return constructor;
}

export function getTimeForLog() {
  let date = new Date(Date.now());
  return `${date.getHours()}:${date.getMinutes()}:${date.getSeconds()}.${date.getMilliseconds()} `;
}
