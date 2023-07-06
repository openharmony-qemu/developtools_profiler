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
import { HeapNode,HeapEdge,HeapTraceFunctionInfo,HeapSample,HeapLocation,HeapSnapshotStruct,FileStruct } from '../../../dist/js-heap/model/DatabaseStruct.js';
// @ts-ignore
window.ResizeObserver = window.ResizeObserver || jest.fn().mockImplementation(() => ({
    disconnect: jest.fn(),
    observe: jest.fn(),
    unobserve: jest.fn(),
}));

describe('DatabaseStruct Test', () => {
    it('DatabaseStructTest01', () => {
        let heapNode = new HeapNode();
        expect(heapNode).not.toBeUndefined();
    });
    it('DatabaseStructTest02', () => {
        let heapEdge = new HeapEdge();
        expect(heapEdge).not.toBeUndefined();
    });
    it('DatabaseStructTest03', () => {
        let heapTraceFunctionInfo = new HeapTraceFunctionInfo();
        expect(heapTraceFunctionInfo).not.toBeUndefined();
    });
    it('DatabaseStructTest04', () => {
        let heapLocation = new HeapLocation();
        expect(heapLocation).not.toBeUndefined();
    });
    it('DatabaseStructTest05', () => {
        let heapSnapshotStruct = new HeapSnapshotStruct();
        expect(heapSnapshotStruct).not.toBeUndefined();
    });
    it('DatabaseStructTest06', () => {
        let fileStruct = new FileStruct();
        expect(fileStruct).not.toBeUndefined();
    });
    it('DatabaseStructTest07', () => {
        let heapNode = new HeapNode();
        expect(heapNode.className()).toBe('(undefined)');
    });
    it('DatabaseStructTest09', () => {
        let heapNode = new HeapNode();
        expect(heapNode.nodeName()).toBeUndefined();
    });
    it('DatabaseStructTest10', () => {
        let heapNode = new HeapNode();
        expect(heapNode.addEdge({})).toBeUndefined;
    });
    it('DatabaseStructTest11', () => {
        let heapNode = new HeapNode();
        expect(heapNode.idHidden()).toBeUndefined;
    });
    it('DatabaseStructTest12', () => {
        let heapNode = new HeapNode();
        expect(heapNode.isArray()).toBeUndefined;
    });
    it('DatabaseStructTest13', () => {
        let heapNode = new HeapNode();
        expect(heapNode.isUserRoot()).toBeUndefined;
    });
    it('DatabaseStructTest14', () => {
        let heapNode = new HeapNode();
        expect(heapNode.isDocumentDOMTreesRoot()).toBeUndefined;
    });
    it('DatabaseStructTest15', () => {
        let heapNode = new HeapSample();
        expect(HeapSample).not.toBeUndefined;
    });
})