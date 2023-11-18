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
import { TabPaneHisysEvents } from '../../../../../../dist/trace/component/trace/sheet/hisysevent/TabPaneHisysEvents.js';

jest.mock('../../../../../../dist/trace/component/trace/base/TraceRow.js', () => {
  return {};
});
const sqlit = require('../../../../../../dist/trace/database/SqlLite.js');
jest.mock('../../../../../../dist/trace/database/SqlLite.js');
window.ResizeObserver =
  window.ResizeObserver ||
  jest.fn().mockImplementation(() => ({
    disconnect: jest.fn(),
    observe: jest.fn(),
    unobserve: jest.fn(),
  }));

describe('TabPaneHisysEvents Test', () => {
  it('TabPaneHisysEvents01 ', function () {
    let tabPaneHisysEvents = new TabPaneHisysEvents();
    let MockRealTime = sqlit.queryRealTime;
    let Realtime = [{
      ts: 1000,
      clock_name: '',
    }];
    let tabData = {
      hiSysEvents: [{
        id: 1,
        domain: 'DISPLAY',
        eventName: 'AMBIENT_LIGHT',
        eventType: 2,
        ts: 2797000000,
        tz: '23925',
        pid: 23925,
        tid: 24086,
        uid: 5523,
        info: '',
        level: 'MINOR',
        seq: '92839',
        contents: '{"LEVEL":126}',
        dur: 0,
        depth: 0,
      }]
    };
    MockRealTime.mockResolvedValue(Realtime);
    tabPaneHisysEvents.data = tabData.hiSysEvents;
    expect(tabPaneHisysEvents.data).toBeUndefined();
  });
  it('TabPaneHisysEvents02 ', function () {
    let tabPaneHisysEvents = new TabPaneHisysEvents();
    const domainValue = 'DISPLAY';
    const tagElement = tabPaneHisysEvents.buildTag(domainValue);
    expect(tagElement.className).toBe('tagElement');
    expect(tagElement.id).toBe(domainValue);
    const tag = tagElement.querySelector('.tag');
    expect(tag.innerHTML).toBe(domainValue);
    const closeButton = tagElement.querySelector('lit-icon');
    expect(closeButton.getAttribute('name')).toBe('close-light');
    expect(closeButton.style.color).toBe('rgb(255, 255, 255)');
  });
  it('TabPaneHisysEvents03', () => {
    let tabPaneHisysEvents = new TabPaneHisysEvents();
    expect(tabPaneHisysEvents.updateData()).toBeUndefined();
  });
  it('TabPaneHisysEvents04', () => {
    let tabPaneHisysEvents = new TabPaneHisysEvents();
    expect(tabPaneHisysEvents.filterData()).toBeTruthy();
  });
  it('TabPaneHisysEvents05', () => {
    let tabPaneHisysEvents = new TabPaneHisysEvents();
    expect(tabPaneHisysEvents.updateDetail()).toBeUndefined();
  });
  it('TabPaneHisysEvents06', () => {
    let tabPaneHisysEvents = new TabPaneHisysEvents();
    let hisysEventSource = [
      { key: 'A', sort: 1 },
      { key: 'B', sort: 2 },
      { key: 'C', sort: 3 },
    ]
    let hiSysEventTable = { recycleDataSource: [] };
    tabPaneHisysEvents.sortByColumn.call({ hisysEventSource, hiSysEventTable }, { key: 'key', sort: 1, type: 'number' });
    expect(hisysEventSource).toEqual([
      { key: 'A', sort: 1 },
      { key: 'B', sort: 2 },
      { key: 'C', sort: 3 },
    ]);
    expect(hiSysEventTable.recycleDataSource).toEqual([
      { key: 'A', sort: 1 },
      { key: 'B', sort: 2 },
      { key: 'C', sort: 3 },
    ]);
  });
  it('TabPaneHisysEvents07', () => {
    let tabPaneHisysEvents = new TabPaneHisysEvents();
    tabPaneHisysEvents.baseTime = '';
    tabPaneHisysEvents.currentDetailList = [{
      key: 'key',
      value: 'value'
    }];
    tabPaneHisysEvents.realTime = 0;
    tabPaneHisysEvents.changeInput = document.createElement('input');
    tabPaneHisysEvents.detailsTbl = document.createElement('table');
    tabPaneHisysEvents.slicerTrack = document.createElement('div');
    tabPaneHisysEvents.boxDetails = document.createElement('div');
    tabPaneHisysEvents.detailbox = document.createElement('div');
    const data = {
      contents: JSON.stringify({
        key1: 'value1',
        key2: 'value2',
        INPUT_TIME: '1234567890'
      })
    };
    tabPaneHisysEvents.convertData(data);
    expect(tabPaneHisysEvents.baseTime).toBe('1234567890000000');
    expect(tabPaneHisysEvents.changeInput.value).toBe('1234567890000000');
    expect(tabPaneHisysEvents.slicerTrack.style.visibility).toBe('visible');
    expect(tabPaneHisysEvents.detailsTbl.style.paddingLeft).toBe('20px');
    expect(tabPaneHisysEvents.boxDetails.style.width).toBe('65%');
    expect(tabPaneHisysEvents.detailbox.style.display).toBe('block');
    expect(tabPaneHisysEvents.detailsTbl.recycleDataSource).toEqual([
      { key: 'key', value: 'value' },
      { key: 'key1', value: 'value1' },
      { key: 'key2', value: 'value2' },
      { key: 'INPUT_TIME', value: '1234567890000000' }
    ]);
  });
  it('TabPaneHisysEvents08 ', function () {
    let tabPaneHisysEvents = new TabPaneHisysEvents();
    let changeInput = {
      value: '',
    };
    let mockUpdateDetail = jest.fn();
    changeInput.value = 'abc';
    tabPaneHisysEvents.changeInputEvent.call({ changeInput: changeInput, updateDetail: mockUpdateDetail });
    expect(changeInput.value).toEqual('abc');
  });
});