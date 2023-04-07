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

export class objectToMemorySize{
    private seen = new WeakSet

    /**
     * objectToSize
     *
     * number：8 bytes (64-bit storage)
     * String：一characters 2 bytes
     * Boolean：4 bytes
     *
     * @param object
     */
    objectToSize(object: any): any{
        const objType = typeof object
        switch (objType) {
            case 'string':
                return object.length * 2
            case 'boolean':
                return 4
            case 'number':
                return 8
            case 'object':
                if (Array.isArray(object)) {
                    return object.map(this.objectToSize).reduce((res, cur) => res + cur, 0)
                } else {
                    return this.sizeOfObj(object)
                }
            default:
                return 0
        }
    }

    sizeOfObj(object: any): number{
        if (object === null) return 0

        let bytes = 0
        // The key in the object also takes up memory space
        const props = Object.keys(object)
        for (let i = 0; i < props.length; i++) {
            const key = props[i]
            // Whether the value is repeated or not, the key needs to be calculated
            bytes += this.objectToSize(key)
            if (typeof object[key] === 'object' && object[key] !== null) {
                // 这里需要注意value使用相同内存空间（只需计算一次内存）
                if (this.seen.has(object[key])) continue
                this.seen.add(object[key])
            }
            bytes += this.objectToSize(object[key])
        }
        return bytes
    }
}

