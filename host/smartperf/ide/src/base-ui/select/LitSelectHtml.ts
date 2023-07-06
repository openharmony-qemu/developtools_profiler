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

export function selectHtmlStr(): string {
  return `
        :host{
            display: inline-flex;
            position: relative;
            overflow: visible;
            cursor: pointer;
            border-radius: 2px;
            outline: none;
            -webkit-user-select:none ;
            -moz-user-select:none;
            user-select:none;
        }
        :host(:not([border])),
        :host([border='true']){
            border: 1px solid var(--bark-prompt,#dcdcdc);
        }
        input{
            border: 0;
            outline: none;
            background-color: transparent;
            cursor: pointer;
            -webkit-user-select:none ;
            -moz-user-select:none;
            user-select:none;
            display: inline-flex;
            color: var(--dark-color2,rgba(0,0,0,0.9));
        }
        :host([highlight]) input {
            color: rgba(255,255,255,0.9);
        }
        :host([mode])  input{
            padding: 6px 0px;
        }
        :host([mode])  .root{
            padding: 1px 8px;
        }
        .root{
            position: relative;
            padding: 3px 6px;
            display: flex;
            align-items: center;
            justify-content: space-between;
            border-radius: 2px;
            outline: none;
            font-size: 1rem;
            z-index: 2;
            -webkit-user-select:none ;
            -moz-user-select:none;
            user-select:none;
            width: 100%;
        }
        .body{
            position: absolute;
            bottom: 100%;
            z-index: 99;
            padding-top: 5px;
            margin-top: 2px;
            background-color: var(--dark-background4,#fff);
            width: 100%;
            transform: scaleY(.6);
            visibility: hidden;
            opacity: 0;
            transform-origin: bottom center;
            display: block;
            flex-direction: column;
        }
        .body-bottom{
            bottom: auto;
            top: 100%;
            transform-origin: top center;
        }
        :host([placement="bottom"]) .body{
            bottom:unset;
            top: 100%;
            transition: none;
            transform: none;
        }

        :host([rounded]) .body {
            border-radius: 16px;
        }
        :host([rounded]) .root {
            border-radius: 16px;
            height: 25px;
        }
        .icon{
            pointer-events: none;
        }
        .noSelect{
          -moz-user-select:none;
          -ms-user-select:none;
          user-select:none;
          -khtml-user-select:none;
          -webkit-touch-callout:none;
          -webkit-user-select:none;
        }

        :host(:not([border]):not([disabled]):focus),
        :host([border='true']:not([disabled]):focus),
        :host(:not([border]):not([disabled]):hover),
        :host([border='true']:not([disabled]):hover){
            border:1px solid var(--bark-prompt,#ccc)
        }
        :host(:not([disabled]):focus) .body,
        :host(:not([disabled]):focus-within) .body{
            transform: scaleY(1);
            opacity: 1;
            z-index: 99;
            visibility: visible;
        }
        :host(:not([disabled]):focus)  input{
            color: var(--dark-color,#bebebe);
        }
        :host(:not([border])[disabled]) *,
        :host([border='true'][disabled]) *{
            background-color: var(--dark-background1,#f5f5f5);
            color: #b7b7b7;
            cursor: not-allowed;
        }
        :host([border='false'][disabled]) *{
            color: #b7b7b7;
            cursor: not-allowed;
        }`;
}
