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
import { LitCheckBox } from '../../../../src/base-ui/checkbox/LitCheckBox';

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
    expect(spEle.traceEvents).toEqual([
      'ability',
      'ace',
      'app',
      'ark',
      'binder',
      'disk',
      'freq',
      'graphic',
      'idle',
      'irq',
      'memreclaim',
      'mmc',
      'multimodalinput',
      'ohos',
      'pagecache',
      'rpc',
      'sched',
      'sync',
      'window',
      'workq',
      'zaudio',
      'zcamera',
      'zimage',
      'zmedia',
    ]);
    expect(spEle.memoryConfig).toEqual([]);
  });

  it(' SpProbesConfig test', function () {
    let spEle = document.querySelector('#spconfig') as SpProbesConfig;
    expect(spEle.initHtml()).toMatchInlineSnapshot(`
"
        <style>
        .recordText {
           font-family: Helvetica-Bold;
           font-size: 1em;
           color: var(--dark-color1,#000000);
           line-height: 28px;
           font-weight: 700;
           margin-bottom: 20px;
        }

        :host{
            display: inline-block;
            background: var(--dark-background3,#FFFFFF);
            width: 100%;
            height: 100%;
            border-radius: 0px 16px 16px 0px;
        }

        .root {
            margin-right: 30px;
            padding-top: 30px;
            padding-left: 54px;
            margin-bottom: 30px;
            font-size:16px;
        }
        
        .config-page {
            height: 95%;
            font-size: 0.875em;
        }

        .trace-config{
           display: flex;
           flex-direction: column;
           width: 50%;
           gap: 10px;
           margin-bottom: 20px;
        }

        .memory-config{
           display: grid;
           grid-template-columns: repeat(2, 1fr);
           border-style: solid none none none;
           border-color: #D5D5D5;
           padding-top: 15px;
           margin-top: 15px;
           gap: 10px;
        }
        
        .ability-config{
           display: grid;
           grid-template-columns: repeat(2, 1fr);
           border-style: solid none none none;
           border-color: #D5D5D5;
           padding-top: 15px;
           margin-top: 15px;
           gap: 10px;
        }

        .span-col-2{
           grid-column: span 2 / auto;
        }

        .log-config{
           display: grid;
           grid-template-columns: repeat(2, 1fr);
           border-style: solid none none none;
           border-color: #D5D5D5;
           padding-top: 15px;
           gap: 10px;
        }

        #hitrace-cat{
           display: grid;
           grid-template-columns: 1fr 1fr;
        }
        .user-events{
           display: grid;
           grid-template-columns: repeat(4, 1fr);
           grid-template-rows: repeat(2, 1fr);
           gap: 10px;
           margin-left: 15px;;
        }
        #ftrace-buff-size-div {
            width: 100%;
            height: min-content;
            display: grid;
            grid-template-columns: 1fr min-content;
        }
        .buffer-size-des {
            opacity: 0.6;
            font-family: Helvetica;
            font-size: 1em;
            color: var(--dark-color,#000000);
            text-align: left;
            line-height: 20px;
            font-weight: 400;
        }
        .ftrace-buff-size-result-div{
            display: grid;
            grid-template-rows: 1fr;
            grid-template-columns:  min-content min-content;
            background-color: var(--dark-background5,#F2F2F2);
            -webkit-appearance:none;
            color:var(--dark-color,#6a6f77);
            width: 150px;
            margin: 0 20px 0 0;
            height: 40px;
            border-radius:20px;
            outline:0;
            border:1px solid var(--dark-border,#c8cccf);
        }
        .ftrace-buff-size-result{
            background-color: var(--dark-background5,#F2F2F2);
            -webkit-appearance:none;
            color:var(--dark-color,#6a6f77);
            border: none;
            text-align: center;
            width: 90px;
            font-size:14px;
            outline:0;
            margin: 5px 0 5px 5px;
        }
        .border-red {
           border:1px solid red;
        }
        </style>
        <div class="root">
            <div class="recordText" >Record mode</div>
            <div class="config-page">
                <div>
                    <div class="trace-config"></div>
                    <div class="span-col-2" id="hitrace-cat">
                      <check-des-box id="hitrace" checked="true" value ="Hitrace categories" des="Enables C++ codebase annotations (HTRACE_BEGIN() / os.Trace())">
                      </check-des-box>
                      <div class="user-events">
                          <slot></slot>
                      </div>
                    </div>
                    <div>
                       <div>
                          <p>Buffer Size</p>
                          <p class="buffer-size-des">The ftrace buffer size range is 2048 KB to 307200 KB</p>
                       </div>
                       <div id="ftrace-buff-size-div">
                          <lit-slider id="ftrace-buff-size-slider" defaultColor="var(--dark-color3,#46B1E3)" open dir="right">
                          </lit-slider>
                          <div class='ftrace-buff-size-result-div'>
                              <input class="ftrace-buff-size-result" type="text" value='20480' onkeyup="this.value=this.value.replace(/\\D/g,'')">
                              <span style="text-align: center; margin: 8px"> KB </span>
                           </div>
                       </div>
                    </div>
                </div>
                <div class="memory-config">
                    <div class="span-col-2">6有
                      <span>Memory Config</span>
                    </div>
                </div>
                <div class="ability-config">
                    <div class="span-col-2">
                      <span>Ability Config</span>
                    </div>
                </div>
            </div>
        </div>
        "
`);
  });
});
