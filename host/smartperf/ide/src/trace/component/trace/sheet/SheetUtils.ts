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

import { LitTable } from '../../../../base-ui/table/lit-table.js';

export function resizeObserver(
  parentEl: HTMLElement,
  tableEl: LitTable,
  tblOffsetHeight: number = 45,
  loadingPage?: HTMLElement,
  loadingPageOffsetHeight: number = 24
) {
  new ResizeObserver((entries) => {
    if (parentEl.clientHeight != 0) {
      // @ts-ignore
      tableEl?.shadowRoot.querySelector('.table').style.height = parentEl.clientHeight - tblOffsetHeight + 'px';
      tableEl?.reMeauseHeight();
      if (loadingPage) {
        loadingPage.style.height = parentEl.clientHeight - loadingPageOffsetHeight + 'px';
      }
    }
  }).observe(parentEl);
}

export function showButtonMenu(filter: any, isShow: boolean) {
  if (isShow) {
    filter.setAttribute('tree', '');
    filter.setAttribute('input', '');
    filter.setAttribute('inputLeftText', '');
  } else {
    filter.removeAttribute('tree');
    filter.removeAttribute('input');
    filter.removeAttribute('inputLeftText');
  }
}
