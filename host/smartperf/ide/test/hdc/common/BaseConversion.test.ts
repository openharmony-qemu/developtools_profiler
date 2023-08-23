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
import { toHex8, toHex16, toHex32, toHex64, uint8ArrayToString } from '../../../dist/hdc/common/BaseConversion.js';

describe('BaseConversionTest', () => {
  it('BaseConversionTest_toHex8_01', () => {
    expect(toHex8('0O8')).toEqual('0O8');
  });

  it('BaseConversionTest_toHex8_02', () => {
    expect(toHex8(32)).toEqual('20');
  });

  it('BaseConversionTest_toHex16_01', () => {
    expect(toHex16(8)).toEqual('08');
  });

  it('BaseConversionTest_toHex16_02', () => {
    expect(toHex16(11)).toEqual('0b');
  });

  it('BaseConversionTest_toHex32_01', () => {
    expect(toHex32(33)).toEqual('0021');
  });

  it('BaseConversionTest_toHex32_02', () => {
    expect(toHex32(36)).toEqual('0024');
  });

  it('BaseConversionTest_toHex64_01', () => {
    expect(toHex64('36')).toEqual('00000036');
  });

  it('BaseConversionTest_toHex64_02', () => {
    expect(toHex64(36)).toEqual('00000024');
  });

  it('BaseConversionTest_uint8ArrayToString_01', () => {
    expect(uint8ArrayToString([21, 31], false)).toEqual('2131');
  });

  it('BaseConversionTest_uint8ArrayToString_02', () => {
    expect(uint8ArrayToString([21, 31], true)).toEqual('151f');
  });
});
