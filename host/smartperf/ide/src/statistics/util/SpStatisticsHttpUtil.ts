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

import { BurialPointRequestBody } from './SpStatisticsHttpBean.js';

export class SpStatisticsHttpUtil {
  static requestServerInfo: string = '';
  static serverTime: number = 0;
  static timeDiff: number = 0;
  static retryCount: number = 0;
  static retryMaxCount: number = 5;
  static pauseRetry: boolean = false;
  static retryRestTimeOut: boolean = false;

  static initStatisticsServerConfig() {
    if (SpStatisticsHttpUtil.requestServerInfo === '') {
      SpStatisticsHttpUtil.requestServerInfo = SpStatisticsHttpUtil.getRequestServerInfo();
    }
    if (SpStatisticsHttpUtil.serverTime == 0) {
      SpStatisticsHttpUtil.getServerTime();
    }
  }

  static getRequestServerInfo(): string {
    let req = new XMLHttpRequest();
    req.open(
      'GET',
      `${window.location.protocol}//${window.location.host.split(':')[0]}:${
        window.location.port
      }/application/serverInfo`,
      false
    );
    req.send(null);
    if (req.status == 200) {
      let requestInfo = req.getResponseHeader('request_info');
      if (requestInfo && requestInfo.length > 0) {
        return requestInfo;
      }
    }
    return '';
  }

  static getServerTime() {
    if (SpStatisticsHttpUtil.requestServerInfo === '') {
      SpStatisticsHttpUtil.requestServerInfo = SpStatisticsHttpUtil.getRequestServerInfo();
    }
    if (SpStatisticsHttpUtil.pauseRetry) {
      return;
    }
    fetch(`https://${SpStatisticsHttpUtil.requestServerInfo}/serverTime`)
      .then((resp) => {
        resp.text().then((it) => {
          if (it && it.length > 0) {
            SpStatisticsHttpUtil.serverTime = Number(it);
            SpStatisticsHttpUtil.timeDiff = SpStatisticsHttpUtil.serverTime - Date.now();
          }
        });
      })
      .catch((e) => {
        this.handleRequestException();
      });
  }

  private static handleRequestException() {
    if (SpStatisticsHttpUtil.retryCount >= SpStatisticsHttpUtil.retryMaxCount) {
      SpStatisticsHttpUtil.pauseRetry = true;
      if (SpStatisticsHttpUtil.retryRestTimeOut) {
        return;
      }
      SpStatisticsHttpUtil.retryRestTimeOut = true;
      setTimeout(() => {
        SpStatisticsHttpUtil.retryCount = 0;
        SpStatisticsHttpUtil.pauseRetry = false;
        SpStatisticsHttpUtil.retryRestTimeOut = false;
      }, 600000);
    }
    ++SpStatisticsHttpUtil.retryCount;
  }

  static addUserVisitAction(requestUrl: string) {
    // @ts-ignore
    if (window.useWb) {
      return;
    }
    if (SpStatisticsHttpUtil.requestServerInfo === '') {
      SpStatisticsHttpUtil.requestServerInfo = SpStatisticsHttpUtil.getRequestServerInfo();
    }
    if (SpStatisticsHttpUtil.pauseRetry) {
      return;
    }
    let visitId = 0;
    fetch(`https://${SpStatisticsHttpUtil.requestServerInfo}/${requestUrl}`, {
      method: 'post',
      headers: {
        'Content-Type': 'application/json',
      },
    })
      .then((resp) => {
        resp.text().then((it) => {
          let res = JSON.parse(it);
          if (res && res.data) {
            visitId = res.data.accessId;
          }
        });
      })
      .catch((err) => {});
    setTimeout(() => {
      fetch(`https://${SpStatisticsHttpUtil.requestServerInfo}/${requestUrl}`, {
        method: 'post',
        headers: {
          'Content-Type': 'application/json',
        },
        body: JSON.stringify({
          effectiveAccess: true,
          visitId: visitId,
        }),
      })
        .catch((err) => {})
        .then((resp) => {});
    }, 1800000);
  }

  static addOrdinaryVisitAction(requestBody: BurialPointRequestBody) {
    // @ts-ignore
    if (window.useWb) {
      return;
    }
    if (SpStatisticsHttpUtil.requestServerInfo === '') {
      SpStatisticsHttpUtil.requestServerInfo = SpStatisticsHttpUtil.getRequestServerInfo();
    }
    if (SpStatisticsHttpUtil.pauseRetry) {
      return;
    }
    requestBody.ts = SpStatisticsHttpUtil.getCorrectRequestTime();
    if (SpStatisticsHttpUtil.serverTime === 0) {
      return;
    }
    fetch(`https://${SpStatisticsHttpUtil.requestServerInfo}/record`, {
      method: 'post',
      headers: {
        'Content-Type': 'application/json',
      },
      body: JSON.stringify(requestBody),
    })
      .catch((err) => {
        this.handleRequestException();
      })
      .then((resp) => {});
  }

  static getCorrectRequestTime(): number {
    if (SpStatisticsHttpUtil.serverTime === 0) {
      SpStatisticsHttpUtil.getServerTime();
    }
    return Date.now() + SpStatisticsHttpUtil.timeDiff;
  }
}
