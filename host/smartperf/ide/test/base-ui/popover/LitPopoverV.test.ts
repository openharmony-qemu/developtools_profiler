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
import { LitPopover } from '../../../dist/base-ui/popover/LitPopoverV.js';

describe('LitPopoverV Test', () => {
  it('LitPopoverV01', () => {
    let litPopover = new LitPopover();
    expect(litPopover).not.toBeUndefined();
    expect(litPopover).not.toBeNull();
  });
  it('LitPopoverV02', () => {
    let litPopoverV = new LitPopover();
    expect(litPopoverV.visible).toBe('false');
  });
  it('LitPopoverV03', () => {
    let litPopoverV = new LitPopover();
    litPopoverV.visible = true;
    expect(litPopoverV.visible).toBe('true');
  });
  it('LitPopoverV04', () => {
    let litPopoverV = new LitPopover();
    litPopoverV.visible = false;
    expect(litPopoverV.visible).toBe('false');
  });
  it('LitPopoverV05', () => {
    let litPopoverV = new LitPopover();
    expect(litPopoverV.trigger).toBe('hover');
  });
  it('LitPopoverV06', () => {
    let litPopoverV = new LitPopover();
    litPopoverV.trigger = 'click';
    expect(litPopoverV.trigger).toBe('click');
  });

  it('LitPopoverV07', () => {
    let litPopoverV = new LitPopover();
    litPopoverV.title = 'test';
    expect(litPopoverV.title).toBe('test');
  });

  it('LitPopoverV08', () => {
    let litPopoverV = new LitPopover();
    litPopoverV.width = '10px';
    expect(litPopoverV.width).toBe('10px');
  });

  it('LitPopoverV09', () => {
    let litPopoverV = new LitPopover();
    litPopoverV.width = '10px';
    expect(litPopoverV.width).toBe('10px');
  });

  it('LitPopoverV10', () => {
    let litPopoverV = new LitPopover();
    expect(litPopoverV.width).toBe('max-content');
  });

  it('LitPopoverV11', () => {
    let litPopoverV = new LitPopover();
    expect(litPopoverV.haveRadio).toBeNull();
  });

  it('LitPopoverV12', () => {
    document.body.innerHTML = `<lit-popover id="litpop"></lit-popover>`;
    let popver = document.querySelector('#litpop') as LitPopover;
    expect(popver.haveRadio).toBeNull();
  });

  it('LitPopoverV14', () => {
    document.body.innerHTML = `<lit-popover id="litpop"></lit-popover>`;
    let popver = document.querySelector('#litpop') as LitPopover;
    expect(popver.adoptedCallback()).toBeUndefined();
  });
});
