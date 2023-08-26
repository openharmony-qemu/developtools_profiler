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

importScripts('sql-wasm.js', 'TempSql.js');
let conn: any = null;
let encoder = new TextEncoder();
function initIndexedDB() {
  return new Promise((resolve, reject) => {
    let request = indexedDB.open('systrace');
    request.onerror = function (event) {};
    request.onsuccess = function (event) {
      let db = request.result;
      resolve(db);
    };
    request.onupgradeneeded = function (event) {
      // @ts-ignore
      let db = event!.target!.result;
      if (!db.objectStoreNames.contains('connection')) {
        db.createObjectStore('connection', { autoIncrement: true });
      }
    };
  });
}

function readConnection(store: IDBObjectStore) {
  return new Promise((resolve, reject) => {
    let readRequest = store.get(1);
    readRequest.onsuccess = function (event) {
      // @ts-ignore
      resolve(event.target.result);
    };
    readRequest.onerror = function (event) {
      // @ts-ignore
      reject(event.target.result);
    };
  });
}

function deleteConnection(store: IDBObjectStore, id: number) {
  return new Promise((resolve, reject) => {
    let deleteRequest = store.delete(id);
    deleteRequest.onsuccess = function (event) {
      // @ts-ignore
      resolve(event.target.result);
    };
    deleteRequest.onerror = function (event) {
      // @ts-ignore
      reject(event.target.result);
    };
  });
}

let mergedUnitArray = (bufferSliceUint8: Array<Uint8Array>) => {
  let length = 0;
  bufferSliceUint8.forEach((item) => {
    length += item.length;
  });
  let mergedArray = new Uint8Array(length);
  let offset = 0;
  bufferSliceUint8.forEach((item) => {
    mergedArray.set(item, offset);
    offset += item.length;
  });
  return mergedArray;
};

self.onerror = function (error) {};

self.onmessage = async (e: any) => {
  if (e.data.action === 'open') {
    let array = new Uint8Array(e.data.buffer);
    // @ts-ignore
    initSqlJs({ locateFile: (filename) => `${filename}` }).then((SQL: any) => {
      conn = new SQL.Database(array);
      // @ts-ignore
      self.postMessage({ id: e.data.id, ready: true, index: 0 });
      temp_init_sql_list.forEach((item, index) => {
        let r = conn.exec(item);
        // @ts-ignore
        self.postMessage({
          id: e.data.id,
          ready: true,
          index: index + 1,
        });
      });
      // @ts-ignore
      self.postMessage({ id: e.data.id, init: true });
    });
  } else if (e.data.action === 'close') {
  } else if (e.data.action === 'exec' || e.data.action === 'exec-buf') {
    try {
      let action = e.data.action; //: "exec"
      let sql = e.data.sql;
      let params = e.data.params;
      const stmt = conn.prepare(sql);
      stmt.bind(params);
      let res = [];
      while (stmt.step()) {
        //
        res.push(stmt.getAsObject());
      }
      stmt.free();
      // @ts-ignore
      self.postMessage({ id: e.data.id, results: res });
    } catch (err: any) {
      // @ts-ignore
      self.postMessage({
        id: e.data.id,
        results: [],
        error: err.message,
      });
    }
  }
};
