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
import { LitPopover } from '../../../dist/base-ui/popover/LitPopover.js';

describe('LitPopover Test', () => {
  it('LitPopover01', () => {
    let litPopover = new LitPopover();
    expect(litPopover).not.toBeUndefined();
    expect(litPopover).not.toBeNull();
  });

  it('LitPopover02', () => {
    let litPopover = new LitPopover();
    expect(litPopover.open).toBeFalsy();
  });

  it('LitPopover03', () => {
    let litPopover = new LitPopover();
    litPopover.open = true;
    expect(litPopover.open).toBeTruthy();
  });

  it('LitPopover04', () => {
    let litPopover = new LitPopover();
    litPopover.open = false;
    expect(litPopover.open).toBeFalsy();
  });

  it('LitPopover05', () => {
    let litPopover = new LitPopover();
    litPopover.direction = 'topleft';
    expect(litPopover.direction).toEqual('topleft');
  });

  it('LitPopover06', () => {
    let litPopover = new LitPopover();
    expect(litPopover.direction).toEqual('topright');
  });

  it('LitPopover07', () => {
    let litPopover = new LitPopover();
    litPopover.type = 'multiple';
    litPopover.dataSource = [
      {
        text: '# Samples',
        isSelected: true,
      },
    ];
    expect(litPopover.select).toEqual(['# Samples']);
  });

  it('LitPopover07', () => {
    let litPopover = new LitPopover();
    litPopover.type = 'radio';
    litPopover.dataSource = [
      {
        text: '# Samples',
        isSelected: true,
      },
    ];
    expect(litPopover.select).toEqual(['# Samples']);
  });

  it('LitPopover08', () => {
    let litPopover = new LitPopover();
    litPopover.type = 'multiple-text';
    litPopover.dataSource = [
      {
        text: '# Samples',
        isSelected: true,
      },
    ];
    expect(litPopover.select).toEqual(['# Samples']);
  });

  it('LitPopover09', () => {
    let litPopover = new LitPopover();
    litPopover.type = 'radio';
    litPopover.title = 'tee';
    litPopover.dataSource = [
      {
        text: '# Samples',
        isSelected: true,
      },
    ];
    expect(litPopover.select).toEqual(['# Samples']);
  });

  it('LitPopover10', () => {
    let litPopover = new LitPopover();
    litPopover.type = 'multiple-text';
    litPopover.title = 'tee';
    litPopover.dataSource = [
      {
        text: '# Samples',
        isSelected: true,
      },
    ];
    expect(litPopover.trigger).not.toBeUndefined();
  });

  it('LitPopover11', () => {
    let litPopover = new LitPopover();
    litPopover.type = 'multiple-text';
    litPopover.title = 'tee';
    litPopover.dataSource = [
      {
        text: '# Samples',
        isSelected: false,
      },
    ];
    expect(litPopover.limit).toEqual({
      textLowerLimit: '0',
      textUpperLimit: '∞',
    });
  });

  it('LitPopover14', () => {
    let litPopover = new LitPopover();
    litPopover.type = 'data-ming';
    litPopover.title = 'tee';
    litPopover.dataSource = [
      {
        text: '# Samples',
        isSelected: false,
      },
    ];
    expect(litPopover.limit).toEqual({
      textLowerLimit: '',
      textUpperLimit: '',
    });
  });

  it('LitPopover15', () => {
    let litPopover = new LitPopover();
    litPopover.type = 'multiple-text';
    litPopover.title = 'tee';
    litPopover.dataSource = [
      {
        text: '# Samples',
        isSelected: true,
      },
    ];
    expect(litPopover.limit).toEqual({
      textLowerLimit: '0',
      textUpperLimit: '∞',
    });
  });

  it('LitPopover13', () => {
    let litPopover = new LitPopover();
    expect(litPopover.connectedCallback()).toBeUndefined();
  });

  it('LitPopover16', () => {
    const onclick = jest.fn();
    let litPopover = (document.body.innerHTML = `
            <lit-popover id='popover'></lit-popover>
        `);
    const popover = document.getElementById('popover');
    expect(onclick).not.toBeCalled();
    popover!.onclick = onclick;
    popover!.click();
    expect(onclick).toBeCalled();
    expect(onclick).toHaveBeenCalledTimes(1);
  });
});
