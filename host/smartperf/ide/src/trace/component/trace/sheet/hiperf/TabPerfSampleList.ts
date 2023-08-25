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

import { BaseElement, element } from '../../../../../base-ui/BaseElement.js';
import { LitTable } from '../../../../../base-ui/table/lit-table.js';
import { SelectionParam } from '../../../../bean/BoxSelection.js';
import { perfDataQuery } from '../../../chart/PerfDataQuery.js';
import {
  queryPerfProcess,
  queryPerfSampleCallChain,
  queryPerfSampleListByTimeRange,
} from '../../../../database/SqlLite.js';
import { PerfFile, PerfSample, PerfStack, PerfThread } from '../../../../bean/PerfProfile.js';
import { Utils } from '../../base/Utils.js';
import '../../../DisassemblingWindow.js';
import { DisassemblingWindow } from '../../../DisassemblingWindow.js';
import { SpApplication } from '../../../../SpApplication.js';
import { log } from '../../../../../log/Log.js';
import '../../../../../base-ui/slicer/lit-slicer.js';
import { Cmd } from '../../../../../command/Cmd.js';

@element('tabpane-perf-sample')
export class TabPanePerfSample extends BaseElement {
  private perfSampleTbl: LitTable | null | undefined;
  private tblData: LitTable | null | undefined;
  private perfSampleSource: Array<PerfSample> = [];
  private processMap: Map<number, PerfThread> = new Map<number, PerfThread>();
  private perfSampleListModal: DisassemblingWindow | null | undefined;
  private sortKey: string = 'timeString';
  private sortType: number = 0;

  set data(perfSampleSelection: SelectionParam | null | undefined) {
    this.perfSampleListModal!.style.display = 'none';
    this.perfSampleTbl!.style.visibility = 'visible';
    // @ts-ignore
    this.perfSampleTbl?.shadowRoot?.querySelector('.table')?.style?.height =
      this.parentElement!.clientHeight - 40 + 'px';
    this.perfSampleTbl!.recycleDataSource = [];
    // @ts-ignore
    this.tblData?.shadowRoot?.querySelector('.table')?.style?.height = this.parentElement!.clientHeight - 25 + 'px';
    this.tblData!.recycleDataSource = [];
    if (perfSampleSelection) {
      Promise.all([
        queryPerfProcess(),
        queryPerfSampleListByTimeRange(
          perfSampleSelection.leftNs,
          perfSampleSelection.rightNs,
          perfSampleSelection.perfAll ? [] : perfSampleSelection.perfCpus,
          perfSampleSelection.perfAll ? [] : perfSampleSelection.perfProcess,
          perfSampleSelection.perfAll ? [] : perfSampleSelection.perfThread
        ),
      ]).then((results) => {
        let processes = results[0] as Array<PerfThread>;
        log('queryPerfProcess size : ' + processes.length);
        let samples = results[1] as Array<PerfSample>;
        log('queryPerfSampleListByTimeRange size : ' + samples.length);
        this.processMap.clear();
        for (let process of processes) {
          this.processMap.set(process.pid, process);
        }
        for (let sample of samples) {
          let process = this.processMap.get(sample.pid);
          sample.processName =
            process == null || process == undefined
              ? `Process(${sample.pid})`
              : `${process!.processName || 'Process'}(${sample.pid})`;
          sample.threadName =
            sample.threadName == null || sample.threadName == undefined
              ? `Thread(${sample.tid})`
              : `${sample.threadName}(${sample.tid})`;
          sample.coreName = `CPU ${sample.core}`;
          sample.timeString = Utils.getTimeString(sample.time);
          sample.backtrace = [];
          let call = perfDataQuery.callChainMap.get(sample.sampleId);
          if (call == undefined || call == null) {
            sample.depth = 0;
            sample.backtrace.push('No Effective Call Stack');
          } else {
            sample.depth = call.depth;
            sample.backtrace.push(call.name);
            sample.backtrace.push(`(${sample.depth} other frames)`);
          }
        }
        this.perfSampleSource = samples;
        this.sortPerfSampleTable(this.sortKey, this.sortType);
      });
    }
  }

  setRightTableData(sample: PerfSample) {
    queryPerfSampleCallChain(sample.sampleId).then((result) => {
      for (let stack of result) {
        let files = (perfDataQuery.filesData[stack.fileId] ?? []) as Array<PerfFile>;
        stack.path = files[stack.symbolId].path;
        stack.type = stack.path.endsWith('.so.1') || stack.path.endsWith('.dll') || stack.path.endsWith('.so') ? 0 : 1;
      }
      this.tblData!.dataSource = result;
    });
  }

  initElements(): void {
    this.perfSampleTbl = this.shadowRoot?.querySelector<LitTable>('#tb-perf-sample');
    this.tblData = this.shadowRoot?.querySelector<LitTable>('#tb-stack-data');
    this.perfSampleListModal = this.shadowRoot?.querySelector<DisassemblingWindow>('tab-native-data-modal');
    this.perfSampleTbl!.addEventListener('row-click', (e) => {
      // @ts-ignore
      let data = e.detail.data as PerfSample;
      this.setRightTableData(data);
    });
    this.perfSampleTbl!.addEventListener('column-click', (evt) => {
      // @ts-ignore
      this.sortKey = evt.detail.key;
      // @ts-ignore
      this.sortType = evt.detail.sort;
      // @ts-ignore
      this.sortPerfSampleTable(evt.detail.key, evt.detail.sort);
    });
    let lastClikTime = 0;
    let spApplication = <SpApplication>document.getElementsByTagName('sp-application')[0];
    this.tblData!.addEventListener('row-click', (e) => {
      if (Date.now() - lastClikTime < 200 && spApplication.vs) {
        this.perfSampleTbl!.style.visibility = 'hidden';
        new ResizeObserver((entries) => {
          this.perfSampleListModal!.style.width = this.perfSampleTbl!.clientWidth + 'px';
          this.perfSampleListModal!.style.height = this.perfSampleTbl!.clientHeight + 'px';
        }).observe(this.perfSampleTbl!);
        this.perfSampleListModal!.showLoading();
        // @ts-ignore
        let data = e.detail.data as PerfStack;
        let perfSampleListPath = data.path;
        let perfSampleListAddr = data.vaddrInFile;
        let perfSampleListAddrHex = perfSampleListAddr.toString(16);
        if (perfSampleListPath.trim() === '[kernel.kallsyms]') {
          this.perfSampleListModal?.showContent(
            `error : Symbol ${data.symbol} lib is [kernel.kallsyms] ,not support `,
            perfSampleListAddrHex
          );
        } else if (perfSampleListPath.trim() === '') {
          this.perfSampleListModal?.showContent(`error : Symbol ${data.symbol} lib is null `, perfSampleListAddrHex);
        } else if (perfSampleListAddr < 0) {
          this.perfSampleListModal?.showContent(
            `error : Symbol ${data.symbol} current addr is error ` + perfSampleListAddrHex,
            perfSampleListAddrHex
          );
        } else {
          const binDir = 'C:/binary_cache';
          let binPath = binDir + perfSampleListPath;
          let cmd = 'C:/binary_cache/llvm-objdump.exe -S ' + binPath;
          Cmd.execObjDump(cmd, perfSampleListAddrHex, (result: any) => {
            this.perfSampleListModal?.showContent(result, perfSampleListAddrHex);
          });
        }
      }
      lastClikTime = Date.now();
    });
    this.perfSampleListModal!.setCloseListener(() => {
      this.perfSampleListModal!.style.display = 'none';
      this.perfSampleTbl!.style.visibility = 'visible';
    });
  }

  connectedCallback() {
    super.connectedCallback();
    new ResizeObserver((entries) => {
      if (this.parentElement?.clientHeight != 0) {
        // @ts-ignore
        this.perfSampleTbl?.shadowRoot.querySelector('.table').style.height = this.parentElement.clientHeight - 40 + 'px';
        // @ts-ignore
        this.tblData?.shadowRoot.querySelector('.table').style.height = this.parentElement.clientHeight - 25 + 'px';
        this.perfSampleTbl?.reMeauseHeight();
        this.tblData?.reMeauseHeight();
      }
      this.perfSampleListModal!.style.height = this.perfSampleTbl!.clientHeight - 2 + 'px'; //2 is borderWidth
    }).observe(this.parentElement!);
  }

  sortPerfSampleTable(key: string, type: number) {
    this.perfSampleSource.sort((perfSampleA, perfSampleB): number => {
      if (key == 'timeString') {
        if (type == 0) {
          return perfSampleA.time - perfSampleB.time;
        } else if (type == 1) {
          return perfSampleA.time - perfSampleB.time;
        } else {
          return perfSampleB.time - perfSampleA.time;
        }
      } else {
        if (type == 0) {
          return perfSampleA.core - perfSampleB.core;
        } else if (type == 1) {
          return perfSampleA.core - perfSampleB.core;
        } else {
          return perfSampleB.core - perfSampleA.core;
        }
      }
    });
    this.perfSampleTbl!.recycleDataSource = this.perfSampleSource;
  }

  initHtml(): string {
    return `<style>
:host{
    display: flex;
    flex-direction: column;
    padding: 10px 10px;
}
</style>
<div class="perf-sample-content" style="display: flex;flex-direction: row">
    <lit-slicer style="width:100%">
    <div id="left_table" style="width: 65%">
        <tab-native-data-modal style="display:none;"/></tab-native-data-modal>
        <lit-table id="tb-perf-sample" style="height: auto">
            <lit-table-column class="perf-sample-column" order width="1fr" title="Sample Time" data-index="timeString" key="timeString" align="flex-start" ></lit-table-column>
            <lit-table-column class="perf-sample-column" order width="70px" title="Core" data-index="coreName" key="coreName" align="flex-start" ></lit-table-column>
            <lit-table-column class="perf-sample-column" width="1fr" title="Process" data-index="processName" key="processName" align="flex-start" ></lit-table-column>
            <lit-table-column class="perf-sample-column" width="1fr" title="Thread" data-index="threadName" key="threadName" align="flex-start" ></lit-table-column>
            <lit-table-column class="perf-sample-column" width="1fr" title="State" data-index="state" key="state" align="flex-start" ></lit-table-column>
            <lit-table-column class="perf-sample-column" width="1fr" title="Backtrace" data-index="backtrace" key="backtrace" align="flex-start" >
                <template>
                    <div>
                        <span class="title-span">{{backtrace[0]}}</span>
                        <span v-if="backtrace.length > 1">⬅</span>
                        <span v-if="backtrace.length > 1"style="color: #565656"> {{backtrace[1]}}</span>
                    </div>
                </template>
            </lit-table-column>
        </lit-table>
    </div>
    <lit-slicer-track ></lit-slicer-track>
    <lit-table id="tb-stack-data" hideDownload no-head style="height: auto;border-left: 1px solid var(--dark-border1,#e2e2e2)">
        <lit-table-column width="60px" title="" data-index="type" key="type"  align="flex-start" >
            <template>
                <img src="img/library.png" size="20" v-if=" type == 1 ">
                <img src="img/function.png" size="20" v-if=" type == 0 ">
            </template>
        </lit-table-column>
        <lit-table-column width="1fr" title="" data-index="symbol" key="symbol"  align="flex-start"></lit-table-column>
    </lit-table>
    </lit-slicer>
</div>`;
  }
}
