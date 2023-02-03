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

import ScaleMode from '../../data/ScaleMode';
import LimitLine,{LimitLabelPosition} from '../LimitLine';
import Paint,{LinePaint,TextPaint,PathPaint} from '../../data/Paint'
import {XAxis,XAxisPosition} from '../XAxis'
import XAxisRenderer from '../../renderer/XAxisRenderer'
import Transformer from '../..//utils/Transformer'
import ViewPortHandler from '../../utils/ViewPortHandler'
import XAixsMode from '../../data/XAixsMode';

@Component
@Preview
export default struct XAxisView {

  paints:Paint[] = []
  handler:ViewPortHandler = new ViewPortHandler();
  @State
  topAxis:XAxis = new XAxis();
  @State
  bottomAxis:XAxis = new XAxis();
  @State
  minOffset:number = 15;
  @State
  yLeftLongestLabel:string="AAA"
  @State
  yRightLongestLabel:string="AAA"
  @State
  XLimtLine:LimitLine = new LimitLine(35, "Index 10");

  @State
  scaleMode:ScaleMode=new ScaleMode()
  aboutToAppear(){
    this.scaleMode.xAixsMode.draw()
//    let minYOffset=this.topAxis.getTextSize()+this.topAxis.getYOffset();
//    this.minOffset=this.minOffset<minYOffset?minYOffset:this.minOffset
//
//    this.handler.restrainViewPort(this.minOffset,this.minOffset,this.minOffset,this.minOffset)
//    this.handler.setChartDimens(this.width,this.height);
//
//    /*this.topAxis.setLabelCount(6, false);
//    this.topAxis.setPosition(XAxisPosition.TOP);
//    this.topAxis.setAxisMinimum(0);
//    this.topAxis.setAxisMaximum(240);*/
//    let mAxisRendererTop:XAxisRenderer = new XAxisRenderer(this.handler,this.topAxis,new Transformer(this.handler));
//    mAxisRendererTop.yLeftLongestLabel=this.yLeftLongestLabel;
//    mAxisRendererTop.yRightLongestLabel=this.yRightLongestLabel;
//    mAxisRendererTop.computeAxis(this.topAxis.mAxisMinimum,this.topAxis.mAxisMaximum,false)
//    this.paints = this.paints.concat(mAxisRendererTop.renderAxisLine());
//    this.paints = this.paints.concat(mAxisRendererTop.renderAxisLabels())
//    this.paints = this.paints.concat(mAxisRendererTop.renderGridLines())
//
//    /*this.bottomAxis.setLabelCount(6, false);
//    this.bottomAxis.setPosition(XAxisPosition.BOTTOM);
//    this.bottomAxis.setAxisMinimum(0);
//    this.bottomAxis.setAxisMaximum(240);*/
//    let mAxisRendererBottom:XAxisRenderer = new XAxisRenderer(this.handler,this.bottomAxis,new Transformer(this.handler));
//    mAxisRendererBottom.yLeftLongestLabel=this.yLeftLongestLabel;
//    mAxisRendererBottom.yRightLongestLabel=this.yRightLongestLabel;
//    mAxisRendererBottom.computeAxis(this.bottomAxis.mAxisMinimum,this.bottomAxis.mAxisMaximum,false)
//    this.paints = this.paints.concat(mAxisRendererBottom.renderAxisLine());
//    this.paints = this.paints.concat(mAxisRendererBottom.renderAxisLabels())
//
//    this.paints = this.paints.concat(mAxisRendererTop.renderLimitLines());
//
//    let arr = [];
//    for(let item of this.paints){
//      if(item){
//        arr.push(item);
//      }
//    }
//    this.paints = arr;
  }

  build() {
    Stack({ alignContent: Alignment.TopStart }) {
      if(this.scaleMode.xAixsMode.paints && this.scaleMode.xAixsMode.paints.length>0) {
        ForEach(this.scaleMode.xAixsMode.paints, (item: Paint) => {
          if (item instanceof LinePaint) {
            Line()
              .startPoint(item.startPoint)
              .endPoint(item.endPoint)
              .fill(item.fill)
              .stroke(item.stroke)
              .strokeWidth(item.strokeWidth)
              .strokeDashArray(item.strokeDashArray)
              .strokeDashOffset(item.strokeDashOffset)
              .strokeOpacity(item.alpha)
              .position({ x: 0, y: 0 })
          } else if (item instanceof TextPaint) {
            Text(item.text)
              .position({ x: item.x, y: item.y })
              .fontWeight(item.typeface)
              .fontSize(item.textSize)
              .textAlign(item.textAlign)
          } else if (item instanceof PathPaint) {
            Path()
              .commands(item.commands)
              .fill(item.fill)
              .stroke(item.stroke)
              .strokeWidth(item.strokeWidth == 0 ? 1 : item.strokeWidth)
              .strokeDashArray(item.strokeDashArray)
              .strokeDashOffset(item.strokeDashOffset)
              .strokeOpacity(item.alpha)
              .position({ x: item.x, y: item.y })
          }
        }, (item: Paint) => (item.alpha + "").toString())
      }
    }
    .width(this.scaleMode.xAixsMode.width)
    .height(this.scaleMode.xAixsMode.height)
    .position({x:this.scaleMode.xAixsMode.xPosition})
  }
}