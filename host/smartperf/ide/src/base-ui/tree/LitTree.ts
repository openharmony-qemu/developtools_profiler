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

import './LitTreeNode.js';
import { BaseElement, element } from '../BaseElement.js';
import { type LitTreeNode } from './LitTreeNode.js';

export interface TreeItemData {
  key: string;
  title: string;
  icon?: string; //节点的自定义图标  设置show-icon才会生效
  selected?: boolean; //控制是否选择该节点
  checked?: boolean;
  disable?: boolean; //控制是否显示checkbox
  children?: Array<TreeItemData> | null | undefined;
}

@element('lit-tree')
export class LitTree extends BaseElement {
  private _treeData: Array<TreeItemData> = [];
  private currentSelectedNode: any;
  private currentSelectedData: any;
  private proxyData: any;
  private nodeList: Array<LitTreeNode> = [];
  private contextMenu: HTMLDivElement | null | undefined;
  private srcDragElement: any;
  private dragDirection: string | null | undefined;

  static get observedAttributes(): string[] {
    return ['show-line', 'show-icon', 'checkable', 'foldable', 'dragable', 'multiple']; //foldable 表示点击目录是否可以折叠
  }

  set treeData(value: Array<TreeItemData>) {
    this._treeData = value;
    this.shadowRoot!.querySelector('#root')!.innerHTML = '';
    this.nodeList = [];
    this.drawTree(this.shadowRoot!.querySelector('#root'), value, true);

    /*双向绑定*/
    const handler = {
      get: (target: any, propkey: any): any => {
        return target[propkey];
      },
      set: (target: any, propkey: any, value: any, receiver: any): boolean => {
        if (target[propkey] !== value) {
          if (!value.children) {
            value.children = new Proxy([], handler);
          } else {
            value.children = new Proxy(value.children, handler);
          }
          target[propkey] = value;
          if (!this.currentSelectedNode) {
            this._insertNode(null, value);
          } else {
            if (this.currentSelectedNode.nextElementSibling) {
              this._insertNode(this.currentSelectedNode.nextElementSibling, value);
            } else {
              this.currentSelectedNode.setAttribute('show-arrow', 'true');
              let ul = document.createElement('ul');
              // @ts-ignore
              ul.open = 'true';
              ul.style.transition = '.3s all';
              this.currentSelectedNode.parentElement.append(ul);
              this.currentSelectedNode.arrow = true;
              this._insertNode(ul, value);
            }
          }
        }
        return true;
      },
    };
    let setProxy = (v: Array<TreeItemData>): void => {
      v.forEach((a) => {
        if (!a.children) {
          a.children = new Proxy([], handler);
        } else {
          a.children = new Proxy(a.children, handler);
          setProxy(a.children || []);
        }
      });
    };
    setProxy(this._treeData);
    this.proxyData = new Proxy(this._treeData, handler);
  }

  set multiple(value: boolean) {
    if (value) {
      this.setAttribute('multiple', '');
    } else {
      this.removeAttribute('multiple');
    }
  }

  get multiple(): boolean {
    return this.hasAttribute('multiple');
  }

  get treeData(): TreeItemData[] {
    return this.proxyData;
  }

  set onSelect(fn: any) {
    this.addEventListener('onSelect', fn);
  }

  set onChange(fn: any) {
    this.addEventListener('onChange', fn);
  }

  set foldable(value: any) {
    if (value) {
      this.setAttribute('foldable', '');
    } else {
      this.removeAttribute('foldable');
    }
  }

  //当 custom element首次被插入文档DOM时，被调用。
  connectedCallback(): void {
    this.onclick = (ev): void => {
      this.contextMenu!.style.display = 'none';
      this.currentSelectedData = null;
      this.currentSelectedNode = null;
      this.selectedNode(null);
    };
  }

  getCheckdKeys(): any[] {
    return Array.from(this.shadowRoot!.querySelectorAll('lit-tree-node[checked]')).map((a: any) => a.data.key);
  }

  getCheckdNodes(): any[] {
    return Array.from(this.shadowRoot!.querySelectorAll('lit-tree-node[checked]')).map((a: any) => a.data);
  }

  //展开所有节点
  expandKeys(keys: Array<string>): void {
    keys.forEach((k) => this.shadowRoot!.querySelectorAll(`lit-tree-node[key='${k}']`).forEach((b: any) => b.expand()));
  }

  //收起所有节点
  collapseKeys(keys: Array<string>): void {
    keys.forEach((k) =>
      this.shadowRoot!.querySelectorAll(`lit-tree-node[key='${k}']`).forEach((b: any) => b.collapse())
    );
  }

  checkedKeys(keys: Array<string>): void {
    keys.forEach((k) =>
      this.shadowRoot!.querySelectorAll(`lit-tree-node[key='${k}']`).forEach((b: any) => {
        b.setAttribute('checked', 'true');
        b.checkHandler();
      })
    );
  }

  uncheckedKeys(keys: Array<string>): void {
    keys.forEach((k) =>
      this.shadowRoot!.querySelectorAll(`lit-tree-node[key='${k}']`).forEach((b: any) => {
        b.removeAttribute('checked');
        b.removeAttribute('missing');
        b.checkHandler();
      })
    );
  }

  //取消所有节点的选中状态 然后选中当前node节点
  selectedNode(node: LitTreeNode | null | undefined): void {
    this.shadowRoot!.querySelectorAll<LitTreeNode>('lit-tree-node').forEach((a) => {
      a.selected = false;
    });
    if (node) {
      node.selected = true;
    }
  }

  closeContextMenu(): void {
    this.contextMenu!.style.display = 'none';
  }

  onDrag(e: MouseEvent): void {}

  onDragStart(ev: MouseEvent): undefined {
    this.srcDragElement = ev.target;
    (ev.target! as LitTreeNode).open = 'true';
    (ev.target! as LitTreeNode).autoExpand();
    return undefined;
  }

  onDragEnd(ev: MouseEvent): undefined {
    this.srcDragElement = null;
    return undefined;
  }

  onDragEnter(ev: MouseEvent): undefined {
    (ev.target as LitTreeNode).style.backgroundColor = '#42b98333';
    return undefined;
  }

  onDragOver(ev: MouseEvent): undefined {
    let node = ev.target as LitTreeNode;
    if (this.srcDragElement.data.key === node.data!.key) {
      return;
    }
    let rect = (ev.currentTarget! as any).getBoundingClientRect();
    if (ev.clientX >= rect.left + rect.width / 3 && ev.clientX < rect.left + rect.width) {
      //bottom-right
      this.dragDirection = 'bottom-right';
      node.drawLine('bottom-right');
    } else if (ev.clientY >= rect.top && ev.clientY < rect.top + rect.height / 2) {
      //上面
      this.dragDirection = 'top';
      node.drawLine('top');
    } else if (ev.clientY <= rect.bottom && ev.clientY > rect.top + rect.height / 2) {
      //下面
      this.dragDirection = 'bottom';
      node.drawLine('bottom');
    }
    return undefined;
  }

  onDragLeave(ev: MouseEvent): undefined {
    (ev.target as LitTreeNode).style.backgroundColor = '#ffffff00';
    (ev.target as LitTreeNode).drawLine('');
    return undefined;
  }

  onDrop(ev: MouseEvent): undefined {
    (ev.target as LitTreeNode).style.backgroundColor = '#ffffff00';
    (ev.target as LitTreeNode).drawLine('');
    //移动的不是node节点 而是上层的li节点
    let srcData = this.srcDragElement.data; //获取原节点的data数据
    let dstData = (ev.target as LitTreeNode).data; //获取目标节点的data数据
    if (srcData.key === dstData!.key) {
      return;
    } //同一个节点不用处理
    let srcElement = this.srcDragElement.parentElement;
    let srcParentElement = srcElement.parentElement;
    let dstElement = (ev.target as LitTreeNode).parentElement;
    srcElement.parentElement.removeChild(srcElement); //node li ul  从 ul 中移除 li
    if (this.dragDirection === 'top') {
      dstElement!.parentElement!.insertBefore(srcElement, dstElement);
    } else if (this.dragDirection === 'bottom') {
      dstElement!.parentElement!.insertBefore(srcElement, dstElement!.nextSibling);
    } else if (this.dragDirection === 'bottom-right') {
      let ul = dstElement!.querySelector('ul');
      if (!ul) {
        ul = document.createElement('ul');
        ul.style.cssText = 'transition: all 0.3s ease 0s;';
        dstElement!.appendChild(ul);
      }
      dstElement!.querySelector<LitTreeNode>('lit-tree-node')!.arrow = true; //拖动进入子节点，需要开启箭头
      ul.appendChild(srcElement);
    }
    let ul1 = dstElement!.querySelector('ul'); //如果拖动后目标节点的子节点没有记录，需要关闭arrow箭头
    if (ul1) {
      if (ul1.childElementCount === 0) {
        (ul1.previousElementSibling! as LitTreeNode).arrow = false;
      }
    }
    if (srcParentElement.childElementCount === 0) {
      srcParentElement.previousElementSibling.arrow = false;
    } //如果拖动的原节点的父节点没有子节点需要 关闭arrow箭头
    //拖动调整结构后修改 data树形数据结构
    this.removeDataNode(this._treeData, srcData);
    this.addDataNode(this._treeData, srcData, dstData!.key, this.dragDirection!);
    this.dispatchEvent(
      new CustomEvent('drop', {
        detail: {
          treeData: this._treeData,
          srcData: srcData,
          dstData: dstData,
          type: this.dragDirection,
        },
      })
    );
    ev.stopPropagation();
    return undefined;
  }

  //移除treeData中指定的节点 通过key匹配
  removeDataNode(arr: Array<TreeItemData>, d: TreeItemData): void {
    let delIndex = arr.findIndex((v) => v.key === d.key);
    if (delIndex > -1) {
      arr.splice(delIndex, 1);
      return;
    }
    for (let i = 0; i < arr.length; i++) {
      if (arr[i].children && arr[i].children!.length > 0) {
        this.removeDataNode(arr[i].children!, d);
      }
    }
  }

  //中array中匹配到key为k的节点， t='bottom-right' 把d加入到该节点的children中去 t='top' 添加到找到的节点前面 t='bottom' 添加到找到的节点后面
  addDataNode(arr: Array<TreeItemData>, d: TreeItemData, k: string, t: string): void {
    for (let i = 0; i < arr.length; i++) {
      if (arr[i].key === k) {
        if (t === 'bottom-right') {
          if (!arr[i].children) {
            arr[i].children = [];
          }
          arr[i].children!.push(d);
        } else if (t === 'top') {
          arr.splice(i, 0, d);
        } else if (t === 'bottom') {
          arr.splice(i + 1, 0, d);
        }
        return;
      } else {
        if (arr[i].children) {
          this.addDataNode(arr[i].children || [], d, k, t);
        }
      }
    }
  }

  insert(obj: TreeItemData): void {
    if (this.currentSelectedData) {
      this.currentSelectedData.children.push(obj);
    } else {
      this.treeData.push(obj);
    }
  }

  initElements(): void {
    this.contextMenu = this.shadowRoot!.querySelector<HTMLDivElement>('#contextMenu');
  }

  initHtml(): string {
    return `
        <style>
        :host{ 
            display: flex;
            color:#333;
            width: 100%;
            height: 100%;
            overflow: auto;
        }
        :host *{
            box-sizing: border-box;
        }
        ul,li{
            list-style-type: none;
            position:relative;
            cursor:pointer;
            overflow: hidden;
        }
        :host([show-line]) ul{
            padding-left:10px;
            border-left: 1px solid #d9d9d9;
            overflow: hidden;
        }
        :host(:not([show-line])) ul{
            padding-left: 10px;
            overflow: hidden;
        }
        /*ul>li:after{content:' ';position:absolute;top:50%;left:-45px;width:45px;border:none;border-top:1px solid #d9d9d9;}*/
        #root{
            width: 100%;
            height: 100%;
        }
        .context-menu { 
            position: absolute; 
            box-shadow: 0 0 10px #00000077;
            pointer-events: auto;
            z-index: 999;
            transition: all .3s;
        } 
        </style>
        <ul id="root" style="margin: 0;overflow: hidden"></ul>
        <div id="contextMenu" class="context-menu" style="display:none"> 
            <slot name="contextMenu"></slot>
        </div> 
        `;
  }
}

if (!customElements.get('lit-tree')) {
  customElements.define('lit-tree', LitTree);
}
