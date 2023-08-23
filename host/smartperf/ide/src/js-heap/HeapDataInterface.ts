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

import { HeapLoader } from './logic/HeapLoader.js';
import {
  AllocationFunction,
  ConstructorComparison,
  ConstructorItem,
  ConstructorType,
  FileInfo,
} from './model/UiStruct.js';
import { HeapNodeToConstructorItem } from './utils/Utils.js';
import { FileStruct, HeapSample, HeapTraceFunctionInfo } from './model/DatabaseStruct.js';

export interface ParseListener {
  parseDone(fileModule: Array<FileInfo>): void;
}

export class HeapDataInterface {
  private static instance: HeapDataInterface;
  private isParseDone = false;
  private parseListener!: ParseListener;
  private fileStructs!: Array<FileStruct>;
  private baseFileStruct!: FileStruct | null;

  public static getInstance() {
    if (!this.instance) {
      this.instance = new HeapDataInterface();
    }
    return this.instance;
  }

  private getFileStructById(id: number): FileStruct | null {
    for (let fileStruct of this.fileStructs) {
      if (fileStruct.id == id) {
        return fileStruct;
      }
    }
    return null;
  }

  /**
   * tell interface current file to provider file interface
   * @param fileId file id
   */
  public setFileId(fileId: number) {
    this.baseFileStruct = this.getFileStructById(fileId);
  }

  /**
   * set ParseListener to callback when node_files table in database is parse done
   * @param listener callback
   */
  public setPraseListener(listener: ParseListener) {
    this.parseListener = listener;
  }

  /**
   * obtain the list of Constructor classes
   * @returns Array<ConstructorItem>
   */
  public getClassesListForSummary(fileId: number, minNodeId?: number, maxNodeId?: number): Array<ConstructorItem> {
    let constructorMap;
    let constructorList = new Array<ConstructorItem>();
    let filStruct = this.baseFileStruct ? this.baseFileStruct : this.getFileStructById(fileId);
    if (this.isParseDone && filStruct) {
      constructorMap = filStruct.heapLoader.getClassesForSummary(minNodeId, maxNodeId);
      constructorMap.forEach((construct, _) => {
        constructorList.push(construct);
      });
      constructorList.sort(function (a, b) {
        return b.retainedSize - a.retainedSize;
      });
    }
    return constructorList;
  }

  /**
   * compare base file and target file, calculate delta size and count to target class
   * @param baseFileId current file id
   * @param targetFileId select id which file is to compare
   * @returns diff class list
   */
  public getClassListForComparison(baseFileId: number, targetFileId: number) {
    let baseFileStruct = this.baseFileStruct ? this.baseFileStruct : this.getFileStructById(baseFileId);
    let targetFileStruct = this.getFileStructById(targetFileId);
    if (!baseFileStruct || !targetFileStruct) {
      return [];
    }
    let diffClassList = new Array<ConstructorComparison>();
    let diffClassMap = baseFileStruct.heapLoader.getClassesForComparison(
      targetFileId,
      targetFileStruct.heapLoader.getClassesForSummary()
    );
    if (!diffClassMap || diffClassMap.size === 0) {
      return [];
    }

    for (let diffClass of diffClassMap.values()) {
      diffClassList.push(diffClass);
    }
    diffClassList.sort((a, b) => b.addedSize - a.addedSize);
    return diffClassList;
  }

  /**
   * get sample data for timeline
   * @param fileId timeline file id
   * @returns time stamp with size
   */
  public getSamples(fileId: number): Array<HeapSample> {
    let filStruct = this.baseFileStruct ? this.baseFileStruct : this.getFileStructById(fileId);
    if (!filStruct) {
      return [];
    }
    let samples = filStruct.snapshotStruct.samples;
    return samples;
  }

  /**
   * get the functions which call the node
   * @param node current select node
   * @returns node.parent
   */
  public getParentFunction(node: AllocationFunction) {
    let filStruct = this.baseFileStruct ? this.baseFileStruct : this.getFileStructById(node.fileId);
    if (!filStruct) {
      return;
    }
    filStruct.heapLoader.loadAllocationParent(node);
  }

  /**
   * get select node children while node type is class
   * @param node current select node
   * @returns node.children
   */
  public getNextForConstructor(node: ConstructorItem): Array<ConstructorItem> {
    let filStruct = this.baseFileStruct ? this.baseFileStruct : this.getFileStructById(node.fileId);
    let children = new Array<ConstructorItem>();
    switch (node.type) {
      case ConstructorType.ClassType:
        children = node.classChildren;
        break;
      case ConstructorType.InstanceType:
      case ConstructorType.FiledType:
        children = filStruct!.heapLoader.getNextNode(node);
        break;
    }
    return children;
  }

  public getNextForComparison(comparisonNode: ConstructorComparison): Array<ConstructorItem> {
    let baseFileStruct = this.baseFileStruct ? this.baseFileStruct : this.getFileStructById(comparisonNode.fileId);
    let targetFileStruct = this.getFileStructById(comparisonNode.targetFileId);
    if (!baseFileStruct || !targetFileStruct) {
      return [];
    }
    let children = new Array<ConstructorItem>();
    if (comparisonNode.type === ConstructorType.ComparisonType) {
      for (let idx of comparisonNode.addedIndx) {
        let node = baseFileStruct.heapLoader.getNodes()[idx];
        let compareNode = HeapNodeToConstructorItem(node);
        compareNode.type = ConstructorType.InstanceType;
        compareNode.addedSize = compareNode.shallowSize;
        compareNode.isAdd = true;
        compareNode.hasNext = node.edgeCount > 0;
        children.push(compareNode);
      }

      for (let idx of comparisonNode.deletedIdx) {
        let node = targetFileStruct.heapLoader.getNodes()[idx];
        let compareNode = HeapNodeToConstructorItem(node);
        compareNode.type = ConstructorType.InstanceType;
        compareNode.removedSize = compareNode.shallowSize;
        compareNode.isAdd = false;
        compareNode.hasNext = node.edgeCount > 0;
        children.push(compareNode);
      }
    } else {
      children = this.getNextForConstructor(comparisonNode);
    }
    return children;
  }

  /**
   * get nodes which referenced this node
   * @param constructor current node
   * @returns reference nodes
   */
  public getRetains(constructor: ConstructorItem) {
    let filStruct = this.baseFileStruct ? this.baseFileStruct : this.getFileStructById(constructor.fileId);
    if (!filStruct) {
      return [];
    }
    return filStruct?.heapLoader.getRetains(constructor);
  }

  /**
   * get AllocationStack page data
   * @param node the row of data clicked
   * @returns AllocationStackFrame[]
   */
  public getAllocationStackData(node: ConstructorItem): Array<HeapTraceFunctionInfo> {
    let functions = new Array<HeapTraceFunctionInfo>();
    let filStruct = this.baseFileStruct ? this.baseFileStruct : this.getFileStructById(node.fileId);
    if (!filStruct && (node.type == ConstructorType.ClassType || node.type == ConstructorType.RetainersType)) {
      return functions;
    } else {
      functions = filStruct!.heapLoader.getAllocationStack(node.traceNodeId);
    }
    return functions;
  }

  /**
   * obtain the minimum  id of the node
   * @param fileId  current file id
   * @returns minNodeId
   */
  public getMinNodeId(fileId: number): number | undefined {
    let filStruct = this.baseFileStruct ? this.baseFileStruct : this.getFileStructById(fileId);
    if (!filStruct) {
      return undefined;
    }
    return filStruct!.heapLoader.getMinAndMaxNodeId().minNodeId;
  }

  /**
   * obtain the maximum id of the node
   * @param fileId current file id
   * @returns maxNodeId
   */
  public getMaxNodeId(fileId: number): number | undefined {
    let filStruct = this.baseFileStruct ? this.baseFileStruct : this.getFileStructById(fileId);
    if (!filStruct) {
      return undefined;
    }
    return filStruct!.heapLoader.getMinAndMaxNodeId().maxNodeId;
  }

  async parseData(fileModule: Array<FileStruct>) {
    this.fileStructs = fileModule;
    this.isParseDone = false;
    let percent: number;
    for (let fileStruct of fileModule) {
      let heapLoader = new HeapLoader(fileStruct);
      fileStruct.heapLoader = heapLoader;
      percent = 50 + Math.floor(50 / fileModule.length) * (fileModule.indexOf(fileStruct) + 1);
    }
    this.isParseDone = true;
    if (this.parseListener) {
      this.parseListener.parseDone(fileModule);
    }
  }

  /**
   * get all file struct in database
   * @returns all fileInfo
   */
  public getFileStructs(): Array<FileInfo> {
    return this.fileStructs;
  }

  /**
   * clear Cache
   */
  public clearData() {
    if (!this.fileStructs) {
      return;
    }
    for (let file of this.fileStructs) {
      file.snapshotStruct.clear();
      file.heapLoader.clear();
    }
    this.fileStructs.length = 0;
  }
}
