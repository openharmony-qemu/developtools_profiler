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
import { PluginConvertUtils } from '../../../../../dist/trace/component/setting/utils/PluginConvertUtils.js';
// @ts-ignore
import { ProfilerSessionConfigMode } from '../../../../../dist/trace/component/setting/bean/ProfilerServiceTypes';

import {
  ProfilerSessionConfigBufferConfig,
  ProfilerSessionConfigBufferConfigPolicy,
  TracePluginConfig,
  // @ts-ignore
} from '../../../../../dist/trace/component/setting/bean/ProfilerServiceTypes.js';
import {
  HilogConfig,
  levelFromJSON,
  Type,
  // @ts-ignore
} from '../../../../../dist/trace/component/setting/bean/ProfilerServiceTypes.js';
import {
  MemoryConfig,
  sysMeminfoTypeFromJSON,
  sysVMeminfoTypeFromJSON,
  // @ts-ignore
} from '../../../../../dist/trace/component/setting/bean/ProfilerServiceTypes.js';
// @ts-ignore
import { SpRecordTrace } from '../../../../../dist/trace/component/SpRecordTrace.js';

describe('PlugConvertUtils Test', () => {
  let bufferConfig: ProfilerSessionConfigBufferConfig = {
    pages: 1000,
    policy: ProfilerSessionConfigBufferConfigPolicy.RECYCLE,
  };
  let sessionConfig = {
    buffers: [bufferConfig],
    sessionMode: ProfilerSessionConfigMode.OFFLINE,
    resultFile: '/data/local/tmp/hiprofiler_data.htrace',
    resultMaxSize: 0,
    sampleDuration: 1000,
    keepAliveTime: 0,
  };
  let tracePluginConfig: TracePluginConfig = {
    ftraceEvents: [],
    bytraceCategories: [],
    bytraceApps: [],
    bufferSizeKb: 1024,
    flushIntervalMs: 1000,
    flushThresholdKb: 4096,
    parseKsyms: true,
    clock: 'mono',
    tracePeriodMs: 200,
    rawDataPrefix: '',
    traceDurationMs: 0,
    debugOn: false,
  };
  let hilogConfig: HilogConfig = {
    deviceType: Type.HI3516,
    logLevel: levelFromJSON('Info'),
    needClear: true,
  };
  let memoryconfig: MemoryConfig = {
    reportProcessTree: true,
    reportSysmemMemInfo: true,
    sysMeminfoCounters: [],
    reportSysmemVmemInfo: true,
    sysVmeminfoCounters: [],
    reportProcessMemInfo: true,
    reportAppMemInfo: false,
    reportAppMemByMemoryService: false,
    pid: [],
  };

  SpRecordTrace.MEM_INFO.forEach((va: any) => {
    memoryconfig.sysMeminfoCounters.push(sysMeminfoTypeFromJSON(va));
  });
  SpRecordTrace.VMEM_INFO.forEach((me: any) => {
    memoryconfig.sysVmeminfoCounters.push(sysVMeminfoTypeFromJSON(me));
  });
  SpRecordTrace.VMEM_INFO_SECOND.forEach((me: any) => {
    memoryconfig.sysVmeminfoCounters.push(sysVMeminfoTypeFromJSON(me));
  });
  SpRecordTrace.VMEM_INFO_THIRD.forEach((me: any) => {
    memoryconfig.sysVmeminfoCounters.push(sysVMeminfoTypeFromJSON(me));
  });

  let request = {
    requestId: 1,
    sessionConfig: sessionConfig,
    pluginConfigs: [tracePluginConfig, hilogConfig, memoryconfig],
  };

  it('PlugConvertUtils01', function () {
    expect(PluginConvertUtils.createHdcCmd('aaaa', 11)).not.toBeNull();
  });

  it('PlugConvertUtils02', function () {
    expect(PluginConvertUtils.BeanToCmdTxt(request, true)).not.toBeNull();
  });

  it('PlugConvertUtils03', function () {
    expect(PluginConvertUtils.BeanToCmdTxtWithObjName(request, false, '', 1)).not.toBeNull();
  });
});
