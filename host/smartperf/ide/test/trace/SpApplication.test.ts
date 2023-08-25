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
import { SpStatisticsHttpUtil } from '../../dist/statistics/util/SpStatisticsHttpUtil.js';

SpStatisticsHttpUtil.initStatisticsServerConfig = jest.fn(() => true);
SpStatisticsHttpUtil.addUserVisitAction = jest.fn(() => true);

const intersectionObserverMock = () => ({
    observe: () => null,
});
window.IntersectionObserver = jest.fn().mockImplementation(intersectionObserverMock);
// @ts-ignore
import { SpApplication } from '../../dist/trace/SpApplication.js';
// @ts-ignore
window.ResizeObserver = window.ResizeObserver ||
    jest.fn().mockImplementation(() => ({
        disconnect: jest.fn(),
        observe: jest.fn(),
        unobserve: jest.fn(),
    }));

describe('spApplication Test', () => {
    document.body.innerHTML = '<sp-application id="sss"></sp-application>';
    it('spApplicationTest01', function () {
        let spApplication = document.querySelector('#sss') as SpApplication;
        spApplication.dark = true;
        expect(SpApplication.name).toEqual('SpApplication');
    });

    it('spApplicationTest02', function () {
        let spApplication = document.querySelector('#sss') as SpApplication;
        spApplication.dark = false;
        expect(spApplication.dark).toBeFalsy();
    });

    it('spApplicationTest03', function () {
        let element = document.querySelector('#sss') as SpApplication;
        element.vs = true;
        expect(element.vs).toBeTruthy();
    });

    it('spApplicationTest04', function () {
        document.body.innerHTML = '<sp-application id="sss"></sp-application>';
        let ele = document.querySelector('#sss') as SpApplication;
        ele.vs = false;
        expect(ele.vs).toBeFalsy();
    });

    it('spApplicationTest05', function () {
        let element = document.querySelector('#sss') as SpApplication;
        element.server = true;
        expect(element.server).toBeTruthy();
    });

    it('spApplicationTest06', function () {
        let element = document.querySelector('#sss') as SpApplication;
        element.server = false;
        expect(element.server).toBeFalsy();
    });

    it('spApplicationTest07', function () {
        let element = document.querySelector('#sss') as SpApplication;
        element.querySql = true;
        expect(element.querySql).toBeTruthy();
    });

    it('spApplicationTest08', function () {
        let element = document.querySelector('#sss') as SpApplication;
        element.querySql = false;
        expect(element.querySql).toBeFalsy();
    });

    it('spApplicationTest09', function () {
        let element = document.querySelector('#sss') as SpApplication;
        element.search = true;
        expect(element.search).toBeTruthy();
    });

    it('spApplicationTest10', function () {
        let element = document.querySelector('#sss') as SpApplication;
        element.search = false;
        expect(element.search).toBeFalsy();
    });

    it('spApplicationTest11', function () {
        let element = document.querySelector('#sss') as SpApplication;
        expect(element.removeSkinListener([])).toBeUndefined();
    });

    it('spApplicationTest15', function () {
        let spApplication = document.querySelector('#sss') as SpApplication;
        expect(spApplication.freshMenuDisable()).toBeUndefined();
    });

    it('spApplicationTest16', function () {
        let spApplication = document.querySelector('#sss') as SpApplication;
        expect(spApplication.addSkinListener()).toBeUndefined();
    });

    it('spApplicationTest17', function () {
        let spApplication = document.querySelector('#sss') as SpApplication;
        expect(spApplication.removeSkinListener()).toBeUndefined();
    });

    it('spApplicationTest18', function () {
        let element = document.querySelector('#sss') as SpApplication;
        element.dispatchEvent(new Event('dragleave'));
    });

    it('spApplicationTest19', function () {
        let element = document.querySelector('#sss') as SpApplication;
        element.dispatchEvent(new Event('drop'));
        SpApplication.removeSkinListener = jest.fn(() => undefined);
        expect(element.removeSkinListener({})).toBeUndefined();
    });
    it('spApplicationTest21', function () {
        let element = document.querySelector('#sss') as SpApplication;
        expect(element.vsDownload()).toBeUndefined();
    });

    it('spApplicationTest22', function () {
        let spApplication = document.querySelector('#sss') as SpApplication;
        spApplication.showConten = false;
        expect(spApplication.showContent).toBeFalsy();
    });

    it('spApplicationTest26', function () {
        let spApplication = document.querySelector('#sss') as SpApplication;
        spApplication.dark = false;
        spApplication.skinChangeArray = ['value'];
        expect(spApplication.dark).toBeFalsy();
    });

    it('spApplicationTest27', function () {
        document.body.innerHTML = '<sp-application id="ss"></sp-application>';
        let spApplication = document.querySelector('#ss') as SpApplication;
        spApplication.dark = true;
        spApplication.skinChange = jest.fn(() => true);
        expect(spApplication.dark).toBeTruthy();
    });

    it('spApplicationTest28', function () {
        document.body.innerHTML = '<sp-application id="sp"></sp-application>';
        let spApplication = document.querySelector('#sp') as SpApplication;
        spApplication.dark = false;
        spApplication.skinChange2 = jest.fn(() => true);
        expect(spApplication.dark).toBeFalsy();
    });

    it('spApplicationTest29', function () {
        document.body.innerHTML = '<sp-application id="ap"></sp-application>';
        let spApplication = document.querySelector('#ap') as SpApplication;
        spApplication.querySql = false;
        expect(spApplication.querySql).toBeFalsy();
    });
});
