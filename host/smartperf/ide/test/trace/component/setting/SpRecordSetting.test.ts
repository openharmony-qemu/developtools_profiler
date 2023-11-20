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
  it('SpRecordSettingTest05', function (){
    let spEle = document.querySelector('#setting') as SpRecordSetting;
    let maxFileSizeEl = null;
    expect(spEle.longTraceSingleFileMaxSize).toEqual(200);
  })
  it('SpRecordSettingTest06', function (){
    let spEle = document.querySelector('#setting') as SpRecordSetting;
    spEle.radioBox = null;
    expect(spEle.recordMod).toBeFalsy();
  })
  it('SpRecordSettingTest07', function (){
    let spEle = document.querySelector('#setting') as SpRecordSetting;
    spEle.outputPath = spEle.shadowRoot?.querySelector('#trace_path') as HTMLInputElement;
    spEle.outputPath.value = ''
    expect(spEle.longOutPath).toEqual('/data/local/tmp/long_trace/');
  })
  it('SpRecordSettingTest08', function (){
    let spEle = document.querySelector('#setting') as SpRecordSetting;
    spEle.outputPath = undefined
    expect(spEle.longOutPath).toEqual('/data/local/tmp/long_trace/');
  })
  it('SpRecordSettingTest09', function (){
    let spEle = document.querySelector('#setting') as SpRecordSetting;
    spEle.outputPath = spEle.shadowRoot?.querySelector('#trace_path') as HTMLInputElement;
    spEle.outputPath.value = 'long_trace/'
    expect(spEle.longOutPath).toEqual('/data/local/tmp/long_trace/');
  })
  it('SpRecordSettingTest10', function (){
    let spEle = document.querySelector('#setting') as SpRecordSetting;
    spEle.outputPath = spEle.shadowRoot?.querySelector('#trace_path') as HTMLInputElement;
    spEle.outputPath.value = 'long'
    expect(spEle.longOutPath).toEqual(`/data/local/tmp/long/`);
  })
  it('SpRecordSettingTest11', function (){
    let spEle = document.querySelector('#setting') as SpRecordSetting;
    spEle.isRecordTemplate = undefined;
    spEle.outputPath = undefined;
    expect(spEle.output).toEqual('/data/local/tmp/hiprofiler_data.htrace');
  })
  it('SpRecordSettingTest12', function (){
    let spEle = document.querySelector('#setting') as SpRecordSetting;
    spEle.isRecordTemplate = true;
    spEle.outputPath = undefined;
    expect(spEle.output).toEqual('/data/local/tmp/hiprofiler_data.htrace');
  })
  it('SpRecordSettingTest13', function (){
    let spEle = document.querySelector('#setting') as SpRecordSetting;
    spEle.isRecordTemplate = true;
    spEle.outputPath = spEle.shadowRoot?.querySelector('#trace_path') as HTMLInputElement;
    spEle.outputPath.value = '1111'
    expect(spEle.output).toEqual(`/data/local/tmp/1111`);
  })
  it('SpRecordSettingTest14', function (){
    let spEle = document.querySelector('#setting') as SpRecordSetting;
    spEle.bufferNumber = undefined;
    expect(spEle.bufferSize).toEqual(64);
  })
  it('SpRecordSettingTest15', function () {
    let spEle = document.querySelector('#setting') as SpRecordSetting;
    spEle.bufferNumber = spEle.shadowRoot.querySelector('.buffer-size') as HTMLElement;
    let parentElement = spEle.memoryBufferSlider!.parentNode as Element;
    let bufferInput = spEle.shadowRoot.querySelector('.memory_buffer_result') as HTMLInputElement;
    let htmlInputElement = spEle.memoryBufferSlider!.shadowRoot.querySelector('#slider') as HTMLInputElement;
    bufferInput.value = '60';
    bufferInput.dispatchEvent(new Event('input'));
    expect(spEle.memoryBufferSlider.percent).toEqual(bufferInput.value);
    expect(htmlInputElement.value).toEqual(bufferInput.value);
    expect(spEle.memoryBufferSlider.sliderStyle).toEqual({
      minRange: 4,
      maxRange: 512,
      defaultValue: bufferInput.value,
      resultUnit: 'MB',
      stepSize: 2,
      lineColor: 'var(--dark-color3,#46B1E3)',
      buttonColor: '#999999',
    });
    expect(parentElement.getAttribute('percent')).toEqual(bufferInput.value);
  });
});
