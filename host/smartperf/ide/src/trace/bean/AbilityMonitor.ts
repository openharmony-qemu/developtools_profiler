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

import { CompareStruct } from '../component/trace/sheet/SheetUtils.js';

export class SystemCpuSummary {
  startTime: number = -1;
  startTimeStr: string = '';
  duration: number = -1;
  durationStr: string = '';
  totalLoad: number = -1;
  totalLoadStr: string = '';
  userLoad: number = -1;
  userLoadStr: string = '';
  systemLoad: number = -1;
  systemLoadStr: string = '';
  threads: number = -1;
  threadsStr: string = '';
}

export class SystemDiskIOSummary {
  startTime: number = -1;
  startTimeStr: string = '';
  duration: number = -1;
  durationStr: string = '';
  dataRead: number = -1;
  dataReadStr: string = '';
  dataReadSec: number = -1;
  dataReadSecStr: string = '';
  dataWrite: number = -1;
  dataWriteStr: string = '';
  dataWriteSec: number = -1;
  dataWriteSecStr: string = '';
  readsIn: number = -1;
  readsInStr: string = '';
  readsInSec: number = -1;
  readsInSecStr: string = '';
  writeOut: number = -1;
  writeOutStr: string = '';
  writeOutSec: number = -1;
  writeOutSecStr: string = '';
}

export class ProcessHistory {
  processId: number = -1;
  alive: string = ''; // 0 alive and 1 dead
  firstSeen: string = '';
  firstSeenNumber: number = -1;
  lastSeen: string = '';
  lastSeenNumber: number = -1;
  processName: string = '';
  responsibleProcess: string = '';
  userName: string = '';
  cpuTime: string = '';
  cpuTimeNumber: number = -1;
  pss: number = -1;
}

export class LiveProcess {
  processId: number = -1;
  processName: string = '';
  responsibleProcess: string = '';
  userName: string = '';
  cpu: string = '';
  threads: number = -1;
  memory: string = '';
  memoryNumber: number = -1;
  diskWrite: number = -1;
  diskReads: number = -1;
  cpuTime: string = '';
  cpuTimeNumber: number = -1;
}

export class SystemNetworkSummary {
  startTime: number = -1;
  startTimeStr: string = '';
  duration: number = -1;
  durationStr: string = '';
  dataReceived: number = -1;
  dataReceivedStr: string = '';
  dataReceivedSec: number = -1;
  dataReceivedSecStr: string = '';
  dataSend: number = -1;
  dataSendStr: string = '';
  dataSendSec: number = -1;
  dataSendSecStr: string = '';
  packetsIn: number = -1;
  packetsInStr: string = '';
  packetsInSec: number = -1;
  packetsInSecStr: string = '';
  packetsOut: number = -1;
  packetsOutStr: string = '';
  packetsOutSec: number = -1;
  packetsOutSecStr: string = '';
}

export class SystemMemorySummary {
  startTimeStr: string = '0';
  durationStr: string = '0';
  durationNumber: number = -1;
  memoryTotal: string = '0';
  memFree: string = '0';
  buffers: string = '0';
  cached: string = '0';
  shmem: string = '0';
  slab: string = '0';
  swapTotal: string = '0';
  swapFree: string = '0';
  mapped: string = '0';
  vmallocUsed: string = '0';
  pageTables: string = '0';
  kernelStack: string = '0';
  active: string = '0';
  inactive: string = '0';
  unevictable: string = '0';
  vmallocTotal: string = '0';
  sUnreclaim: string = '0';
  kReclaimable: string = '0';
  cmaTotal: string = '0';
  cmaFree: string = '0';
  zram: string = '0';
}

export class Dma {
  processId: number = -1;
  timeStamp: string = '';
  startNs: number = -1;
  expTaskComm: string | number = '';
  avgSize: number = 0;
  minSize: number = -1;
  maxSize: number = -1;
  bufName: string | number = '';
  expName: string | number = '';
  size: number = -1;
  processName: string = '';
  process: string = ''; //processName + processId
  fd: number = -1;
  ino: number = -1;
  expPid: number = -1;
  flag: number = -1;
  avgSizes: string = '';
  minSizes: string = '';
  maxSizes: string = '';
  sizes: string = '';
  sumSize: number = -1;
  sumSizes: string = '';
}

export class GpuMemory {
  processId: number = -1;
  threadId: number = -1;
  timeStamp: string = '';
  expTaskComm: string = '';
  startNs: number = -1;
  avgSize: number = -1;
  minSize: number = -1;
  maxSize: number = -1;
  gpuName: string = '';
  gpuNameId: number = -1;
  processName: string = '';
  process: string = ''; //processName + processId
  threadName: string = '';
  thread: string = ''; //threadName + threadI
  size: number = -1;
  avgSizes: string = '';
  minSizes: string = '';
  maxSizes: string = '';
  sizes: string = '';
  sumSize: number = -1;
  sumSizes: string = '';
}

export class DmaComparison extends CompareStruct {
  processId: number = -1;
  processName: string = '';
  process: string = ''; //processName + processId
  sizes: string = '';
  thread: string = '';

  constructor(process: string, value: number) {
    super(process, value);
    this.process = process;
  }

  clone(isBase?: boolean): DmaComparison {
    const value = isBase ? this.value : -this.value;
    return new DmaComparison(this.process, value);
  }
}

export class GpuMemoryComparison extends CompareStruct {
  processId: number = -1;
  processName: string = '';
  process: string = ''; //processName + processId
  sizes: string = '';
  gpuNameId: number = -1;
  gpuName: string = '';
  threadName: string = '';
  threadId: number = -1;
  thread: string = '';

  constructor(process: string, thread: string, gpuName: string, value: number) {
    super(process + '' + thread + '' + gpuName, value);
    this.process = process;
    this.gpuName = gpuName;
    this.thread = thread;
  }

  clone(isBase?: boolean): GpuMemoryComparison {
    const value = isBase ? this.value : -this.value;
    return new GpuMemoryComparison(this.process, this.thread, this.gpuName, value);
  }
}
