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
                };
            },
        },
        clone: () => {
            return {
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
            };
        },
    };
});

describe('Allocation Test', () => {
    let set: Set<number> = new Set([]);
    let data = {
        end_ts: 88473497504466,
        id: 0,
        isParseSuccess: true,
        name: 'Test',
        path: '',
        pid: 4243,
        tart_ts: 88473061693464,
        type: 0,
        heapLoader: {
            rootNode: {
                detachedness: 0,
                displayName: '',
                distance: 100000000,
                edgeCount: 3575,
                fileId: 0,
                firstEdgeIndex: 0,
                flag: 0,
                id: 1,
                name: 'Test',
                nodeIndex: 0,
                nodeOldIndex: 0,
                retainedSize: 1898167,
                retainsCount: 0,
                retainsEdgeIdx: [0],
                retainsNodeIdx: [0],
                selfSize: 0,
                traceNodeId: 0,
                type: 9,
                edges: [
                    {
                        edgeIndex: 0,
                        edgeOldIndex: 0,
                        fromNodeId: 1,
                        nameOrIndex: '-test-',
                        nodeId: 152376,
                        retainEdge: [],
                        retainsNode: [],
                        toNodeId: 43537,
                        type: 5,
                    },
                    {
                        edgeIndex: 1,
                        edgeOldIndex: 3,
                        fromNodeId: 1,
                        nameOrIndex: '-test-',
                        nodeId: 155414,
                        retainEdge: [],
                        retainsNode: [],
                        toNodeId: 44405,
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
                    fileId: 0,
                    functionIndex: 22,
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
            ],
            nodeMap: new Map(),
            nodeCount: 1,
            edges: [
                {
                    edgeIndex: 0,
                    edgeOldIndex: 0,
                    fromNodeId: 1,
                    nameOrIndex: '-test-',
                    nodeId: 152376,
                    retainEdge: [],
                    retainsNode: [],
                    toNodeId: 43537,
                    type: 5,
                },
                {
                    edgeIndex: 1,
                    edgeOldIndex: 3,
                    fromNodeId: 1,
                    nameOrIndex: '-test-',
                    nodeId: 155414,
                    retainEdge: [],
                    retainsNode: [],
                    toNodeId: 44405,
                    type: 5,
                },
            ],
            samples: [],
            functionInfos: [1],
        },
    };
    it('AllocationTest01', () => {
        let allocationLogic = new AllocationLogic(data);
        let nodeById = allocationLogic.getNodeById(12);
        expect(nodeById).not.toBe(null);
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
            fileId: 0,
            functionIndex: 0,
            parentsId: [1],
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
