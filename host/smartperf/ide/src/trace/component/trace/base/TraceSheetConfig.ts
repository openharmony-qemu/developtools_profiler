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

import { TabPaneCurrentSelection } from '../sheet/TabPaneCurrentSelection.js';
import { TabPaneFreq } from '../sheet/freq/TabPaneFreq.js';
import { TabPaneCpuByThread } from '../sheet/cpu/TabPaneCpuByThread.js';
import { SelectionParam } from '../../../bean/BoxSelection.js';
import { TabPaneCpuByProcess } from '../sheet/cpu/TabPaneCpuByProcess.js';
import { TabPaneCpuUsage } from '../sheet/cpu/TabPaneCpuUsage.js';
import { TabPaneSPT } from '../sheet/cpu/TabPaneSPT.js';
import { TabPanePTS } from '../sheet/cpu/TabPanePTS.js';
import { TabPaneSlices } from '../sheet/process/TabPaneSlices.js';
import { TabPaneCounter } from '../sheet/process/TabPaneCounter.js';
import { TabPaneFps } from '../sheet/fps/TabPaneFps.js';
import { TabPaneFlag } from '../timer-shaft/TabPaneFlag.js';
import { TabPaneBoxChild } from '../sheet/cpu/TabPaneBoxChild.js';
import { TabPaneNMStatstics } from '../sheet/native-memory/TabPaneNMStatstics.js';
import { TabPaneNMemory } from '../sheet/native-memory/TabPaneNMemory.js';
import { TabPaneNMSampleList } from '../sheet/native-memory/TabPaneNMSampleList.js';
import { TabpanePerfProfile } from '../sheet/hiperf/TabPerfProfile.js';
import { TabPanePerfSample } from '../sheet/hiperf/TabPerfSampleList.js';
import { TabPaneLiveProcesses } from '../sheet/ability/TabPaneLiveProcesses.js';
import { TabPaneHistoryProcesses } from '../sheet/ability/TabPaneHistoryProcesses.js';
import { TabPaneCpuAbility } from '../sheet/ability/TabPaneCpuAbility.js';
import { TabPaneMemoryAbility } from '../sheet/ability/TabPaneMemoryAbility.js';
import { TabPaneDiskAbility } from '../sheet/ability/TabPaneDiskAbility.js';
import { TabPaneNetworkAbility } from '../sheet/ability/TabPaneNetworkAbility.js';
import { TabPaneFileStatistics } from '../sheet/file-system/TabPaneFilesystemStatistics.js';
import { TabpaneFilesystemCalltree } from '../sheet/file-system/TabPaneFileSystemCalltree.js';
import { TabPaneFileSystemEvents } from '../sheet/file-system/TabPaneFileSystemEvents.js';
import { TabPaneFileSystemDescHistory } from '../sheet/file-system/TabPaneFileSystemDescHistory.js';
import { TabPaneFileSystemDescTimeSlice } from '../sheet/file-system/TabPaneFileSystemDescTimeSlice.js';
import { TabPaneSdkSlice } from '../sheet/sdk/TabPaneSdkSlice.js';
import { TabPaneSdkCounter } from '../sheet/sdk/TabPaneSdkCounter.js';
import { TabPaneCounterSample } from '../sheet/cpu/TabPaneCounterSample.js';
import { TabPaneThreadStates } from '../sheet/process/TabPaneThreadStates.js';
import { TabPaneThreadUsage } from '../sheet/process/TabPaneThreadUsage.js';
import { TabPaneFrequencySample } from '../sheet/cpu/TabPaneFrequencySample.js';
import { TabPaneEnergyAnomaly } from '../sheet/energy/TabPaneEnergyAnomaly.js';
import { TabPaneSystemDetails } from '../sheet/energy/TabPaneSystemDetails.js';
import { TabPanePowerDetails } from '../sheet/energy/TabPanePowerDetails.js';
import { TabPanePowerBattery } from '../sheet/energy/TabPanePowerBattery.js';
import { TabPaneCpuStateClick } from '../sheet/cpu/TabPaneCpuStateClick.js';
import { TabPaneVirtualMemoryStatistics } from '../sheet/file-system/TabPaneVirtualMemoryStatistics.js';
import { TabPaneIOTierStatistics } from '../sheet/file-system/TabPaneIOTierStatistics.js';
import { TabPaneIOCallTree, TabPaneVMCallTree } from '../sheet/file-system/TabPaneIOCallTree.js';
import { TabPaneIoCompletionTimes } from '../sheet/file-system/TabPaneIoCompletionTimes.js';
import { TabPaneVirtualMemoryEvents } from '../sheet/file-system/TabPaneVMEvents.js';
import { TabPaneSmapsStatistics } from '../sheet/smaps/TabPaneSmapsStatistics.js';
import { TabPaneSmapsRecord } from '../sheet/smaps/TabPaneSmapsRecord.js';
import { TabPaneFreqLimit } from '../sheet/freq/TabPaneFreqLimit.js';
import { TabPaneCpuFreqLimits } from '../sheet/freq/TabPaneCpuFreqLimits.js';
import { TabpaneNMCalltree } from '../sheet/native-memory/TabPaneNMCallTree.js';
import { TabPaneClockCounter } from '../sheet/clock/TabPaneClockCounter.js';
import { TabPaneIrqCounter } from '../sheet/irq/TabPaneIrqCounter.js';
import { TabPaneFrames } from '../sheet/jank/TabPaneFrames.js';
import { TabPaneSummary } from '../sheet/snapshot/TabPaneSummary.js';
import { TabPaneComparison } from '../sheet/snapshot/TabPaneComparison.js';
import { TabPanePerfAnalysis } from '../sheet/hiperf/TabPanePerfAnalysis.js';
import { TabPaneNMStatisticAnalysis } from '../sheet/native-memory/TabPaneNMStatisticAnalysis.js';
import { TabPaneFilesystemStatisticsAnalysis } from '../sheet/file-system/TabPaneFilesystemStatisticsAnalysis.js';
import { TabPaneIOTierStatisticsAnalysis } from '../sheet/file-system/TabPaneIOTierStatisticsAnalysis.js';
import { TabPaneVirtualMemoryStatisticsAnalysis } from '../sheet/file-system/TabPaneVirtualMemoryStatisticsAnalysis.js';
export let tabConfig: any = {
  'current-selection': {
    title: 'Current Selection',
    type: TabPaneCurrentSelection,
  }, //cpu data click
  'cpu-state-click': {
    title: 'Cpu State',
    type: TabPaneCpuStateClick,
  },
  'box-freq': {
    title: 'Frequency',
    type: TabPaneFreq,
  }, //freq data click
  'box-freq-limit': {
    title: 'Frequency Limits',
    type: TabPaneFreqLimit,
  },
  'box-cpu-freq-limit': {
    title: 'Cpu Frequency Limits',
    type: TabPaneCpuFreqLimits,
    require: (param: SelectionParam) => param.cpuFreqLimitDatas.length > 0,
  },
  'box-cpu-thread': {
    title: 'CPU by thread',
    type: TabPaneCpuByThread,
    require: (param: SelectionParam) => param.cpus.length > 0,
  }, //range select
  'box-cpu-process': {
    title: 'CPU by process',
    type: TabPaneCpuByProcess,
    require: (param: SelectionParam) => param.cpus.length > 0,
  },
  'box-cpu-usage': {
    title: 'CPU Usage',
    type: TabPaneCpuUsage,
    require: (param: SelectionParam) => param.cpus.length > 0,
  },
  'box-spt': {
    title: 'Thread Switches',
    type: TabPaneSPT,
    require: (param: SelectionParam) => param.cpus.length > 0,
  },
  'box-pts': {
    title: 'Thread States',
    type: TabPanePTS,
    require: (param: SelectionParam) => param.cpus.length > 0,
  },
  'box-thread-states': {
    title: 'Thread States',
    type: TabPaneThreadStates,
    require: (param: SelectionParam) => param.threadIds.length > 0,
  },
  'box-thread-usage': {
    title: 'Thread Usage',
    type: TabPaneThreadUsage,
    require: (param: SelectionParam) => param.threadIds.length > 0,
  },
  'box-slices': {
    title: 'Slices',
    type: TabPaneSlices,
    require: (param: SelectionParam) => param.funTids.length > 0 || param.funAsync.length > 0,
  },
  'box-counters': {
    title: 'Counters',
    type: TabPaneCounter,
    require: (param: SelectionParam) => param.processTrackIds.length > 0 || param.virtualTrackIds.length > 0,
  },
  'box-clock-counters': {
    title: 'Clock Counters',
    type: TabPaneClockCounter,
    require: (param: SelectionParam) => param.clockMapData.size > 0,
  },
  'box-irq-counters': {
    title: 'Irq Counters',
    type: TabPaneIrqCounter,
    require: (param: SelectionParam) => param.irqMapData.size > 0,
  },
  'box-fps': {
    title: 'FPS',
    type: TabPaneFps,
    require: (param: SelectionParam) => param.hasFps,
  },
  'box-flag': {
    title: 'Current Selection',
    type: TabPaneFlag,
  },
  'box-cpu-child': {
    title: '',
    type: TabPaneBoxChild,
  },
  'box-native-statstics': {
    title: 'Statistics',
    type: TabPaneNMStatstics,
    require: (param: SelectionParam) => param.nativeMemory.length > 0,
  },
  'box-native-statistic-analysis': {
    title: 'Analysis',
    type: TabPaneNMStatisticAnalysis,
    require: (param: SelectionParam) => param.nativeMemory.length > 0 || param.nativeMemoryStatistic.length > 0,
  },
  'box-native-calltree': {
    title: 'Call Info',
    type: TabpaneNMCalltree,
    require: (param: SelectionParam) => param.nativeMemory.length > 0 || param.nativeMemoryStatistic.length > 0,
  },
  'box-native-memory': {
    title: 'Native Memory',
    type: TabPaneNMemory,
    require: (param: SelectionParam) => param.nativeMemory.length > 0,
  },
  'box-native-sample': {
    title: 'Snapshot List',
    type: TabPaneNMSampleList,
    require: (param: SelectionParam) => param.nativeMemory.length > 0,
  },
  'box-perf-analysis': {
    title: 'Analysis',
    type: TabPanePerfAnalysis,
    require: (param: SelectionParam) => param.perfSampleIds.length > 0,
  },
  'box-perf-profile': {
    title: 'Perf Profile',
    type: TabpanePerfProfile,
    require: (param: SelectionParam) => param.perfSampleIds.length > 0,
  },
  'box-perf-sample': {
    title: 'Sample List',
    type: TabPanePerfSample,
    require: (param: SelectionParam) => param.perfSampleIds.length > 0,
  },
  'box-live-processes-child': {
    title: 'Live Processes',
    type: TabPaneLiveProcesses,
    require: (param: SelectionParam) =>
      param.cpuAbilityIds.length > 0 ||
      (param.memoryAbilityIds.length > 0 && param.diskAbilityIds.length > 0) ||
      param.networkAbilityIds.length > 0,
  },
  'box-history-processes-child': {
    title: 'Processes History',
    type: TabPaneHistoryProcesses,
    require: (param: SelectionParam) =>
      param.cpuAbilityIds.length > 0 ||
      param.memoryAbilityIds.length > 0 ||
      param.diskAbilityIds.length > 0 ||
      param.networkAbilityIds.length > 0,
  },
  'box-system-cpu-child': {
    title: 'System CPU Summary',
    type: TabPaneCpuAbility,
    require: (param: SelectionParam) => param.cpuAbilityIds.length > 0,
  },
  'box-system-memory-child': {
    title: 'System Memory Summary',
    type: TabPaneMemoryAbility,
    require: (param: SelectionParam) => param.memoryAbilityIds.length > 0,
  },
  'box-system-diskIo-child': {
    title: 'System Disk Summary',
    type: TabPaneDiskAbility,
    require: (param: SelectionParam) => param.diskAbilityIds.length > 0,
  },
  'box-system-network-child': {
    title: 'System Network Summary',
    type: TabPaneNetworkAbility,
    require: (param: SelectionParam) => param.networkAbilityIds.length > 0,
  },
  'box-sdk-slice-child': {
    title: 'Sdk Slice',
    type: TabPaneSdkSlice,
    require: (param: SelectionParam) => param.sdkSliceIds.length > 0,
  },
  'box-system-counter-child': {
    title: 'SDK Counter',
    type: TabPaneSdkCounter,
    require: (param: SelectionParam) => param.sdkCounterIds.length > 0,
  },
  'box-counter-sample': {
    title: 'Cpu State',
    type: TabPaneCounterSample,
    require: (param: SelectionParam) => param.cpuStateFilterIds.length > 0,
  },

  'box-frequency-sample': {
    title: 'Cpu Frequency',
    type: TabPaneFrequencySample,
    require: (param: SelectionParam) => param.cpuFreqFilterIds.length > 0,
  },
  'box-anomaly-details': {
    title: 'Anomaly details',
    type: TabPaneEnergyAnomaly,
    require: (param: SelectionParam) => param.anomalyEnergy.length > 0,
  },
  'box-system-details': {
    title: 'System Details',
    type: TabPaneSystemDetails,
    require: (param: SelectionParam) => param.systemEnergy.length > 0,
  },
  'box-power-battery': {
    title: 'Power Battery',
    type: TabPanePowerBattery,
    require: (param: SelectionParam) => param.powerEnergy.length > 0,
  },
  'box-power-details': {
    title: 'Power Details',
    type: TabPanePowerDetails,
    require: (param: SelectionParam) => param.powerEnergy.length > 0,
  },
  'box-file-system-statistics': {
    title: 'Filesystem statistics',
    type: TabPaneFileStatistics,
    require: (param: SelectionParam) => param.fileSystemType.length > 0,
  },
  'box-file-system-statistics-analysis': {
    title: 'Analysis',
    type: TabPaneFilesystemStatisticsAnalysis,
    require: (param: SelectionParam) => param.fileSystemType.length > 0,
  },
  'box-file-system-calltree': {
    title: 'Filesystem Calltree',
    type: TabpaneFilesystemCalltree,
    require: (param: SelectionParam) => param.fileSystemType.length > 0 || param.fsCount > 0,
  },
  'box-file-system-event': {
    title: 'Filesystem Events',
    type: TabPaneFileSystemEvents,
    require: (param: SelectionParam) => param.fileSystemType.length > 0,
  },
  'box-file-system-desc-history': {
    title: 'File Descriptor History',
    type: TabPaneFileSystemDescHistory,
    require: (param: SelectionParam) => param.fileSystemType.length > 0,
  },
  'box-file-system-desc-time-slice': {
    title: 'File Descriptor Time Slice',
    type: TabPaneFileSystemDescTimeSlice,
    require: (param: SelectionParam) => param.fileSystemType.length > 0,
  },
  'box-virtual-memory-statistics': {
    title: 'Page Fault Statistics',
    type: TabPaneVirtualMemoryStatistics,
    require: (param: SelectionParam) => param.fileSysVirtualMemory,
  },
  'box-virtual-memory-statistics-analysis': {
    title: 'Analysis',
    type: TabPaneVirtualMemoryStatisticsAnalysis,
    require: (param: SelectionParam) => param.fileSysVirtualMemory,
  },
  'box-vm-calltree': {
    title: 'Page Fault CallTree',
    type: TabPaneVMCallTree,
    require: (param: SelectionParam) => param.fileSysVirtualMemory || param.vmCount > 0,
  },
  'box-vm-events': {
    title: 'Page Fault Events',
    type: TabPaneVirtualMemoryEvents,
    require: (param: SelectionParam) => param.fileSysVirtualMemory,
  },
  'box-io-tier-statistics': {
    title: 'Disk I/O Tier Statistics',
    type: TabPaneIOTierStatistics,
    require: (param: SelectionParam) => param.diskIOLatency,
  },
  'box-io-tier-statistics-analysis': {
    title: 'Analysis',
    type: TabPaneIOTierStatisticsAnalysis,
    require: (param: SelectionParam) => param.diskIOLatency,
  },
  'box-io-calltree': {
    title: 'Disk I/O Latency Calltree',
    type: TabPaneIOCallTree,
    require: (param: SelectionParam) => param.diskIOLatency || param.diskIOipids.length > 0,
  },
  'box-io-events': {
    title: 'Trace Completion Times',
    type: TabPaneIoCompletionTimes,
    require: (param: SelectionParam) => param.diskIOLatency,
  },
  'box-smaps-statistics': {
    title: 'VM Tracker Statistics',
    type: TabPaneSmapsStatistics,
    require: (param: SelectionParam) => param.smapsType.length > 0,
  },
  'box-smaps-record': {
    title: 'VM Tracker Record List',
    type: TabPaneSmapsRecord,
    require: (param: SelectionParam) => param.smapsType.length > 0,
  },
  'box-frames': {
    title: 'Frames',
    type: TabPaneFrames,
    require: (param: SelectionParam) => param.jankFramesData.length > 0,
  },
  'box-heap-summary': {
    title: 'Summary',
    type: TabPaneSummary,
    require: (param: SelectionParam) => param.jsMemory.length > 0,
  },
  'box-heap-comparison': {
    title: 'Comparison',
    type: TabPaneComparison,
  }, // snapshot data click
};
