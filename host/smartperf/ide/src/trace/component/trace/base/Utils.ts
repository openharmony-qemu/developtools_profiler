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

export class Utils {
  private static statusMap: Map<string, string> = new Map<string, string>();
  private static instance: Utils | null = null;
  static THREAD_MAP: Map<number, string> = new Map<number, string>();
  static PROCESS_MAP: Map<number, string> = new Map<number, string>();
  static SCHED_SLICE_MAP: Map<
    string,
    {
      endState: string;
      priority: number;
    }
  > = new Map<
    string,
    {
      endState: string;
      priority: number;
    }
  >();

  constructor() {
    Utils.statusMap.set('D', 'Uninterruptible Sleep');
    Utils.statusMap.set('D-NIO', 'Uninterruptible Sleep(non-IO)');
    Utils.statusMap.set('D-IO', 'Uninterruptible Sleep(IO)');
    Utils.statusMap.set('DK', 'Uninterruptible Sleep + Wake Kill');
    Utils.statusMap.set('DK-NIO', 'Uninterruptible Sleep(non-IO) + Wake Kill');
    Utils.statusMap.set('DK-IO', 'Uninterruptible Sleep(IO) + Wake Kill');
    Utils.statusMap.set('S', 'Sleeping');
    Utils.statusMap.set('R', 'Runnable');
    Utils.statusMap.set('Running', 'Running');
    Utils.statusMap.set('R+', 'Runnable (Preempted)');
    Utils.statusMap.set('R-B', 'Runnable (Binder)');
    Utils.statusMap.set('I', 'Task Dead');
    Utils.statusMap.set('T', 'Traced');
    Utils.statusMap.set('t', 'Traced');
    Utils.statusMap.set('X', 'Exit (Dead)');
    Utils.statusMap.set('Z', 'Exit (Zombie)');
    Utils.statusMap.set('K', 'Wake Kill');
    Utils.statusMap.set('W', 'Waking');
    Utils.statusMap.set('P', 'Parked');
    Utils.statusMap.set('N', 'No Load');
  }

  public static getInstance(): Utils {
    if (Utils.instance == null) {
      Utils.instance = new Utils();
    }
    return Utils.instance;
  }

  public static clearData() {
    Utils.THREAD_MAP.clear();
    Utils.PROCESS_MAP.clear();
    Utils.SCHED_SLICE_MAP.clear();
  }

  public static getEndState(state: string): string {
    if (Utils.getInstance().getStatusMap().has(state)) {
      return Utils.getInstance().getStatusMap().get(state) || 'Unknown State';
    } else {
      if ('' == state || state == null) {
        return '';
      }
      return 'Unknown State';
    }
  }

  public static getStateColor(state: string): string {
    if (state == 'D-NIO' || state == 'DK-NIO') {
      return '#795548';
    } else if (state == 'D-IO' || state == 'DK-IO' || state == 'D' || state == 'DK') {
      return '#f19b38';
    } else if (state == 'R' || state == 'R+') {
      return '#a0b84d';
    } else if (state == 'R-B') {
      return '#87CEFA';
    } else if (state == 'I') {
      return '#673ab7';
    } else if (state == 'Running') {
      return '#467b3b';
    } else if (state == 'S') {
      return '#e0e0e0';
    } else {
      return '#ff6e40';
    }
  }

  public static getTimeString(ns: number): string {
    let currentTime = ns;
    let hour1 = 3600_000_000_000;
    let minute1 = 60_000_000_000;
    let second1 = 1_000_000_000;
    let millisecond1 = 1_000_000;
    let microsecond1 = 1_000;
    let res = '';
    if (currentTime >= hour1) {
      res += Math.floor(currentTime / hour1) + 'h ';
      currentTime = currentTime - Math.floor(currentTime / hour1) * hour1;
    }
    if (currentTime >= minute1) {
      res += Math.floor(currentTime / minute1) + 'm ';
      currentTime = currentTime - Math.floor(ns / minute1) * minute1;
    }
    if (currentTime >= second1) {
      res += Math.floor(currentTime / second1) + 's ';
      currentTime = currentTime - Math.floor(currentTime / second1) * second1;
    }
    if (currentTime >= millisecond1) {
      res += Math.floor(currentTime / millisecond1) + 'ms ';
      currentTime = currentTime - Math.floor(currentTime / millisecond1) * millisecond1;
    }
    if (currentTime >= microsecond1) {
      res += Math.floor(currentTime / microsecond1) + 'μs ';
      currentTime = currentTime - Math.floor(currentTime / microsecond1) * microsecond1;
    }
    if (currentTime > 0) {
      res += currentTime + 'ns ';
    }
    if (res == '') {
      res = ns + '';
    }
    return res;
  }

  public static getProbablyTime(timeNs: number): string {
    let currentNs = timeNs;
    let probablyHour = 3600_000_000_000;
    let probablyMinute1 = 60_000_000_000;
    let probablySecond1 = 1_000_000_000;
    let probablyMillisecond1 = 1_000_000;
    let probablyMicrosecond1 = 1_000;
    let res = '';
    if (currentNs >= probablyHour) {
      res += (currentNs / probablyHour).toFixed(2) + 'h ';
    } else if (currentNs >= probablyMinute1) {
      res += (currentNs / probablyMinute1).toFixed(2) + 'm ';
    } else if (currentNs >= probablySecond1) {
      res += (currentNs / probablySecond1).toFixed(2) + 's ';
    } else if (currentNs >= probablyMillisecond1) {
      res += (currentNs / probablyMillisecond1).toFixed(2) + 'ms ';
    } else if (currentNs >= probablyMicrosecond1) {
      res += (currentNs / probablyMicrosecond1).toFixed(2) + 'μs ';
    } else if (currentNs > 0) {
      res += currentNs + 'ns ';
    } else if (res == '') {
      res = timeNs + '';
    }
    return res;
  }

  public static getTimeStringHMS(ns: number): string {
    let currentNs = ns;
    let hour1 = 3600_000_000_000;
    let minute1 = 60_000_000_000;
    let second1 = 1_000_000_000; // 1 second
    let millisecond1 = 1_000_000; // 1 millisecond
    let microsecond1 = 1_000; // 1 microsecond
    let res = '';
    if (currentNs >= hour1) {
      res += Math.floor(currentNs / hour1) + ':';
      currentNs = currentNs - Math.floor(currentNs / hour1) * hour1;
    }
    if (currentNs >= minute1) {
      res += Math.floor(currentNs / minute1) + ':';
      currentNs = currentNs - Math.floor(ns / minute1) * minute1;
    }
    if (currentNs >= second1) {
      res += Math.floor(currentNs / second1) + ':';
      currentNs = currentNs - Math.floor(currentNs / second1) * second1;
    }
    if (currentNs >= millisecond1) {
      res += Math.floor(currentNs / millisecond1) + '.';
      currentNs = currentNs - Math.floor(currentNs / millisecond1) * millisecond1;
    }
    if (currentNs >= microsecond1) {
      res += Math.floor(currentNs / microsecond1) + '.';
      currentNs = currentNs - Math.floor(currentNs / microsecond1) * microsecond1;
    }
    if (currentNs > 0) {
      res += currentNs + '';
    }
    if (res == '') {
      res = ns + '';
    }
    return res;
  }

  public static getByteWithUnit(bytes: number): string {
    if (bytes < 0) {
      return '-' + this.getByteWithUnit(Math.abs(bytes));
    }
    let currentByte = bytes;
    let kb1 = 1 << 10;
    let mb1 = (1 << 10) << 10;
    let gb1 = ((1 << 10) << 10) << 10; // 1 gb
    let res = '';
    if (currentByte > gb1) {
      res += (currentByte / gb1).toFixed(2) + ' Gb';
    } else if (currentByte > mb1) {
      res += (currentByte / mb1).toFixed(2) + ' Mb';
    } else if (currentByte > kb1) {
      res += (currentByte / kb1).toFixed(2) + ' Kb';
    } else {
      res += Math.round(currentByte) + ' byte';
    }
    return res;
  }

  public static groupByMap(array: Array<any>, key: string) {
    let result = new Map();
    array.forEach((item) => {
      let value = item[key];
      if (!result.has(value)) {
        result.set(value, []);
      }
      result.get(value).push(item);
    });
    return result;
  }

  public static groupBy(array: Array<any>, key: string) {
    return array.reduce((pre, current, index, arr) => {
      (pre[current[key]] = pre[current[key]] || []).push(current);
      return pre;
    }, {});
  }

  public static timeMsFormat2p(ms: number) {
    let currentNs = ms;
    let hour1 = 3600_000;
    let minute1 = 60_000;
    let second1 = 1_000; // 1 second
    let result = '';
    if (currentNs >= hour1) {
      result += Math.round(currentNs / hour1).toFixed(2) + 'h';
      return result;
    }
    if (currentNs >= minute1) {
      result += Math.round(currentNs / minute1).toFixed(2) + 'min';
      return result;
    }
    if (currentNs >= second1) {
      result += Math.round(currentNs / second1).toFixed(2) + 's';
      return result;
    }
    if (currentNs > 0) {
      result += currentNs.toFixed(2) + 'ms';
      return result;
    }
    if (result == '') {
      result = '0s';
    }
    return result;
  }

  public static uuid(): string {
    // @ts-ignore
    return ([1e7] + -1e3 + -4e3 + -8e3 + -1e11).replace(/[018]/g, (c) =>
      (c ^ (crypto.getRandomValues(new Uint8Array(1))[0] & (15 >> (c / 4)))).toString(16)
    );
  }

  public static getBinaryKBWithUnit(kbytes: number): string {
    if (kbytes == 0) {
      return '0KB';
    }
    let currentBytes = kbytes;
    let mib1 = 1024;
    let gib1 = 1024 * 1024;
    let res = '';
    if (currentBytes >= gib1) {
      res += (currentBytes / gib1).toFixed(2) + 'GB';
    } else if (currentBytes >= mib1) {
      res += (currentBytes / mib1).toFixed(2) + 'MB';
    } else {
      res += currentBytes.toFixed(2) + 'KB';
    }
    return res;
  }

  public static getBinaryByteWithUnit(bytes: number): string {
    if (bytes == 0) {
      return '0Bytes';
    }
    let currentBytes = bytes;
    let kib1 = 1024;
    let mib1 = 1024 * 1024;
    let gib1 = 1024 * 1024 * 1024;
    let res = '';
    if (bytes < 0) {
      res = '-';
      currentBytes = Math.abs(currentBytes);
    }
    if (currentBytes >= gib1) {
      res += (currentBytes / gib1).toFixed(2) + 'GB';
    } else if (currentBytes >= mib1) {
      res += (currentBytes / mib1).toFixed(2) + 'MB';
    } else if (currentBytes >= kib1) {
      res += (currentBytes / kib1).toFixed(2) + 'KB';
    } else {
      res += currentBytes.toFixed(2) + 'Bytes';
    }
    return res;
  }

  public static getTimeStampHMS(ns: number): string {
    let currentNs = ns;
    let hour1 = 3600_000_000_000;
    let minute1 = 60_000_000_000;
    let second1 = 1_000_000_000; // 1 second
    let millisecond1 = 1_000_000; // 1 millisecond
    let microsecond1 = 1_000; // 1 microsecond
    let res = '';
    if (currentNs >= hour1) {
      res += this.getCompletionTime(Math.floor(currentNs / hour1), 2) + ':';
      currentNs = currentNs - Math.floor(currentNs / hour1) * hour1;
    }
    if (currentNs >= minute1) {
      res += this.getCompletionTime(Math.floor(currentNs / minute1), 2) + ':';
      currentNs = currentNs - Math.floor(ns / minute1) * minute1;
    }
    if (currentNs >= second1) {
      res += this.getCompletionTime(Math.floor(currentNs / second1), 2) + ':';
      currentNs = currentNs - Math.floor(currentNs / second1) * second1;
    } else {
      res += '00:';
    }
    if (currentNs >= millisecond1) {
      res += this.getCompletionTime(Math.floor(currentNs / millisecond1), 3) + '.';
      currentNs = currentNs - Math.floor(currentNs / millisecond1) * millisecond1;
    } else {
      res += '000.';
    }
    if (currentNs >= microsecond1) {
      res += this.getCompletionTime(Math.floor(currentNs / microsecond1), 3) + '.';
      currentNs = currentNs - Math.floor(currentNs / microsecond1) * microsecond1;
    } else {
      res += '000';
    }
    if (currentNs > 0) {
      res += this.getCompletionTime(currentNs, 3);
    }
    if (res == '') {
      res = ns + '';
    }
    return res;
  }

  public static getDurString(ns: number): string {
    let currentNs = ns;
    let second1 = 1000000000;
    let millisecond1 = 1000000;
    let res = '';
    if (currentNs >= second1) {
      let cu = currentNs / second1;
      res += cu.toFixed(3) + ' s ';
      return res;
    }
    if (currentNs >= millisecond1) {
      res += Math.floor(currentNs / millisecond1) + ' ms ';
      return res;
    }
    if (res == '') {
      res = ns + '';
    }
    return res;
  }

  private static getCompletionTime(time: number, maxLength: number): string {
    if (maxLength == 2) {
      if (time.toString().length == 2) {
        return '' + time;
      } else {
        return '0' + time;
      }
    } else if (maxLength == 3) {
      if (time.toString().length == 3) {
        return time.toString();
      } else if (time.toString().length == 2) {
        return '0' + time;
      } else {
        return '00' + time;
      }
    } else {
      return '0';
    }
  }

  public getStatusMap(): Map<string, string> {
    return Utils.statusMap;
  }

  public static removeDuplicates(array1: any[], array2: any[], key: string) {
    let obj: any = {};
    return array1.concat(array2).reduce(function (total, item) {
      if (!obj[`${item[key]}-${item['pid']}`]) {
        obj[`${item[key]}-${item['pid']}`] = true;
        total.push(item);
      }
      return total;
    }, []);
  }

  static getFrequencyWithUnit = (maxFreq: number) => {
    let maxFreqObj = {
      maxFreqName: ' ',
      maxFreq: 0,
    };
    let units: Array<string> = ['', 'K', 'M', 'G', 'T', 'E'];
    let sb = ' ';
    if (maxFreq > 0) {
      let log10: number = Math.ceil(Math.log10(maxFreq));
      let pow10: number = Math.pow(10, log10);
      let afterCeil: number = Math.ceil(maxFreq / (pow10 / 4)) * (pow10 / 4);
      maxFreqObj.maxFreq = afterCeil;
      let unitIndex: number = Math.floor(log10 / 3);
      sb = `${afterCeil / Math.pow(10, unitIndex * 3)}${units[unitIndex + 1]}`;
    }
    maxFreqObj.maxFreqName = sb.toString();
    return maxFreqObj;
  };

  public static getTimeIsCross(startTime: number, endTime: number, startTime1: number, endTime1: number) {
    return Math.max(startTime, startTime1) <= Math.min(endTime, endTime1);
  }
}
