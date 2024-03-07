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
import fileio from '@ohos.fileio';
import { fileOpen, travelFile, createGPData, isAccess} from '../base/BaseProfilerUtils';
import { BaseProfiler } from '../base/BaseProfiler'
import { CollectorType } from '../base/ProfilerConstant'
import { GPData } from '../../entity/DatabaseEntity'
import SPLogger from '../../utils/SPLogger'
enum CPU_CONFIG {
  CPU_BASE = "/sys/devices/system/cpu",
  CPU_CUR_FREQ = "/cpufreq/scaling_cur_freq",
  CPU_LOAD = "/proc/stat"
}

export class CPU extends BaseProfiler {
  private cpuMap: Map<String, String> = new Map
  private cpuCoreNum: number
  private prebufferArr = ["", "", "", "", "", "", "", "", ""]

  public static instance: CPU = null
  public static getInstance() {
    if (this.instance == null) {
      this.instance = new CPU()
    }
    return this.instance
  }

  init() {
    //初始化CPU 核心数 节点路径
    this.cpuCoreNum = travelFile(CPU_CONFIG.CPU_BASE,"cpu[0-9]").length
    return CollectorType.TYPE_CPU
  }

  isSupport(){
    if (globalThis.useDaemon) {
      return true
    }
    return false
  }

  readData() {
    for (var i = 0;i < this.cpuCoreNum; i++) {
      const path = CPU_CONFIG.CPU_BASE + "/cpu" + i + CPU_CONFIG.CPU_CUR_FREQ
      var cpu = fileOpen(path)
      this.cpuMap.set("cpu" + i + "Freq", cpu)

    }
    return createGPData("CPU", this.cpuMap)
  }

  readCPULoad(): GPData {
    const path = CPU_CONFIG.CPU_LOAD
    var workLoadArr = []
    try {
      var fd = -1
      fd = fileio.openSync(path, 0o0)
      SPLogger.DEBUG(CPU.name,"readCPULoad fd:" + fd);
      let buf = new ArrayBuffer(2048);
      fileio.readSync(fd, buf);
      let cpuStr: String = String.fromCharCode.apply(null, new Uint8Array(buf))
      SPLogger.DEBUG(CPU.name,"readCPULoad cpuStr:" + cpuStr);
      let cpuStrArr = []
      cpuStr = cpuStr.substring(0, cpuStr.indexOf("intr"))
      let nextj = 0;
      let j
      for (var i = 1;i < cpuStr.length; i++) {
        if (cpuStr.charAt(i) == 'c') {
          j = nextj
          nextj = i
          cpuStrArr.push(cpuStr.substring(j, nextj))
        }
      }
      cpuStrArr.push(cpuStr.substring(nextj, nextj + 50))
      let buffer = ""
      for (let index = 1;index < cpuStrArr.length; index++) {
        buffer = cpuStrArr[index]
        let load = this.calCPULoad(buffer, this.prebufferArr[index])
        workLoadArr.push(load)
        this.prebufferArr[index] = buffer
      }
    } catch (err) {

    } finally {
      fileio.closeSync(fd)
    }

    let map = new Map
    for (let index = 0; index < workLoadArr.length; index++) {
      const element = workLoadArr[index];
      map.set("cpu" + index + "Load", element)
    }
    return createGPData("CPULoad", map)
  }

  calCPULoad(buffer: string, pre_buffer: string): number {

    if (pre_buffer.length == 0) {
      return -1
    }
    let timeArr: string[] = buffer.split(" ");
    let pre_timeArr: string[] = pre_buffer.split(" ");

    timeArr.reverse().pop()
    pre_timeArr.reverse().pop()
    timeArr.reverse()
    pre_timeArr.reverse()

    let time = this.ArrStr2Number(timeArr)
    let pre_time = this.ArrStr2Number(pre_timeArr)

    let user = time[0] + time[1] - pre_time[0] - pre_time[1];
    let sys = time[2] - pre_time[2];
    let idle = time[3] - pre_time[3];
    let iowait = time[4] - pre_time[4];
    let irq = time[5] + time[6] - pre_time[5] - pre_time[6];
    let total = user + sys + idle + iowait + irq;
    if (user < 0 || sys < 0 || idle < 0 || iowait < 0 || irq < 0) {
      return 0;
    }
    let per_user = user * 100.0 / total;
    let per_sys = sys * 100.0 / total;
    let per_iowait = iowait * 100.0 / total;
    let per_irq = irq * 100.0 / total;
    let workload = per_user + per_sys + per_iowait + per_irq;
    return Number(workload.toFixed(2)).valueOf()
  }

  ArrStr2Number(arr: Array<String>): Array<number> {
    let result = new Array<number>()
    for (var index = 0; index < arr.length; index++) {
      const element = arr[index].replace("\n", "");
      result.push(new Number(element).valueOf())
    }
    return result
  }
}
