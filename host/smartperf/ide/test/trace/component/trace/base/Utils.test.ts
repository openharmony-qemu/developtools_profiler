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
import { Utils } from '../../../../../dist/trace/component/trace/base/Utils.js';

describe('Utils Test', () => {
  beforeAll(() => {});

  it('Utils Test01', () => {
    let instance = Utils.getInstance();
    let instance2 = Utils.getInstance();
    expect(instance).toBe(instance2);
  });

  it('Utils Test02', () => {
    let instance = Utils.getInstance();
    expect(instance.getStatusMap().get('D')).toBe('Uninterruptible Sleep');
  });

  it('Utils Test03', () => {
    expect(Utils.getEndState('D')).toBe('Uninterruptible Sleep');
  });

  it('Utils Test04', () => {
    expect(Utils.getEndState('')).toBe('');
  });

  it('Utils Test05', () => {
    expect(Utils.getEndState('ggg')).toBe('Unknown State');
  });

  it('Utils Test06', () => {
    expect(Utils.getStateColor('D')).toBe('#f19b38');
  });

  it('Utils Test07', () => {
    expect(Utils.getStateColor('R')).toBe('#a0b84d');
  });
  it('Utils Test08', () => {
    expect(Utils.getStateColor('I')).toBe('#673ab7');
  });

  it('Utils Test09', () => {
    expect(Utils.getStateColor('Running')).toBe('#467b3b');
  });

  it('Utils Test09', () => {
    expect(Utils.getStateColor('S')).toBe('#e0e0e0');
  });

  it('Utils Test10', () => {
    expect(Utils.getTimeString(5900_000_000_000)).toBe('1h 38m ');
  });

  it('Utils Test11', () => {
    expect(Utils.getByteWithUnit(2_000_000_000)).toBe('1.86 Gb');
  });

  it('Utils Test12', () => {
    expect(Utils.getByteWithUnit(1_000_000_000)).toBe('953.67 Mb');
  });

  it('Utils Test13', () => {
    expect(Utils.getByteWithUnit(1000_000)).toBe('976.56 Kb');
  });

  it('Utils Test23', () => {
    expect(Utils.getByteWithUnit(-2_000)).toBe('-1.95 Kb');
  });

  it('Utils Test14', () => {
    expect(Utils.getTimeString(1_000_000_000_000)).toBe('16m 40s ');
  });

  it('Utils Test15', () => {
    expect(Utils.getTimeString(2_000_000)).toBe('2ms ');
  });

  it('Utils Test16', () => {
    expect(Utils.getTimeString(3_000)).toBe('3μs ');
  });

  it('Utils Test17', () => {
    expect(Utils.getTimeString(300)).toBe('300ns ');
  });

  it('Utils Test18', () => {
    expect(Utils.getTimeStringHMS(5900_000_000_000)).toBe('1:38:');
  });

  it('Utils Test19', () => {
    expect(Utils.getTimeStringHMS(3_000_000_000)).toBe('3:');
  });

  it('Utils Test20', () => {
    expect(Utils.getTimeStringHMS(2_000_000)).toBe('2.');
  });

  it('Utils Test21', () => {
    expect(Utils.getTimeStringHMS(5_000)).toBe('5.');
  });

  it('Utils Test22', () => {
    expect(Utils.getTimeStringHMS(90)).toBe('90');
  });

  it('Utils Test24', () => {
    expect(Utils.getBinaryByteWithUnit(0)).toBe('0Bytes');
  });

  it('Utils Test25', () => {
    expect(Utils.getBinaryByteWithUnit(3_000_000_000)).toBe('2.79GB');
  });

  it('Utils Test26', () => {
    expect(Utils.getBinaryByteWithUnit(2_000_000)).toBe('1.91MB');
  });

  it('Utils Test27', () => {
    expect(Utils.getBinaryByteWithUnit(2_000)).toBe('1.95KB');
  });

  it('Utils Test28', () => {
    expect(Utils.getTimeStampHMS(3900_000_000_000)).toBe('01:05:00:000.000');
  });

  it('Utils Test29', () => {
    expect(Utils.getTimeStampHMS(70_000_000_000)).toBe('01:10:000.000');
  });

  it('Utils Test30', () => {
    expect(Utils.getTimeStampHMS(2_000_000_000)).toBe('02:000.000');
  });

  it('Utils Test31', () => {
    expect(Utils.getTimeStampHMS(2_000_000)).toBe('00:002.000');
  });

  it('Utils Test32', () => {
    expect(Utils.getTimeStampHMS(2_000)).toBe('00:000.002.');
  });

  it('Utils Test33', () => {
    expect(Utils.getTimeStampHMS(1)).toBe('00:000.000001');
  });

  it('Utils Test40', () => {
    expect(Utils.getDurString(61_000_000_000)).toBe('61.000 s ');
  });

  it('Utils Test34', () => {
    expect(Utils.getDurString(2_000_000_000)).toBe('2.000 s ');
  });

  it('Utils Test35', () => {
    expect(Utils.getDurString(1_800_000)).toBe('1 ms ');
  });

  it('Utils Test36', () => {
    expect(Utils.timeMsFormat2p(3800_000)).toBe('1.00h');
  });

  it('Utils Test37', () => {
    expect(Utils.timeMsFormat2p(90_000)).toBe('2.00min');
  });

  it('Utils Test38', () => {
    expect(Utils.timeMsFormat2p(2_000)).toBe('2.00s');
  });

  it('Utils Test39', () => {
    expect(Utils.timeMsFormat2p(1)).toBe('1.00ms');
  });

  it('Utils Test41', () => {
    expect(Utils.getProbablyTime(3600_000_000_000)).toBe('1.00h ');
  });

  it('Utils Test42', () => {
    expect(Utils.getProbablyTime(60_000_000_000)).toBe('1.00m ');
  });

  it('Utils Test43', () => {
    expect(Utils.getProbablyTime(1_000_000_000)).toBe('1.00s ');
  });

  it('Utils Test44', () => {
    expect(Utils.getProbablyTime(1_000_000)).toBe('1.00ms ');
  });

  it('Utils Test45', () => {
    expect(Utils.getProbablyTime(1_000)).toBe('1.00μs ');
  });

  it('Utils Test46', () => {
    expect(Utils.getProbablyTime(0)).toBe('0');
  });

  it('Utils Test47', () => {
    expect(Utils.groupByMap([], '')).not.toBeUndefined();
  });

  it('Utils Test48', () => {
    expect(Utils.uuid).not.toBeUndefined();
  });

  it('Utils Test49', () => {
    expect(Utils.removeDuplicates([10], [11], 'pid')).not.toBeUndefined();
  });

  it('Utils Test50', () => {
    expect(Utils.groupBy([], '')).not.toBeUndefined();
  });

  it('Utils Test51', () => {
    expect(Utils.getTimeStringHMS(0)).toBe('0');
  });

  it('Utils Test52', () => {
    expect(Utils.timeMsFormat2p(0)).toBe('0s');
  });

  it('Utils Test53', () => {
    expect(Utils.getBinaryByteWithUnit(1)).toBe('1.00Bytes');
  });

  it('Utils Test54', () => {
    expect(Utils.getTimeStampHMS(0)).toBe('00:000.000');
  });

  it('Utils Test55', () => {
    expect(Utils.getDurString(1)).toBe('1');
  });

  it('Utils Test56', () => {
    expect(Utils.getCompletionTime(300, 3)).toBe('300');
  });

  it('Utils Test57', () => {
    expect(Utils.getCompletionTime(30, 3)).toBe('030');
  });

  it('Utils Test58', () => {
    expect(Utils.getCompletionTime(0, 0)).toBe('0');
  });

  it('Utils Test59', () => {
    expect(Utils.groupByMap([10], 'pid')).not.toBeUndefined();
  });

  it('Utils Test60', () => {
    expect(Utils.getProbablyTime(1)).toBe('1ns ');
  });

  it('Utils Test61', () => {
    expect(Utils.groupBy([10], 'pid')).not.toBeUndefined();
  });

  afterAll(() => {
    // 后处理操作
  });
});
