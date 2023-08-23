import { info } from '../../log/Log.js';

/**
 * 数据缓存期限
 */
const file_cache_due = 24 * 60 * 60 * 1000;
const db_version = 6;
/**
 * 初始化indexed db
 */
export function initIndexedDB(): Promise<IDBDatabase> {
  return new Promise((resolve, reject) => {
    let request = indexedDB.open('smart_perf', db_version);
    request.onerror = function (event): void {};
    request.onsuccess = function (event): void {
      let db = request.result;
      resolve(db);
    };
    request.onupgradeneeded = function (event): void {
      // @ts-ignore
      let db = event!.target!.result;
      if (db.objectStoreNames.contains('trace_file')) {
        info('delete trace_file table');
        db.deleteObjectStore('trace_file');
      }
      info('create trace_file table');
      let objectStore = db.createObjectStore('trace_file', { keyPath: 'file_index' });
      objectStore.createIndex('file_no', 'file_no');
      objectStore.createIndex('file_id', 'file_id');
      objectStore.createIndex('file_time', 'file_time');
      objectStore.createIndex('file_buffer', 'file_buffer');
    };
  });
}

/**
 * 删除过期数据
 * @param db
 */
export function deleteExpireData(db: IDBDatabase): void {
  if (db && db.objectStoreNames.contains('trace_file')) {
    let objectStore = db.transaction(['trace_file'], 'readwrite').objectStore('trace_file');
    let request = objectStore.getAll();
    request.onsuccess = function (event): void {
      let now = new Date().getTime();
      for (let re of request.result) {
        if (now - re.file_time > file_cache_due) {
          objectStore.delete(re.file_index);
        }
      }
      db.close();
    };
    request.onerror = function (): void {
      info('delete expire data failed');
    };
  }
}

/**
 * 缓存数据
 * @param db 数据库链接对象
 * @param oldFileId 上次打开的文件id
 * @param fileId 当前打开的文件id
 * @param buffer 二进制数据
 */
export function cacheTraceFileBuffer(db: IDBDatabase, oldFileId: string, fileId: string, buffer: ArrayBuffer): void {
  if (db && db.objectStoreNames.contains('trace_file')) {
    let objectStore = db.transaction(['trace_file'], 'readwrite').objectStore('trace_file');
    let request = objectStore.index('file_id').getAll(oldFileId);
    request.onsuccess = function (event): void {
      for (let re of request.result) {
        objectStore.delete(re.file_index);
      }
      info('delete file success');
      let size = buffer.byteLength;
      let index = 0;
      let no = 0;
      let time = new Date().getTime();
      while (index < size) {
        let sliceLen = Math.min(size - index, 4 * 1024 * 1024);
        objectStore.add({
          file_index: randomUUID(),
          file_no: no,
          file_id: fileId,
          file_time: time,
          file_buffer: buffer.slice(index, index + sliceLen),
        });
        no++;
        index += sliceLen;
      }
      info('cache file success', fileId, buffer.byteLength);
      db.close();
    };
    request.onerror = function (ev): void {
      info('delete error', fileId);
    };
  }
}

function randomUUID(): string {
  // @ts-ignore
  return ([1e7] + -1e3 + -4e3 + -8e3 + -1e11).replace(/[018]/g, (c: any) =>
    (c ^ (crypto.getRandomValues(new Uint8Array(1))[0] & (15 >> (c / 4)))).toString(16)
  );
}

/**
 * 获取当前文件的 二进制数据
 * @param fileId
 */
export function getTraceFileBuffer(fileId: string): Promise<ArrayBuffer | null> {
  return new Promise((resolve) => {
    if (fileId === null || fileId === undefined || fileId === '') {
      resolve(new Uint8Array(0).buffer);
    } else {
      initIndexedDB().then((db) => {
        if (db && db.objectStoreNames.contains('trace_file')) {
          let request = db
            .transaction(['trace_file'], 'readwrite')
            .objectStore('trace_file')
            .index('file_id')
            .getAll(fileId);
          request.onsuccess = function (ev): void {
            let totalLen = 0;
            let arr = request.result.sort((a, b) => a.file_no - b.file_no);
            for (let re of arr) {
              totalLen += re.file_buffer.byteLength;
            }
            let buffer = new Uint8Array(totalLen);
            let offset = 0;
            for (let re of arr) {
              let ua = new Uint8Array(re.file_buffer);
              buffer.set(ua, offset);
              offset += re.file_buffer.byteLength;
            }
            arr.length = 0;
            request.result.length = 0;
            resolve(buffer.buffer);
          };
        } else {
          resolve(new Uint8Array(0).buffer);
        }
      });
    }
  });
}
