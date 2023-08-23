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
import { HiProfilerClient } from '../../../dist/trace/grpc/HiProfilerClient.js';

describe('HiProfilerClient Test', () => {
  let hiProfilerClient = new HiProfilerClient();
  it('HiProfilerClientTest01', function () {
    expect(hiProfilerClient.address).toBeUndefined();
  });

  it('HiProfilerClientTest02', function () {
    hiProfilerClient.address = true;
    expect(hiProfilerClient.address).toBeTruthy();
  });

  it('HiProfilerClientTest03', function () {
    expect(hiProfilerClient.client).toBeUndefined();
  });

  it('HiProfilerClientTest04', function () {
    hiProfilerClient.client = true;
    expect(hiProfilerClient.client).toBeTruthy();
  });

  it('HiProfilerClientTest05', function () {
    expect(hiProfilerClient.getProfilerClient()).toBeTruthy();
  });
});
