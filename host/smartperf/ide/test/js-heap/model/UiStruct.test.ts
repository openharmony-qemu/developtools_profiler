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
import { ConstructorItem, ConstructorComparison,FileInfo,AllocationFunction } from '../../../dist/js-heap/model/UiStruct.js';

jest.mock('../../../dist/js-heap/logic/HeapLoader.js', () => {
    return {};
});
jest.mock('../../../dist/js-heap/model/DatabaseStruct.js', () => {});
describe('UiStruct Test', () => {
    it('UiStructTest01', () => {
        let constructorItem = new ConstructorItem();
        expect(constructorItem).not.toBeUndefined();
    });

    it('UiStructTest02', () => {
        let constructorComparison  = new ConstructorComparison();
        expect(constructorComparison).not.toBeUndefined();
    });

    it('UiStructTest02', () => {
        let constructorComparison  = new ConstructorComparison();
        expect(constructorComparison).not.toBeUndefined();
    });
    it('UiStructTest03', () => {
        let allocationFunction  = new AllocationFunction();
        expect(allocationFunction).not.toBeUndefined();
    });
    it('UiStructTest04', () => {
        let constructorItem = new ConstructorItem();
        expect(constructorItem.getChildren()).toStrictEqual([]);
    });
    it('UiStructTest05', () => {
        let constructorItem = new ConstructorItem();
        let data = [
            {
                fileId :1,
                distance:2,
                shallowSize:123,
                nodeName: '',
                edgeCount:1,
                edgeType:'',
                childCount:[],
                hasNext:true,
            }
            ]
        expect(constructorItem.cloneContent(data)).toBeUndefined();
    });
    it('UiStructTest06', () => {
        let constructorComparison  = new ConstructorComparison();
        expect(constructorComparison.getChildren()).toStrictEqual([]);
    });
    it('UiStructTest07', () => {
        let allocationFunction  = new AllocationFunction();
        expect(allocationFunction.getChildren()).toStrictEqual([]);
    });
})