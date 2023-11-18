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

function getBusyTime(initFreqResult: Array<any>, initStateResult: Array<any>, sampleMap: Map<any, any>, leftStartNs: number, rightEndNs: number) {
    if (initFreqResult.length == 0) return;
    if (initStateResult.length == 0) return;
    //处理被框选的freq的第一个数据
    let includeData = initFreqResult.findIndex((a) => a.ts >= leftStartNs);
    if (includeData !== 0) {
        initFreqResult = initFreqResult.slice(
            includeData == -1 ? initFreqResult.length - 1 : includeData - 1,
            initFreqResult.length
        )
    }
    let startNS = includeData == 0 ? initFreqResult[0].ts : leftStartNs;
    if (initFreqResult[0].ts < leftStartNs && includeData !== 0) initFreqResult[0].ts - leftStartNs;
    //处理对应的state泳道被框选的第一个数据
    let includeStateData = initStateResult.findIndex((a) => a.ts >= startNS);
    if (includeStateData !== 0) {
        initStateResult = initStateResult.slice(
            includeStateData == -1 ? initStateResult.length - 1 : includeStateData - 1,
            initStateResult.length
        )
    }
    if (initStateResult[0].ts < startNS && includeStateData !== 0 && includeStateData !== -1) initStateResult[0].ts = startNS;
    //处理被框选的freq最后一个数据
    if (initFreqResult[initFreqResult.length - 1].ts != rightEndNs) {
        initFreqResult.push({
            ts: rightEndNs,
            value: initFreqResult[initFreqResult.length - 1].value,
            filterId: initFreqResult[initFreqResult.length - 1].filterId
        })
    }
    //处理被框选的freq最后一个数据
    if (initStateResult[initStateResult.length - 1].ts != rightEndNs) {
        initStateResult.push({
            ts: rightEndNs,
            value: initStateResult[initStateResult.length - 1].value
        })
    }
    let freqIndex = 1;
    let stateIndex = 1;
    let beginNs = startNS;
    //value和Id的起始值是第0项
    let freqId = initFreqResult[0].filterId;
    let freqVal = initFreqResult[0].value;
    let stateVal = initStateResult[0].value;
    //从index = 1开始循环
    while (freqIndex < initFreqResult.length && stateIndex < initStateResult.length) {
        let newBeginNs = beginNs;
        let newfreqId = freqId;
        let newfreqVal = freqVal;
        let newStateVal = stateVal;
        let busyTime = 0;
        //比较ts值，每次比较取ts相对小的那一项
        if (initFreqResult[freqIndex].ts < initStateResult[stateIndex].ts) {
            newfreqVal = initFreqResult[freqIndex].value;
            newBeginNs = initFreqResult[freqIndex].ts;
            newfreqId = initFreqResult[freqIndex].filterId;
            freqIndex++;
        } else if (initFreqResult[freqIndex].ts > initStateResult[stateIndex].ts) {
            newStateVal = initStateResult[stateIndex].value;
            newBeginNs = initStateResult[stateIndex].ts;
            stateIndex++;
        } else {
            newStateVal = initStateResult[stateIndex].value;
            newfreqVal = initFreqResult[freqIndex].value;
            newfreqId = initFreqResult[freqIndex].filterId;
            newBeginNs = initStateResult[stateIndex].ts;
            freqIndex++;
            stateIndex++;
        }
        //取state = 0的情况并根据频率去加等赋值
        if (stateVal == 0) {
            busyTime = newBeginNs - beginNs;
            if (sampleMap.has(freqId + '-' + freqVal)) {
                let obj = sampleMap.get(freqId + '-' + freqVal);
                obj.busyTime += busyTime
            }
        }
        beginNs = newBeginNs;
        freqId = newfreqId;
        freqVal = newfreqVal;
        stateVal = newStateVal
    }
}

self.onmessage = (e: MessageEvent) => {
    let leftStartNs = e.data.frqSampleParam.leftNs + e.data.frqSampleParam.recordStartNs;
    let rightEndNs = e.data.frqSampleParam.rightNs + e.data.frqSampleParam.recordStartNs;
    e.data.cpuFiliterOrder.forEach((a: number) => {
        getBusyTime(
            e.data.result.filter((f: any) => f.cpu == a),
            e.data.res.filter((f: any) => f.cpu == a),
            e.data.sampleMap,
            leftStartNs,
            rightEndNs
        )
    })
    e.data.sampleMap.forEach((a: any) => {
        a.busyTime = parseFloat((a.busyTime / 1000000.0).toFixed(6))
    })

    self.postMessage(e.data.sampleMap)
}