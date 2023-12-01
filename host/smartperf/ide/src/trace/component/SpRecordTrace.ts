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

import { BaseElement, element } from '../../base-ui/BaseElement.js';
import '../../base-ui/popover/LitPopover.js';
import '../../base-ui/button/LitButton.js';
import { LitMainMenuGroup } from '../../base-ui/menu/LitMainMenuGroup.js';
import { LitMainMenuItem } from '../../base-ui/menu/LitMainMenuItem.js';
import { SpRecordSetting } from './setting/SpRecordSetting.js';
import { LitMainMenu, MenuGroup, MenuItem } from '../../base-ui/menu/LitMainMenu.js';
import { SpProbesConfig } from './setting/SpProbesConfig.js';
import { SpTraceCommand } from './setting/SpTraceCommand.js';
import { FlagsConfig } from './SpFlags.js';
import LitSwitch from '../../base-ui/switch/lit-switch.js';
import { LitSlider } from '../../base-ui/slider/LitSlider';

import {
  CpuConfig,
  CreateSessionRequest,
  DiskioConfig,
  HiebpfConfig,
  FpsConfig,
  HilogConfig,
  HiperfPluginConfig,
  HiSystemEventConfig,
  ArkTSConfig,
  levelFromJSON,
  MemoryConfig,
  NativeHookConfig,
  NetworkConfig,
  ProcessConfig,
  ProfilerPluginConfig,
  ProfilerSessionConfig,
  ProfilerSessionConfigBufferConfig,
  ProfilerSessionConfigBufferConfigPolicy,
  ProfilerSessionConfigMode,
  sysMeminfoTypeFromJSON,
  sysVMeminfoTypeFromJSON,
  TracePluginConfig,
  Type,
} from './setting/bean/ProfilerServiceTypes.js';
import { PluginConvertUtils } from './setting/utils/PluginConvertUtils.js';
import { SpAllocations } from './setting/SpAllocations.js';
import { SpRecordPerf } from './setting/SpRecordPerf.js';
import { HdcDeviceManager } from '../../hdc/HdcDeviceManager.js';
import { LitButton } from '../../base-ui/button/LitButton.js';
import { SpApplication } from '../SpApplication.js';
import { LitSearch } from './trace/search/Search.js';
import { LitProgressBar } from '../../base-ui/progress-bar/LitProgressBar.js';
import { info, log } from '../../log/Log.js';
import { CmdConstant } from '../../command/CmdConstant.js';
import { Cmd } from '../../command/Cmd.js';
import { SpFileSystem } from './setting/SpFileSystem.js';
import { SpSdkConfig } from './setting/SpSdkConfig.js';
import { SpVmTracker } from './setting/SpVmTracker.js';
import { SpHisysEvent } from './setting/SpHisysEvent.js';
import { SpRecordTemplate } from './setting/SpRecordTemplate.js';
import { SpStatisticsHttpUtil } from '../../statistics/util/SpStatisticsHttpUtil.js';
import { SpArkTs } from './setting/SpArkTs.js';
import { SpWebHdcShell } from './setting/SpWebHdcShell.js';
import { SpHilogRecord } from './setting/SpHilogRecord.js';
import { LongTraceDBUtils } from '../database/LongTraceDBUtils.js';

@element('sp-record-trace')
export class SpRecordTrace extends BaseElement {
  public static serialNumber: string = '';
  public static selectVersion: string | null;
  // 1GB
  public static MaxFileSize: number = 1024 * 1024 * 1024;
  public static isVscode = false;
  public static cancelRecord = false;
  static supportVersions = ['3.2', '4.0+'];
  private nowChildItem: HTMLElement | undefined;
  private longTraceList: Array<string> = [];

  set record_template(re: boolean) {
    if (re) {
      this.setAttribute('record_template', '');
    } else {
      this.removeAttribute('record_template');
    }
    if (this.recordSetting) {
      this.recordSetting.isRecordTemplate = re;
    }
  }

  get record_template(): boolean {
    return this.hasAttribute('record_template');
  }

  set vs(vs: boolean) {
    if (vs) {
      SpRecordTrace.isVscode = true;
      this.setAttribute('vs', '');
    } else {
      SpRecordTrace.isVscode = false;
      this.removeAttribute('vs');
    }
  }

  get vs(): boolean {
    return this.hasAttribute('vs');
  }

  static MEM_INFO = [
    'MEMINFO_ACTIVE',
    'MEMINFO_ACTIVE_ANON',
    'MEMINFO_ACTIVE_FILE',
    'MEMINFO_ANON_PAGES',
    'MEMINFO_BUFFERS',
    'MEMINFO_CACHED',
    'MEMINFO_CMA_FREE',
    'MEMINFO_CMA_TOTAL',
    'MEMINFO_COMMIT_LIMIT',
    'MEMINFO_COMMITED_AS',
    'MEMINFO_DIRTY',
    'MEMINFO_INACTIVE',
    'MEMINFO_INACTIVE_ANON',
    'MEMINFO_INACTIVE_FILE',
    'MEMINFO_KERNEL_STACK',
    'MEMINFO_MAPPED',
    'MEMINFO_MEM_AVAILABLE',
    'MEMINFO_MEM_FREE',
    'MEMINFO_MEM_TOTAL',
    'MEMINFO_MLOCKED',
    'MEMINFO_PAGE_TABLES',
    'MEMINFO_SHMEM',
    'MEMINFO_SLAB',
    'MEMINFO_SLAB_RECLAIMABLE',
    'MEMINFO_SLAB_UNRECLAIMABLE',
    'MEMINFO_SWAP_CACHED',
    'MEMINFO_SWAP_FREE',
    'MEMINFO_SWAP_TOTAL',
    'MEMINFO_UNEVICTABLE',
    'MEMINFO_VMALLOC_CHUNK',
    'MEMINFO_VMALLOC_TOTAL',
    'MEMINFO_VMALLOC_USED',
    'MEMINFO_WRITEBACK',
    'MEMINFO_KERNEL_RECLAIMABLE',
    'PMEM_ACTIVE_PURG',
    'PMEM_INACTIVE_PURG',
    'PMEM_PINED_PURG'
  ];
  static VMEM_INFO = [
    'VMEMINFO_UNSPECIFIED',
    'VMEMINFO_NR_FREE_PAGES',
    'VMEMINFO_NR_ALLOC_BATCH',
    'VMEMINFO_NR_INACTIVE_ANON',
    'VMEMINFO_NR_ACTIVE_ANON',
    'VMEMINFO_NR_INACTIVE_FILE',
    'VMEMINFO_NR_ACTIVE_FILE',
    'VMEMINFO_NR_UNEVICTABLE',
    'VMEMINFO_NR_MLOCK',
    'VMEMINFO_NR_ANON_PAGES',
    'VMEMINFO_NR_MAPPED',
    'VMEMINFO_NR_FILE_PAGES',
    'VMEMINFO_NR_DIRTY',
    'VMEMINFO_NR_WRITEBACK',
    'VMEMINFO_NR_SLAB_RECLAIMABLE',
    'VMEMINFO_NR_SLAB_UNRECLAIMABLE',
    'VMEMINFO_NR_PAGE_TABLE_PAGES',
    'VMEMINFO_NR_KERNEL_STACK',
    'VMEMINFO_NR_OVERHEAD',
    'VMEMINFO_NR_UNSTABLE',
    'VMEMINFO_NR_BOUNCE',
    'VMEMINFO_NR_VMSCAN_WRITE',
    'VMEMINFO_NR_VMSCAN_IMMEDIATE_RECLAIM',
    'VMEMINFO_NR_WRITEBACK_TEMP',
    'VMEMINFO_NR_ISOLATED_ANON',
    'VMEMINFO_NR_ISOLATED_FILE',
    'VMEMINFO_NR_SHMEM',
    'VMEMINFO_NR_DIRTIED',
    'VMEMINFO_NR_WRITTEN',
    'VMEMINFO_NR_PAGES_SCANNED',
    'VMEMINFO_WORKINGSET_REFAULT',
    'VMEMINFO_WORKINGSET_ACTIVATE',
    'VMEMINFO_WORKINGSET_NODERECLAIM',
    'VMEMINFO_NR_ANON_TRANSPARENT_HUGEPAGES',
    'VMEMINFO_NR_FREE_CMA',
    'VMEMINFO_NR_SWAPCACHE',
    'VMEMINFO_NR_DIRTY_THRESHOLD',
    'VMEMINFO_NR_DIRTY_BACKGROUND_THRESHOLD',
    'VMEMINFO_PGPGIN',
    'VMEMINFO_PGPGOUT',
    'VMEMINFO_PGPGOUTCLEAN',
    'VMEMINFO_PSWPIN',
    'VMEMINFO_PSWPOUT',
    'VMEMINFO_PGALLOC_DMA',
  ];
  static VMEM_INFO_SECOND = [
    'VMEMINFO_PGALLOC_NORMAL',
    'VMEMINFO_PGALLOC_MOVABLE',
    'VMEMINFO_PGFREE',
    'VMEMINFO_PGACTIVATE',
    'VMEMINFO_PGDEACTIVATE',
    'VMEMINFO_PGFAULT',
    'VMEMINFO_PGMAJFAULT',
    'VMEMINFO_PGREFILL_DMA',
    'VMEMINFO_PGREFILL_NORMAL',
    'VMEMINFO_PGREFILL_MOVABLE',
    'VMEMINFO_PGSTEAL_KSWAPD_DMA',
    'VMEMINFO_PGSTEAL_KSWAPD_NORMAL',
    'VMEMINFO_PGSTEAL_KSWAPD_MOVABLE',
    'VMEMINFO_PGSTEAL_DIRECT_DMA',
    'VMEMINFO_PGSTEAL_DIRECT_NORMAL',
    'VMEMINFO_PGSTEAL_DIRECT_MOVABLE',
    'VMEMINFO_PGSCAN_KSWAPD_DMA',
    'VMEMINFO_PGSCAN_KSWAPD_NORMAL',
    'VMEMINFO_PGSCAN_KSWAPD_MOVABLE',
    'VMEMINFO_PGSCAN_DIRECT_DMA',
    'VMEMINFO_PGSCAN_DIRECT_NORMAL',
    'VMEMINFO_PGSCAN_DIRECT_MOVABLE',
    'VMEMINFO_PGSCAN_DIRECT_THROTTLE',
    'VMEMINFO_PGINODESTEAL',
    'VMEMINFO_SLABS_SCANNED',
    'VMEMINFO_KSWAPD_INODESTEAL',
    'VMEMINFO_KSWAPD_LOW_WMARK_HIT_QUICKLY',
    'VMEMINFO_KSWAPD_HIGH_WMARK_HIT_QUICKLY',
    'VMEMINFO_PAGEOUTRUN',
    'VMEMINFO_ALLOCSTALL',
    'VMEMINFO_PGROTATED',
    'VMEMINFO_DROP_PAGECACHE',
    'VMEMINFO_DROP_SLAB',
    'VMEMINFO_PGMIGRATE_SUCCESS',
    'VMEMINFO_PGMIGRATE_FAIL',
    'VMEMINFO_COMPACT_MIGRATE_SCANNED',
    'VMEMINFO_COMPACT_FREE_SCANNED',
    'VMEMINFO_COMPACT_ISOLATED',
    'VMEMINFO_COMPACT_STALL',
    'VMEMINFO_COMPACT_FAIL',
    'VMEMINFO_COMPACT_SUCCESS',
    'VMEMINFO_COMPACT_DAEMON_WAKE',
    'VMEMINFO_UNEVICTABLE_PGS_CULLED',
    'VMEMINFO_UNEVICTABLE_PGS_SCANNED',
    'VMEMINFO_UNEVICTABLE_PGS_RESCUED',
    'VMEMINFO_UNEVICTABLE_PGS_MLOCKED',
    'VMEMINFO_UNEVICTABLE_PGS_MUNLOCKED',
  ];
  static VMEM_INFO_THIRD = [
    'VMEMINFO_UNEVICTABLE_PGS_CLEARED',
    'VMEMINFO_UNEVICTABLE_PGS_STRANDED',
    'VMEMINFO_NR_ZSPAGES',
    'VMEMINFO_NR_ION_HEAP',
    'VMEMINFO_NR_GPU_HEAP',
    'VMEMINFO_ALLOCSTALL_DMA',
    'VMEMINFO_ALLOCSTALL_MOVABLE',
    'VMEMINFO_ALLOCSTALL_NORMAL',
    'VMEMINFO_COMPACT_DAEMON_FREE_SCANNED',
    'VMEMINFO_COMPACT_DAEMON_MIGRATE_SCANNED',
    'VMEMINFO_NR_FASTRPC',
    'VMEMINFO_NR_INDIRECTLY_RECLAIMABLE',
    'VMEMINFO_NR_ION_HEAP_POOL',
    'VMEMINFO_NR_KERNEL_MISC_RECLAIMABLE',
    'VMEMINFO_NR_SHADOW_CALL_STACK_BYTES',
    'VMEMINFO_NR_SHMEM_HUGEPAGES',
    'VMEMINFO_NR_SHMEM_PMDMAPPED',
    'VMEMINFO_NR_UNRECLAIMABLE_PAGES',
    'VMEMINFO_NR_ZONE_ACTIVE_ANON',
    'VMEMINFO_NR_ZONE_ACTIVE_FILE',
    'VMEMINFO_NR_ZONE_INACTIVE_ANON',
    'VMEMINFO_NR_ZONE_INACTIVE_FILE',
    'VMEMINFO_NR_ZONE_UNEVICTABLE',
    'VMEMINFO_NR_ZONE_WRITE_PENDING',
    'VMEMINFO_OOM_KILL',
    'VMEMINFO_PGLAZYFREE',
    'VMEMINFO_PGLAZYFREED',
    'VMEMINFO_PGREFILL',
    'VMEMINFO_PGSCAN_DIRECT',
    'VMEMINFO_PGSCAN_KSWAPD',
    'VMEMINFO_PGSKIP_DMA',
    'VMEMINFO_PGSKIP_MOVABLE',
    'VMEMINFO_PGSKIP_NORMAL',
    'VMEMINFO_PGSTEAL_DIRECT',
    'VMEMINFO_PGSTEAL_KSWAPD',
    'VMEMINFO_SWAP_RA',
    'VMEMINFO_SWAP_RA_HIT',
    'VMEMINFO_WORKINGSET_RESTORE',
  ];
  // sys.mem.total   sys.mem.free sys.mem.buffers sys.mem.cached  sys.mem.shmem  sys.mem.slab  sys.mem.swap.total
  // sys.mem.swap.free sys.mem.mapped  sys.mem.vmalloc.used  sys.mem.page.tables  sys.mem.kernel.stack
  // sys.mem.active sys.mem.inactive  sys.mem.unevictable  sys.mem.vmalloc.total sys.mem.slab.unreclaimable
  // sys.mem.cma.total sys.mem.cma.free
  static ABALITY_MEM_INFO = [
    'MEMINFO_MEM_TOTAL',
    'MEMINFO_MEM_FREE',
    'MEMINFO_BUFFERS',
    'MEMINFO_CACHED',
    'MEMINFO_SHMEM',
    'MEMINFO_SLAB',
    'MEMINFO_SWAP_TOTAL',
    'MEMINFO_SWAP_FREE',
    'MEMINFO_MAPPED',
    'MEMINFO_VMALLOC_USED',
    'MEMINFO_PAGE_TABLES',
    'MEMINFO_KERNEL_STACK',
    'MEMINFO_ACTIVE',
    'MEMINFO_INACTIVE',
    'MEMINFO_UNEVICTABLE',
    'MEMINFO_VMALLOC_TOTAL',
    'MEMINFO_SLAB_UNRECLAIMABLE',
    'MEMINFO_CMA_TOTAL',
    'MEMINFO_CMA_FREE',
    'MEMINFO_KERNEL_RECLAIMABLE',
    'PMEM_ACTIVE_PURG',
    'PMEM_INACTIVE_PURG',
    'PMEM_PINED_PURG'
  ];

  schedulingEvents = [
    'sched/sched_switch',
    'power/suspend_resume',
    'sched/sched_wakeup',
    'sched/sched_wakeup_new',
    'sched/sched_waking',
    'sched/sched_process_exit',
    'sched/sched_process_free',
    'task/task_newtask',
    'task/task_rename',
  ];
  powerEvents = [
    'regulator/regulator_set_voltage',
    'regulator/regulator_set_voltage_complete',
    'power/clock_enable',
    'power/clock_disable',
    'power/clock_set_rate',
    'power/suspend_resume',
  ];
  cpuFreqEvents = ['power/cpu_frequency', 'power/cpu_idle', 'power/suspend_resume'];
  sysCallsEvents = ['raw_syscalls/sys_enter', 'raw_syscalls/sys_exit'];
  highFrequencyEvents = [
    'mm_event/mm_event_record',
    'kmem/rss_stat',
    'ion/ion_stat',
    'dmabuf_heap/dma_heap_stat',
    'kmem/ion_heap_grow',
    'kmem/ion_heap_shrink',
  ];
  advancedConfigEvents = [
    'sched/sched_switch',
    'sched/sched_wakeup',
    'sched/sched_wakeup_new',
    'sched/sched_waking',
    'sched/sched_process_exit',
    'sched/sched_process_free',
    'irq/irq_handler_entry',
    'irq/irq_handler_exit',
    'irq/softirq_entry',
    'irq/softirq_exit',
    'irq/softirq_raise',
    'power/clock_disable',
    'power/clock_enable',
    'power/clock_set_rate',
    'power/cpu_frequency',
    'power/cpu_idle',
    'clk/clk_disable',
    'clk/clk_disable_complete',
    'clk/clk_enable',
    'clk/clk_enable_complete',
    'clk/clk_set_rate',
    'clk/clk_set_rate_complete',
    'binder/binder_transaction',
    'binder/binder_transaction_alloc_buf',
    'binder/binder_transaction_received',
    'binder/binder_lock',
    'binder/binder_locked',
    'binder/binder_unlock',
    'workqueue/workqueue_execute_start',
    'workqueue/workqueue_execute_end',
    'oom/oom_score_adj_update',
    'ftrace/print',
  ];
  private _menuItems: Array<MenuItem> | undefined;

  public deviceSelect: HTMLSelectElement | undefined;
  public deviceVersion: HTMLSelectElement | undefined;
  private recordButtonText: HTMLSpanElement | undefined;
  private devicePrompt: HTMLSpanElement | undefined;
  private recordButton: LitButton | undefined;
  private cancelButton: LitButton | undefined;
  private sp: SpApplication | undefined;
  private progressEL: LitProgressBar | undefined;
  private litSearch: LitSearch | undefined;
  private addButton: LitButton | undefined | null;
  private disconnectButton: LitButton | undefined | null;

  private recordSetting: SpRecordSetting | undefined;
  private probesConfig: SpProbesConfig | undefined;
  private traceCommand: SpTraceCommand | undefined;
  private spAllocations: SpAllocations | undefined;
  private spRecordPerf: SpRecordPerf | undefined;
  private spFileSystem: SpFileSystem | undefined;
  private spSdkConfig: SpSdkConfig | undefined;
  private spVmTracker: SpVmTracker | undefined;
  private spHisysEvent: SpHisysEvent | undefined;
  private spRecordTemplate: SpRecordTemplate | undefined;
  private spArkTs: SpArkTs | undefined;
  private spHilog: SpHilogRecord | undefined;

  private ftraceSlider: LitSlider | undefined | null;

  private spWebShell: SpWebHdcShell | undefined;
  private menuGroup: LitMainMenuGroup | undefined | null;
  private appContent: HTMLElement | undefined | null;
  private record = 'Record';
  private stop = 'StopRecord';

  compareArray(devs: Array<string>): boolean {
    let clearFlag: boolean = false;
    if (devs.length != this.deviceSelect!.options.length) {
      clearFlag = true;
    } else {
      let optionArray = new Array();
      for (let i = 0; i < this.deviceSelect!.options.length; i++) {
        optionArray.push(this.deviceSelect!.options[i].value);
      }
      devs.forEach((value) => {
        if (optionArray.indexOf(value) == -1) {
          clearFlag = true;
        }
      });
    }
    return clearFlag;
  }

  refreshDeviceList(): void {
    if (this.vs) {
      Cmd.execHdcCmd(CmdConstant.CMD_HDC_DEVICES, (res: string) => {
        let devs: string[] = res.trim().replace(/\r\n/g, '\r').replace(/\n/g, '\r').split(/\r/);
        if (devs.length == 1 && devs[0].indexOf('Empty') != -1) {
          this.deviceSelect!.innerHTML = '';
          return;
        }
        let clearFlag = this.compareArray(devs);
        if (clearFlag) {
          this.deviceSelect!.innerHTML = '';
          if (devs.length == 0) {
            this.recordButton!.hidden = true;
            this.disconnectButton!.hidden = true;
            this.devicePrompt!.innerText = 'Device not connected';
          }
          for (let i = 0; i < devs.length; i++) {
            let dev = devs[i];
            let option = document.createElement('option');
            option.className = 'select';
            option.textContent = dev;
            this.deviceSelect!.appendChild(option);
            if (i == 0) {
              option.selected = true;
              this.recordButton!.hidden = false;
              this.disconnectButton!.hidden = false;
              SpRecordTrace.serialNumber = option.value;
              this.devicePrompt!.innerText = '';
            }
          }
        }
      });
    } else {
      this.deviceSelect!.innerHTML = '';
      // @ts-ignore
      HdcDeviceManager.getDevices().then((devs: USBDevice[]) => {
        if (devs.length == 0) {
          this.recordButton!.hidden = true;
          this.disconnectButton!.hidden = true;
          this.devicePrompt!.innerText = 'Device not connected';
        }
        for (let len = 0; len < devs.length; len++) {
          let dev = devs[len];
          let option = document.createElement('option');
          option.className = 'select';
          if (typeof dev.serialNumber === 'string') {
            option.value = dev.serialNumber;
          }
          option.textContent = dev!.serialNumber ? dev!.serialNumber!.toString() : 'hdc Device';
          this.deviceSelect!.appendChild(option);
          if (len == 0) {
            option.selected = true;
            this.recordButton!.hidden = false;
            this.disconnectButton!.hidden = false;
            this.devicePrompt!.innerText = '';
            SpRecordTrace.serialNumber = option.value;
            HdcDeviceManager.connect(option.value).then((result) => {
              if (result) {
                HdcDeviceManager.shellResultAsString(CmdConstant.CMD_GET_VERSION, false).then((version) => {
                  SpRecordTrace.selectVersion = this.getDeviceVersion(version);
                  this.setDeviceVersionSelect(SpRecordTrace.selectVersion);
                  this.nativeMemoryHideBySelectVersion();
                  this.traceCommand!.hdcCommon = PluginConvertUtils.createHdcCmd(
                    PluginConvertUtils.BeanToCmdTxt(this.makeRequest(), false),
                    this.recordSetting!.output,
                    this.recordSetting!.maxDur
                  );
                  if (this.nowChildItem === this.spWebShell) {
                    window.publish(window.SmartEvent.UI.DeviceConnect, option.value);
                  }
                });
              } else {
                SpRecordTrace.selectVersion = SpRecordTrace.supportVersions[0];
                this.setDeviceVersionSelect(SpRecordTrace.selectVersion);
                this.nativeMemoryHideBySelectVersion();
                let cmdTxt = PluginConvertUtils.BeanToCmdTxt(this.makeRequest(), false);
                this.traceCommand!.hdcCommon = PluginConvertUtils.createHdcCmd(
                  cmdTxt,
                  this.recordSetting!.output,
                  this.recordSetting!.maxDur
                );
              }
            });
          }
        }
      });
    }
  }

  getDeviceVersion(version: string): string {
    if (version.indexOf('3.2') != -1) {
      return '3.2';
    } else if (version.indexOf('4.') != -1) {
      return '4.0+';
    }
    return '3.2';
  }
  freshMenuDisable(disable: boolean): void {
    let mainMenu = this.sp!.shadowRoot?.querySelector('#main-menu') as LitMainMenu;
    mainMenu.menus?.forEach((men) => {
      men.children.forEach((child: any) => {
        // @ts-ignore
        child.disabled = disable;
      });
    });
    mainMenu.menus = mainMenu.menus;
  }

  refreshConfig(isTraceConfig: boolean) {
    let recordSettingEl = this.shadowRoot?.querySelector('record-setting') as SpRecordSetting;
    if (recordSettingEl) {
      if (isTraceConfig) {
        recordSettingEl.setAttribute('trace_config', '');
      } else {
        if (recordSettingEl.hasAttribute('trace_config')) {
          recordSettingEl.removeAttribute('trace_config');
        }
      }
    }
  }

  refreshHint() {
    let flags = FlagsConfig.getAllFlagConfig();
    let showHint = false;
    for (let i = 0; i < flags.length; i++) {
      let flag = flags[i];
      if (this.selectedTemplate.has(flag.title)) {
        let selectedOption = flag.switchOptions.filter((option) => {
          return option.selected;
        });
        if (selectedOption[0].option === 'Disabled') {
          showHint = true;
          break;
        }
      }
    }
    this.showHint = showHint;
  }

  get showHint(): boolean {
    return this.hasAttribute('show_hint');
  }

  private hintTimeOut: number = -1;

  set showHint(bool: boolean) {
    if (bool) {
      if (this.hasAttribute('show_hint')) {
        this.removeAttribute('show_hint');
        this.hintTimeOut = window.setTimeout(() => {
          this.setAttribute('show_hint', '');
        }, 200);
      } else {
        this.setAttribute('show_hint', '');
      }
    } else {
      if (this.hintTimeOut !== -1) {
        window.clearTimeout(this.hintTimeOut);
        this.hintTimeOut = -1;
      }
      this.removeAttribute('show_hint');
    }
  }

  private refreshDeviceTimer: number | undefined;
  private hintEl: HTMLSpanElement | undefined;
  private selectedTemplate: Map<string, number> = new Map();

  private nativeMemoryHideBySelectVersion() {
    let divConfigs = this.spAllocations?.shadowRoot?.querySelectorAll<HTMLDivElement>('.version-controller');
    if (divConfigs) {
      if (SpRecordTrace.selectVersion != '3.2') {
        for (let divConfig of divConfigs) {
          divConfig!.style.zIndex = '1';
        }
      } else {
        for (let divConfig of divConfigs) {
          divConfig!.style.zIndex = '-1';
        }
      }
    }
  }

  private selectedDevice(deviceVersion: string): void {
    SpRecordTrace.selectVersion = this.getDeviceVersion(deviceVersion);
    this.setDeviceVersionSelect(SpRecordTrace.selectVersion);
  }

  private appendDeviceVersion(): void {
    SpRecordTrace.supportVersions.forEach((supportVersion) => {
      let option = document.createElement('option');
      option.className = 'select';
      option.textContent = `OpenHarmony-${supportVersion}`;
      option.setAttribute('device-version', supportVersion);
      this.deviceVersion!.append(option);
    });
  }

  private setDeviceVersionSelect(selected: string): void {
    let children = this.deviceVersion!.children;
    for (let i = 0; i < children.length; i++) {
      let child = children[i] as HTMLOptionElement;
      if (child.getAttribute('device-version') === selected) {
        child.selected = true;
        break;
      }
    }
  }

  stopRecordListener(): void {
    this.recordButtonText!.textContent = this.record;
    this.recordButtonDisable(true);
    this.cancelButtonShow(false);
    if (this.vs) {
      let cmd = Cmd.formatString(CmdConstant.CMS_HDC_STOP, [SpRecordTrace.serialNumber]);
      Cmd.execHdcCmd(cmd, (res: string) => {});
    } else {
      let selectedOption = this.deviceSelect!.options[this.deviceSelect!.selectedIndex] as HTMLOptionElement;
      HdcDeviceManager.connect(selectedOption.value).then((result) => {
        if (result) {
          try {
            HdcDeviceManager.shellResultAsString(CmdConstant.CMS_STOP, true).then((result) => {});
          } catch (exception) {
            this.recordButtonDisable(false);
            log(exception);
          }
        }
      });
    }
  }

  cancelRecordListener(): void {
    this.recordButtonText!.textContent = this.record;
    this.cancelButtonShow(false);
    if (this.vs) {
      let cmd = Cmd.formatString(CmdConstant.CMS_HDC_CANCEL, [SpRecordTrace.serialNumber]);
      Cmd.execHdcCmd(cmd, (res: string) => {
        this.freshMenuDisable(false);
        this.freshConfigMenuDisable(false);
        this.progressEL!.loading = false;
        this.sp!.search = false;
        this.litSearch!.clear();
        this.addButton!.style.pointerEvents = 'auto';
        this.deviceSelect!.style.pointerEvents = 'auto';
        this.disconnectButton!.style.pointerEvents = 'auto';
        this.deviceVersion!.style.pointerEvents = 'auto';
      });
    } else {
      let selectedOption = this.deviceSelect!.options[this.deviceSelect!.selectedIndex] as HTMLOptionElement;
      HdcDeviceManager.connect(selectedOption.value).then((result) => {
        if (result) {
          this.freshMenuDisable(false);
          this.freshConfigMenuDisable(false);
          try {
            this.progressEL!.loading = false;
            this.sp!.search = false;
            this.litSearch!.clear();
            this.disconnectButton!.style.pointerEvents = 'auto';
            this.addButton!.style.pointerEvents = 'auto';
            this.deviceSelect!.style.pointerEvents = 'auto';
            this.deviceVersion!.style.pointerEvents = 'auto';
            SpRecordTrace.cancelRecord = true;
            HdcDeviceManager.stopHiprofiler(CmdConstant.CMS_CANCEL).then((result) => {});
          } catch (exception) {
            log(exception);
          }
        }
      });
    }
  }

  private cancelButtonShow(show: boolean) {
    if (show) {
      this.cancelButton!.style.visibility = 'visible';
    } else {
      this.cancelButton!.style.visibility = 'hidden';
    }
  }

  private initMenuItems(): void {
    let that = this;
    if (this.record_template) {
      this._menuItems = [
        {
          title: 'Record setting',
          icon: 'properties',
          fileChoose: false,
          clickHandler: function (event: InputEvent): void {
            that.appContent!.innerHTML = '';
            that.appContent!.append(that.recordSetting!);
            that.freshMenuItemsStatus('Record setting');
          },
        },
        {
          title: 'Trace template',
          icon: 'realIntentionBulb',
          clickHandler: function (ev: InputEvent): void {
            that.appContent!.innerHTML = '';
            that.appContent!.append(that.spRecordTemplate!);
            that.freshMenuItemsStatus('Trace template');
          },
        },
        {
          title: 'Trace command',
          icon: 'dbsetbreakpoint',
          fileChoose: false,
          clickHandler: function (ev: InputEvent): void {
            that.appContent!.innerHTML = '';
            that.appContent!.append(that.traceCommand!);
            that.traceCommand!.hdcCommon = PluginConvertUtils.createHdcCmd(
              PluginConvertUtils.BeanToCmdTxt(that.makeRequest(), false),
              that.recordSetting!.output,
              that.recordSetting!.maxDur
            );
            that.freshMenuItemsStatus('Trace command');
          },
        },
      ];
    } else {
      this._menuItems = [
        {
          title: 'Record setting',
          icon: 'properties',
          fileChoose: false,
          clickHandler: function (ev: InputEvent): void {
            that.appContent!.innerHTML = '';
            that.appContent!.append(that.recordSetting!);
            that.freshMenuItemsStatus('Record setting');
          },
        },
        {
          title: 'Trace command',
          icon: 'dbsetbreakpoint',
          fileChoose: false,
          clickHandler: function (ev: InputEvent): void {
            that.freshMenuItemsStatus('Trace command');
            let request = that.makeRequest();
            that.appContent!.innerHTML = '';
            that.appContent!.append(that.traceCommand!);
            that.traceCommand!.hdcCommon = PluginConvertUtils.createHdcCmd(
              PluginConvertUtils.BeanToCmdTxt(request, false),
              that.recordSetting!.output,
              that.recordSetting!.maxDur
            );
          },
        },
        {
          title: 'Hdc Shell',
          icon: 'file-config',
          fileChoose: false,
          clickHandler: function (ev: InputEvent) {
            that.appContent!.innerHTML = '';
            that.appContent!.append(that.spWebShell!);
            that.spWebShell!.shellDiv!.scrollTop = that.spWebShell!.currentScreenRemain;
            setTimeout(() => {
              that.spWebShell!.hdcShellFocus();
            }, 100);
            that.nowChildItem = that.spWebShell!;
            that.freshMenuItemsStatus('Hdc Shell');
          },
        },
        {
          title: 'Probes config',
          icon: 'realIntentionBulb',
          fileChoose: false,
          clickHandler: function (ev: InputEvent): void {
            that.appContent!.innerHTML = '';
            that.appContent!.append(that.probesConfig!);
            that.freshMenuItemsStatus('Probes config');
          },
        },
        {
          title: 'Native Memory',
          icon: 'externaltools',
          fileChoose: false,
          clickHandler: function (ev: InputEvent): void {
            let startNativeSwitch = that.spAllocations?.shadowRoot?.getElementById('switch-disabled') as LitSwitch;
            let recordModeSwitch = that.probesConfig?.shadowRoot?.querySelector('lit-switch') as LitSwitch;
            let checkDesBoxDis = that.probesConfig?.shadowRoot?.querySelectorAll('check-des-box');
            let litCheckBoxDis = that.probesConfig?.shadowRoot?.querySelectorAll('lit-check-box');

            that.ftraceSlider = that.probesConfig?.shadowRoot?.querySelector<LitSlider>('#ftrace-buff-size-slider');
            startNativeSwitch.addEventListener('change', (event: any) => {
              let detail = event.detail;
              if (detail!.checked) {
                recordModeSwitch.removeAttribute('checked');

                checkDesBoxDis?.forEach((item: any) => {
                  item.setAttribute('disabled', '');
                  item.checked = false;
                });

                litCheckBoxDis?.forEach((item: any) => {
                  item.setAttribute('disabled', '');
                  item.checked = false;
                });

                that.ftraceSlider!.setAttribute('disabled', '');
              }
            });

            let divConfigs = that.spAllocations?.shadowRoot?.querySelectorAll<HTMLDivElement>('.version-controller');
            if ((!SpRecordTrace.selectVersion || SpRecordTrace.selectVersion === '3.2') && divConfigs) {
              for (let divConfig of divConfigs) {
                divConfig!.style.zIndex = '-1';
              }
            }
            that.appContent!.innerHTML = '';
            that.appContent!.append(that.spAllocations!);
            that.freshMenuItemsStatus('Native Memory');
          },
        },
        {
          title: 'Hiperf',
          icon: 'realIntentionBulb',
          fileChoose: false,
          clickHandler: function (ev: InputEvent): void {
            that.appContent!.innerHTML = '';
            that.appContent!.append(that.spRecordPerf!);
            that.freshMenuItemsStatus('Hiperf');
          },
        },
        {
          title: 'eBPF Config',
          icon: 'file-config',
          fileChoose: false,
          clickHandler: function (ev: InputEvent): void {
            that.appContent!.innerHTML = '';
            that.appContent!.append(that.spFileSystem!);
            that.spFileSystem!.setAttribute('long_trace', '');
            that.freshMenuItemsStatus('eBPF Config');
          },
        },
        {
          title: 'VM Tracker',
          icon: 'vm-tracker',
          fileChoose: false,
          clickHandler: function (ev: InputEvent): void {
            that.appContent!.innerHTML = '';
            that.appContent!.append(that.spVmTracker!);
            that.freshMenuItemsStatus('VM Tracker');
          },
        },
        {
          title: 'HiSystemEvent',
          icon: 'externaltools',
          fileChoose: false,
          clickHandler: function (ev: InputEvent): void {
            that.appContent!.innerHTML = '';
            that.appContent!.append(that.spHisysEvent!);
            that.freshMenuItemsStatus('HiSystemEvent');
          },
        },
        {
          title: 'Ark Ts',
          icon: 'file-config',
          fileChoose: false,
          clickHandler: function (ev: InputEvent): void {
            that.appContent!.innerHTML = '';
            that.appContent!.append(that.spArkTs!);
            that.freshMenuItemsStatus('Ark Ts');
          },
        },
        {
          title: 'Hilog',
          icon: 'realIntentionBulb',
          fileChoose: false,
          clickHandler: function (ev: InputEvent): void {
            that.appContent!.innerHTML = '';
            that.appContent!.append(that.spHilog!);
            that.freshMenuItemsStatus('Hilog');
          },
        },
        {
          title: 'SDK Config',
          icon: 'file-config',
          fileChoose: false,
          clickHandler: function (ev: InputEvent): void {
            that.appContent!.innerHTML = '';
            that.appContent!.append(that.spSdkConfig!);
            that.freshMenuItemsStatus('SDK Config');
          },
        },
      ];
    }

    this._menuItems?.forEach((item) => {
      let th = new LitMainMenuItem();
      th.setAttribute('icon', item.icon || '');
      th.setAttribute('title', item.title || '');
      th.style.height = '60px';
      th.style.fontFamily = 'Helvetica-Bold';
      th.style.fontSize = '16px';
      th.style.lineHeight = '28px';
      th.style.fontWeight = '700';
      th.removeAttribute('file');
      th.addEventListener('click', (e) => {
        if (item.clickHandler) {
          item.clickHandler(item);
        }
      });
      this.menuGroup!.appendChild(th);
    });
  }

  // @ts-ignore
  usbConnectionListener(event: USBConnectionEvent): void {
    if (event.isTrusted) {
      this.recordButton!.hidden = false;
      this.disconnectButton!.hidden = false;
      // @ts-ignore
      let usbDevice: USBDevice = event.device;
      let option = document.createElement('option');
      option.className = 'select';
      if (typeof usbDevice.serialNumber === 'string') {
        option.value = usbDevice.serialNumber;
      }
      option.selected = true;
      option.textContent = usbDevice!.serialNumber ? usbDevice!.serialNumber.replace(/"/g, '') : 'hdc Device';
      this.deviceSelect!.appendChild(option);
      SpRecordTrace.serialNumber = option.value;
      if (this.nowChildItem === this.spWebShell) {
        window.publish(window.SmartEvent.UI.DeviceConnect, option.value);
      }
    }
  }

  // @ts-ignore
  usbDisConnectionListener(event: USBConnectionEvent): void {
    // @ts-ignore
    let disConnectDevice: USBDevice = event.device;
    for (let index = 0; index < this.deviceSelect!.children.length; index++) {
      let option = this.deviceSelect!.children[index] as HTMLOptionElement;
      if (option.value == disConnectDevice.serialNumber) {
        let optValue = option.value;
        HdcDeviceManager.disConnect(optValue).then(() => {});
        this.deviceSelect!.removeChild(option);
        if (SpRecordTrace.serialNumber == optValue) {
          if (this.nowChildItem === this.spWebShell) {
            window.publish(window.SmartEvent.UI.DeviceDisConnect, optValue);
          }
          let options = this.deviceSelect!.options;
          if (options.length > 0) {
            let selectedOpt = options[this.deviceSelect!.selectedIndex];
            SpRecordTrace.serialNumber = selectedOpt.value;
          } else {
            this.recordButton!.hidden = true;
            this.disconnectButton!.hidden = true;
            this.devicePrompt!.innerText = 'Device not connected';
            SpRecordTrace.serialNumber = '';
          }
        }
      }
    }
  }

  recordButtonListener(): void {
    SpRecordTrace.cancelRecord = false;
    let request = this.makeRequest();
    if (request.pluginConfigs.length == 0) {
      this.hintEl!.textContent = "It looks like you didn't add any probes. Please add at least one";
      this.showHint = true;
      return;
    } else {
      this.showHint = false;
    }
    let traceCommandStr = PluginConvertUtils.createHdcCmd(
      PluginConvertUtils.BeanToCmdTxt(request, false),
      this.recordSetting!.output,
      this.recordSetting!.maxDur
    );
    let pluginList: Array<string> = [];
    request.pluginConfigs.forEach((pluginConfig) => {
      pluginList.push(pluginConfig.pluginName);
    });
    SpStatisticsHttpUtil.addOrdinaryVisitAction({
      action: 'config_page',
      event: 'online_record',
      eventData: {
        plugin: pluginList,
      },
    });
    let selectedOption = this.deviceSelect!.options[this.deviceSelect!.selectedIndex] as HTMLOptionElement;
    if (selectedOption) {
      SpRecordTrace.serialNumber = selectedOption.value;
    } else {
      this.sp!.search = true;
      this.litSearch!.clear();
      this.progressEL!.loading = false;
      this.litSearch!.setPercent('please connect device', -2);
    }

    if (this.vs) {
      this.appContent!.innerHTML = '';
      this.appContent!.append(this.traceCommand!);
      this.traceCommand!.hdcCommon = PluginConvertUtils.createHdcCmd(
        PluginConvertUtils.BeanToCmdTxt(this.makeRequest(), false),
        this.recordSetting!.output,
        this.recordSetting!.maxDur
      );
      this.freshMenuItemsStatus('Trace command');
      Cmd.execHdcCmd(Cmd.formatString(CmdConstant.CMS_HDC_STOP, [SpRecordTrace.serialNumber]), (stopRes: string) => {
        let cmd = Cmd.formatString(CmdConstant.CMD_MOUNT_DEVICES, [SpRecordTrace.serialNumber]);
        Cmd.execHdcCmd(cmd, (res: string) => {
          this.sp!.search = true;
          this.progressEL!.loading = true;
          this.litSearch!.clear();
          this.litSearch!.setPercent(`tracing  ${this.recordSetting!.maxDur * 1000}ms`, -1);
          this.initRecordUIState();
          this.recordButtonText!.textContent = this.stop;
          this.cancelButtonShow(true);
          Cmd.execHdcTraceCmd(traceCommandStr, SpRecordTrace.serialNumber, (traceResult: string) => {
            if (traceResult.indexOf('DestroySession done') != -1) {
              this.litSearch!.setPercent('tracing htrace down', -1);
              let cmd = Cmd.formatString(CmdConstant.CMD_FIEL_RECV_DEVICES, [
                SpRecordTrace.serialNumber,
                this.recordSetting!.output,
              ]);
              Cmd.execFileRecv(cmd, this.recordSetting!.output, (rt: ArrayBuffer) => {
                this.litSearch!.setPercent('downloading Hitrace file ', 101);
                let fileName = this.recordSetting!.output.substring(this.recordSetting!.output.lastIndexOf('/') + 1);
                let file = new File([rt], fileName);
                let main = this!.parentNode!.parentNode!.querySelector('lit-main-menu') as LitMainMenu;
                let children = main.menus as Array<MenuGroup>;
                let child = children[0].children as Array<MenuItem>;
                let fileHandler = child[0].fileHandler;
                if (fileHandler && !SpRecordTrace.cancelRecord) {
                  this.recordButtonText!.textContent = this.record;
                  this.cancelButtonShow(false);
                  this.freshMenuDisable(false);
                  this.freshConfigMenuDisable(false);
                  fileHandler({ detail: file });
                } else {
                  SpRecordTrace.cancelRecord = false;
                }
              });
            } else {
              this.litSearch!.setPercent('tracing htrace failed, please check your config ', -2);
              this.recordButtonText!.textContent = this.record;
              this.freshMenuDisable(false);
              this.freshConfigMenuDisable(false);
              this.progressEL!.loading = false;
            }
            this.buttonDisable(false);
          });
        });
      });
    } else {
      HdcDeviceManager.connect(selectedOption.value).then((result) => {
        log(`result is ${result}`);
        if (result) {
          this.appContent!.innerHTML = '';
          this.appContent!.append(this.traceCommand!);
          let config = this.makeRequest();
          this.traceCommand!.hdcCommon = PluginConvertUtils.createHdcCmd(
            PluginConvertUtils.BeanToCmdTxt(config, false),
            this.recordSetting!.output,
            this.recordSetting!.maxDur
          );
          this.freshMenuItemsStatus('Trace command');
          try {
            HdcDeviceManager.stopHiprofiler(CmdConstant.CMS_CANCEL).then(() => {
              HdcDeviceManager.shellResultAsString(CmdConstant.CMD_MOUNT, true).then(() => {
                this.sp!.search = true;
                this.progressEL!.loading = true;
                this.litSearch!.clear();
                this.litSearch!.setPercent(`tracing  ${this.recordSetting!.maxDur * 1000}ms`, -1);
                this.buttonDisable(true);
                this.freshMenuDisable(true);
                this.freshConfigMenuDisable(true);
                if (SpApplication.isLongTrace) {
                  HdcDeviceManager.shellResultAsString(
                    CmdConstant.CMD_CLEAR_LONG_FOLD + this.recordSetting!.longOutPath,
                    false
                  ).then(() => {
                    HdcDeviceManager.shellResultAsString(
                      CmdConstant.CMD_MKDIR_LONG_FOLD + this.recordSetting!.longOutPath,
                      false
                    ).then(() => {
                      this.recordLongTraceCmd(traceCommandStr);
                    });
                  });
                } else {
                  this.recordTraceCmd(traceCommandStr);
                }
              });
            });
          } catch (e) {
            this.freshMenuDisable(false);
            this.freshConfigMenuDisable(false);
            this.buttonDisable(false);
          }
        } else {
          this.sp!.search = true;
          this.litSearch!.clear();
          this.litSearch!.setPercent('please kill other hdc-server !', -2);
        }
      });
    }
  }

  private recordTraceCmd(traceCommandStr: string): void {
    HdcDeviceManager.shellResultAsString(CmdConstant.CMD_SHELL + traceCommandStr, false).then((traceResult) => {
      let re = this.isSuccess(traceResult);
      if (re == 0) {
        this.litSearch!.setPercent('tracing htrace down', -1);
        HdcDeviceManager.shellResultAsString(CmdConstant.CMD_TRACE_FILE_SIZE + this.recordSetting!.output, false).then(
          (traceFileSize) => {
            this.litSearch!.setPercent(`traceFileSize is ${traceFileSize}`, -1);
            if (traceFileSize.indexOf('No such') != -1) {
              this.litSearch!.setPercent('No such file or directory', -2);
              this.buttonDisable(false);
              this.freshConfigMenuDisable(false);
              this.freshMenuDisable(false);
            } else if (Number(traceFileSize) <= SpRecordTrace.MaxFileSize) {
              HdcDeviceManager.fileRecv(this.recordSetting!.output, (perNumber: number) => {
                this.litSearch!.setPercent('downloading Hitrace file ', perNumber);
              }).then((pullRes) => {
                this.litSearch!.setPercent('downloading Hitrace file ', 101);
                pullRes.arrayBuffer().then((buffer) => {
                  let fileName = this.recordSetting!.output.substring(this.recordSetting!.output.lastIndexOf('/') + 1);
                  let file = new File([buffer], fileName);
                  let main = this!.parentNode!.parentNode!.querySelector('lit-main-menu') as LitMainMenu;
                  let children = main.menus as Array<MenuGroup>;
                  let child = children[0].children as Array<MenuItem>;
                  let fileHandler = child[0].fileHandler;
                  if (fileHandler && !SpRecordTrace.cancelRecord) {
                    this.freshConfigMenuDisable(false);
                    this.freshMenuDisable(false);
                    this.buttonDisable(false);
                    this.recordButtonDisable(false);
                    fileHandler({
                      detail: file,
                    });
                  } else {
                    SpRecordTrace.cancelRecord = false;
                  }
                });
              });
            } else {
              this.recordButtonText!.textContent = this.record;
              this.recordButtonDisable(false);
              this.litSearch!.setPercent('htrace file is too big', -2);
              this.buttonDisable(false);
              this.freshConfigMenuDisable(false);
              this.freshMenuDisable(false);
            }
          }
        );
      } else if (re == 2) {
        this.recordButtonDisable(false);
        this.litSearch!.setPercent('stop tracing htrace ', -1);
        this.freshConfigMenuDisable(false);
        this.freshMenuDisable(false);
        this.buttonDisable(false);
      } else if (re == -1) {
        this.recordButtonDisable(false);
        this.litSearch!.setPercent('The device is abnormal', -2);
        this.progressEL!.loading = false;
        this.freshConfigMenuDisable(false);
        this.freshMenuDisable(false);
        this.buttonDisable(false);
      } else {
        this.recordButtonDisable(false);
        this.litSearch!.setPercent('tracing htrace failed, please check your config ', -2);
        this.freshConfigMenuDisable(false);
        this.freshMenuDisable(false);
        this.buttonDisable(false);
      }
    });
  }

  private recordLongTraceCmd(traceCommandStr: string): void {
    HdcDeviceManager.shellResultAsString(CmdConstant.CMD_SHELL + traceCommandStr, false).then((traceResult) => {
      let re = this.isSuccess(traceResult);
      if (re === 0) {
        this.litSearch!.setPercent('tracing htrace down', -1);
        HdcDeviceManager.shellResultAsString(
          CmdConstant.CMD_TRACE_FILE_SIZE + this.recordSetting!.longOutPath,
          false
        ).then((traceFileSize) => {
          this.litSearch!.setPercent(`traceFileSize is ${traceFileSize}`, -1);
          if (traceFileSize.indexOf('No such') != -1) {
            this.litSearch!.setPercent('No such file or directory', -2);
            this.buttonDisable(false);
            this.freshConfigMenuDisable(false);
            this.freshMenuDisable(false);
          } else {
            this.recordLongTrace();
          }
        });
      } else if (re === 2) {
        this.recordButtonDisable(false);
        this.litSearch!.setPercent('stop tracing htrace ', -1);
        this.freshConfigMenuDisable(false);
        this.freshMenuDisable(false);
        this.buttonDisable(false);
      } else if (re === -1) {
        this.recordButtonDisable(false);
        this.litSearch!.setPercent('The device is abnormal', -2);
        this.progressEL!.loading = false;
        this.freshConfigMenuDisable(false);
        this.freshMenuDisable(false);
        this.buttonDisable(false);
      } else {
        this.recordButtonDisable(false);
        this.litSearch!.setPercent('tracing htrace failed, please check your config ', -2);
        this.freshConfigMenuDisable(false);
        this.freshMenuDisable(false);
        this.buttonDisable(false);
      }
    });
  }

  private loadLongTraceFile(timStamp: number) {
    return new Promise(async (resolve) => {
      let maxSize = 48 * 1024 * 1024;
      let traceTypePage: Array<number> = [];
      for (let fileIndex = 0; fileIndex < this.longTraceList.length; fileIndex++) {
        let traceFileName = this.longTraceList[fileIndex];
        if (this.sp!.fileTypeList.some((fileType) => traceFileName.toLowerCase().includes(fileType))) {
          continue;
        }
        let firstLastIndexOf = traceFileName.lastIndexOf('.');
        let firstText = traceFileName.slice(0, firstLastIndexOf);
        let resultLastIndexOf = firstText.lastIndexOf('_');
        traceTypePage.push(Number(firstText.slice(resultLastIndexOf + 1, firstText.length)) - 1);
      }
      traceTypePage.sort((leftNum: number, rightNum: number) => leftNum - rightNum);
      for (let fileIndex = 0; fileIndex < this.longTraceList.length; fileIndex++) {
        if (this.longTraceList[fileIndex] !== '') {
          let types = this.sp!.fileTypeList.filter((type) =>
            this.longTraceList[fileIndex].toLowerCase().includes(type.toLowerCase())
          );
          let pageNumber = 0;
          let fileType = types[0];
          if (types.length === 0) {
            fileType = 'trace';
            let searchNumber =
              Number(
                this.longTraceList[fileIndex].substring(
                  this.longTraceList[fileIndex].lastIndexOf('_') + 1,
                  this.longTraceList[fileIndex].lastIndexOf('.')
                )
              ) - 1;
            pageNumber = traceTypePage.lastIndexOf(searchNumber);
          }
          let pullRes = await HdcDeviceManager.fileRecv(
            this.recordSetting!.longOutPath + this.longTraceList[fileIndex],
            (perNumber: number) => {
              this.litSearch!.setPercent(`downloading ${fileType} file `, perNumber);
            }
          );
          this.litSearch!.setPercent(`downloading ${fileType} file `, 101);
          let buffer = await pullRes.arrayBuffer();
          let chunks = Math.ceil(buffer.byteLength / maxSize);
          let offset = 0;
          let sliceLen = 0;
          let message = {
            fileType: '',
            startIndex: 0,
            endIndex: 0,
            size: 0,
          };
          for (let chunkIndex = 0; chunkIndex < chunks; chunkIndex++) {
            let start = chunkIndex * maxSize;
            let end = Math.min(start + maxSize, buffer.byteLength);
            let chunk = buffer.slice(start, end);
            if (chunkIndex === 0) {
              message.fileType = fileType;
              message.startIndex = chunkIndex;
            }
            sliceLen = Math.min(buffer.byteLength - offset, maxSize);
            if (chunkIndex === 0 && fileType === 'trace') {
              this.sp!.longTraceHeadMessageList.push({
                pageNum: pageNumber,
                data: buffer.slice(offset, 1024),
              });
            }
            this.sp!.longTraceDataList.push({
              index: chunkIndex,
              fileType: fileType,
              pageNum: pageNumber,
              startOffsetSize: offset,
              endOffsetSize: offset + sliceLen,
            });
            await LongTraceDBUtils.getInstance().indexedDBHelp.add(LongTraceDBUtils.getInstance().tableName, {
              buf: chunk,
              id: `${fileType}_${timStamp}_${pageNumber}_${chunkIndex}`,
              fileType: fileType,
              pageNum: pageNumber,
              startOffset: offset,
              endOffset: offset + sliceLen,
              index: chunkIndex,
              timStamp: timStamp,
            });
            offset += sliceLen;
            if (offset >= buffer.byteLength) {
              message.endIndex = chunkIndex;
              message.size = buffer.byteLength;
              if (this.sp!.longTraceTypeMessageMap) {
                if (this.sp!.longTraceTypeMessageMap?.has(pageNumber)) {
                  let oldTypeList = this.sp!.longTraceTypeMessageMap?.get(pageNumber);
                  oldTypeList?.push(message);
                  this.sp!.longTraceTypeMessageMap?.set(pageNumber, oldTypeList!);
                } else {
                  this.sp!.longTraceTypeMessageMap?.set(pageNumber, [message]);
                }
              } else {
                this.sp!.longTraceTypeMessageMap = new Map();
                this.sp!.longTraceTypeMessageMap.set(pageNumber, [message]);
              }
            }
          }
        }
      }
      resolve(1);
    });
  }

  private recordLongTrace(): void {
    let querySelector = this.sp!.shadowRoot?.querySelector('.long_trace_page') as HTMLDivElement;
    if (querySelector) {
      querySelector.style.display = 'none';
    }
    HdcDeviceManager.shellResultAsString(CmdConstant.CMD_GET_LONG_FILES + this.recordSetting!.longOutPath, false).then(
      (result) => {
        this.longTraceList = result.split('\n').filter((fileName) => Boolean(fileName));
        if (this.longTraceList.length > 0) {
          this.sp!.longTraceHeadMessageList = [];
          this.sp!.longTraceDataList = [];
          this.sp!.longTraceTypeMessageMap = undefined;
          let timStamp = new Date().getTime();
          this.loadLongTraceFile(timStamp).then(() => {
            let main = this!.parentNode!.parentNode!.querySelector('lit-main-menu') as LitMainMenu;
            let children = main.menus as Array<MenuGroup>;
            let child = children[0].children as Array<MenuItem>;
            let fileHandler = child[1].clickHandler;
            if (fileHandler && !SpRecordTrace.cancelRecord) {
              this.freshConfigMenuDisable(false);
              this.freshMenuDisable(false);
              this.buttonDisable(false);
              this.recordButtonDisable(false);
              fileHandler(
                {
                  detail: {
                    timeStamp: timStamp,
                  },
                },
                true
              );
            } else {
              SpRecordTrace.cancelRecord = false;
            }
          });
        }
      }
    );
  }

  private initRecordUIState(): void {
    this.buttonDisable(true);
    this.freshMenuDisable(true);
    this.freshConfigMenuDisable(true);
  }

  private isSuccess(traceResult: string): number {
    if (traceResult.indexOf('CreateSession FAIL') != -1 || traceResult.indexOf('failed') != -1) {
      return 1;
    } else if (traceResult.indexOf('Signal') != -1) {
      return 2;
    } else if (traceResult.indexOf('signal(2)') != -1) {
      return 0;
    } else if (traceResult.indexOf('The device is abnormal') != -1) {
      return -1;
    } else {
      return 0;
    }
  }

  private createSessionRequest(): CreateSessionRequest {
    let bufferConfig: ProfilerSessionConfigBufferConfig = {
      pages: this.recordSetting!.bufferSize * 256,
      policy: ProfilerSessionConfigBufferConfigPolicy.RECYCLE,
    };
    let sessionConfig: ProfilerSessionConfig = {
      buffers: [bufferConfig],
      sessionMode: ProfilerSessionConfigMode.OFFLINE,
      resultMaxSize: 0,
      keepAliveTime: 0,
    };
    return {
      requestId: 1,
      sessionConfig: sessionConfig,
      pluginConfigs: [],
    };
  }

  private createMonitorPlugin(that: this, request: CreateSessionRequest): void {
    let processPlugin = that.createProcessPlugin();
    let cpuPlugin = that.createCpuPlugin();
    let diskIoPlugin = that.createDiskIOPlugin();
    let netWorkPlugin = that.createNetworkPlugin();
    request.pluginConfigs.push(processPlugin);
    request.pluginConfigs.push(cpuPlugin);
    request.pluginConfigs.push(diskIoPlugin);
    request.pluginConfigs.push(netWorkPlugin);
  }

  private createNetworkPlugin(): ProfilerPluginConfig<NetworkConfig> {
    let netWorkConfig: NetworkConfig = {};
    let netWorkPlugin: ProfilerPluginConfig<NetworkConfig> = {
      pluginName: 'network-plugin',
      sampleInterval: 1000,
      configData: netWorkConfig,
    };
    return netWorkPlugin;
  }

  private createDiskIOPlugin(): ProfilerPluginConfig<DiskioConfig> {
    let diskIoConfig: DiskioConfig = {
      reportIoStats: 'IO_REPORT',
    };
    let diskIoPlugin: ProfilerPluginConfig<DiskioConfig> = {
      pluginName: 'diskio-plugin',
      sampleInterval: 1000,
      configData: diskIoConfig,
    };
    return diskIoPlugin;
  }

  private createCpuPlugin(): ProfilerPluginConfig<CpuConfig> {
    let cpuConfig: CpuConfig = {
      pid: 0,
      reportProcessInfo: true,
    };
    let cpuPlugin: ProfilerPluginConfig<CpuConfig> = {
      pluginName: 'cpu-plugin',
      sampleInterval: 1000,
      configData: cpuConfig,
    };
    return cpuPlugin;
  }

  private createProcessPlugin(): ProfilerPluginConfig<ProcessConfig> {
    let processConfig: ProcessConfig = {
      report_process_tree: true,
      report_cpu: true,
      report_diskio: true,
      report_pss: true,
    };
    let processPlugin: ProfilerPluginConfig<ProcessConfig> = {
      pluginName: 'process-plugin',
      sampleInterval: 1000,
      configData: processConfig,
    };
    return processPlugin;
  }

  createTraceEvents(traceConfig: Array<string>): Array<string> {
    let traceEvents = new Set<string>();
    traceConfig.forEach((config) => {
      switch (config) {
        case 'Scheduling details':
          this.schedulingEvents.forEach((eve: string) => {
            traceEvents.add(eve);
          });
          break;
        case 'CPU Frequency and idle states':
          this.cpuFreqEvents.forEach((eve: string) => {
            traceEvents.add(eve);
          });
          break;
        case 'High frequency memory':
          this.highFrequencyEvents.forEach((eve: string) => {
            traceEvents.add(eve);
          });
          break;
        case 'Advanced ftrace config':
          this.advancedConfigEvents.forEach((eve: string) => {
            traceEvents.add(eve);
          });
          break;
        case 'Syscalls':
          this.sysCallsEvents.forEach((eve: string) => {
            traceEvents.add(eve);
          });
          break;
        case 'Board voltages & frequency':
          this.powerEvents.forEach((eve: string) => {
            traceEvents.add(eve);
          });
          break;
      }
    });
    let ftraceEventsArray: string[] = [];
    info('traceEvents length is: ', traceEvents.size);
    for (const ftraceEvent of traceEvents) {
      ftraceEventsArray.push(ftraceEvent);
    }
    return ftraceEventsArray;
  }

  private createHilogConfig(reportingFrequency: number): ProfilerPluginConfig<HilogConfig> {
    let appProcess = this.spHilog!.appProcess;
    let re = /^[0-9]+.?[0-9]*/;
    let pid = 0;
    let processId = '';
    if (appProcess.indexOf('(') != -1) {
      processId = appProcess.slice(appProcess.lastIndexOf('(') + 1, appProcess.lastIndexOf(')'));
    } else {
      processId = appProcess;
    }
    if (re.test(processId)) {
      pid = Number(processId);
    }
    let hilogConfig: HilogConfig = {
      pid: pid,
      logLevel: levelFromJSON(this.spHilog!.appLogLevel),
      needClear: true,
    };
    let hilogConfigProfilerPluginConfig: ProfilerPluginConfig<HilogConfig> = {
      pluginName: 'hilog-plugin',
      sampleInterval: reportingFrequency * 1000,
      configData: hilogConfig,
    };
    return hilogConfigProfilerPluginConfig;
  }

  private isNumber(str: string): boolean {
    return !isNaN(Number(str));
  }

  private createHiperConfig(reportingFrequency: number): ProfilerPluginConfig<HiperfPluginConfig> {
    let perfConfig = this.spRecordPerf!.getPerfConfig();
    let recordArgs = '';
    recordArgs = `${recordArgs}-f ${perfConfig?.frequency}`;
    if (perfConfig?.process && !perfConfig?.process.includes('ALL') && perfConfig?.process.length > 0) {
      let process = perfConfig.process;
      if (process.indexOf(',') != -1) {
        let processIdOrName = process.split(',');
        if (this.isNumber(processIdOrName[0])) {
          recordArgs = `${recordArgs} -p ${perfConfig?.process}`;
        } else {
          recordArgs = `${recordArgs} --app ${perfConfig?.process}`;
        }
      } else {
        if (this.isNumber(process)) {
          recordArgs = `${recordArgs} -p ${perfConfig?.process}`;
        } else {
          recordArgs = `${recordArgs} --app ${perfConfig?.process}`;
        }
      }
    } else {
      recordArgs = `${recordArgs} -a `;
    }
    if (perfConfig?.cpu && !perfConfig?.cpu.includes('ALL') && perfConfig?.cpu.length > 0) {
      recordArgs = `${recordArgs} -c ${perfConfig?.cpu}`;
    }
    if (perfConfig?.cpuPercent != 0) {
      recordArgs = `${recordArgs} --cpu-limit ${perfConfig?.cpuPercent}`;
    }
    if (perfConfig?.eventList && !perfConfig?.eventList.includes('NONE') && perfConfig?.eventList.length > 0) {
      recordArgs = `${recordArgs} -e ${perfConfig?.eventList}`;
      if (perfConfig?.isOffCpu) {
        recordArgs = `${recordArgs},sched:sched_waking`;
      }
    } else {
      recordArgs = `${recordArgs} -e hw-cpu-cycles`;
      if (perfConfig?.isOffCpu) {
        recordArgs = `${recordArgs},sched:sched_waking`;
      }
    }
    if (perfConfig?.callStack != 'none') {
      recordArgs = `${recordArgs} --call-stack ${perfConfig?.callStack}`;
    }

    if (perfConfig?.branch != 'none') {
      recordArgs = `${recordArgs} -j ${perfConfig?.branch}`;
    }

    if (perfConfig?.clockType) {
      recordArgs = `${recordArgs} --clockid ${perfConfig?.clockType}`;
    }

    if (perfConfig?.isOffCpu) {
      recordArgs = `${recordArgs} --offcpu`;
    }

    if (perfConfig?.noInherit) {
      recordArgs = `${recordArgs} --no-inherit`;
    }

    if (perfConfig?.mmap) {
      recordArgs = `${recordArgs} -m ${perfConfig.mmap}`;
    }
    info('record config Args is: ', recordArgs);
    let hiPerf: HiperfPluginConfig = {
      isRoot: false,
      outfileName: '/data/local/tmp/perf.data',
      recordArgs: recordArgs,
    };
    if (SpApplication.isLongTrace) {
      hiPerf.splitOutfileName = `${this.recordSetting!.longOutPath}hiprofiler_data_hiperf.htrace`;
    }
    let hiPerfPluginConfig: ProfilerPluginConfig<HiperfPluginConfig> = {
      pluginName: 'hiperf-plugin',
      sampleInterval: reportingFrequency * 1000,
      configData: hiPerf,
    };
    return hiPerfPluginConfig;
  }

  private createSystemConfig(): ProfilerPluginConfig<HiebpfConfig> {
    let systemConfig = this.spFileSystem!.getSystemConfig();
    let recordArgs = 'hiebpf';
    let recordEvent = [];
    if (this.spFileSystem?.startFileSystem) {
      recordEvent.push('fs');
    }
    if (this.spFileSystem?.startVirtualMemory) {
      recordEvent.push('ptrace');
    }
    if (this.spFileSystem?.startIo) {
      recordEvent.push('bio');
    }
    if (recordEvent.length > 0) {
      recordArgs += ` --events ${recordEvent.toString()}`;
    }
    recordArgs += ` --duration ${this.recordSetting?.maxDur}`;
    if (systemConfig?.process && !systemConfig?.process.includes('ALL') && systemConfig?.process.length > 0) {
      recordArgs = `${recordArgs} --pids ${systemConfig?.process}`;
    }
    recordArgs += ` --max_stack_depth ${systemConfig?.unWindLevel}`;
    let systemPluginConfig: HiebpfConfig = {
      cmdLine: recordArgs,
      outfileName: '/data/local/tmp/ebpf.data',
    };
    if (SpApplication.isLongTrace) {
      systemPluginConfig.splitOutfileName = `${this.recordSetting?.longOutPath}hiprofiler_data_ebpf.htrace`;
    }
    let ebpfPluginConfig: ProfilerPluginConfig<HiebpfConfig> = {
      pluginName: 'hiebpf-plugin',
      sampleInterval: 1000,
      configData: systemPluginConfig,
    };
    return ebpfPluginConfig;
  }

  private createMemoryPluginConfig(
    reportingFrequency: number,
    hasmemoryConfig: boolean,
    hasMonitorMemory: boolean,
    hasSmaps: boolean
  ): ProfilerPluginConfig<MemoryConfig> {
    let memoryconfig: MemoryConfig = {
      reportProcessTree: false,
      reportSysmemMemInfo: false,
      sysMeminfoCounters: [],
      reportSysmemVmemInfo: false,
      sysVmeminfoCounters: [],
      reportProcessMemInfo: false,
      reportAppMemInfo: false,
      reportAppMemByMemoryService: false,
      pid: [],
    };
    if (hasmemoryConfig || hasMonitorMemory) {
      memoryconfig.reportProcessTree = true;
      memoryconfig.reportSysmemMemInfo = true;
      memoryconfig.reportSysmemVmemInfo = true;
      memoryconfig.reportProcessMemInfo = true;
    }
    if (hasSmaps || hasMonitorMemory) {
      memoryconfig.reportPurgeableAshmemInfo = true;
      memoryconfig.reportDmaMemInfo = true;
      memoryconfig.reportGpuMemInfo = true;
    }
    if (hasSmaps) {
      memoryconfig.reportSmapsMemInfo = true;
      memoryconfig.reportGpuDumpInfo = true;
      let pid = Number(this.spVmTracker?.process);
      memoryconfig.pid.push(pid);
    }
    if (hasMonitorMemory) {
      SpRecordTrace.ABALITY_MEM_INFO.forEach((va) => {
        memoryconfig.sysMeminfoCounters.push(sysMeminfoTypeFromJSON(va));
      });
    }
    this.probesConfig!.memoryConfig.forEach((value) => {
      if (value.indexOf('Kernel meminfo') != -1) {
        if (hasMonitorMemory) {
          memoryconfig.sysMeminfoCounters = [];
        }
        SpRecordTrace.MEM_INFO.forEach((va) => {
          memoryconfig.sysMeminfoCounters.push(sysMeminfoTypeFromJSON(va));
        });
      }
      if (value.indexOf('Virtual memory stats') != -1) {
        SpRecordTrace.VMEM_INFO.forEach((me) => {
          memoryconfig.sysVmeminfoCounters.push(sysVMeminfoTypeFromJSON(me));
        });
        SpRecordTrace.VMEM_INFO_SECOND.forEach((me) => {
          memoryconfig.sysVmeminfoCounters.push(sysVMeminfoTypeFromJSON(me));
        });
        SpRecordTrace.VMEM_INFO_THIRD.forEach((me) => {
          memoryconfig.sysVmeminfoCounters.push(sysVMeminfoTypeFromJSON(me));
        });
      }
    });
    let profilerPluginConfig: ProfilerPluginConfig<MemoryConfig> = {
      pluginName: 'memory-plugin',
      sampleInterval: reportingFrequency * 1000,
      configData: memoryconfig,
    };
    return profilerPluginConfig;
  }

  private createArkTsConfig(): ProfilerPluginConfig<ArkTSConfig> {
    let process = this.spArkTs!.process;
    let re = /^[0-9]+.?[0-9]*/;
    let pid = 0;
    let processId = '';
    if (process.indexOf('(') != -1) {
      processId = process.slice(process.lastIndexOf('(') + 1, process.lastIndexOf(')'));
    } else {
      processId = process;
    }
    if (re.test(processId)) {
      pid = Number(processId);
    }
    let arkTSConfig: ArkTSConfig = {
      pid: pid,
      type: this.spArkTs!.radioBoxType,
      interval: this.spArkTs!.intervalValue,
      capture_numeric_value: this.spArkTs!.grabNumeric,
      track_allocations: this.spArkTs!.grabAllocations,
      enable_cpu_profiler: this.spArkTs!.grabCpuProfiler,
      cpu_profiler_interval: this.spArkTs!.intervalCpuValue,
    };
    if (SpApplication.isLongTrace) {
      arkTSConfig.splitOutfileName = `${this.recordSetting?.longOutPath}hiprofiler_data_arkts.htrace`;
    }
    let arkTSPluginConfig: ProfilerPluginConfig<ArkTSConfig> = {
      pluginName: 'arkts-plugin',
      sampleInterval: 5000,
      configData: arkTSConfig,
    };

    return arkTSPluginConfig;
  }

  private createFpsPluginConfig(): ProfilerPluginConfig<FpsConfig> {
    let fpsConfig: FpsConfig = {
      reportFps: true,
    };
    let fpsPlugin: ProfilerPluginConfig<FpsConfig> = {
      pluginName: 'hidump-plugin',
      sampleInterval: 1000,
      configData: fpsConfig,
    };
    return fpsPlugin;
  }

  private createHiSystemEventPluginConfig(
    domainName: string,
    eventName: string
  ): ProfilerPluginConfig<HiSystemEventConfig> {
    let hiSystemEventConfig: HiSystemEventConfig = {
      msg: 'hisysevent-plugin',
      subscribe_domain: domainName,
      subscribe_event: eventName,
    };
    let hiSystemEventPlugin: ProfilerPluginConfig<HiSystemEventConfig> = {
      pluginName: 'hisysevent-plugin',
      configData: hiSystemEventConfig,
    };
    return hiSystemEventPlugin;
  }

  private createHtracePluginConfig(): ProfilerPluginConfig<TracePluginConfig> {
    let tracePluginConfig: TracePluginConfig = {
      ftraceEvents: this.createTraceEvents(this.probesConfig!.traceConfig),
      hitraceCategories: [],
      hitraceApps: [],
      bufferSizeKb: this.probesConfig!.ftraceBufferSize,
      flushIntervalMs: 1000,
      flushThresholdKb: 4096,
      parseKsyms: true,
      clock: 'boot',
      tracePeriodMs: 200,
      rawDataPrefix: '',
      traceDurationMs: 0,
      debugOn: false,
    };
    if (this.probesConfig!.traceEvents.length > 0) {
      tracePluginConfig.hitraceCategories = this.probesConfig!.traceEvents;
    }
    let htraceProfilerPluginConfig: ProfilerPluginConfig<TracePluginConfig> = {
      pluginName: 'ftrace-plugin',
      sampleInterval: 1000,
      configData: tracePluginConfig,
    };
    return htraceProfilerPluginConfig;
  }

  private createSdkConfig(): ProfilerPluginConfig<{}> {
    let gpuConfig = this.spSdkConfig!.getGpuConfig();
    let gpuPluginConfig: ProfilerPluginConfig<{}> = {
      pluginName: this.spSdkConfig!.getPlugName(),
      sampleInterval: this.spSdkConfig!.getSampleInterval(),
      configData: gpuConfig,
    };
    return gpuPluginConfig;
  }

  freshConfigMenuDisable(disable: boolean): void {
    let querySelectors = this.shadowRoot?.querySelectorAll<LitMainMenuItem>('lit-main-menu-item');
    querySelectors!.forEach((item) => {
      if (item.title !== 'Hdc Shell') {
        if (disable) {
          item.style.pointerEvents = 'none';
        } else {
          item.style.pointerEvents = 'auto';
        }
        item.disabled = disable;
      }
    });
  }

  public startRefreshDeviceList(): void {
    if (this.refreshDeviceTimer === undefined) {
      this.refreshDeviceTimer = window.setInterval(() => {
        this.refreshDeviceList();
      }, 5000);
    }
  }

  recordButtonDisable(disable: boolean): void {
    if (disable) {
      this.recordButton!.style.pointerEvents = 'none';
    } else {
      this.recordButton!.style.pointerEvents = 'auto';
    }
  }
  buttonDisable(disable: boolean): void {
    if (disable) {
      this.disconnectButton!.style.pointerEvents = 'none';
      this.recordButtonText!.textContent = this.stop;
      this.cancelButtonShow(true);
      this.addButton!.style.pointerEvents = 'none';
      this.deviceSelect!.style.pointerEvents = 'none';
      this.deviceVersion!.style.pointerEvents = 'none';
    } else {
      this.recordButtonText!.textContent = this.record;
      this.cancelButtonShow(false);
      this.disconnectButton!.style.pointerEvents = 'auto';
      this.addButton!.style.pointerEvents = 'auto';
      this.deviceSelect!.style.pointerEvents = 'auto';
      this.deviceVersion!.style.pointerEvents = 'auto';
    }
  }

  freshMenuItemsStatus(currentValue: string): void {
    let litMainMenuGroup = this.shadowRoot?.querySelector<LitMainMenuGroup>('lit-main-menu-group');
    let litMainMenuItemNodeListOf = litMainMenuGroup!.querySelectorAll<LitMainMenuItem>('lit-main-menu-item');
    litMainMenuItemNodeListOf.forEach((item) => {
      item.back = item.title == currentValue;
    });
  }

  synchronizeDeviceList(): void {
    this.deviceSelect!.innerHTML = '';
    if (SpRecordTrace.serialNumber !== '') {
      let option = document.createElement('option');
      option.className = 'select';
      option.selected = true;
      option.value = SpRecordTrace.serialNumber;
      option.textContent = SpRecordTrace.serialNumber;
      this.deviceSelect!.appendChild(option);
      this.recordButton!.hidden = false;
      this.disconnectButton!.hidden = false;
      this.devicePrompt!.innerText = '';
      if (SpRecordTrace.selectVersion && SpRecordTrace.selectVersion !== '') {
        this.setDeviceVersionSelect(SpRecordTrace.selectVersion);
      }
    }
  }
}
