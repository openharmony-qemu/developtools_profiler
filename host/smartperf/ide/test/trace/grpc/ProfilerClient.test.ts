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
import { ProfilerClient } from '../../../dist/trace/grpc/ProfilerClient.js';

describe('HiProfilerClient Test', () => {
  it('HiProfilerClientTest01 ', function () {
    expect(ProfilerClient.client).toBeUndefined();
  });
  it('HiProfilerClientTest02', function () {
    ProfilerClient.client = true;
    expect(ProfilerClient.client).toBeTruthy();
  });
  it('HiProfilerClientTest03 ', function () {
    expect(ProfilerClient.filePaths).toBeUndefined();
  });
  it('HiProfilerClientTest04', function () {
    ProfilerClient.filePaths = true;
    expect(ProfilerClient.filePaths).toBeTruthy();
  });
  it('HiProfilerClientTest05', function () {
    expect(ProfilerClient.profiler_proto).toBeUndefined();
  });
  it('HiProfilerClientTest06', function () {
    ProfilerClient.profiler_proto = true;
    expect(ProfilerClient.profiler_proto).toBeTruthy();
  });

  it('HiProfilerClientTest07 ', function () {
    expect(ProfilerClient.shutdown).toBeUndefined();
  });
  it('HiProfilerClientTest08', function () {
    ProfilerClient.getChannel = jest.fn(() => true);
    expect(ProfilerClient.getChannel()).toBeTruthy();
  });
});
