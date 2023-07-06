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

import { WireType } from '../message/WireType.js';
import { SessionHandShake } from '../message/SessionHandShake.js';
import { PayloadProtect } from '../message/PayloadProtect.js';
import { TransferConfig } from '../message/TransferConfig.js';
import { TransferPayload } from '../message/TransferPayload.js';
import { log } from '../../log/Log.js';

export class Serialize {
  static bannerByteLength: number = 8;

  static serializeSessionHandShake(handShake: SessionHandShake): Uint8Array {
    let bannerValue = this.serializeToString(1, handShake.banner);
    let authTypeValue = this.serializeU8(2, handShake.authType);
    let sessionIdValue = this.serializeU32(3, handShake.sessionId);
    let connectKey = this.serializeToString(4, handShake.connectKey);
    let buf = this.serializeToString(5, handShake.buf);
    let mergedArray = new Uint8Array(
      bannerValue.length + authTypeValue.length + sessionIdValue.length + connectKey.length + buf.length
    );
    mergedArray.set(bannerValue);
    mergedArray.set(authTypeValue, bannerValue.length);
    mergedArray.set(sessionIdValue, bannerValue.length + authTypeValue.length);
    mergedArray.set(connectKey, bannerValue.length + authTypeValue.length + sessionIdValue.length);
    mergedArray.set(buf, bannerValue.length + authTypeValue.length + sessionIdValue.length + connectKey.length);
    return mergedArray;
  }

  static serializePayloadProtect(payloadProtect: PayloadProtect): Uint8Array {
    let channelId = this.serializeU32(1, payloadProtect.channelId);
    let commandFlag = this.serializeU32(2, payloadProtect.commandFlag);
    let checkSum = this.serializeU8(3, payloadProtect.checkSum);
    let vCode = this.serializeU8(4, payloadProtect.vCode);
    let mergedArray = new Uint8Array(channelId.length + commandFlag.length + checkSum.length + vCode.length);
    mergedArray.set(channelId);
    mergedArray.set(commandFlag, channelId.length);
    mergedArray.set(checkSum, channelId.length + commandFlag.length);
    mergedArray.set(vCode, channelId.length + commandFlag.length + checkSum.length);
    return mergedArray;
  }

  static serializeTransferConfig(transferConfig: TransferConfig): Uint8Array {
    let fileSizeValue = this.serializeU64(1, transferConfig.fileSize);
    let atimeValue = this.serializeU64(2, transferConfig.atime);
    let mtimeValue = this.serializeU64(3, transferConfig.mtime);
    let optionsValue = this.serializeToString(4, transferConfig.options);
    let pathValue = this.serializeToString(5, transferConfig.path);
    let optionalNameValue = this.serializeToString(6, transferConfig.optionalName);
    let updateIfNewValue = this.serializeU32(7, transferConfig.updateIfNew ? 1 : 0);
    let compressTypeValue = this.serializeU8(8, transferConfig.compressType);
    let holdTimestampValue = this.serializeU32(9, transferConfig.holdTimestamp ? 1 : 0);
    let functionNameValue = this.serializeToString(10, transferConfig.functionName);
    let clientCwdValue = this.serializeToString(11, transferConfig.clientCwd);
    let reserve1Value = this.serializeToString(12, transferConfig.reserve1);
    let reserve2Value = this.serializeToString(13, transferConfig.reserve2);
    let mergedArray = new Uint8Array(
      fileSizeValue.length +
        atimeValue.length +
        mtimeValue.length +
        optionsValue.length +
        pathValue.length +
        optionalNameValue.length +
        updateIfNewValue.length +
        compressTypeValue.length +
        holdTimestampValue.length +
        functionNameValue.length +
        clientCwdValue.length +
        reserve1Value.length +
        reserve2Value.length
    );
    let offset = 0;
    mergedArray.set(fileSizeValue);
    offset += fileSizeValue.length;
    mergedArray.set(atimeValue, offset);
    offset += atimeValue.length;
    mergedArray.set(mtimeValue, offset);
    offset += mtimeValue.length;
    mergedArray.set(optionsValue, offset);
    offset += optionsValue.length;
    mergedArray.set(pathValue, offset);
    offset += pathValue.length;
    mergedArray.set(optionalNameValue, offset);
    offset += optionalNameValue.length;
    mergedArray.set(updateIfNewValue, offset);
    offset += updateIfNewValue.length;
    mergedArray.set(compressTypeValue, offset);
    offset += compressTypeValue.length;
    mergedArray.set(holdTimestampValue, offset);
    offset += holdTimestampValue.length;
    mergedArray.set(functionNameValue, offset);
    offset += functionNameValue.length;
    mergedArray.set(clientCwdValue, offset);
    offset += clientCwdValue.length;
    mergedArray.set(reserve1Value, offset);
    offset += reserve1Value.length;
    mergedArray.set(reserve2Value, offset);
    return mergedArray;
  }

  static serializeTransferPayload(transferPayload: TransferPayload): Uint8Array {
    let indexValue = this.serializeU64(1, transferPayload.index);
    let compressTypeValue = this.serializeU8(2, transferPayload.compressType);
    let compressSizeValue = this.serializeU32(3, transferPayload.compressSize);
    let uncompressSizeValue = this.serializeU32(4, transferPayload.uncompressSize);
    let mergedArray = new Uint8Array(
      indexValue.length + compressTypeValue.length + compressSizeValue.length + uncompressSizeValue.length
    );
    let offset = 0;
    mergedArray.set(indexValue);
    offset += indexValue.length;
    mergedArray.set(compressTypeValue, offset);
    offset += compressTypeValue.length;
    mergedArray.set(compressSizeValue, offset);
    offset += compressSizeValue.length;
    mergedArray.set(uncompressSizeValue, offset);
    return mergedArray;
  }

  static serializeToString(tag: number, value: string): Uint8Array {
    let number = this.makeTagWireType(tag, WireType.LENGTH_DELIMETED);
    let data = this.writeVarIntU32(number);
    let dataLength = this.writeVarIntU32(value.length);
    const encoder = new TextEncoder();
    const buffer = encoder.encode(value);
    let stringValue = new Uint8Array(data.length + dataLength.length + buffer.length);
    stringValue.set(data);
    stringValue.set(dataLength, data.length);
    stringValue.set(buffer, data.length + dataLength.length);
    return stringValue;
  }

  static serializeU8(tag: number, value: number): Uint8Array {
    return this.serializeU32(tag, value);
  }

  static serializeU32(tag: number, value: number): Uint8Array {
    let makeTagWireType = this.writeTagWriteType(tag, WireType.VARINT);
    let writeVarInt = this.writeVarIntU32(value);
    let mergedArray = new Uint8Array(makeTagWireType.length + writeVarInt.length);
    mergedArray.set(makeTagWireType);
    mergedArray.set(writeVarInt, makeTagWireType.length);
    return mergedArray;
  }

  static serializeU64(tag: number, value: number): Uint8Array {
    let makeTagWireType = this.writeTagWriteType(tag, WireType.VARINT);
    let writeVarInt = this.writeVarIntU64(value);
    let mergedArray = new Uint8Array(makeTagWireType.length + writeVarInt.length);
    mergedArray.set(makeTagWireType);
    mergedArray.set(writeVarInt, makeTagWireType.length);
    return mergedArray;
  }

  static writeTagWriteType(tag: number, wireType: number): Uint8Array {
    let number = this.makeTagWireType(tag, wireType);
    return this.writeVarIntU32(number);
  }

  static makeTagWireType(tag: number, wireType: number): number {
    return (tag << 3) | wireType;
  }

  static writeVarIntU64(u64value: number): Uint8Array {
    let uint64buffer: number[] = [];
    for (let index = 0; index < 10; index++) {
      uint64buffer[index] = u64value & 0b01111111;
      u64value = u64value >> 7;
      if (u64value) {
        uint64buffer[index] = uint64buffer[index] | 0b10000000;
      } else {
        break;
      }
    }
    return new Uint8Array(uint64buffer);
  }

  static writeVarIntU32(u32value: number): Uint8Array {
    let u32buffer: number[] = [];
    for (let index = 0; index < 5; index++) {
      u32buffer[index] = u32value & 0b01111111;
      u32value = u32value >> 7;
      if (u32value) {
        u32buffer[index] = u32buffer[index] | 0b10000000;
      } else {
        break;
      }
    }
    return new Uint8Array(u32buffer);
  }

  static parseHandshake(data: Uint8Array): SessionHandShake {
    // banner
    let bannerBuffer = data.buffer;
    let bannerTag = this.getTag(WireType.LENGTH_DELIMETED, new DataView(bannerBuffer));
    let bannerLengthBuf = bannerBuffer.slice(1);
    let bannerSize = this.readVarIntU32(new DataView(bannerLengthBuf));
    let bannerDataBuffer = bannerLengthBuf.slice(1);
    let banner = this.parseString(bannerDataBuffer, bannerSize);

    // authType
    let authBuffer = bannerDataBuffer.slice(bannerSize);
    let authTypeTag = this.getTag(WireType.VARINT, new DataView(authBuffer));
    let authDataBuffer = authBuffer.slice(1);
    let authTypeDataView = new DataView(authDataBuffer);
    let authType = this.parseU8(authTypeDataView, 1);

    // sessionId
    let sessionIdBuffer = authDataBuffer.slice(1);
    let sessionIdTag = this.getTag(WireType.VARINT, new DataView(sessionIdBuffer));
    let sessionDataBuffer = sessionIdBuffer.slice(1);
    let sessionIdDataView = new DataView(sessionDataBuffer);
    let sessionId = this.parseU32(sessionIdDataView, 3);

    // connectKey
    let connectKeyBuffer = sessionDataBuffer.slice(3);
    let connectKeyTag = this.getTag(WireType.LENGTH_DELIMETED, new DataView(connectKeyBuffer));
    let connectLengthBuffer = connectKeyBuffer.slice(1);
    let connectKeyDataView = new DataView(connectLengthBuffer);
    let connectKeySize = this.readVarIntU32(connectKeyDataView);
    let connectDataBuffer = connectLengthBuffer.slice(1);
    let connectKey = this.parseString(connectDataBuffer, connectKeySize);

    // buf
    let bufBuffer = connectDataBuffer.slice(connectKeySize);
    let bufTag = this.getTag(WireType.LENGTH_DELIMETED, new DataView(bufBuffer));
    let lengthBuffer = bufBuffer.slice(1);
    let bufDataView = new DataView(lengthBuffer);
    let bufSize = this.readVarIntU32(bufDataView);
    let dataBuffer = lengthBuffer.slice(1);
    let buf = this.parseString(dataBuffer, bufSize);
    return new SessionHandShake(banner, authType, sessionId, connectKey, buf);
  }

  static parseTransferConfig(data: ArrayBuffer): TransferConfig {
    let uint8Array = new Uint8Array(data);
    // channelId
    let dataBuffer = uint8Array.buffer;
    let channelTag = this.getTag(WireType.VARINT, new DataView(dataBuffer));
    let channelDataBuffer = dataBuffer.slice(1);
    let channelDataView = new DataView(channelDataBuffer);
    let fileSize = this.parseU64(channelDataView, 1);
    return new TransferConfig(fileSize, 0, 0, '', '', '', false, 0, false, '', '', '', '');
  }

  static parsePayloadProtect(data: ArrayBuffer): PayloadProtect {
    let uint8Array = new Uint8Array(data);

    // channelId
    let dataBuffer = uint8Array.buffer;
    let channelTag = this.getTag(WireType.VARINT, new DataView(dataBuffer));
    let channelDataBuffer = dataBuffer.slice(1);
    let channelDataView = new DataView(channelDataBuffer);
    let channelId = this.parseU32(channelDataView, 1);

    // commandFlag
    let commandTagBuffer = channelDataBuffer.slice(1);
    let commandTag = this.getTag(WireType.VARINT, new DataView(commandTagBuffer));
    let commandDataBuffer = commandTagBuffer.slice(1);
    let commandDataView = new DataView(commandDataBuffer);
    let commandFlag = this.parseU32(commandDataView, 1);

    // checkSum
    let checkSumTagBuffer = commandDataBuffer.slice(1);
    let checkSumTag = this.getTag(WireType.VARINT, new DataView(checkSumTagBuffer));
    let checkSumDataBuffer = checkSumTagBuffer.slice(1);
    let checkSumDataView = new DataView(checkSumDataBuffer);
    let checkSum = this.parseU8(checkSumDataView, 1);

    // vCode
    let vCodeTagBuffer = checkSumDataBuffer.slice(1);
    let vCodeTag = this.getTag(WireType.VARINT, new DataView(vCodeTagBuffer));
    let vCodeDataBuffer = vCodeTagBuffer.slice(1);
    let vCodeDataView = new DataView(vCodeDataBuffer);
    let vCode = this.parseU8(vCodeDataView, 1);

    return new PayloadProtect(channelId, commandFlag, checkSum, vCode);
  }

  static parseString(buffer: ArrayBuffer, size: number): string {
    let arrayBuffer = buffer.slice(0, size);
    let textDecoder = new TextDecoder();
    return textDecoder.decode(arrayBuffer);
  }

  static parseU8(dataView: DataView, size: number): number {
    return this.parseU32(dataView, size);
  }

  static parseU32(dataView: DataView, size: number): number {
    return this.readVarIntU32(dataView);
  }

  static parseU64(dataView: DataView, size: number): number {
    return this.readVarIntU64(dataView);
  }

  static readVarIntU32(dataView: DataView): number {
    let u32IntValue: number = 0;
    for (let u32IntIndex = 0; u32IntIndex < 5; u32IntIndex++) {
      if (dataView.byteLength < u32IntIndex + 1) {
        return -1;
      }
      let int8 = dataView.getUint8(u32IntIndex);
      let readValue = int8 & 0b01111111;
      u32IntValue |= readValue << (7 * u32IntIndex);
      if (!(int8 & 0b10000000)) {
        return u32IntValue;
      }
    }
    return -1;
  }

  static readVarIntU64(dataView: DataView): number {
    let u62IntValue: number = 0;
    for (let u62IntIndex = 0; u62IntIndex < 10; u62IntIndex++) {
      if (dataView.byteLength < u62IntIndex + 1) {
        return -1;
      }
      let int8 = dataView.getUint8(u62IntIndex);
      let readValue = int8 & 0b01111111;
      u62IntValue |= readValue << (7 * u62IntIndex);
      if (!(int8 & 0b10000000)) {
        return u62IntValue;
      }
    }
    return -1;
  }

  static readTagWireType(tagKey: number) {
    let wireNewType: WireType = tagKey & 0b0111;
    return tagKey >> 3;
  }

  static getTag(wireType: WireType, dataView: DataView) {
    let tagKey = this.readVarIntU32(dataView);
    return this.readTagWireType(tagKey);
  }
}
