/*
* Copyright (C) 2025 Huawei Device Co., Ltd.
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

import { BusinessError } from '@ohos.base';

function createVoidBusinessError(code: number, msg: string): BusinessError<void> {
  let error = new BusinessError<void>();
  error.code = code;
  error.message = msg;
  return error;
}

export namespace hidebug {
  loadLibrary("hidebug_ani");
  export native function getNativeHeapSize(): bigint;

  export native function getNativeHeapAllocatedSize(): bigint;

  export native function getNativeHeapFreeSize(): bigint;

  export native function getVss(): bigint;

  export native function getPss(): bigint;

  export native function getSharedDirty(): bigint;

  export native function getPrivateDirty(): bigint;

  export native function getCpuUsage(): number;

  export native function getServiceDump(serviceid: number, fd: number, args: Array<string>): void;

  export native function getSystemCpuUsage(): number;

  export interface ThreadCpuUsage {
    threadId: number;
    cpuUsage: number;
  }

  class ThreadCpuUsageImpl implements ThreadCpuUsage {
    threadId: number;
    cpuUsage: number;
  }

  export native function getAppThreadCpuUsage(): ThreadCpuUsage[];

  interface SystemMemInfo {
    totalMem: bigint;
    freeMem: bigint;
    availableMem: bigint;
  }

  class SystemMemInfoImpl implements SystemMemInfo {
    totalMem: bigint;
    freeMem: bigint;
    availableMem: bigint;
  }

  export native function getSystemMemInfo(): SystemMemInfo;

  export interface NativeMemInfo {
    pss: bigint;
    vss: bigint;
    rss: bigint;
    sharedDirty: bigint;
    privateDirty: bigint;
    sharedClean: bigint;
    privateClean: bigint;
  }

  class NativeMemInfoImpl implements NativeMemInfo {
    pss: bigint;
    vss: bigint;
    rss: bigint;
    sharedDirty: bigint;
    privateDirty: bigint;
    sharedClean: bigint;
    privateClean: bigint;
  }

  export native function getAppNativeMemInfo(): NativeMemInfo;

  export enum TraceFlag {
    MAIN_THREAD = 1,
    ALL_THREADS = 2
  }

  export namespace tags {
    const ABILITY_MANAGER: number = 2147483648; // 1 << 31;
    const ARKUI: number = 549755813888; // 1 << 39;
    const ARK: number = 140737488355328; // 1 << 47;
    const BLUETOOTH: number = 1152921504606846976; // 1 << 60;
    const COMMON_LIBRARY: number = 65536; // 1 << 16;
    const DISTRIBUTED_HARDWARE_DEVICE_MANAGER: number = 18014398509481984; // 1 << 54;
    const DISTRIBUTED_AUDIO: number = 134217728; // 1 << 27;
    const DISTRIBUTED_CAMERA: number = 2251799813685248; // 1 << 51;
    const DISTRIBUTED_DATA: number = 68719476736; // 1 << 36;
    const DISTRIBUTED_HARDWARE_FRAMEWORK: number = 4503599627370496; // 1 << 52;
    const DISTRIBUTED_INPUT: number = 576460752303423488; // 1 << 59;
    const DISTRIBUTED_SCREEN: number = 1125899906842624; // 1 << 50;
    const DISTRIBUTED_SCHEDULER: number = 144115188075855872; // 1 << 57;
    const FFRT: number = 8192; // 1 << 13;
    const FILE_MANAGEMENT: number = 536870912; // 1 << 29;
    const GLOBAL_RESOURCE_MANAGER: number = 9007199254740992; // 1 << 53;
    const GRAPHICS: number = 274877906944; // 1 << 38;
    const HDF: number = 262144; // 1 << 18;
    const MISC: number = 2199023255552; // 1 << 41;
    const MULTIMODAL_INPUT: number = 4398046511104; // 1 << 42;
    const NET: number = 8388608; // 1 << 23;
    const NOTIFICATION: number = 1099511627776; // 1 << 40;
    const NWEB: number = 16777216; // 1 << 24;
    const OHOS: number = 1073741824; // 1 << 30;
    const POWER_MANAGER: number = 72057594037927936; // 1 << 56;
    const RPC: number = 70368744177664; // 1 << 46;
    const SAMGR: number = 36028797018963968; // 1 << 55;
    const WINDOW_MANAGER: number = 281474976710656; // 1 << 48;
    const AUDIO: number = 34359738368; // 1 << 35;
    const CAMERA: number = 4294967296; // 1 << 30;
    const IMAGE: number = 17179869184; // 1 << 34;
    const MEDIA: number = 8589934592; // 1 << 33;
  }

  export native function startAppTraceCapture(tags: number[], flag: TraceFlag, limitSize: number): string;

  export native function stopAppTraceCapture(): void;

  export function getGraphicsMemory(): Promise<number> {
    return new Promise<number>((resolve: (v: number) => void, reject: (e: BusinessError<void>) => void) => {
      taskpool.execute(getGraphicsMemorySync).then((ret: NullishType): void => {
        resolve(ret as number);
      }).catch((err: Object | null | undefined) => {
        reject(err as BusinessError);
      });
    });
  }

  export native function getGraphicsMemorySync(): number;
}
