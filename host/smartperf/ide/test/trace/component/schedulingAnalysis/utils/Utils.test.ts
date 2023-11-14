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
import {getDataNo,
  getFormatData,
  getInitializeTime
  // @ts-ignore
} from "../../../../../dist/trace/component/schedulingAnalysis/utils/Utils.js";

describe('schedulingAnalysis utils  Test', () => {
  it('schedulingAnalysisUtilsTest01', () => {
    expect(getFormatData([{
      avg:5,
      max:10,
      min:1,
      sum:16
    }])).toStrictEqual([
      {
        "avg": "5ns ",
        "index": 1,
        "max": "10ns ",
        "min": "1ns ",
        "sum": "16ns "
      }
    ]);
  });
  it('schedulingAnalysisUtilsTest02', () => {
    expect(getDataNo([{
      avg:5,
      max:10,
      min:1,
      sum:16
    }])).toStrictEqual([
      {
        "avg": 5,
        "index": 1,
        "max": 10,
        "min": 1,
        "sum": 16
      }
    ]);
  });
  // 23m46s12ms32μs
  it('schedulingAnalysisUtilsTest03', () => {
    expect(getInitializeTime('1h')).toBe('3600000000000');
  });
  it('schedulingAnalysisUtilsTest04', () => {
    expect(getInitializeTime(' 23m')).toBe('1380000000000');
  });
  it('schedulingAnalysisUtilsTest05', () => {
    expect(getInitializeTime('46s')).toBe('46000000000');
  });
})
