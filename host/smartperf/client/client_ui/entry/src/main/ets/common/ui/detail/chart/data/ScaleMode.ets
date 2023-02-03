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

import XAixsMode from './XAixsMode';
import { YAxisModel } from '../components/renderer/YAxisView';

export default class ScaleMode {
    width: number = 300; //表的宽度
    height: number = 300; //表的高度
    scaleX: number= 1 //当前X累计放大倍数
    scaleY: number= 1 //当前Y累计放大倍数
    preTouchScaleX: number= 1 //上一次 手势、双击缩放后的缩放

    moveX: number= 0 //累计平移X
    moveY: number= 0 //累计平移Y
    currentMoveX:number=0//当前次的位移
    currentMoveY:number=0//当前次的位移
    startMoveX: number= 0
    startMoveY: number= 0

    currentXSpace:number=0//缩放之后 需要移回的距离
    currentYSpace:number=0//缩放之后 需要移回的距离


    isZoom: boolean= false
    centerX: number= 0 //双指缩放、双击的中心点X
    centerY: number= 0 //双指缩放、双击的中心点Y
    firstTouch: TouchObject //第一个手指按下点
    secondTouch: TouchObject //第二个手指按下点
    firstClickTime: number= 0 //第一次点击的时间 用于判断是否为双击

    onDoubleClickScale:number=0.1;//双击放大倍数 增加0.4倍
    leftAxisModel:YAxisModel = new YAxisModel();
    rightAxisModel:YAxisModel = new YAxisModel();

    xAixsMode:XAixsMode=new XAixsMode()
    isNeedScale:boolean=true

    public touchScale(xScale: number, yScale: number,event: TouchEvent) {
        this.scaleX = (xScale * this.preTouchScaleX)
        this.scaleY = (yScale * this.preTouchScaleX)
        if (this.scaleX < 1) {
            this.scaleX = 1
            this.centerX = 0
        }
        if (this.scaleY < 1) {
            this.scaleY = 1
            this.centerY = 0
        }
        this.xAixsMode.width=this.width*this.scaleX
        this.onScale(event)
        this.leftAxisModel.scale(this.scaleY)
        this.rightAxisModel.scale(this.scaleY)
    }

    public doubleClickScale(event: ClickEvent,xScale: number, yScale: number) {
        this.scaleX += xScale
        this.scaleY += yScale
        this.preTouchScaleX = this.scaleX
        if (this.scaleX < 1) {
            this.scaleX = 1
            this.centerX = 0
        }
        if (this.scaleY < 1) {
            this.scaleY = 1
            this.centerY = 0
        }
        this.xAixsMode.width=this.width*this.scaleX
        this.onDoubleClick(event)
        this.leftAxisModel.scale(this.scaleY)
        this.rightAxisModel.scale(this.scaleY)
    }

    public move(moveX, moveY,event: TouchEvent) {
        this.currentMoveX = moveX
        this.currentMoveY = moveY
        this.moveX += moveX
        this.moveY += moveY
        this.onMove(event)
    }
    public setXPosition(xPosition:number){
        this.xAixsMode.xPosition=xPosition
        this.xAixsMode.draw()
    }
    /**
     * 重置
     */
    public reset() {
        this.scaleX = 1
        this.scaleY = 1
        this.moveX = 0
        this.moveY = 0
        this.centerX = 0
        this.centerY = 0
        this.preTouchScaleX = 1
        this.currentXSpace=0
        this.currentYSpace=0
    }

    /**
     * view 调用 点击事件 此事件分发单击、双击事件
     * @param event
     */
    public onClick(event: ClickEvent){
        let currnetTime=new Date().getTime();
        let spaceTime=currnetTime-this.firstClickTime
        this.firstClickTime=currnetTime

        if(spaceTime<300){
            /**
             * 双击放大事件
             */
            this.centerX=event.x;
            this.centerY=event.y;
            this.doubleClickScale(event,this.onDoubleClickScale,this.onDoubleClickScale)
        }else{
            //单击
            this.onSingleClick(event)
        }

    }
    /**
     * view 调用的触摸事件 此事件可分发 缩放、平移两个事件，缩放 监听为两指
     * @param event
     */
    public onTouch(event: TouchEvent){
        /**
         * 监听按下手指的数量，最大监听两个手指
         */
        if (event.type === TouchType.Down) {
            this.firstTouch=event.touches[0]
            if(event.touches.length>1){
                this.secondTouch=event.touches[1]
            }else{
                this.startMoveX=0
                this.startMoveY=0
            }
        }
        if (event.type === TouchType.Up) {
            //双指 突然变一指 后，界面将不在移动
            if(event.touches.length==1){
                if(this.isZoom==true){
                    this.preTouchScaleX=this.scaleX
                }
                this.isZoom=false
            }
        }
//        if (event.type === TouchType.Move) {
//
//            if(event.touches.length==1&&!this.isZoom){
//                /**
//                 * 平移
//                 */
//                let spaceX=event.touches[0].x-this.firstTouch.x
//                let spaceY=event.touches[0].y-this.firstTouch.y
//                this.move(spaceX-this.startMoveX,spaceY-this.startMoveY,event)
//                this.startMoveX=spaceX
//                this.startMoveY=spaceY
//            }else{
//                /**
//                 * 缩放
//                 */
//                this.isZoom=true
//                //第一次双指按下的距离
//                let startSpaceX=Math.abs(this.firstTouch.x-this.secondTouch.x)
//                let startSpaceY=Math.abs(this.firstTouch.y-this.secondTouch.y)
//                //当前双指的距离
//                let spaceX=Math.abs(event.touches[0].x-event.touches[1].x)
//                let spaceY=Math.abs(event.touches[0].y-event.touches[1].y)
//                //当前双指距离 相对 第一次双指 之间的距离
//                let moveSpaceX=spaceX-startSpaceX
//                let moveSpaceY=spaceY-startSpaceY
//
//                //缩放倍数
//                let scalcX=Math.abs((this.width+moveSpaceX)/this.width)
//                let scalcY=Math.abs((this.height+moveSpaceY)/this.height)
//
//
//                //双指位移后的中心点
//                this.centerX=Math.abs(event.touches[0].x+event.touches[1].x)/2;
//                this.centerY=Math.abs(event.touches[0].y+event.touches[1].y)/2;
//
//                this.touchScale(scalcX,scalcY,event)
//            }
//        }
    }
    /**
     * 需要覆写的 单击事件
     * @param event
     */
    public onSingleClick(event: ClickEvent){
        /* scaleX: number= 1 //当前X累计放大倍数
         scaleY: number= 1 //当前Y累计放大倍数
         moveX: number= 0 //累计平移X
         moveY: number= 0 //累计平移Y
         centerX: number= 0 //双指缩放、双击的中心点X
         centerY: number= 0 //双指缩放、双击的中心点Y*/
        //参数可在子类直接调用
    }
    /**
    * 需要覆写的 双击击事件
    * @param event
    */
    public onDoubleClick(event: ClickEvent){

    }
    /**
     * 需要覆写的平移事件
     * @param event
     */
    public onMove(event: TouchEvent){

    }
    /**
     * 需要覆写的平移事件 的缩放事件
     * @param event
     */
    public onScale(event: TouchEvent){

    }


}