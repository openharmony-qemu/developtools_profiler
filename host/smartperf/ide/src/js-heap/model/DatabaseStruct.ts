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

import { HeapLoader } from '../logic/HeapLoader.js';
import { AllocationFunction, FileInfo } from './UiStruct.js';

export enum EdgeType {
  CONTEXT = 0,
  ELEMENT = 1,
  PROPERTY = 2,
  INTERNAL = 3,
  HIDDEN = 4,
  SHORTCUT = 5,
  WEAK = 6,
  STRING_OR_NUMBER = 6,
  NODE = 7,
  INVISIBLE = 8,
}
export enum NodeType {
  HIDDEN = 0,
  ARRAY = 1,
  STRING = 2,
  OBJECT = 3,
  CODE = 4,
  CLOSURE = 5,
  REGEXP = 6,
  NUMBER = 7,
  NATIVE = 8,
  SYNTHETIC = 9,
  CONCATENATED_STRING = 10,
  SLICED_STRING = 11,
  SYMBOL = 12,
  BIGINT = 13,
  OBJECT_SHAPE = 14,
}

function getNodeTypeName(nodeType: NodeType): keyof typeof NodeType {
  return Object.keys(NodeType).find(
    (key) => NodeType[key as keyof typeof NodeType] === nodeType
  ) as keyof typeof NodeType;
}

function getEdgeTypeName(nodeType: EdgeType): keyof typeof EdgeType {
  return Object.keys(EdgeType).find(
    (key) => EdgeType[key as keyof typeof EdgeType] === nodeType
  ) as keyof typeof EdgeType;
}

export enum DetachedNessState {
  UNKNOWN,
  ATTACHED,
  DETACHED,
}

export class HeapNode {
  fileId: number;
  nodeIndex: number;
  nodeOldIndex: number;
  type: NodeType;
  name: string;
  nameIdx!: number;
  id: number;
  selfSize: number;
  edgeCount: number;
  traceNodeId: number;
  detachedness: number;
  edges: Set<HeapEdge>;
  distance: number = -5;
  retainedSize: number;
  displayName: string = '';
  firstEdgeIndex: number;
  flag: number;
  retainsCount: number = 0;
  retainsEdgeIdx: Array<number>;
  retainsNodeIdx: Array<number>;

  constructor(
    fileId: number,
    nodeIndex: number,
    type: number,
    name: string,
    id: number,
    selfSize: number,
    edgeCount: number,
    traceNodeId: number,
    detachedness: number,
    firstEdgeIndex: number
  ) {
    this.fileId = fileId;
    this.nodeIndex = nodeIndex;
    this.nodeOldIndex = nodeIndex * 7;
    this.type = type;
    this.name = name;
    this.id = id;
    this.selfSize = selfSize;
    this.retainedSize = selfSize;
    this.edgeCount = edgeCount;
    this.traceNodeId = traceNodeId;
    this.detachedness = detachedness;
    this.firstEdgeIndex = firstEdgeIndex;
    this.edges = new Set<HeapEdge>();
    this.retainsEdgeIdx = new Array<number>();
    this.retainsNodeIdx = new Array<number>();
    this.flag = 0;
  }

  className(): string {
    switch (this.type) {
      case NodeType.HIDDEN:
        return '(system)';
      case NodeType.OBJECT:
      case NodeType.NATIVE:
        return this.nodeName();
      case NodeType.CODE:
        return '(compiled code)';
      default:
        let typeName = '(' + getNodeTypeName(this.type) + ')';
        return typeName.toLowerCase();
    }
  }

  nodeName(): string {
    return this.displayName || this.name;
  }

  addEdge(edge: HeapEdge) {
    this.edges.add(edge);
  }

  idHidden(): boolean {
    return this.type == NodeType.HIDDEN;
  }

  isArray(): boolean {
    return this.type === NodeType.ARRAY;
  }

  isUserRoot(): boolean {
    return this.type != NodeType.SYNTHETIC;
  }

  isDocumentDOMTreesRoot(): boolean {
    return this.type != NodeType.SYNTHETIC && this.name === '(Document DOM trees)';
  }
}

export class HeapEdge {
  edgeOldIndex: number;
  edgeIndex: number;
  type: EdgeType;
  nameOrIndex: string;
  nodeId: number;
  fromNodeId: number;
  toNodeId: number;
  retainsNode: Array<HeapNode>;
  retainEdge: Array<HeapEdge>;

  constructor(
    edgeIndex: number,
    type: number,
    nameOrIndex: string,
    nodeId: number,
    fromNodeId: number,
    toNodeId: number
  ) {
    this.edgeIndex = edgeIndex;
    this.edgeOldIndex = edgeIndex * 3;
    this.type = type;
    this.nameOrIndex = nameOrIndex;
    this.nodeId = nodeId;
    this.fromNodeId = fromNodeId;
    this.toNodeId = toNodeId;
    this.retainsNode = new Array<HeapNode>();
    this.retainEdge = new Array<HeapEdge>();
  }
}

export class HeapTraceFunctionInfo {
  id: number;
  index: number;
  name: string;
  scriptName: string;
  scriptId: number;
  line: number;
  column: number;

  constructor(
    id: number,
    index: number,
    name: string,
    scriptName: string,
    scriptId: number,
    line: number,
    column: number
  ) {
    this.id = id;
    this.index = index;
    this.name = name;
    this.scriptName = scriptName;
    this.scriptId = scriptId;
    this.line = line;
    this.column = column;
  }
}

export class HeapSample {
  timestamp: number;
  lastAssignedId: number;
  size: number = 0;

  constructor(timestamp: number, lastAssignedId: number) {
    this.timestamp = timestamp;
    this.lastAssignedId = lastAssignedId;
  }
}
export class HeapLocation {
  objectIndex: number;
  scriptId: number;
  line: number;
  column: number;

  constructor(objectIndex: number, scriptId: number, line: number, column: number) {
    this.objectIndex = objectIndex;
    this.scriptId = scriptId;
    this.line = line;
    this.column = column;
  }
}

export class HeapSnapshotStruct {
  nodeCount!: number;
  edgeCount!: number;
  functionCount!: number;

  nodeMap: Map<number, HeapNode>;
  edges: Array<HeapEdge>;
  functionInfos: Array<HeapTraceFunctionInfo>;
  traceNodes: Array<AllocationFunction>;
  samples: Array<HeapSample>;
  strings: Array<string>;

  rootNodeId: number = -1;

  constructor() {
    this.nodeMap = new Map<number, HeapNode>();
    this.edges = new Array<HeapEdge>();
    this.functionInfos = new Array<HeapTraceFunctionInfo>();
    this.traceNodes = new Array<AllocationFunction>();
    this.samples = new Array<HeapSample>();
    this.strings = new Array<string>();
  }

  public clear() {
    this.nodeMap.clear();
    this.edges.length = 0;
    this.functionInfos.length = 0;
    this.traceNodes.length = 0;
    this.samples.length = 0;
    this.strings.length = 0;
  }
}

export class FileStruct extends FileInfo {
  snapshotStruct: HeapSnapshotStruct;
  isParseSuccess: boolean;
  heapLoader!: HeapLoader;

  constructor() {
    super();
    this.isParseSuccess = true;
    this.snapshotStruct = new HeapSnapshotStruct();
  }
}
