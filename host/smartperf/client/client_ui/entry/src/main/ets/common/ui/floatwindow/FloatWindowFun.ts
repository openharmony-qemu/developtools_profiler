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

import wm from '@ohos.window'
import SPLogger from '../../utils/SPLogger'

const TAG = "FloatWindowFun"

export class FloatWindowFun {
    static floatingWindowOffsetX: number = 50
    static floatingWindowOffsetY: number = 300
    static titleWindowOffsetX: number = 300
    static titleWindowOffsetY: number = 200
    static lineChartWindowOffsetX: number= 700
    static lineChartWindowOffsetY: number= 200
    static windowWidth: number = 2560
    static windowHeight: number = 1600
    static atWidth: number =  95
    static initAllFun() {
        globalThis.CreateFloatingWindow = (() => {
            //5.5SP2  2106 改成 8
      wm.create(globalThis.abilityContext, 'sp_floatingWindow', wm.WindowType.TYPE_FLOAT).then((floatWin) => {
                floatWin.moveTo(this.floatingWindowOffsetX, this.floatingWindowOffsetY).then(() => {
                    floatWin.resetSize(this.atWidth*globalThis.coefficient, this.atWidth*globalThis.coefficient).then(() => {
                        floatWin.getProperties().then((property) => {
                            property.isTransparent = false
                        })
                        floatWin.loadContent('pages/FloatBall').then(() => {
                            floatWin.setBackgroundColor("#00000000").then(() => { //透明
                                floatWin.show().then(() => {
                                    globalThis.showFloatingWindow = true
                                })
                            })
                        })
                    })
                })
            })
        })
        globalThis.MoveFloatingWindow = ((offsetX: number, offsetY: number) => {
            var xx = (this.floatingWindowOffsetX + offsetX * 2) < 0 ? 0 : ((this.floatingWindowOffsetX + offsetX * 2) > (this.windowWidth - 200) ? (this.windowWidth - 200) : (this.floatingWindowOffsetX + offsetX * 2))
            var yy = (this.floatingWindowOffsetY + offsetY * 2) < 0 ? 0 : ((this.floatingWindowOffsetY + offsetY * 2) > (this.windowHeight - 200) ? (this.windowHeight - 200) : (this.floatingWindowOffsetY + offsetY * 2))

            wm.find("sp_floatingWindow").then((fltWin) => {
                fltWin.moveTo(xx, yy)
            })
        })

        globalThis.SetFloatingWindowPosition = ((offsetX: number, offsetY: number) => {
            this.floatingWindowOffsetX = (this.floatingWindowOffsetX + offsetX * 2) < 0 ? 0 : ((this.floatingWindowOffsetX + offsetX * 2) > (this.windowWidth - 200) ? (this.windowWidth - 200) : (this.floatingWindowOffsetX + offsetX * 2))
            this.floatingWindowOffsetY = (this.floatingWindowOffsetY + offsetY * 2) < 0 ? 0 : ((this.floatingWindowOffsetY + offsetY * 2) > (this.windowHeight - 200) ? (this.windowHeight - 200) : (this.floatingWindowOffsetY + offsetY * 2))
        })

        globalThis.DestroyFloatingWindow = (() => {
            wm.find("sp_floatingWindow").then((fltWin) => {
                fltWin.destroy().then(() => {
                    globalThis.showFloatingWindow = false
                })
            })
        })

        globalThis.CreateTitleWindow = (() => {
      wm.create(globalThis.abilityContext, 'sp_TitleWindow', wm.WindowType.TYPE_FLOAT).then((floatWin) => {
                floatWin.moveTo(this.titleWindowOffsetX, this.titleWindowOffsetY).then(() => {
                    floatWin.resetSize(350*globalThis.coefficient, 480*globalThis.coefficient).then(() => {
                        floatWin.getProperties().then((property) => {
                            property.isTransparent = false
                        })
                        floatWin.loadContent('pages/TitleWindowPage').then(() => {
                            floatWin.setBackgroundColor("#00000000")
                            floatWin.hide()
                            SPLogger.DEBUG(TAG, 'CreateTitleWindow Done')
                        })
                    })
                })
            })
        })

        globalThis.MoveTitleWindow = ((offsetX: number, offsetY: number) => {
            var xx = (this.titleWindowOffsetX + offsetX * 2) < 0 ? 0 : ((this.titleWindowOffsetX + offsetX * 2) > (this.windowWidth - 500) ? (this.windowWidth - 500) : (this.titleWindowOffsetX + offsetX * 2))
            var yy = (this.titleWindowOffsetY + offsetY * 2) < 0 ? 0 : ((this.titleWindowOffsetY + offsetY * 2) > (this.windowHeight - 330) ? (this.windowHeight - 330) : (this.titleWindowOffsetY + offsetY * 2))
            wm.find("sp_TitleWindow").then((fltWin) => {
                fltWin.moveTo(xx, yy)
            })
        })

        globalThis.SetTitleWindowPosition = ((offsetX: number, offsetY: number) => {
            this.titleWindowOffsetX = (this.titleWindowOffsetX + offsetX * 2) < 0 ? 0 : ((this.titleWindowOffsetX + offsetX * 2) > (this.windowWidth - 500) ? (this.windowWidth - 500) : (this.titleWindowOffsetX + offsetX * 2))
            this.titleWindowOffsetY = (this.titleWindowOffsetY + offsetY * 2) < 0 ? 0 : ((this.titleWindowOffsetY + offsetY * 2) > (this.windowHeight - 330) ? (this.windowHeight - 330) : (this.titleWindowOffsetY + offsetY * 2))
        })

        globalThis.DestroyTitleWindow = (() => {
            wm.find("sp_TitleWindow").then((fltWin) => {
                fltWin.destroy().then(() => {
                })
            })
        })

        globalThis.HideTitleWindow = (() => {
            wm.find("sp_TitleWindow").then((fltWin) => {
                fltWin.hide()
            })
        })

        globalThis.ShowTitleWindow = (() => {
            wm.find("sp_TitleWindow").then((fltWin) => {
                fltWin.show()
            })
        })

    }
}

