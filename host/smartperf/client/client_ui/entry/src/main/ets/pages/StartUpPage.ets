import router from '@system.router';
import { CollectItem, AppInfoItem } from '../common/entity/LocalConfigEntity';
import { StartTestTitleComponent } from '../common/ui/StartTestTitleComponent';
import prompt from '@system.prompt';
import {dateNormal } from  '../common/utils/TimeUtils';
import promptAction from '@ohos.promptAction'

const TAG = 'StartUpPage'

/*
 *  测试配置页
*/

@Entry
@Component
struct StartUpPage {
  @State private curAppList:Array<AppInfoItem> = globalThis.appList
  @State selectApps: String[] = []
  @State selectAppIcon: string = ''
  @State customPopup: boolean = true
  @State startType: boolean = false
  @State private collectConfigs: CollectItem[] = []

  aboutToAppear() {
    this.collectConfigs.push(
      new CollectItem('冷启动时延', true, true),
      new CollectItem('热启动时延', true, false)
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
      StartTestTitleComponent({ title: '启动测试'})
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
                        Radio({ value: 'checkbox1', group: 'checkboxGroup'})
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
      .padding({ top: '0vp', right: '5vp', bottom: '10vp', left: '5vp'})
      .margin({ top: '12vp', right: 'auto', left: 'auto'})
      // 冷热启动方式
      Row() {
        Row() {
          Text('冷启动')
          Radio({ value: 'coldStart', group: 'radioGroup'}).checked(false)
            .height(20)
            .width(20)
            .onChange((isChecked:boolean) => {
              if (isChecked) {
                this.startType = isChecked
                globalThis.startType = 'coldStart'
              }
            })
        }
        .margin({right:'10vp'})
        Row() {
          Text('热启动')
          Radio({ value: 'hotStart', group: 'radioGroup'}).checked(false)
            .height(20)
            .width(20)
            .onChange((isChecked:boolean) => {
              if (isChecked) {
                this.startType = isChecked
                globalThis.startType = 'hotStart'
              }
            })
        }
        .margin({left:'10vp'})
      }
      .height('60vp')
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
          if (this.startType == false) {
            prompt.showToast({ message: '请选择启动类型', duration: 1000})
            return
          }
          if (!globalThis.useDaemon) {
            prompt.showToast({ message: '请确认daemon服务是否启动', duration: 1000})
            return
          }
          globalThis.selectApps = this.selectApps
          let selectAppName = ''
          for (let i = 0; i < globalThis.selectApps.length; i++) {
            selectAppName = this.selectApps[i].split(':')[1]
          }
          let message = globalThis.startType == 'hotStart' ? '热启动' : '冷启动'
          promptAction.showDialog({
            title: '测试方法',
            message: `${message}为【自动测试】\n请确认被测应用「${selectAppName}」与SmartPerf应用图标在同一桌面上，否则会导致应用启动失败。`,
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
                globalThis.CreateControlWindow()
                router.back({ uri: 'pages/MainPage'})
              }
            })
            .catch(err =>{})
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
    let curDate = dateNormal()
    globalThis.testTaskName = '测试' + curDate
  }
}