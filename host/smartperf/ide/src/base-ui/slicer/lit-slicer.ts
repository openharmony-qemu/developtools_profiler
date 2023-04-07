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

export class LitSlicer extends HTMLElement {
    static get observedAttributes() {
        return ["direction"] //direction = 'horizontal'或者'vertical'
    }

    constructor() {
        super();
        const shadowRoot = this.attachShadow({mode: 'open'});
        shadowRoot.innerHTML = `
        <style>
        :host{  
            display: flex;
        }
       
        #root{
            display: flex;
            width: 100%;
            height: 100%;
            overflow: auto;
        }
        </style>
        <div id="root"  style="${this.style}">
            <slot></slot>
        </div>
        `
    }

    set direction(val: any) {
        if (val.startsWith('h')) {
            this.shadowRoot!.querySelector('div')!.style.flexDirection = 'row'
        } else if (val.startsWith('v')) {
            this.shadowRoot!.querySelector('div')!.style.flexDirection = 'column'
        }
    }

    connectedCallback() {

    }

    disconnectedCallback() {

    }

    attributeChangedCallback(name: any, oldValue: any, newValue: any) {
        (this as any)[name] = newValue;
    }

    set style(v: any) {
        // this.shadowRoot.querySelector('#root').setAttribute('style',v)
    }
}

if (!customElements.get('lit-slicer')) {
    customElements.define('lit-slicer', LitSlicer);
}


export class LitSlicerTrack extends HTMLElement {
    private line: HTMLElement | null | undefined;
    private draging: boolean = false;
    private normalWidth: number = 0;

    static get observedAttributes() {
        return ["range-left", "range-right"]
    }

    get rangeLeft() {
        return parseInt(this.getAttribute('range-left') || '200');
    }

    set rangeLeft(val: number) {
        this.setAttribute('range-left', `${val}`);
    }

    get rangeRight() {
        return parseInt(this.getAttribute('range-right') || '300');
    }

    set rangeRight(val: number) {
        this.setAttribute('range-right', `${val}`);
    }

    constructor() {
        super();
        const shadowRoot = this.attachShadow({mode: 'open'});
        shadowRoot.innerHTML = `
        <style>
        :host{ 
            flex
        }
        .rootH{
            width:var(--lit-slicer-track--width,5px);
            background-color: var(--lit-slicer-track--background-color,#d1d1d1);
            height: 100%;
            cursor: ew-resize;
        }
        .rootV{
            height:var(--lit-slicer-track--height,5px);
            background-color: var(--lit-slicer-track--background-color,#d1d1d1);
            width: 100%;
            cursor: ns-resize;
        }
        </style>
        <div id="root">
        </div>
        `
    }

    //当 custom element首次被插入文档DOM时，被调用。
    connectedCallback() {
        this.line = this.shadowRoot?.querySelector('#root')
        let parentDirection = this.parentElement!.getAttribute("direction") || 'horizontal'
        if (parentDirection.startsWith("h")) {
            this.line!.className = 'rootH'
            let previousElementSibling = this.previousElementSibling as HTMLElement;
            let preX: number, preY: number, preWidth: number;
            this.line!.onmousedown = (e) => {
                this.draging = true;
                preX = e.pageX;
                preWidth = previousElementSibling!.clientWidth;
                if (this.normalWidth == 0) this.normalWidth = previousElementSibling!.clientWidth;
                previousElementSibling!.style.width = preWidth + 'px'
                document.body.style.userSelect = 'none';
                document.body.style.webkitUserSelect = 'none';
                // @ts-ignore
                document.body.style.msUserSelect = 'none';
                document.onmousemove = (e1) => {
                    if (this.draging) {
                        if ((preWidth + e1.pageX - preX) >= (this.normalWidth - this.rangeLeft) && (preWidth + e1.pageX - preX) <= (this.normalWidth + this.rangeRight)) {
                            previousElementSibling!.style.width = (preWidth + e1.pageX - preX) + 'px'
                        }
                    }
                }
                document.onmouseleave = (e2) => {
                    this.draging = false;
                    document.body.style.userSelect = 'auto';
                    document.body.style.webkitUserSelect = 'auto';
                    // @ts-ignore
                    document.body.style.msUserSelect = 'auto';
                }
                document.onmouseup = (e3) => {
                    this.draging = false;
                    document.body.style.userSelect = 'auto';
                    document.body.style.webkitUserSelect = 'auto';
                    // @ts-ignore
                    document.body.style.msUserSelect = 'auto';
                }
            }
        } else {
            this.line!.className = 'rootV'
            let previousElementSibling = this.previousElementSibling as HTMLElement;
            let preY: number, preHeight: number;
            this.line!.onmousedown = (e) => {
                this.draging = true;
                preY = e.pageY;
                preHeight = previousElementSibling?.clientHeight;
                previousElementSibling!.style!.height = preHeight + 'px'
                document.onmousemove = (e1) => {
                    if (this.draging) {
                        previousElementSibling.style.height = (preHeight + e1.pageY - preY) + 'px'
                    }
                }
                document.onmouseleave = (e2) => {
                    this.draging = false;
                }
                document.onmouseup = (e3) => {
                    this.draging = false;
                }
            }
        }

    }

    //当 custom element从文档DOM中删除时，被调用。
    disconnectedCallback() {
        this.line!.onmousedown = null;
    }

    //当 custom element被移动到新的文档时，被调用。
    adoptedCallback() {
    }

    //当 custom element增加、删除、修改自身属性时，被调用。
    attributeChangedCallback(name: any, oldValue: any, newValue: any) {
    }
}

if (!customElements.get('lit-slicer-track')) {
    customElements.define('lit-slicer-track', LitSlicerTrack);
}