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

import { ColorUtils } from "../../component/trace/base/ColorUtils.js";
import {
    BaseStruct,
    drawFlagLine,
    drawLines,
    drawLoading,
    drawSelection, drawWakeUp,
    ns2x,
    Rect, Render,
    RequestMessage
} from "./ProcedureWorkerCommon.js";

export class FuncRender extends Render{
    render(req: RequestMessage, list: Array<any>, filter: Array<any>) {
        if (!req.params.isLive) {
            self.postMessage({
                id: req.id,
                type: req.type,
                results: req.canvas ? undefined : filter,
                hover: undefined
            });
            return;
        }
        if (req.lazyRefresh) {
            func(list, filter, req.startNS, req.endNS, req.totalNS, req.frame, req.useCache || !req.range.refresh);
        } else {
            if (!req.useCache) {
                func(list, filter, req.startNS, req.endNS, req.totalNS, req.frame, false);
            }
        }
        if (req.canvas) {
            if (req.canvas.height == 150) {
                req.canvas.width = req.frame.width;
                req.canvas.height = req.params.maxHeight;
                req.context.scale(req.params.dpr, req.params.dpr);
            }
            req.context.clearRect(0, 0, req.frame.width, req.frame.height);
            let arr = filter;
            if (arr.length > 0 && !req.range.refresh && !req.useCache && req.lazyRefresh) {
                drawLoading(req.context, req.startNS, req.endNS, req.totalNS, req.frame, arr[0].startTs, arr[arr.length - 1].startTs + arr[arr.length - 1].dur)
            }
            req.context.beginPath();
            drawLines(req.context, req.xs, req.frame.height, req.lineColor)
            FuncStruct.hoverFuncStruct = undefined;
            if (req.isHover) {
                for (let re of filter) {
                    if (re.dur == 0 || re.dur == null || re.dur == undefined) {
                        if (re.frame && req.hoverX >= re.frame.x - 5 && req.hoverX <= re.frame.x + 5 && req.hoverY >= re.frame.y + (re.depth * 20) && req.hoverY <= re.frame.y + re.frame.height + (re.depth * 20)) {
                            FuncStruct.hoverFuncStruct = re;
                            break;
                        }
                    } else {
                        if (re.frame && req.hoverX >= re.frame.x && req.hoverX <= re.frame.x + re.frame.width && req.hoverY >= re.frame.y + (re.depth * 20) && req.hoverY <= re.frame.y + re.frame.height + (re.depth * 20)) {
                            FuncStruct.hoverFuncStruct = re;
                            break;
                        }
                    }
                }
            } else {
                FuncStruct.hoverFuncStruct = req.params.hoverFuncStruct;
            }
            FuncStruct.selectFuncStruct = req.params.selectFuncStruct;
            for (let re of filter) {
                FuncStruct.draw(req.context, re, req.totalNS)
            }
            drawSelection(req.context, req.params);
            drawWakeUp(req.context, req.wakeupBean, req.startNS, req.endNS, req.totalNS, req.frame);
            req.context.closePath();
            drawFlagLine(req.context, req.flagMoveInfo, req.flagSelectedInfo, req.startNS, req.endNS, req.totalNS, req.frame, req.slicesTime);
        }
        // @ts-ignore
        self.postMessage({
            id: req.id,
            type: req.type,
            results: req.canvas ? undefined : filter,
            hover: FuncStruct.hoverFuncStruct
        });
    }
}

export function func(list: Array<any>, res: Array<any>, startNS: number, endNS: number, totalNS: number, frame: any, use: boolean) {
    if (use && res.length > 0) {
        for (let i = 0, len = res.length; i < len; i++) {
            if ((res[i].startTs || 0) + (res[i].dur || 0) >= startNS && (res[i].startTs || 0) <= endNS) {
                FuncStruct.setFuncFrame(res[i], 0, startNS, endNS, totalNS, frame)
            } else {
                res[i].frame = null;
            }
        }
        return;
    }
    res.length = 0;
    if (list) {
        let groups = list.filter(it => (it.startTs || 0) + (it.dur || 0) >= startNS && (it.startTs || 0) <= endNS).map(it => {
            FuncStruct.setFuncFrame(it, 0, startNS, endNS, totalNS, frame)
            return it;
        }).reduce((pre, current, index, arr) => {
            (pre[`${current.frame.x}`] = pre[`${current.frame.x}`] || []).push(current);
            return pre;
        }, {});
        Reflect.ownKeys(groups).map((kv => {
            let arr = (groups[kv].sort((a: any, b: any) => b.dur - a.dur));
            res.push(arr[0]);
        }));
    }
}

export class FuncStruct extends BaseStruct {
    static hoverFuncStruct: FuncStruct | undefined;
    static selectFuncStruct: FuncStruct | undefined;
    argsetid: number | undefined // 53161
    depth: number | undefined // 0
    dur: number | undefined // 570000
    funName: string | undefined //"binder transaction"
    id: number | undefined // 92749
    is_main_thread: number | undefined // 0
    parent_id: number | undefined // null
    startTs: number | undefined // 9729867000
    threadName: string | undefined // "Thread-15"
    tid: number | undefined // 2785
    identify: number | undefined
    track_id: number | undefined // 414

    static setFuncFrame(node: any, padding: number, startNS: number, endNS: number, totalNS: number, frame: any) {
        let x1: number, x2: number;
        if ((node.startTs || 0) > startNS && (node.startTs || 0) < endNS) {
            x1 = ns2x((node.startTs || 0), startNS, endNS, totalNS, frame);
        } else {
            x1 = 0;
        }
        if ((node.startTs || 0) + (node.dur || 0) > startNS && (node.startTs || 0) + (node.dur || 0) < endNS) {
            x2 = ns2x((node.startTs || 0) + (node.dur || 0), startNS, endNS, totalNS, frame);
        } else {
            x2 = frame.width;
        }
        if (!node.frame) {
            node.frame = {};
        }
        let getV: number = x2 - x1 <= 1 ? 1 : x2 - x1;
        node.frame.x = Math.floor(x1);
        node.frame.y = 0;
        node.frame.width = Math.floor(getV);
        node.frame.height = 20;
    }

    static getInt(data: FuncStruct): number {
        let str = data.funName || "";
        let sum = 0;
        for (let i = 0; i < str.length; i++) {
            sum += str.charCodeAt(i)
        }
        return (sum + (data?.depth || 0)) % ColorUtils.FUNC_COLOR.length;
    }

    static draw(ctx: CanvasRenderingContext2D, data: FuncStruct, totalNS: number) {
        if (data.frame) {
            let isBinder = FuncStruct.isBinder(data);
            if (data.dur == undefined || data.dur == null || data.dur == 0) {
            } else {
                ctx.fillStyle = ColorUtils.FUNC_COLOR[ColorUtils.hashFunc(data.funName || '', 0, ColorUtils.FUNC_COLOR.length)];//data.depth ||
                let miniHeight = 20
                ctx.fillRect(data.frame.x, data.frame.y, data.frame.width, miniHeight - padding * 2)
                if (data.frame.width > 10) {
                    ctx.fillStyle = "#fff"
                    FuncStruct.drawString(ctx, `${data.funName || ''}`, 5, data.frame)
                }
                if (FuncStruct.isSelected(data)) {
                    ctx.strokeStyle = "#000"
                    ctx.lineWidth = 2
                    ctx.strokeRect(data.frame.x, data.frame.y + 1, data.frame.width, miniHeight - padding * 2 - 2)
                }
            }
        }
    }

    static drawString(ctx: CanvasRenderingContext2D, str: string, textPadding: number, frame: Rect): boolean {
        let textMetrics = ctx.measureText(str);
        let charWidth = Math.round(textMetrics.width / str.length)
        if (textMetrics.width < frame.width - textPadding * 2) {
            let x2 = Math.floor(frame.width / 2 - textMetrics.width / 2 + frame.x + textPadding)
            ctx.fillText(str, x2, Math.floor(frame.y + frame.height / 2 + 2), frame.width - textPadding * 2)
            return true;
        }
        if (frame.width - textPadding * 2 > charWidth * 4) {
            let chatNum = (frame.width - textPadding * 2) / charWidth;
            let x1 = frame.x + textPadding
            ctx.fillText(str.substring(0, chatNum - 4) + '...', x1, Math.floor(frame.y + frame.height / 2 + 2), frame.width - textPadding * 2)
            return true;
        }
        return false;
    }

    static isSelected(data: FuncStruct): boolean {
        return (FuncStruct.selectFuncStruct != undefined &&
            FuncStruct.selectFuncStruct.startTs == data.startTs &&
            FuncStruct.selectFuncStruct.depth == data.depth)
    }

    static isBinder(data: FuncStruct): boolean {
        if (data.funName != null &&
            (
                data.funName.toLowerCase().startsWith("binder transaction async")
                || data.funName.toLowerCase().startsWith("binder async")
            )
        ) {
            return true;
        } else {
            return false;
        }
    }
}

const padding = 1;

