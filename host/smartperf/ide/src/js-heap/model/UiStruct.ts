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

import { HeapDataInterface } from '../HeapDataInterface.js';
import { EdgeType, NodeType } from './DatabaseStruct.js';
const ROW_TYPE = 'js-memory';
export enum FileType {
  SNAPSHOT,
  TIMELINE,
}

export enum ConstructorType {
  ClassType,
  InstanceType,
  FiledType,
  RetainersType,
  ComparisonType,
}

export class ConstructorItem {
  rowName = ROW_TYPE;
  fileId = -1;
  nodeName = '';
  edgeName = '';
  childCount = 1; // child count
  distance = -1;
  shallowSize = -1;
  retainedSize = -1;
  retainedPercent = ''; //retained percent
  shallowPercent = ''; //shallow percent
  hasNext = true;
  status = true;
  isSelected: boolean = false;
  objectName = '';
  expanded: boolean = true;

  edgeCount = 0;
  edgeType!: EdgeType;
  type!: ConstructorType;
  nodeType!: NodeType;
  nextId: [] = [];
  id = -1;
  index = -1;
  traceNodeId = -1;
  parent!: ConstructorItem;
  children: Array<ConstructorItem> = [];
  retains: Array<ConstructorItem> = [];
  classChildren: Array<ConstructorItem> = [];

  getChildren(): ConstructorItem[] {
    if (!this.hasNext) return [];
    let data = HeapDataInterface.getInstance();
    switch (this.type) {
      case ConstructorType.ClassType:
      case ConstructorType.InstanceType:
      case ConstructorType.FiledType:
        this.children = data.getNextForConstructor(this);
        break;
      case ConstructorType.RetainersType:
        this.children = data.getRetains(this);
        break;
    }
    return this.children;
  }

  isString(): boolean {
    return [NodeType.STRING, NodeType.CONCATENATED_STRING, NodeType.SLICED_STRING].includes(this.nodeType);
  }

  clone(): ConstructorItem {
    let copyItem = new ConstructorItem();
    this.cloneContent(copyItem);
    return copyItem;
  }

  protected cloneContent(copyItem: ConstructorItem) {
    copyItem.fileId = this.fileId;
    copyItem.distance = this.distance;
    copyItem.shallowSize = this.shallowSize;
    copyItem.nodeName = this.nodeName;
    copyItem.edgeCount = this.edgeCount;
    copyItem.edgeType = this.edgeType;
    copyItem.childCount = this.childCount;
    copyItem.hasNext = this.hasNext;
  }
}

export class ConstructorComparison extends ConstructorItem {
  targetFileId = -1;
  addedCount = 0;
  removedCount = 0;
  deltaCount = 0;

  addedSize = 0;
  removedSize = 0;
  deltaSize = 0;

  deletedIdx: Array<number> = [];
  addedIndx: Array<number> = [];

  isAdd = false;

  getChildren(): ConstructorItem[] {
    if (this.type !== ConstructorType.ComparisonType) {
      return super.getChildren();
    }
    if (!this.hasNext) return [];
    let data = HeapDataInterface.getInstance();
    if (this.type == ConstructorType.ComparisonType) {
      this.children = data.getNextForComparison(this);
    }
    return this.children;
  }

  clone(): ConstructorComparison {
    let copyItem = new ConstructorComparison();
    this.cloneContent(copyItem);
    return copyItem;
  }
}
export class AllocationFunction {
  fileId = -1;
  functionIndex = -1;
  parentsId: Array<number>;
  parents: Array<AllocationFunction>;
  combineId: Set<number>;
  status = true;

  id: number;
  name: string;
  scriptName: string;
  scriptId: number;
  line: number;
  column: number;
  count: number;
  size: number;
  liveCount: number;
  liveSize: number;
  hasParent: boolean;

  constructor(
    nodeId: number,
    functionName: string,
    scriptName: string,
    scriptId: number,
    line: number,
    column: number,
    count: number,
    size: number,
    liveCount: number,
    liveSize: number,
    hasParent: boolean
  ) {
    this.combineId = new Set<number>();
    this.parentsId = new Array<number>();
    this.parents = new Array<AllocationFunction>();
    this.id = nodeId;
    this.name = functionName;
    this.scriptName = scriptName;
    this.scriptId = scriptId;
    this.line = line;
    this.column = column;
    this.count = count;
    this.size = size;
    this.liveCount = liveCount;
    this.liveSize = liveSize;
    this.hasParent = hasParent;
  }

  /**
   * bottom up next level is parent
   * return Parents
   */
  getChildren(): AllocationFunction[] {
    if (!this.hasParent) return [];
    let data = HeapDataInterface.getInstance();
    //bottom up next level is parent
    data.getParentFunction(this);
    return this.parents;
  }

  clone(): AllocationFunction {
    let cloneItem = new AllocationFunction(
      this.id,
      this.name,
      this.scriptName,
      this.scriptId,
      this.line,
      this.column,
      this.count,
      this.size,
      this.liveCount,
      this.liveSize,
      this.hasParent
    );
    cloneItem.parentsId = this.parentsId;
    return cloneItem;
  }
}

export class FileInfo {
  id: number = -1;
  name: string = '';
  type!: FileType;
  startTs: number = 0;
  endTs: number = 0;
  pid: number = 0;
  size: number = 0;
}
