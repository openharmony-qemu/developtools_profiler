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

import './LitTreeNode.js'
import { BaseElement, element } from '../BaseElement.js';
import { LitTreeNode } from './LitTreeNode.js';

export interface TreeItemData {
  key: string;
  title: string;
  icon?: string; //节点的自定义图标  设置show-icon才会生效
  selected?: boolean; //控制是否选择该节点
  checked?: boolean;
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

  static get observedAttributes() {
    return ['show-line', 'show-icon', 'checkable', 'foldable', 'dragable', 'multiple']; //foldable 表示点击目录是否可以折叠
  }

  set treeData(value: Array<TreeItemData>) {
    this._treeData = value;
    this.shadowRoot!.querySelector('#root')!.innerHTML = '';
    this.nodeList = [];
    this.drawTree(this.shadowRoot!.querySelector('#root'), value, true);

    /*双向绑定*/
    const handler = {
      get: (target: any, propkey: any) => {
        return target[propkey];
      },
      set: (target: any, propkey: any, value: any, receiver: any) => {
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
              ul.open = 'true'
              ul.style.transition = '.3s all';
              this.currentSelectedNode.parentElement.append(ul);
              this.currentSelectedNode.arrow = true;
              this._insertNode(ul, value);
            }
          }
        }
        return true;
      }
    };
    let setProxy = (v: Array<TreeItemData>) => {
      v.forEach(a => {
        if (!a.children) {
          a.children = new Proxy([], handler);
        } else {
          a.children = new Proxy(a.children, handler);
          setProxy(a.children || []);
        }
      })
    };
    setProxy(this._treeData);
    this.proxyData = new Proxy(this._treeData, handler);
  }

  set multiple(value: boolean) {
    if (value) {
      this.setAttribute('multiple','');
    } else {
      this.removeAttribute('multiple');
    }
  }

  get multiple() {
    return this.hasAttribute('multiple');
  }

  get treeData() {
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
  connectedCallback() {
    this.onclick = ev => {
      this.contextMenu!.style.display = 'none';
      this.currentSelectedData = null;
      this.currentSelectedNode = null;
      this.selectedNode(null);
    }
  }

  getCheckdKeys() {
    return Array.from(this.shadowRoot!.querySelectorAll('lit-tree-node[checked]')).map((a: any) => a.data.key);
  }

  getCheckdNodes() {
    return Array.from(this.shadowRoot!.querySelectorAll('lit-tree-node[checked]')).map((a: any) => a.data);
  }

  //展开所有节点
  expandKeys(keys: Array<string>) {
    keys.forEach(k => this.shadowRoot!.querySelectorAll(`lit-tree-node[key='${k}']`).forEach((b: any) => b.expand()));
  }

  //收起所有节点
  collapseKeys(keys: Array<string>) {
    keys.forEach(k => this.shadowRoot!.querySelectorAll(`lit-tree-node[key='${k}']`).forEach((b: any) => b.collapse()));
  }

  checkedKeys(keys: Array<string>) {
    keys.forEach(k => this.shadowRoot!.querySelectorAll(`lit-tree-node[key='${k}']`).forEach((b: any) => {
      b.setAttribute('checked', 'true');
      b.checkHandler();
    }));
  }

  uncheckedKeys(keys: Array<string>) {
    keys.forEach(k => this.shadowRoot!.querySelectorAll(`lit-tree-node[key='${k}']`).forEach((b: any) => {
      b.removeAttribute('checked');
      b.removeAttribute('missing');
      b.checkHandler();
    }));
  }

  drawTree(parent: any, array: Array<TreeItemData>, topDepth: boolean = false) {
    let that = this;
    array.forEach(a => {
      let li = document.createElement('li');
      let node: LitTreeNode = document.createElement('lit-tree-node') as LitTreeNode;
      node.title = a.title;
      node.setAttribute('key', a.key);
      node.topDepth = topDepth;
      if (this.hasAttribute('dragable')) {
        node.draggable = true;
        document.ondragover = function (e) {
          e.preventDefault();
        };
        //在拖动目标上触发事件 (源元素)
        node.ondrag = ev => this.onDrag(ev);//元素正在拖动时触发
        node.ondragstart = ev => this.onDragStart(ev);//用户开始拖动元素时触发
        node.ondragend = ev => this.onDragEnd(ev);// 用户完成元素拖动后触发
        //释放目标时触发的事件:
        node.ondragenter = ev => this.onDragEnter(ev);//当被鼠标拖动的对象进入其容器范围内时触发此事件
        node.ondragover = ev => this.onDragOver(ev);//当某被拖动的对象在另一对象容器范围内拖动时触发此事件
        node.ondragleave = ev => this.onDragLeave(ev);//当被鼠标拖动的对象离开其容器范围内时触发此事件
        node.ondrop = ev => this.onDrop(ev);//在一个拖动过程中，释放鼠标键时触发此事件
      }
      node.selected = a.selected || false; //是否选中行
      node.checked = a.checked || false;// 是否勾选
      node.data = a;
      node.addEventListener('change', (e: any) => {
        if (e.detail && !this.multiple) {
          this.nodeList.forEach(item => {
            item.checked = item.data!.key === node.data!.key;
            item.data!.checked = item.checked;
          });
        }
        var litTreeNodes = this.nodeList.filter(it => it.checked);
        if (litTreeNodes.length === 0) {
          node.checked = true;
          node.data!.checked = true;
        }
        that.dispatchEvent(new CustomEvent('onChange', {detail: {data: (node as any).data, checked: e.detail}}));
      });
      node.multiple = this.hasAttribute('multiple');
      node.checkable = this.getAttribute('checkable') || 'false';
      this.nodeList.push(node);
      // @ts-ignore
      li.data = a;
      li.append(node);
      parent.append(li);
      let ul = document.createElement('ul');
      // @ts-ignore
      ul.open = 'true';
      ul.style.transition = '.3s all';
      if (a.children && a.children.length > 0) {
        if (this.hasAttribute('show-icon')) {
          if (a.icon) {
            (node as any).iconName = a.icon;
          } else {
            (node as any).iconName = 'folder'
          }
        } else {
          node.iconName = '';
        }
        node.arrow = true;
        li.append(ul);
        this.drawTree(ul, a.children);
      } else {
        if (this.hasAttribute('show-icon')) {
          if (a.icon) {
            node.iconName = a.icon;
          } else {
            node.iconName = 'file';
          }
        } else {
          node.iconName = '';
        }
        node.arrow = false;
      }
      li.onclick = (e) => {
        e.stopPropagation()
        if (this.hasAttribute('foldable')) {
          // @ts-ignore
          if (li.data.children && li.data.children.length > 0) {
            node.autoExpand();
          } else {
            // @ts-ignore
            this.dispatchEvent(new CustomEvent('onSelect', {detail: li.data}))
            this.selectedNode(node);
          }
        } else {
          // @ts-ignore
          this.dispatchEvent(new CustomEvent('onSelect', {detail: li.data}))
          this.selectedNode(node);
        }
      };
      // node 添加右键菜单功能
      node.oncontextmenu = ev => {
        ev.preventDefault();
        this.selectedNode(node);
        this.currentSelectedNode = node;
        this.currentSelectedData = node.data;
        this.contextMenu!.style.display = 'block';
        this.contextMenu!.style.left = ev.pageX + 'px';
        this.contextMenu!.style.top = ev.pageY + 'px';
      };
    });
    this.oncontextmenu = ev => {
      ev.preventDefault();
      this.contextMenu!.style.display = 'block';
      this.contextMenu!.style.left = ev.pageX + 'px';
      this.contextMenu!.style.top = ev.pageY + 'px';
    };
  }

  //取消所有节点的选中状态 然后选中当前node节点
  selectedNode(node: LitTreeNode | null | undefined) {
    this.shadowRoot!.querySelectorAll<LitTreeNode>('lit-tree-node').forEach((a) => {
      a.selected = false;
    })
    if (node) {
      node.selected = true;
    }
  }

  closeContextMenu() {
    this.contextMenu!.style.display = 'none';
  }

  onDrag(e: MouseEvent) {

  }

  onDragStart(ev: MouseEvent) {
    this.srcDragElement = ev.target;
    (ev.target! as LitTreeNode).open = 'true';
    (ev.target! as LitTreeNode).autoExpand();
    return undefined;
  }

  onDragEnd(ev: MouseEvent) {
    this.srcDragElement = null;
    return undefined;
  }

  onDragEnter(ev: MouseEvent) {
    (ev.target as LitTreeNode).style.backgroundColor = '#42b98333';
    return undefined;
  }

  onDragOver(ev: MouseEvent) {
    let node = ev.target as LitTreeNode;
    if (this.srcDragElement.data.key === node.data!.key) return;
    let rect = (ev.currentTarget! as any).getBoundingClientRect();
    if (ev.clientX >= rect.left + rect.width / 3 && ev.clientX < rect.left + rect.width) { //bottom-right
      this.dragDirection = 'bottom-right';
      node.drawLine('bottom-right');
    } else if (ev.clientY >= rect.top && ev.clientY < rect.top + rect.height / 2) {//上面
      this.dragDirection = 'top';
      node.drawLine('top');
    } else if (ev.clientY <= rect.bottom && ev.clientY > rect.top + rect.height / 2) {//下面
      this.dragDirection = 'bottom';
      node.drawLine('bottom');
    }
    return undefined;
  }

  onDragLeave(ev: MouseEvent) {
    (ev.target as LitTreeNode).style.backgroundColor = '#ffffff00';
    (ev.target as LitTreeNode).drawLine('');
    return undefined;
  }

  onDrop(ev: MouseEvent) {
    (ev.target as LitTreeNode).style.backgroundColor = '#ffffff00';
    (ev.target as LitTreeNode).drawLine('');
    //移动的不是node节点 而是上层的li节点
    let srcData = this.srcDragElement.data;//获取原节点的data数据
    let dstData = (ev.target as LitTreeNode).data;//获取目标节点的data数据
    if (srcData.key === dstData!.key) return;//同一个节点不用处理
    let srcElement = this.srcDragElement.parentElement;
    let srcParentElement = srcElement.parentElement;
    let dstElement = (ev.target as LitTreeNode).parentElement;
    srcElement.parentElement.removeChild(srcElement);//node li ul  从 ul 中移除 li
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
      dstElement!.querySelector<LitTreeNode>('lit-tree-node')!.arrow = true;//拖动进入子节点，需要开启箭头
      ul.appendChild(srcElement);
    }
    let ul1 = dstElement!.querySelector('ul');//如果拖动后目标节点的子节点没有记录，需要关闭arrow箭头
    if (ul1) {
      if (ul1.childElementCount == 0) (ul1.previousElementSibling! as LitTreeNode).arrow = false;
    }
    if (srcParentElement.childElementCount === 0) srcParentElement.previousElementSibling.arrow = false;//如果拖动的原节点的父节点没有子节点需要 关闭arrow箭头
    //拖动调整结构后修改 data树形数据结构
    this.removeDataNode(this._treeData, srcData);
    this.addDataNode(this._treeData, srcData, dstData!.key, this.dragDirection!);
    this.dispatchEvent(new CustomEvent('drop', {
      detail: {
        treeData: this._treeData,
        srcData: srcData,
        dstData: dstData,
        type: this.dragDirection
      }
    }))
    ev.stopPropagation();
    return undefined;
  }

  //移除treeData中指定的节点 通过key匹配
  removeDataNode(arr: Array<TreeItemData>, d: TreeItemData) {
    let delIndex = arr.findIndex(v => v.key === d.key);
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
  addDataNode(arr: Array<TreeItemData>, d: TreeItemData, k: string, t: string) {
    for (let i = 0; i < arr.length; i++) {
      if (arr[i].key === k) {
        if (t === 'bottom-right') {
          if (!arr[i].children) arr[i].children = [];
          arr[i].children!.push(d);
        } else if (t === 'top') {
          arr.splice(i, 0, d);
        } else if (t === 'bottom') {
          arr.splice(i + 1, 0, d);
        }
        return;
      } else {
        if (arr[i].children) this.addDataNode(arr[i].children || [], d, k, t);
      }
    }
  }

  insert(obj: TreeItemData) {
    if (this.currentSelectedData) {
      this.currentSelectedData.children.push(obj);
    } else {
      this.treeData.push(obj);
    }
  }

  _insertNode(parent: any, a: any) {
    if (!parent) parent = this.shadowRoot!.querySelector('#root');
    let li = document.createElement('li');
    let insertNode = document.createElement('lit-tree-node') as LitTreeNode;
    insertNode.title = a.title;
    insertNode.setAttribute('key', a.key);
    if (this.hasAttribute('dragable')) {
      insertNode.draggable = true;
      document.ondragover = function (e) {
        e.preventDefault();
      }
      //在拖动目标上触发事件 (源元素)
      insertNode.ondrag = ev => this.onDrag(ev);//元素正在拖动时触发
      insertNode.ondragstart = ev => this.onDragStart(ev);//用户开始拖动元素时触发
      insertNode.ondragend = ev => this.onDragEnd(ev);// 用户完成元素拖动后触发
      //释放目标时触发的事件:
      insertNode.ondragenter = ev => this.onDragEnter(ev);//当被鼠标拖动的对象进入其容器范围内时触发此事件
      insertNode.ondragover = ev => this.onDragOver(ev);//当某被拖动的对象在另一对象容器范围内拖动时触发此事件
      insertNode.ondragleave = ev => this.onDragLeave(ev);//当被鼠标拖动的对象离开其容器范围内时触发此事件
      insertNode.ondrop = ev => this.onDrop(ev);//在一个拖动过程中，释放鼠标键时触发此事件
    }
    insertNode.selected = a.selected || false; //是否选中行
    insertNode.checked = a.checked || false;// 是否勾选
    insertNode.data = a;
    insertNode.addEventListener('change', (e: any) => {
      if (e.detail && !this.multiple) {
        this.nodeList.forEach(node => {
          node.checked = node.data!.key === insertNode.data!.key;
        });
      }
      this.dispatchEvent(new CustomEvent('onChange', {detail: {data: insertNode.data, checked: e.detail}}));
    })
    this.nodeList.push(insertNode);
    insertNode.checkable = this.getAttribute('checkable') || 'false';
    insertNode.multiple = this.hasAttribute('multiple');
    // @ts-ignore
    li.data = a;
    li.append(insertNode);
    parent.append(li)
    let ul = document.createElement('ul');
    // @ts-ignore
    ul.open = 'true'
    ul.style.transition = '.3s all';
    if (a.children && a.children.length > 0) {
      if (this.hasAttribute('show-icon')) {
        if (a.icon) {
          insertNode.iconName = a.icon;
        } else {
          insertNode.iconName = 'folder';
        }
      } else {
        insertNode.iconName = '';
      }
      insertNode.arrow = true;
      li.append(ul);
      this.drawTree(ul, a.children);
    } else {
      if (this.hasAttribute('show-icon')) {
        if (a.icon) {
          insertNode.iconName = a.icon;
        } else {
          insertNode.iconName = 'file';
        }
      } else {
        insertNode.iconName = '';
      }
      insertNode.arrow = false;
    }
    li.onclick = (e) => {
      e.stopPropagation()
      if (this.hasAttribute('foldable')) {
        // @ts-ignore
        if (li.data.children && li.data.children.length > 0) {
          insertNode.autoExpand();
        } else {
          // @ts-ignore
          this.dispatchEvent(new CustomEvent('onSelect', {detail: li.data}));
          this.selectedNode(insertNode);
        }
      } else {
        // @ts-ignore
        this.dispatchEvent(new CustomEvent('onSelect', {detail: li.data}));
        this.selectedNode(insertNode);
      }
    }
    // node 添加右键菜单功能
    insertNode.oncontextmenu = (ev) => {
      ev.preventDefault();
      this.selectedNode(insertNode);
      this.currentSelectedNode = insertNode;
      this.currentSelectedData = insertNode.data;
      this.contextMenu!.style.display = 'block';
      this.contextMenu!.style.left = ev.pageX + 'px';
      this.contextMenu!.style.top = ev.pageY + 'px';
    };
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