import {TaskStatus } from '../common/profiler/base/ProfilerConstant'
import database from '../common/database/DatabaseUtils'
import { destroyFloatWindow } from '../common/ui/floatwindow/utils/FloatWindowUtils'
import socket from '@ohos.net.socket';
import { PerformInfo } from '../common/entity/DatabaseEntity'

let tcp = null
@Entry
@Component
struct ControlBall {
  @State playerState: number = TaskStatus.task_init
  private perFromInfo = new PerformInfo()
  offsetX: number = -1
  offsetY: number = -1
  ballWidth = 360
  ballHeight = 45
  DURATION: number = 300
  isConnectTcp = false

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
      setTimeout(() => {
        this.onStart()
      },2000)
    })
  }

  onTcpClose() {
    if (tcp != null) {
      tcp.close().then(() => {
        console.log('tcp close success');
      })
    }
    globalThis.showFloatingWindow = false
    destroyFloatWindow("sp_controlWindow")
  }

  onStart() {
    let date = new Date()
    this.playerState = TaskStatus.task_running
    this.perFromInfo.startTime = date.getTime().toString()
    let timeoutDur = 20000
    for (let i = 0; i < globalThis.selectApps.length; i++) {
      let curApp = globalThis.selectApps[i]
      let arrStr = curApp.split(":")
      if(globalThis.startType == 'coldStart') {
        this.perFromInfo.name = arrStr[1] + '冷启动'
        timeoutDur = 20000
      }
      if(globalThis.startType == 'hotStart') {
        this.perFromInfo.name = arrStr[1] + '热启动'
        timeoutDur = 30000
      }
      this.perFromInfo.package = arrStr[0]
      let startMsg = 'SP_daemon -editor' + globalThis.startType + '' + arrStr[0] + '' + arrStr[1]
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
        }, timeoutDur)
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
      this.perFromInfo.endTime = date.getTime().toString()
      this.perFromInfo.value = value.toString()
      database.insertPerformData("t_perform_info", this.perFromInfo)
    }
    setTimeout(() => {
      this.onTcpClose()
    }, 2000)
  }

  aboutToAppear() {
    this.onTcpInit()
  }

  MoveWindow(offsetX: number, offsetY: number) {
    globalThis.MoveFloatingWindow(offsetX, offsetY)
  }

  SetWindowPosition(offsetX: number, offsetY: number) {
    globalThis.SetFloatingWindowPosition(offsetX, offsetY)
  }

  build() {
    Stack({ alignContent: Alignment.Center}) {
      if (this.playerState == TaskStatus.task_init) {
        Circle()
          .width(this.ballWidth)
          .height(this.ballHeight)
          .fill(Color.White)
          .fillOpacity(0)
          .opacity(0.8)
          .border({radius: '90vp', width: '0.5vp', color: $r("app.color.color_333")})
          .linearGradient({
            angle: 135,
            direction: GradientDirection.Left,
            colors: [[$r("app.color.color_333"), 1.0], [$r("app.color.color_333"), 1.0]]
          })
        Text('初始化..')
          .fontSize(12)
          .textAlign(TextAlign.Center)
          .fontColor($r("app.color.color_fff"))
          .width('100%')
          .height('100%')
          .gesture(
          GestureGroup(GestureMode.Exclusive,
          TapGesture({ count: 2})
          .onAction(()=>{
          }),
          PanGesture({})
          .onActionStart(() =>{
          })
          .onActionUpdate((event: GestureEvent) => {
            this.offsetX = event.offsetX
            this.offsetY = event.offsetY
          })
          .onActionEnd(() => {
            this.MoveWindow(this.offsetX, this.offsetY)
            this.SetWindowPosition(this.offsetX, this.offsetY)
          })
          ))
      }
      // 测试中
      if (this.playerState == TaskStatus.task_running) {
        Circle()
          .width(this.ballWidth)
          .height(this.ballHeight)
          .fill(Color.White)
          .fillOpacity(0)
          .opacity(0.5)
          .border({radius: '90vp', width: '0.5vp', color: $r("app.color.color_333")})
          .linearGradient({
            angle: 135,
            direction: GradientDirection.Left,
            colors: [[$r("app.color.color_333"), 0.7], [$r("app.color.color_333"), 0.7]]
          })
        Text('测试中..')
          .fontSize(12)
          .textAlign(TextAlign.Center)
          .fontColor($r("app.color.color_fff"))
          .width('100%')
          .height('100%')
          .gesture(
            GestureGroup(GestureMode.Exclusive,
            LongPressGesture({ fingers: 1, repeat: false, duration: 1000})
            .onAction(()=>{
            }),
            PanGesture({})
            .onActionStart(() =>{
            })
            .onActionUpdate((event: GestureEvent) => {
              this.offsetX = event.offsetX
              this.offsetY = event.offsetY
            })
            .onActionEnd(() => {
              this.MoveWindow(this.offsetX, this.offsetY)
              this.SetWindowPosition(this.offsetX, this.offsetY)
            })
            )
          )
      }
      if (this.playerState == TaskStatus.task_stop) {
        Circle()
          .width(this.ballWidth)
          .height(this.ballHeight)
          .fill(Color.White)
          .fillOpacity(0)
          .opacity(0.8)
          .border({radius: '90vp', width: '0.5vp', color: $r("app.color.color_333")})
          .linearGradient({
            angle: 135,
            direction: GradientDirection.Left,
            colors: [[$r("app.color.color_333"), 1.0], [$r("app.color.color_333"), 1.0]]
          })
        Text('测试完成，保存数据中..')
          .fontSize(12)
          .textAlign(TextAlign.Center)
          .fontColor($r("app.color.color_fff"))
          .width('100%')
          .height('100%')
      }
      if (this.playerState == TaskStatus.task_Timeout) {
        Circle()
          .width(this.ballWidth)
          .height(this.ballHeight)
          .fill(Color.White)
          .fillOpacity(0)
          .opacity(0.5)
          .border({radius: '90vp', width: '0.5vp', color: $r("app.color.color_333")})
          .linearGradient({
            angle: 135,
            direction: GradientDirection.Left,
            colors: [[$r("app.color.color_333"), 0.7], [$r("app.color.color_333"), 0.7]]
          })
        Text('请求超时，请确认daemon服务是否启动')
          .fontSize(12)
          .textAlign(TextAlign.Center)
          .fontColor($r("app.color.color_fff"))
          .width('100%')
          .height('100%')
      }

    }.width('100%').height('100%')
  }
}