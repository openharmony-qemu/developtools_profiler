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
        id: 3,
        isParseSuccess: true,
        name: 'Test',
        path: '',
        pid: 4243,
        tart_ts: 88473061693464,
        type: 3,
        heapLoader: {
            rootNode: {
                detachedness: 3,
                displayName: '',
                distance: 100000000,
                edgeCount: 35375,
                fileId: 30,
                firstEdgeIndex: 0,
                flag: 0,
                id: 1,
                name: 'Test',
                nodeIndex: 30,
                nodeOldIndex: 0,
                retainedSize: 1838167,
                retainsCount: 0,
                retainsEdgeIdx: [0],
                retainsNodeIdx: [0],
                selfSize: 30,
                traceNodeId: 0,
                type: 9,
                edges: [
                    {
                        edgeIndex: 2,
                        edgeOldIndex: 2,
                        fromNodeId: 4,
                        nameOrIndex: '-test-',
                        nodeId: 1523400,
                        retainEdge: [],
                        retainsNode: [],
                        toNodeId: 44500,
                        type: 6,
                    },
                    {
                        edgeIndex: 6,
                        edgeOldIndex: 3,
                        fromNodeId: 1,
                        nameOrIndex: '-test-',
                        nodeId: 233330,
                        retainEdge: [],
                        retainsNode: [],
                        toNodeId: 32405,
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
                    edgeIndex: 22,
                    edgeOldIndex: 32,
                    fromNodeId: 111,
                    nameOrIndex: '-test-',
                    nodeId: 13331,
                    retainEdge: [],
                    retainsNode: [],
                    toNodeId: 133,
                    type: 32,
                },
                {
                    edgeIndex: 32,
                    edgeOldIndex: 231,
                    fromNodeId: 231,
                    nameOrIndex: '-test-',
                    nodeId: 42321,
                    retainEdge: [],
                    retainsNode: [],
                    toNodeId: 21201,
                    type: 95,
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
