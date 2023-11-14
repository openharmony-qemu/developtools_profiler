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
import { SpProbesConfig } from '../../../../dist/trace/component/setting/SpProbesConfig.js';

describe('SpProbesConfig Test', () => {
  beforeAll(() => {
    document.body.innerHTML = `
            <probes-config id = "spconfig"><probes-config>
        `;
  });
  it('new SpProbesConfig', function () {
    expect(new SpProbesConfig()).not.toBeNull();
  });

  it(' SpProbesConfig get Default attrValue', function () {
    let spEle = document.querySelector('#spconfig') as SpProbesConfig;
    expect(spEle.traceConfig).toEqual(['Scheduling details', 'CPU Frequency and idle states', 'Hitrace categories']);
    expect(spEle.traceEvents.length).toEqual(24);
    expect(spEle.memoryConfig).toEqual([]);
  });
  it('new SpProbesConfig02', function () {
    let spEle = document.querySelector('#spconfig') as SpProbesConfig;
    expect(spEle.recordAbility).toBeFalsy();
  });
});
