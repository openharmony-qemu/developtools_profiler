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
import { Smaps, SmapsTreeObj } from '../../../dist/trace/bean/SmapsStruct.js';

describe('SmapsStruct Test', () => {
    it('SmapsStructTest01', function () {
        let sMaps = new Smaps();
        sMaps = {
            tsNS: -1,
            start_addr: 'start_addr',
            end_addr: 'end_addr',
            permission: 'permission',
            path: 'path',
            size: 0,
            rss: 0,
            pss: 0,
            reside: 0,
            dirty: 0,
            swapper: 0,
            address: 'address',
            type: 'type',
            dirtyStr: 'dirtyStr',
            swapperStr: 'swapperStr',
            rssStr: 'rssStr',
            pssStr: 'pssStr',
            sizeStr: 'sizeStr',
            resideStr: 'resideStr',
        };
        expect(sMaps).not.toBeUndefined();
        expect(sMaps).toMatchInlineSnapshot(
            {
                tsNS: expect.any(Number),
                start_addr: expect.any(String),
                end_addr: expect.any(String),
                permission: expect.any(String),
                path: expect.any(String),
                size: expect.any(Number),
                rss: expect.any(Number),
                pss: expect.any(Number),
                reside: expect.any(Number),
                dirty: expect.any(Number),
                swapper: expect.any(Number),
                address: expect.any(String),
                type: expect.any(String),
                dirtyStr: expect.any(String),
                swapperStr: expect.any(String),
                rssStr: expect.any(String),
                pssStr: expect.any(String),
                sizeStr: expect.any(String),
                resideStr: expect.any(String)
            }, `
{
  "address": Any<String>,
  "dirty": Any<Number>,
  "dirtyStr": Any<String>,
  "end_addr": Any<String>,
  "path": Any<String>,
  "permission": Any<String>,
  "pss": Any<Number>,
  "pssStr": Any<String>,
  "reside": Any<Number>,
  "resideStr": Any<String>,
  "rss": Any<Number>,
  "rssStr": Any<String>,
  "size": Any<Number>,
  "sizeStr": Any<String>,
  "start_addr": Any<String>,
  "swapper": Any<Number>,
  "swapperStr": Any<String>,
  "tsNS": Any<Number>,
  "type": Any<String>,
}
`
        );
    });

    it('SmapsStructTest02', function () {
        let sMapsTreeObj = new SmapsTreeObj();
        sMapsTreeObj = {
            id: 'id',
            pid: 'pid',
            rsspro: 0,
            rssproStr: 'rssproStr',
            type: 'type',
            reg: 0,
            regStr: 'regStr',
            path: 'path',
            rss: 0,
            rssStr: 'rssStr',
            dirty: 0,
            dirtyStr: 'dirtyStr',
            swapper: 0,
            swapperStr: 'swapperStr',
            pss: 0,
            pssStr: 'pssStr',
            size: 0,
            sizeStr: 'sizeStr',
            respro: 0,
            resproStr: 'resproStr',
            children: [],
        };
        expect(sMapsTreeObj).not.toBeUndefined();
        expect(sMapsTreeObj).toMatchInlineSnapshot(
{
  id: expect.any(String),
  pid: expect.any(String),
  rsspro: expect.any(Number),
  rssproStr: expect.any(String),
  type: expect.any(String),
  reg: expect.any(Number),
  regStr: expect.any(String),
  path: expect.any(String),
  rss: expect.any(Number),
  rssStr: expect.any(String),
  dirty: expect.any(Number),
  dirtyStr: expect.any(String),
  swapper: expect.any(Number),
  swapperStr: expect.any(String),
  pss: expect.any(Number),
  pssStr: expect.any(String),
  size: expect.any(Number),
  sizeStr: expect.any(String),
  respro: expect.any(Number),
  resproStr: expect.any(String),
  children: expect.any(Array) }, `
{
  "children": Any<Array>,
  "dirty": Any<Number>,
  "dirtyStr": Any<String>,
  "id": Any<String>,
  "path": Any<String>,
  "pid": Any<String>,
  "pss": Any<Number>,
  "pssStr": Any<String>,
  "reg": Any<Number>,
  "regStr": Any<String>,
  "respro": Any<Number>,
  "resproStr": Any<String>,
  "rss": Any<Number>,
  "rssStr": Any<String>,
  "rsspro": Any<Number>,
  "rssproStr": Any<String>,
  "size": Any<Number>,
  "sizeStr": Any<String>,
  "swapper": Any<Number>,
  "swapperStr": Any<String>,
  "type": Any<String>,
}
`);
    });
});
