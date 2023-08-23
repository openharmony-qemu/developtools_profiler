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

import { SpRecordTrace } from '../trace/component/SpRecordTrace.js';
import { CmdConstant } from './CmdConstant.js';
import { HdcDeviceManager } from '../hdc/HdcDeviceManager.js';

export class Cmd {
  static CmdSendPostUtils(uri: string, callback: Function, requestData: any) {
    // @ts-ignore
    if (window.useWb) {
      return;
    }
    fetch(uri, {
      method: 'POST',
      headers: {
        'Content-Type': 'application/json',
      },
      body: JSON.stringify(requestData),
    }).then((response) => {
      if (response.ok) {
        let result = response.text();
        result.then((output) => {
          callback(output);
        });
      }
    });
  }

  /**
   * exec objdump to disassembling binary and find addr to show 100 line
   * @param command  obj dump command
   * @param addr addr of select line
   * @param callback result callback
   */
  static execObjDump(command: string, addr: string, callback: Function) {
    // @ts-ignore
    if (window.useWb) {
      return;
    }
    const data = { cmd: command, addr: addr };
    let uri = `http://${window.location.host.split(':')[0]}:${window.location.port}/exec`;
    Cmd.CmdSendPostUtils(uri, callback, data);
  }

  static execHdcCmd(command: string, callback: Function) {
    // @ts-ignore
    if (window.useWb) {
      return;
    }
    const data = {
      cmd: command,
      tag: 'shell',
    };
    let uri = `http://${window.location.host.split(':')[0]}:${window.location.port}/hdcCmd`;
    Cmd.CmdSendPostUtils(uri, callback, data);
  }

  static async execFileRecv(command: string, filePath: string, callback: Function) {
    // @ts-ignore
    if (window.useWb) {
      return;
    }
    let fileName = filePath.substring(filePath.lastIndexOf('/') + 1);
    const data = {
      cmd: command,
      tag: 'file',
      fileName: fileName,
    };
    let uri = `http://${window.location.host.split(':')[0]}:${window.location.port}/hdcCmd`;
    let buf = await fetch(uri, {
      method: 'POST',
      headers: {
        'Content-Type': 'application/json',
      },
      body: JSON.stringify(data),
    }).then((res) => res.arrayBuffer());
    callback(buf);
  }

  static execHdcTraceCmd(command: string, serialNumber: string, callback: Function) {
    // @ts-ignore
    if (window.useWb) {
      return;
    }
    const data = {
      cmd: command,
      tag: 'hiprofiler_cmd',
      serialNumber: serialNumber,
    };
    let uri = `http://${window.location.host.split(':')[0]}:${window.location.port}/hdcCmd`;
    Cmd.CmdSendPostUtils(uri, callback, data);
  }

  static formatString(string: string, params: string[]) {
    if (params.length === 0) {
      return string;
    }
    for (let i = 0; i < params.length; i++) {
      string = string.replace(new RegExp('\\{' + i + '\\}', 'g'), params[i]);
    }
    return string;
  }

  static showSaveFile(callback: Function) {
    // @ts-ignore
    if (window.useWb) {
      return;
    }
    let uri = `http://${window.location.host.split(':')[0]}:${window.location.port}/showSaveDialog`;
    fetch(uri, {
      method: 'GET',
    }).then((response) => {
      if (response.ok) {
        let result = response.text();
        result.then((output) => {
          callback(output);
        });
      }
    });
  }

  static uploadFile(fd: FormData, callback: Function) {
    // @ts-ignore
    if (window.useWb) {
      return;
    }
    let uri = `http://${window.location.host.split(':')[0]}:${window.location.port}/upload`;
    fetch(uri, {
      method: 'POST',
      body: fd,
    }).then((response) => {
      callback(response);
    });
  }

  static copyFile(fileName: string, distFile: string, callback: Function) {
    // @ts-ignore
    if (window.useWb) {
      return;
    }
    const data = {
      filename: fileName,
      distfile: distFile,
    };
    let uri = `http://${window.location.host.split(':')[0]}:${window.location.port}/copyfile`;
    fetch(uri, {
      method: 'POST',
      headers: {
        'Content-Type': 'application/json',
      },
      body: JSON.stringify(data),
    }).then((response) => {
      callback(response);
    });
  }

  static async openFileDialog() {
    // @ts-ignore
    if (window.useWb) {
      return '';
    }
    let uri = `http://${window.location.host.split(':')[0]}:${window.location.port}/showOpenDialog`;
    let res = await fetch(uri, { method: 'POST' });
    let result = res.ok ? await res.text() : '';
    return result;
  }

  static convertOutProcessList(res: string): string[] {
    let processData: string[] = [];
    if (res) {
      let lineValues: string[] = res.replace(/\r\n/g, '\r').replace(/\n/g, '\r').split(/\r/);
      for (let lineVal of lineValues) {
        lineVal = lineVal.trim();
        if (lineVal.indexOf('__progname') !== -1 || lineVal.indexOf('CMD') !== -1 || lineVal.length === 0) {
          continue;
        } else {
          let process: string[] = lineVal.split(' ');
          if (process.length === 2) {
            processData.push(process[1] + '(' + process[0] + ')');
          }
        }
      }
    }
    return processData;
  }
  static getDebugProcess(): Promise<string[]> {
    return new Promise((resolve, reject) => {
      if (SpRecordTrace.isVscode) {
        let cmd = Cmd.formatString(CmdConstant.CMD_GET_DEBUG_PROCESS_DEVICES, [SpRecordTrace.serialNumber]);
        Cmd.execHdcCmd(cmd, (res: string) => {
          resolve(Cmd.convertOutProcessList(res));
        });
      } else {
        HdcDeviceManager.connect(SpRecordTrace.serialNumber).then((conn) => {
          if (conn) {
            HdcDeviceManager.shellResultAsString(CmdConstant.CMD_GET_DEBUG_PROCESS, false).then((res) => {
              resolve(Cmd.convertOutProcessList(res));
            });
          } else {
            reject(-1);
          }
        });
      }
    });
  }

  static getProcess(): Promise<string[]> {
    return new Promise((resolve, reject) => {
      if (SpRecordTrace.isVscode) {
        let cmd = Cmd.formatString(CmdConstant.CMD_GET_PROCESS_DEVICES, [SpRecordTrace.serialNumber]);
        Cmd.execHdcCmd(cmd, (res: string) => {
          resolve(Cmd.convertOutProcessList(res));
        });
      } else {
        HdcDeviceManager.connect(SpRecordTrace.serialNumber).then((conn) => {
          if (conn) {
            HdcDeviceManager.shellResultAsString(CmdConstant.CMD_GET_PROCESS, false).then((res) => {
              resolve(Cmd.convertOutProcessList(res));
            });
          } else {
            reject(-1);
          }
        });
      }
    });
  }
}
