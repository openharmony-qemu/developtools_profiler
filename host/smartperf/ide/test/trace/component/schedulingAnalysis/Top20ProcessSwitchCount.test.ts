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
import { Top20ProcessSwitchCount } from '../../../../dist/trace/component/schedulingAnalysis/Top20ProcessSwitchCount.js';
// @ts-ignore
window.ResizeObserver = window.ResizeObserver || jest.fn().mockImplementation(() => ({
    disconnect: jest.fn(),
    observe: jest.fn(),
    unobserve: jest.fn(),
}));

describe('Top20ProcessSwitchCount Test', () => {
    it('Top20ProcessSwitchCountTest01', () => {
        let top20ProcessSwitchCount = new Top20ProcessSwitchCount();
        expect(top20ProcessSwitchCount).not.toBeUndefined();
    });
    it('Top20ProcessSwitchCountTest02', () => {
        let top20ProcessSwitchCount = new Top20ProcessSwitchCount();
        expect(
            top20ProcessSwitchCount.sortByColumn({
                key: 'number',
            })
        ).toBeUndefined();
    });
    it('Top20ProcessSwitchCountTest03', () => {
        let top20ProcessSwitchCount = new Top20ProcessSwitchCount();
        top20ProcessSwitchCount.queryLogicWorker = jest.fn();
        expect(top20ProcessSwitchCount.queryLogicWorker('','',{})).toBeUndefined();
    });
})