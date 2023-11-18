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

import { SpSystemTrace } from '../SpSystemTrace.js';
import {
  queryAnomalyData,
  queryConfigEnergyAppName,
  queryEnergyEventExits,
  queryMaxStateValue,
  queryPowerData,
  queryStateData,
  queryStateInitValue,
  queryEnergyAppName,
  querySystemLocationData,
  querySystemLockData,
  querySystemSchedulerData,
} from '../../database/SqlLite.js';
import { info } from '../../../log/Log.js';
import { TraceRow } from '../trace/base/TraceRow.js';
import { BaseStruct } from '../../bean/BaseStruct.js';
import { LitPopover } from '../../../base-ui/popover/LitPopoverV.js';
import { EnergyAnomalyRender, EnergyAnomalyStruct } from '../../database/ui-worker/ProcedureWorkerEnergyAnomaly.js';
import { EnergySystemStruct, EnergySystemRender } from '../../database/ui-worker/ProcedureWorkerEnergySystem.js';
import { EnergyPowerStruct, EnergyPowerRender } from '../../database/ui-worker/ProcedureWorkerEnergyPower.js';
import { EnergyStateStruct, EnergyStateRender } from '../../database/ui-worker/ProcedureWorkerEnergyState.js';
import { renders } from '../../database/ui-worker/ProcedureWorker.js';
import { EmptyRender } from '../../database/ui-worker/ProcedureWorkerCPU.js';

export class SpHiSysEnergyChart {
  static app_name: string | null;
  private trace: SpSystemTrace;
  private energyTraceRow: TraceRow<BaseStruct> | undefined;
  private timer: any;
  private stateName: Array<string> = [
    'BRIGHTNESS_NIT',
    'SIGNAL_LEVEL',
    'WIFI_EVENT_RECEIVED',
    'AUDIO_STREAM_CHANGE',
    'AUDIO_VOLUME_CHANGE',
    'WIFI_STATE',
    'BLUETOOTH_BR_SWITCH_STATE',
    'BR_SWITCH_STATE',
    'LOCATION_SWITCH_STATE',
    'SENSOR_STATE',
  ];
  private initValueList: Array<string> = [
    'brightness',
    'nocolumn',
    'nocolumn',
    'nocolumn',
    'nocolumn',
    'wifi',
    'bt_state',
    'bt_state',
    'location',
    'nocolumn',
  ];
  private stateList: Array<string> = [
    'Brightness Nit',
    'Signal Level',
    'Wifi Event Received',
    'Audio Stream Change',
    'Audio Volume Change',
    'Wifi State',
    'Bluetooth Br Switch State',
    'Br Switch State',
    'Location Switch State',
    'Sensor State',
  ];

  constructor(trace: SpSystemTrace) {
    this.trace = trace;
  }

  async init() {
    let result = await queryEnergyEventExits();
    if (result.length <= 0) return;
    let anomalyData = await queryAnomalyData();
    let systemData = await Promise.all([querySystemLocationData(), querySystemLockData(), querySystemSchedulerData()]);
    let systemDataList: any = {};
    systemDataList[0] = this.handleLockData(systemData[1]);
    systemDataList[1] = this.handleLocationData(systemData[0]);
    systemDataList[2] = this.handleWorkData(systemData[2]);
    let powerData = await queryPowerData();
    let stateData: any = {};
    let stateDataSize: number = 0;
    for (let index = 0; index < this.stateList.length; index++) {
      let stateResult = await Promise.all([
        queryStateInitValue(this.stateName[index], this.initValueList[index]),
        queryStateData(this.stateName[index]),
      ]);
      let stateInitValue = this.initValueList[index] == 'nocolumn' ? [] : stateResult[0];
      stateData[index] = stateInitValue.concat(stateResult[1]);
      stateDataSize += stateData[index].length;
    }
    if (anomalyData.length > 0 || systemDataList[0].length > 0 || systemDataList[1].length > 0 || systemDataList[2].length > 0 || powerData.length > 0 || stateDataSize > 0) {
      await this.initEnergyRow();
      this.initAnomaly(anomalyData);
      this.initSystem(systemDataList);
      this.initPower(powerData);
      await this.initState(stateData);
    }
  }

  private initEnergyRow = async () => {
    SpHiSysEnergyChart.app_name = '';
    let appNameFromTable = await queryEnergyAppName();
    let configAppName = await queryConfigEnergyAppName();
    if (configAppName.length > 0 && appNameFromTable.length > 0) {
      let name = configAppName[0].process_name;
      if (name != null) {
        let filterList = appNameFromTable.filter((appNameFromTableElement) => {
          return appNameFromTableElement.string_value?.trim() == name;
        });
        if (filterList.length > 0) {
          SpHiSysEnergyChart.app_name = name;
        }
      }
    }
    if (appNameFromTable.length > 0 && SpHiSysEnergyChart.app_name == '') {
      SpHiSysEnergyChart.app_name = appNameFromTable[0].string_value;
    }
    this.energyTraceRow = TraceRow.skeleton<BaseStruct>();
    let appNameList = this.energyTraceRow?.shadowRoot!.querySelector<LitPopover>('#appNameList');
    let addFlag = false;
    appNameList?.addEventListener('click', () => {
      let itemDiv = appNameList!.querySelector('div');
      if (!addFlag) {
        for (let index = 0; index < appNameFromTable.length; index++) {
          let div = document.createElement('div');
          div.setAttribute('style', 'margin-bottom: 5px');
          let appName = appNameFromTable[index].string_value;
          // @ts-ignore
          let formatAppName = 'appName' + index;
          div.setAttribute('id', formatAppName);
          let inputId = 'appName' + index + 1;
          div.innerHTML =
            "<input class='radio'  name='processoption' " +
            (index == 0 ? 'checked' : '') +
            " type='radio'" +
            'id=' +
            inputId +
            ' app_name=' +
            formatAppName +
            ' value= ' +
            appName +
            '> <label for=' +
            inputId +
            '>' +
            appName +
            '</label>';
          itemDiv!.append(div);
          let appList = this.energyTraceRow?.shadowRoot!.querySelectorAll<HTMLInputElement>(
            'input[type=radio][name=processoption]'
          );
          appList!.forEach((appName) => [
            (appName.onclick = (e: MouseEvent) => {
              if (appName.checked) {
                SpHiSysEnergyChart.app_name = appName.getAttribute('value');
                TraceRow.range!.refresh = true;
                // @ts-ignore
                appNameList!.visible = false;
                this.trace.refreshCanvas(false);
              }
            }),
          ]);
        }
        addFlag = true;
      }
    });

    this.energyTraceRow.rowId = `energy`;
    this.energyTraceRow.rowType = TraceRow.ROW_TYPE_ENERGY;
    this.energyTraceRow.rowParentId = '';
    this.energyTraceRow.folder = true;
    this.energyTraceRow.addTemplateTypes('EnergyEvent');
    this.energyTraceRow.name = 'Energy';
    this.energyTraceRow.style.height = '40px';
    this.energyTraceRow.favoriteChangeHandler = this.trace.favoriteChangeHandler;
    this.energyTraceRow.selectChangeHandler = this.trace.selectChangeHandler;
    this.energyTraceRow.supplier = () => new Promise<Array<BaseStruct>>((resolve) => resolve([]));
    this.energyTraceRow.onThreadHandler = (useCache) => {
      this.energyTraceRow?.canvasSave(this.trace.canvasPanelCtx!);
      if (this.energyTraceRow!.expansion) {
        this.trace.canvasPanelCtx?.clearRect(0, 0, this.energyTraceRow!.frame.width, this.energyTraceRow!.frame.height);
      } else {
        (renders['empty'] as EmptyRender).renderMainThread(
          {
            context: this.trace.canvasPanelCtx,
            useCache: useCache,
            type: ``,
          },
          this.energyTraceRow!
        );
      }
      this.energyTraceRow?.canvasRestore(this.trace.canvasPanelCtx!);
    };
    this.energyTraceRow.addEventListener('expansion-change', () => {
      TraceRow.range!.refresh = true;
      this.trace.refreshCanvas(false);
      if (this.timer) {
        clearTimeout(this.timer);
      }
      this.timer = setTimeout(() => {
        TraceRow.range!.refresh = false;
      }, 200);
    });
    this.trace.rowsEL?.appendChild(this.energyTraceRow!);
  };

  private initAnomaly = (anomalyData: EnergyAnomalyStruct[]): void => {
    let time = new Date().getTime();
    let anomalyTraceRow = TraceRow.skeleton<EnergyAnomalyStruct>();
    anomalyTraceRow.rowParentId = `energy`;
    anomalyTraceRow.rowHidden = true;
    anomalyTraceRow.rowId = 'energy-anomaly';
    anomalyTraceRow.rowType = TraceRow.ROW_TYPE_ANOMALY_ENERGY;
    anomalyTraceRow.favoriteChangeHandler = this.trace.favoriteChangeHandler;
    anomalyTraceRow.selectChangeHandler = this.trace.selectChangeHandler;
    anomalyTraceRow.setAttribute('height', `55px`);
    let element = anomalyTraceRow.shadowRoot?.querySelector('.root') as HTMLDivElement;
    element!.style.height = `55px`;
    anomalyTraceRow.style.width = `100%`;
    anomalyTraceRow.setAttribute('children', '');
    anomalyTraceRow.name = 'Anomaly Event';
    anomalyTraceRow.supplier = () => new Promise<Array<EnergyAnomalyStruct>>((resolve): void => resolve(anomalyData));
    anomalyTraceRow.focusHandler = () => {
      this.trace?.displayTip(
        anomalyTraceRow,
        EnergyAnomalyStruct.hoverEnergyAnomalyStruct,
        `<span>AnomalyName:${EnergyAnomalyStruct.hoverEnergyAnomalyStruct?.eventName || ''}</span>`
      );
    };
    anomalyTraceRow.findHoverStruct = () => {
      EnergyAnomalyStruct.hoverEnergyAnomalyStruct = anomalyTraceRow.getHoverStruct();
    };
    anomalyTraceRow.onThreadHandler = (useCache) => {
      let context: CanvasRenderingContext2D;
      if (anomalyTraceRow.currentContext) {
        context = anomalyTraceRow.currentContext;
      } else {
        context = anomalyTraceRow.collect ? this.trace.canvasFavoritePanelCtx! : this.trace.canvasPanelCtx!;
      }
      anomalyTraceRow.canvasSave(context);
      (renders['energyAnomaly'] as EnergyAnomalyRender).renderMainThread(
        {
          context: context,
          useCache: useCache,
          type: `energyAnomaly`,
          appName: SpHiSysEnergyChart.app_name || '',
          canvasWidth: this.trace.canvasPanel?.width || 0,
        },
        anomalyTraceRow
      );
      anomalyTraceRow.canvasRestore(context);
    };
    this.energyTraceRow?.addChildTraceRow(anomalyTraceRow);
    let durTime = new Date().getTime() - time;
    info('The time to load the anomaly is: ', durTime);
  };

  private initSystem = (systemDataList: any): void => {
    let time = new Date().getTime();
    let systemTraceRow = TraceRow.skeleton<EnergySystemStruct>();
    systemTraceRow.rowParentId = `energy`;
    systemTraceRow.rowHidden = true;
    systemTraceRow.rowId = 'energy-system';
    systemTraceRow.rowType = TraceRow.ROW_TYPE_SYSTEM_ENERGY;
    systemTraceRow.favoriteChangeHandler = this.trace.favoriteChangeHandler;
    systemTraceRow.selectChangeHandler = this.trace.selectChangeHandler;
    systemTraceRow.setAttribute('height', `80px`);
    let element = systemTraceRow.shadowRoot?.querySelector('.root') as HTMLDivElement;
    element!.style.height = `90px`;
    systemTraceRow.style.width = `100%`;
    systemTraceRow.setAttribute('children', '');
    systemTraceRow.name = 'System Event';
    systemTraceRow.supplier = () => new Promise<Array<EnergySystemStruct>>((resolve): void => resolve(systemDataList));
    systemTraceRow.focusHandler = () => {
      this.trace?.displayTip(
        systemTraceRow,
        EnergySystemStruct.hoverEnergySystemStruct,
        `<div style="width: 250px">
                                <div style="display: flex"><div style="width: 75%;text-align: left">WORKSCHEDULER: </div><div style="width: 20%;text-align: left">${
                                  EnergySystemStruct.hoverEnergySystemStruct?.workScheduler! || 0
                                }</div></div>
                                <div style="display: flex"><div style="width: 75%;text-align: left">POWER_RUNNINGLOCK: </div><div style="width: 20%;text-align: left">${
                                  EnergySystemStruct.hoverEnergySystemStruct?.power! || 0
                                }</div></div>
                                <div style="display: flex"><div style="width: 75%;text-align: left">LOCATION: </div><div style="width: 20%;text-align: left">${
                                  EnergySystemStruct.hoverEnergySystemStruct?.location! || 0
                                }</div></div>
                            </div>`
      );
    };
    systemTraceRow.findHoverStruct = () => {
      EnergySystemStruct.hoverEnergySystemStruct = systemTraceRow.getHoverStruct();
    };
    systemTraceRow.onThreadHandler = (useCache) => {
      let context: CanvasRenderingContext2D;
      if (systemTraceRow.currentContext) {
        context = systemTraceRow.currentContext;
      } else {
        context = systemTraceRow.collect ? this.trace.canvasFavoritePanelCtx! : this.trace.canvasPanelCtx!;
      }
      systemTraceRow.canvasSave(context);
      (renders['energySystem'] as EnergySystemRender).renderMainThread(
        {
          context: context,
          useCache: useCache,
          type: `energySystem`,
        },
        systemTraceRow
      );
      systemTraceRow.canvasRestore(context);
    };
    this.energyTraceRow?.addChildTraceRow(systemTraceRow);
    let durTime = new Date().getTime() - time;
    info('The time to load the Ability Memory is: ', durTime);
  };

  private handleLocationData(result: Array<{
    ts: string;
    eventName: string;
    appKey: string;
    Value: string;
  }>) {
    let locationIndex = -1;
    let locationCount = 0;
    let locationData: any[] = [];
    result.forEach((item: any) => {
      let da: any = {};
      if (item.Value == 'stop') {
        if (locationIndex == -1) {
          da.startNs = 0;
          da.count = 1;
        } else {
          da.startNs = item.ts;
          locationCount--;
          da.count = locationCount;
        }
        da.state = 'stop';
      } else {
        da.startNs = item.ts;
        locationCount++;
        da.count = locationCount;
        da.state = 'start';
      }
      locationIndex = 0;
      da.type = 2;
      locationData.push(da);
    });
    return locationData;
  }

  private handleLockData(result: Array<{
    ts: string;
    eventName: string;
    appKey: string;
    Value: string;
  }>) {
    let lockCount = 0;
    let tokedIds: Array<string> = [];
    let lockData: any[] = [];
    result.forEach((item: any) => {
      let running: any = {};
      let split = item.Value.split(',');
      if (item.Value.indexOf('ADD') > -1) {
        running.startNs = item.ts;
        lockCount++;
        running.count = lockCount;
        running.token = split[0].split('=')[1];
        running.type = 1;
        tokedIds.push(running.token);
        lockData.push(running);
      } else {
        running.startNs = item.ts;
        let toked = split[0].split('=')[1];
        let number = tokedIds.indexOf(toked);
        if (number > -1) {
          lockCount--;
          running.count = lockCount;
          running.token = split[0].split('=')[1];
          running.type = 1;
          lockData.push(running);
          delete tokedIds[number];
        }
      }
    });

    return lockData;
  }

  private handleWorkData(workDataArray: Array<any>) {
    let workCountMap: Map<string, number> = new Map<string, number>();
    let nameIdMap: Map<string, Array<any>> = new Map<string, []>();
    let workData: any[] = [];
    for (let i = 0; i < workDataArray.length; i++) {
      let dd: any = {};
      let item = workDataArray[i];
      let keys = item.appKey.split(',');
      let values = item.Value.split(',');
      for (let j = 0; j < keys.length; j++) {
        let key = keys[j];
        switch (key) {
          case 'NAME':
            dd.appName = values[j];
            break;
          case 'WORKID':
            dd.workId = values[j];
            break;
        }
      }
      if (item.eventName == 'WORK_START') {
        let nameIdList = nameIdMap.get(dd.appName);
        let workCount = 0;
        if (nameIdList == undefined) {
          workCount = 1;
          nameIdMap.set(dd.appName, [dd.workId]);
        } else {
          nameIdList.push(dd.workId);
          workCount = nameIdList.length;
        }
        let count = workCountMap.get(dd.appName);
        if (count == undefined) {
          workCountMap.set(dd.appName, 1);
        } else {
          workCountMap.set(dd.appName, count + 1);
        }
        dd.startNs = item.ts;
        dd.count = workCount;
        dd.type = 0;
        workData.push(dd);
      } else if (item.eventName == 'WORK_STOP') {
        let nameIdList: any = nameIdMap.get(dd.appName);
        let index = nameIdList.indexOf(dd.workId);
        if (nameIdList != undefined && index > -1) {
          delete nameIdList[index];
          let workCount = workCountMap.get(dd.appName);
          if (workCount != undefined) {
            workCount = workCount - 1;
            workCountMap.set(dd.appName, workCount);
            dd.startNs = item.startNS;
            dd.count = workCount;
            dd.type = 0;
            workData.push(dd);
          }
        }
      }
    }
    return workData;
  }

  private initPower = (powerData: Array<{
    startNS: number;
    eventName: string;
    appKey: string;
    eventValue: string;
  }>): void => {
    let time = new Date().getTime();
    let powerTraceRow = TraceRow.skeleton<EnergyPowerStruct>();
    powerTraceRow.rowParentId = `energy`;
    powerTraceRow.rowHidden = true;
    powerTraceRow.rowId = 'energy-power';
    powerTraceRow.rowType = TraceRow.ROW_TYPE_POWER_ENERGY;
    powerTraceRow.favoriteChangeHandler = this.trace.favoriteChangeHandler;
    powerTraceRow.selectChangeHandler = this.trace.selectChangeHandler;
    powerTraceRow.setAttribute('height', `200px`);
    let element = powerTraceRow.shadowRoot?.querySelector('.root') as HTMLDivElement;
    element!.style.height = `200px`;
    powerTraceRow.style.width = `100%`;
    powerTraceRow.setAttribute('children', '');
    powerTraceRow.name = 'Power';
    powerTraceRow.supplier = () => this.getPowerData(powerData);
    powerTraceRow.findHoverStruct = () => {
      EnergyPowerStruct.hoverEnergyPowerStruct = powerTraceRow.getHoverStruct();
    };
    powerTraceRow.focusHandler = () => {
      this.trace?.displayTip(
        powerTraceRow,
        EnergyPowerStruct.hoverEnergyPowerStruct,
        `<div style="width: 120px">
                            <div style="display: flex"><div style="width: 80%;text-align: left">CPU: </div><div style="width: 20%;text-align: left">${
                              EnergyPowerStruct.hoverEnergyPowerStruct?.cpu! || 0
                            }</div></div>
                            <div style="display: flex"><div style="width: 80%;text-align: left">location: </div><div style="width: 20%;text-align: left">${
                              EnergyPowerStruct.hoverEnergyPowerStruct?.location! || 0
                            }</div></div>
                            <div style="display: flex"><div style="width: 80%;text-align: left">GPU: </div><div style="width: 20%;text-align: left">${
                              EnergyPowerStruct.hoverEnergyPowerStruct?.gpu! || 0
                            }</div></div>
                            <div style="display: flex"><div style="width: 80%;text-align: left">display: </div><div style="width: 20%;text-align: left">${
                              EnergyPowerStruct.hoverEnergyPowerStruct?.display! || 0
                            }</div></div>
                            <div style="display: flex"><div style="width: 80%;text-align: left">camera: </div><div style="width: 20%;text-align: left">${
                              EnergyPowerStruct.hoverEnergyPowerStruct?.camera! || 0
                            }</div></div>
                            <div style="display: flex"><div style="width: 80%;text-align: left">bluetooth: </div><div style="width: 20%;text-align: left">${
                              EnergyPowerStruct.hoverEnergyPowerStruct?.bluetooth! || 0
                            }</div></div>
                            <div style="display: flex"><div style="width: 80%;text-align: left">flashlight: </div><div style="width: 20%;text-align: left">${
                              EnergyPowerStruct.hoverEnergyPowerStruct?.flashlight! || 0
                            }</div></div>
                            <div style="display: flex"><div style="width: 80%;text-align: left">audio: </div><div style="width: 20%;text-align: left">${
                              EnergyPowerStruct.hoverEnergyPowerStruct?.audio! || 0
                            }</div></div>
                            <div style="display: flex"><div style="width: 80%;text-align: left">wifiScan: </div><div style="width: 20%;text-align: left">${
                              EnergyPowerStruct.hoverEnergyPowerStruct?.wifiscan! || 0
                            }</div></div>
                        </div>`
      );
    };
    powerTraceRow.onThreadHandler = (useCache) => {
      let context: CanvasRenderingContext2D;
      if (powerTraceRow.currentContext) {
        context = powerTraceRow.currentContext;
      } else {
        context = powerTraceRow.collect ? this.trace.canvasFavoritePanelCtx! : this.trace.canvasPanelCtx!;
      }
      powerTraceRow.canvasSave(context);
      (renders['energyPower'] as EnergyPowerRender).renderMainThread(
        {
          context: context,
          useCache: useCache,
          type: `energyPower`,
          appName: SpHiSysEnergyChart.app_name || '',
        },
        powerTraceRow!
      );
      powerTraceRow!.canvasRestore(context);
    };
    this.energyTraceRow?.addChildTraceRow(powerTraceRow);
    let durTime = new Date().getTime() - time;
    info('The time to load the energy power is: ', durTime);
  };

  async getPowerData(items: any): Promise<any> {
    let powerDataMap: any = {};
    let appNameList: any = [];
    items.forEach((item: any) => {
      let dataItem = powerDataMap[item.startNS];
      if (dataItem == undefined) {
        if (item.appKey == 'APPNAME') {
          let appMap: any = {};
          let appNames = item.eventValue.split(',');
          appNameList = appNames;
          if (appNames.length > 0) {
            for (let appNamesKey of appNames) {
              appMap[appNamesKey] = new EnergyPowerStruct();
              appMap[appNamesKey].name = appNamesKey;
              appMap[appNamesKey].ts = item.startNS;
            }
            powerDataMap[item.startNS] = appMap;
          }
        }
      } else {
        if (item.appKey != 'APPNAME') {
          let values = item.eventValue.split(',');
          for (let i = 0; i < values.length; i++) {
            let appName = appNameList[i];
            let obj = dataItem[appName];
            if (obj != undefined) {
              let eventName = item.eventName.split('_');
              let s = eventName[eventName.length - 1].toLocaleLowerCase();
              if (obj[s] == undefined) {
                obj[s] = parseInt(values[i]);
              } else {
                obj[s] += parseInt(values[i]);
              }
            }
          }
        } else {
          let dataMap = powerDataMap[item.startNS];
          let appNames = item.eventValue.split(',');
          appNameList = appNames;
          if (appNames.length > 0) {
            for (let appNamesKey of appNames) {
              dataMap[appNamesKey] = new EnergyPowerStruct();
              dataMap[appNamesKey].name = appNamesKey;
              dataMap[appNamesKey].ts = item.startNS;
            }
          }
        }
      }
    });
    // @ts-ignore
    return Object.values(powerDataMap);
  }

  private initState = async (stateData: any) => {
    let time = new Date().getTime();
    for (let index = 0; index < this.stateList.length; index++) {
      let maxStateData = await queryMaxStateValue(this.stateName[index]);
      if (!maxStateData[0]) {
        continue;
      }
      let maxStateTotal = maxStateData[0].maxValue.toString();
      let statType = maxStateData[0].type.toLocaleLowerCase();
      if (statType.includes('state') && !statType.endsWith('br_switch_state')) {
        if (maxStateData[0].maxValue == 0) {
          maxStateTotal = 'enable';
        } else {
          maxStateTotal = 'disable';
        }
      }
      let stateTraceRow = TraceRow.skeleton<EnergyStateStruct>();
      stateTraceRow.rowParentId = `energy`;
      stateTraceRow.rowHidden = true;
      stateTraceRow.rowId = `energy-state-${this.stateList[index]}`;
      stateTraceRow.rowType = TraceRow.ROW_TYPE_STATE_ENERGY;
      stateTraceRow.favoriteChangeHandler = this.trace.favoriteChangeHandler;
      stateTraceRow.selectChangeHandler = this.trace.selectChangeHandler;
      stateTraceRow.style.height = '40px';
      stateTraceRow.style.width = `100%`;
      stateTraceRow.setAttribute('children', '');
      stateTraceRow.name = `${this.stateList[index]}`;
      stateTraceRow.supplier = () => new Promise<Array<any>>((resolve): void => resolve(stateData[index]));
      stateTraceRow.findHoverStruct = () => {
        EnergyStateStruct.hoverEnergyStateStruct = stateTraceRow.getHoverStruct();
      };
      stateTraceRow.focusHandler = () => {
        let tip = '';
        if (EnergyStateStruct.hoverEnergyStateStruct?.type!.toLocaleLowerCase().includes('state')) {
          tip = `<span>Switch Status: ${
            EnergyStateStruct.hoverEnergyStateStruct?.value == 1 ? 'disable' : 'enable'
          }</span>`;
          if (EnergyStateStruct.hoverEnergyStateStruct?.type!.toLocaleLowerCase().endsWith('br_switch_state')) {
            tip = `<span>${SpHiSysEnergyChart.getBlueToothState(
              EnergyStateStruct.hoverEnergyStateStruct?.value
            )}</span>`;
          }
        } else {
          tip = `<span>value: ${EnergyStateStruct.hoverEnergyStateStruct?.value || 0}</span>`;
        }
        this.trace?.displayTip(stateTraceRow, EnergyStateStruct.hoverEnergyStateStruct, tip);
      };
      stateTraceRow.onThreadHandler = (useCache) => {
        let context: CanvasRenderingContext2D;
        if (stateTraceRow.currentContext) {
          context = stateTraceRow.currentContext;
        } else {
          context = stateTraceRow.collect ? this.trace.canvasFavoritePanelCtx! : this.trace.canvasPanelCtx!;
        }
        stateTraceRow.canvasSave(context);
        (renders['energyState'] as EnergyStateRender).renderMainThread(
          {
            context: context,
            useCache: useCache,
            type: `energyState${index}`,
            maxState: maxStateData[0].maxValue,
            maxStateName:
              maxStateData[0].type.toLocaleLowerCase().endsWith('br_switch_state') ? '-1' : maxStateTotal.toString(),
          },
          stateTraceRow
        );
        stateTraceRow.canvasRestore(context);
      };
      this.energyTraceRow?.addChildTraceRow(stateTraceRow);
      let durTime = new Date().getTime() - time;
      info('The time to load the Ability Memory is: ', durTime);
    }
  };

  public static getBlueToothState(num: number | undefined): string {
    switch (num) {
      case 0:
        return 'STATE_TURNING_ON';
      case 1:
        return 'STATE_TURN_ON';
      case 2:
        return 'STATE_TURNING_OFF';
      case 3:
        return 'STATE_TURN_OFF';
      default:
        return 'UNKNOWN_STATE';
    }
  }
}
