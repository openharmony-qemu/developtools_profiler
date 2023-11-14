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

import { TransmissionInterface } from './TransmissionInterface.js';
import { info } from '../../log/Log.js';
import { HDC_DEVICE_FILTER } from '../common/ConstantType.js';

export interface matchingUsbDevice {
  configurationValue: number;
  interfaceNumber: number;
  // @ts-ignore
  endPoints: USBEndpoint[];
}

export class UsbTransmissionChannel implements TransmissionInterface {
  // @ts-ignore
  private _device: USBDevice | null;
  private readonly endpointIn: number;
  private readonly endpointOut: number;
  private readonly interfaceNumber: number;

  private constructor(
    // @ts-ignore
    device: USBDevice,
    endpointIn: number,
    endpointOut: number,
    interfaceNumber: number
  ) {
    this._device = device;
    this.endpointIn = endpointIn;
    this.endpointOut = endpointOut;
    this.interfaceNumber = interfaceNumber;
  }

  /**
   * Send data to the device side
   *
   * @param writeData writeData
   */
  async writeData(writeData: ArrayBuffer): Promise<void> {
    await this._device?.transferOut(this.endpointOut, writeData);
  }

  /**
   * read data from device via usb
   *
   * @param length
   */
  async readData(length: number): Promise<DataView> {
    const result = await this._device?.transferIn(this.endpointOut, length);
    if (result?.data) {
      return result.data;
    } else {
      return Promise.resolve(new DataView(new ArrayBuffer(0)));
    }
  }

  /**
   * Close the device connection
   */
  async close(): Promise<void> {
    try {
      await this._device?.releaseInterface(this.interfaceNumber);
      await this._device?.close();
    } catch (e) {}
    this._device = null;
  }

  /**
   * 打开设备
   *
   * @param usbDevice
   */
  static async openHdcDevice(
    // @ts-ignore
    usbDevice: USBDevice
  ): Promise<UsbTransmissionChannel | null> {
    try {
      await usbDevice.open();
      const matchDevice = this.filterAndFindDevice(usbDevice, HDC_DEVICE_FILTER);
      info('matchDevice is', matchDevice);
      if (!matchDevice) {
        throw new Error('Could not find hdc device');
      }
      await usbDevice.selectConfiguration(matchDevice.configurationValue);
      await usbDevice.claimInterface(matchDevice.interfaceNumber);
      const endpointIn = UsbTransmissionChannel.filterEndpointNumber(matchDevice.endPoints, 'in');
      const endpointOut = UsbTransmissionChannel.filterEndpointNumber(matchDevice.endPoints, 'out');
      return new UsbTransmissionChannel(usbDevice, endpointIn, endpointOut, matchDevice.interfaceNumber);
    } catch (e) {
      return Promise.resolve(null);
    }
  }

  /**
   * Filter out matching devices
   *
   * @param device device
   * @param filter filter
   */
  private static filterAndFindDevice(
    // @ts-ignore
    device: USBDevice,
    // @ts-ignore
    filter: USBDeviceFilter
  ): matchingUsbDevice | null {
    for (const config of device.configurations) {
      for (const intf of config.interfaces)
        for (const al of intf.alternates) {
          if (
            filter.classCode === al.interfaceClass &&
            filter.subclassCode === al.interfaceSubclass &&
            filter.protocolCode === al.interfaceProtocol
          ) {
            return {
              configurationValue: config.configurationValue,
              interfaceNumber: intf.interfaceNumber,
              endPoints: al.endpoints,
            };
          }
        }
    }
    return null;
  }

  private static filterEndpointNumber(
    // @ts-ignore
    usbEndpoints: USBEndpoint[],
    dir: 'in' | 'out',
    type = 'bulk'
  ): number {
    let endpoint = usbEndpoints.filter((element) => {
      return element.direction === dir && element.type === type;
    });
    return endpoint[0].endpointNumber;
  }
}
