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

import { AllocationLogic } from './Allocation.js';
import { AllocationFunction, ConstructorComparison, ConstructorItem, ConstructorType } from '../model/UiStruct.js';
import {
  DetachedNessState,
  EdgeType,
  FileStruct,
  HeapEdge,
  HeapNode,
  HeapSample,
  HeapTraceFunctionInfo,
  NodeType,
} from '../model/DatabaseStruct.js';
import { HeapNodeToConstructorItem } from '../utils/Utils.js';

const BASE_SYSTEM_DISTANCE = 100000000;
const CAN_BE_QUERIED = 1;
const DETACHED_DOM_NODE = 2;
const PAGE_PROJECT = 4;

export class HeapLoader {
  private fileId: number;
  private fileStruct: FileStruct;
  private allocationLogic: AllocationLogic;
  private nodeMap: Map<number, HeapNode>;
  private edges: Array<HeapEdge>;
  private rootNode: HeapNode | undefined;
  private nodeCount: number;
  private edgeCount: number;
  private nodes: Array<HeapNode>;
  private retainingNodes: Uint32Array;
  private retainingEdges: Uint32Array;
  private firstRetainerIndex: Uint32Array;
  private dominatorTree: Uint32Array;
  private firstDominatedNodesIdx: Uint32Array;
  private dominatedNodes: Uint32Array;
  private allClasses?: Map<string, ConstructorItem>;
  private diffToOtherFile: Map<number, Map<string, ConstructorComparison>>;

  constructor(fileStruct: FileStruct) {
    this.fileStruct = fileStruct;
    this.fileId = fileStruct.id;
    this.nodeCount = fileStruct.snapshotStruct.nodeCount;
    this.edgeCount = fileStruct.snapshotStruct.edgeCount;
    this.nodeMap = fileStruct.snapshotStruct.nodeMap;
    this.edges = fileStruct.snapshotStruct.edges;
    this.allocationLogic = new AllocationLogic(this.fileStruct);
    this.nodes = new Array<HeapNode>(this.nodeCount);
    this.nodeMap.forEach((value) => {
      this.nodes[value.nodeIndex] = value;
    });
    this.nodes.sort((a, b) => a.nodeIndex - b.nodeIndex);
    this.rootNode = this.nodes[0];
    this.retainingNodes = new Uint32Array(this.edgeCount); //每一个值为node index
    this.retainingEdges = new Uint32Array(this.edgeCount); // 每一个值为edge index
    this.firstRetainerIndex = new Uint32Array(this.nodeCount + 1);
    this.dominatorTree = new Uint32Array(this.nodeCount);
    this.firstDominatedNodesIdx = new Uint32Array(this.nodeCount + 1);
    this.dominatedNodes = new Uint32Array(this.nodeCount - 1);
    this.diffToOtherFile = new Map<number, Map<string, ConstructorComparison>>();
    this.preprocess();
  }

  get allocation() {
    return this.allocationLogic;
  }

  private preprocess() {
    if (!this.rootNode) {
      return;
    }
    this.buildNodeEdge();
    this.buildRetainers();
    this.distributeDOMState();
    this.calFlags();
    this.calDistances();
    this.calRetainedSize();
    // use to cal class Retained Size
    this.buildDominatedNode();
    this.buildSamples();
  }

  /**
   * set node parents node
   * node has multi parent because bottom up combine multi node
   * @param node selected node
   */
  loadAllocationParent(node: AllocationFunction) {
    this.allocationLogic.getParent(node);
  }

  /**
   * get Bottom Up Function List
   * @returns bottomUpList
   */
  getAllocationFunctionList(): Array<AllocationFunction> {
    return this.allocationLogic.getFunctionList();
  }

  /**
   * get full stack for node
   * @param allocationNodeId node.traceNodeId
   * @returns stack list
   */
  getAllocationStack(traceNodeId: number): Array<HeapTraceFunctionInfo> {
    return this.allocationLogic.getNodeStack(traceNodeId);
  }

  getFunctionNodeIds(id: number) {
    return this.allocationLogic.getFunctionNodeIds(id);
  }

  getAllocation() {
    return this.allocationLogic;
  }

  private buildNodeEdge(): void {
    for (let node of this.nodes) {
      for (let i = 0; i < node.edgeCount; i++) {
        let edgeIndex = node.firstEdgeIndex + i;
        if (this.edges.length > edgeIndex) {
          node.addEdge(this.edges[edgeIndex]);
        }
      }
    }
  }

  private buildRetainers(): void {
    // Iterate over edges and count how many times each node is retained by other nodes
    for (let edge of this.edges) {
      let toNode = this.nodeMap.get(edge.toNodeId);
      if (toNode) {
        ++this.firstRetainerIndex[toNode.nodeIndex];
      }
    }
    // Assign the first retainer slot index for each node
    let firstUnusedRetainerSlot = 0;
    for (let node of this.nodes) {
      let retainCount = this.firstRetainerIndex[node.nodeIndex];
      this.firstRetainerIndex[node.nodeIndex] = firstUnusedRetainerSlot;
      this.retainingNodes[firstUnusedRetainerSlot] = retainCount;
      firstUnusedRetainerSlot += retainCount;
    }
    this.firstRetainerIndex[this.nodeCount] = this.retainingNodes.length;

    // Fill the retainer slots with the retaining nodes and edges
    for (let node of this.nodes) {
      for (let edge of node.edges) {
        let childNode = this.nodeMap.get(edge.toNodeId);
        if (!childNode) {
          continue;
        }
        if (node.nodeIndex !== this.rootNode?.nodeIndex) {
          childNode.retainsNodeIdx.push(node.nodeIndex);
          childNode.retainsEdgeIdx.push(edge.edgeIndex);
        }

        let firstRetainerSlotIndex = this.firstRetainerIndex[childNode.nodeIndex];

        let nextUnusedRetainerSlotIndex = firstRetainerSlotIndex + --this.retainingNodes[firstRetainerSlotIndex];
        this.retainingNodes[nextUnusedRetainerSlotIndex] = node.nodeIndex;
        this.retainingEdges[nextUnusedRetainerSlotIndex] = edge.edgeIndex;
      }
    }
  }

  private distributeDOMState(): void {
    // 1.存储传播的状态。虽然已知节点的状态已经设置，
    // 但仍然需要经过处理以调整其名称并将其放入各自的队列中。
    let domState = new DOMState(this.nodeCount);
    for (let node of this.nodes) {
      if (node.detachedness === DetachedNessState.UNKNOWN) {
        continue;
      }
      this.processNode(domState, node, node.detachedness);
    }

    // 2. 如果父节点被Attached，那么子节点也被Attached。
    while (domState.attached.length > 0) {
      let nodeId = domState.attached.pop() as number;
      let node = this.nodeMap.get(nodeId);
      if (node) {
        this.iterateFilteredChildren(domState, node, DetachedNessState.ATTACHED);
      }
    }

    // 3. 如果父节点为Attached，那么子节点将继承父节点的状态。
    while (domState.detached.length > 0) {
      let nodeId = domState.detached.pop() as number;
      let node = this.nodeMap.get(nodeId);
      if (node && node.detachedness !== DetachedNessState.ATTACHED) {
        this.iterateFilteredChildren(domState, node, DetachedNessState.DETACHED);
      }
    }
  }

  private calFlags(): void {
    this.markQueryableNodes();
    this.markPageOwnedNodes();
  }

  private buildOrderIdxAndDominateTree(): Uint32Array {
    let stackNodes = new Uint32Array(this.nodeCount); // node index
    let stackCurrentEdge = new Uint32Array(this.nodeCount); // edge index
    let orderIdx2NodeIdx = new Uint32Array(this.nodeCount);
    let nodeIdx2OrderIdx = new Uint32Array(this.nodeCount);
    let visited = new Uint8Array(this.nodeCount);
    let postOrderIdx = 0;
    let stack = 0;
    stackNodes[0] = this.rootNode!.nodeIndex;
    visited[this.rootNode!.nodeIndex] = 1;

    let iteration = 0;
    while (true) {
      ++iteration;
      while (stack >= 0 && stack < this.nodeCount) {
        let nodeIndex = stackNodes[stack];
        let node = this.nodes[nodeIndex];
        if (!node) {
          continue;
        }
        let edgeIndex = stackCurrentEdge[stack];
        let edgeEnd = node.firstEdgeIndex + node.edgeCount;
        if (edgeIndex < edgeEnd) {
          stackCurrentEdge[stack] += 1;
          let edge = this.edges[edgeIndex];
          if (!this.isEssentialEdge(edge, node.id)) {
            continue;
          }
          let childNode = this.nodeMap.get(edge.toNodeId);
          if (!childNode || visited[childNode!.nodeIndex]) {
            continue;
          }
          //Skip the edges from non-page-object nodes to page-object nodes
          let childNodeFlag = childNode.flag & PAGE_PROJECT;
          if (node.id != this.rootNode!.id && childNodeFlag && !node.flag) {
            continue;
          }
          ++stack;
          stackNodes[stack] = childNode.nodeIndex;
          stackCurrentEdge[stack] = childNode.firstEdgeIndex;
          visited[childNode.nodeIndex] = 1;
        } else {
          nodeIdx2OrderIdx[node.nodeIndex] = postOrderIdx;
          orderIdx2NodeIdx[postOrderIdx++] = node.nodeIndex;
          --stack;
        }
      }
      if (postOrderIdx == this.nodeCount || iteration > 1) {
        break;
      }

      // Remove root from the result (last node in the array) and put it at the bottom of the stack so that it is
      // visited after all orphan nodes and their subgraphs.
      --postOrderIdx;
      stack = 0;
      stackNodes[0] = this.rootNode!.nodeIndex;
      stackCurrentEdge[0] = this.nodes[this.rootNode!.nodeIndex + 1].firstEdgeIndex;
      for (let node of this.nodes) {
        if (visited[node.nodeIndex] || !this.hasOnlyWeakRetainers(node)) {
          continue;
        }
        stackNodes[++stack] = node.nodeIndex;
        stackCurrentEdge[stack] = node.firstEdgeIndex;
        visited[node.nodeIndex] = 1;
      }
    }
    // If we already processed all orphan nodes that have only weak retainers and still have some orphans...
    if (postOrderIdx !== this.nodeCount) {
      // Remove root from the result (last node in the array) and put it at the bottom of the stack so that it is
      // visited after all orphan nodes and their subgraphs.
      --postOrderIdx;
      for (let i = 0; i < this.nodeCount; i++) {
        if (visited[i]) {
          continue;
        }
        // Fix it by giving the node a postorder index anyway.
        nodeIdx2OrderIdx[i] = postOrderIdx;
        orderIdx2NodeIdx[postOrderIdx++] = i;
      }
      nodeIdx2OrderIdx[this.rootNode!.nodeIndex] = postOrderIdx;
      orderIdx2NodeIdx[postOrderIdx++] = this.rootNode!.nodeIndex;
    }
    this.buildDominatorTree(orderIdx2NodeIdx, nodeIdx2OrderIdx);
    return orderIdx2NodeIdx;
  }

  // The algorithm is based on the article:
  // K. Cooper, T. Harvey and K. Kennedy "A Simple, Fast Dominance Algorithm"
  // Softw. Pract. Exper. 4 (2001), pp. 1-10.
  private buildDominatorTree(orderIdx2NodeIdx: Uint32Array, nodeIdx2OrderIdx: Uint32Array): void {
    let rootOrderedIdx = this.nodeCount - 1;
    let dominators = new Uint32Array(this.nodeCount).fill(this.nodeCount);
    dominators[rootOrderedIdx] = rootOrderedIdx;

    // The affected array is used to mark entries which dominators
    // have to be racalculated because of changes in their retainers.
    let affected = new Uint8Array(this.nodeCount);

    // 标记root节点的子节点为affected.
    for (let edge of this.rootNode!.edges) {
      if (!this.isEssentialEdge(edge, this.rootNode!.id)) {
        continue;
      }
      let childNode = this.nodeMap.get(edge.toNodeId);
      if (childNode) {
        affected[nodeIdx2OrderIdx[childNode.nodeIndex]] = 1;
      }
    }

    let changed = true;
    let nodeIdx;
    while (changed) {
      changed = false;
      for (let orderIdx = rootOrderedIdx - 1; orderIdx >= 0; --orderIdx) {
        // If dominator of the entry has already been set to root,
        // then it can't propagate any further.
        if (affected[orderIdx] === 0) {
          continue;
        }
        affected[orderIdx] = 0;
        if (dominators[orderIdx] === rootOrderedIdx) {
          continue;
        }
        nodeIdx = orderIdx2NodeIdx[orderIdx];
        let node = this.nodes[nodeIdx];
        let nodeFlag = node.flag & PAGE_PROJECT;
        let newDominatorIdx = this.nodeCount;
        let retainerStart = this.firstRetainerIndex[nodeIdx];
        let retainerEnd = this.firstRetainerIndex[nodeIdx + 1];
        let orphanNode = true;
        for (let idx = retainerStart; idx < retainerEnd; idx++) {
          let retainerNodeIdx = this.retainingNodes[idx];
          let retainerEdgeIdx = this.retainingEdges[idx];
          let node = this.nodes[retainerNodeIdx];
          let edge = this.edges[retainerEdgeIdx];
          if (!this.isEssentialEdge(edge, node.id)) {
            continue;
          }
          orphanNode = false;
          let retainerNodeFlag = node.flag & PAGE_PROJECT;
          if (retainerNodeIdx !== this.rootNode?.nodeIndex && nodeFlag && !retainerNodeFlag) {
            continue;
          }
          let retainerOrderIdx = nodeIdx2OrderIdx[retainerNodeIdx];
          if (dominators[retainerOrderIdx] !== this.nodeCount) {
            if (newDominatorIdx === this.nodeCount) {
              newDominatorIdx = retainerOrderIdx;
            } else {
              while (retainerOrderIdx !== newDominatorIdx) {
                while (retainerOrderIdx < newDominatorIdx) {
                  retainerOrderIdx = dominators[retainerOrderIdx];
                }
                while (newDominatorIdx < retainerOrderIdx) {
                  newDominatorIdx = dominators[newDominatorIdx];
                }
              }
            }
            // If idom has already reached the root, it doesn't make sense
            // to check other retainers.
            if (newDominatorIdx === rootOrderedIdx) {
              break;
            }
          }
        }
        // Make root dominator of orphans.
        if (orphanNode) {
          newDominatorIdx = rootOrderedIdx;
        }
        if (newDominatorIdx !== this.nodeCount && dominators[orderIdx] !== newDominatorIdx) {
          dominators[orderIdx] = newDominatorIdx;
          changed = true;
          nodeIdx = orderIdx2NodeIdx[orderIdx];
          let node = this.nodes[nodeIdx];
          for (let edge of node.edges) {
            let childNode = this.nodeMap.get(edge.toNodeId);
            if (childNode) {
              affected[nodeIdx2OrderIdx[childNode.nodeIndex]] = 1;
            }
          }
        }
      }
    }

    for (let orderIdx = 0; orderIdx < dominators.length; orderIdx++) {
      nodeIdx = orderIdx2NodeIdx[orderIdx];
      this.dominatorTree[nodeIdx] = orderIdx2NodeIdx[dominators[orderIdx]];
    }
  }

  private calDistances(): void {
    if (!this.rootNode) return;
    let nodesToVisit = new Uint32Array(this.nodeCount);
    let nodesToVisitLen = 0;
    // root node's edges distance is 1
    for (let edge of this.rootNode.edges) {
      let node = this.nodeMap.get(edge.toNodeId);
      if (!node) {
        continue;
      }
      if (node.isUserRoot() || node.isDocumentDOMTreesRoot()) {
        node.distance = 1;
        nodesToVisit[nodesToVisitLen++] = node.id;
      }
    }
    this.bfs(nodesToVisit, nodesToVisitLen);

    // 如果roo节点下有能访问到的子节点,将root节点的distance设置为100000000,否则为0
    this.rootNode.distance = nodesToVisitLen > 0 ? BASE_SYSTEM_DISTANCE : 0;
    // 设置root节点下访问不到的哪些独立节点的distance,从100000000开始计数
    nodesToVisit[0] = this.rootNode.id;
    nodesToVisitLen = 1;
    this.bfs(nodesToVisit, nodesToVisitLen);
  }

  private calRetainedSize(): void {
    let orderIdx2NodeIdx = this.buildOrderIdxAndDominateTree();
    // Propagate retained sizes for each node excluding root.
    for (let idx = 0; idx < this.nodeCount - 1; idx++) {
      let node = this.nodes[orderIdx2NodeIdx[idx]];
      let dominatorNode = this.nodes[this.dominatorTree[node.nodeIndex]];
      dominatorNode.retainedSize += node.retainedSize;
    }
  }

  private buildDominatedNode() {
    //赋值两个数组:
    //-dominatedNodes是一个连续数组，其中每个节点拥有一个与相应的被支配节点的间隔(可以为空)。
    //—indexArray 是dominatedNodes中与_nodeIndex位置相同的索引数组。
    let fromNodeIdx = 0;
    let toNodeIdx = this.nodeCount;

    if (this.rootNode?.nodeIndex === 0) {
      fromNodeIdx = 1;
    } else if (this.rootNode?.nodeIndex == toNodeIdx - 1) {
      toNodeIdx -= 1;
    } else {
      throw new Error('Root node is expected to be either first or last');
    }
    for (let nodeIdx = fromNodeIdx; nodeIdx < toNodeIdx; ++nodeIdx) {
      ++this.firstDominatedNodesIdx[this.dominatorTree[nodeIdx]];
    }
    // Put in the first slot of each dominatedNodes slice the count of entries
    // that will be filled.
    let firstDominatedNodeIdx = 0;
    for (let idx = 0; idx < this.nodeCount; idx++) {
      let dominateCount = (this.dominatedNodes[firstDominatedNodeIdx] = this.firstDominatedNodesIdx[idx]);
      this.firstDominatedNodesIdx[idx] = firstDominatedNodeIdx;
      firstDominatedNodeIdx += dominateCount;
    }
    this.firstDominatedNodesIdx[this.nodeCount] = this.dominatedNodes.length;
    // Fill up the dominatedNodes array with indexes of dominated nodes. Skip the root (node at
    // index 0) as it is the only node that dominates itself.
    for (let nodeIdx = fromNodeIdx; nodeIdx < toNodeIdx; nodeIdx++) {
      let dominatorIdx = this.dominatorTree[nodeIdx];
      let dominatorRefIdx = this.firstDominatedNodesIdx[dominatorIdx];
      dominatorRefIdx += --this.dominatedNodes[dominatorRefIdx];
      this.dominatedNodes[dominatorRefIdx] = nodeIdx;
    }
  }

  private buildSamples() {
    let samples = this.fileStruct.snapshotStruct.samples;
    if (!samples.length) {
      return;
    }
    for (let node of this.nodes) {
      if (node.id % 2 === 0) {
        continue;
      }
      let rangeIdx = this.binarySearchNodeInSamples(node.id, samples);
      if (rangeIdx === samples.length) {
        continue;
      }
      samples[rangeIdx].size += node.selfSize;
    }
  }

  getMinAndMaxNodeId() {
    return {
      minNodeId: this.nodes[0].id,
      maxNodeId: this.nodes[this.nodeCount - 1].id,
    };
  }

  private binarySearchNodeInSamples(nodeId: number, samples: Array<HeapSample>): number {
    let left = 0;
    let right = samples.length - 1;

    while (left <= right) {
      const mid = Math.floor((left + right) / 2);
      const currentSample = samples[mid];
      if (currentSample.lastAssignedId === nodeId) {
        return left;
      } else if (currentSample.lastAssignedId < nodeId) {
        left = mid + 1;
      } else {
        right = mid - 1;
      }
    }
    return left;
  }

  private filterForBpf(node: HeapNode, edge: HeapEdge): boolean {
    if (node.type === NodeType.HIDDEN) {
      return edge.nameOrIndex !== 'sloppy_function_map' || node.name !== 'system / NativeContext';
    }
    if (node.type === NodeType.ARRAY) {
      if (node.name !== '(map descriptors)') {
        return true;
      }
      const index = parseInt(edge.nameOrIndex, 10);
      return index < 2 || index % 3 !== 1;
    }
    return true;
  }

  /**
   * 广度优先算法遍历所有能从root节点访问到的节点，设置节点的的distance
   * @param nodesToVisit 能被访问到的node id 数组
   * @param nodesToVisitLen 有效的node数量
   */
  private bfs(nodesToVisit: Uint32Array, nodesToVisitLen: number) {
    let index = 0;
    while (index < nodesToVisitLen) {
      let nodeId = nodesToVisit[index++];
      let node = this.nodeMap.get(nodeId);
      if (!node) {
        continue;
      }
      let distance = node.distance + 1;

      for (let edge of node.edges) {
        if (edge.type === EdgeType.WEAK || !this.nodeMap.has(edge.toNodeId)) {
          continue;
        }
        let childNode = this.nodeMap.get(edge.toNodeId);
        // if distance is set,not set again
        if (!childNode || childNode.distance != -5 || !this.filterForBpf(node, edge)) {
          continue;
        }
        childNode.distance = distance;
        nodesToVisit[nodesToVisitLen++] = childNode.id;
      }
    }
    if (nodesToVisitLen > this.nodeCount) {
      throw new Error('BFS failed. Nodes to visit ' + nodesToVisitLen + ' is more than nodes count ' + this.nodeCount);
    }
  }

  private processNode(domState: DOMState, node: HeapNode, newState: number) {
    if (domState.visited[node.nodeIndex]) {
      return;
    }
    if (node.type !== NodeType.NATIVE) {
      domState.visited[node.nodeIndex] = 1;
      return;
    }

    node.detachedness = newState;

    if (newState === DetachedNessState.ATTACHED) {
      domState.attached.push(node.id);
    } else if (newState === DetachedNessState.DETACHED) {
      node.displayName = 'Detached ' + node.name;
      // mark detached dom
      node.flag |= DETACHED_DOM_NODE;
      domState.detached.push(node.id);
    }
    domState.visited[node.nodeIndex] = 1;
  }

  /**
   * Iterates children of a node.
   */
  private iterateFilteredChildren(domState: DOMState, node: HeapNode, newState: number): void {
    for (let edge of node.edges) {
      if (edge.type === EdgeType.HIDDEN || edge.type === EdgeType.WEAK || edge.type === EdgeType.INVISIBLE) {
        continue;
      }
      let childNode = this.nodeMap.get(edge.toNodeId);
      if (childNode) {
        this.processNode(domState, childNode, newState);
      }
    }
  }

  /**
   * mark the node can reachable from root node
   */
  private markQueryableNodes(): void {
    let list = new Array<HeapNode>();
    let flag = CAN_BE_QUERIED;
    for (let edge of this.rootNode!.edges) {
      let childNode = this.nodeMap.get(edge.toNodeId);
      if (childNode && childNode.isUserRoot()) {
        list.push(childNode);
      }
    }

    while (list.length) {
      let node = list.pop() as HeapNode;
      if (!node || node.flag & flag) {
        continue;
      }
      node.flag |= flag;
      for (let edge of node.edges) {
        let childNode = this.nodeMap.get(edge.toNodeId);
        if (
          !childNode ||
          childNode.flag & flag ||
          // @ts-ignore
          [EdgeType.HIDDEN, EdgeType.INVISIBLE, EdgeType.INTERNAL, EdgeType.WEAK].includes(edge.type)
        ) {
          continue;
        }
        list.push(childNode);
      }
    }
  }

  private markPageOwnedNodes(): void {
    let nodesToVisitLen = 0;
    let nodesToVisit = new Array<HeapNode>(this.nodeCount);
    let flag = PAGE_PROJECT;
    if (!this.rootNode) {
      return;
    }

    for (let edge of this.rootNode.edges) {
      let node = this.nodeMap.get(edge.toNodeId);
      if (!node) {
        continue;
      }
      if (edge.type === EdgeType.ELEMENT) {
        if (!node.isDocumentDOMTreesRoot()) {
          continue;
        }
      } else if (edge.type !== EdgeType.SHORTCUT) {
        continue;
      }
      nodesToVisit[nodesToVisitLen++] = node;
      node.flag |= flag;
    }

    while (nodesToVisitLen) {
      let node = nodesToVisit[--nodesToVisitLen];
      for (let edge of node.edges) {
        let childNode = this.nodeMap.get(edge.toNodeId);
        if (!childNode || childNode.flag & flag || edge.type === EdgeType.WEAK) {
          continue;
        }
        nodesToVisit[nodesToVisitLen++] = childNode;
        childNode.flag |= flag;
      }
    }
  }

  private isEssentialEdge(edge: HeapEdge, nodeId: number): boolean {
    return edge.type !== EdgeType.WEAK && (edge.type !== EdgeType.SHORTCUT || nodeId === this.rootNode!.id);
  }

  private hasOnlyWeakRetainers(node: HeapNode): boolean {
    let retainerStart = this.firstRetainerIndex[node.nodeIndex];
    let retainerEnd = this.firstRetainerIndex[node.nodeIndex + 1];
    for (let index = retainerStart; index < retainerEnd; index++) {
      let retainingEdgeIdx = this.retainingEdges[index];
      let edge = this.edges[retainingEdgeIdx];
      if (edge.type !== EdgeType.WEAK && edge.type !== EdgeType.SHORTCUT) {
        return false;
      }
    }
    return true;
  }

  private calClassDiff(targetClass: ConstructorItem, baseClass?: ConstructorItem) {
    let diff = new ConstructorComparison();
    diff.type = ConstructorType.ComparisonType;
    diff.fileId = this.fileId;
    diff.targetFileId = targetClass.fileId;
    diff.nodeName = targetClass.nodeName;
    let i = 0;
    let j = 0;
    let baseLen = baseClass ? baseClass.childCount : 0;
    let targetLen = targetClass.childCount;
    targetClass.classChildren.sort((a, b) => a.id - b.id);
    if (baseClass) {
      baseClass.classChildren.sort((a, b) => a.id - b.id);
    }
    // The overlap between the base class and the target class
    while (i < targetLen && j < baseLen) {
      let baseNode = baseClass!.classChildren[j];
      let targetNode = targetClass.classChildren[i];
      if (targetNode.id < baseNode.id) {
        diff.deletedIdx.push(targetNode.index);
        diff.removedCount++;
        diff.removedSize += targetNode.shallowSize;
        i++;
      } else if (targetNode.id > baseNode.id) {
        diff.addedIndx.push(baseNode.index);
        diff.addedCount++;
        diff.addedSize += baseNode.shallowSize;
        j++;
      } else {
        i++;
        j++;
      }
    }
    // base more then target
    while (i < targetLen) {
      let targetNode = targetClass!.classChildren[i];
      diff.deletedIdx.push(targetNode.index);
      diff.removedCount++;
      diff.removedSize += targetNode.shallowSize;
      i++;
    }
    //
    while (j < baseLen) {
      let baseNode = baseClass!.classChildren[j];
      diff.addedIndx.push(baseNode.index);
      diff.addedCount++;
      diff.addedSize += baseNode.shallowSize;
      j++;
    }
    diff.deltaCount = diff.addedCount - diff.removedCount;
    diff.deltaSize = diff.addedSize - diff.removedSize;
    if (diff.addedCount == 0 && diff.removedCount == 0) {
      return null;
    }
    diff.childCount = diff.addedCount + diff.removedCount;
    diff.hasNext = true;
    return diff;
  }

  public getClassesForSummary(minNodeId?: number, maxNodeId?: number): Map<string, ConstructorItem> {
    let hasFiler = typeof minNodeId === 'number' && typeof maxNodeId === 'number';
    function filter(nodeId: number): boolean {
      if (hasFiler) {
        if (hasFiler && nodeId >= minNodeId! && nodeId <= maxNodeId!) {
          return true;
        } else {
          return false;
        }
      } else {
        return true;
      }
    }
    if (!hasFiler && this.allClasses) {
      return this.allClasses;
    }
    let classes = new Map<string, ConstructorItem>();
    // combine node with className
    for (let node of this.nodes) {
      if (!filter(node.id) || (node.selfSize === 0 && node.type !== NodeType.NATIVE)) {
        continue;
      }

      if (!classes.has(node.className())) {
        let classItem = HeapNodeToConstructorItem(node);
        classItem.fileId = this.fileId;
        classItem.childCount = 1;
        classItem.type = ConstructorType.ClassType;
        classItem.retainedSize = 0;
        classItem.nodeName = node.className();
        classItem.hasNext = true;
        classes.set(node.className(), classItem);

        let instanceItem = classItem.clone();
        instanceItem.type = ConstructorType.InstanceType;
        instanceItem.id = node.id;
        instanceItem.index = node.nodeIndex;
        instanceItem.childCount = node.edgeCount;
        instanceItem.retainedSize = node.retainedSize;
        instanceItem.hasNext = instanceItem.childCount > 0;
        instanceItem.traceNodeId = node.traceNodeId;
        classItem.classChildren.push(instanceItem);
      } else {
        let classItem = classes.get(node.className());
        if (!classItem) {
          continue;
        }
        // set min node distance to class distance
        classItem.distance = Math.min(classItem.distance, node.distance);
        ++classItem.childCount;
        classItem.shallowSize += node.selfSize;

        let nodeItem = HeapNodeToConstructorItem(node);
        nodeItem.fileId = this.fileId;
        nodeItem.type = ConstructorType.InstanceType;
        nodeItem.id = node.id;
        nodeItem.index = node.nodeIndex;
        nodeItem.childCount = node.edgeCount;
        nodeItem.hasNext = nodeItem.childCount > 0;

        classItem.classChildren.push(nodeItem);
      }
    }

    // cal class retained size
    let list = [this.rootNode];
    const sizes = [-1];
    const classesName = [];
    let seenClassName = new Map<string, boolean>();

    while (list.length) {
      let node = list.pop();
      if (!node) {
        continue;
      }
      let nodeClassName = node.className();
      let seen = Boolean(seenClassName.get(nodeClassName));
      let dominatorFromIdx = this.firstDominatedNodesIdx[node.nodeIndex];
      let dominatorToIdx = this.firstDominatedNodesIdx[node.nodeIndex + 1];

      if (!seen && (!hasFiler || filter(node.id)) && (node.selfSize || node.type === NodeType.NATIVE)) {
        let classItem = classes.get(nodeClassName);
        if (classItem) {
          classItem.retainedSize += node.retainedSize;
          if (dominatorFromIdx !== dominatorToIdx) {
            seenClassName.set(nodeClassName, true);
            sizes.push(list.length);
            classesName.push(nodeClassName);
          }
        }
      }

      for (let idx = dominatorFromIdx; idx < dominatorToIdx; idx++) {
        let nodeIdx = this.dominatedNodes[idx];
        let domNode = this.nodes[nodeIdx];
        list.push(domNode);
      }

      while (sizes[sizes.length - 1] === list.length) {
        sizes.pop();
        nodeClassName = classesName.pop() as string;
        seenClassName.set(nodeClassName, false);
      }
    }
    if (!hasFiler) {
      this.allClasses = classes;
    }
    return classes;
  }

  /**
   * compare base file and target file, calculate delta size and count to target class
   * @param targetFileId to compare file's id
   * @param targetFileClasses to compare file's constructor
   */
  public getClassesForComparison(targetFileId: number, targetFileClasses: Map<string, ConstructorItem>) {
    // Return the result if it has been obtained before
    if (this.diffToOtherFile.has(targetFileId)) {
      return this.diffToOtherFile.get(targetFileId);
    }
    // get base file class if not init before
    if (!this.allClasses) {
      this.allClasses = this.getClassesForSummary();
    }

    //deal target class
    let diffMap = new Map<string, ConstructorComparison>();
    for (let targetClass of targetFileClasses.values()) {
      let classes = this.allClasses.get(targetClass.nodeName);
      let different = this.calClassDiff(targetClass, classes);
      if (different) {
        diffMap.set(targetClass.nodeName, different);
      }
    }

    // deal base class which is not in target
    for (let classItem of this.allClasses.values()) {
      if (targetFileClasses.has(classItem.nodeName)) {
        continue;
      }
      let different = this.calClassDiff(classItem);
      if (different) {
        diffMap.set(classItem.nodeName, different);
      }
    }
    this.diffToOtherFile.set(targetFileId, diffMap);
    return diffMap;
  }

  /**
   * Summary get Node Children
   * @param item select Node
   * @returns child Nodes
   */
  public getNextNode(item: ConstructorItem): Array<ConstructorItem> {
    if (item.children.length > 0) {
      return item.children;
    }
    // get children from edge
    let node = this.nodes[item.index];
    let childNodes = new Array<ConstructorItem>();
    for (let edge of node.edges) {
      let childNode = this.nodeMap.get(edge.toNodeId);
      if (!childNode) {
        continue;
      }
      let instanceItem = HeapNodeToConstructorItem(childNode);
      instanceItem.childCount = instanceItem.edgeCount = childNode.edgeCount;
      instanceItem.edgeName = edge.nameOrIndex;
      instanceItem.hasNext = instanceItem.childCount > 0;
      instanceItem.traceNodeId = childNode.traceNodeId;
      instanceItem.type = ConstructorType.FiledType;
      instanceItem.parent = item;
      childNodes.push(instanceItem);
    }

    let clickNode = childNodes[0].parent;
    // If there are duplicate IDs in the third layer and beyond, they will not be expanded again
    if (clickNode!.type == ConstructorType.FiledType) {
      this.findParentsFunc(childNodes, clickNode);
    }
    let filterChildNodes = new Array<ConstructorItem>();
    for (let item of childNodes) {
      if (item.id !== this.rootNode!.id) {
        filterChildNodes.push(item);
      }
    }
    item.children = filterChildNodes;
    return filterChildNodes;
  }

  private findParentsFunc(childNodes: ConstructorItem[], clickNode: ConstructorItem) {
    function findParents(clickNode: any, parents: any): any {
      if (!clickNode.parent) {
        return parents;
      }
      // add the parent of the current node to the result array
      parents.push(clickNode);
      for (let childNode of childNodes) {
        for (let heapParent of parents) {
          if (heapParent.id === childNode!.id) {
            childNode.hasNext = false;
          }
        }
      }
      return findParents(clickNode.parent, parents);
    }
    findParents(clickNode, []);
  }

  /**
   * get nodes which referenced this node
   * @param constructor current node
   * @returns reference nodes
   */
  public getRetains(item: ConstructorItem): Array<ConstructorItem> {
    if (item.retains.length > 0) {
      return item.retains;
    }
    if (item.type === ConstructorType.ClassType) {
      return [];
    }
    let node = this.nodes[item.index];
    let retains = new Array<ConstructorItem>();
    if (node && node.retainsEdgeIdx.length === node.retainsNodeIdx.length) {
      for (let i = 0; i < node.retainsNodeIdx.length; i++) {
        let retainsNode = this.nodes[node.retainsNodeIdx[i]];
        let retainEdge = this.edges[node.retainsEdgeIdx[i]];

        if (retainEdge.type == EdgeType.WEAK) {
          continue;
        }
        let retainsItem = HeapNodeToConstructorItem(retainsNode);
        retainsItem.edgeName = retainEdge.nameOrIndex;
        retainsItem.edgeType = retainEdge.type;
        retainsItem.type = ConstructorType.RetainersType;
        retainsItem.childCount = retainsNode.retainsNodeIdx.length;
        retainsItem.hasNext = retainsNode.retainsNodeIdx.length > 0;
        if (item!.type == ConstructorType.RetainersType) {
          retainsItem.parent = item;
        }
        retains.push(retainsItem);
      }
    }

    // Because the node with id 1 needs to be deleted, there is only one child and id 1 does not need to expand the symbol
    for (let childNode of retains) {
      let node = this.nodes[childNode.index];
      if (node && node.retainsEdgeIdx.length === node.retainsNodeIdx.length) {
        for (let i = 0; i < node.retainsNodeIdx.length; i++) {
          let retainsNode = this.nodes[node.retainsNodeIdx[i]];
          if (node.retainsNodeIdx.length == 1 && retainsNode.id == this.rootNode!.id) {
            childNode.hasNext = false;
          }
        }
      }
    }

    if (retains.length > 0 && retains[0].parent) {
      let clickNode = retains[0].parent;
      // If there are duplicate IDs in the third layer and beyond, they will not be expanded again
      if (clickNode!.type == ConstructorType.RetainersType) {
        this.findParentsFunc(retains, clickNode);
      }
    }

    retains.sort(function (a: any, b: any) {
      return a.distance - b.distance;
    });

    let filterRetains = new Array<ConstructorItem>();
    for (let item of retains) {
      if (item.id !== this.rootNode!.id) {
        filterRetains.push(item);
      }
    }
    return filterRetains;
  }

  public getNodes(): Array<HeapNode> {
    return this.nodes;
  }

  public clear() {
    this.allClasses?.clear();
    this.diffToOtherFile.clear();
    this.nodeMap.clear();
    this.edges.length = 0;
    this.nodes.length = 0;
  }
}

class DOMState {
  visited: Uint8Array;
  attached: Array<number>;
  detached: Array<number>;

  constructor(nodeSize: number) {
    this.visited = new Uint8Array(nodeSize);
    this.attached = new Array<number>();
    this.detached = new Array<number>();
  }
}
