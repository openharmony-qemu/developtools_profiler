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
import { LitSlicer, LitSlicerTrack } from '../../../dist/base-ui/slicer/lit-slicer.js';

describe('slicer Test', () => {
  it('slicerTest01', function () {
    let litSlicer = new LitSlicer();
    expect(litSlicer).not.toBeUndefined();
    expect(litSlicer).not.toBeNull();
  });
  it('slicerTest02', function () {
    let litSlicerTrack = new LitSlicerTrack();
    expect(litSlicerTrack).not.toBeUndefined();
    expect(litSlicerTrack).not.toBeNull();
  });
  it('slicerTest03', function () {
    let litSlicerTrack = new LitSlicerTrack();
    expect(litSlicerTrack.rangeLeft).toBe(200);
  });
  it('slicerTest04', function () {
    let litSlicerTrack = new LitSlicerTrack();
    litSlicerTrack.rangeLeft = true;
    expect(litSlicerTrack.rangeLeft).not.toBeTruthy();
  });
  it('slicerTest05', function () {
    let litSlicerTrack = new LitSlicerTrack();
    expect(litSlicerTrack.rangeRight).toBe(300);
  });
  it('slicerTest09', function () {
    let litSlicerTrack = new LitSlicerTrack();
    litSlicerTrack.rangeRight = false;
    expect(litSlicerTrack.rangeRight).toBeFalsy();
  });
  it('slicerTest06', function () {
    let litSlicer = new LitSlicer();
    expect(litSlicer.attributeChangedCallback()).toBeUndefined();
  });
  it('slicerTest07', function () {
    let litSlicerTrack = new LitSlicerTrack();
    expect(litSlicerTrack.adoptedCallback()).toBeUndefined();
  });
  it('slicerTest08', function () {
    let litSlicerTrack = new LitSlicerTrack();
    expect(litSlicerTrack.attributeChangedCallback()).toBeUndefined();
  });
  it('slicerTest10', function () {
    let litSlicer = new LitSlicer();
    litSlicer.direction = 'h';
    expect(litSlicer.connectedCallback()).toBeUndefined();
  });
  it('slicerTest11', function () {
    let litSlicer = new LitSlicer();
    litSlicer.direction = 'v';
    expect(litSlicer.direction).toBeUndefined();
  });
  it('slicerTest12', function () {
    let litSlicer = new LitSlicer();
    litSlicer.style = 'v';
    expect(litSlicer.style).toBeUndefined();
  });

  it('slicerTest13', function () {
    let litSlicer = new LitSlicer();
    litSlicer.style = 'v';
    document.body.innerHTML = `
        <lit-slicer-track id="slicer_2"></lit-slicer-track>
        <lit-slicer-track id="slicer"></lit-slicer-track>
        `;
    let slice = document.getElementById('slicer') as LitSlicerTrack;
    let line = slice.shadowRoot?.querySelector('#root') as HTMLDivElement;
    let mouseOutEvent: MouseEvent = new MouseEvent('mousedown', <MouseEventInit>{ movementX: 1, movementY: 2 });

    line.dispatchEvent(mouseOutEvent);
    let onmousemove: MouseEvent = new MouseEvent('mousemove', <MouseEventInit>{ movementX: 1, movementY: 2 });
    document.dispatchEvent(onmousemove);
    let onmouseleave: MouseEvent = new MouseEvent('mouseleave', <MouseEventInit>{ movementX: 1, movementY: 2 });
    document.dispatchEvent(onmouseleave);
    let onmouseup: MouseEvent = new MouseEvent('mouseup', <MouseEventInit>{
      movementX: 1,
      movementY: 2,
    });
    document.dispatchEvent(onmouseup);
    expect(litSlicer.style).toBeUndefined();
  });
});
