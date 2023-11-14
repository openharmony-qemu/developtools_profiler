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

export class SearchFuncBean {
  depth: number | undefined; // 0
  dur: number | undefined; // 570000
  funName: string | undefined; //"binder transaction"
  id: number | undefined; // 92749
  startTime: number | undefined; // 9729867000
  tid: number | undefined; //
  pid: number | undefined; // 2785
  type: string | undefined;
}

export class SearchThreadProcessBean {
  type: string | undefined;
  rowId: string | undefined | null;
  name: string | undefined;
  rowType: string | undefined | null;
  rowParentId: string | undefined | null;
}

export class SearchSdkBean {
  type: string | undefined;
  rowId: string | undefined | null;
  startTime: number | undefined;
  dur: number | undefined;
  name: string | undefined;
  rowType: string | undefined | null;
  rowParentId: string | undefined | null;
}
