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
import { SpRecordTrace } from '../../../dist/trace/component/SpRecordTrace.js';
// @ts-ignore
import { EventCenter } from '../../../dist/trace/component/trace/base/EventCenter.js';
import '../../../dist/trace/SpApplication.js';
// @ts-ignore
import { SpApplication } from '../../../dist/trace/SpApplication.js';
// @ts-ignore
import { BaseElement } from '../../../dist/base-ui/BaseElement.js';
declare global {
  interface Window {
    SmartEvent: {
      UI: {
        DeviceConnect: string;
        DeviceDisConnect: string;
      };
    };
    subscribe(evt: string, fn: (b: any) => void): void;
    unsubscribe(evt: string, fn: (b: any) => void): void;
    subscribeOnce(evt: string, fn: (b: any) => void): void;
    publish(evt: string, data: any): void;
    clearTraceRowComplete(): void;
  }
}

window.SmartEvent = {
  UI: {
    DeviceConnect: 'SmartEvent-DEVICE_CONNECT',
    DeviceDisConnect: 'SmartEvent-DEVICE_DISCONNECT',
  },
};

Window.prototype.subscribe = (ev, fn) => EventCenter.subscribe(ev, fn);
Window.prototype.unsubscribe = (ev, fn) => EventCenter.unsubscribe(ev, fn);
Window.prototype.publish = (ev, data) => EventCenter.publish(ev, data);
Window.prototype.subscribeOnce = (ev, data) => EventCenter.subscribeOnce(ev, data);
Window.prototype.clearTraceRowComplete = () => EventCenter.clearTraceRowComplete();
// @ts-ignore
window.ResizeObserver =
  window.ResizeObserver ||
  jest.fn().mockImplementation(() => ({
    disconnect: jest.fn(),
    observe: jest.fn(),
    unobserve: jest.fn(),
  }));

const intersectionObserverMock = () => ({
  observe: () => null,
});
window.IntersectionObserver = jest.fn().mockImplementation(intersectionObserverMock);

describe('SpRecordTrace Test', () => {
  SpRecordTrace.patentNode = jest.fn(()=> document.createElement('div'))
  let spRecordTrace = new SpRecordTrace();
  it('SpRecordTraceTest01', function () {
    expect(SpRecordTrace.initHtml).not.toBe('');
  });

  it('SpRecordTraceTest02', function () {
    SpRecordTrace.patentNode = jest.fn(() => true);
    expect(SpRecordTrace.initElements).toBeUndefined();
  });

  it('SpRecordTraceTest04', function () {
    let traceEvents = (SpRecordTrace.createTraceEvents = [
      'Scheduling details',
      'CPU Frequency and idle states',
      'High frequency memory',
      'Advanced ftrace config',
      'Syscalls',
      'Board voltages & frequency',
    ]);
    expect(traceEvents[0].indexOf('binder/binder_lock')).toBe(-1);
  });

  it('SpRecordTraceTest05', function () {
    spRecordTrace.spAllocations = jest.fn(() => undefined);
    spRecordTrace.spAllocations.appProcess = jest.fn(() => '');
    spRecordTrace.spAllocations.appProcess.indexOf = jest.fn(() => '');
    spRecordTrace.spAllocations.appProcess.lastIndexOf = jest.fn(() => 1);
    spRecordTrace.spAllocations.appProcess.slice = jest.fn(() => 1);
    expect(spRecordTrace.createNativePluginConfig(1)).toEqual({
      configData: {
        blocked: true,
        fileName: '',
        filterSize: undefined,
        fpUnwind: undefined,
        maxStackDepth: undefined,
        pid: 1,
        processName: '',
        saveFile: false,
        smbPages: undefined,
        stringCompressed: true,
      },
      pluginName: 'nativehook',
      sampleInterval: 1000,
    });
  });

  it('SpRecordTraceTest06', function () {
    expect(spRecordTrace.createFpsPluginConfig()).not.toBeUndefined();
  });
  it('SpRecordTraceTest07', function () {
    expect(spRecordTrace.vs).not.toBeUndefined();
  });
  it('SpRecordTraceTest08', function () {
    spRecordTrace.vs = true;
    expect(spRecordTrace.vs).toBeTruthy();
  });

  it('SpRecordTraceTest10', function () {
    let devs = {
      length: 0,
    };
    spRecordTrace.deviceSelect = document.createElement('select');
    let option = document.createElement('option');
    spRecordTrace.deviceSelect.add(option)
    expect(spRecordTrace.compareArray(devs)).toBeTruthy();
  });
  it('SpRecordTraceTest09', function () {
    spRecordTrace.vs = false;
    expect(spRecordTrace.vs).toBeFalsy();
  });
  it('SpRecordTraceTest11', function () {
    let devs = {
      length: 1,
    };
    expect(spRecordTrace.compareArray(!devs)).toBeTruthy();
  });
  it('SpRecordTraceTest12', function () {
    spRecordTrace.showHint = true;
    expect(spRecordTrace.showHint).toBeTruthy();
  });
  it('SpRecordTraceTest13', function () {
    spRecordTrace.showHint = false;
    expect(spRecordTrace.showHint).toBeFalsy();
  });
  it('SpRecordTraceTest14', function () {
    let event = {
      isTrusted: true,
      device: {
        serialNumber: 'string',
      },
    };
    expect(spRecordTrace.usbDisConnectionListener(event)).toBeUndefined();
  });
  it('SpRecordTraceTest15', function () {
    let traceResult = {
      indexOf: jest.fn(() => undefined),
    };

    expect(spRecordTrace.isSuccess(traceResult)).toBe(1);
  });
  it('SpRecordTraceTest16', function () {
    expect(spRecordTrace.createSessionRequest()).toStrictEqual({
      pluginConfigs: [],
      requestId: 1,
      sessionConfig: {
        buffers: [{ pages: 16384, policy: 0 }],
        keepAliveTime: 0,
        resultMaxSize: 0,
        sessionMode: 0,
      },
    });
  });
  it('SpRecordTraceTest17', function () {
    let that = {
      createProcessPlugin: jest.fn(() => undefined),
      createCpuPlugin: jest.fn(() => undefined),
      createDiskIOPlugin: jest.fn(() => undefined),
      createNetworkPlugin: jest.fn(() => undefined),
    };
    let request = {
      pluginConfigs: {
        push: jest.fn(() => undefined),
      },
    };
    expect(spRecordTrace.createMonitorPlugin(that, request)).toBeUndefined();
  });
  it('SpRecordTraceTest18', function () {
    expect(spRecordTrace.createNetworkPlugin()).toStrictEqual({
      configData: {},
      pluginName: 'network-plugin',
      sampleInterval: 1000,
    });
  });
  it('SpRecordTraceTest19', function () {
    expect(spRecordTrace.createDiskIOPlugin()).toStrictEqual({
      configData: { reportIoStats: 'IO_REPORT' },
      pluginName: 'diskio-plugin',
      sampleInterval: 1000,
    });
  });
  it('SpRecordTraceTest20', function () {
    expect(spRecordTrace.createCpuPlugin()).toStrictEqual({
      configData: { pid: 0, reportProcessInfo: true },
      pluginName: 'cpu-plugin',
      sampleInterval: 1000,
    });
  });
  it('SpRecordTraceTest21', function () {
    expect(spRecordTrace.createProcessPlugin()).toStrictEqual({
      configData: {
        report_cpu: true,
        report_diskio: true,
        report_process_tree: true,
        report_pss: true,
      },
      pluginName: 'process-plugin',
      sampleInterval: 1000,
    });
  });
  it('SpRecordTraceTest22', function () {
    let traceConfig = {
      forEach: jest.fn(() => undefined),
    };
    expect(spRecordTrace.createTraceEvents(traceConfig)).toStrictEqual([]);
  });
  it('SpRecordTraceTest23', function () {
    spRecordTrace.spRecordPerf = jest.fn(() => undefined);
    spRecordTrace.spRecordPerf.getPerfConfig = jest.fn(() => undefined);
    expect(spRecordTrace.createHiperConfig()).toStrictEqual({
      configData: {
        isRoot: false,
        outfileName: '/data/local/tmp/perf.data',
        recordArgs: '-f undefined -a  --cpu-limit undefined -e hw-cpu-cycles --call-stack undefined -j undefined',
      },
      pluginName: 'hiperf-plugin',
      sampleInterval: NaN,
    });
  });

  it('SpRecordTraceTest24', function () {
    expect(spRecordTrace.isSuccess('Signal')).toBe(2);
  });

  it('SpRecordTraceTest25', function () {
    expect(spRecordTrace.isSuccess('The device is abnormal')).toBe(-1);
  });

  it('SpRecordTraceTest26', function () {
    expect(spRecordTrace.isSuccess('')).toBe(0);
  });
  it('SpRecordTraceTest27', function () {
    expect(spRecordTrace.synchronizeDeviceList()).toBeUndefined();
  });
  it('SpRecordTraceTest28', function () {
    expect(spRecordTrace.freshMenuItemsStatus('Trace command')).toBeUndefined();
  });
  it('SpRecordTraceTest29', function () {
    expect(spRecordTrace.buttonDisable(true)).toBeUndefined();
  });
  it('SpRecordTraceTest30', function () {
    expect(spRecordTrace.startRefreshDeviceList()).toBeUndefined();
  });
  it('SpRecordTraceTest31', function () {
    expect(spRecordTrace.freshConfigMenuDisable(true)).toBeUndefined();
  });
  it('SpRecordTraceTest31', function () {
    expect(spRecordTrace.createSdkConfig()).toStrictEqual({ configData: {}, pluginName: '', sampleInterval: 5000 });
  });
  it('SpRecordTraceTest32', function () {
    expect(spRecordTrace.createHtracePluginConfig()).toStrictEqual({
      configData: {
        bufferSizeKb: 20480,
        clock: 'boot',
        debugOn: false,
        flushIntervalMs: 1000,
        flushThresholdKb: 4096,
        ftraceEvents: [
          'sched/sched_switch',
          'power/suspend_resume',
          'sched/sched_wakeup',
          'sched/sched_wakeup_new',
          'sched/sched_waking',
          'sched/sched_process_exit',
          'sched/sched_process_free',
          'task/task_newtask',
          'task/task_rename',
          'power/cpu_frequency',
          'power/cpu_idle',
        ],
        hitraceApps: [],
        hitraceCategories: [
          'ability',
          'ace',
          'app',
          'ark',
          'binder',
          'disk',
          'freq',
          'graphic',
          'idle',
          'irq',
          'memreclaim',
          'mmc',
          'multimodalinput',
          'ohos',
          'pagecache',
          'rpc',
          'sched',
          'sync',
          'window',
          'workq',
          'zaudio',
          'zcamera',
          'zimage',
          'zmedia',
        ],
        parseKsyms: true,
        rawDataPrefix: '',
        traceDurationMs: 0,
        tracePeriodMs: 200,
      },
      pluginName: 'ftrace-plugin',
      sampleInterval: 1000,
    });
  });
  it('SpRecordTraceTest33', function () {
    expect(spRecordTrace.createArkTsConfig()).toStrictEqual({
      configData: {
        capture_numeric_value: false,
        cpu_profiler_interval: 1000,
        interval: 0,
        enable_cpu_profiler: false,
        pid: 0,
        track_allocations: false,
        type: -1,
      },
      pluginName: 'arkts-plugin',
      sampleInterval: 5000,
    });
  });
  it('SpRecordTraceTest34', function () {
    expect(spRecordTrace.createMemoryPluginConfig(1, true, true, true)).toStrictEqual({
      configData: {
        pid: [0],
        reportAppMemByMemoryService: false,
        reportAppMemInfo: false,
        reportProcessMemInfo: true,
        reportProcessTree: true,
        reportSmapsMemInfo: true,
        reportSysmemMemInfo: true,
        reportDmaMemInfo: true,
        reportGpuDumpInfo: true,
        reportGpuMemInfo: true,
        reportSysmemVmemInfo: true,
        reportPurgeableAshmemInfo: true,
        sysMeminfoCounters: [
          'PMEM_MEM_TOTAL',
          'PMEM_MEM_FREE',
          'PMEM_BUFFERS',
          'PMEM_CACHED',
          'PMEM_SHMEM',
          'PMEM_SLAB',
          'PMEM_SWAP_TOTAL',
          'PMEM_SWAP_FREE',
          'PMEM_MAPPED',
          'PMEM_VMALLOC_USED',
          'PMEM_PAGE_TABLES',
          'PMEM_KERNEL_STACK',
          'PMEM_ACTIVE',
          'PMEM_INACTIVE',
          'PMEM_UNEVICTABLE',
          'PMEM_VMALLOC_TOTAL',
          'PMEM_SLAB_UNRECLAIMABLE',
          'PMEM_CMA_TOTAL',
          'PMEM_CMA_FREE',
          'PMEM_KERNEL_RECLAIMABLE',
        ],
        sysVmeminfoCounters: [],
      },
      pluginName: 'memory-plugin',
      sampleInterval: 1000,
    });
  });
  it('SpRecordTraceTest35', function () {
    expect(spRecordTrace.createSystemConfig()).toStrictEqual({
      configData: { cmdLine: 'hiebpf --duration 30 --max_stack_depth 10', outfileName: '/data/local/tmp/ebpf.data' },
      pluginName: 'hiebpf-plugin',
      sampleInterval: 1000,
    });
  });
  it('SpRecordTraceTest36', function () {
    expect(spRecordTrace.createSystemConfig({}, 1)).toStrictEqual({
      configData: { cmdLine: 'hiebpf --duration 30 --max_stack_depth 10', outfileName: '/data/local/tmp/ebpf.data' },
      pluginName: 'hiebpf-plugin',
      sampleInterval: 1000,
    });
  });
  it('SpRecordTraceTest37', function () {
    spRecordTrace.record_template = 'record_template';
    expect(spRecordTrace.record_template).toBeTruthy();
  });
});
