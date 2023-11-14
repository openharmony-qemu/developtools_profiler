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

import { HdcClient } from './hdcclient/HdcClient.js';
import { UsbTransmissionChannel } from './transmission/UsbTransmissionChannel.js';
import { HDC_DEVICE_FILTERS } from './common/ConstantType.js';
import { FormatCommand } from './hdcclient/FormatCommand.js';
import { log } from '../log/Log.js';
import { HdcStream } from './hdcclient/HdcStream.js';
import { HdcCommand } from './hdcclient/HdcCommand.js';
import { SpRecordTrace } from '../trace/component/SpRecordTrace.js';
import { DataMessage } from './message/DataMessage.js';

export class HdcDeviceManager {
  static escapeCharacterDict = {
    Escape: [27],
    Tab: [9],
    Backspace: [8],
    Enter: [13],
    Insert: [27, 91, 50, 126],
    Home: [27, 91, 49, 126],
    Delete: [27, 91, 51, 126],
    End: [27, 91, 52, 126],
    PageDown: [27, 91, 54, 126],
    PageUp: [27, 91, 53, 126],
    ArrowUp: [27, 91, 65],
    ArrowDown: [27, 91, 66],
    ArrowLeft: [27, 91, 68],
    ArrowRight: [27, 91, 67],
  };
  static ctrlKey = {
    c: [3],
  };
  private static clientList: Map<string, HdcClient> = new Map();
  private static currentHdcClient: HdcClient;
  private static FILE_RECV_PREFIX_STRING = 'hdc file recv -cwd C:\\ ';

  /**
   * getDevices
   */
  // @ts-ignore
  public static async getDevices(): Promise<USBDevice[]> {
    // @ts-ignore
    return navigator.usb.getDevices();
  }

  /**
   * findDevice
   */
  public static findDevice(): Promise<USBDevice> {
    if (!('usb' in navigator)) {
      throw new Error('WebUSB not supported by the browser (requires HTTPS)');
    }
    // @ts-ignore
    return navigator.usb.requestDevice({ filters: HDC_DEVICE_FILTERS });
  }

  /**
   * connect by serialNumber
   *
   * @param serialNumber serialNumber
   */
  public static async connect(serialNumber: string): Promise<boolean> {
    const client = this.clientList.get(serialNumber);
    if (client) {
      if (client.usbDevice!.opened) {
        log('device Usb is Open');
        return true;
      } else {
        if (SpRecordTrace.serialNumber === serialNumber) {
          SpRecordTrace.serialNumber = '';
        }
        log('device Usb not Open');
        return false;
      }
    } else {
      const connectDevice = await this.getDeviceBySerialNumber(serialNumber);
      const usbChannel = await UsbTransmissionChannel.openHdcDevice(connectDevice);
      if (usbChannel) {
        const hdcClient = new HdcClient(usbChannel, connectDevice);
        const connected = await hdcClient.connectDevice();
        if (connected) {
          this.currentHdcClient = hdcClient;
          this.clientList.set(serialNumber, hdcClient);
        }
        log(`device Usb connected : ${connected}`);
        return connected;
      } else {
        log('device Usb connected failed: ');
        return false;
      }
    }
  }

  // @ts-ignore
  public static async getDeviceBySerialNumber(serialNumber: string): Promise<USBDevice> {
    // @ts-ignore
    const devices = await navigator.usb.getDevices();
    // @ts-ignore
    return devices.find((dev) => dev.serialNumber === serialNumber);
  }

  /**
   * disConnect by serialNumber
   *
   * @param serialNumber
   */
  public static async disConnect(serialNumber: string): Promise<boolean> {
    const hdcClient = this.clientList.get(serialNumber);
    if (hdcClient) {
      await hdcClient.disconnect();
      this.clientList['delete'](serialNumber);
      return true;
    } else {
      return true;
    }
  }

  /**
   * Execute shell on the currently connected device and return the result as a string
   *
   * @param cmd cmd
   */
  public static async shellResultAsString(cmd: string, isSkipResult: boolean): Promise<string> {
    if (this.currentHdcClient) {
      const hdcStream = new HdcStream(this.currentHdcClient, false);
      await hdcStream.DoCommand(cmd);
      let result = '';
      while (true) {
        const dataMessage = await hdcStream.getMessage();
        if (dataMessage.channelClose || isSkipResult) {
          result += dataMessage.getDataToString();
          await hdcStream.DoCommandRemote(new FormatCommand(HdcCommand.CMD_KERNEL_CHANNEL_CLOSE, '0', false));
          log('result is end, close');
          break;
        }
        if (dataMessage.usbHead.sessionId === -1) {
          await hdcStream.closeStream();
          return Promise.resolve('The device is abnormal');
        }
        result += dataMessage.getDataToString();
      }
      await hdcStream.closeStream();
      await hdcStream.DoCommandRemote(new FormatCommand(HdcCommand.CMD_KERNEL_CHANNEL_CLOSE, '0', false));
      return Promise.resolve(result);
    }
    return Promise.reject('not select device');
  }

  /**
   * Execute shell on the currently connected device and return the result as a string
   *
   * @param cmd cmd
   */
  public static async stopHiprofiler(cmd: string): Promise<string> {
    if (this.currentHdcClient) {
      const hdcStream = new HdcStream(this.currentHdcClient, true);
      await hdcStream.DoCommand(cmd);
      let result = '';
      let dataMessage = await hdcStream.getMessage();
      result += dataMessage.getDataToString();
      while (!dataMessage.channelClose) {
        dataMessage = await hdcStream.getMessage();
        result += dataMessage.getDataToString();
      }
      await hdcStream.DoCommandRemote(new FormatCommand(HdcCommand.CMD_KERNEL_CHANNEL_CLOSE, '0', false));
      await hdcStream.closeStream();
      return Promise.resolve(result);
    }
    return Promise.reject('not select device');
  }

  public static startShell(
    resultCallBack: (res: DataMessage) => void
  ): ((keyboardEvent: KeyboardEvent | string) => void | undefined) | undefined {
    if (this.currentHdcClient) {
      const hdcShellStream = new HdcStream(this.currentHdcClient, false);
      this.shellInit(hdcShellStream, resultCallBack);
      return (keyboardEvent: KeyboardEvent | string): void => {
        let code = undefined;
        if (keyboardEvent instanceof KeyboardEvent) {
          const cmd = keyboardEvent.key;
          if (keyboardEvent.shiftKey && keyboardEvent.key.toUpperCase() === 'SHIFT') {
            return;
          } else if (keyboardEvent.metaKey) {
            return;
          } else if (keyboardEvent.ctrlKey) {
            // @ts-ignore
            code = this.ctrlKey[keyboardEvent.key];
            if (!code) {
              return;
            } else {
              const dataArray = new Uint8Array(code);
              hdcShellStream.sendToDaemon(
                new FormatCommand(HdcCommand.CMD_SHELL_DATA, cmd, false),
                dataArray,
                dataArray.length
              );
            }
          } else if (keyboardEvent.altKey) {
            return;
          } else {
            // @ts-ignore
            code = this.escapeCharacterDict[cmd];
            if (code) {
              const dataArray = new Uint8Array(code);
              hdcShellStream.sendToDaemon(
                new FormatCommand(HdcCommand.CMD_SHELL_DATA, cmd, false),
                dataArray,
                dataArray.length
              );
            } else {
              if (cmd.length === 1) {
                hdcShellStream.DoCommand(cmd);
              }
            }
          }
        } else {
          hdcShellStream.DoCommand(HdcDeviceManager.processCommand(keyboardEvent));
        }
      };
    }
  }

  private static processCommand(command: string): string {
    if (command.indexOf('\\') === -1) {
      return command;
    }
    const lines = command.split('\r\n');
    let processedCommand = '';
    lines.forEach((line) => {
      if (line.endsWith('\\')) {
        line = line.slice(0, -1);
        processedCommand += line;
      } else {
        processedCommand += line;
        processedCommand += '\n';
      }
    });
    return processedCommand;
  }

  /**
   * Execute shell on the currently connected device, the result is returned as Blob
   *
   * @param cmd cmd
   */
  public static async shellResultAsBlob(cmd: string, isSkipResult: boolean): Promise<Blob> {
    if (this.currentHdcClient) {
      const hdcStream = new HdcStream(this.currentHdcClient, false);
      log(`cmd is ${cmd}`);
      await hdcStream.DoCommand(cmd);
      let finalBuffer;
      while (true) {
        const dataMessage = await hdcStream.getMessage();
        if (dataMessage.channelClose || isSkipResult) {
          log('result is end, close');
          break;
        }
        const res = dataMessage.getData();
        if (res) {
          if (!finalBuffer) {
            finalBuffer = new Uint8Array(res);
          } else {
            finalBuffer = HdcDeviceManager.appendBuffer(finalBuffer, new Uint8Array(res));
          }
        }
      }
      await hdcStream.closeStream();
      if (finalBuffer) {
        return Promise.resolve(new Blob([finalBuffer]));
      }
      return Promise.resolve(new Blob());
    }
    return Promise.reject('not select device');
  }

  /**
   * Pull the corresponding file from the device side
   *
   * @param filename filename
   */
  // 进度
  public static async fileRecv(filename: string, callBack: (schedule: number) => void): Promise<Blob> {
    let finalBuffer;
    if (this.currentHdcClient) {
      const hdcStream = new HdcStream(this.currentHdcClient, false);
      await hdcStream.DoCommand(`${HdcDeviceManager.FILE_RECV_PREFIX_STRING + filename} ./`);
      if (!finalBuffer && hdcStream.fileSize > 0) {
        finalBuffer = new Uint8Array(hdcStream.fileSize);
        log(`Uint8Array size is ${finalBuffer.byteLength}`);
      }
      let offset = 0;
      while (true) {
        const dataMessage = await hdcStream.getMessage();
        if (dataMessage.channelClose) {
          log('result is end, close');
          break;
        }
        if (dataMessage.commandFlag === HdcCommand.CMD_FILE_FINISH) {
          await hdcStream.DoCommandRemote(new FormatCommand(HdcCommand.CMD_KERNEL_CHANNEL_CLOSE, '', false));
          log('CMD_FILE_FINISH is end, close');
          break;
        }
        const res = dataMessage.getData();
        if (res) {
          const resRS: ArrayBuffer = res.slice(64);
          if (finalBuffer) {
            finalBuffer.set(new Uint8Array(resRS), offset);
            offset += resRS.byteLength;
            callBack(Number(((offset / hdcStream.fileSize) * 100).toFixed(3)));
          }
        }
        if (hdcStream.fileSize !== -1 && offset >= hdcStream.fileSize) {
          callBack(100);
          await hdcStream.DoCommandRemote(new FormatCommand(HdcCommand.CMD_FILE_FINISH, '', false));
        }
      }
    }
    if (finalBuffer) {
      return Promise.resolve(new Blob([finalBuffer]));
    } else {
      return Promise.resolve(new Blob([]));
    }
  }

  private static shellInit(hdcShellStream: HdcStream, resultCallBack: (res: DataMessage) => void): void {
    hdcShellStream.DoCommand('hdc_std shell').then(async () => {
      while (true) {
        const data = await hdcShellStream.getMessage();
        resultCallBack(data);
        if (data.channelClose) {
          const channelClose = new FormatCommand(HdcCommand.CMD_KERNEL_CHANNEL_CLOSE, '0', false);
          hdcShellStream.DoCommandRemote(channelClose).then(() => {
            hdcShellStream.closeStream();
          });
          return;
        }
      }
    });
  }

  /**
   * appendBuffer
   *
   * @param buffer1 firstBuffer
   * @param buffer2 secondBuffer
   * @private
   */
  private static appendBuffer(buffer1: Uint8Array, buffer2: Uint8Array): Uint8Array {
    const tmp = new Uint8Array(buffer1.byteLength + buffer2.byteLength);
    tmp.set(buffer1, 0);
    tmp.set(buffer2, buffer1.byteLength);
    return tmp;
  }
}
