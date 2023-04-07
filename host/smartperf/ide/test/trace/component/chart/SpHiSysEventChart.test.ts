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
import {SpHiSysEventChart} from "../../../../dist/trace/component/chart/SpHiSysEventChart.js"
import "../../../../dist/trace/component/chart/SpHiSysEventChart.js";
// @ts-ignore
import {SpChartManager} from "../../../../dist/trace/component/chart/SpChartManager.js";
import "../../../../dist/trace/component/chart/SpChartManager.js";
// @ts-ignore
import {SpSystemTrace} from "../../../../dist/trace/component/SpSystemTrace.js";
import "../../../../dist/trace/component/SpSystemTrace.js";
// @ts-ignore
import {LitPopover} from "../../../../dist/base-ui/popover/LitPopoverV.js";

window.ResizeObserver = window.ResizeObserver ||
    jest.fn().mockImplementation(() => ({
        disconnect: jest.fn(),
        observe: jest.fn(),
        unobserve: jest.fn(),
    }));

const sqlite = require("../../../../dist/trace/database/SqlLite.js")
jest.mock("../../../../dist/trace/database/SqlLite.js");

describe('SpHiSysEventChart Test', () => {
    let ss = new SpChartManager();
    let spHiSysEventChart = new SpHiSysEventChart(ss);

    let htmlDivElement = document.createElement<LitPopover>("div");
    htmlDivElement.setAttribute("id", "appNameList")

    let anomalyData = sqlite.queryAnomalyData;
    anomalyData.mockResolvedValue([])

    let maxStateValue = sqlite.queryMaxStateValue;
    let max = [{
        maxValue: 200,
        type: "state"
    }, {
        maxValue: 300,
        type: "sensor"
    }]
    maxStateValue.mockResolvedValue(max)
    let MockExits = sqlite.queryHisystemEventExits
    MockExits.mockResolvedValue(["trace_hisys_event"])
    let powerData = sqlite.queryPowerData;
    let power = [
        {
            startNS: 5999127353,
            eventName: "POWER_IDE_AUDIO",
            appkey: "APPNAME",
            eventValue: "com.example.himusicdemo,com.example.himusicdemo_js,com.example.himusicdemo_app",
        }, {
            startNS: 5999127353,
            eventName: "POWER_IDE_AUDIO",
            appkey: "BACKGROUND_ENERGY",
            eventValue: "854,258,141",
        }
    ]
    powerData.mockResolvedValue(power)

    let stateData = sqlite.queryStateData;
    stateData.mockResolvedValue([])

    let sysEventAppName = sqlite.querySyseventAppName;
    let appName = [{
        string_value: "app_name"
    }]
    sysEventAppName.mockResolvedValue(appName)

    let systemData = sqlite.querySystemData;
    let sys = [
        {
            ts: 1005938319,
            eventName: "WORK_ADD",
            appkey: "TYPE",
            Value: "1",
        }, {
            ts: 3005933657,
            eventName: "POWER_RUNNINGLOCK",
            appkey: "TAG",
            Value: "DUBAI_TAG_RUNNINGLOCK_REMOVE",
        }
    ]
    systemData.mockResolvedValue(sys)

    it('spHiSysEventChartTest01', function () {
        spHiSysEventChart.init();
        expect(SpHiSysEventChart.app_name).toBeUndefined();
    });

    it('spHiSysEventChartTest02', function () {
        let result = [
            {
                ts: 1005938319,
                eventName: "WORK_ADD",
                appkey: "TYPE",
                Value: "1",
            }, {
                ts: 3005933657,
                eventName: "POWER_RUNNINGLOCK",
                appkey: "TAG",
                Value: "DUBAI_TAG_RUNNINGLOCK_REMOVE",
            }, {
                ts: 4005938319,
                eventName: "GNSS_STATE",
                appkey: "STATE",
                Value: "stop",
            }, {
                ts: 5005933657,
                eventName: "POWER_RUNNINGLOCK",
                appkey: "TAG",
                Value: "DUBAI_TAG_RUNNINGLOCK_ADD",
            }, {
                ts: 6005938319,
                eventName: "GNSS_STATE",
                appkey: "STATE",
                Value: "start",
            }, {
                ts: 9005938319,
                eventName: "WORK_STOP",
                appkey: "TYPE",
                Value: "1",
            }, {
                ts: 10005938319,
                eventName: "WORK_REMOVE",
                appkey: "TYPE",
                Value: "1",
            }
        ]
        expect(spHiSysEventChart.getSystemData(result)).toEqual([{
            "dur": 3005933657,
            "isHover": false,
            "startNs": 0,
            "type": 1
        }, {"dur": 4005938319, "isHover": false, "startNs": 0, "type": 2}, {
            "dur": 9000000000,
            "isHover": false,
            "startNs": 1005938319,
            "type": 0
        }, {"dur": 9000000000, "isHover": false, "startNs": 1005938319, "type": 0}, {
            "dur": undefined,
            "isHover": false,
            "startNs": 5005933657,
            "type": 1
        }, {"dur": undefined, "isHover": false, "startNs": 6005938319, "type": 2}]);
    });

    it('spHiSysEventChartTest03', function () {
        expect(spHiSysEventChart.getSystemData([])).toEqual([]);
    });

    it('spHiSysEventChartTest04', function () {
        let result = [
            {
                startNS: 5999127353,
                eventName: "POWER_IDE_AUDIO",
                appkey: "APPNAME",
                eventValue: "com.example.himusicdemo,com.example.himusicdemo_js,com.example.himusicdemo_app",
            }, {
                startNS: 5999127353,
                eventName: "POWER_IDE_AUDIO",
                appkey: "BACKGROUND_ENERGY",
                eventValue: "854,258,141",
            }, {
                startNS: 5999127353,
                eventName: "POWER_IDE_BLUETOOTH",
                appkey: "APPNAME",
                eventValue: "com.ohos.settings,bt_switch,bt_switch_js,bt_switch_app",
            }, {
                startNS: 5999127353,
                eventName: "POWER_IDE_BLUETOOTH",
                appkey: "BACKGROUND_ENERGY",
                eventValue: "76,12,43,431",
            }, {
                startNS: 5999127388,
                eventName: "POWER_IDE_CAMERA",
                appkey: "APPNAME",
                eventValue: "com.ohos.camera,com.ohos.camera_app,com.ohos.camera_js,com.ohos.camera_ts",
            }, {
                startNS: 5999127388,
                eventName: "POWER_IDE_CAMERA",
                appkey: "BACKGROUND_ENERGY",
                eventValue: "375,475,255,963",
            }
        ]
        expect(spHiSysEventChart.getPowerData(result)).toStrictEqual(Promise.resolve());
    });

    it('spHiSysEventChartTest05', function () {
        expect(spHiSysEventChart.getPowerData([])).toStrictEqual(Promise.resolve());
    });

    it('spHiSysEventChartTest6', function () {
        expect(spHiSysEventChart.initHtml).toMatchInlineSnapshot(`undefined`);
    });

    it('spHiSysEventChartTest7', function () {
        expect(htmlDivElement.onclick).toBe(null);
    });
})