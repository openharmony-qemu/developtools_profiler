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
//@ts-ignore
import { AllocationLogic } from '../../../dist/js-heap/logic/Allocation.js';
//@ts-ignore
import { AllocationFunction } from '../../../dist/js-heap/model/UiStruct.js';

jest.mock('../../../dist/js-heap/model/UiStruct.js', () => {
  return {
    AllocationFunction: {
      fileId: 0,
      functionIndex: 0,
      parentsId: [1, 2, 3],
      parents: [],
      combineId: new Set([]),
      status: true,
      id: '12',
      name: 'name1',
      scriptName: 'scriptName1',
      scriptId: 1,
      line: 1,
      column: 1,
      count: 1,
      size: 1,
      liveCount: 1,
      liveSize: 1,
      hasParent: true,
      clone: () => {
        return {
          fileId: 1,
          functionIndex: 324,
          parentsId: [10, 56, 3],
          parents: [],
          combineId: new Set([]),
          status: true,
          id: '21',
          name: 'name2',
          scriptName: 'scriptName2',
          scriptId: 21,
          line: 321,
          column: 3,
          count: 43,
          size: 45,
          liveCount: 11,
          liveSize: 43,
          hasParent: true,
        };
      },
    },
    clone: () => {
      return {
        fileId: 42,
        functionIndex: 44,
        parentsId: [1, 24],
        parents: [],
        combineId: new Set([]),
        status: true,
        id: '90',
        name: 'name3',
        scriptName: 'scriptName3',
        scriptId: 65,
        line: 6,
        column: 87,
        count: 23,
        size: 54,
        liveCount: 56,
        liveSize: 44,
        hasParent: true,
      };
    },
  };
});

describe('Allocation Test', () => {
  let set: Set<number> = new Set([]);
  let data = {
    end_ts: 342211,
    id: 3,
    isParseSuccess: true,
    name: 'Test',
    path: '',
    pid: 34,
    tart_ts: 8847320334,
    type: 1,
    heapLoader: {
      rootNode: {
        detachedness: 43,
        displayName: '',
        distance: 230000000,
        edgeCount: 432,
        fileId: 46,
        firstEdgeIndex: 54,
        flag: 5,
        id: 54,
        name: 'Test',
        nodeIndex: 0,
        nodeOldIndex: 0,
        retainedSize: 5411,
        retainsCount: 0,
        retainsEdgeIdx: [0, 1],
        retainsNodeIdx: [320, 49],
        selfSize: 0,
        traceNodeId: 0,
        type: 34,
        edges: [
          {
            edgeIndex: 5,
            edgeOldIndex: 31,
            fromNodeId: 1,
            nameOrIndex: '-test-',
            nodeId: 425,
            retainEdge: [],
            retainsNode: [],
            toNodeId: 1134,
            type: 25,
          },
          {
            edgeIndex: 54,
            edgeOldIndex: 35,
            fromNodeId: 3,
            nameOrIndex: '-test-',
            nodeId: 9800,
            retainEdge: [],
            retainsNode: [],
            toNodeId: 2311,
            type: 5,
          },
        ],
      },
    },
    snapshotStruct: {
      traceNodes: [
        {
          fileId: 0,
          functionIndex: 0,
          parentsId: [1, 2, 3],
          parents: [],
          combineId: set,
          status: true,
          id: '12',
          name: 'name1',
          scriptName: 'scriptName1',
          scriptId: 1,
          line: 1,
          column: 1,
          count: 1,
          size: 1,
          liveCount: 1,
          liveSize: 1,
          hasParent: true,
          clone: () => {
            return {
              fileId: 0,
              functionIndex: 0,
              parentsId: [1, 2, 3],
              parents: [],
              combineId: set,
              status: true,
              id: '12',
              name: 'name1',
              scriptName: 'scriptName1',
              scriptId: 1,
              line: 1,
              column: 1,
              count: 1,
              size: 1,
              liveCount: 1,
              liveSize: 1,
              hasParent: true,
            };
          },
        },
        {
          fileId: 65,
          functionIndex: 6,
          parentsId: [1, 65, 3],
          parents: [],
          combineId: set,
          status: true,
          id: '45',
          name: 'name6',
          scriptName: 'scriptName6',
          scriptId: 1,
          line: 64,
          column: 65,
          count: 66,
          size: 6,
          liveCount: 1,
          liveSize: 120,
          hasParent: true,
          clone: () => {
            return {
              fileId: 21,
              functionIndex: 33,
              parentsId: [54, 23],
              parents: [],
              combineId: set,
              status: true,
              id: '12',
              name: 'name7',
              scriptName: 'scriptName7',
              scriptId: 321,
              line: 341,
              column: 123,
              count: 21,
              size: 134,
              liveCount: 16,
              liveSize: 109,
              hasParent: true,
            };
          },
        },
        {
          fileId: 23,
          functionIndex: 1,
          parentsId: [111, 24, 3],
          parents: [],
          combineId: set,
          status: true,
          id: '4345',
          name: 'name7',
          scriptName: 'scriptName7',
          scriptId: 421,
          line: 13,
          column: 51,
          count: 61,
          size: 165,
          liveCount: 10,
          liveSize: 651,
          hasParent: true,
          clone: () => {
            return {
              fileId: 3,
              functionIndex: 57,
              parentsId: [1, 87, 31],
              parents: [],
              combineId: set,
              status: true,
              id: '251',
              name: 'name8',
              scriptName: 'scriptName8',
              scriptId: 91,
              line: 189,
              column: 891,
              count: 81,
              size: 14,
              liveCount: 431,
              liveSize: 21,
              hasParent: true,
            };
          },
        },
      ],
      nodeMap: new Map(),
      nodeCount: 1,
      edges: [
        {
          edgeIndex: 6,
          edgeOldIndex: 0,
          fromNodeId: 1,
          nameOrIndex: '-test-',
          nodeId: 67,
          retainEdge: [],
          retainsNode: [],
          toNodeId: 77,
          type: 61,
        },
        {
          edgeIndex: 16,
          edgeOldIndex: 37,
          fromNodeId: 16,
          nameOrIndex: '-test-',
          nodeId: 67,
          retainEdge: [],
          retainsNode: [],
          toNodeId: 876,
          type: 8,
        },
      ],
      samples: [],
      functionInfos: [1],
    },
  };
  it('AllocationTest01', () => {
    let allocationLogic = new AllocationLogic(data);
    let nodeById = allocationLogic.getNodeById(12);
    expect(nodeById).not.toBe();
  });

  it('AllocationTest02', () => {
    let allocationLogic = new AllocationLogic(data);
    let nodeById = allocationLogic.getNodeById(11);
    expect(nodeById).toBe(null);
  });
  it('AllocationTest03', () => {
    let allocationLogic = new AllocationLogic(data);
    let nodeStack = allocationLogic.getNodeStack(12);
    expect(nodeStack.length).toBe(1);
  });
  it('AllocationTest04', () => {
    let allocationLogic = new AllocationLogic(data);
    let nodeStack = allocationLogic.getFunctionNodeIds(12);
    expect(nodeStack).not.toBe([]);
  });
  it('AllocationTest05', () => {
    let allocationLogic = new AllocationLogic(data);
    let parentData = data.snapshotStruct.traceNodes[0];
    let nodeStack = allocationLogic.getParent(parentData);
    expect(nodeStack).toBeUndefined();
  });
  it('AllocationTest06', () => {
    let allocationLogic = new AllocationLogic(data);
    let parentData = {
      fileId: 23,
      functionIndex: 35,
      parentsId: [1],
      parents: [],
      combineId: set,
      status: true,
      id: '2450',
      name: 'name9',
      scriptName: 'scriptName9',
      scriptId: 981,
      line: 8341,
      column: 21,
      count: 211,
      size: 134,
      liveCount: 541,
      liveSize: 176,
      hasParent: true,
    };
    let nodeStack = allocationLogic.getParent(parentData);
    expect(nodeStack).toBeUndefined();
  });

  it('AllocationTest07', () => {
    let allocationLogic = new AllocationLogic(data);
    let nodeStack = allocationLogic.getFunctionList();
    expect(nodeStack.length).not.toEqual(0);
  });

  it('AllocationTest08', () => {
    let allocationLogic = new AllocationLogic(data);
    let parentData = data.snapshotStruct.traceNodes[0];
    let nodeStack = allocationLogic.getFunctionStack(parentData, []);
    expect(nodeStack).toBeUndefined();
  });
});
