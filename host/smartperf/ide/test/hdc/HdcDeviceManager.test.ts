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
import { HdcDeviceManager } from '../../dist/hdc/HdcDeviceManager.js';

describe('HdcDeviceManager', () => {
  it('HdcDeviceManagerTest_01', () => {
    expect(HdcDeviceManager.disConnect(1)).toBeTruthy();
  });

  it('HdcDeviceManagerTest_02', () => {
    expect(HdcDeviceManager.fileRecv('1')).toBeTruthy();
  });
});
