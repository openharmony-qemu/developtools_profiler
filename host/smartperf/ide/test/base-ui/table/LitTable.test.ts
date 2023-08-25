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
import { LitTable } from '../../../dist/base-ui/table/lit-table.js';
// @ts-ignore
import { LitTableColumn } from '../../../dist/base-ui/table/lit-table-column.js';
// @ts-ignore
import { TableRowObject } from '../../../dist/base-ui/table/TableRowObject.js';
// @ts-ignore
import { LitProgressBar } from '../../../dist/base-ui/progress-bar/LitProgressBar.js';
describe('LitTable Test', () => {
  window.ResizeObserver =
    window.ResizeObserver ||
    jest.fn().mockImplementation(() => ({
      disconnect: jest.fn(),
      observe: jest.fn(),
      unobserve: jest.fn(),
    }));
  let litTable = new LitTable();
  litTable.selectable = true;
  litTable.selectable = false;
  litTable.scrollY = 'scrollY';

  litTable.dataSource = [];

  litTable.dataSource = [
    {
      id: 1,
      name: 'name',
    },
    {
      id: 2,
      name: 'nameValue',
    },
  ];
  const td = {
    style: {
      position: 'sticky',
      left: '0px',
      right: '0px',
      boxShadow: '3px 0px 5px #33333333',
    },
  };
  const placement = 'left';

  const element = {
    style: {
      display: 'none',
      transform: 'translateY',
    },
    childNodes: { forEach: true },
    onclick: 1,
  };
  const rowObject = {
    children: {
      length: 1,
    },
    data: [{ isSelected: undefined }],
    depth: 1,
    top: 1,
  };
  const firstElement =
    {
      style: {
        display: 'none',
        paddingLeft: '',
        transform: 'translateY',
      },
      innerHTML: '',
      title: '',
      firstChild: null,
      onclick: 1,
    } || undefined;

  litTable.columns = litTable.columns || jest.fn(() => true);

  litTable.tbodyElement = jest.fn(() => ({
    innerHTML: '',
  }));

  litTable.tableColumns = jest.fn(() => []);

  litTable.tableColumns.forEach = jest.fn(() => []);

  it('LitTableTest01', () => {
    expect(litTable.adoptedCallback()).toBeUndefined();
  });

  it('LitTableTest02', () => {
    litTable.ds = [
      {
        name: 'StartTime',
        value: '1s 489ms 371μs ',
      },
      {
        name: 'Duration',
        value: '6ms 440μs ',
      },
      {
        name: 'State',
        value: 'Sleeping',
      },
      {
        name: 'Process',
        value: 'hilogd [441] ',
      },
    ];
    litTable.setAttribute('selectable', '123');
    let tableColmn = document.createElement('lit-table-column') as LitTableColumn;
    tableColmn.setAttribute('title', '1');
    tableColmn.setAttribute('data-index', '1');
    tableColmn.setAttribute('key', '1');
    tableColmn.setAttribute('align', 'flex-start');
    tableColmn.setAttribute('height', '32px');
    let tableColmn1 = document.createElement('lit-table-column') as LitTableColumn;
    tableColmn1.setAttribute('title', '2');
    tableColmn1.setAttribute('data-index', '2');
    tableColmn1.setAttribute('key', '2');
    tableColmn1.setAttribute('align', 'flex-start');
    tableColmn1.setAttribute('height', '32px');

    let tableColmn2 = document.createElement('lit-table-column') as LitTableColumn;
    tableColmn2.setAttribute('title', '3');
    tableColmn2.setAttribute('data-index', '3');
    tableColmn2.setAttribute('key', '3');
    tableColmn2.setAttribute('align', 'flex-start');
    tableColmn2.setAttribute('height', '32px');
    litTable.tableColumns = [tableColmn, tableColmn1, tableColmn2];
    litTable.tbodyElement = document.createElement('div');
    expect(litTable.renderTable()).toBeUndefined();
  });

  it('LitTableTest04', () => {
    litTable.switch = document.querySelector('#switch') as HTMLInputElement;
    expect(litTable.connectedCallback()).toBeUndefined();
  });

  it('LitTableTest05', () => {
    let rowLength = litTable.getCheckRows().length == 0;
    expect(rowLength).toBeTruthy();
  });

  it('LitTableTest06', () => {
    expect(
      litTable.deleteRowsCondition(() => {
        return true;
      })
    ).toBeUndefined();
  });

  it('LitTableTest07', () => {
    expect(litTable.selectable).not.toBeUndefined();
  });

  it('LitTableTest08', () => {
    litTable.selectable = true;
    expect(litTable.selectable).toBeTruthy();
  });

  it('LitTableTest09', () => {
    expect(litTable.scrollY).not.toBeUndefined();
  });

  it('LitTableTest10', () => {
    expect(litTable.dataSource).not.toBeUndefined();
  });

  it('LitTableTest11', () => {
    expect(litTable.recycleDataSource).not.toBeUndefined();
  });

  it('LitTableTest12', () => {
    expect(litTable.fixed(td, placement)).toBeUndefined();
  });

  it('LitTableTest13', () => {
    expect(litTable.fixed(td, 'right')).toBe(undefined);
  });

  it('LitTableTest14', () => {
    expect(litTable.meauseElementHeight()).toBe(27);
  });

  it('LitTableTest15', () => {
    expect(litTable.meauseTreeElementHeight()).toBe(27);
  });

  it('LitTableTest16', () => {
    document.body.innerHTML = "<lit-table id='tab' tree></lit-table>";
    let table = document.querySelector('#tab') as LitTable;
    let htmlElement = document.createElement('lit-table-column') as LitTableColumn;
    htmlElement.setAttribute('title', '1');
    htmlElement.setAttribute('data-index', '1');
    htmlElement.setAttribute('key', '1');
    htmlElement.setAttribute('align', 'flex-start');
    htmlElement.setAttribute('height', '32px');
    table!.appendChild(htmlElement);
    setTimeout(() => {
      table.recycleDataSource = [
        {
          id: 1,
          name: 'name',
        },
        {
          id: 2,
          name: 'nameValue',
        },
      ];
      expect(table.meauseTreeElementHeight()).toBe(27);
    }, 20);
  });

  it('LitTableTest17', () => {
    expect(litTable.shadowRoot.innerHTML).toMatchInlineSnapshot(`
"
        <style>
        :host{
            display: grid;
            grid-template-columns: repeat(1,1fr);
            width: 100%;
            position: relative;
            font-weight: 500;
            flex:1;
        }
        .tr{
            display: grid;
            grid-column-gap: 5px;
            min-width:100%;
        }
        .tr:nth-of-type(even){
        }
        .tr{
            background-color: var(--dark-background,#FFFFFF);
        }
        .tr:hover{
            background-color: var(--dark-background6,#DEEDFF);
        }
        .tr[selected]{
            background-color: var(--dark-background6,#DEEDFF);
        }
        .tr[high-light]{
            font-weight: 600;
        }
        .td{
            box-sizing: border-box;
            padding: 3px;
            display: flex;
            justify-content: flex-start;
            align-items: center;
            width: 100%;
            height: auto;
            cursor: pointer;
        }
        .td text{
            overflow: hidden; 
            text-overflow: ellipsis; 
            white-space: nowrap;
        }
        .td-order{
        }
        .td-order:before{

        }
        :host([grid-line]) .td{
            border-left: 1px solid #f0f0f0;
        }
        :host([grid-line]) .td:last-of-type{
            border-right: 1px solid #f0f0f0;
        }
        .table{
            width: 100%;
             color: var(--dark-color2,#262626);
        }
        .thead{
            display: grid;
            position: sticky;
            top: 0;
            font-weight: bold;
            font-size: .9rem;
            color: var(--dark-color1,#000);
            background-color: var(--dark-background,#FFFFFF);
            z-index: 1;
        }
        .tbody{
            width: 100%;
            top: 0;
            left: 0;
            right:0;
            bottom:0;
            display: flex;
            flex-direction: row
            row-gap: 1px;
            column-gap: 1px;
        }
        .tree{
            overflow-x:hidden;
            overflow-y:hidden;
            display: grid;
            grid-template-columns: 1fr;
            row-gap: 1px;
            column-gap: 1px;
            position:relative;
        }
        .tree:hover{
            overflow-x: overlay;
        }
        .tree-first-body{
            min-width: 100%;
            box-sizing: border-box;
            display:flex;
            align-items:center;
            white-space: nowrap;
            font-weight: 500;
            cursor: pointer;
        }
        .tree-first-body[high-light]{
            font-weight: 600;
        }
        .tree-first-body:hover{
            background-color: var(--dark-background6,#DEEDFF); /*antd #fafafa 42b983*/
        }
        .body{
            display: grid;
            grid-template-columns: 1fr;
            row-gap: 1px;
            column-gap: 1px;
            flex:1;
            position: relative;
        }
        :host([grid-line])  .tbody{
            border-bottom: 1px solid #f0f0f0;
            background-color: #f0f0f0;
        }
        .th{
            grid-column-gap: 5px;
            display: grid;
            background-color: var(--dark-background,#FFFFFF);
        }

        .tree-icon{
            font-size: 1.2rem;
            width: 20px;
            height: 20px;
            padding-right: 5px;
            padding-left: 5px;
            cursor: pointer;
        }
        .tree-icon:hover{
            color: #42b983;
        }
        .row-checkbox,row-checkbox-all{

        }
        :host([no-head]) .thead{
            display: none;
        }
        .up-svg{
            position: absolute;
            right: 5px;
            top: 8px;
            bottom: 8px;
            width: 15px;
            height: 15px;
        }
        .down-svg{
            position: absolute;
            top: 8px;
            right: 5px;
            bottom: 8px;
            width: 15px;
            height: 15px;
        }
        .mouse-select{
            background-color: var(--dark-background6,#DEEDFF);
        }
        .mouse-in{
            background-color: var(--dark-background6,#DEEDFF);
        }
        .export{
            height:32px;
            width: 32px;
            cursor:pointer;
            display:none;
            align-items:center;
            justify-content:center;
            border-radius:5px;
            box-sizing: border-box;
            background-color: #000000;
            opacity: 0.3;
            position:absolute;
            right:20px;
            bottom:20px;
            z-index: 999999;
        }
        .progress{
            position: absolute;
            height: 1px;
            top: 0;
            left: 0;
            right: 0;
            z-index: 999999;
        } 
        :host([hideDownload]) .export{
            display: none;
        }
        </style>
        <lit-progress-bar id="export_progress_bar" class="progress"></lit-progress-bar>
        <slot id="slot" style="display: none"></slot>
        <slot name="head"></slot>
        <div class="export">
            <lit-icon size="18" style="color: #ffffff" name="copyhovered"></lit-icon>
        </div>
        <div class="table" style="overflow-x:auto;">
            <div class="thead"></div>
            <div class="tbody">
                <div class="tree"></div>
                <div class="body"></div>
        </div>
        </div>
        "
`);
  });

  it('LitTableTest18', () => {
    expect(litTable.createExpandBtn({ expanded: false,data:{status:true} })).not.toBeUndefined();
  });

  it('LitTableTest19', () => {
    let newTableElement = document.createElement('div');
    newTableElement.classList.add('tr');
    newTableElement.style.cursor = 'pointer';
    newTableElement.style.gridTemplateColumns = '1,2,3';
    newTableElement.style.position = 'absolute';
    newTableElement.style.top = '0px';
    newTableElement.style.left = '0px';
    litTable.currentRecycleList = [newTableElement];
    litTable.recycleDs = [{ rowHidden: false, data: { isSearch: true } }];
    litTable.tbodyElement = document.createElement('div');
    litTable.treeElement = document.createElement('div');
    litTable.tableElement = document.createElement('div');
    litTable.theadElement = document.createElement('div');
    expect(litTable.reMeauseHeight()).toBeUndefined();
  });

  it('LitTableTest20', () => {
    const rowData = {
      data: [
        {
          isSelected: undefined,
        },
      ],
    };
    litTable.columns.forEach = jest.fn(() => true);
    expect(litTable.createNewTableElement(rowData)).not.toBeUndefined();
  });

  it('LitTableTest21', () => {
    let element = document.createElement('div');
    let ch = document.createElement('div');
    element.appendChild(ch);
    let rowObject = { rowHidden: false, data: { isSearch: true } };
    let tableColmn = document.createElement('lit-table-column') as LitTableColumn;
    tableColmn.setAttribute('title', '1');
    tableColmn.setAttribute('data-index', '1');
    tableColmn.setAttribute('key', '1');
    tableColmn.setAttribute('align', 'flex-start');
    tableColmn.setAttribute('height', '32px');
    let tableColmn1 = document.createElement('lit-table-column') as LitTableColumn;
    tableColmn1.setAttribute('title', '2');
    tableColmn1.setAttribute('data-index', '2');
    tableColmn1.setAttribute('key', '2');
    tableColmn1.setAttribute('align', 'flex-start');
    tableColmn1.setAttribute('height', '32px');
    litTable.columns = [tableColmn, tableColmn1];
    expect(litTable.freshCurrentLine(element, rowObject)).toBeUndefined();
  });

  it('LitTableTest22', () => {
    litTable.recycleDs.length = 1;
    litTable.setCurrentSelection = jest.fn(() => true);
    expect(litTable.scrollToData()).toBeUndefined();
  });

  it('LitTableTest23', () => {
    litTable.recycleDs = [{ rowHidden: false, data: { isSearch: true } }];
    let dataSource = [
      {
        id: 1,
        name: 'name',
      },
      {
        id: 2,
        name: 'nameValue',
      },
    ];
    expect(litTable.expandList(dataSource)).toBeUndefined();
  });

  it('LitTableTest24', () => {
    expect(litTable.clearAllSelection()).toBeUndefined();
  });

  it('LitTableTest25', () => {
    expect(litTable.dispatchRowClickEvent({ data: { isSelected: '' } })).toBeUndefined();
  });

  it('LitTableTest26', () => {
    litTable.treeElement = jest.fn(() => undefined);
    litTable.treeElement.children = jest.fn(() => [1]);
    litTable.columns.forEach = jest.fn(() => true);
    litTable.treeElement.lastChild = jest.fn(() => true);
    litTable.treeElement.lastChild.style = jest.fn(() => true);
    expect(litTable.createNewTreeTableElement({ data: '' })).not.toBeUndefined();
  });

  it('LitTableTest27', () => {
    litTable.tableElement = jest.fn(() => undefined);
    litTable.tableElement.scrollTop = jest.fn(() => 1);
    expect(litTable.move1px()).toBeUndefined();
  });

  it('LitTableTest28', () => {
    document.body.innerHTML = `<lit-table id="aaa"></lit-table>`;
    let litTable = document.querySelector('#aaa') as LitTable;
    let tableColmn = document.createElement('lit-table-column') as LitTableColumn;
    tableColmn.setAttribute('title', '1');
    tableColmn.setAttribute('data-index', '1');
    tableColmn.setAttribute('key', '1');
    tableColmn.setAttribute('align', 'flex-start');
    tableColmn.setAttribute('height', '32px');
    let tableColmn1 = document.createElement('lit-table-column') as LitTableColumn;
    tableColmn1.setAttribute('title', '2');
    tableColmn1.setAttribute('data-index', '2');
    tableColmn1.setAttribute('key', '2');
    tableColmn1.setAttribute('align', 'flex-start');
    tableColmn1.setAttribute('height', '32px');
    litTable.tableColumns = [tableColmn, tableColmn1];
    litTable.columns = [tableColmn, tableColmn1];
    litTable.selectable = true;
    litTable.ds = [
      { id: 1, pid: null, name: '1' },
      { id: 2, pid: 1, name: '2' },
      { id: 3, pid: 1, name: '3' },
      { id: 4, pid: null, name: '4' },
      { id: 5, pid: 4, name: '4' },
      { id: 6, pid: 4, name: '5' },
      { id: 7, pid: 3, name: '7' },
      { id: 8, pid: 4, name: '8' },
    ];
    expect(litTable.renderTreeTable()).toBeUndefined();
  });

  it('LitTableTest29', () => {
    document.body.innerHTML = `<lit-table id="aaa"></lit-table>`;
    let litTable = document.querySelector('#aaa') as LitTable;
    expect(litTable.setMouseIn(true, [])).toBeUndefined();
  });

  it('LitTableTest30', () => {
    document.body.innerHTML = `<lit-table id="aaa"></lit-table>`;
    let litTable = document.querySelector('#aaa') as LitTable;
    const data = {
      isSelected: true,
    };
    expect(litTable.setCurrentSelection(data)).toBeUndefined();
  });

  it('LitTableTest31', () => {
    document.body.innerHTML = `<lit-table id="aaa"></lit-table>`;
    let litTable = document.querySelector('#aaa') as LitTable;
    litTable.formatName = true;
    expect(litTable.formatName).toBeTruthy();
  });
  it('LitTableTest32', () => {
    let litTable = new LitTable();
    expect(litTable.formatName()).toBe('');
  });

  it('LitTableTest33', () => {
    let litTable = new LitTable();
    expect(litTable.dataExportInit()).toBeUndefined();
  });
  it('LitTableTest34', () => {
    let litTable = new LitTable();
    let htmlElement = document.createElement('lit-table-column') as LitTableColumn;
    htmlElement.setAttribute('title', '1');
    htmlElement.setAttribute('data-index', '1');
    htmlElement.setAttribute('key', '1');
    htmlElement.setAttribute('align', 'flex-start');
    htmlElement.setAttribute('height', '32px');
    litTable.columns = [htmlElement];
    document.body.innerHTML = `<lit-table id="aaa"> <lit-progress-bar id="export_progress_bar" class="progress"></lit-progress-bar></lit-table>`;
    let progressBar = document.querySelector('#export_progress_bar') as LitProgressBar;
    litTable.exportProgress = progressBar;
    expect(litTable.exportData()).toBeUndefined();
  });

  it('LitTableTest35', () => {
    expect(litTable.formatExportData()).not.toBeUndefined();
  });

  it('LitTableTest36', () => {
    expect(litTable.setSelectedRow(true, [])).toBeUndefined();
  });

  it('LitTableTest37', () => {
    document.body.innerHTML = `<lit-table id="aaa"></lit-table>`;
    let litTable = document.querySelector('#aaa') as LitTable;
    litTable.setAttribute('tree', true);
    expect(litTable.dataSource).toStrictEqual([]);
  });

  it('LitTableTest38', () => {
    document.body.innerHTML = `<lit-table id="aaa"></lit-table>`;
    let litTable = document.querySelector('#aaa') as LitTable;
    litTable.rememberScrollTop = true;
    expect(litTable.recycleDataSource).toStrictEqual([]);
  });

  it('LitTableTest39', () => {
    let litTable = new LitTable();
    expect(litTable.dataExportInit()).toBeUndefined();
  });

  it('LitTableTest40', () => {
    let tableColmn = document.createElement('lit-table-column') as LitTableColumn;
    tableColmn.setAttribute('title', '1');
    tableColmn.setAttribute('data-index', '1');
    tableColmn.setAttribute('key', '1');
    tableColmn.setAttribute('align', 'flex-start');
    tableColmn.setAttribute('height', '32px');
    let tableColmn1 = document.createElement('lit-table-column') as LitTableColumn;
    tableColmn1.setAttribute('title', '2');
    tableColmn1.setAttribute('data-index', '2');
    tableColmn1.setAttribute('key', '2');
    tableColmn1.setAttribute('align', 'flex-start');
    tableColmn1.setAttribute('height', '32px');

    let tableColmn2 = document.createElement('lit-table-column') as LitTableColumn;
    tableColmn2.setAttribute('title', '3');
    tableColmn2.setAttribute('data-index', '3');
    tableColmn2.setAttribute('key', '3');
    tableColmn2.setAttribute('align', 'flex-start');
    tableColmn2.setAttribute('height', '32px');
    litTable.columns = [tableColmn, tableColmn1, tableColmn2];
    let dataSource = [
      {
        id: 1,
        name: 'name',
      },
      {
        id: 2,
        name: 'nameValue',
      },
    ];
    expect(litTable.formatExportData(dataSource)).toBeTruthy();
  });

  it('LitTableTest41', () => {
    let list = [
      {
        memoryTap: 'All Heap',
        existing: 1938,
        existingString: '1.89 Kb',
        freeByteString: '4.54 Kb',
        allocCount: 46,
        freeCount: 103,
        freeByte: 4653,
        totalBytes: 6591,
        totalBytesString: '6.44 Kb',
        maxStr: '200 byte',
        max: 200,
        totalCount: 149,
        existingValue: [1938, 6591, 566720],
      },
    ];
    LitTable.createNewTreeTableElement = jest.fn().mockResolvedValue({});
    litTable.treeElement = document.createElement('div');
    litTable.tableElement = document.createElement('div');
    litTable.setAttribute('selectable', '123');
    let tableColmn = document.createElement('lit-table-column') as LitTableColumn;
    tableColmn.setAttribute('title', '1');
    tableColmn.setAttribute('data-index', '1');
    tableColmn.setAttribute('key', '1');
    tableColmn.setAttribute('align', 'flex-start');
    tableColmn.setAttribute('height', '32px');
    let tableColmn1 = document.createElement('lit-table-column') as LitTableColumn;
    tableColmn1.setAttribute('title', '2');
    tableColmn1.setAttribute('data-index', '2');
    tableColmn1.setAttribute('key', '2');
    tableColmn1.setAttribute('align', 'flex-start');
    tableColmn1.setAttribute('height', '32px');

    let tableColmn2 = document.createElement('lit-table-column') as LitTableColumn;
    tableColmn2.setAttribute('title', '3');
    tableColmn2.setAttribute('data-index', '3');
    tableColmn2.setAttribute('key', '3');
    tableColmn2.setAttribute('align', 'flex-start');
    tableColmn2.setAttribute('height', '32px');
    litTable.columns = [tableColmn, tableColmn1, tableColmn2];
    litTable.tbodyElement = document.createElement('div');
    litTable.theadElement = document.createElement('div');
    expect(litTable.meauseTreeRowElement(list)).toBeTruthy();
  });

  it('LitTableTest42', () => {
    let list = [
      {
        memoryTap: 'All Heap',
        existing: 1938,
        existingString: '1.89 Kb',
        freeByteString: '4.54 Kb',
        allocCount: 46,
        freeCount: 103,
        freeByte: 4653,
        totalBytes: 6591,
        totalBytesString: '6.44 Kb',
        maxStr: '200 byte',
        max: 200,
        totalCount: 149,
        existingValue: [1938, 6591, 566720],
      },
    ];
    LitTable.createNewTreeTableElement = jest.fn().mockResolvedValue({});
    litTable.treeElement = document.createElement('div');
    litTable.tableElement = document.createElement('div');
    litTable.setAttribute('selectable', '123');
    let tableColmn = document.createElement('lit-table-column') as LitTableColumn;
    tableColmn.setAttribute('title', '1');
    tableColmn.setAttribute('data-index', '1');
    tableColmn.setAttribute('key', '1');
    tableColmn.setAttribute('align', 'flex-start');
    tableColmn.setAttribute('height', '32px');
    let tableColmn1 = document.createElement('lit-table-column') as LitTableColumn;
    tableColmn1.setAttribute('title', '2');
    tableColmn1.setAttribute('data-index', '2');
    tableColmn1.setAttribute('key', '2');
    tableColmn1.setAttribute('align', 'flex-start');
    tableColmn1.setAttribute('height', '32px');

    let tableColmn2 = document.createElement('lit-table-column') as LitTableColumn;
    tableColmn2.setAttribute('title', '3');
    tableColmn2.setAttribute('data-index', '3');
    tableColmn2.setAttribute('key', '3');
    tableColmn2.setAttribute('align', 'flex-start');
    tableColmn2.setAttribute('height', '32px');
    litTable.columns = [tableColmn, tableColmn1, tableColmn2];
    litTable.tbodyElement = document.createElement('div');
    litTable.theadElement = document.createElement('div');
    expect(litTable.meauseAllRowHeight(list)).toBeTruthy();
  });

  it('LitTableTest43', () => {
    let tableColmn = document.createElement('lit-table-column') as LitTableColumn;
    tableColmn.setAttribute('title', '1');
    tableColmn.setAttribute('data-index', '1');
    tableColmn.setAttribute('key', '1');
    tableColmn.setAttribute('align', 'flex-start');
    tableColmn.setAttribute('height', '32px');
    let tableColmn1 = document.createElement('lit-table-column') as LitTableColumn;
    tableColmn1.setAttribute('title', '2');
    tableColmn1.setAttribute('data-index', '2');
    tableColmn1.setAttribute('key', '2');
    tableColmn1.setAttribute('align', 'flex-start');
    tableColmn1.setAttribute('height', '32px');

    let tableColmn2 = document.createElement('lit-table-column') as LitTableColumn;
    tableColmn2.setAttribute('title', '3');
    tableColmn2.setAttribute('data-index', '3');
    tableColmn2.setAttribute('key', '3');
    tableColmn2.setAttribute('align', 'flex-start');
    tableColmn2.setAttribute('height', '32px');
    litTable.columns = [tableColmn, tableColmn1, tableColmn2];
    let dataSource = [
      {
        id: 1,
        name: 'name',
      },
      {
        id: 2,
        name: 'nameValue',
      },
    ];
    expect(litTable.formatExportCsvData(dataSource)).toBeTruthy();
  });

  it('LitTableTest44', () => {
    let element = document.createElement('div');
    litTable.tableElement = document.createElement('div');
    let firstElement = document.createElement('div');
    let ch = document.createElement('div');
    element.appendChild(ch);
    let rowObject = { rowHidden: false, data: { isSearch: true } };
    let tableColmn = document.createElement('lit-table-column') as LitTableColumn;
    tableColmn.setAttribute('title', '1');
    tableColmn.setAttribute('data-index', '1');
    tableColmn.setAttribute('key', '1');
    tableColmn.setAttribute('align', 'flex-start');
    tableColmn.setAttribute('height', '32px');
    let tableColmn1 = document.createElement('lit-table-column') as LitTableColumn;
    tableColmn1.setAttribute('title', '2');
    tableColmn1.setAttribute('data-index', '2');
    tableColmn1.setAttribute('key', '2');
    tableColmn1.setAttribute('align', 'flex-start');
    tableColmn1.setAttribute('height', '32px');
    litTable.columns = [tableColmn, tableColmn1];
    expect(litTable.freshCurrentLine(element, rowObject, firstElement)).toBeUndefined();
  });
  it('LitTableTest45', () => {
    litTable.hideDownload = true;
    expect(litTable.hideDownload).toBeTruthy();
  });
  it('LitTableTest46', () => {
    litTable.hideDownload = false;
    expect(litTable.hideDownload).not.toBeUndefined();
  });
  it('LitTableTest47', () => {
    expect(litTable.createBtn({ expanded: false,data:{status:true} })).not.toBeUndefined();
  });
  it('LitTableTest48', () => {
    expect(litTable.mouseOut()).toBeUndefined();
  });
  it('LitTableTest49', () => {
    expect(litTable.setCurrentHover({})).toBeUndefined();
  });
  it('LitTableTest50', () => {
    expect(litTable.clearAllHover({})).toBeUndefined();
  });
});
