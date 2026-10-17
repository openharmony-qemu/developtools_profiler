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

import Ability from '@ohos.app.ability.UIAbility';
import { NetWork } from '../common/profiler/item/NetWork';
import display from '@ohos.display'; // 导入模块
import WorkerHandler from '../common/profiler/WorkerHandler';
import worker from '@ohos.worker';

let MainWorker = null;
let abilityWindowStage;

function initWorker() {
  try {
    if (!MainWorker) {
      MainWorker = new worker.Worker('entry/ets/workers/worker.js');
      globalThis.MainWorker = MainWorker;

      MainWorker.onmessage = function (result): void {
        try {
          WorkerHandler.socketHandler(result);
        } catch (e) {
          console.log('cm-MainAbility-WorkerHandler error: ' + JSON.stringify(e));
        }
      };

      MainWorker.onerror = function (error): void {
        console.log('cm-MainAbility-Worker error: ' + JSON.stringify(error));
      };

      console.log('cm-MainAbility-Worker initialized');
    }
  } catch (e) {
    console.log('cm-MainAbility-initWorker error: ' + JSON.stringify(e));
  }
}
export default class MainAbility extends Ability {
  onCreate(): void {
    console.log('cm-MainAbility-onCreate');
    globalThis.showFloatingWindow = false;
    globalThis.coefficient = 1;
    globalThis.screenWith = 1080;
    initWorker();
  }
  onDestroy(): void {
    try {
      if (MainWorker) {
        MainWorker.terminate();
        MainWorker = null;
        globalThis.MainWorker = null;
      }
    } catch (e) {
      console.log('cm-MainAbility-onDestroy error: ' + JSON.stringify(e));
    }
    console.log('cm-MainAbility-onDestroy');
  }
  onWindowStageCreate(windowStage): void {
    console.log('cm-MainAbility-onWindowStageCreate');
    globalThis.abilityContext = this.context;
    abilityWindowStage = windowStage;
    abilityWindowStage.setUIContent(this.context, 'pages/LoginPage', null);
    globalThis.useDaemon = false;

    setTimeout(() => {
      display.getDefaultDisplay().then(
        (disp) => {
          globalThis.screenWith = disp.width;
          if (globalThis.screenWith > 1400) {
            globalThis.coefficient = 1;
            console.log(
              'globalThis.screenWith :coefficient-- ' + globalThis.coefficient
            );
          } else if (
            globalThis.screenWith > 800 &&
            globalThis.screenWith < 1400
          ) {
            globalThis.coefficient = 1.8;
            console.log(
              'globalThis.screenWith :coefficient-- ' + globalThis.coefficient
            );
          } else {
            globalThis.coefficient = 1;
            console.log(
              'globalThis.screenWith :coefficient-- ' + globalThis.coefficient
            );
          }
          console.log('globalThis.screenWith : ' + globalThis.screenWith);
        },
        (err) => {
          console.log(
            'display.getDefaultDisplay failed, error : ' + JSON.stringify(err)
          );
        }
      );
    }, 100);
  }
  onWindowStageDestroy(): void {}
  onForeground(): void {
    setTimeout(() => {
      try {
        if (MainWorker && globalThis.MainWorker) {
          MainWorker.postMessage({ testConnection: true });
        } else {
          console.log('cm-MainAbility-onForeground Worker not initialized');
        }
      } catch (e) {
        console.log('cm-MainAbility-onForeground error: ' + JSON.stringify(e));
      }
    }, 50);
    console.log('cm-MainAbility-onForeground');
  }
  onBackground(): void {}
}
