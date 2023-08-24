
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
import { HeapLoader } from '../../../dist/js-heap/logic/HeapLoader.js';
//@ts-ignore
import { ConstructorItem, FileType } from '../../../dist/js-heap/model/UiStruct.js';

jest.mock('../../../dist/js-heap/model/DatabaseStruct.js', () => ({
    DetachedNessState: {
        UNKNOWN: 0,
        ATTACHED: 1,
        DETACHED: 2,
    },
    NodeType: {
        HIDDEN : 0,
        ARRAY : 1,
        STRING : 2,
        OBJECT : 3,
        CODE : 4,
        CLOSURE : 5,
        REGEXP : 6,
        NUMBER : 7,
        NATIVE : 8,
        SYNTHETIC : 9,
        CONCATENATED_STRING : 10,
        SLICED_STRING : 11,
        SYMBOL : 12,
        BIGINT : 13,
        OBJECT_SHAPE : 14,
    },
    EdgeType: {
        CONTEXT : 0,
        ELEMENT : 1,
        PROPERTY : 2,
        INTERNAL : 3,
        HIDDEN : 4,
        SHORTCUT : 5,
        WEAK : 6,
        STRING_OR_NUMBER : 6,
        NODE : 7,
        INVISIBLE : 8,
    }
}))

jest.mock('../../../dist/js-heap/utils/Utils.js', () => {
    return {
        HeapNodeToConstructorItem: (node: any) => {
            return {};
        },
    };
});

describe('HeapLoader Test', () => {
    let rootNode = {
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
    };
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
            rootNode: rootNode,
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

    let item = {
        addedCount: 648,
        addedIndx: [319, 326],
        addedSize: 38936,
        childCount: 1296,
        children: [],
        classChildren: [],
        deletedIdx: [325, 338],
        deltaCount: 0,
        deltaSize: -16,
        distance: -1,
        edgeCount: 0,
        edgeName: '',
        fileId: 0,
        hasNext: true,
        id: -1,
        index: 0,
        isAdd: false,
        isHover: false,
        isSelected: false,
        nextId: [],
        nodeName: 'nodeName',
        objectName: 'objectName',
        removedCount: 648,
        removedSize: 38952,
        retainedSize: -1,
        retains: [],
        shallowSize: -1,
        showBox: false,
        showCut: false,
        status: true,
        targetFileId: 1,
        traceNodeId: -1,
        type: 4,
    };

    it('HeapLoaderTest01', () => {
        let heapLoader = new HeapLoader(data);
        heapLoader.fileId = jest.fn(() => true);
        expect(heapLoader).not.toBeUndefined();
    });
    it('HeapLoaderTest02', () => {
        let heapLoader = new HeapLoader(data);
        expect(heapLoader.loadAllocationParent({})).toBeUndefined();
    });

    it('HeapLoaderTest03', () => {
        let heapLoader = new HeapLoader(data);
        heapLoader.rootNode = rootNode;
        heapLoader.nodes = [rootNode];
        heapLoader.nodes[0].addEdge = jest.fn(() => true);
        heapLoader.isEssentialEdge = jest.fn(() => false);
        expect(heapLoader.preprocess()).toBeUndefined();
    });

    it('HeapLoaderTest04', () => {
        let heapLoader = new HeapLoader(data);
        heapLoader.nodes = [rootNode];
        expect(heapLoader.getClassesForSummary(1,2)).toBeTruthy();
    });

    it('HeapLoaderTest05', () => {
        let heapLoader = new HeapLoader(data);
        heapLoader.nodes = [rootNode];
        heapLoader.rootNode = rootNode;
        expect(heapLoader.getRetains(item)).toBeDefined();
    });
    it('HeapLoaderTest06', () => {
        let heapLoader = new HeapLoader(data);
        expect(heapLoader.getAllocationFunctionList()).toStrictEqual([]);
    });
    it('HeapLoaderTest07', () => {
        let heapLoader = new HeapLoader(data);
        expect(heapLoader.getAllocationStack(1)).toStrictEqual([]);
    });
    it('HeapLoaderTest08', () => {
        let heapLoader = new HeapLoader(data);
        expect(heapLoader.getFunctionNodeIds(1)).toStrictEqual([]);
    });
    it('HeapLoaderTest09', () => {
        let heapLoader = new HeapLoader(data);
        expect(heapLoader.calDistances()).toBeUndefined();
    });
    it('HeapLoaderTest10', () => {
        let heapLoader = new HeapLoader(data);
        heapLoader.buildOrderIdxAndDominateTree = jest.fn(()=>{true});
        expect(heapLoader.calRetainedSize()).toBeUndefined();
    });
    it('HeapLoaderTest11', () => {
        let heapLoader = new HeapLoader(data);
        heapLoader.buildDominatedNode = jest.fn(()=>{true});
        expect(heapLoader.buildDominatedNode()).toStrictEqual();
    });
    it('HeapLoaderTest12', () => {
        let heapLoader = new HeapLoader(data);
        expect(heapLoader.buildSamples()).toBeUndefined();
    });
    it('HeapLoaderTest13', () => {
        let heapLoader = new HeapLoader(data);
        let samples = [
            {length: 1},
            {mid: 1}
        ]
        expect(heapLoader.binarySearchNodeInSamples(1, samples)).toBe(0);
    });
    it('HeapLoaderTest14', () => {
        let heapLoader = new HeapLoader(data);
        let node = {
            index:1,
            nodesToVisitLen:1
        }
        let edge = {
            nodesToVisitLen:3,
        }
        expect(heapLoader.bfs(node,edge)).toBeUndefined();
    });
    it('HeapLoaderTest15', () => {
        let heapLoader = new HeapLoader(data);
        expect(heapLoader.markPageOwnedNodes()).toBeUndefined();
    });
    it('HeapLoaderTest16', () => {
        let heapLoader = new HeapLoader(data);
        let node = {
            nodeIndex:1
        }
        expect(heapLoader.hasOnlyWeakRetainers(node)).toBe(true);
    });
    it('HeapLoaderTest17', () => {
        let heapLoader = new HeapLoader(data);
        let targetClass = {
            fileId:1,
            nodeName:'',
            childCount:2,
            classChildren:[],
        }
        let baseClass = {
            childCount:1,
            classChildren:[]
        }
        heapLoader.calClassDiff = jest.fn(()=>{true})
        expect(heapLoader.calClassDiff(targetClass,baseClass)).toBeUndefined();
    });
    it('HeapLoaderTest18', () => {
        let heapLoader = new HeapLoader(data);
        let item = {
            children:[{length: 1}],
            index:1,
            childCount:1,
            edgeName:'',
            hasNext:true,
            traceNodeId:1,
            type:0,
            parent:{},
            id:1
        }
        expect(heapLoader.getNextNode(item)).toStrictEqual([{"length": 1}]);
    });
    it('HeapLoaderTest19', () => {
        let heapLoader = new HeapLoader(data);
        let item = {
            retains:[{length: 1}],
            index:1,
            childCount:1,
            edgeName:'',
            hasNext:true,
            traceNodeId:1,
            type:0,
            parent:{},
            id:1
        }
        expect(heapLoader.getRetains(item)).toStrictEqual([{"length": 1}]);
    });
    it('HeapLoaderTest20', () => {
        let heapLoader = new HeapLoader(data);
        let node = {
            index:1,
            nodesToVisitLen:1
        }
        let edge = {
            nodesToVisitLen:3,
        }
        expect(heapLoader.filterForBpf(node,edge)).toBe(true);
    });
    it('HeapLoaderTest21', () => {
        let heapLoader = new HeapLoader(data);
        let datas = {
            visited:[
                {node:{nodeIndex:1}},
            ],
            attached:[],
            detached:[],
        }
        let node = {
            nodeIndex:1,
            type:1,
            detachedness:1,
            id:1,
            displayName: 'Detached ',
            name:'',
            flag:1
        }
        expect(heapLoader.processNode(datas,node,1)).toBeUndefined();
    });
});