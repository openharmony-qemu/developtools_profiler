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
import { LitTree } from '../../../dist/base-ui/tree/LitTree.js';

describe('LitTree Test', () => {
  let litTree = new LitTree();
  litTree.treeData = [];
  litTree.multiple = true;
  litTree.foldable = true;

  it('litTreeTest01', () => {
    expect(litTree.treeData).toStrictEqual([]);
  });
  it('litTreeTest02', () => {
    expect(litTree.multiple).toStrictEqual(true);
  });
  it('litTreeTest03', () => {
    expect(litTree.foldable).toBeUndefined();
  });
  it('litTreeTest04', () => {
    expect(litTree.getCheckdKeys()).toStrictEqual([]);
  });
  it('litTreeTest05', () => {
    expect(litTree.getCheckdNodes()).toStrictEqual([]);
  });
  it('litTreeTest06', () => {
    expect(litTree.expandKeys(['c', 'a'])).toBeUndefined();
  });
  it('litTreeTest07', () => {
    expect(litTree.collapseKeys(['cq', 'a'])).toBeUndefined();
  });
  it('litTreeTest08', () => {
    expect(litTree.checkedKeys(['a'])).toBeUndefined();
  });
  it('litTreeTest09', () => {
    expect(litTree.uncheckedKeys(['abc'])).toBeUndefined();
  });
  it('litTreeTest10', () => {
    document.body.innerHTML = `<ul id="ul"></ul>`;
    let ul = document.querySelector('#ul') as HTMLDivElement;
    expect(litTree.drawTree(ul, [{}, {}], true)).toBeUndefined();
  });
  it('litTreeTest11', () => {
    expect(litTree.selectedNode([{}, {}])).toBeUndefined();
  });
  it('litTreeTest12', () => {
    expect(litTree.closeContextMenu()).toBeUndefined();
  });
  it('litTreeTest13', () => {
    expect(litTree.insert([])).toBeUndefined();
  });
  it('litTreeTest14', () => {
    document.body.innerHTML = `<ul id="ul"></ul>`;
    let ul = document.querySelector('#ul') as HTMLDivElement;
    expect(litTree._insertNode(ul, [])).toBeUndefined();
  });
  it('litTreeTest15', () => {
    expect(litTree.connectedCallback()).toBeUndefined();
    let mouseClickEvent: MouseEvent = new MouseEvent('click', <MouseEventInit>{ clientX: 1, clientY: 2 });
    litTree.dispatchEvent(mouseClickEvent);
  });
  it('litTreeTest16', () => {
    expect(litTree.onDrag()).toBeUndefined();
  });
  it('litTreeTest17', () => {
    expect(litTree.onDragEnd()).toBeUndefined();
  });
  it('litTreeTest18', () => {
    let ev = {
      target:{
        open:true,
        autoExpand:jest.fn(()=>true),
      },
    }
    expect(litTree.onDragStart(ev)).toBeUndefined();
  });
  it('litTreeTest19', () => {
    let ev = {
      target:{
        style:{
          backgroundColor:'#ffffff',
        }
      },
    }
    expect(litTree.onDragEnter(ev)).toBeUndefined();
  });
  it('litTreeTest20', () => {
    let ev = {
      target:{
        data:{
          key:1,
        },
      },
      currentTarget:{
        getBoundingClientRect:jest.fn(()=>true),
      },
    }
    litTree.srcDragElement = jest.fn(()=>true);
    litTree.srcDragElement.data = jest.fn(()=>true);
    litTree.srcDragElement.data.key = jest.fn(()=>true);
    expect(litTree.onDragOver(ev)).toBeUndefined();
  });
  it('litTreeTest21', () => {
    let ev = {
      target:{
        style:{
          backgroundColor:'#ffffff',
        },
        drawLine:jest.fn(()=>true),
      },
    }
    expect(litTree.onDragLeave(ev)).toBeUndefined();
  });
  it('litTreeTest22', () => {
    let ev = {
      target:{
        style:{
          backgroundColor:'#ffffff',
        },
        data:1,
        drawLine:jest.fn(()=>true),
      },
    };
    litTree.srcDragElement = jest.fn(()=>true);
    litTree.srcDragElement.data = jest.fn(()=>true);
    expect(litTree.onDrop(ev)).toBeUndefined();
  });
  it('litTreeTest23', () => {
    let arr = {
      findIndex:jest.fn(()=>true),
      splice:jest.fn(()=>true),
    };
    let d = {};
    expect(litTree.removeDataNode(arr, d)).toBeUndefined();
  });
  it('litTreeTest24', () => {
    let arr = {
      findIndex:jest.fn(()=>true),
      splice:jest.fn(()=>true),
    };
    let d = {};
    expect(litTree.addDataNode(arr, d,[],[])).toBeUndefined();
  });
});
