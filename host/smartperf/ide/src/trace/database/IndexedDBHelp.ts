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

export class IndexedDBHelp {
  private dbName: string = '';
  private dbVersion: number = 1;
  private db: IDBDatabase | undefined;

  public open(dbName: string, dbVersion: number, storeOptions?: Array<StoreOptions>): Promise<IDBDatabase> {
    this.dbName = dbName;
    this.dbVersion = dbVersion;
    return new Promise((resolve, reject) => {
      if (this.db && this.db.name === dbName && this.db.version == dbVersion) {
        resolve(this.db);
        return;
      }
      const idbOpenDBRequest = indexedDB.open(dbName, dbVersion);
      idbOpenDBRequest.onupgradeneeded = () => {
        const database: IDBDatabase = idbOpenDBRequest.result;
        this.db = database;
        storeOptions?.forEach((option) => {
          let optionName = option.name;
          if (database.objectStoreNames.contains(optionName) === false) {
            if (option.objectStoreParameters) {
              let objectStore = database.createObjectStore(optionName, option.objectStoreParameters);
              option.dataItems?.forEach((dataItem) => {
                if (dataItem.indexParameters) {
                  objectStore.createIndex(dataItem.name, dataItem.keypath, dataItem.indexParameters);
                } else {
                  objectStore.createIndex(dataItem.name, dataItem.keypath);
                }
              });
            } else {
              let objectStore = database.createObjectStore(optionName);
              option.dataItems?.forEach((dataItem) => {
                if (dataItem.indexParameters) {
                  objectStore.createIndex(dataItem.name, dataItem.name, dataItem.indexParameters);
                } else {
                  objectStore.createIndex(dataItem.name, dataItem.name);
                }
              });
            }
          }
        });
        resolve(database);
      };
      idbOpenDBRequest.onsuccess = (event) => {
        const database: IDBDatabase = idbOpenDBRequest.result;
        this.db = database;
        resolve(database);
      };
      idbOpenDBRequest.onerror = (event) => {
        reject(event);
      };
    });
  }

  private async transaction(storeName: string): Promise<IDBTransaction> {
    if (this.db == undefined) {
      this.db = await this.open(this.dbName, this.dbVersion);
    }
    return this.db.transaction([storeName], 'readwrite');
  }

  public async getObjectStore(storeName: string): Promise<IDBObjectStore> {
    let transaction = await this.transaction(storeName);
    return transaction.objectStore(storeName);
  }

  public get(storeName: string, query: IDBValidKey | IDBKeyRange, queryIndex?: string) {
    return new Promise((resolve, reject) => {
      this.getObjectStore(storeName).then((objectStore: IDBObjectStore) => {
        let request: IDBRequest<any>;
        if (queryIndex) {
          const index = objectStore.index(queryIndex);
          request = index.getAll(query);
        } else {
          request = objectStore.getAll(query);
        }
        request.onsuccess = function (event) {
          // @ts-ignore
          resolve(event.target.result);
        };
        request.onerror = (event) => {
          reject(event);
        };
      });
    });
  }

  public add(storeName: string, value: any, key?: IDBValidKey) {
    return new Promise((resolve, reject) => {
      this.getObjectStore(storeName).then((objectStore: IDBObjectStore) => {
        const request = objectStore.add(value, key);
        request.onsuccess = function (event) {
          // @ts-ignore
          resolve(event.target.result);
        };
        request.onerror = (event) => {
          reject(event);
        };
      });
    });
  }

  public delete(storeName: string, query: IDBValidKey | IDBKeyRange) {
    return new Promise((resolve, reject) => {
      this.getObjectStore(storeName).then((objectStore: IDBObjectStore) => {
        const request = objectStore['delete'](query);
        request.onsuccess = function (event) {
          // @ts-ignore
          resolve(event.target.result);
        };
        request.onerror = (event) => {
          reject(event);
        };
      });
    });
  }

  public put(storeName: string, value: any, key?: IDBValidKey) {
    return new Promise((resolve, reject) => {
      this.getObjectStore(storeName).then((objectStore: IDBObjectStore) => {
        const request = objectStore.put(value, key);
        request.onsuccess = function (event) {
          // @ts-ignore
          resolve(event.target.result);
        };
        request.onerror = (event) => {
          reject(event);
        };
      });
    });
  }
}

export class StoreOptions {
  name: string = '';
  objectStoreParameters?: IDBObjectStoreParameters;
  dataItems?: Array<{ name: string; keypath: string[] | string; indexParameters?: IDBIndexParameters }>;
}
