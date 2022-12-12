import { CatchTraceStatus } from './base/ProfilerConstant'

export default class WorkerHandler {
    static socketHandler(result) {
        let arr = result.data.split("$")
        switch (arr[0]) {
            case "RAM":
                globalThis.ramArr.push(arr[1])
                break;
            case "FPS":
                globalThis.fpsArr.push(arr[1])
                globalThis.fpsJitterArr.push(arr[2])
                globalThis.timerFps = arr[1]
                if (
                globalThis.catchTraceState == CatchTraceStatus.catch_trace_start ||
                globalThis.catchTraceState == CatchTraceStatus.catch_trace_finish ||
                globalThis.catchTraceState == CatchTraceStatus.catch_trace_first_running) {
                    if (globalThis.fpsJitterArr != undefined && globalThis.fpsJitterArr != null && globalThis.fpsJitterArr != "") {
                        let tempQueue: Array<String> = globalThis.fpsJitterArr
                        let curJitter = tempQueue.pop()
                        let tempJitterArr = curJitter.split("==")
                        for (var i = 0; i < tempJitterArr.length; i++) {
                            let tmp = tempJitterArr[i]
                            let jitter = parseInt(tmp) / 1e6
                            if (jitter > 100) {
                                globalThis.jitterTrace = true
                                return
                            }
                        }
                    }
                }
                break;
            case "UdpStatus":
                let isSocketConnect = Number(arr[1]).valueOf()
                if (isSocketConnect > 0) {
                    globalThis.useDaemon = true
                } else {
                    globalThis.useDaemon = false
                }
            default:
                break;
        }
    }
}