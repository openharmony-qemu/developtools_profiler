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

export class JSONToCSV {
  static setCsvData(obj: any) {
    let that = this;
    let browserType = this.browserType();
    if (browserType['ie'] < 9) return;
    let data = obj['data'];
    let isShowLabel = typeof obj['showLabel'] === 'undefined' ? true : obj['showLabel'];
    let fileName = (obj['fileName'] || 'UserExport') + '.csv';
    let columns = obj['columns'] || {
      title: [],
      key: [],
      formatter: undefined,
    };
    let showLabel = typeof isShowLabel === 'undefined' ? true : isShowLabel;
    let row = '', csv = '', key;
    // 如果要现实表头文字
    if (showLabel) {
      // 如果有传入自定义的表头文字
      if (columns.title.length) {
        columns.title.map(function (n: any) {
          row += n + ',';
        });
      } else {
        // 如果没有，就直接取数据第一条的对象的属性
        for (key in data[0]) row += key + ',';
      }
      row = row.slice(0, -1);
      csv += row + '\r\n';
    }
    // 具体的数据处理
    data.map(function (n: any) {
      row = '';
      // 如果存在自定义key值
      if (columns.key.length) {
        columns.key.map(function (m: any, idx: number) {
          let strItem = n[m];
          if (typeof n[m] == 'undefined') {
            strItem = '';
          } else if (typeof n[m] == 'object') {
            strItem = JSON.stringify(n[m]);
            strItem = strItem.replaceAll('"', '');
          }
          if (idx === 0 && typeof n['depthCSV'] !== 'undefined') {
            row +=
              '"' +
              that.treeDepth(n['depthCSV']) +
              (typeof columns.formatter === 'function' ? columns.formatter(m, n[m]) || n[m] : strItem) +
              '",';
          } else {
            row +=
              '"' + (typeof columns.formatter === 'function' ? columns.formatter(m, n[m]) || n[m] : strItem) + '",';
          }
        });
      } else {
        for (key in n) {
          row +=
            '"' + (typeof columns.formatter === 'function' ? columns.formatter(key, n[key]) || n[key] : n[key]) + '",';
        }
      }
      row.slice(0, row.length - 1); // 删除最后一个,
      csv += row + '\r\n'; // 添加换行符号
    });
    if (!csv) return;
    this.saveCsvFile(fileName, csv);
  }

  static saveCsvFile(fileName: any, csvData: any) {
    let browserType: any = this.browserType();
    if (!browserType['edge'] || !browserType['ie']) {
      let alink: any = document.createElement('a');
      alink.id = 'csvDownloadLink';
      alink.href = this.getDownloadUrl(csvData);
      document.body.appendChild(alink);
      let linkDom: any = document.getElementById('csvDownloadLink');
      linkDom.setAttribute('download', fileName);
      linkDom.click();
      document.body.removeChild(linkDom);
    } else if (browserType['ie'] >= 10 || browserType['edge'] == 'edge') {
      (navigator as any).msSaveBlob(new Blob(['\uFEFF' + csvData], {
        type: 'text/csv',
      }), fileName);
    } else {
      let oWin: any = window.top?.open('about:blank', '_blank');
      oWin.document.write('sep=,\r\n' + csvData);
      oWin.document.close();
      oWin.document.execCommand('SaveAs', true, fileName);
      oWin.close();
    }
  }

  static getDownloadUrl(csvData: any) {
    if (window.Blob && window.URL && (window.URL as any).createObjectURL) {
      return URL.createObjectURL(new Blob(['\uFEFF' + csvData], {
        type: 'text/csv',
      }));
    }
  }

  static browserType() {
    let type: any = {};
    let agent = navigator.userAgent.toLowerCase();
    let has;
    (has = agent.indexOf('edge') !== -1 ? (type.edge = 'edge') : agent.match(/rv:([\d.]+)\) like gecko/))
      ? (type.ie = has[1])
      : (has = agent.match(/msie ([\d.]+)/))
        ? (type.ie = has[1])
        : (has = agent.match(/firefox\/([\d.]+)/))
          ? (type.firefox = has[1])
          : (has = agent.match(/chrome\/([\d.]+)/))
            ? (type.chrome = has[1])
            : (has = agent.match(/opera.([\d.]+)/))
              ? (type.opera = has[1])
              : (has = agent.match(/version\/([\d.]+).*safari/))
                ? (type.safari = has[1])
                : 0;
    return type;
  }

  static treeDepth(depth: number) {
    let str = '';
    for (let i = 0; i < depth; i++) {
      str += '    ';
    }
    return str;
  }

  static treeToArr(data: any) {
    const result: Array<any> = [];
    data.forEach((item: any) => {
      let depthCSV = 0;
      const loop = (data: any, depth: any) => {
        result.push({depthCSV: depth, ...data});
        let child = data.children;
        if (child) {
          for (let i = 0; i < child.length; i++) {
            loop(child[i], depth + 1);
          }
        }
      };
      loop(item, depthCSV);
    });
    return result;
  }

  static columnsData(columns: Array<any>) {
    let titleList: Array<any> = [];
    let ketList: Array<any> = [];
    columns.forEach((column) => {
      let dataIndex = column.getAttribute('data-index');
      let columnName = column.getAttribute('title');
      if (columnName == '') {
        columnName = dataIndex;
      }
      if (columnName !== '  ') {
        titleList.push(columnName);
        ketList.push(dataIndex);
      }
    });
    return {
      titleList: titleList,
      ketList: ketList,
    };
  }

  static async csvExport(dataSource: { columns: any[]; tables: any[]; fileName: string }): Promise<string> {
    return new Promise((resolve) => {
      let data: any = this.columnsData(dataSource.columns);
      let columns = {
          title: data.titleList,
          key: data.ketList,
      };
      if (dataSource.tables.length > 0) {
        if (Array.isArray(dataSource.tables[0])) {
          dataSource.tables.forEach((childArr, childIndex) => {
            let resultArr = JSONToCSV.treeToArr(childArr);
            JSONToCSV.setCsvData({
              data: resultArr,
              fileName: `${dataSource.fileName}_${childIndex}`,
              columns: columns
            });
          })
        } else {
          let resultArr = JSONToCSV.treeToArr(dataSource.tables);
          JSONToCSV.setCsvData({
            data: resultArr,
            fileName: dataSource.fileName,
            columns: columns,
          });
        }
      }
      resolve('ok');
    });
  }
}
