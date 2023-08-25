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
import { SpMetrics } from '../../../dist/trace/component/SpMetrics.js';

describe('SpMetrics Test', () => {
  let spMetrics = new SpMetrics();
  it('SpMetricsTest01', function () {
    expect(SpMetrics.initElements).toBeUndefined();
  });
  it('SpMetricsTest02', function () {
    expect(spMetrics.metric).toBe('');
  });
  it('SpMetricsTest03', function () {
    spMetrics.metric = true;
    expect(spMetrics.metric).toBe('');
  });
  it('SpMetricsTest04', function () {
    expect(spMetrics.metricResult).toBe('');
  });
  it('SpMetricsTest05', function () {
    spMetrics.metricResult = true;
    expect(spMetrics.metricResult).toBeTruthy();
  });

  it('SpMetricsTest06', function () {
    expect(spMetrics.attributeChangedCallback('metric')).toBeUndefined();
  });

  it('SpMetricsTest07', function () {
    expect(spMetrics.attributeChangedCallback('metricResult')).toBeUndefined();
  });

  it('SpMetricsTest09', function () {
    expect(spMetrics.reset()).toBeUndefined();
  });

  it('SpMetricsTest10', function () {
    expect(spMetrics.connectedCallback()).toBeUndefined();
  });

  it('SpMetricsTest11', function () {
    expect(spMetrics.disconnectedCallback()).toBeUndefined();
  });

  it('SpMetricsTest12', function () {
    expect(spMetrics.initMetricSelectOption()).toBeUndefined();
  });

  it('SpMetricsTest13', function () {
    expect(spMetrics.initMetricDataHandle()).toBeUndefined();
  });
});
