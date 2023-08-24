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
import { LitModal } from '../../../dist/base-ui/modal/LitModal.js';

window.ResizeObserver =
  window.ResizeObserver ||
  jest.fn().mockImplementation(() => ({
    disconnect: jest.fn(),
    observe: jest.fn(),
    unobserve: jest.fn(),
  }));

describe('LitModal Test', () => {
  it('LitModalTest01', function () {
    let litModal = new LitModal();
    expect(litModal).not.toBeUndefined();
  });

  it('LitModalTest02', function () {
    let litModal = new LitModal();
    litModal.resizeable = true;
    expect(litModal).not.toBeUndefined();
  });

  it('LitModalTest03', function () {
    let litModal = new LitModal();
    litModal.resizeable = false;
    expect(litModal).not.toBeUndefined();
  });

  it('LitModalTest04', function () {
    let litModal = new LitModal();
    litModal.moveable = false;
    expect(litModal).not.toBeUndefined();
  });

  it('LitModalTest05', function () {
    let litModal = new LitModal();
    litModal.moveable = true;
    expect(litModal).not.toBeUndefined();
  });

  it('LitModalTest06', function () {
    document.body.innerHTML = `
        <div>
            <lit-modal resizeable="true" style='width:100px height:100px ' id='lit-modal'></lit-modal>
        </div> `;
    let litmode = document.getElementById('lit-modal') as LitModal;
    let mouseOutEvent: MouseEvent = new MouseEvent('mousemove', <MouseEventInit>{ movementX: 1, movementY: 2 });
    litmode.dispatchEvent(mouseOutEvent);
  });

  it('LitModalTest06', function () {
    document.body.innerHTML = `
        <div>
            <lit-modal moveable="true" style='width:100px height:100px ' id='lit-modal'></lit-modal>
        </div> `;
    let litmode = document.getElementById('lit-modal') as LitModal;

    let mouseOutEvent: MouseEvent = new MouseEvent('mousedown', <MouseEventInit>{ movementX: 1, movementY: 2 });
    litmode.dispatchEvent(mouseOutEvent);
  });

  it('LitModalTest07', function () {
    document.body.innerHTML = `
        <div>
            <lit-modal moveable="true" style='width:100px height:100px ' id='lit-modal'></lit-modal>
        </div> `;
    let litmode = document.getElementById('lit-modal') as LitModal;

    let mouseOutEvent: MouseEvent = new MouseEvent('mouseleave', <MouseEventInit>{ movementX: 1, movementY: 2 });
    litmode.dispatchEvent(mouseOutEvent);
  });

  it('LitModalTest08', function () {
    document.body.innerHTML = `
        <div>
            <lit-modal moveable="true" style='width:100px height:100px ' id='lit-modal'></lit-modal>
        </div> `;
    let litmode = document.getElementById('lit-modal') as LitModal;
    let mouseOutEvent: MouseEvent = new MouseEvent('mousemove', <MouseEventInit>{ clientX: 1, clientY: 2 });
    litmode.dispatchEvent(mouseOutEvent);
  });
  it('LitModalTest08', function () {
    let litModal = new LitModal();
    litModal.okText = 'ok-text';
    expect(litModal).not.toBeUndefined();
  });
  it('LitModalTest09', function () {
    let litModal = new LitModal();
    litModal.cancelText = 'cancel-text';
    expect(litModal).not.toBeUndefined();
  });
  it('LitModalTest10', function () {
    let litModal = new LitModal();
    litModal.title = 'title';
    expect(litModal).not.toBeUndefined();
  });
  it('LitModalTest11', function () {
    let litModal = new LitModal();
    litModal.visible = false;
    expect(litModal).not.toBeUndefined();
  });
  it('LitModalTest12', function () {
    let litModal = new LitModal();
    litModal.visible = true;
    expect(litModal).not.toBeUndefined();
  });
  it('LitModalTest13', function () {
    let litModal = new LitModal();
    litModal.width = 'width';
    expect(litModal).not.toBeUndefined();
  });
  it('LitModalTest14', function () {
    let litModal = new LitModal();
    expect(litModal.adoptedCallback()).toBeUndefined();
  });
});
