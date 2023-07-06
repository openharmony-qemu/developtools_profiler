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
import { SpSdkConfig } from '../../../../dist/trace/component/setting/SpSdkConfig.js';

describe('spSdkConfig Test', () => {
  let spSdkConfig = new SpSdkConfig();
  it('spSdkConfigTest01', function () {
    spSdkConfig.show = true;
    expect(spSdkConfig.show).toBeTruthy();
  });

  it('spSdkConfigTest02', function () {
    spSdkConfig.show = false;
    expect(spSdkConfig.show).toBeFalsy();
  });

  it('spSdkConfigTest03', function () {
    spSdkConfig.startSamp = true;
    expect(spSdkConfig.startSamp).toBeTruthy();
  });

  it('spSdkConfigTest04', function () {
    spSdkConfig.startSamp = false;
    expect(spSdkConfig.startSamp).toBeFalsy();
  });

  it('spSdkConfigTest05', function () {
    spSdkConfig.configName = '';
    expect(spSdkConfig.configName).toBeDefined();
  });

  it('spSdkConfigTest06', function () {
    spSdkConfig.configName = 'configName';
    expect(spSdkConfig.configName).toBeDefined();
  });

  it('spSdkConfigTest07', function () {
    spSdkConfig.type = '';
    expect(spSdkConfig.type).toBeDefined();
  });

  it('spSdkConfigTest08', function () {
    spSdkConfig.type = 'configName';
    expect(spSdkConfig.type).toBeDefined();
  });

  it('spSdkConfigTest09', function () {
    expect(spSdkConfig.getPlugName()).not.toBeUndefined();
  });

  it('spSdkConfigTest10', function () {
    expect(spSdkConfig.getSampleInterval()).not.toBeUndefined();
  });

  it('spSdkConfigTest11', function () {
    expect(spSdkConfig.getGpuConfig()).not.toBeUndefined();
  });

  it('spSdkConfigTest12', function () {
    expect(spSdkConfig.checkIntegerInput('')).not.toBeUndefined();
  });

  it('spSdkConfigTest13', function () {
    expect(spSdkConfig.checkIntegerInput('checkIntegerInput')).not.toBeUndefined();
  });

  it('spSdkConfigTest14', function () {
    expect(spSdkConfig.checkFloatInput('checkFloatInput')).not.toBeUndefined();
  });

  it('spSdkConfigTest15', function () {
    expect(spSdkConfig.isAbleShowConfig(false)).toBeUndefined();
  });

  it('spSdkConfigTest16', function () {
    expect(spSdkConfig.isAbleShowConfig(true)).toBeUndefined();
  });

  it('spSdkConfigTest17', function () {
    spSdkConfig.sdkConfigList = {
      name: '',
      configuration: {
        ss: {
          type: 'string',
          default: 'strsadsa',
          description: 'xxxx',
        },
        aa: {
          type: 'string',
          default: '11',
          enum: ['consistent', '11', 'delegated'],
        },
        cc: {
          type: 'number',
          description: 'number1111',
        },
        ee: {
          type: 'integer',
          default: '12',
          description: 'integer1222',
        },
        ff: {
          type: 'boolean',
          description: 'switchhh',
        },
      },
    };
    expect(spSdkConfig.initConfig()).toBeUndefined();
  });
  it('spSdkConfigTest18', function () {
    expect(spSdkConfig.getGpuConfig()).toStrictEqual({
      aa: '11',
      cc: 0,
      ee: 12,
      ff: false,
      ss: 'strsadsa',
    });
  });
});
