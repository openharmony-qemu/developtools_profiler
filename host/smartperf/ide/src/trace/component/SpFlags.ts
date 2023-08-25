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

@element('sp-flags')
export class SpFlags extends BaseElement {
  private bodyEl: HTMLElement | undefined | null;

  initElements(): void {
    let parentElement = this.parentNode as HTMLElement;
    parentElement.style.overflow = 'hidden';
    this.bodyEl = this.shadowRoot?.querySelector('.body');
    this.initConfigList();
  }

  initHtml(): string {
    return `
        ${this.initHtmlStyle()}
        <div class="sp-flags-container">
         <div class="body">
           <h3 class="title">Feature flags</h3>
         </div>
        </div>
        `;
  }

  private initHtmlStyle(): string {
    return `
      <style>
        .sp-flags-container {
          background-color: var(--dark-background5,#F6F6F6);
          min-height: 100%;
          display: grid;
          grid-template-columns: 1fr;
          grid-template-rows:1fr;
        }
        :host{
          width: 100%;
          height: 100%;
          background-color: var(--dark-background5,#F6F6F6);
          display: block;
        }
        .body{
          width: 85%;
          margin: 2% 5% 2% 5%;
          background-color: var(--dark-background3,#FFFFFF);
          border-radius: 16px 16px 16px 16px;
          padding-left: 2%;
          padding-right: 4%;
        }
        .title {
          padding-left: 2%;
          margin-left: 8%;
        }
        .flag-widget {
          width: 80%;
          padding: 1% 2% 1% 2%;
          margin-left: 8%;
          margin-right: 8%;
          border-radius: 10px 10px 10px 10px;
        }
        .flag-widget:nth-child(2n+1) {
          background-color: #F5F5F5;
        }
        .flag-title-label {
          margin-right: 10px;
          flex-grow: 1;
          text-align: left;
          opacity: 0.9;
          font-family: Helvetica-Bold;
          font-size: 16px;
          color: #000000;
          line-height: 28px;
          font-weight: 700;
        }
        .flag-head-div {
          display: flex;
          align-items: center;
        }
        .flag-des-div {
          opacity: 0.6;
          font-family: Helvetica;
          font-size: 12px;
          color: var(--dark-color,#000000);
          text-align: left;
          line-height: 20px;
          font-weight: 400;
          margin-top: 0.1%;
        }
        .config_footer {
          margin-top: 1%;
        }
        .flag-select {
          width: 12rem;
          border: 1px solid var(--dark-color1,#4D4D4D);
          border-radius: 16px;
          opacity: 0.6;
          font-family: Helvetica;
          font-size: 12px;
          color: var(--dark-color1,#000000);
          text-align: center;
          line-height: 20px;
          font-weight: 400;
          -webkit-appearance: none;
          background: url(img/down.png) no-repeat 96% center;
        }
        .device_label {
          font-weight: 500;
          margin-right: 10px;
          opacity: 0.9;
          font-family: Helvetica-Bold;
          font-size: 14px;
        }
        .device_input {
          line-height: 20px;
          font-weight: 400;
          margin-right: 2%;
          border-radius: 16px;
          border: 1px solid #ccc;
          padding-left: 10px;
        }
        </style>
    `;
  }

  private createConfigDiv(): HTMLDivElement {
    let configDiv = document.createElement('div');
    configDiv.className = 'flag-widget';
    return configDiv;
  }

  private createCustomDiv(config: FlagConfigItem, configDiv: HTMLDivElement): void {
    let configHadDiv = document.createElement('div');
    configHadDiv.className = 'flag-head-div';
    let titleLabel = document.createElement('label');
    titleLabel.textContent = config.title;
    titleLabel.className = 'flag-title-label';
    let configSelect = document.createElement('select');
    configSelect.className = 'flag-select';
    configSelect.setAttribute('title', config.title);
    config.switchOptions.forEach((optionItem) => {
      let configOption = document.createElement('option');
      configOption.value = optionItem.option;
      configOption.textContent = optionItem.option;
      if (optionItem.selected) {
        configOption.selected = true;
      }
      configSelect.appendChild(configOption);
    });
    configSelect.addEventListener('change', () => {
      let title = configSelect.getAttribute('title');
      FlagsConfig.updateFlagsConfig(title!, configSelect.selectedOptions[0].value);
    });
    let description = document.createElement('div');
    description.className = 'flag-des-div';
    description.textContent = config.describeContent;
    configHadDiv.appendChild(titleLabel);
    configHadDiv.appendChild(configSelect);
    configDiv.appendChild(configHadDiv);
    configDiv.appendChild(description);
  }

  private initConfigList(): void {
    let allConfig = FlagsConfig.getAllFlagConfig();
    allConfig.forEach((config) => {
      let configDiv = this.createConfigDiv();
      this.createCustomDiv(config, configDiv);
      if (config.title === 'AnimationAnalysis') {
        let configFooterDiv = document.createElement('div');
        configFooterDiv.className = 'config_footer';
        let deviceWidthLabelEl = document.createElement('label');
        deviceWidthLabelEl.className = 'device_label';
        deviceWidthLabelEl.textContent = 'PhysicalWidth :';
        let deviceWidthEl = document.createElement('input');
        deviceWidthEl.value = <string> config.addInfo!.physicalWidth;
        deviceWidthEl.addEventListener('keyup', () => {
          deviceWidthEl.value = deviceWidthEl.value.replace(/\D/g, '');
        });
        deviceWidthEl.addEventListener('blur', () => {
          if (deviceWidthEl.value !== '') {
            FlagsConfig.updateFlagsConfig('physicalWidth', Number(deviceWidthEl.value));
          }
        });
        deviceWidthEl.className = 'device_input';
        let deviceHeightLabelEl = document.createElement('label');
        deviceHeightLabelEl.textContent = 'PhysicalHeight :';
        deviceHeightLabelEl.className = 'device_label';
        let deviceHeightEl = document.createElement('input');
        deviceHeightEl.className = 'device_input';
        deviceHeightEl.value = <string> config.addInfo!.physicalHeight;
        deviceHeightEl.addEventListener('keyup', () => {
          deviceHeightEl.value = deviceHeightEl.value.replace(/\D/g, '');
        });
        deviceHeightEl.addEventListener('blur', () => {
          if (deviceWidthEl.value !== '') {
            FlagsConfig.updateFlagsConfig('physicalHeight', Number(deviceHeightEl.value));
          }
        });
        configFooterDiv.appendChild(deviceWidthLabelEl);
        configFooterDiv.appendChild(deviceWidthEl);
        configFooterDiv.appendChild(deviceHeightLabelEl);
        configFooterDiv.appendChild(deviceHeightEl);
        configDiv.appendChild(configFooterDiv);
      }
      this.bodyEl!.appendChild(configDiv);
    });
  }
}

export type Params = {
  [key: string]: unknown;
};

export class FlagsConfig {
  static FLAGS_CONFIG_KEY = 'FlagsConfig';
  static DEFAULT_CONFIG: Array<FlagConfigItem> = [
    {
      title: 'TaskPool',
      switchOptions: [{ option: 'Enabled' }, { option: 'Disabled', selected: true }],
      describeContent: 'Analyze TaskPool templates',
    },
    {
      title: 'AnimationAnalysis',
      switchOptions: [{ option: 'Enabled' }, { option: 'Disabled', selected: true }],
      describeContent: 'Analyze Animation effect templates',
      addInfo: { physicalWidth: 0, physicalHeight: 0 }
    },
    {
      title: 'AppStartup',
      switchOptions: [{ option: 'Enabled' }, { option: 'Disabled', selected: true }],
      describeContent: 'App Startup templates',
    },
    {
      title: 'SchedulingAnalysis',
      switchOptions: [{ option: 'Enabled' }, { option: 'Disabled', selected: true }],
      describeContent: 'Scheduling analysis templates',
    },
    {
      title: 'BinderRunnable',
      switchOptions: [{ option: 'Enabled' }, { option: 'Disabled', selected: true }],
      describeContent: 'support Cpu State Binder-Runnable',
    },
  ];

  static getAllFlagConfig(): Array<FlagConfigItem> {
    let flagsConfigStr = window.localStorage.getItem(FlagsConfig.FLAGS_CONFIG_KEY);
    if (flagsConfigStr === null) {
      let flagConfigObj: Params = {};
      FlagsConfig.DEFAULT_CONFIG.forEach((config) => {
        let selectedOption = config.switchOptions.filter((option) => {
          return option.selected;
        });
        let value = config.switchOptions[0].option;
        if (selectedOption[0] !== undefined) {
          value = selectedOption[0].option;
        }
        flagConfigObj[config.title] = value;
        if (config.addInfo) {
          for (const [key, value] of Object.entries(config.addInfo)) {
            flagConfigObj[key] = value;
          }
        }
      });
      window.localStorage.setItem(FlagsConfig.FLAGS_CONFIG_KEY, JSON.stringify(flagConfigObj));
      return FlagsConfig.DEFAULT_CONFIG;
    } else {
      let flagsConfig = JSON.parse(flagsConfigStr);
      FlagsConfig.DEFAULT_CONFIG.forEach((config) => {
        let cfg = flagsConfig[config.title];
        if (cfg) {
          config.switchOptions.forEach((option) => {
            if (option.option === cfg) {
              option.selected = true;
            } else {
              option.selected = false;
            }
          });
        }
        if (config.addInfo) {
          for (const [key, value] of Object.entries(config.addInfo)) {
            let cfg = flagsConfig[key];
            if (cfg) {
              config.addInfo[key] = cfg;
            }
          }
        }
      });
    }
    return FlagsConfig.DEFAULT_CONFIG;
  }

  static getSpTraceStreamParseConfig(): string {
    let parseConfig = {};
    FlagsConfig.getAllFlagConfig().forEach(
      configItem => {
        let selectedOption = configItem.switchOptions.filter((option) => {
          return option.selected;
        });
        // @ts-ignore
        parseConfig[configItem.title] = selectedOption[0].option === 'Enabled' ? 1 : 0;
      }
    );
    return JSON.stringify({ 'config': parseConfig });
  }

  static getFlagsConfig(flagName: string): Params | undefined {
    let flagConfigObj: Params = {};
    let configItem = FlagsConfig.getAllFlagConfig().find((config) => {
      return config.title === flagName;
    });
    if (configItem) {
      let selectedOption = configItem.switchOptions.filter((option) => {
        return option.selected;
      });
      let value = configItem.switchOptions[0].option;
      if (selectedOption[0] !== undefined) {
        value = selectedOption[0].option;
      }
      flagConfigObj[configItem.title] = value;
      if (configItem.addInfo) {
        for (const [key, value] of Object.entries(configItem.addInfo)) {
          flagConfigObj[key] = value;
        }
      }
      return flagConfigObj;
    } else {
      return configItem;
    }
  }

  static getFlagsConfigEnableStatus(flagName: string): boolean {
    let config = FlagsConfig.getFlagsConfig(flagName);
    let enable: boolean = false;
    if (config && config[flagName]) {
      enable = config[flagName] === 'Enabled';
    }
    return enable;
  }

  static updateFlagsConfig(key: string, value: unknown): void {
    let flagsConfigStr = window.localStorage.getItem(FlagsConfig.FLAGS_CONFIG_KEY);
    let flagConfigObj: Params = {};
    if (flagsConfigStr !== null) {
      flagConfigObj = JSON.parse(flagsConfigStr);
    }
    flagConfigObj[key] = value;
    window.localStorage.setItem(FlagsConfig.FLAGS_CONFIG_KEY, JSON.stringify(flagConfigObj));
  }
}

export interface FlagConfigItem {
  title: string;
  switchOptions: OptionItem[];
  describeContent: string;
  addInfo?: Params;
}

export interface OptionItem {
  option: string;
  selected?: boolean;
}
