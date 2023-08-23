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

// @ts-ignore
import { debug, error, info, log, trace, warn, SpLog, LogLevel } from '../../dist/log/Log.js';

describe(' logTest', () => {
  ``;
  it('LogTest01', () => {
    error('111');
  });
  it('LogTest02', () => {
    warn('111');
  });
  it('LogTest03', () => {
    info('111');
  });
  it('LogTest04', () => {
    debug('111');
  });
  it('LogTest05', () => {
    trace('111');
  });
  it('LogTest06', () => {
    log('111');
  });
});
