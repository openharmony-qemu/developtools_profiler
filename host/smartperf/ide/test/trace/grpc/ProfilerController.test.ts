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
import {ClientContainer, SettingRegistry} from "../../../dist/trace/grpc/ProfilerController.js"

describe('HiProfilerClient Test', ()=>{

    let profilerController= new ClientContainer();

    it('ProfilerClientTest01', function () {
        expect(profilerController.port).toBeUndefined();
    });

    it('ProfilerClientTest02', function () {
        profilerController.port = true;
        expect(profilerController.port).toBeTruthy();
    });

    it('ProfilerClientTest03', function () {
        expect(profilerController.host).toBeUndefined();
    });

    it('ProfilerClientTest04', function () {
        profilerController.host = true;
        expect(profilerController.host).toBeTruthy();
    });

    it('ProfilerClientTest06', function () {
        profilerController.loadSettings = jest.fn(()=>true)
        expect(profilerController.start()).toBeUndefined();
    });

    it('ProfilerClientTest07', function () {
        profilerController.loadSettings = jest.fn(()=>true)
        expect(profilerController.loadSettings()).toBeTruthy();
    });
    it('ProfilerClientTest08', function () {
        expect(profilerController. registryClient()).toBeUndefined()
    });
    it('ProfilerClientTest09', function () {
        expect(SettingRegistry. registry()).toBeUndefined()
    });
})