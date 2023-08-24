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
import { SpAllocations } from '../../../../dist/trace/component/setting/SpAllocations.js';

describe('SpAllocations Test', () => {
  beforeAll(() => {
    document.body.innerHTML = `
            <sp-allocations id = "sp"><sp-allocations>
        `;
  });

  it(' SpAllocations get Default attrValue', function () {
    let spEle = document.querySelector('#sp') as SpAllocations;
    spEle.unwindEL = jest.fn(() => true);
    spEle.unwindEL.value = jest.fn(() => true);
    spEle.shareMemory = jest.fn(() => true);
    spEle.shareMemory.value = jest.fn(() => true);
    spEle.shareMemoryUnit = jest.fn(() => true);
    spEle.shareMemoryUnit.value = jest.fn(() => true);
    spEle.filterMemory = jest.fn(() => true);
    spEle.filterMemory.value = jest.fn(() => true);
    spEle.filterMemoryUnit = jest.fn(() => true);
    spEle.filterMemoryUnit.value = jest.fn(() => true);
    expect(spEle.pid).toEqual(undefined);
    expect(spEle.unwind).toBeNaN();
    expect(spEle.shared).toBe(16384);
    expect(spEle.filter).toBeNaN();
  });

  it(' SpAllocations set  attrValue', function () {
    let spEle = document.querySelector('#sp') as SpAllocations;
    spEle.processId.value = '2';
    spEle.unwindEL.value = '111';
    spEle.shareMemory.value = '222';
    spEle.shareMemoryUnit.value = 'MB';
    spEle.filterMemory.value = '111';
    spEle.filterMemoryUnit.value = 'MB';
    expect(spEle.pid).toEqual(undefined);
    expect(spEle.unwind).toEqual(111);
    expect(spEle.shared).toEqual(222);
    expect(spEle.filter).toEqual(111);
  });

  it(' SpAllocations set  attrValue2', function () {
    let spEle = document.querySelector('#sp') as SpAllocations;
    spEle.processId.value = '3';
    spEle.unwindEL.value = '1121';
    spEle.shareMemory!.value = '222';
    spEle.shareMemoryUnit.value = 'KB';
    spEle.filterMemory.value = '111';
    spEle.filterMemoryUnit.value = 'KB';
    expect(spEle.pid).toEqual(undefined);
    expect(spEle.unwind).toEqual(1121);
    expect(spEle.shared).toEqual(222);
    expect(spEle.filter).toEqual(111);
  });

  it(' SpAllocations set  attrValue03', function () {
    let spEle = new SpAllocations();
    spEle.processId.value = '3';
    spEle.unwindEL.value = '1121';
    spEle.shareMemory.value = '222';
    spEle.filterMemory.value = '111';
    expect(spEle.pid).toEqual(undefined);
    expect(spEle.unwind).toEqual(1121);
    expect(spEle.shared).toEqual(222);
    expect(spEle.filter).toEqual(111);
  });

  it('SpAllocations test04', function () {
    let spEle = document.querySelector('#sp') as SpAllocations;
    expect(spEle.initHtml()).toMatchInlineSnapshot(`
"
        <style>
        .root {
            padding-top: 45px;
            margin-left: 40px;
            display: grid;
            grid-template-columns: repeat(2, 1fr);
            grid-template-rows: min-content 1fr min-content;
            width: 90%;
            border-radius: 0px 16px 16px 0px;
        }
        :host{
            display: block;
            width: 100%;
            border-radius: 0px 16px 16px 0px;
            height: 100%;
        }
        .title {
            grid-column: span 2 / auto;
        }

        .font-style{
            font-family: Helvetica-Bold;
            font-size: 1em;
            color: var(--dark-color1,#000000);
            line-height: 28px;
            font-weight: 700;
        }
        .inner-font-style {
            font-family: Helvetica,serif;
            font-size: 1em;
            color: var(--dark-color1,#000000);
            text-align: left;
            line-height: 20px;
            font-weight: 400;
            display:flex;
            width:75%; 
            margin-top: 3px;
           
        }
        input {
           width: 72%;
           height: 25px;
           border:0;
           outline:none;
           border-radius: 16px;
           text-indent:2%
        }
        input::-webkit-input-placeholder{
            color:var(--bark-prompt,#999999);
        }
        .select {
            height: 30px;
            border:0;
            border-radius: 3px;
            outline:none;
            border: 1px solid var(--dark-border,#B3B3B3);
            width: 60px;
            background-color:var(--dark-background5, #FFFFFF)
            font-family: Helvetica;
            font-size: 14px;
            color:var(--dark-color,#212121)
            text-align: center;
            line-height: 16px;
            font-weight: 400;
            border-radius: 16px;
        }
        .application{
           display: flex;
           flex-direction: column;
           grid-gap: 15px;
           margin-top: 40px;
        }
        .switchstyle{
           margin-top: 40px;
           display: flex;
        }
        .inputstyle{
            background: var(--dark-background5,#FFFFFF);
            border: 1px solid var(--dark-background5,#999999);
            font-family: Helvetica;
            font-size: 14px;
            color: var(--dark-color1,#212121);
            text-align: left;
            line-height: 16px;
            font-weight: 400;
        }
        .inputstyle::-webkit-input-placeholder {
           background: var(--dark-background5,#FFFFFF);
        }
        #one_mb{
            background-color:var(--dark-background5, #FFFFFF)
        }
        #one_kb{
            background-color:var(--dark-background5, #FFFFFF)
        }
        #two_mb{
            background-color:var(--dark-background5, #FFFFFF)
        }
        #two_kb{
            background-color:var(--dark-background5, #FFFFFF)
        }
        .processSelect {
          border-radius: 15px;
          width: 84%;
        }
        .value-range {
          opacity: 0.6;
          font-family: Helvetica;
          font-size: 1em;
          color: var(--dark-color,#000000);
          text-align: left;
          line-height: 20px;
          font-weight: 400;
        }
        .record-title{
            margin-bottom: 16px;
            grid-column: span 3;
        }
        #interval-slider {
            margin: 0 8px;
            grid-column: span 2;
        }
        .resultSize{
            display: grid;
            grid-template-rows: 1fr;
            grid-template-columns:  min-content min-content;
            background-color: var(--dark-background5,#F2F2F2);
            -webkit-appearance:none;
            color:var(--dark-color,#6a6f77);
            width: 150px;
            margin: 0 30px 0 0;
            height: 40px;
            border-radius:20px;
            outline:0;
            border:1px solid var(--dark-border,#c8cccf);
        }
        .record-mode{
            font-family: Helvetica-Bold;
            font-size: 1em;
            color: var(--dark-color1,#000000);
            line-height: 28px;
            font-weight: 400;
            margin-bottom: 16px;
            grid-column: span 1;
        }
        .record-prompt{
              opacity: 0.6;
              font-family: Helvetica;
              font-size: 14px;
              text-align: center;
              line-height: 35px;
              font-weight: 400;
        }
        .interval-result{
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
        
        </style>
        <div class="root">
          <div class = "title">
            <span class="font-style">Native Memory</span>
          </div>
          <div class="application">
             <span class="inner-font-style">ProcessId or ProcessName</span>
             <span class="value-range">Record process</span>
             <lit-allocation-select show-search class="processSelect" rounded default-value="" id="pid" placement="bottom" title="process" placeholder="please select process">
             </lit-allocation-select>
          </div>
          <div class="application">
            <span class="inner-font-style" >Max unwind level</span>
            <span class="value-range">Max Unwind Level Rang is 0 - 512, default 10</span>
            <input id= "unwind"  class="inputstyle" type="text" placeholder="Enter the Max Unwind Level" oninput="if(this.value > 512) this.value = '512'" onkeyup="this.value=this.value.replace(/\\D/g,'')" value="10">
          </div>
          <div class="application">
            <span class="inner-font-style">Shared Memory Size (One page equals 4 KB)</span>
            <span class="value-range">Shared Memory Size Range is 0 - 131072 page, default 16384 page</span>
            <div>
              <input id = "shareMemory" class="inputstyle" type="text" placeholder="Enter the Shared Memory Size" oninput="if(this.value > 131072) this.value = '131072'" onkeyup="this.value=this.value.replace(/\\D/g,'')" value="16384">
              <span>Page</span>
            </div>
          </div>
          <div class="application">
            <span class="inner-font-style" >Filter Memory Size </span>
            <span class="value-range">Filter size Range is 0 - 65535 byte, default 4096 byte</span> 
            <div>
                <input id = "filterSized" class="inputstyle" type="text" placeholder="Enter the Filter Memory Size" oninput="if(this.value > 65535) this.value = '65535'" onkeyup="this.value=this.value.replace(/\\D/g,'')" value="4096">
                 <span>Byte</span>
            </div>
          </div>
          <div class="switchstyle">
              <span class="inner-font-style" id="fp-unwind">Use Fp Unwind</span>               
              <lit-switch class="lts" id="use_fp_unwind" title="fp unwind" checked="true"></lit-switch>
          </div>
          <div class="switchstyle">
              <span class="inner-font-style" id="record_accurately ">Use Record Accurately (Available on recent OpenHarmony 4.0)</span> 
              <lit-switch   class="lts" id="use_record_accurately" title="record_accurately" checked="true"></lit-switch>
          </div>
          <div class="switchstyle">
              <span class="inner-font-style" id="offline_symbolization">Use Offline Symbolization (Available on recent OpenHarmony 4.0)</span> 
              <lit-switch   class="lts" id="use_offline_symbolization" title="offline_symbolization" checked="true"></lit-switch>
          </div>
            
          <div class="switchstyle record-statistics-result" style="grid-row: 6; grid-column: 1 / 3;height: min-content;display: grid;grid-template-rows: 1fr;grid-template-columns: 1fr min-content;">
            <div class="record-title">
                <span class="record-mode">Use Record Statistics (Available on recent OpenHarmony 4.0)</span> 
                <span class="record-prompt"> Time between following interval (0 = disabled) </span>
            </div>
            <lit-slider id="interval-slider" defaultColor="var(--dark-color3,#46B1E3)" open dir="right">
            </lit-slider>
            <div class='resultSize'>
                <input class="interval-result" type="text" value='0' onkeyup="this.value=this.value.replace(/\\D/g,'')">
                <span style="text-align: center; margin: 8px"> S </span>
            </div>
          </div>
        </div>
        "
`);
  });

  it('SpAllocations test05', function () {
    let spAllocations = document.querySelector('#sp') as SpAllocations;
    expect(spAllocations.appProcess).toBe('3');
  });

  it('SpAllocations test06', function () {
    let spAllocations = document.querySelector('#sp') as SpAllocations;
    expect(spAllocations.convertToValue('0', 'MB')).toBe(0);
  });

  it('SpAllocations test07', function () {
    let spAllocations = document.querySelector('#sp') as SpAllocations;
    expect(spAllocations.convertToValue('1', 'KB')).toBe(16384);
  });

  it('SpAllocations test08', function () {
    let spAllocations = document.querySelector('#sp') as SpAllocations;
    expect(spAllocations.convertToValue('1', '')).toBe(0);
  });
  it('SpAllocations test09', function () {
    let spAllocations = document.querySelector('#sp') as SpAllocations;
    expect(spAllocations.fp_unwind).toBeTruthy();
  });
});
