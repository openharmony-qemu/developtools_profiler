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
import {RangeRuler} from "../../../../../dist/trace/component/trace/timer-shaft/RangeRuler.js"
// @ts-ignore
import {Mark} from "../../../../../dist/trace/component/trace/timer-shaft/RangeRuler.js";
import {TimerShaftElement} from "../../../../../src/trace/component/trace/TimerShaftElement";

describe('RangeRuler Test', () => {
    const canvas = document.createElement('canvas');
    canvas.width = 1;
    canvas.height = 1;
    const ctx = canvas.getContext('2d');

    document.body.innerHTML = '<timer-shaft-element id="timerShaftEL"><timer-shaft-element>'

    let timerShaftElement = document.querySelector('#timerShaftEL') as TimerShaftElement;

    let rangeRuler = new RangeRuler(timerShaftElement, {
        x: 20,
        y: 20,
        width: 100,
        height: 100
    }, {
        startX: 10,
        endX: 30
    }, () => {
    });
    let mark = new Mark(canvas, ctx, '', {
        x: 20,
        y: 20,
        width: 100,
        height: 100
    });

    rangeRuler.cpuUsage = ([{
        cpu: 1,
        ro: 2,
        rate: 2
    }])

    mark.isHover = true

    it('RangeRulerTest01', function () {
        expect(rangeRuler.drawCpuUsage()).toBeUndefined();
    });

    it('RangeRulerTest02', function () {
        expect(rangeRuler.fillX()).toBeUndefined();
    });

    it('RangeRulerTest21', function () {
        rangeRuler.range.startNS =-1
        expect(rangeRuler.fillX()).toBe(undefined);
    });

    it('RangeRulerTest22', function () {
        rangeRuler.range.endNS =-1
        expect(rangeRuler.fillX()).toBe(undefined);
    });

    it('RangeRulerTest23', function () {
        rangeRuler.range.endNS =-1;
        rangeRuler.range.totalNS =-2
        expect(rangeRuler.fillX()).toBe(undefined);
    });

    it('RangeRulerTest24', function () {
        rangeRuler.range.startNS =-1;
        rangeRuler.range.totalNS =-2
        expect(rangeRuler.fillX()).toBe(undefined);
    });

    it('RangeRulerTest03', function () {
        expect(rangeRuler.keyPress({
            key: 'w'
        })).toBeUndefined();
    });

    it('RangeRulerTest04', function () {
        expect(rangeRuler.keyPress({
            key: 's'
        })).toBeUndefined();
    });

    it('RangeRulerTest05', function () {
        expect(rangeRuler.keyPress({
            key: 'a'
        })).toBeUndefined();
    });

    it('RangeRulerTest06', function () {
        expect(rangeRuler.keyPress({
            key: 'd'
        })).toBeUndefined();
    });

    it('RangeRulerTest07', function () {
        expect(rangeRuler.keyUp({
            key: 'w'
        })).toBeUndefined();
    });

    it('RangeRulerTest08', function () {
        expect(rangeRuler.keyUp({
            key: 's'
        })).toBeUndefined();
    });

    it('RangeRulerTest09', function () {
        expect(rangeRuler.keyUp({
            key: 'a'
        })).toBeUndefined();
    });

    it('RangeRulerTest10', function () {
        expect(rangeRuler.keyUp({
            key: 'd'
        })).toBeUndefined();
    });

    it('RangeRulerTest11', function () {
        expect(rangeRuler.mouseUp({
            key: ''
        })).toBeUndefined();
    });

    it('RangeRulerTest12', function () {
        expect(rangeRuler.mouseOut({
            key: ''
        })).toBeUndefined();
    });

    it('RangeRulerTest13', function () {
        rangeRuler.markA = jest.fn(() => true)
        rangeRuler.rangeRect = jest.fn(() => true)
        rangeRuler.rangeRect.containsWithPadding = jest.fn(() => true)

        rangeRuler.markA = jest.fn(() => {
            return {
                frame: {
                    x: 20
                }
            }
        })
        rangeRuler.markA.isHover = jest.fn(() => true)
        rangeRuler.markA.frame = jest.fn(() => [])
        rangeRuler.markA.frame.x = jest.fn(() => true)

        expect(rangeRuler.mouseDown({
            key: ''
        })).toBeUndefined();
    });

    it('RangeRulerTest14', function () {
        rangeRuler.markA = jest.fn(() => true)
        rangeRuler.rangeRect = jest.fn(() => true)
        rangeRuler.rangeRect.containsWithPadding = jest.fn(() => false)
        rangeRuler.frame = jest.fn(() => false)
        rangeRuler.frame.containsWithMargin = jest.fn(() => true)
        rangeRuler.rangeRect.containsWithMargin = jest.fn(() => false)
        rangeRuler.markB.isHover = jest.fn(() => true)
        rangeRuler.markB.frame = jest.fn(() => true)
        rangeRuler.markB.frame.x = jest.fn(() => true)
        expect(rangeRuler.mouseDown({
            key: ''
        })).toBeUndefined();
    });

    it('RangeRulerTest15', function () {
        rangeRuler.markA = jest.fn(() => true)
        rangeRuler.markA.inspectionFrame = jest.fn(() => true)
        rangeRuler.markA.inspectionFrame.contains = jest.fn(() => true)
        rangeRuler.markA.frame = jest.fn(() => true)
        rangeRuler.markA.frame.x = jest.fn(() => true)
        rangeRuler.markA.draw = jest.fn(() => true)
        rangeRuler.centerXPercentage = jest.fn(() => -1)
        expect(rangeRuler.mouseMove({
            key: ''
        })).toBeUndefined();
    });

    it('RangeRulerTest16', () => {
        rangeRuler.markA = jest.fn(() => false)
        rangeRuler.markA.draw = jest.fn(() => true)
        rangeRuler.markA.frame = jest.fn(() => true)
        rangeRuler.markA.frame.x = jest.fn(() => true)
        rangeRuler.markA.inspectionFrame = jest.fn(() => false)
        rangeRuler.markA.inspectionFrame.contains = jest.fn(() => false)
        rangeRuler.movingMark = jest.fn(() => false)
        rangeRuler.movingMark.frame = jest.fn(() => false)
        rangeRuler.movingMark.frame.x = jest.fn(() => false)
        rangeRuler.rangeRect = jest.fn(() => true)
        rangeRuler.rangeRect.containsWithPadding = jest.fn(() => true)
        rangeRuler.movingMark.inspectionFrame = jest.fn(() => false)
        rangeRuler.movingMark.inspectionFrame.x = jest.fn(() => false)
        expect(rangeRuler.mouseMove({
            key: ''
        })).toBeUndefined();
    });

    it('RangeRulerTest17', () => {
        rangeRuler.notifyHandler = jest.fn(() => true)
        rangeRuler.movingMark.inspectionFrame.x = jest.fn(() => false)
        // rangeRuler.c = jest.fn(()=>true)
        // rangeRuler.c.clearRect = jest.fn(()=>true)
        rangeRuler.frame = jest.fn(() => true)
        rangeRuler.frame.x = jest.fn(() => true)
        rangeRuler.frame.y = jest.fn(() => true)
        expect(rangeRuler.draw()).toBeUndefined();
    });

    it('RangeRulerTest18', function () {
        expect(mark.isHover).toBeTruthy();
    });
    it('RangeRulerTest19', function () {
        rangeRuler.clearRect = jest.fn(() => true)
        expect(rangeRuler.draw()).toBeUndefined();

    });

    it('RangeRulerTest20', function () {
        rangeRuler.setRangeNS(0, 2000)
        expect(rangeRuler.getRange().startX).toBe(0)

    });

    it('RangeRulerTest25', function () {
        expect(rangeRuler.delayDraw()).toBeUndefined();
    });

})
