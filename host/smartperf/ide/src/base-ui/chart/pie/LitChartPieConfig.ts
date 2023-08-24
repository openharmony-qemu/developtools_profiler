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

export interface LitChartPieConfig {
  appendPadding: number;
  data: any[];
  angleField: string;
  colorField: string;
  colorFieldTransferHandler?: (value: any) => any;
  radius: number;
  angleClick?: (it: object) => void;
  label: {
    type: string; // spider|inner|outer
    offset?: string;
    content?: (it: object) => string;
    color?: (it: object) => string;
    style?: {
      fontSize: number;
      textAlign: string;
    };
  };
  hoverHandler?: (data: any) => void;
  showChartLine?: boolean;
  tip: ((a: any) => string) | undefined;
  interactions: {
    type: string; //element-active | element-selected
  }[];
}
