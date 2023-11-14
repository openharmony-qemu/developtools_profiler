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
//@ts-ignore
import {PageNation} from '../../../../dist/base-ui/chart/pagenation/PageNation.js';
//@ts-ignore
import {PaginationBox} from '../../../../dist/base-ui/chart/pagenation/pagination-box.js';

// @ts-ignore
window.ResizeObserver = window.ResizeObserver || jest.fn().mockImplementation(() => ({
    disconnect: jest.fn(),
    observe: jest.fn(),
    unobserve: jest.fn(),
}));

describe('PageNation Test', () => {
    it('pageNationTest01', function () {
        document.body.innerHTML = `
        <pagination-box id="pagination-info"></pagination-box> `;
        let pageInfo = document.getElementById('pagination-info') as PaginationBox;
        let pageNation = new PageNation(pageInfo, {
            current: 1,
            totalpage: 9,
            pageSize: 50,
            change(num: number) {},
        });
        expect(pageNation).not.toBeUndefined();
    });

    it('pageNationTest01', function () {
        document.body.innerHTML = `
        <pagination-box id="pagination-info"></pagination-box> `;
        let pageInfo = document.getElementById('pagination-info') as PaginationBox;
        let pageNation = new PageNation(pageInfo, {
            current: 1,
            totalpage: 200,
            pageSize: 50,
            change(num: number) {},
        });
        expect(pageNation.pageInfo.current).toEqual(1);
    });

    it('pageNationTest02', function () {
        document.body.innerHTML = `
        <pagination-box id="pagination-info"></pagination-box> `;
        let pageInfo = document.getElementById('pagination-info') as PaginationBox;
        let pageNation = new PageNation(pageInfo, {
            current: 1,
            totalpage: 1000,
            pageSize: 10,
            change(num: number) {},
        });
        expect(pageNation.pageInfo.pageSize).toEqual(10);
    });

    it('pageNationTest03', function () {
        document.body.innerHTML = `
        <pagination-box id="pagination-info"></pagination-box> `;
        let pageInfo = document.getElementById('pagination-info') as PaginationBox;
        let pageNation = new PageNation(pageInfo, {
            current: 5,
            totalpage: 1000,
            pageSize: 10,
            change(num: number) {},
        });
        expect(pageNation).not.toBeUndefined();
    });

    it('pageNationTest04', function () {
        document.body.innerHTML = `
        <pagination-box id="pagination-info"></pagination-box> `;
        let pageInfo = document.getElementById('pagination-info') as PaginationBox;
        let pageNation = new PageNation(pageInfo, {
            current: 149,
            totalpage: 150,
            pageSize: 10,
            change(num: number) {},
        });
        expect(pageNation.pageInfo.totalpage).toEqual(150);
    });

    it('pageNationTest05', function () {
        document.body.innerHTML = `
        <pagination-box id="pagination-info"></pagination-box> `;
        let pageInfo = document.getElementById('pagination-info') as PaginationBox;
        let pageNation = new PageNation(pageInfo, {
            current: 11,
            totalpage: 15,
            pageSize: 10,
            change(num: number) {},
        });
        expect(pageNation.pageInfo.total).toEqual(100);
    });

    it('pageNationTest06', function () {
        document.body.innerHTML = `
        <pagination-box id="pagination-info"></pagination-box> `;
        let pageInfo = document.getElementById('pagination-info') as PaginationBox;
        let pageNation = new PageNation(pageInfo, {
            current: 1,
            totalpage: 150,
            pageSize: 10,
            change(num: number) {},
        });
        let mouseMoveEvent: MouseEvent = new MouseEvent('click', <MouseEventInit>{ movementX: 1, movementY: 2 });
        pageNation.element.dispatchEvent(mouseMoveEvent);
    });
})