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
import wm from '@ohos.window';

export let floatingWindowOffsetX: number = 50;
export let floatingWindowOffsetY: number = 200;
export let titleWindowOffsetX: number = 300;
export let titleWindowOffsetY: number = 200;
export let lineChartWindowOffsetX: number = 30;
export let lineChartWindowOffsetY: number = 20;
export let windowWidth: number = 2560;
export let windowHeight: number = 1600;

export function initFloatWindow() {
  console.log('cm-floatWindowUtils-initFloatWindow1');

  const windows = [
    'sp_currentNow',
    'sp_shellBackTemp',
    'sp_ddrFrequency',
    'sp_gpuFrequency',
    'sp_cpu0Frequency',
    'sp_cpu1Frequency',
    'sp_cpu2Frequency',
    'sp_RAM',
    'sp_FPS'
  ];

  for (let i = 0; i < windows.length; i++) {
    setTimeout(() => {
      createFloatWindow(windows[i]);
    }, i * 50);
  }

  console.log('cm-floatWindowUtils-initFloatWindow2');
}
export async function createFloatWindow(floatName: string) {
  //5.5SP2  2106 改成 8
  console.log('cm-floatWindowUtils-createFloatWindow1');
  try {
    const floatWin = await wm.create(globalThis.abilityContext, floatName, wm.WindowType.TYPE_FLOAT);
    console.log('cm-floatWindowUtils-createFloatWindow2');

    await floatWin.moveTo(lineChartWindowOffsetX, lineChartWindowOffsetY);
    await floatWin.resetSize(360 * globalThis.coefficient, 270 * globalThis.coefficient);

    try {
      const property = await floatWin.getProperties();
      property.isTransparent = false;
    } catch (e) {
      console.log('cm-floatWindowUtils-getProperties error: ' + JSON.stringify(e));
    }

    console.log('cm-floatWindowUtils-createFloatWindow3');

    let contentPage = '';
    if (floatName == 'sp_currentNow') {
      contentPage = 'pages/CurrentNowLineChartPage';
    } else if (floatName == 'sp_shellBackTemp') {
      contentPage = 'pages/ShellBackTempLineChartPage';
    } else if (floatName == 'sp_ddrFrequency') {
      contentPage = 'pages/DDRLineChartPage';
    } else if (floatName == 'sp_gpuFrequency') {
      console.log('cm-floatWindowUtils-sp_gpuFrequency1');
      contentPage = 'pages/GPULineChartPage';
    } else if (floatName == 'sp_cpu0Frequency') {
      contentPage = 'pages/CPU0LineChartPage';
    } else if (floatName == 'sp_cpu1Frequency') {
      contentPage = 'pages/CPU1LineChartPage';
    } else if (floatName == 'sp_cpu2Frequency') {
      contentPage = 'pages/CPU2LineChartPage';
    } else if (floatName == 'sp_RAM') {
      contentPage = 'pages/RAMLineChartPage';
    } else if (floatName == 'sp_FPS') {
      contentPage = 'pages/FpsLineChartPage';
    }

    if (contentPage) {
      await floatWin.loadContent(contentPage);
      try {
        await floatWin.setBackgroundColor('#B3000000');
        console.log('xhq win setTransparent end.');
        await floatWin.setBackgroundColor('#59020000');
      } catch (e) {
        console.log('cm-floatWindowUtils-setBackgroundColor error: ' + JSON.stringify(e));
      }
      await floatWin.show();
      await floatWin.hide();

      if (floatName == 'sp_gpuFrequency') {
        console.log('cm-floatWindowUtils-sp_gpuFrequency-hide2');
      }
    }
  } catch (e) {
    console.log('cm-floatWindowUtils-createFloatWindow error: ' + JSON.stringify(e));
  }
}
export function destoryAllFloatWindow() {
  console.log('cm-floatWindowUtils-destoryAllFloatWindow');
  destroyFloatWindow('sp_currentNow');
  destroyFloatWindow('sp_shellBackTemp');
  destroyFloatWindow('sp_ddrFrequency');
  destroyFloatWindow('sp_gpuFrequency');
  destroyFloatWindow('sp_cpu0Frequency');
  destroyFloatWindow('sp_cpu1Frequency');
  destroyFloatWindow('sp_cpu2Frequency');
  destroyFloatWindow('sp_RAM');
  destroyFloatWindow('sp_FPS');
}
export function destroyFloatWindow(floatName: string) {
  console.log('cm-floatWindowUtils-destoryAllFloatWindow1');
  wm.find(floatName).then((fltWin) => {
    fltWin.destroy().then(() => {
      globalThis.showFPSLineChartWindow = false;
    });
  });
}
export function hideFloatWindow(floatName: string) {
  wm.find(floatName).then((fltWin) => {
    fltWin.hide();
  });
}
export function showFloatWindow(floatName: string) {
  wm.find(floatName).then((fltWin) => {
    fltWin.show();
  });
}
export function moveFloatWindow(floatName: string, offsetX: number, offsetY: number) {
  var xx =
      floatingWindowOffsetX + offsetX * 2 < 0
      ? 0
      : floatingWindowOffsetX + offsetX * 2 > windowWidth - 200
          ? windowWidth - 200
          : floatingWindowOffsetX + offsetX * 2;
  var yy =
      floatingWindowOffsetY + offsetY * 2 < 0
      ? 0
      : floatingWindowOffsetY + offsetY * 2 > windowHeight - 200
          ? windowHeight - 200
          : floatingWindowOffsetY + offsetY * 2;

  wm.find(floatName).then((fltWin) => {
    fltWin.moveTo(xx, yy);
  });
}
export function setFloatWindow(offsetX: number, offsetY: number) {
  floatingWindowOffsetX =
    floatingWindowOffsetX + offsetX * 2 < 0
    ? 0
    : floatingWindowOffsetX + offsetX * 2 > windowWidth - 200
        ? windowWidth - 200
        : floatingWindowOffsetX + offsetX * 2;
  floatingWindowOffsetY =
    floatingWindowOffsetY + offsetY * 2 < 0
    ? 0
    : floatingWindowOffsetY + offsetY * 2 > windowHeight - 200
        ? windowHeight - 200
        : floatingWindowOffsetY + offsetY * 2;
}
