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
export class Smaps {
  startNs: number = -1;
  startAddr: string = '';
  endAddr: string = '';
  address: string = '';
  permission: string = '';
  type: SmapsType = 0;
  typeName: string = '';
  path: string = '';
  size: number = 0;
  sizeStr: string = '';
  count: number = 0;
  rss: number = 0;
  rssStr: string = '';
  pss: number = 0;
  pssStr: string = '';
  sharedClean: number = 0;
  sharedDirty: number = 0;
  privateClean: number = 0;
  privateDirty: number = 0;
  swap: number = 0;
  swapPss: number = 0;
  resideStr: string = '';
}
export class SmapsTreeObj {
  constructor(id: string, pid: string, type: string) {
    this.id = id;
    this.pid = pid;
    this.typeName = type;
  }
  id: string = '';
  pid: string = '';
  typeName: string = '';
  path: any = '';
  size: number = 0;
  sizeStr: string = '';
  sizePro: number = 0;
  sizeProStr: string = '';
  count: number = 0;
  rss: number = 0;
  rssStr: string = '';
  pss: number = 0;
  pssStr: string = '';
  sharedClean: number = 0;
  sharedCleanStr: string = '';
  sharedDirty: number = 0;
  sharedDirtyStr: string = '';
  privateClean: number = 0;
  privateCleanStr: string = '';
  privateDirty: number = 0;
  privateDirtyStr: string = '';
  swap: number = 0;
  swapStr: string = '';
  swapPss: number = 0;
  swapPssStr: string = '';
  children: Array<SmapsTreeObj> = [];
}
export enum SmapsType {
  TYPE_CODE_SYS,
  TYPE_CODE_APP,
  TYPE_DATA_SYS,
  TYPE_DATA_APP,
  TYPE_UNKNOWN_ANON,
  TYPE_STACK,
  TYPE_JS_HEAP,
  TYPE_JAVA_VM,
  TYPE_NATIVE_HEAP,
  TYPE_ASHMEM,
  TYPE_OTHER_SYS,
  TYPE_OTHER_APP,
}
export const TYPE_STRING = [
  'CODE_SYS',
  'CODE_APP',
  'DATA_SYS',
  'DATA_APP',
  'UNKNOWN_ANON',
  'STACK',
  'JS_HEAP',
  'JAVA_VM',
  'NATIVE_HEAP',
  'ASHMEM',
  'OTHER_SYS',
  'OTHER_APP',
];
