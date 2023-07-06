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
import { HeapDataInterface } from '../../dist/js-heap/HeapDataInterface.js';
//@ts-ignore
import { HeapNode } from '../../dist/js-heap/model/DatabaseStruct.js';
jest.mock('../../dist/js-heap/model/DatabaseStruct.js', () => {});
// @ts-ignore
window.ResizeObserver =
    window.ResizeObserver ||
    jest.fn().mockImplementation(() => ({
        disconnect: jest.fn(),
        observe: jest.fn(),
        unobserve: jest.fn(),
    }));
jest.mock('../../dist/js-heap/utils/Utils.js', () => {
    return {
        HeapNodeToConstructorItem: (node: HeapNode) => {},
    };
});
jest.mock('../../dist/js-heap/model/DatabaseStruct.js', () => {});

describe('HeapDataInterface Test', () => {
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
            traceNodes: [],
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
        },
    };
    it('HeapDataInterface01', () => {
        let heapDataInterface = new HeapDataInterface();
        heapDataInterface.fileStructs = [data];
        expect(heapDataInterface.getClassesListForSummary(1, 0, 0).length).toBe(0);
    });
    it('HeapDataInterface02', () => {
        let heapDataInterface = new HeapDataInterface();
        heapDataInterface.fileStructs = [data];
        expect(heapDataInterface.setFileId(1)).toBeFalsy();
    });
    it('HeapDataInterface03', () => {
        let heapDataInterface = new HeapDataInterface();
        heapDataInterface.fileStructs = [data];
        expect(heapDataInterface.setPraseListener({})).toBeFalsy();
    });
    it('HeapDataInterface04', () => {
        let heapDataInterface = new HeapDataInterface();
        heapDataInterface.fileStructs = [data];
        expect(heapDataInterface.getClassesListForSummary(1, 1, 1)).not.toBeUndefined();
    });
    it('HeapDataInterface05', () => {
        let heapDataInterface = new HeapDataInterface();
        heapDataInterface.fileStructs = [data];
        expect(heapDataInterface.getClassListForComparison(1, 2)).toEqual([]);
    });
    it('HeapDataInterface06', () => {
        let heapDataInterface = new HeapDataInterface();
        heapDataInterface.fileStructs = [data];
        expect(heapDataInterface.getParentFunction({})).toBeUndefined();
    });
    it('HeapDataInterface07', () => {
        let heapDataInterface = new HeapDataInterface();
        heapDataInterface.fileStructs = [data];
        expect(heapDataInterface.getNextForConstructor({})).not.toBeUndefined();
    });
    it('HeapDataInterface08', () => {
        let heapDataInterface = new HeapDataInterface();
        heapDataInterface.fileStructs = [data];
        expect(heapDataInterface.getNextForComparison({})).not.toBeUndefined();
    });
    it('HeapDataInterface09', () => {
        let heapDataInterface = new HeapDataInterface();
        heapDataInterface.fileStructs = [data];
        expect(heapDataInterface.getRetains({})).not.toBeUndefined();
    });
    it('HeapDataInterface10', async () => {
        let heapDataInterface = new HeapDataInterface();
        heapDataInterface.fileStructs = [data];
        expect(await heapDataInterface.parseData([])).toBeUndefined();
    });
    it('HeapDataInterface11', () => {
        let heapDataInterface = new HeapDataInterface();
        heapDataInterface.fileStructs = [data];
        expect(heapDataInterface.getFileStructs().length).toBe(1);
    });
    it('HeapDataInterface12', () => {
        let heapDataInterface = new HeapDataInterface();
        heapDataInterface.fileStructs = [data];
        expect(heapDataInterface.getSamples(1)).toStrictEqual([]);
    });
    it('HeapDataInterface13', () => {
        let heapDataInterface = new HeapDataInterface();
        heapDataInterface.fileStructs = [data];
        let node = {
            fileId:1,
            type:0,
            traceNodeId:1
        }
        expect(heapDataInterface.getAllocationStackData(node)).toStrictEqual([]);
    });
    it('HeapDataInterface14', () => {
        let heapDataInterface = new HeapDataInterface();
        heapDataInterface.fileStructs = [data];
        expect(heapDataInterface.getMinNodeId(1)).toBeUndefined();
    });
    it('HeapDataInterface15', () => {
        let heapDataInterface = new HeapDataInterface();
        heapDataInterface.fileStructs = [data];
        expect(heapDataInterface.getMaxNodeId(1)).toBeUndefined();
    });
});
