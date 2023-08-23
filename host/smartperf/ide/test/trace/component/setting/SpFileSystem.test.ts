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
import { SpFileSystem } from '../../../../dist/trace/component/setting/SpFileSystem';

describe('spFileSystem Test', () => {
  let spFileSystem = new SpFileSystem();
  it('SpFileSystemTest01', function () {
    spFileSystem.startSamp = true;
    expect(spFileSystem.startSamp).toBeTruthy();
  });

  it('SpFileSystemTest02', function () {
    spFileSystem.startSamp = false;
    expect(spFileSystem.startSamp).toBeFalsy();
  });

  it('SpFileSystemTest03', function () {
    expect(spFileSystem.getSystemConfig()).toStrictEqual({
      process: '',
      unWindLevel: 10,
    });
  });

  it('SpFileSystemTest04', function () {
    expect(spFileSystem.unDisable()).toBeUndefined();
  });

  it('SpFileSystemTest05', function () {
    expect(spFileSystem.disable()).toBeUndefined();
  });

  it('SpFileSystemTest06', function () {
    expect(spFileSystem.initConfigList()).toBeUndefined();
  });

  it('SpFileSystemTest07', function () {
    spFileSystem.startRecord = true;
    expect(spFileSystem.startRecord).toBeTruthy();
  });

  it('SpFileSystemTest08', function () {
    spFileSystem.startRecord = false;
    expect(spFileSystem.startRecord).toBeFalsy();
  });

  it('SpFileSystemTest09', function () {
    spFileSystem.startFileSystem = true;
    expect(spFileSystem.startFileSystem).toBeTruthy();
  });

  it('SpFileSystemTest10', function () {
    spFileSystem.startFileSystem = false;
    expect(spFileSystem.startFileSystem).toBeFalsy();
  });

  it('SpFileSystemTest11', function () {
    spFileSystem.startVirtualMemory = true;
    expect(spFileSystem.startVirtualMemory).toBeTruthy();
  });

  it('SpFileSystemTest12', function () {
    spFileSystem.startVirtualMemory = false;
    expect(spFileSystem.startVirtualMemory).toBeFalsy();
  });

  it('SpFileSystemTest13', function () {
    spFileSystem.startIo = true;
    expect(spFileSystem.startIo).toBeTruthy();
  });

  it('SpFileSystemTest14', function () {
    spFileSystem.startIo = false;
    expect(spFileSystem.startIo).toBeFalsy();
  });
});
