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

import { BaseStruct } from './BaseStruct.js';

export class JanksStruct extends BaseStruct {
  static hoverJankStruct: JanksStruct | undefined;
  static selectJankStruct: JanksStruct | undefined;
  static selectJankStructList: Array<JanksStruct> = new Array<JanksStruct>();
  static delJankLineFlag: boolean = true;
  id: number | undefined; // sliceid
  ts: number | undefined;
  dur: number | undefined;
  name: string | undefined;
  depth: number | undefined;
  jank_tag: number | undefined;
  cmdline: string | undefined; // process
  jank_type: string | undefined;
  type: string | undefined;
  pid: number | undefined;
  frame_type: string | undefined; // app、renderService、FrameTime
  app_dur: number | undefined;
  src_slice: string | undefined;
  dst_slice: string | undefined;
  rs_ts: number | undefined;
  rs_vsync: string | undefined;
  rs_dur: number | undefined;
  rs_pid: number | undefined;
  rs_name: string | undefined;
  gpu_dur: number | undefined;
}
