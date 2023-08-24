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

import { BaseElement, element } from '../../../base-ui/BaseElement.js';
import LitSwitch, { LitSwitchChangeEvent } from '../../../base-ui/switch/lit-switch.js';
import { HiperfPluginConfig, ProfilerPluginConfig, TracePluginConfig } from './bean/ProfilerServiceTypes.js';
import { SpRecordTrace } from '../SpRecordTrace.js';

@element('sp-record-template')
export class SpRecordTemplate extends BaseElement {
  static SCHEDULING_ANALYSIS_EVENT = [
    'sched/sched_wakeup',
    'sched/sched_switch',
    'sched/sched_wakeup_new',
    'sched/sched_waking',
    'sched/sched_process_exit',
    'sched/sched_process_free',
    'task/task_newtask',
    'task/task_rename',
    'power/cpu_frequency',
    'power/cpu_idle',
    'irq/irq_handler_entry',
    'irq/irq_handler_exit',
    'irq/softirq_entry',
    'irq/softirq_exit',
    'irq/softirq_raise',
  ];
  static FRAME_TIMELINE_EVENTS = [
    'sched/sched_switch',
    'sched/sched_wakeup',
    'sched/sched_wakeup_new',
    'sched/sched_waking',
    'sched/sched_process_exit',
    'sched/sched_process_free',
    'sched/sched_process_free',
    'task/task_rename',
    'power/cpu_frequency',
    'power/cpu_idle',
    'power/suspend_resume',
  ];
  static FRAME_TIMELINE_CATEGORIES_EVENT = [
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
  ];
  static HIPERF_DEFAULT_RECORD_ARGS = '-f 1000 -a  --cpu-limit 100 -e hw-cpu-cycles,sched:sched_waking' +
    ' --call-stack dwarf --clockid monotonic --offcpu -m 256';
  private frameTimeline: LitSwitch | undefined | null;
  private schedulingAnalysis: LitSwitch | undefined | null;
  private appStartup: LitSwitch | undefined | null;
  private taskPoolEl: LitSwitch | undefined | null;
  private dynamicEffectEl: LitSwitch | undefined | null;

  initElements(): void {
    this.frameTimeline = this.shadowRoot?.querySelector<LitSwitch>('#frame_timeline');
    this.schedulingAnalysis = this.shadowRoot?.querySelector<LitSwitch>('#scheduling_analysis');
    this.appStartup = this.shadowRoot?.querySelector<LitSwitch>('#app_startup');
    this.taskPoolEl = this.shadowRoot?.querySelector<LitSwitch>('#task_pool');
    this.dynamicEffectEl = this.shadowRoot?.querySelector<LitSwitch>('#dynamic_effect');
    this.addProbeListener(this.frameTimeline!, this.schedulingAnalysis!, this.appStartup!, this.taskPoolEl!, this.dynamicEffectEl!);
  }

  addProbeListener(...elements: HTMLElement[]) {
    elements.forEach(element => {
      element.addEventListener('change', (event: CustomEventInit<LitSwitchChangeEvent>) => {
        let detail = event.detail;
        if (detail!.checked) {
          this.dispatchEvent(new CustomEvent('addProbe', {}));
        }
      });
    });
  }
  getTemplateConfig(): Array<ProfilerPluginConfig<{}>> {
    let config: Array<ProfilerPluginConfig<{}>> = [];
    let traceEventSet = new Array<string>();
    let hitraceCategories = new Array<string>();
    let useFtracePlugin: boolean = false;
    if (this.frameTimeline?.checked || this.appStartup?.checked || this.dynamicEffectEl?.checked) {
      useFtracePlugin = true;
      SpRecordTemplate.FRAME_TIMELINE_CATEGORIES_EVENT.forEach((categories) => {
        if (hitraceCategories.indexOf(categories) == -1) {
          hitraceCategories.push(categories);
        }
      });
      if (this.appStartup?.checked) {
        hitraceCategories.push('musl');
        config.push(this.createHiperfDefaultConfig());
      }
      SpRecordTemplate.FRAME_TIMELINE_EVENTS.forEach((ev) => {
        if (traceEventSet.indexOf(ev) == -1) {
          traceEventSet.push(ev);
        }
      });
    }
    if (this.schedulingAnalysis?.checked) {
      useFtracePlugin = true;
      SpRecordTemplate.SCHEDULING_ANALYSIS_EVENT.forEach((event) => {
        if (traceEventSet.indexOf(event) < 0) {
          traceEventSet.push(event);
        }
      });
    }
    if (this.taskPoolEl!.checked) {
      useFtracePlugin = true;
      traceEventSet.push('commonlibrary');
    }
    if (useFtracePlugin) {
      let tracePluginConfig: TracePluginConfig = {
        ftraceEvents: traceEventSet,
        hitraceCategories: hitraceCategories,
        flushIntervalMs: 1000,
        hitraceApps: [],
        bufferSizeKb: 2048,
        debugOn: false,
        flushThresholdKb: 4096,
        clock: 'boot',
        tracePeriodMs: 200,
        parseKsyms: true,
        rawDataPrefix: '',
        traceDurationMs: 0,
        hitraceTime: this.args.recordSetting!.maxDur,
      };
      let htraceProfilerPluginConfig: ProfilerPluginConfig<TracePluginConfig> = {
        pluginName: 'ftrace-plugin',
        sampleInterval: 1000,
        configData: tracePluginConfig,
      };
      SpRecordTrace.appendSerialize(htraceProfilerPluginConfig);
      config.push(htraceProfilerPluginConfig);
    }
    return config;
  }

  private createHiperfDefaultConfig() {
    let hiPerf: HiperfPluginConfig = {
      isRoot: false,
      outfileName: '/data/local/tmp/perf.data',
      recordArgs: SpRecordTemplate.HIPERF_DEFAULT_RECORD_ARGS,
    };
    let htraceProfilerPluginConfig: ProfilerPluginConfig<HiperfPluginConfig> = {
      pluginName: 'hiperf-plugin',
      sampleInterval: 5000,
      configData: hiPerf,
    };
    return htraceProfilerPluginConfig;
  }
  initHtml(): string {
    return `
        <style>
        .template-config-div {
          flex-direction: column;
          margin-bottom: 3vh;
          width: 80%;
          display: flex;
        }
        .template-title {
            line-height: 40px;
            font-weight: 700;
            margin-right: 10px;
            opacity: 0.9;
            font-family: Helvetica-Bold;
            font-size: 18px;
            text-align: center;
        }
        :host{
            background: var(--dark-background3,#FFFFFF);
            border-radius: 0px 16px 16px 0px;
            display: inline-block;
            width: 100%;
            height: 100%;
         }
         .root {
            font-size:16px;
            width: 100%;
            height: 95%;
            padding-top: 50px;
            padding-left: 54px;
            margin-right: 30px;
            margin-bottom: 30px;
        }
        lit-switch {
             display: inline;
             float: right;
             height: 38px;
             margin-top: 10px;
        }
        </style>
        <div class="root">
            <div class="template-config-div">
               <div>
                 <span class="template-title">Frame timeline</span>
                 <lit-switch class="config_switch" id="frame_timeline"></lit-switch>
               </div>
            </div>
             <div class="template-config-div">
               <div>
                 <span class="template-title">Scheduling analysis</span>
                 <lit-switch class="config_switch" id="scheduling_analysis"></lit-switch>
               </div>
            </div>
            <div class="template-config-div">
               <div>
                 <span class="template-title">App startup</span>
                 <lit-switch class="config_switch" id="app_startup"></lit-switch>
               </div>
            </div>
            <div class="template-config-div">
               <div>
                 <span class="template-title">Task pool</span>
                 <lit-switch class="config_switch" id="task_pool"></lit-switch>
               </div>
            </div>
            <div class="template-config-div">
               <div>
                 <span class="template-title">Animation effect</span>
                 <lit-switch class="config_switch" id="dynamic_effect"></lit-switch>
               </div>
            </div> 
        </div>
        `;
  }
}
