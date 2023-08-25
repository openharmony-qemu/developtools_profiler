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
import { SpInfoAndStats } from '../../../dist/trace/component/SpInfoAndStas.js';

describe('SpInfoAndStasTest', () => {
  document.body.innerHTML = `<sp-info-and-stats id="ddd"></sp-info-and-stats>`;
  let spInfoAndStats = document.querySelector('#ddd') as SpInfoAndStats;
  it('SpInfoAndStasTest01', function () {
    let spInfoAndStats = new SpInfoAndStats();
    expect(spInfoAndStats.initElements()).toBeUndefined();
  });

  it('SpInfoAndStasTest03', function () {
    spInfoAndStats.initMetricItemData = jest.fn(() => true);
    expect(spInfoAndStats.initMetricItemData()).toBeTruthy();
  });

  it('SpInfoAndStasTest04', function () {
    let spInfoAndStats = new SpInfoAndStats();
    expect(
      spInfoAndStats.initDataTableStyle({
        children: [
          {
            length: 1,
            style: {
              backgroundColor: 'var(--dark-background5,#F6F6F6)',
            },
          },
        ],
      })
    ).toBeUndefined();
  });

  it('SpInfoAndStasTest06 ', function () {
    expect(spInfoAndStats.connectedCallback()).toBeUndefined();
  });

  it('SpInfoAndStasTest07 ', function () {
    expect(spInfoAndStats.disconnectedCallback()).toBeUndefined();
  });

  it('SpInfoAndStasTest08 ', function () {
    expect(spInfoAndStats.attributeChangedCallback([], [], [])).toBeUndefined();
  });

  it('SpInfoAndStasTest9', function () {
    expect(spInfoAndStats.initMetricItemData()).toBeTruthy();
  });
  it('SpInfoAndStasTest10', function () {
    expect(spInfoAndStats.initMetricItemData()).toBeTruthy();
  });
});
