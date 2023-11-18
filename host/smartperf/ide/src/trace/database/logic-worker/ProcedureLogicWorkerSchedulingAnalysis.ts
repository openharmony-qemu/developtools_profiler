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

import { convertJSON, getProbablyTime, LogicHandler } from './ProcedureLogicWorkerCommon.js';

export class ProcedureLogicWorkerSchedulingAnalysis extends LogicHandler {
  currentEventId: string = '';
  endTs: number = 0;
  startTs: number = 0;
  totalDur: number = 0;
  cpu: number = 0;
  freq: number = 0;
  bigCores: Array<number> = [];
  midCores: Array<number> = [];
  smallCores: Array<number> = [];
  cpuFreqMap: Map<number, Array<CpuMeasure>> = new Map<number, Array<CpuMeasure>>();
  cpuIdle0Map: Map<number, Array<CpuMeasure>> = new Map<number, Array<CpuMeasure>>();
  threadMap: Map<number, string> = new Map<number, string>();
  processMap: Map<number, string> = new Map<number, string>();
  cpuAnalysisMap: Map<string, any> = new Map<string, any>();

  clearAll() {
    this.bigCores.length = 0;
    this.midCores.length = 0;
    this.smallCores.length = 0;
    this.cpuAnalysisMap.clear();
    this.threadMap.clear();
    this.processMap.clear();
    this.cpuFreqMap.clear();
    this.cpuIdle0Map.clear();
  }

  handle(data: any): void {
    this.currentEventId = data.id;
    if (data.params.endTs) {
      this.endTs = data.params.endTs;
      this.totalDur = data.params.total;
      this.startTs = this.endTs - this.totalDur;
    }
    if (data && data.type) {
      switch (data.type) {
        case 'scheduling-clearData':
          this.clearAll();
          self.postMessage({
            id: data.id,
            action: data.action,
            results: [],
          });
          break;
        case 'scheduling-initFreqData':
          if (data.params.list) {
            this.groupFreqByCpu(convertJSON(data.params.list) || []);
            self.postMessage({
              id: data.id,
              action: data.action,
              results: [],
            });
          } else {
            this.getCpuFrequency('scheduling-initFreqData');
          }
          break;
        case 'scheduling-getProcessAndThread':
          if (data.params.list) {
            let arr = convertJSON(data.params.list) || [];
            this.handleProcessThread(arr);
            self.postMessage({
              id: data.id,
              action: data.action,
              results: [],
            });
          } else {
            this.getProcessAndThread();
          }
          break;
        case 'scheduling-getCpuIdle0':
          if (data.params.list) {
            let arr = convertJSON(data.params.list) || [];
            this.handleCPUIdle0Map(arr);
            self.postMessage({
              id: data.id,
              action: data.action,
              results: [],
            });
          } else {
            this.getCpuIdle0();
          }
          break;
        case 'scheduling-getCpuUsage':
          if (data.params.list) {
            let arr = convertJSON(data.params.list) || [];
            self.postMessage({
              id: data.id,
              action: data.action,
              results: arr,
            });
            arr = [];
          } else {
            this.getCpuUsage();
          }
          break;
        case 'scheduling-CPU Frequency':
          if (this.cpuAnalysisMap.has('freq')) {
            self.postMessage({
              id: data.id,
              action: data.action,
              results: this.cpuAnalysisMap.get('freq') || [],
            });
          } else {
            if (data.params.list) {
              let res = this.computeCpuMeasureDur(convertJSON(data.params.list) || [], 'freq');
              this.cpuAnalysisMap.set('freq', res);
              self.postMessage({
                id: data.id,
                action: data.action,
                results: res,
              });
            } else {
              this.getCpuFrequency('scheduling-CPU Frequency');
            }
          }
          break;
        case 'scheduling-CPU Frequency Thread':
          if (data.params.list) {
            self.postMessage({
              id: data.id,
              action: data.action,
              results: this.handlerFreqThreadData(convertJSON(data.params.list) || []),
            });
          } else {
            this.cpu = data.params.cpu;
            this.freq = data.params.freq;
            this.getThreadStateByCpu(data.params.cpu);
          }
          break;
        case 'scheduling-CPU Idle':
          if (this.cpuAnalysisMap.has('idle')) {
            self.postMessage({
              id: data.id,
              action: data.action,
              results: this.cpuAnalysisMap.get('idle') || [],
            });
          } else {
            if (data.params.list) {
              let res = this.computeCpuMeasureDur(convertJSON(data.params.list) || []);
              this.cpuAnalysisMap.set('idle', res);
              self.postMessage({
                id: data.id,
                action: data.action,
                results: res,
              });
            } else {
              this.getCpuIdle();
            }
          }
          break;
        case 'scheduling-CPU Irq':
          if (this.cpuAnalysisMap.has('irq')) {
            self.postMessage({
              id: data.id,
              action: data.action,
              results: this.cpuAnalysisMap.get('irq') || [],
            });
          } else {
            if (data.params.list) {
              let res = this.groupIrgDataByCpu(convertJSON(data.params.list) || []);
              this.cpuAnalysisMap.set('irq', res);
              self.postMessage({
                id: data.id,
                action: data.action,
                results: res,
              });
            } else {
              this.getCpuIrq();
            }
          }
          break;
        case 'scheduling-Thread CpuUsage':
          if (data.params.list) {
            self.postMessage({
              id: data.id,
              action: data.action,
              results: this.handlerThreadCpuUsageData(convertJSON(data.params.list) || []),
            });
          } else {
            this.bigCores = data.params.bigCores || [];
            this.midCores = data.params.midCores || [];
            this.smallCores = data.params.smallCores || [];
            this.queryThreadCpuUsage(
              data.params.bigCores || [],
              data.params.midCores || [],
              data.params.smallCores || []
            );
          }
          break;
        case 'scheduling-Thread RunTime':
          if (data.params.list) {
            let arr = convertJSON(data.params.list) || [];
            self.postMessage({
              id: data.id,
              action: data.action,
              results: arr.map((it) => {
                it.maxDurationStr = getProbablyTime(it.maxDuration);
                it.pName = this.processMap.get(it.pid) || 'null';
                it.tName = this.threadMap.get(it.tid) || 'null';
                return it;
              }),
            });
          } else {
            this.queryThreadRunTime(data.params.cpuMax);
          }
          break;
        case 'scheduling-Process ThreadCount':
          if (data.params.list) {
            self.postMessage({
              id: data.id,
              action: data.action,
              results: convertJSON(data.params.list) || [],
            });
          } else {
            this.queryProcessThreadCount();
          }
          break;
        case 'scheduling-Process SwitchCount':
          if (data.params.list) {
            let arr = convertJSON(data.params.list) || [];
            self.postMessage({
              id: data.id,
              action: data.action,
              results: arr.map((it) => {
                it.pName = this.processMap.get(it.pid) || 'null';
                it.tName = this.threadMap.get(it.tid) || 'null';
                return it;
              }),
            });
          } else {
            this.queryProcessSwitchCount();
          }
          break;
        case 'scheduling-Thread Freq':
          if (data.params.list) {
            self.postMessage({
              id: data.id,
              action: data.action,
              results: this.handlerThreadFreqData(convertJSON(data.params.list) || []),
            });
          } else {
            this.queryThreadStateByTid(data.params.tid);
          }
          break;
      }
    }
  }

  getProcessAndThread() {
    this.queryData(
      this.currentEventId,
      'scheduling-getProcessAndThread',
      `
select tid id,ifnull(name,'null') name,'t' type from thread
union all
select pid id,ifnull(name,'null') name,'p' type from process;
        `,
      {}
    );
  }

  getCpuUsage() {
    this.queryData(
      this.currentEventId,
      'scheduling-getCpuUsage',
      `
select cpu,
       sum(case
               when A.ts < B.start_ts
                   then (A.ts - B.start_ts + A.dur)
               when A.ts >= B.start_ts
                   and (A.ts + A.dur) <= B.end_ts
                   then A.dur
               when (A.ts + A.dur) > B.end_ts
                   then (B.end_ts - A.ts) end) / cast(B.end_ts - B.start_ts as float) as usage
from thread_state A,
     trace_range B
where (A.ts - B.start_ts) > 0
  and A.dur > 0
  and (A.ts + A.dur) > B.start_ts
  and cpu is not null
  and A.ts < B.end_ts
group by cpu
order by cpu;
`,
      {}
    );
  }

  getCpuFrequency(name: string) {
    this.queryData(
      this.currentEventId,
      name,
      `
select cpu,value,ts,dur
from measure left join cpu_measure_filter cmf on measure.filter_id = cmf.id
where cmf.name = 'cpu_frequency'
order by cpu,ts;
`,
      {}
    );
  }

  getThreadStateByCpu(cpu: number) {
    let sql = `
select st.tid,
       st.pid,
       dur,
       ts - tr.start_ts as ts
from thread_state st,trace_range tr
where cpu not null
  and dur > 0
  and ts > tr.start_ts
  and ts + st.dur < tr.end_ts
  and cpu = ${cpu}
order by ts;`;
    this.queryData(this.currentEventId, 'scheduling-CPU Frequency Thread', sql, {});
  }

  getCpuIdle0() {
    this.queryData(
      this.currentEventId,
      'scheduling-getCpuIdle0',
      `
select cpu,value,ts,dur
from measure left join cpu_measure_filter cmf on measure.filter_id = cmf.id
where cmf.name = 'cpu_idle' and value = 0
`,
      {}
    );
  }

  getCpuIdle() {
    this.queryData(
      this.currentEventId,
      'scheduling-CPU Idle',
      `
select cpu,value,ts,dur
from measure left join cpu_measure_filter cmf on measure.filter_id = cmf.id
where cmf.name = 'cpu_idle' and value != 0
`,
      {}
    );
  }

  getCpuIrq() {
    this.queryData(
      this.currentEventId,
      'scheduling-CPU Irq',
      `
        SELECT callid AS cpu,
        CASE WHEN cat = 'ipi' THEN 'irq' ELSE cat END AS block,
        CASE WHEN cat = 'ipi' THEN 'IPI' || name ELSE name END AS value,
	    sum( dur ) sum,
	    min( dur ) min,
	    max( dur ) max,
	    avg( dur ) avg 
    FROM
	    irq 
    WHERE
        cat = 'ipi' 
        OR cat = 'softirq' 
        OR ( cat = 'irq' AND flag = '1' ) 
    GROUP BY
        callid,
        cat,
        name;`,
      {}
    );
  }

  queryThreadCpuUsage(bigCores: number[], midCores: number[], smallCores: number[]) {
    let sql = `
        select A.pid,A.tid,A.cpu,
       sum(A.dur) as total
from thread_state A
where cpu not null
group by A.pid, A.tid,A.cpu`;
    this.queryData(this.currentEventId, 'scheduling-Thread CpuUsage', sql, {});
  }

  queryThreadRunTime(cpuMax: number) {
    let sql = `
        select (row_number() over (order by max(A.dur) desc)) no,A.tid, A.cpu,A.ts as timestamp,A.pid, max(A.dur) maxDuration
    from thread_state A, trace_range B
    where cpu not null and A.ts between B.start_ts and B.end_ts
    group by A.tid, A.pid
    order by maxDuration desc
    limit 20`;
    this.queryData(this.currentEventId, 'scheduling-Thread RunTime', sql, {});
  }

  queryProcessThreadCount() {
    this.queryData(
      this.currentEventId,
      'scheduling-Process ThreadCount',
      `
select row_number() over (order by count(tid) desc) NO,count(tid) threadNumber,p.pid,ifnull(p.name,'null') pName
from thread t
left join process p on t.ipid = p.ipid
group by p.pid, p.name
order by threadNumber desc limit 20;`,
      {}
    );
  }

  queryProcessSwitchCount() {
    this.queryData(
      this.currentEventId,
      'scheduling-Process SwitchCount',
      `
select row_number() over (order by count(a.tid) desc) NO,
       count(a.tid) as switchCount,
       a.tid,
       a.pid
from thread_state a
where cpu not null
group by a.pid,a.tid limit 20;`,
      {}
    );
  }

  queryThreadStateByTid(tid: number) {
    let sql = `
select cpu,dur,ts - tr.start_ts as ts
from thread_state st,trace_range tr
where cpu not null
  and tid = ${tid}
  and dur > 0
  and ts > tr.start_ts
  and ts + st.dur < tr.end_ts
  order by cpu,ts;`;
    this.queryData(this.currentEventId, 'scheduling-Thread Freq', sql, {});
  }

  groupIrgDataByCpu(arr: Irq[]) {
    //首先计算 每个频点的持续时间，并根据Cpu来分组
    let map: Map<number, Array<Irq>> = new Map<number, Array<Irq>>();
    let sumMap: Map<number, number> = new Map<number, number>();
    for (let i = 0, len = arr.length; i < len; i++) {
      let ca = arr[i];
      if (map.has(ca.cpu)) {
        map.get(ca.cpu)!.push(ca);
      } else {
        map.set(ca.cpu, [ca]);
      }
      sumMap.set(ca.cpu, (sumMap.get(ca.cpu) || 0) + ca.sum);
    }
    let target: Map<number, CpuAnalysis[]> = new Map<number, CpuAnalysis[]>();
    for (let key of map.keys()) {
      let cpuArr = map
        .get(key)!
        .sort((a, b) => b.sum - a.sum)
        .slice(0, 20);
      target.set(
        key,
        cpuArr.map((irqBean) => {
          return {
            cpu: irqBean.cpu,
            value: irqBean.value,
            sum: irqBean.sum,
            sumTimeStr: getProbablyTime(irqBean.sum),
            min: getProbablyTime(irqBean.min),
            max: getProbablyTime(irqBean.max),
            avg: getProbablyTime(irqBean.avg),
            minValue: irqBean.min,
            maxValue: irqBean.max,
            avgValue: irqBean.avg,
            ratio: ((irqBean.sum / (sumMap.get(key) || 1)) * 100).toFixed(2),
            block: irqBean.block,
          } as any;
        })
      );
    }
    return target;
  }

  handleProcessThread(arr: { id: number; name: string; type: string }[]) {
    this.processMap.clear();
    this.threadMap.clear();
    for (let pt of arr) {
      if (pt.type === 'p') {
        this.processMap.set(pt.id, pt.name);
      } else {
        this.threadMap.set(pt.id, pt.name);
      }
    }
  }

  handleCPUIdle0Map(arr: CpuMeasure[]) {
    this.cpuIdle0Map.clear();
    for (let i = 0, len = arr.length; i < len; i++) {
      let ca = arr[i];
      ca.ts = ca.ts - this.startTs;
      if (ca.dur === null || ca.dur === undefined) {
        ca.dur = this.totalDur - ca.ts;
      }
      if (this.cpuIdle0Map.has(ca.cpu)) {
        this.cpuIdle0Map.get(ca.cpu)!.push(ca);
      } else {
        this.cpuIdle0Map.set(ca.cpu, [ca]);
      }
    }
  }

  getEffectiveFrequencyDur(m: CpuMeasure) {
    let arr = this.cpuIdle0Map.get(m.cpu) || [];
    let filterArr = [];
    for (let it of arr) {
      if (Math.min(m.ts + m.dur, it.ts + it.dur) - Math.max(m.ts, it.ts) > 0) {
        filterArr.push(it);
      }
      if (it.ts > m.ts + m.dur) {
        break;
      }
    }
    let dur = 0;
    for (let idle of filterArr) {
      dur += Math.min(m.ts + m.dur, idle.ts + idle.dur) - Math.max(m.ts, idle.ts);
    }
    m.dur = dur;
  }

  groupFreqByCpu(arr: CpuMeasure[]) {
    let map: Map<number, Array<CpuMeasure>> = new Map<number, Array<CpuMeasure>>();
    for (let i = 0, len = arr.length; i < len; i++) {
      let ca = arr[i];
      ca.ts = ca.ts - this.startTs;
      if (ca.dur === null || ca.dur === undefined) {
        ca.dur = this.totalDur - ca.ts;
      }
      if (ca.dur > 0) {
        if (map.has(ca.cpu)) {
          map.get(ca.cpu)!.push(ca);
        } else {
          let cpuArr: CpuMeasure[] = [];
          if (ca.ts > 0) {
            cpuArr.push({
              cpu: ca.cpu,
              value: -1,
              block: '',
              ts: 0,
              dur: ca.ts,
            })
          }
          cpuArr.push(ca)
          map.set(ca.cpu, cpuArr);
        }
      }
    }
    this.cpuFreqMap.clear();
    this.cpuFreqMap = map;
  }

  //根据查询的数据，加工出CPU调度分析所需要展示的相关数据
  computeCpuMeasureDur(arr: Array<CpuMeasure>, type?: string) {
    //首先计算 每个频点的持续时间，并根据Cpu来分组
    let map: Map<number, Array<CpuMeasure>> = new Map<number, Array<CpuMeasure>>();
    let sumMap: Map<number, number> = new Map<number, number>();
    for (let i = 0, len = arr.length; i < len; i++) {
      let ca = arr[i];
      ca.ts = ca.ts - this.startTs;
      if (ca.dur === null || ca.dur === undefined) {
        ca.dur = this.totalDur - ca.ts;
      }
      if (type === 'freq') {
        this.getEffectiveFrequencyDur(ca);
      }
      if (ca.dur > 0) {
        if (map.has(ca.cpu)) {
          map.get(ca.cpu)!.push(ca);
        } else {
          map.set(ca.cpu, [ca]);
        }
        sumMap.set(ca.cpu, (sumMap.get(ca.cpu) || 0) + ca.dur);
      }
    }
    //再根据频点值进行分组求和
    let target: Map<number, CpuAnalysis[]> = new Map<number, CpuAnalysis[]>();
    for (let key of map.keys()) {
      let obj = map.get(key)!.reduce((group: any, ca) => {
        const { value } = ca;
        if (group[value]) {
          group[value].sum = group[value].sum + ca.dur;
          group[value].min = group[value].min < ca.dur ? group[value].min : ca.dur;
          group[value].max = group[value].max > ca.dur ? group[value].max : ca.dur;
          group[value].count = group[value].count + 1;
          group[value].avg = (group[value].sum / group[value].count).toFixed(2);
        } else {
          group[value] = {
            cpu: ca.cpu,
            value: ca.value,
            sum: ca.dur,
            min: ca.dur,
            max: ca.dur,
            avg: ca.dur,
            count: 1,
            ratio: '',
            block: ca.block,
          };
        }
        return group;
      }, {});
      // @ts-ignore
      let cpuArr = (Object.values(obj) as CpuAnalysis[])
        .sort((a, b) => {
          if (type === 'freq') {
            return b.sum - a.sum;
          } else {
            return a.value - b.value;
          }
        })
        .slice(0, 20);
      target.set(
        key,
        cpuArr.map((cpuAnalysisBean) => {
          return {
            cpu: cpuAnalysisBean.cpu,
            value: cpuAnalysisBean.value,
            sum: cpuAnalysisBean.sum,
            sumTimeStr: getProbablyTime(cpuAnalysisBean.sum),
            min: getProbablyTime(cpuAnalysisBean.min),
            minValue: cpuAnalysisBean.min,
            max: getProbablyTime(cpuAnalysisBean.max),
            maxValue: cpuAnalysisBean.max,
            avgValue: cpuAnalysisBean.avg,
            avg: getProbablyTime(cpuAnalysisBean.avg),
            count: cpuAnalysisBean.count,
            ratio: ((cpuAnalysisBean.sum / (sumMap.get(key) || 1)) * 100).toFixed(2),
            block: cpuAnalysisBean.block,
          } as any;
        })
      );
    }
    return target;
  }

  handlerFreqThreadData(arr: FreqThread[]) {
    let cpuFreqArr: CpuMeasure[] = (this.cpuFreqMap.get(this.cpu) || []).filter((it) => it.value === this.freq);
    let map: Map<number, { tid: number; tName: string; pid: number; pName: string; dur: number; durStr: string; ratio: string; }> = new Map();
    let sumFreqDur = 0;
    cpuFreqArr.map((it) => {
      sumFreqDur += it.dur;
      let freqEndTs = it.ts + it.dur;
      let threads = arr.filter((f) => Math.min(f.ts + f.dur, freqEndTs) - Math.max(f.ts, it.ts) > 0);
      for (let tf of threads) {
        let tfEndTs = tf.ts + tf.dur;
        let dur = Math.min(tfEndTs, tfEndTs) - Math.max(it.ts, tf.ts);
        if (map.has(tf.tid)) {
          map.get(tf.tid)!.dur = map.get(tf.tid)!.dur + dur;
          map.get(tf.tid)!.durStr = getProbablyTime(map.get(tf.tid)!.dur);
        } else {
          map.set(tf.tid, {
            tid: tf.tid,
            tName: this.threadMap.get(tf.tid) || 'null',
            pid: tf.pid,
            pName: this.processMap.get(tf.pid) || 'null',
            dur: dur,
            ratio: '0',
            durStr: getProbablyTime(dur),
          });
        }
      }
    });
    let target = Array.from(map.values()).sort((a, b) => b.dur - a.dur);
    return target
      .map((it) => {
        it.ratio = ((it.dur / sumFreqDur) * 100).toFixed(2);
        return it;
      })
      .slice(0, 20);
  }

  //加工Top20线程大中小核占用率数据
  handlerThreadCpuUsageData(arr: Array<ThreadCpuUsage>) {
    let sumBig = 0,
      sumMid = 0,
      sumSmall = 0;
    let reduceObj = arr.reduce((group: any, item) => {
      const { tid } = item;
      let tidObj: any = group[`${tid}`];
      let cpuType: string = 'mid';
      // @ts-ignore
      if (this.bigCores.includes(item.cpu)) {
        cpuType = 'big';
        sumBig += item.total;
      }
      // @ts-ignore
      if (this.midCores.includes(item.cpu)) {
        cpuType = 'mid';
        sumMid += item.total;
      }
      // @ts-ignore
      if (this.smallCores.includes(item.cpu)) {
        cpuType = 'small';
        sumSmall += item.total;
      }
      if (tidObj) {
        tidObj.big += cpuType === 'big' ? item.total : 0;
        tidObj.mid += cpuType === 'mid' ? item.total : 0;
        tidObj.small += cpuType === 'small' ? item.total : 0;
        tidObj.total += item.total;
        tidObj[`cpu${item.cpu}`] = item.total;
      } else {
        group[`${tid}`] = {
          pid: item.pid,
          pName: this.processMap.get(item.pid) || 'null',
          tid: item.tid,
          tName: this.threadMap.get(item.tid) || 'null',
          total: item.total,
          big: cpuType === 'big' ? item.total : 0,
          mid: cpuType === 'mid' ? item.total : 0,
          small: cpuType === 'small' ? item.total : 0,
        };
        group[`${tid}`][`cpu${item.cpu}`] = item.total;
      }
      return group;
    }, {});
    // @ts-ignore
    let source: any[] = Object.values(reduceObj) as any[];
    for (let obj of source) {
      obj['bigPercent'] = sumBig === 0 ? '0' : ((obj.big / sumBig) * 100).toFixed(2);
      obj['midPercent'] = sumMid === 0 ? '0' : ((obj.mid / sumMid) * 100).toFixed(2);
      obj['smallPercent'] = sumSmall === 0 ? '0' : ((obj.small / sumSmall) * 100).toFixed(2);
      obj['bigTimeStr'] = getProbablyTime(obj.big);
      obj['midTimeStr'] = getProbablyTime(obj.mid);
      obj['smallTimeStr'] = getProbablyTime(obj.small);
    }
    let map: Map<string, Array<ThreadCpuUsage>> = new Map<string, Array<ThreadCpuUsage>>();
    map.set('total', source.sort((a, b) => b.total - a.total).slice(0, 20));
    map.set('big', source.sort((a, b) => b.big - a.big).slice(0, 20));
    map.set('mid', source.sort((a, b) => b.mid - a.mid).slice(0, 20));
    map.set('small', source.sort((a, b) => b.small - a.small).slice(0, 20));
    return map;
  }

  handlerThreadFreqData(
    arr: {
      cpu: number;
      dur: number;
      ts: number;
      freqArr: { cpu: number; freq: number; dur: number }[];
    }[]
  ) {
    let sumDur: number = 0;
    arr.map((it) => {
      it.freqArr = [];
      let itEndTs = it.ts + it.dur;
      let freqArr: CpuMeasure[] = this.cpuFreqMap.get(it.cpu) || [];
      let threadFreqArr = freqArr.filter(
        (f) =>
          (it.ts >= f.ts && it.ts <= f.ts + f.dur) ||
          (it.ts <= f.ts && itEndTs >= f.ts + f.dur) ||
          (itEndTs > f.ts && itEndTs <= f.ts + f.dur)
      );
      for (let tf of threadFreqArr) {
        let tfEndTs = tf.ts + tf.dur;
        it.freqArr.push({
          cpu: it.cpu,
          freq: tf.value as number,
          dur: Math.min(itEndTs, tfEndTs) - Math.max(it.ts, tf.ts),
        });
      }
      sumDur += it.dur;
      return it;
    });
    let obj: any = arr.reduce((group: any, tf) => {
      for (let fa of tf.freqArr) {
        const { cpu, freq } = fa;
        if (group[`${cpu}-${freq}`]) {
          group[`${cpu}-${freq}`].time = group[`${cpu}-${freq}`].time + fa.dur;
          group[`${cpu}-${freq}`].timeStr = getProbablyTime(group[`${cpu}-${freq}`].time);
          group[`${cpu}-${freq}`].ratio = ((group[`${cpu}-${freq}`].time / sumDur) * 100).toFixed(2);
        } else {
          group[`${cpu}-${freq}`] = {
            freq: freq,
            cpu: cpu,
            time: fa.dur,
            timeStr: getProbablyTime(fa.dur),
            ratio: ((fa.dur / sumDur) * 100).toFixed(2),
            totalDur: sumDur,
          };
        }
      }
      return group;
    }, {});
    let target: {
      cpu: number;
      freq: number;
      time: number;
      ratio: string;
      totalDur: number;
      // @ts-ignore
    }[] = Object.values(obj);
    return target.sort((a, b) => b.time - a.time);
  }
}

export class CpuUsage {
  cpu: number = 0;
  usage: number = 0;
}

export class Irq {
  cpu: number = 0;
  value: string = '';
  block: string = '';
  max: number = 0;
  min: number = 0;
  avg: number = 0;
  sum: number = 0;
  ratio: string = '';
}

export class CpuMeasure {
  cpu: number = 0;
  value: number | string = 0;
  block: string = '';
  ts: number = 0;
  dur: number = 0;
}

export class CpuAnalysis {
  cpu: number = 0;
  value: number = 0;
  sum: number = 0;
  min: number = 0;
  max: number = 0;
  avg: number = 0;
  count: number = 0;
  ratio: string = '';
  block: string = '';
}

export class ThreadCpuUsage {
  cpu: number = 0;
  pid: number = 0;
  pName: string = '';
  tid: number = 0;
  tName: string = '';
  total: number = 0;
  big: number = 0;
  mid: number = 0;
  small: number = 0;
  bigPercent: string = '';
  bigTimeStr: string = '';
  midPercent: string = '';
  midTimeStr: string = '';
  smallPercent: string = '';
  smallTimeStr: string = '';
}

export class FreqThread {
  pid: number = 0;
  pName: string = '';
  tid: number = 0;
  tName: string = '';
  dur: number = 0;
  durStr: string = '';
  ts: number = 0;
  freq: number = 0;
}
