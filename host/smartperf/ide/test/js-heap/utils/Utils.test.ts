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

// @ts-ignore
import { HeapNodeToConstructorItem } from "../../../dist/js-heap/utils/Utils.js";
// @ts-ignore
import {HeapNode} from "../../../dist/js-heap/model/DatabaseStruct.js";
jest.mock('../../../dist/js-heap/model/DatabaseStruct.js', () => {});

jest.mock('../../../dist/js-heap/HeapDataInterface.js', () => {
    return {};
});
jest.mock('../../../dist/js-heap/model/DatabaseStruct.js', () => {});

describe('Utils Test', () => {
    it('HeapNodeToConstructorItemTest01', () => {
        let data = {
            fileId: 1,
            nodeIndex: 1,
            nodeOldIndex: 1,
            type: '',
            name: '',
            id: 1,
            selfSize: 1,
            edgeCount: 1,
            traceNodeId: 1,
            detachedness: 1,
            edges: '',
            distance: -5,
            retainedSize: 1,
            displayName: '',
            firstEdgeIndex: 1,
            flag: 1,
            retainsCount: 0,
            retainsEdgeIdx: [],
            retainsNodeIdx: [],
        }
        // @ts-ignore
        data.nodeName = jest.fn(() => 'test');
        HeapNodeToConstructorItem(data);
    });
})