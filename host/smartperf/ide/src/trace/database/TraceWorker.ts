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

importScripts('trace_streamer_builtin.js', 'TempSql.js');

let Module: any = null;
let enc = new TextEncoder();
let dec = new TextDecoder();
let arr: Uint8Array | undefined;
let start: number;
const REQ_BUF_SIZE = 4 * 1024 * 1024;
let reqBufferAddr: number = -1;
let bufferSlice: Array<any> = [];
let json: string;

let headUnitArray: Uint8Array | undefined;
let thirdWasmMap = new Map();
let thirdJsonResult = new Map();

let arkTsData: Array<Uint8Array> = [];
let arkTsDataSize: number = 0;

let currentAction: string = '';
let currentActionId: string = '';

function clear() {
  if (Module != null) {
    Module._TraceStreamerReset();
    Module = null;
  }
  if (arr) {
    arr = undefined;
  }
  if (headUnitArray) {
    headUnitArray = undefined;
  }
  if (bufferSlice) {
    bufferSlice.length = 0;
  }
  thirdWasmMap.clear();
  thirdJsonResult.clear();
}

self.addEventListener('unhandledrejection', (err) => {
  self.postMessage({
    id: currentActionId,
    action: currentAction,
    init: false,
    status: false,
    msg: err.reason.message,
  });
});

function initWASM() {
  return new Promise((resolve, reject) => {
    // @ts-ignore
    let wasm = trace_streamer_builtin_wasm;
    Module = wasm({
      locateFile: (s: any) => {
        return s;
      },
      print: (line: any) => {},
      printErr: (line: any) => {},
      onRuntimeInitialized: () => {
        resolve('ok');
      },
      onAbort: () => {
        reject('on abort');
      },
    });
  });
}

function initThirdWASM(wasmFunctionName: string) {
  function callModelFun(functionName: string) {
    let func = eval(functionName);
    return new func({
      locateFile: (s: any) => {
        return s;
      },
      print: (line: any) => {},
      printErr: (line: any) => {},
      onRuntimeInitialized: () => {},
      onAbort: () => {},
    });
  }

  return callModelFun(wasmFunctionName);
}

let merged = () => {
  let length = 0;
  bufferSlice.forEach((item) => {
    length += item.length;
  });
  let mergedArray = new Uint8Array(length);
  let offset = 0;
  bufferSlice.forEach((item) => {
    mergedArray.set(item, offset);
    offset += item.length;
  });
  return mergedArray;
};

let translateJsonString = (str: string): string => {
  return str //   .padding
    .replace(/[\t|\r|\n]/g, '');
};

let convertJSON = () => {
  try {
    let str = dec.decode(arr);
    let jsonArray = [];
    str = str.substring(str.indexOf('\n') + 1);
    if (!str) {
    } else {
      let parse;
      let tansStr: string;
      try {
        tansStr = str.replace(/[\t\r\n]/g, '');
        parse = JSON.parse(tansStr);
      } catch {
        try {
          tansStr = tansStr!.replace(/[^\x20-\x7E]/g, '?'); //匹配乱码字 符，将其转换为？
          parse = JSON.parse(tansStr);
        } catch {
          tansStr = tansStr!.replace(/\\/g, '\\\\');
          parse = JSON.parse(tansStr);
        }
      }
      let columns = parse.columns;
      let values = parse.values;
      for (let i = 0; i < values.length; i++) {
        let obj: any = {};
        for (let j = 0; j < columns.length; j++) {
          obj[columns[j]] = values[i][j];
        }
        jsonArray.push(obj);
      }
    }
    return jsonArray;
  } catch (e) {
    self.postMessage({
      id: currentActionId,
      action: currentAction,
      init: false,
      status: false,
      msg: (e as any).message,
    });
    return [];
  }
};

self.onmessage = async (e: MessageEvent) => {
  currentAction = e.data.action;
  currentActionId = e.data.id;
  if (e.data.action === 'reset') {
    clear();
  } else if (e.data.action === 'open') {
    await initWASM();
    // @ts-ignore
    self.postMessage({
      id: e.data.id,
      action: e.data.action,
      ready: true,
      index: 0,
    });
    let uint8Array = new Uint8Array(e.data.buffer);
    let callback = (heapPtr: number, size: number, isEnd: number) => {
      let out: Uint8Array = Module.HEAPU8.slice(heapPtr, heapPtr + size);
      bufferSlice.push(out);
      if (isEnd == 1) {
        arr = merged();
        bufferSlice.length = 0;
      }
    };
    let fn = Module.addFunction(callback, 'viii');
    reqBufferAddr = Module._Initialize(fn, REQ_BUF_SIZE);
    let parseConfig = e.data.parseConfig;
    if (parseConfig !== '') {
      let parseConfigArray = enc.encode(parseConfig);
      let parseConfigAddr = Module._InitializeParseConfig(1024);
      Module.HEAPU8.set(parseConfigArray, parseConfigAddr);
      Module._TraceStreamerParserConfigEx(parseConfigArray.length);
    }
    let wasmConfigStr = e.data.wasmConfig;
    if (wasmConfigStr != '' && wasmConfigStr.indexOf('WasmFiles') != -1) {
      let wasmConfig = JSON.parse(wasmConfigStr);
      let wasmConfigs = wasmConfig.WasmFiles;
      let itemArray = wasmConfigs.map((item: any) => {
        return item.componentId + ';' + item.pluginName;
      });
      let thirdWasmStr: string = itemArray.join(';');
      let configUintArray = enc.encode(thirdWasmStr + ';');
      Module.HEAPU8.set(configUintArray, reqBufferAddr);
      Module._TraceStreamer_Init_ThirdParty_Config(configUintArray.length);
      let first = true;
      let sendDataCallback = (heapPtr: number, size: number, componentID: number) => {
        if (componentID == 100) {
          if (first) {
            first = false;
            headUnitArray = Module.HEAPU8.slice(heapPtr, heapPtr + size);
          }
          return;
        }
        let configs = wasmConfigs.filter((wasmConfig: any) => {
          return wasmConfig.componentId == componentID;
        });
        if (configs.length > 0) {
          let config = configs[0];
          let model = thirdWasmMap.get(componentID);
          if (model == null && config.componentId == componentID) {
            importScripts(config.wasmJsName);
            let thirdMode = initThirdWASM(config.wasmName);
            let configPluginName = config.pluginName;
            let pluginNameUintArray = enc.encode(configPluginName);
            let pluginNameBuffer = thirdMode._InitPluginName(pluginNameUintArray.length);
            thirdMode.HEAPU8.set(pluginNameUintArray, pluginNameBuffer);
            thirdMode._TraceStreamerGetPluginNameEx(configPluginName.length);
            let thirdQueryDataCallBack = (heapPtr: number, size: number, isEnd: number, isConfig: number) => {
              if (isConfig == 1) {
                let out: Uint8Array = thirdMode.HEAPU8.slice(heapPtr, heapPtr + size);
                thirdJsonResult.set(componentID, {
                  jsonConfig: dec.decode(out),
                  disPlayName: config.disPlayName,
                  pluginName: config.pluginName,
                });
              } else {
                let out: Uint8Array = thirdMode.HEAPU8.slice(heapPtr, heapPtr + size);
                bufferSlice.push(out);
                if (isEnd == 1) {
                  arr = merged();
                  bufferSlice.length = 0;
                }
              }
            };
            let fn = thirdMode.addFunction(thirdQueryDataCallBack, 'viiii');
            let thirdreqBufferAddr = thirdMode._Init(fn, REQ_BUF_SIZE);
            let updateTraceTimeCallBack = (heapPtr: number, size: number) => {
              let out: Uint8Array = thirdMode.HEAPU8.slice(heapPtr, heapPtr + size);
              Module.HEAPU8.set(out, reqBufferAddr);
              Module._UpdateTraceTime(out.length);
            };
            let traceRangeFn = thirdMode.addFunction(updateTraceTimeCallBack, 'vii');
            let mm = thirdMode._InitTraceRange(traceRangeFn, 1024);
            thirdMode._TraceStreamer_In_JsonConfig();
            thirdMode.HEAPU8.set(headUnitArray, thirdreqBufferAddr);
            thirdMode._ParserData(headUnitArray!.length, 100);
            let out: Uint8Array = Module.HEAPU8.slice(heapPtr, heapPtr + size);
            thirdMode.HEAPU8.set(out, thirdreqBufferAddr);
            thirdMode._ParserData(out.length, componentID);
            thirdWasmMap.set(componentID, {
              model: thirdMode,
              bufferAddr: thirdreqBufferAddr,
            });
          } else {
            let mm = model.model;
            let out: Uint8Array = Module.HEAPU8.slice(heapPtr, heapPtr + size);
            mm.HEAPU8.set(out, model.bufferAddr);
            mm._ParserData(out.length, componentID);
          }
        }
      };
      let fn1 = Module.addFunction(sendDataCallback, 'viii');
      let reqBufferAddr1 = Module._TraceStreamer_Set_ThirdParty_DataDealer(fn1, REQ_BUF_SIZE);
    }
    let wrSize = 0;
    let r2 = -1;
    while (wrSize < uint8Array.length) {
      const sliceLen = Math.min(uint8Array.length - wrSize, REQ_BUF_SIZE);
      const dataSlice = uint8Array.subarray(wrSize, wrSize + sliceLen);
      Module.HEAPU8.set(dataSlice, reqBufferAddr);
      wrSize += sliceLen;
      r2 = Module._TraceStreamerParseDataEx(sliceLen);
      if (r2 == -1) {
        break;
      }
    }
    Module._TraceStreamerParseDataOver();
    for (let value of thirdWasmMap.values()) {
      value.model._TraceStreamer_In_ParseDataOver();
    }
    if (r2 == -1) {
      // @ts-ignore
      self.postMessage({
        id: e.data.id,
        action: e.data.action,
        init: false,
        msg: 'parse data error',
      });
      return;
    }
    // @ts-ignore
    temp_init_sql_list.forEach((item, index) => {
      let r = createView(item);
      // @ts-ignore
      self.postMessage({ id: e.data.id, ready: true, index: index + 1 });
    });
    self.postMessage(
      {
        id: e.data.id,
        action: e.data.action,
        init: true,
        msg: 'ok',
        configSqlMap: thirdJsonResult,
        buffer: e.data.buffer,
      },
      // @ts-ignore
      [e.data.buffer]
    );
  } else if (e.data.action === 'exec') {
    query(e.data.name, e.data.sql, e.data.params);
    let jsonArray = convertJSON();
    // @ts-ignore
    self.postMessage({
      id: e.data.id,
      action: e.data.action,
      results: jsonArray,
    });
  } else if (e.data.action == 'exec-buf') {
    query(e.data.name, e.data.sql, e.data.params);
    self.postMessage(
      { id: e.data.id, action: e.data.action, results: arr!.buffer },
      // @ts-ignore
      [arr.buffer]
    );
  } else if (e.data.action.startsWith('exec-sdk')) {
    querySdk(e.data.name, e.data.sql, e.data.params, e.data.action);
    let jsonArray = convertJSON();
    // @ts-ignore
    self.postMessage({
      id: e.data.id,
      action: e.data.action,
      results: jsonArray,
    });
  } else if (e.data.action.startsWith('exec-metric')) {
    queryMetric(e.data.sql);
    let metricResult = dec.decode(arr);
    // @ts-ignore
    self.postMessage({
      id: e.data.id,
      action: e.data.action,
      results: metricResult,
    });
  } else if (e.data.action == 'init-port') {
    let port = e.ports[0];
    port.onmessage = (me) => {
      query(me.data.action, me.data.sql, me.data.params);
      let msg = {
        id: me.data.id,
        action: me.data.action,
        results: arr!.buffer,
      };
      port.postMessage(msg, [arr!.buffer]);
    };
  } else if (e.data.action == 'download-db') {
    let bufferSliceUint: Array<any> = [];
    let mergedUint = () => {
      let length = 0;
      bufferSliceUint.forEach((item) => {
        length += item.length;
      });
      let mergedArray = new Uint8Array(length);
      let offset = 0;
      bufferSliceUint.forEach((item) => {
        mergedArray.set(item, offset);
        offset += item.length;
      });
      return mergedArray;
    };
    let getDownloadDb = (heapPtr: number, size: number, isEnd: number) => {
      let out: Uint8Array = Module.HEAPU8.slice(heapPtr, heapPtr + size);
      bufferSliceUint.push(out);
      if (isEnd == 1) {
        let arr: Uint8Array = mergedUint();
        self.postMessage({
          id: e.data.id,
          action: e.data.action,
          results: arr,
        });
      }
    };
    let fn1 = Module.addFunction(getDownloadDb, 'viii');
    Module._WasmExportDatabase(fn1);
  } else if (e.data.action === 'upload-so') {
    uploadSoActionId = e.data.id;
    let fileList = e.data.params as Array<File>;
    if (fileList) {
      soFileList = fileList;
      uploadFileIndex = 0;
      if (!uploadSoCallbackFn) {
        uploadSoCallbackFn = Module.addFunction(uploadSoCallBack, 'viii');
      }
      uploadSoFile(soFileList[uploadFileIndex]).then();
    }
  } else if (e.data.action === 'cut-file') {
    cutFileByRange(e);
  } else if (e.data.action === 'long_trace') {
    await initWASM();
    let result = {};
    let headArray = e.data.params.headArray;
    let timStamp = e.data.params.timeStamp;
    let allIndexDataList = e.data.params.splitDataList;
    let splitFileInfos = e.data.params.splitFileInfo as Array<{
      fileType: string;
      startIndex: number;
      endIndex: number;
      size: number;
    }>;
    let maxSize = 48 * 1024 * 1024;
    let maxPageNum = headArray.length / 1024;
    let currentPageNum = 0;
    let splitReqBufferAddr: number;
    if (splitFileInfos) {
      let splitFileInfo = splitFileInfos.filter((splitFileInfo) => splitFileInfo.fileType !== 'trace');
      if (splitFileInfo && splitFileInfo.length > 0) {
        let traceFileType: string = '';
        let db = await openDB();
        let newCutFilePageInfo: Map<
          string,
          {
            traceFileType: string;
            dataArray: [{ data: Uint8Array | Array<{ offset: number; size: number }>; dataTypes: string }];
          }
        > = new Map();
        let cutFileCallBack = (heapPtr: number, size: number, dataType: number, isEnd: number) => {
          let key = `${traceFileType}_${currentPageNum}`;
          let out: Uint8Array = Module.HEAPU8.slice(heapPtr, heapPtr + size);
          if (DataTypeEnum.data === dataType) {
            if (traceFileType === 'arkts') {
              arkTsData.push(out);
              arkTsDataSize += size;
            } else {
              if (newCutFilePageInfo.has(key)) {
                let newVar = newCutFilePageInfo.get(key);
                newVar?.dataArray.push({ data: out, dataTypes: 'data' });
              } else {
                newCutFilePageInfo.set(key, {
                  traceFileType: traceFileType,
                  dataArray: [{ data: out, dataTypes: 'data' }],
                });
              }
            }
          } else if (DataTypeEnum.json === dataType) {
            let cutFilePageInfo = newCutFilePageInfo.get(key);
            if (cutFilePageInfo) {
              let jsonStr: string = dec.decode(out);
              let jsonObj = JSON.parse(jsonStr);
              let valueArray: Array<{ offset: number; size: number }> = jsonObj.value;
              cutFilePageInfo.dataArray.push({ data: valueArray, dataTypes: 'json' });
            }
          }
        };
        splitReqBufferAddr = Module._InitializeSplitFile(Module.addFunction(cutFileCallBack, 'viiii'), REQ_BUF_SIZE);
        Module.HEAPU8.set(headArray, splitReqBufferAddr);
        Module._TraceStreamerGetLongTraceTimeSnapEx(headArray.length);
        for (let fileIndex = 0; fileIndex < splitFileInfo.length; fileIndex++) {
          let fileInfo = splitFileInfo[fileIndex];
          traceFileType = fileInfo.fileType;
          for (let pageNum = 0; pageNum < maxPageNum; pageNum++) {
            currentPageNum = pageNum;
            await splitFileAndSave(
              timStamp,
              fileInfo.fileType,
              fileInfo.startIndex,
              fileInfo.endIndex,
              fileInfo.size,
              db,
              pageNum,
              maxSize,
              splitReqBufferAddr
            );
            await initWASM();
            splitReqBufferAddr = Module._InitializeSplitFile(
              Module.addFunction(cutFileCallBack, 'viiii'),
              REQ_BUF_SIZE
            );
            Module.HEAPU8.set(headArray, splitReqBufferAddr);
            Module._TraceStreamerGetLongTraceTimeSnapEx(headArray.length);
          }
        }
        for (const [fileTypePageNum, fileMessage] of newCutFilePageInfo) {
          let fileTypePageNumArr = fileTypePageNum.split('_');
          let fileType = fileTypePageNumArr[0];
          let pageNum = Number(fileTypePageNumArr[1]);
          let saveIndex = 0;
          let saveStartOffset = 0;
          let dataArray = fileMessage.dataArray;
          let currentChunk = new Uint8Array(maxSize);
          let currentChunkOffset = 0;
          for (let fileDataIndex = 0; fileDataIndex < dataArray.length; fileDataIndex++) {
            let receiveData = dataArray[fileDataIndex];
            if (receiveData.dataTypes === 'data') {
              let receiveDataArray = receiveData.data as Uint8Array;
              if (currentChunkOffset + receiveDataArray.length > maxSize) {
                let freeSize = maxSize - currentChunkOffset;
                let freeSaveData = receiveDataArray.slice(0, freeSize);
                currentChunk.set(freeSaveData, currentChunkOffset);
                await addDataToIndexeddb(db, {
                  buf: currentChunk,
                  id: `${fileType}_new_${timStamp}_${pageNum}_${saveIndex}`,
                  fileType: `${fileType}_new`,
                  pageNum: pageNum,
                  startOffset: saveStartOffset,
                  endOffset: saveStartOffset + maxSize,
                  index: saveIndex,
                  timStamp: timStamp,
                });
                saveStartOffset += maxSize;
                saveIndex++;
                currentChunk = new Uint8Array(maxSize);
                let remnantArray = receiveDataArray.slice(freeSize);
                currentChunkOffset = 0;
                currentChunk.set(remnantArray, currentChunkOffset);
                currentChunkOffset += remnantArray.length;
              } else {
                currentChunk.set(receiveDataArray, currentChunkOffset);
                currentChunkOffset += receiveDataArray.length;
              }
            } else {
              if (receiveData.data.length > 0) {
                let needCutMessage = receiveData.data as Array<{ offset: number; size: number }>;
                let startOffset = needCutMessage[0].offset;
                let endOffset =
                  needCutMessage[needCutMessage.length - 1].offset + needCutMessage[needCutMessage.length - 1].size;
                let searchDataInfo = allIndexDataList
                  .filter(
                    (value: {
                      fileType: string;
                      index: number;
                      pageNum: number;
                      startOffsetSize: number;
                      endOffsetSize: number;
                    }) => {
                      return (
                        value.fileType === fileType &&
                        value.startOffsetSize <= endOffset &&
                        value.endOffsetSize >= startOffset
                      );
                    }
                  )
                  .sort((valueA: { startOffsetSize: number }, valueB: { startOffsetSize: number }) => {
                    return valueA.startOffsetSize - valueB.startOffsetSize;
                  });
                let startIndex = searchDataInfo[0].index;
                let endIndex = searchDataInfo[searchDataInfo.length - 1].index;
                let startQueryIndex = startIndex;
                let endQueryIndex = startIndex + 10;
                do {
                  endQueryIndex = Math.min(endQueryIndex, endIndex);
                  let searchCutFilter = searchDataInfo.filter(
                    (value: {
                      fileType: string;
                      index: number;
                      pageNum: number;
                      startOffsetSize: number;
                      endOffsetSize: number;
                    }) => {
                      if (endQueryIndex === startQueryIndex) {
                        return value.index >= startQueryIndex;
                      }
                      return value.index >= startQueryIndex && value.index <= endQueryIndex;
                    }
                  );
                  let minStartOffsetSize = Math.min(
                    ...searchCutFilter.map((item: { startOffsetSize: number }) => item.startOffsetSize)
                  );
                  let maxEndOffsetSize = Math.max(
                    ...searchCutFilter.map((item: { endOffsetSize: number }) => item.endOffsetSize)
                  );
                  let cutUseOffsetObjs = needCutMessage.filter((offseObj) => {
                    return offseObj.offset > minStartOffsetSize && offseObj.offset < maxEndOffsetSize;
                  });
                  let transaction = db.transaction(STORE_NAME, 'readonly');
                  let store = transaction.objectStore(STORE_NAME);
                  let index = store.index('QueryCompleteFile');
                  let range = IDBKeyRange.bound(
                    [timStamp, fileType, 0, startQueryIndex],
                    [timStamp, fileType, 0, endQueryIndex],
                    false,
                    false
                  );
                  const getRequest = index.openCursor(range);
                  let queryAllData = await queryDataFromIndexeddb(getRequest);
                  let mergeData = indexedDataToBufferData(queryAllData);
                  for (let cutOffsetObjIndex = 0; cutOffsetObjIndex < cutUseOffsetObjs.length; cutOffsetObjIndex++) {
                    let cutUseOffsetObj = cutUseOffsetObjs[cutOffsetObjIndex];
                    let endOffset = cutUseOffsetObj.offset + cutUseOffsetObj.size;
                    let sliceData = mergeData.slice(
                      cutUseOffsetObj.offset - minStartOffsetSize,
                      endOffset - minStartOffsetSize
                    );
                    let sliceDataLength = sliceData.length;
                    if (currentChunkOffset + sliceDataLength >= maxSize) {
                      let handleCurrentData = new Uint8Array(currentChunkOffset + sliceDataLength);
                      let freeSaveArray = currentChunk.slice(0, currentChunkOffset);
                      handleCurrentData.set(freeSaveArray, 0);
                      handleCurrentData.set(sliceData, freeSaveArray.length);
                      let newSliceDataLength: number = Math.ceil(handleCurrentData.length / maxSize);
                      for (let newSliceIndex = 0; newSliceIndex < newSliceDataLength; newSliceIndex++) {
                        let newSliceSize = newSliceIndex * maxSize;
                        let number = Math.min(newSliceSize + maxSize, handleCurrentData.length);
                        let saveArray = handleCurrentData.slice(newSliceSize, number);
                        if (newSliceIndex === newSliceDataLength - 1 && number - newSliceSize < maxSize) {
                          currentChunk = new Uint8Array(maxSize);
                          currentChunkOffset = 0;
                          currentChunk.set(saveArray, currentChunkOffset);
                          currentChunkOffset += saveArray.length;
                        } else {
                          await addDataToIndexeddb(db, {
                            buf: saveArray,
                            id: `${fileType}_new_${timStamp}_${pageNum}_${saveIndex}`,
                            fileType: `${fileType}_new`,
                            pageNum: pageNum,
                            startOffset: saveStartOffset,
                            endOffset: saveStartOffset + maxSize,
                            index: saveIndex,
                            timStamp: timStamp,
                          });
                          saveStartOffset += maxSize;
                          saveIndex++;
                        }
                      }
                    } else {
                      currentChunk.set(sliceData, currentChunkOffset);
                      currentChunkOffset += sliceDataLength;
                    }
                  }
                  startQueryIndex += 10;
                  endQueryIndex += 10;
                } while (startQueryIndex <= endIndex && endQueryIndex <= endIndex);
              }
            }
          }
          if (currentChunkOffset !== 0) {
            let freeArray = currentChunk.slice(0, currentChunkOffset);
            await addDataToIndexeddb(db, {
              buf: freeArray,
              id: `${fileType}_new_${timStamp}_${pageNum}_${saveIndex}`,
              fileType: `${fileType}_new`,
              pageNum: pageNum,
              startOffset: saveStartOffset,
              endOffset: saveStartOffset + maxSize,
              index: saveIndex,
              timStamp: timStamp,
            });
            saveStartOffset += maxSize;
            saveIndex++;
          }
        }
      }
    }
    self.postMessage({
      id: e.data.id,
      action: e.data.action,
      results: result,
    });
    return;
  }
};

function indexedDataToBufferData(sourceData: any): Uint8Array {
  let uintArrayLength = 0;
  let uintDataList = sourceData.map((item: any) => {
    let currentBufData = new Uint8Array(item.buf);
    uintArrayLength += currentBufData.length;
    return currentBufData;
  });
  let resultUintArray = new Uint8Array(uintArrayLength);
  let offset = 0;
  uintDataList.forEach((currentArray: Uint8Array) => {
    resultUintArray.set(currentArray, offset);
    offset += currentArray.length;
  });
  return resultUintArray;
}

async function splitFileAndSave(
  timStamp: number,
  fileType: string,
  startIndex: number,
  endIndex: number,
  fileSize: number,
  db: IDBDatabase,
  pageNum: number,
  maxSize: number,
  splitReqBufferAddr?: any
) {
  let queryStartIndex = startIndex;
  let queryEndIndex = startIndex;
  let saveIndex = 0;
  let saveStartOffset = 0;
  let currentChunk = new Uint8Array(maxSize);
  let currentChunkOffset = 0;
  do {
    queryEndIndex = queryStartIndex + 9;
    if (queryEndIndex > endIndex) {
      queryEndIndex = endIndex;
    }
    const transaction = db.transaction(STORE_NAME, 'readonly');
    const store = transaction.objectStore(STORE_NAME);
    const index = store.index('QueryCompleteFile');
    let range = IDBKeyRange.bound([timStamp, fileType, 0, startIndex], [timStamp, fileType, 0, endIndex], false, false);
    const getRequest = index.openCursor(range);
    let res = await queryDataFromIndexeddb(getRequest);
    queryStartIndex = queryEndIndex + 1;
    let resultFileSize = 0;
    for (let i = 0; i < res.length; i++) {
      let arrayBuffer = res[i];
      let uint8Array = new Uint8Array(arrayBuffer.buf);
      let cutFileSize = 0;
      while (cutFileSize < uint8Array.length) {
        const sliceLen = Math.min(uint8Array.length - cutFileSize, REQ_BUF_SIZE);
        const dataSlice = uint8Array.subarray(cutFileSize, cutFileSize + sliceLen);
        Module.HEAPU8.set(dataSlice, splitReqBufferAddr);
        cutFileSize += sliceLen;
        resultFileSize += sliceLen;
        if (resultFileSize >= fileSize) {
          Module._TraceStreamerLongTraceSplitFileEx(sliceLen, 1, pageNum);
        } else {
          Module._TraceStreamerLongTraceSplitFileEx(sliceLen, 0, pageNum);
        }
        if (arkTsDataSize > 0 && fileType === 'arkts') {
          for (let arkTsAllDataIndex = 0; arkTsAllDataIndex < arkTsData.length; arkTsAllDataIndex++) {
            let currentArkTsData = arkTsData[arkTsAllDataIndex];
            let freeSize = maxSize - currentChunkOffset;
            if (currentArkTsData.length > freeSize) {
              let freeSaveData = currentArkTsData.slice(0, freeSize);
              currentChunk.set(freeSaveData, currentChunkOffset);
              await addDataToIndexeddb(db, {
                buf: currentChunk,
                id: `${fileType}_new_${timStamp}_${pageNum}_${saveIndex}`,
                fileType: `${fileType}_new`,
                pageNum: pageNum,
                startOffset: saveStartOffset,
                endOffset: saveStartOffset + maxSize,
                index: saveIndex,
                timStamp: timStamp,
              });
              saveStartOffset += maxSize;
              saveIndex++;
              let remnantData = currentArkTsData.slice(freeSize);
              let remnantDataLength: number = Math.ceil(remnantData.length / maxSize);
              for (let newSliceIndex = 0; newSliceIndex < remnantDataLength; newSliceIndex++) {
                let newSliceSize = newSliceIndex * maxSize;
                let number = Math.min(newSliceSize + maxSize, remnantData.length);
                let saveArray = remnantData.slice(newSliceSize, number);
                if (newSliceIndex === remnantDataLength - 1 && number - newSliceSize < maxSize) {
                  currentChunk = new Uint8Array(maxSize);
                  currentChunkOffset = 0;
                  currentChunk.set(saveArray, currentChunkOffset);
                  currentChunkOffset += saveArray.length;
                } else {
                  await addDataToIndexeddb(db, {
                    buf: saveArray,
                    id: `${fileType}_new_${timStamp}_${pageNum}_${saveIndex}`,
                    fileType: `${fileType}_new`,
                    pageNum: pageNum,
                    startOffset: saveStartOffset,
                    endOffset: saveStartOffset + maxSize,
                    index: saveIndex,
                    timStamp: timStamp,
                  });
                  saveStartOffset += maxSize;
                  saveIndex++;
                }
              }
            } else {
              currentChunk.set(currentArkTsData, currentChunkOffset);
              currentChunkOffset += currentArkTsData.length;
            }
          }
        }
      }
    }
  } while (queryEndIndex < endIndex);
  if (fileType === 'arkts' && currentChunkOffset > 0) {
    let remnantArray = new Uint8Array(currentChunkOffset);
    let remnantChunk = currentChunk.slice(0, currentChunkOffset);
    remnantArray.set(remnantChunk, 0);
    await addDataToIndexeddb(db, {
      buf: remnantArray,
      id: `${fileType}_new_${timStamp}_${pageNum}_${saveIndex}`,
      fileType: `${fileType}_new`,
      pageNum: pageNum,
      startOffset: saveStartOffset,
      endOffset: saveStartOffset + maxSize,
      index: saveIndex,
      timStamp: timStamp,
    });
    arkTsDataSize = 0;
    arkTsData.length = 0;
  }
}

enum DataTypeEnum {
  data,
  json,
}

let uploadSoActionId: string = '';
let uploadFileIndex: number = 0;
let uploadSoCallbackFn: any;
let soFileList: Array<File | null> = [];
const uploadSoFile = async (file: File | null): Promise<void> => {
  if (file) {
    let fileNameBuffer: Uint8Array | null = enc.encode(file.webkitRelativePath);
    let fileNameLength = fileNameBuffer.length;
    let addr = Module._InitFileName(uploadSoCallbackFn, fileNameBuffer.length);
    Module.HEAPU8.set(fileNameBuffer, addr);
    let writeSize = 0;
    let upRes = -1;
    while (writeSize < file.size) {
      let sliceLen = Math.min(file.size - writeSize, REQ_BUF_SIZE);
      let blob: Blob | null = file.slice(writeSize, writeSize + sliceLen);
      let buffer: ArrayBuffer | null = await blob.arrayBuffer();
      let data: Uint8Array | null = new Uint8Array(buffer);
      let size = file.size;
      let lastFile = uploadFileIndex === soFileList.length - 1 ? 1 : 0;
      Module.HEAPU8.set(data, reqBufferAddr);
      writeSize += sliceLen;
      upRes = Module._TraceStreamerDownloadELFEx(size, fileNameLength, sliceLen, lastFile);
      data = null;
      buffer = null;
      blob = null;
    }
    file = null;
    soFileList[uploadFileIndex] = null;
    fileNameBuffer = null;
  }
};

const uploadSoCallBack = (heapPtr: number, size: number, isFinish: number): void => {
  let out: Uint8Array | null = Module.HEAPU8.slice(heapPtr, heapPtr + size);
  if (out) {
    let res = dec.decode(out);
    out = null;
    if (res.includes('file send over')) {
      if (uploadFileIndex < soFileList.length - 1) {
        uploadFileIndex = uploadFileIndex + 1;
        uploadSoFile(soFileList[uploadFileIndex]).then();
      }
    } else {
      soFileList.length = 0;
      self.postMessage({
        id: uploadSoActionId,
        action: 'upload-so',
        results: res.includes('ok') ? 'ok' : 'failed',
      });
    }
  }
};

let splitReqBufferAddr = -1;

enum FileTypeEnum {
  data,
  json,
}

function cutFileBufferByOffSet(out: Uint8Array, uint8Array: Uint8Array) {
  let jsonStr: string = dec.decode(out);
  let jsonObj = JSON.parse(jsonStr);
  let valueArray: Array<{ offset: number; size: number }> = jsonObj.value;
  const sum = valueArray.reduce((total, obj) => total + obj.size, 0);
  let cutBuffer = new Uint8Array(sum);
  let offset = 0;
  valueArray.forEach((item, index) => {
    const dataSlice = uint8Array.subarray(item.offset, item.offset + item.size);
    cutBuffer.set(dataSlice, offset);
    offset += item.size;
  });
  return cutBuffer;
}

function cutFileByRange(e: MessageEvent) {
  let cutLeftTs = e.data.leftTs;
  let cutRightTs = e.data.rightTs;
  let uint8Array = new Uint8Array(e.data.buffer);
  let resultBuffer: Array<any> = [];
  let cutFileCallBack = (heapPtr: number, size: number, fileType: number, isEnd: number) => {
    let out: Uint8Array = Module.HEAPU8.slice(heapPtr, heapPtr + size);
    if (FileTypeEnum.data === fileType) {
      resultBuffer.push(out);
    } else if (FileTypeEnum.json === fileType) {
      let cutBuffer = cutFileBufferByOffSet(out, uint8Array);
      resultBuffer.push(cutBuffer);
    }
    if (isEnd) {
      const cutResultFileLength = resultBuffer.reduce((total, obj) => total + obj.length, 0);
      let cutBuffer = new Uint8Array(cutResultFileLength);
      let offset = 0;
      resultBuffer.forEach((item) => {
        cutBuffer.set(item, offset);
        offset += item.length;
      });
      resultBuffer.length = 0;
      self.postMessage(
        {
          id: e.data.id,
          action: e.data.action,
          cutStatus: true,
          msg: 'split success',
          buffer: e.data.buffer,
          cutBuffer: cutBuffer.buffer,
        },
        // @ts-ignore
        [e.data.buffer, cutBuffer.buffer]
      );
    }
  };
  splitReqBufferAddr = Module._InitializeSplitFile(Module.addFunction(cutFileCallBack, 'viiii'), REQ_BUF_SIZE);
  let cutTimeRange = `${cutLeftTs};${cutRightTs};`;
  let cutTimeRangeBuffer = enc.encode(cutTimeRange);
  Module.HEAPU8.set(cutTimeRangeBuffer, splitReqBufferAddr);
  Module._TraceStreamerSplitFileEx(cutTimeRangeBuffer.length);
  let cutFileSize = 0;
  let receiveFileResult = -1;
  while (cutFileSize < uint8Array.length) {
    const sliceLen = Math.min(uint8Array.length - cutFileSize, REQ_BUF_SIZE);
    const dataSlice = uint8Array.subarray(cutFileSize, cutFileSize + sliceLen);
    Module.HEAPU8.set(dataSlice, splitReqBufferAddr);
    cutFileSize += sliceLen;
    try {
      if (cutFileSize >= uint8Array.length) {
        receiveFileResult = Module._TraceStreamerReciveFileEx(sliceLen, 1);
      } else {
        receiveFileResult = Module._TraceStreamerReciveFileEx(sliceLen, 0);
      }
    } catch (error) {
      self.postMessage(
        {
          id: e.data.id,
          action: e.data.action,
          cutStatus: false,
          msg: 'split failed',
          buffer: e.data.buffer,
        },
        // @ts-ignore
        [e.data.buffer]
      );
    }
  }
}

function createView(sql: string) {
  let array = enc.encode(sql);
  Module.HEAPU8.set(array, reqBufferAddr);
  let res = Module._TraceStreamerSqlOperateEx(array.length);
  return res;
}

function queryJSON(name: string, sql: string, params: any) {
  query(name, sql, params);
  return convertJSON();
}

function query(name: string, sql: string, params: any): void {
  if (params) {
    Reflect.ownKeys(params).forEach((key: any) => {
      if (typeof params[key] === 'string') {
        sql = sql.replace(new RegExp(`\\${key}`, 'g'), `'${params[key]}'`);
      } else {
        sql = sql.replace(new RegExp(`\\${key}`, 'g'), params[key]);
      }
    });
  }
  start = new Date().getTime();
  let sqlUintArray = enc.encode(sql);
  Module.HEAPU8.set(sqlUintArray, reqBufferAddr);
  Module._TraceStreamerSqlQueryEx(sqlUintArray.length);
}

function querySdk(name: string, sql: string, sdkParams: any, action: string) {
  if (sdkParams) {
    Reflect.ownKeys(sdkParams).forEach((key: any) => {
      if (typeof sdkParams[key] === 'string') {
        sql = sql.replace(new RegExp(`\\${key}`, 'g'), `'${sdkParams[key]}'`);
      } else {
        sql = sql.replace(new RegExp(`\\${key}`, 'g'), sdkParams[key]);
      }
    });
  }
  let sqlUintArray = enc.encode(sql);
  let commentId = action.substring(action.lastIndexOf('-') + 1);
  let key = Number(commentId);
  let wasm = thirdWasmMap.get(key);
  if (wasm != undefined) {
    let wasmModel = wasm.model;
    wasmModel.HEAPU8.set(sqlUintArray, wasm.bufferAddr);
    wasmModel._TraceStreamerSqlQueryEx(sqlUintArray.length);
  }
}

function queryMetric(name: string): void {
  start = new Date().getTime();
  let metricArray = enc.encode(name);
  Module.HEAPU8.set(metricArray, reqBufferAddr);
  Module._TraceStreamerSqlMetricsQuery(metricArray.length);
}

const DB_NAME = 'sp';
const DB_VERSION = 1;
const STORE_NAME = 'longTable';

function openDB(): Promise<IDBDatabase> {
  return new Promise((resolve, reject) => {
    const openRequest = indexedDB.open(DB_NAME, DB_VERSION);
    openRequest.onerror = () => reject(openRequest.error);
    openRequest.onsuccess = () => {
      resolve(openRequest.result);
    };
  });
}

function queryDataFromIndexeddb(getRequest: IDBRequest<IDBCursorWithValue | null>): Promise<any> {
  return new Promise((resolve, reject) => {
    let results: any[] = [];
    getRequest.onerror = (event) => {
      // @ts-ignore
      reject(event.target.error);
    };
    getRequest.onsuccess = (event) => {
      // @ts-ignore
      const cursor = event.target!.result;
      if (cursor) {
        results.push(cursor.value);
        cursor['continue']();
      } else {
        // @ts-ignore
        resolve(results);
      }
    };
  });
}

function addDataToIndexeddb(db: IDBDatabase, value: any, key?: IDBValidKey) {
  return new Promise((resolve, reject) => {
    const transaction = db.transaction(STORE_NAME, 'readwrite');
    const objectStore = transaction.objectStore(STORE_NAME);
    const request = objectStore.add(value, key);
    request.onsuccess = function (event) {
      // @ts-ignore
      resolve(event.target.result);
    };
    request.onerror = (event) => {
      reject(event);
    };
  });
}
