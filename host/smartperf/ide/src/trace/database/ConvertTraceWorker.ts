importScripts('trace_converter_builtin.js');
self.onerror = function (error: any) {};

let convertModule: any = null;

function initConvertWASM() {
  return new Promise((resolve, reject) => {
    // @ts-ignore
    let wasm = trace_converter_builtin_wasm;
    convertModule = wasm({
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

const ARRAY_BUF_SIZE = 2 * 1024 * 1024;
self.onmessage = async (e: MessageEvent) => {
  if (e.data.action === 'getConvertData') {
    await initConvertWASM();
    let fileData = e.data.buffer;
    const stepSize = 4 * 1024 * 1024;
    const totalSize = fileData.byteLength;
    // 获取TraceConverter 实例
    let traceInsPtr = convertModule._GetTraceConverterIns();
    // 设置是否为debug模式
    convertModule._SetDebugFlag(false, traceInsPtr);
    let dataHeader = convertModule._malloc(1100);
    let uint8Array = new Uint8Array(fileData.slice(0, 1024));
    convertModule.HEAPU8.set(uint8Array, dataHeader);
    convertModule._SendFileHeader(dataHeader, 1024, traceInsPtr);
    let dataPtr = convertModule._malloc(stepSize);
    // 申请分片内存
    let arrayBufferPtr = convertModule._malloc(ARRAY_BUF_SIZE);
    convertModule._free(dataHeader);
    let bodyDataStr: string[] = [];
    let callback = (heapPtr: number, size: number) => {
      let out = convertModule.HEAPU8.slice(heapPtr, heapPtr + size);
      let dec = new TextDecoder();
      let str = dec.decode(out);
      bodyDataStr.push(str);
    };
    let bodyFn = convertModule.addFunction(callback, 'vii');
    convertModule._SetCallback(bodyFn, traceInsPtr);
    let currentPosition = 1024;
    while (currentPosition < totalSize) {
      let endPosition = Math.min(currentPosition + stepSize, totalSize);
      let currentChunk = new Uint8Array(fileData.slice(currentPosition, endPosition));
      convertModule.HEAPU8.set(currentChunk, dataPtr);
      let leftLen = currentChunk.length;
      let processedLen = 0;
      let blockSize = 0;
      let blockPtr = dataPtr;
      while (leftLen > 0) {
        if (leftLen > ARRAY_BUF_SIZE) {
          blockSize = ARRAY_BUF_SIZE;
        } else {
          blockSize = leftLen;
        }
        let subArrayBuffer = convertModule.HEAPU8.subarray(blockPtr, blockPtr + blockSize);
        convertModule.HEAPU8.set(subArrayBuffer, arrayBufferPtr);
        // 调用分片转换接口
        convertModule._ConvertBlockData(arrayBufferPtr, subArrayBuffer.length, traceInsPtr);
        processedLen = processedLen + blockSize;
        blockPtr = dataPtr + processedLen;
        leftLen = currentChunk.length - processedLen;
      }
      currentPosition = endPosition;
    }

    convertModule._GetRemainingData(traceInsPtr);
    let headerData: string[] = [];
    let headerCallback = (heapPtr: number, size: number) => {
      let out = convertModule.HEAPU8.slice(heapPtr, heapPtr + size);
      let dec = new TextDecoder();
      let str = dec.decode(out);
      headerData.push(str);
    };
    let headerFn = convertModule.addFunction(headerCallback, 'vii');
    convertModule._SetCallback(headerFn, traceInsPtr);
    convertModule._GetFinalHeader(traceInsPtr);
    let allDataStr: string[] = [];
    allDataStr = headerData.concat(bodyDataStr);
    convertModule._ReleaseTraceConverterIns(traceInsPtr); // 释放TraceConverter 实例
    convertModule._free(arrayBufferPtr); //释放分片内存
    convertModule._free(dataPtr);
    self.postMessage(
      {
        id: e.data.id,
        action: 'convert',
        status: true,
        results: new Blob(allDataStr, { type: 'text/plain' }),
        buffer: e.data.buffer,
      },
      // @ts-ignore
      [e.data.buffer!]
    );
  }
};
