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

import { BaseElement, element } from '../../base-ui/BaseElement.js';

@element('sp-welcome')
export class SpWelcomePage extends BaseElement {
  initElements(): void {}

  initHtml(): string {
    return `
        <style>
        :host {
            width: 100%;
            height: 100%;
            display: grid;
            justify-content: center;
            align-content: center;
            background: var(--dark-background5,#F6F6F6);
        }
        .home-page{
            margin-top: -100px;
        }
        .lit-icon{
            margin-top: 50px;
            margin-left: 50px;
            content: var(--dark-img,url('img/pic.png'));
        }
        </style>
        <div>
            <div class="home-page">
                <p/>黄区域名: <a href="https://smartperf.rnd.huawei.com/smartperf/">https://smartperf.rnd.huawei.com/smartperf/</a></p>
                <p/>绿区域名: <a href="https://devecotesting.rnd.huawei.com/smartperf/">https://devecotesting.rnd.huawei.com/smartperf/</a></p>
                <p/>3ms社区: <a href="http://3ms.huawei.com/km/groups/3956611/home?|=zh-cn">http://3ms.huawei.com/km/groups/3956611/home?|=zh-cn</a></p>
                <p/>welink讨论群: 群1: 593524364461277889 群2: 473395703(已满) 群3: 485625665(已满)</p>
            </div>
            <img class="lit-icon" >
        </div>
        `;
  }
}
