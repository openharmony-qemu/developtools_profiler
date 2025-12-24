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

export namespace hidebug {
  loadLibraryWithPermissionCheck("hidebug_ani", "@ohos.hidebug");

  export native function startJsCpuProfiling(filename: string): void;

  export native function stopJsCpuProfiling(): void;

  native function getNativeHeapSizeAdapter(): long;

  export function getNativeHeapSize(): bigint {
    return new bigint(getNativeHeapSizeAdapter());
  }

  native function getNativeHeapAllocatedSizeAdapter(): long;

  export function getNativeHeapAllocatedSize(): bigint {
    return new bigint(getNativeHeapAllocatedSizeAdapter());
  }

  native function getNativeHeapFreeSizeAdapter(): long;

  export function getNativeHeapFreeSize(): bigint {
    return new bigint(getNativeHeapFreeSizeAdapter());
  }

  native function getVssAdapter(): long;

  export function getVss(): bigint {
     return new bigint(getVssAdapter());
  }

  native function getPssAdapter(): long;

  export function getPss(): bigint {
    return new bigint(getPssAdapter());
  }

  native function getSharedDirtyAdapter(): long;

  export function getSharedDirty(): bigint {
    return new bigint(getSharedDirtyAdapter());
  }

  native function getPrivateDirtyAdapter(): long;

  export function getPrivateDirty(): bigint {
    return new bigint(getPrivateDirtyAdapter());
  }

  export native function getCpuUsage(): number;

  export native function getServiceDump(serviceid: int, fd: int, args: Array<string>): void;

  export native function getSystemCpuUsage(): number;

  export interface ThreadCpuUsage {
    threadId: long;
    cpuUsage: double;
  }

  class ThreadCpuUsageImpl implements ThreadCpuUsage {
    threadId: long;
    cpuUsage: double;
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

  class SystemMemInfoAdapter {
    totalMem: long;
    freeMem: long;
    availableMem: long;
  }

  native function getSystemMemInfoAdapter() : SystemMemInfoAdapter;

  export function getSystemMemInfo(): SystemMemInfo {
    let systemMemoryInfo = getSystemMemInfoAdapter();
    let retValue = new SystemMemInfoImpl();
    retValue.totalMem = new bigint(systemMemoryInfo.totalMem);
    retValue.freeMem = new bigint(systemMemoryInfo.freeMem);
    retValue.availableMem = new bigint(systemMemoryInfo.availableMem);
    return retValue;
  }

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

  class NativeMemInfoAdapter {
    pss: long;
    vss: long;
    rss: long;
    sharedDirty: long;
    privateDirty: long;
    sharedClean: long;
    privateClean: long;
  }

  native function getAppNativeMemInfoAdapter(forceRefresh: boolean): NativeMemInfoAdapter;

  export function getAppNativeMemInfoWithCache(forceRefresh?: boolean): NativeMemInfo {
    let nativeMemoryInfo = getAppNativeMemInfoAdapter(forceRefresh === true);
    let retValue = new NativeMemInfoImpl();
    retValue.pss = new bigint(nativeMemoryInfo.pss);
    retValue.vss = new bigint(nativeMemoryInfo.vss);
    retValue.rss = new bigint(nativeMemoryInfo.rss);
    retValue.sharedDirty = new bigint(nativeMemoryInfo.sharedDirty);
    retValue.privateDirty = new bigint(nativeMemoryInfo.privateDirty);
    retValue.sharedClean = new bigint(nativeMemoryInfo.sharedClean);
    retValue.privateClean = new bigint(nativeMemoryInfo.privateClean);
    return retValue;
  }

  export function getAppNativeMemInfo(): NativeMemInfo {
    return getAppNativeMemInfoWithCache();
  }

  export function getAppNativeMemInfoAsync(): Promise<NativeMemInfo> {
    return new Promise<NativeMemInfo>((resolve: (v: NativeMemInfo) => void,
      reject: (e: BusinessError<void>) => void) => {
        taskpool.execute(getAppNativeMemInfo).then((ret: Any): void => {
          resolve(ret as NativeMemInfo);
        }).catch((err: Object | null | undefined) => {
          reject(err as BusinessError);
        });
      });
  }

  export interface VMMemoryInfo {
    totalHeap: bigint;
    heapUsed: bigint;
    allArraySize: bigint;
  }

  class VMMemoryInfoImpl implements VMMemoryInfo {
    totalHeap: bigint;
    heapUsed: bigint;
    allArraySize: bigint;
  }

  class VMMemoryInfoAdapter {
    totalHeap: long;
    heapUsed: long;
    allArraySize: long;
  }

  native function getAppVMMemoryInfoAdapter(): VMMemoryInfoAdapter;

  export function getAppVMMemoryInfo(): VMMemoryInfo {
    let vmMemoryInfo = getAppVMMemoryInfoAdapter();
    let ret = new VMMemoryInfoImpl();
    ret.totalHeap = new bigint(vmMemoryInfo.totalHeap);
    ret.heapUsed = new bigint(vmMemoryInfo.heapUsed);
    ret.allArraySize = new bigint(vmMemoryInfo.allArraySize);
    return ret;
  }

  native function getAppVMObjectUsedSizeAdapter(): long;

  export function getAppVMObjectUsedSize(): bigint {
    return new bigint(getAppVMObjectUsedSizeAdapter());
  }

  export interface MemoryLimit {
    rssLimit: bigint;
    vssLimit: bigint;
    vmHeapLimit: bigint;
    vmTotalHeapSize: bigint;
  }

  class MemoryLimitImpl implements MemoryLimit {
    rssLimit: bigint;
    vssLimit: bigint;
    vmHeapLimit: bigint;
    vmTotalHeapSize: bigint;
  }

  class MemoryLimitImplAdapter {
    rssLimit: long;
    vssLimit: long;
    vmHeapLimit: long;
    vmTotalHeapSize: long;
  }

  native function getMemoryLimitImplAdapter(): MemoryLimitImplAdapter;

  export function getAppMemoryLimit(): MemoryLimit {
    let memoryLimit = getMemoryLimitImplAdapter();
    let ret = new MemoryLimitImpl();
    ret.rssLimit = new bigint(memoryLimit.rssLimit);
    ret.vssLimit = new bigint(memoryLimit.vssLimit);
    ret.vmHeapLimit = new bigint(memoryLimit.vmHeapLimit);
    ret.vmTotalHeapSize = new bigint(memoryLimit.vmTotalHeapSize);
    return ret;
  }

  export enum TraceFlag {
    MAIN_THREAD = 1,
    ALL_THREADS = 2
  }

  export namespace tags {
    const ABILITY_MANAGER: long = 2147483648; // 1 << 31;
    const ARKUI: long = 549755813888; // 1 << 39;
    const ARK: long = 140737488355328; // 1 << 47;
    const BLUETOOTH: long = 1152921504606846976; // 1 << 60;
    const COMMON_LIBRARY: long = 65536; // 1 << 16;
    const DISTRIBUTED_HARDWARE_DEVICE_MANAGER: long = 18014398509481984; // 1 << 54;
    const DISTRIBUTED_AUDIO: long = 134217728; // 1 << 27;
    const DISTRIBUTED_CAMERA: long = 2251799813685248; // 1 << 51;
    const DISTRIBUTED_DATA: long = 68719476736; // 1 << 36;
    const DISTRIBUTED_HARDWARE_FRAMEWORK: long = 4503599627370496; // 1 << 52;
    const DISTRIBUTED_INPUT: long = 576460752303423488; // 1 << 59;
    const DISTRIBUTED_SCREEN: long = 1125899906842624; // 1 << 50;
    const DISTRIBUTED_SCHEDULER: long = 144115188075855872; // 1 << 57;
    const FFRT: long = 8192; // 1 << 13;
    const FILE_MANAGEMENT: long = 536870912; // 1 << 29;
    const GLOBAL_RESOURCE_MANAGER: long = 9007199254740992; // 1 << 53;
    const GRAPHICS: long = 274877906944; // 1 << 38;
    const HDF: long = 262144; // 1 << 18;
    const MISC: long = 2199023255552; // 1 << 41;
    const MULTIMODAL_INPUT: long = 4398046511104; // 1 << 42;
    const NET: long = 8388608; // 1 << 23;
    const NOTIFICATION: long = 1099511627776; // 1 << 40;
    const NWEB: long = 16777216; // 1 << 24;
    const OHOS: long = 1073741824; // 1 << 30;
    const POWER_MANAGER: long = 72057594037927936; // 1 << 56;
    const RPC: long = 70368744177664; // 1 << 46;
    const SAMGR: long = 36028797018963968; // 1 << 55;
    const WINDOW_MANAGER: long = 281474976710656; // 1 << 48;
    const AUDIO: long = 34359738368; // 1 << 35;
    const CAMERA: long = 4294967296; // 1 << 30;
    const IMAGE: long = 17179869184; // 1 << 34;
    const MEDIA: long = 8589934592; // 1 << 33;
  }
  type GcStats = Record<string, long>;

  export native function getVMRuntimeStats(): GcStats;

  export native function getVMRuntimeStat(item: string): long;

  export native function startAppTraceCapture(tags: long[], flag: TraceFlag, limitSize: int): string;

  export native function stopAppTraceCapture(): void;

  export native function isDebugState(): boolean;

  export function getGraphicsMemory(): Promise<int> {
    return new Promise<int>((resolve: (v: int) => void, reject: (e: BusinessError<void>) => void) => {
      taskpool.execute(getGraphicsMemorySync).then((ret: Any): void => {
        resolve(ret as int);
      }).catch((err: Object | null | undefined) => {
        reject(err as BusinessError);
      });
    });
  }

  interface GraphicsMemorySummary {
    gl: int;
    graph: int;
  }

  class GraphicsMemorySummaryImpl implements GraphicsMemorySummary {
    gl: int;
    graph: int;
  }

  native function getGraphicsMemoryAdapter(interval: int): GraphicsMemorySummary;

  export function getGraphicsMemorySummary(interval?: int): Promise<GraphicsMemorySummary> {
    return new Promise<GraphicsMemorySummary>((resolve: (v: GraphicsMemorySummary) => void,
      reject: (e: BusinessError<void>) => void) => {
        taskpool.execute(() => {
          if (interval === undefined) {
            return getGraphicsMemoryAdapter(300);
          } else {
            return getGraphicsMemoryAdapter(interval as int);
          }
        }).then((ret: Any): void => {
          resolve(ret as GraphicsMemorySummary);
        }).catch((err: Object | null | undefined) => {
          reject(err as BusinessError);
        });
      });
  }

  export function getGraphicsMemorySync(): int {
    let graphicMemory = getGraphicsMemoryAdapter(2);
    return graphicMemory.gl + graphicMemory.graph;
  }

  export interface GwpAsanOptions {
    alwaysEnabled?: boolean;
    sampleRate?: int;
    maxSimutaneousAllocations?: int;
  }

  export native function enableGwpAsanGrayscale(options?: GwpAsanOptions, duration?: int): void;

  export native function disableGwpAsanGrayscale(): void;

  export native function getGwpAsanGrayscaleState(): int;
}
