import router from '@system.router';
import { CollectItem, AppInfoItem } from '../common/entity/LocalConfigEntity';
import { StartTestTitleComponent } from '../common/ui/StartTestTitleComponent';
import prompt from '@system.prompt';
import promptAction from '@ohos.promptAction'

const TAG = 'FrameRateTestPage'


@Entry
@Component
struct FrameRateTestPage {
  @State private curAppList:Array<AppInfoItem> = globalThis.appList
  @State selectApps: Array<String> = new Array<String>()
  @State private collectConfigs: CollectItem[] = []

  aboutToAppear() {
    this.collectConfigs.push(
      new CollectItem('启动帧率', true, true),
      new CollectItem('退出帧率', true, false),
      new CollectItem('切换帧率', true, false),
      new CollectItem('滑动帧率', true, false)
    )
  }

  delItemArray(id: string, myArray: Array<String>) {
    for (let i = 0; i < myArray.length; i++) {
      if (myArray[i]  === id) {
        myArray.splice(i, 1)
      }
    }
  }
  build() {
    Column() {
      StartTestTitleComponent({ title: '帧率测试'})
      Column() {
        if (this.curAppList == null || this.curAppList == undefined) {
          Column() {
            LoadingProgress().width(50).height(50).color(0x606266)
            Text('加载中...').fontSize(12).fontColor(0x606266).margin({ left: 10, right: 12})
          }.alignItems(HorizontalAlign.Center).padding({top:'50%'})
        } else {
          Scroll() {
            Column() {
              Divider().strokeWidth(8).color('#F1F3F5')
              Grid() {
                ForEach(this.curAppList, (appInfoItem: AppInfoItem) => {
                  GridItem() {
                    Column() {
                      Image(appInfoItem.appIcon ? appInfoItem.appIcon : $r('app.media.icon'))
                        .width(40)
                        .height(40)
                      Flex({ justifyContent: FlexAlign.Center, alignItems: ItemAlign.Center}) {
                        Radio({ value: 'Radio1', group: 'radioGroup1'})
                          .onChange((value: boolean) => {
                            if (value) {
                              this.selectApps.push(appInfoItem.packageName + ':' + appInfoItem.appName)
                            }
                            if(!value) {
                              this.delItemArray(appInfoItem.packageName + ':' + appInfoItem.appName, this.selectApps)
                            }
                          })
                          .width(10)
                          .height(10)
                        Text(appInfoItem.appName).fontSize(12).fontColor('#182431')
                      }
                      .padding({top:'5vp'})
                    }
                    .width('70vp')
                    .height('70vp')
                  }
                  .width('70vp')
                  .height('70vp')
                }, appInfoItem => appInfoItem.packageName)
              }
              .columnsGap(20)
              .rowsGap(5)
              .margin({ top: '4vp' })
              .padding({ top: '15vp' })
            }
            .margin({ top: '-10vp' })
            .backgroundColor('#ffffff')
            .borderRadius('15')
          }
          .margin({ top: '0vp' })
          .scrollBarColor('#ffffff')
        }
      }
      .height('59%')
      .width('94%')
      .backgroundColor('#ffffff')
      .borderRadius('10')
      .padding({ top: '0vp', right: '5vp', bottom: '0vp', left: '5vp'})
      .margin({ top: '12vp', right: 'auto', left: 'auto'})
      // 帧率类型选择
      Column() {
        Row() {
          Row() {
            Text('启动帧率')
            Radio({ value: 'startFrame', group: 'radioGroup'}).checked(false)
              .height(18)
              .width(18)
              .onChange((isChecked:boolean) => {
                if (isChecked) {
                  globalThis.FrameRateType = 'startFrame'
                }
              })
          }
          .margin({right:'10vp'})
          Row() {
            Text('退出帧率')
            Radio({ value: 'ExitFrame', group: 'radioGroup'}).checked(false)
              .height(20)
              .width(20)
              .onChange((isChecked:boolean) => {
                if (isChecked) {
                  globalThis.FrameRateType = 'ExitFrame'
                }
              })
          }
          .margin({left:'10vp'})
        }
        Row() {
          Row() {
            Text('切换帧率')
            Radio({ value: 'SwitchingFrame', group: 'radioGroup'}).checked(false)
              .height(20)
              .width(20)
              .onChange((isChecked:boolean) => {
                if (isChecked) {
                  globalThis.FrameRateType = 'SwitchingFrame'
                }
              })
          }
          .margin({right:'10vp'})
          Row() {
            Text('滑动帧率')
            Radio({ value: 'SlidingFrame', group: 'radioGroup'}).checked(false)
              .height(20)
              .width(20)
              .onChange((isChecked:boolean) => {
                if (isChecked) {
                  globalThis.FrameRateType = 'SlidingFrame'
                }
              })
          }
          .margin({left:'10vp'})
        }
        .margin({top:'5vp'})
      }
      .height('80vp')
      .width('94%')
      .backgroundColor('#ffffff')
      .borderRadius('10vp')
      .margin({ top: '12vp' })
      .justifyContent(FlexAlign.Center)
      // 底部按钮
      Blank()
      // 底部开始测试按钮
      Button('开始测试')
        .fontSize('15fp')
        .fontColor('#ffffff')
        .border({ radius: '20vp'})
        .width('80%')
        .height('60vp')
        .backgroundColor($r('app.color.colorPrimary'))
        .margin({ top: '30vp' })
        .onClick(() => {
          if( this.selectApps.length == 0) {
            prompt.showToast({ message: '请选择应用', duration: 1000})
            return
          }
          if (!globalThis.FrameRateType) {
            prompt.showToast({ message: '请选择帧率测试类型', duration: 1000})
            return
          }
          if (!globalThis.useDaemon) {
            prompt.showToast({ message: '请确认daemon服务是否启动', duration: 1000})
            return
          }
          globalThis.selectApps = this.selectApps
          for (let i = 0; i < globalThis.selectApps.length; i++) {
            globalThis.FrameRateName = this.selectApps[i].split(':')[1]
          }
          let message = ''
          if ( globalThis.FrameRateType === 'startFrame') {
            message = `启动帧率为【自动测试】\n请确认被测应用「${globalThis.FrameRateName}」与SmartPerf应用图标在同一桌面上，否则会导致应用启动失败。`
          } else if (globalThis.FrameRateType === 'ExitFrame') {
            message = `退出帧率为【手动测试】\n1、请手动打开「${globalThis.FrameRateName}」应用；\n2、点击悬浮窗“开始测试”按钮；\n3、悬浮窗显示”测试中“；\n4、手动退出「${globalThis.FrameRateName}」应用，等待测试完成。`
          } else if (globalThis.FrameRateType === 'SwitchingFrame') {
            message = `切换帧率为【手动测试】\n1、请手动打开「${globalThis.FrameRateName}」应用，并进入被测页面，确保被测页面可切换；\n2、点击悬浮窗”开始测试“按钮；\n3、悬浮窗显示”测试中“；\n4、点击切换页面一次，等待测试完成。`
          } else if (globalThis.FrameRateType === 'SlidingFrame') {
            message = `滑动帧率为【手动测试】\n1、请手动打开「${globalThis.FrameRateName}」应用，并进入被测页面，确保被测页面可上下滑动；\n2、点击悬浮窗”开始测试“按钮；\n3、悬浮窗显示”测试中“；\n4、上滑或下滑页面一次，等待测试完成。`
          }
          try {
            promptAction.showDialog({
              title: '测试方法',
              message: message,
              buttons: [
                {
                  text: '取消',
                  color: '#000000'
                },
                {
                  text: '确认',
                  color: '#1890ff'
                }
              ]
            })
              .then(data => {
                if (data.index == 1) {
                  globalThis.CreateFrameRateWindow()
                  router.back({ uri: 'pages/MainPage'})
                }
              })
              .catch(err =>{})
          } catch (error) {
            console.log(error)
          }
        })
      Divider().height('15%').width('80%').visibility(Visibility.Hidden)
    }
    .height('100%')
    .width('100%')
    .backgroundColor('#eeeeee')
  }
  onPageShow() {
    if (this.curAppList == null || this.curAppList == undefined) {
      const loadList = setInterval(() =>{
        this.curAppList = globalThis.appList
      }, 100)
      if (this.curAppList != null || this.curAppList != undefined) {
        clearInterval(loadList)
      }
    }
  }
}