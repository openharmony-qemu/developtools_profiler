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
import { LitTabs } from '../../../dist/base-ui/tabs/lit-tabs.js';

window.ResizeObserver =
  window.ResizeObserver ||
  jest.fn().mockImplementation(() => ({
    disconnect: jest.fn(),
    observe: jest.fn(),
    unobserve: jest.fn(),
  }));

describe('LitSwitch Test', () => {
  let litTabs = new LitTabs();

  litTabs.position = 'position';
  litTabs.mode = 'mode';
  litTabs.activekey = 'activekey';

  litTabs.nav = jest.fn(() => {
    let el = document.createElement('div');
    let htmlDivElement = document.createElement('div');
    htmlDivElement.setAttribute('class', "nav-item[data-key='${key}']");

    el.appendChild(htmlDivElement);
    return el;
  });

  LitTabs.nav = jest.fn(() => {
    return document.createElement('div') as HTMLDivElement;
  });

  LitTabs.nav.querySelectorAll = jest.fn(() => {
    return ['fd'];
  });

  it('litTabsTest1', () => {
    expect(litTabs.activekey).toBe('activekey');
  });

  it('litTabsTest01', () => {
    expect(litTabs.onTabClick).toBeUndefined();
  });

  it('litTabsTest02', () => {
    litTabs.nav = jest.fn(() => true);
    litTabs.nav.querySelector = jest.fn(() => {
      return document.createElement('div') as HTMLDivElement;
    });
    litTabs.nav.querySelectorAll = jest.fn(() => true);
    expect(litTabs.updateDisabled('key', 'value')).toBeUndefined();
  });

  it('litTabsTest03', () => {
    litTabs.nav = jest.fn(() => true);
    litTabs.nav.querySelector = jest.fn(() => {
      return document.createElement('div') as HTMLDivElement;
    });
    litTabs.nav.querySelectorAll = jest.fn(() => true);
    expect(litTabs.updateCloseable('key', 'value')).toBeUndefined();
  });

  it('litTabsTest04', () => {
    litTabs.nav = jest.fn(() => true);
    litTabs.nav.querySelector = jest.fn(() => {
      return document.createElement('div') as HTMLDivElement;
    });
    litTabs.nav.querySelectorAll = jest.fn(() => true);

    expect(litTabs.updateHidden('key', 'true')).toBeUndefined();
  });

  it('litTabsTest13', () => {
    litTabs.nav = jest.fn(() => true);
    litTabs.nav.querySelector = jest.fn(() => {
      return document.createElement('div') as HTMLDivElement;
    });
    litTabs.nav.querySelectorAll = jest.fn(() => true);

    expect(litTabs.updateHidden('key', !'true')).toBeUndefined();
  });

  it('litTabsTest05', () => {
    expect(litTabs.initTabPos()).toBeUndefined();
  });

  it('litTabsTest07', () => {
    expect(litTabs.activeByKey(null || undefined)).toBeUndefined();
  });

  it('litTabsTest06', () => {
    expect(litTabs.activePane('Key')).toBeFalsy();
  });

  it('litTabsTest007', () => {
    expect(litTabs.connectedCallback()).toBeUndefined();
  });
  it('litTabsTest8', () => {
    expect(litTabs.attributeChangedCallback('activekey', 'disabled', 'activekey')).toBeUndefined();
  });

  it('litTabsTest9', () => {
    expect(litTabs.adoptedCallback()).toBeUndefined();
  });

  it('litTabsTest09', () => {
    expect(litTabs.disconnectedCallback()).toBeUndefined();
  });
  it('litTabsTest10', () => {
    expect(litTabs.position).toBe('position');
  });

  it('litTabsTest11', () => {
    expect(litTabs.mode).toBe('mode');
  });

  it('litTabsTest14', () => {
    litTabs.nav = jest.fn(() => true);
    litTabs.nav.querySelector = jest.fn(() => {
      return document.createElement('div') as HTMLDivElement;
    });
    litTabs.nav.querySelectorAll = jest.fn(() => true);
    expect(litTabs.updateDisabled('key', undefined)).toBeUndefined();
  });

  it('litTabsTest15', () => {
    litTabs.nav = jest.fn(() => true);
    litTabs.nav.querySelector = jest.fn(() => {
      return document.createElement('div') as HTMLDivElement;
    });
    litTabs.nav.querySelectorAll = jest.fn(() => true);
    expect(litTabs.updateCloseable('key', undefined)).toBeUndefined();
  });

  it('litTabsTest19', () => {
    expect(litTabs.activePane(null)).toBe(false);
  });
});
