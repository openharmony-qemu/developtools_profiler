import {TaskStatus } from '../common/profiler/base/ProfilerConstant'
import database from '../common/database/DatabaseUtils'
import { destroyFloatWindow } from '../common/ui/floatwindow/utils/FloatWindowUtils'
import socket from '@ohos.net.socket';
import { FrameRateInfo } from '../common/entity/DatabaseEntity'

let tcp = null
@Entry
@Component
struct FrameRateBall {
  @State playerState: number = TaskStatus.task_init
  @State startTimerNum: number = 9
  @State timerNum: number = 6
  private frameRateInfo = new FrameRateInfo()
  offsetX: number = -1
  offsetY: number = -1
  isConnectTcp = false

  startCountdown() {
    let timer = setInterval(() => {
      if (this.startTimerNum > 0) {
        this.startTimerNum --
      } else {
        clearInterval(timer)
      }
    }, 1000)
  }

  countdown() {
    let timer = setInterval(() => {
      if (this.timerNum > 0) {
        this.timerNum --
      } else {
        clearInterval(timer)
      }
    }, 1000)
  }

  onTcpInit() {
    tcp = socket.constructTCPSocketInstance()
    tcp.setExtraOptions({
      keepAlive: true,
      OOBInline: true,
      TCPNoDelay: true,
      socketLinger: {on: true, linger: 10},
      receiveBufferSize: 1000,
      sendBufferSize: 1000,
      reuseAddress: true,
      socketTimeout: 3000
    })
    tcp.bind({address: '127.0.0.1', port: 8281, family: 1},err => {
      if (err) {
        return
      }
    })
    let promise = tcp.connect({ address: {address: '127.0.0.1', port: 8284, family: 1}, timeout: 6000})
    promise.then(()=> {
      this.isConnectTcp = true
      globalThis.task_status = TaskStatus.task_init
      tcp.on('message', value => {
        this.onStop(value)
      })
      tcp.send({
        data: 'SP_daemon -editor findAppPage'
      }).then(()=>{})
      if (globalThis.FrameRateType === 'startFrame') {
        setTimeout(() => {
          this.onStart()
        },2000)
      }
    })
  }

  onTcpClose() {
    if (tcp != null) {
      tcp.close().then(() => {
        console.log('tcp close success');
      })
      tcp.off('message')
    }
    globalThis.showFloatingWindow = false
    destroyFloatWindow("sp_FrameRateWindow")
  }

  onStart() {
    let date = new Date()
    this.playerState = TaskStatus.task_running
    this.frameRateInfo.startTime = date.getTime().toString()
    for (let i = 0; i < globalThis.selectApps.length; i++) {
      let curApp = globalThis.selectApps[i]
      let arrStr = curApp.split(":")
      this.frameRateInfo.name = arrStr[1]
      let commandName = ''
      if(globalThis.FrameRateType == 'startFrame') {
        this.frameRateInfo.type = '启动帧率'
        commandName = 'startFrame'
      } else if (globalThis.FrameRateType == 'ExitFrame') {
        this.frameRateInfo.type = '退出帧率'
        commandName = 'pagefps'
      } else if (globalThis.FrameRateType == 'SwitchingFrame') {
        this.frameRateInfo.type = '切换帧率'
        commandName = 'pagefps'
      } else if (globalThis.FrameRateType == 'SlidingFrame') {
        this.frameRateInfo.type = '滑动帧率'
        commandName = 'fps'
      }
      this.frameRateInfo.package = arrStr[0]
      let startMsg = 'SP_daemon -editor' + commandName + '' + arrStr[0] + '' + arrStr[1]
      tcp.send({
        data: startMsg
      }).then(() => {
        setTimeout(() => {
          if (this.playerState != TaskStatus.task_stop) {
            this.playerState = TaskStatus.task_Timeout
            setTimeout(()=>{
              this.onTcpClose()
            }, 5000)
          }
        }, 15000)
        if (globalThis.FrameRateType === 'startFrame') {
          this.startCountdown()
        }
      }).catch(err => {
        globalThis.showFloatingWindow = false
        destroyFloatWindow('sp_FrameRateWindow')
      })
    }
  }

  onStop(value) {
    this.playerState = TaskStatus.task_stop
    let buffer = value.message
    let dataView = new DataView(buffer)
    let str = ""
    for (let i = 0; i < dataView.byteLength; ++i) {
      str += String.fromCharCode(dataView.getUint8(i))
    }
    if (str.indexOf(":") > 0) {
      let arr = str.split(":")
      let value = arr[1]
      let date = new Date()
      this.frameRateInfo.endTime = date.getTime().toString()
      this.frameRateInfo.value = Number(value.split("fps")[0]) === -1 ? value.split("fps")[0] : Number(value.split("fps")[0]).toFixed(2)
      database.insertFrameRateData("t_FrameRate_info", this.frameRateInfo)
    }
    setTimeout(() => {
      this.onTcpClose()
    }, 2000)
  }

  aboutToAppear() {
    this.onTcpInit()
  }

  build() {
    Stack({ alignContent: Alignment.Top}) {
      if (this.playerState == TaskStatus.task_init) {
        Column() {
          if (globalThis.FrameRateType === 'startFrame') {
            Text('开始测试')
            .fontSize(12)
            .fontColor($r("app.color.color_fff"))
          } else {
            Text(`请手动打开「${globalThis.FrameRateName}」后，点击开始测试`)
              .fontSize(12)
              .fontColor($r("app.color.color_fff"))
              .margin({bottom: 2})
            Button('开始测试')
            .width('80vp')
            .height('25vp')
            .fontSize(12)
            .backgroundColor($r("app.color.colorPrimary"))
            .fontColor($r("app.color.color_fff"))
            .gesture(
              GestureGroup(GestureMode.Exclusive,
               TapGesture({ count: 1})
                 .onAction(() => {
                   if (globalThis.FrameRateType !== 'startFrame') {
                     this.onStart()
                     this.countdown()
                   }
                 })
                 )
            )
          }
        }
        .justifyContent(FlexAlign.Center)
        .padding({ right: 10, left: 10, top:6, bottom: 6})
        .backgroundColor('#333')
        .opacity(0.8)
        .borderRadius(4)
      }
      // 测试中
      if (this.playerState == TaskStatus.task_running) {
        Column() {
          if (globalThis.FrameRateType === 'startFrame') {
            Row() {
              Text('测试中')
                .fontSize(12)
                .fontColor($r("app.color.color_fff"))
              Text('0' + this.startTimerNum)
                .fontSize(12)
                .fontColor($r("app.color.color_fff"))
                .backgroundColor($r("app.color.colorPrimary"))
                .borderRadius(4)
                .margin({ left: '2vp'})
                .padding({ top: '0vp', right: '3vp', bottom: '0vp', left: '3vp'})
            }
          } else {
            if (globalThis.FrameRateType == "ExitFrame") {
              Text(`请在6s内手动退出「${globalThis.FrameRateName}」`)
                .fontSize(12)
                .fontColor($r("app.color.color_fff"))
                .margin({ bottom: 2})
            } else if (globalThis.FrameRateType == "SwitchingFrame") {
              Text(`请在6s内手动切换页面一次`)
                .fontSize(12)
                .fontColor($r("app.color.color_fff"))
                .margin({ bottom: 2})
            } else if (globalThis.FrameRateType == "SlidingFrame") {
              Text(`请在6s内手动上滑或下滑页面一次`)
                .fontSize(12)
                .fontColor($r("app.color.color_fff"))
                .margin({ bottom: 2})
            }
            Row() {
              Text("测试中")
                .fontSize(12)
                .fontColor($r("app.color.color_fff"))
              Text('0' + this.timerNum)
                .fontSize(12)
                .fontColor($r("app.color.color_fff"))
                .backgroundColor($r("app.color.colorPrimary"))
                .borderRadius(4)
                .margin({ left: '2vp'})
                .padding({ top: '0vp', right: '3vp', bottom: '0vp', left: '3vp'})
            }
          }
        }
        .justifyContent(FlexAlign.Center)
        .padding({ right: 10, left: 10, top:6, bottom: 6})
        .backgroundColor($r("app.color.color_333"))
        .opacity(0.8)
        .borderRadius(4)
      }

      if (this.playerState == TaskStatus.task_stop) {
        Column() {
          Text('测试完成，保存数据中..')
            .fontSize(12)
            .fontColor($r("app.color.color_fff"))
            .textAlign(TextAlign.Center)
        }
        .justifyContent(FlexAlign.Center)
        .padding({ right: 10, left: 10, top:6, bottom: 6})
        .backgroundColor($r("app.color.color_333"))
        .opacity(0.8)
        .borderRadius(4)
      }

      if (this.playerState == TaskStatus.task_Timeout) {
        Column() {
          Text('请求超时，请确认daemon服务是否启动')
          .fontSize(12)
          .textAlign(TextAlign.Center)
          .fontColor($r("app.color.color_fff"))
        }
        .justifyContent(FlexAlign.Center)
        .padding({ right: 10, left: 10, top:6, bottom: 6})
        .backgroundColor($r("app.color.color_333"))
        .opacity(0.8)
        .borderRadius(4)
      }
    }.width('100%').height('100%')
  }
}