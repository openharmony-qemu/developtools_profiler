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

import EntryOhos from '../data/EntryOhos';
import { LineDataSet } from '../data/LineDataSet';
import Runnable from '../data/Runnable';
import LegendEntry from '../components/LegendEntry';
import { JArrayList } from '../utils/JArrayList';
import LegendView from '../components/LegendView';
import Legend, {LegendForm, LegendVerticalAlignment} from '../components/Legend';
import MyRect from '../data/Rect';
import { TextPaint } from '../data/Paint';
import LineData from '../data/LineData';
import {XAxis} from '../components/XAxis';
import XAxisView from '../components/renderer/XAxisView';
import YAxisView from '../components/renderer/YAxisView'
import YAxis, {AxisDependency} from '../components/YAxis'
import PathView, { PathViewModel } from '../components/PathView'
import Utils from '../utils/Utils'
import YAxisRenderer from '../renderer/YAxisRenderer'
import Transformer from '../utils/Transformer'
import ViewPortHandler from '../utils/ViewPortHandler'

@Component
@Preview
export default struct LineChart {

  @State
  xStartPoint:number[] = [];
  @State
  xEndPoint:number[] = [];
  @State
  yStartPoint:number[] = [];
  @State
  yEndPoint:number[] = [];

  @State
  pathViewModel: PathViewModel = new PathViewModel();
  @State
  lineChartModel: LineChartModel = new LineChartModel();



  build() {
    Column() {
      Stack({ alignContent: Alignment.TopStart }) {
        Stack({ alignContent: Alignment.TopStart }) {
          XAxisView({
            scaleMode: this.lineChartModel.pathViewModel
          });
        }
        .clip(new Path().commands(this.lineChartModel.clipPath))
        .visibility(this.lineChartModel.isShowXAxis ? Visibility.Visible : Visibility.Hidden)

        Stack({ alignContent: Alignment.TopStart }) {
          YAxisView({ model:this.lineChartModel.pathViewModel.leftAxisModel })
          YAxisView({ model: this.lineChartModel.pathViewModel.rightAxisModel })
        }
        .visibility(this.lineChartModel.isShowXAxis ? Visibility.Visible : Visibility.Hidden)

        PathView({ model: this.lineChartModel.pathViewModel })
      }.backgroundColor(this.lineChartModel.rootViewBgColor)
      if (this.lineChartModel.isShowLegend) {
        LegendView({
          model: this.lineChartModel.legendModel
        })
      }
    }
    .width(this.lineChartModel.width)
    .height(this.lineChartModel.height)
  }

  public aboutToAppear() {

    this.initPathViewModel();

    // 数据设置 setLegend
    let entries : JArrayList<LegendEntry> = new JArrayList<LegendEntry>();
    for (let i = 0; i < this.lineChartModel.lineData.getDataSets().size(); i++) {
      let dataSet = this.lineChartModel.lineData.getDataSetByIndex(i)
      let entry = new LegendEntry(
      dataSet.getLabel(), // 设置图例的字符串,mLabel
      dataSet.getForm(), // 设置图例的形状,mShape,默认值LegendForm.SQUARE
      dataSet.getFormSize(), // 图例大小,mFormSize,默认值8
      dataSet.getFormLineWidth(), // 图例线宽,mFormLineWidth,默认值3
        null, // 设置虚线,dataSet.getFormLineDashEffect()
      dataSet.getColor() // 设置图例图形的颜色,
      )
      entries.add(entry)
    }

    this.lineChartModel.legend.setTextSize(14);
    this.lineChartModel.legend.setCustom(entries);
    this.lineChartModel.legend.setVerticalAlignment(LegendVerticalAlignment.CENTER)
    console.log('LineChart left:' + this.lineChartModel.left + ', top:' + this.lineChartModel.top + ', right:' + this.lineChartModel.right + ', bottom:' + this.lineChartModel.bottom);
  }

  private initPathViewModel() {
    this.pathViewModel.lineChartModel = this.lineChartModel
    this.pathViewModel.width = this.lineChartModel.width;
    this.pathViewModel.height = this.lineChartModel.height;
    this.pathViewModel.setIsInverted(this.lineChartModel.isInverted);
    this.pathViewModel.setYLeftAxis(this.lineChartModel.leftAxis);
    this.pathViewModel.setYRightAxis(this.lineChartModel.rightAxis);
    this.pathViewModel.setXAxis(this.lineChartModel.bottomAxis);
    this.pathViewModel.setBackgroundColor(this.lineChartModel.chartBgColor);
    this.pathViewModel.setMinOffset(this.lineChartModel.minOffset);
    this.pathViewModel.setIsShowClickValue(this.lineChartModel.isShowValue);
    this.pathViewModel.setPathViewData(this.lineChartModel.lineData);

    this.pathViewModel.leftAxisModel.setWidth(this.lineChartModel.width);
    this.pathViewModel.leftAxisModel.setHeight(this.lineChartModel.height);
    this.pathViewModel.leftAxisModel.setMinOffset(this.lineChartModel.minOffset);
    this.pathViewModel.leftAxisModel.setYAxis(this.lineChartModel.leftAxis);
    this.pathViewModel.rightAxisModel.setWidth(this.lineChartModel.width);
    this.pathViewModel.rightAxisModel.setHeight(this.lineChartModel.height);
    this.pathViewModel.rightAxisModel.setMinOffset(this.lineChartModel.minOffset);
    this.pathViewModel.rightAxisModel.setYAxis(this.lineChartModel.rightAxis);

    this.pathViewModel.xAixsMode.width = this.lineChartModel.width;
    this.pathViewModel.xAixsMode.height = this.lineChartModel.height;
    this.pathViewModel.xAixsMode.topAxis = this.lineChartModel.topAxis;
    this.pathViewModel.xAixsMode.bottomAxis = this.lineChartModel.bottomAxis;
    this.pathViewModel.xAixsMode.minOffset = this.lineChartModel.minOffset;
    this.pathViewModel.xAixsMode.clipPath = this.pathViewModel.xAixsMode.clipPath;
    this.lineChartModel.setPathViewModel(this.pathViewModel);
  }

}

export class LineChartModel {
  private topAxis: XAxis; //顶部X轴
  private bottomAxis: XAxis; //底部X轴
  private width: number = 300; //表的宽度
  private height: number = 300; //表的高度
  private rootViewBgColor: number | string | Color = Color.White; //chart区域的背景色
  private chartBgColor: number | string | Color = '#00FFFFFF'; //根布局的背景色
  private legendWidth: number = 300; //legend的宽度
  private legendHeight: number = 50; //legend的高度
  private minOffset: number = 15; //轴线偏移量
  private leftAxis: YAxis;
  private rightAxis: YAxis;
  private lineData: LineData;
  private legend: Legend = new Legend();
  private isShowLegend:boolean = true;
  private isInverted: boolean = false;
  private legendModel: LegendView.Model = new LegendView.Model();
  private leftTextWidth: number = 0;
  private rightTextWidth: number = 0;
  private top: number = 0;
  private bottom: number = 0;
  private left: number = 0;
  private right: number = 0;
  private isShowHeightlight:boolean = true;
  private isShowValue:boolean = true;
  private xScale: number = 1;
  private yLeftScale: number = 1;
  private yRightScale: number = 1;
  private value: string = '';
  private valueSize: number = 12;
  private valueWidth: number = 36;
  private valueHeight: number = 12;
  private imageWidth: number = 40;
  private imageHeight: number = 30;
  private clipPath: string = '';
  private clipYPath: string = '';
  private test: string = '';
  private isShowXAxis:boolean = true;
  private isShowYAxis:boolean = true;

  private pathViewModel: PathViewModel;

  constructor() {

  }

  public init(): void {
    let textPaint:TextPaint = new TextPaint();
    textPaint.setTextSize(this.leftAxis.getTextSize());
    this.leftTextWidth = Utils.calcTextWidth(textPaint, this.getYLongestLabel(this.leftAxis));
    this.rightTextWidth = Utils.calcTextWidth(textPaint, this.getYLongestLabel(this.rightAxis));
    this.left = this.minOffset + this.leftTextWidth;
    this.top = this.minOffset;
    this.right = this.width - this.minOffset - this.rightTextWidth;
    this.bottom = this.height - this.minOffset;
    this.lineData.mDisplayRect = new MyRect(this.left, this.top, this.right, this.bottom);

    this.xScale = (this.lineData.mDisplayRect.right - this.lineData.mDisplayRect.left) /
      (this.topAxis.getAxisMaximum() - (this.topAxis.getAxisMinimum() < 0 ? this.topAxis.getAxisMinimum() : 0));
    this.yLeftScale = (this.lineData.mDisplayRect.bottom - this.lineData.mDisplayRect.top) /
      (this.leftAxis.getAxisMaximum() - this.leftAxis.getAxisMinimum());
    this.yRightScale = (this.lineData.mDisplayRect.bottom - this.lineData.mDisplayRect.top) /
      (this.rightAxis.getAxisMaximum() - this.rightAxis.getAxisMinimum());

    this.legendModel.setLegend(this.legend)
      .setWidth(this.legendWidth)
      .setHeight(this.legendHeight)

    this.calcClipPath();
    this.calcYClipPath();
  }

  public getYLongestLabel(yAxis: YAxis): string {
    let handler: ViewPortHandler = new ViewPortHandler();

    handler.restrainViewPort(this.minOffset, this.minOffset, this.minOffset, this.minOffset)
    handler.setChartDimens(this.width, this.height);

    let mTran: Transformer = new Transformer(handler);
    var mAxisRenderer: YAxisRenderer = new YAxisRenderer(handler, yAxis, mTran);
    mAxisRenderer.computeAxis(yAxis.mAxisMinimum, yAxis.mAxisMaximum, yAxis.isInverted())

    return yAxis.getLongestLabel();
  }


  public setWidth(width: number) {
    this.width = width;
  }

  public setHeight(height: number) {
    this.height = height;
  }

  public setRootViewBgColor(rootViewBgColor: number | string | Color) {
    this.rootViewBgColor = rootViewBgColor;
  }

  public setChartBgColor(chartBgColor: number | string | Color) {
    this.chartBgColor = chartBgColor;
  }

  public setTopAxis(topAxis: XAxis) {
    this.topAxis = topAxis;
  }

  public setBottomAxis(bottomAxis: XAxis) {
    this.bottomAxis = bottomAxis;
  }

  public setLeftAxis(leftAxis: YAxis) {
    this.leftAxis = leftAxis;
  }

  public setRightAxis(rightAxis: YAxis) {
    this.rightAxis = rightAxis;
  }

  public setMinOffset(minOffset: number) {
    this.minOffset = minOffset;
  }

  public setIsShowLegend(isShowLegend: boolean) {
    this.isShowLegend = isShowLegend;
  }

  public setIsInverted(isInverted: boolean) {
    this.isInverted = isInverted;
  }

  public setIsShowHeightlight(isShowHeightlight: boolean) {
    this.isShowHeightlight = isShowHeightlight;
  }

  public setIsShowValue(isShowValue: boolean) {
    this.isShowValue = isShowValue;
  }

  public setPathViewModel(pathViewModel: PathViewModel) {
    this.pathViewModel = pathViewModel;
  }

  public calcClipPath() {
    let rect = this.lineData.mDisplayRect;
    this.clipPath = 'M' + Utils.convertDpToPixel(rect.left) + ' ' + Utils.convertDpToPixel(0) +
    'L' + Utils.convertDpToPixel(rect.right) + ' ' + Utils.convertDpToPixel(0) +
    'L' + Utils.convertDpToPixel(rect.right) + ' ' + Utils.convertDpToPixel(this.height) +
    'L' + Utils.convertDpToPixel(rect.left) + ' ' + Utils.convertDpToPixel(this.height) +
    ' Z'
  }

  public calcYClipPath() {
    let rect = this.lineData.mDisplayRect;
    this.clipYPath = 'M' + Utils.convertDpToPixel(this.minOffset / 2) + ' ' + Utils.convertDpToPixel(rect.top) +
    'L' + Utils.convertDpToPixel(this.width - this.minOffset / 2) + ' ' + Utils.convertDpToPixel(rect.top) +
    'L' + Utils.convertDpToPixel(this.width - this.minOffset / 2) + ' ' + Utils.convertDpToPixel(rect.bottom) +
    'L' + Utils.convertDpToPixel(this.minOffset / 2) + ' '+ tils.convertDpToPixel(rect.bottom) +
    ' Z'
  }

  public getPathViewModel(): PathViewModel {
    return this.pathViewModel;
  }

  public setLineData(lineData: LineData) {
    this.lineData = lineData;
  }

  public setPathViewData(lineData: LineData) {
    this.lineData = lineData;
    this.init();
    this.pathViewModel.setPathViewData(lineData);
  }

  public getLineData(): LineData {
    return this.lineData;
  }

}
