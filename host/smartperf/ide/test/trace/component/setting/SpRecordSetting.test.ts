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
import { SpRecordSetting } from '../../../../dist/trace/component/setting/SpRecordSetting.js';

describe('SpRecordSetting Test', () => {
  beforeAll(() => {
    document.body.innerHTML = `
            <record-setting id = "setting"><sp-allocations>
        `;
  });
  it('new SpRecordSetting', function () {
    expect(new SpRecordSetting()).not.toBeNull();
  });

  it(' SpAllocations get Default attrValue', function () {
    let spEle = document.querySelector('#setting') as SpRecordSetting;
    expect(spEle.recordMod).toBeTruthy();
    expect(spEle.bufferSize).toEqual(64);
    expect(spEle.maxDur).toEqual(30);
  });

  it(' SpRecordSettingTest04', function () {
    let spEle = document.querySelector('#setting') as SpRecordSetting;
    expect(spEle.resetValue()).toBeUndefined();
  });
});
