# SP_daemon
## 简介

- OpenHarmony性能测试工具SmartPerf 命令行版本，可采集CPU、GPU、Temperature、Power、应用RAM、FPS等指标，通过设置采集指标，对采集数据进行实时打印、导出csv。

- 性能较差或无屏幕设备请使用命令行版本,带屏幕的设备且性能较好的设备推荐使用[UI版本](https://gitee.com/openharmony/developtools_profiler/blob/master/host/smartperf/client/client_ui/README_zh.md)。

## 代码目录
```
/developtools/profiler/host/smartperf/client/client_command
├── include                                     # 头文件目录
├── BUILD.gn                                    # SP_daemon bin打包配置文件
├── ByTrace.cpp                                 # trace抓取代码文件
├── Capture.cpp                                 # 截图代码文件
├── CPU.cpp                                     # CPU采集代码文件
├── DDR.cpp                                     # DDR采集代码文件
├── FPS.cpp                                     # FPS采集代码文件
├── GPU.cpp                                     # GPU采集代码文件
├── GpuCounter.cpp                              # GpuCounter采集代码文件
├── Network.cpp                                 # 网络上下行速率采集代码文件
├── Power.cpp                                   # 功耗采集代码文件
├── RAM.cpp                                     # 内存采集代码文件
├── smartperf_command.cpp                       # 程序执行文件
├── smartperf_main.cpp                          # 程序入口文件
├── sp_log.cpp                                  # log文件
├── sp_profiler_factory.cpp                     # 采集工厂文件
├── sp_server_socket.cpp                        # 与SmartPerf hap通讯代码文件
├── sp_task.cpp                                 # 与SmartPerf editor通讯代码文件
├── sp_utils.cpp                                # 工具类
├── Temperature.cpp                             # 温度采集代码文件
```

## 约束条件
   SmartPerf应用在3.2系统版本后开始预制使用。

## 功能特性

**1. 参数说明**
**1.1 基础采集命令参数**

| 命令   | 功能                   |必选|
| :-----| :--------------------- |:-----|
| -N    | 设置采集次数(一秒采集一次)|是|
| -PKG  | 设置包名                | 否|
| -c    | 采集cpu的频点和使用率，设置应用包名：采集整机和应用CPU信息，不设置应用包名：采集整机CPU信息     | 否|
| -g    | 采集gpu的频点和负载信息   |否|
| -f    | 采集指定应用的fps，必须设置应用包名        |否|
| -profilerfps | 采集当前界面fps          |否|
| -t    | 采集电池等温度           |否|
| -p    | 采集电流                 |否|
| -r    | 采集内存，设置应用包名：采集整机和应用内存信息，不设置应用包名：采集整机内存信息             |否|
| -snapshot | 屏幕截图             |否|
| -net | 采集网络速率              |否|
| -VIEW | 设置图层，需要先获取应用图层名                |否|
| -screen | 采集屏幕分辨率和刷新率               |否|
| -d    | 采集DDR                 |否|
| -sections| 设置分段采集          |否|

**1.2 启停采集命令参数**
| 命令   | 功能                   |必选|
| :-----| :--------------------- |:-----|
| -start | 开始采集，该命令后可添加基础采集命令             |是|
| -stop | 结束采集，执行后会生成采集报告              |是|
---

**2. 使用方式**<br>
1）目前命令行版本已系统预制，可以进入shell，执行SP_daemon --help查看。

```bash
C:\Users\test>hdc_std shell
# SP_daemon --help
OpenHarmony performance testing tool SmartPerf command-line version
Usage: SP_daemon <options> <arguments>
options:
 -N             set the collection times(default value is 0) range[1,2147483647], for example: -N 10
 -PKG           set package name, must add, for example: -PKG ohos.samples.ecg
 -c             get device CPU frequency and CPU usage, process CPU usage and CPU load ..
 -g             get device GPU frequency and GPU load 
 -f             get app refresh fps(frames per second) and fps jitters and refreshrate
 -profilerfps   get refresh fps and timestamp
 -sections      set collection time period(using with profilerfps)
 -t             get remaining battery power and temperature..
 -p             get battery power consumption and voltage
 -r             get process memory and total memory
 -snapshot      get screen capture
 -net           get uplink and downlink traffic
 -start         collection start command
 -stop          collection stop command
 -VIEW          set layler, for example: -VIEW DisplayNode
 -screen        get screen resolution
 -OUT           set csv output path
 -d             get device DDR information
 example:
 SP_daemon -N 20 -c -g -t -p -r -net -snapshot -d
 SP_daemon -N 20 -PKG ohos.samples.ecg -c -g -t -p -f -r -net -snapshot -d
 SP_daemon -start -c
 SP_daemon -stop
 SP_daemon -screen

command exec finished!
#
```
2）执行示例命令：SP_daemon -N 20 -PKG ohos.samples.ecg -c -g -t -p -f -r -d -net -snapshot
```
----------------------------------Print START------------------------------------
order:0 timestamp=1710916175201
order:1 ProcAppName=ohos.samples.ecg
order:2 ProcCpuLoad=0.0015567
order:3 ProcCpuUsage=0.000000
order:4 ProcId=18510
order:5 ProcSCpuUsage=0.000000
order:6 ProcUCpuUsage=0.000000
order:7 cpu0Frequency=418000
order:8 cpu0Usage=0.000000
order:9 cpu0idleUsage=0.000000
order:10 cpu0ioWaitUsage=0.000000
order:11 cpu0irqUsage=0.000000
order:12 cpu0niceUsage=0.000000
order:13 cpu0softIrqUsage=0.000000
order:14 cpu0systemUsage=0.000000
order:15 cpu0userUsage=0.000000
...
order:115 gpuFrequency=279000000
order:116 gpuload=61.000000
order:117 Battery=28.000000
order:118 shell_back=31.529000
order:119 shell_frame=30.529000
order:120 shell_front=30.548000
order:121 soc_thermal=49.624000
order:122 system_h=30.150000
order:123 currentNow=278
order:124 voltageNow=4250532
order:125 fps=3
order:126 fpsJitters=881659966;;108846354;;8289583
order:127 refreshrate=120
order:128 memAvailable=6354252
order:129 memFree=5971776
order:130 memTotal=11530092
order:131 pss=78045
order:132 arktsHeapPss=13394
order:133 gpuPss=280
order:134 graphicPss=0
order:135 heapAlloc=48080
order:136 heapFree=2576
order:137 heapSize=50788
order:138 nativeHeapPss=41897
order:139 privateClean=67232
order:140 privateDirty=12848
order:141 sharedClean=76224
order:142 sharedDirty=12848
order:143 stackPss=1096
order:144 swap=0
order:145 swapPss=0
order:146 ddrFrequency=1531000000
order:147 networkDown=0
order:148 networkUp=0
order:149 capture=data/local/tmp/capture/screenCap_1711190737580.png


----------------------------------Print END--------------------------------------
----------------------------------Print START------------------------------------
order:0 timestamp=1710916175201
order:1 ProcAppName=ohos.samples.ecg
order:2 ProcCpuLoad=0.001379
order:3 ProcCpuUsage=0.008162
order:4 ProcId=18510
order:5 ProcSCpuUsage=0.008162
order:6 ProcUCpuUsage=0.000000
order:7 cpu0Frequency=418000
order:8 cpu0Usage=16.346154
order:9 cpu0idleUsage=83.653846
order:10 cpu0ioWaitUsage=0.961538
order:11 cpu0irqUsage=4.807692
order:12 cpu0niceUsage=0.000000
order:13 cpu0softIrqUsage=0.000000
order:14 cpu0systemUsage=5.769231
order:15 cpu0userUsage=4.807692
...
order:115 gpuFrequency=279000000
order:116 gpuload=61.000000
order:117 Battery=28.000000
order:118 shell_back=31.529000
order:119 shell_frame=30.529000
order:120 shell_front=30.548000
order:121 soc_thermal=47.810000
order:122 system_h=30.200000
order:123 currentNow=303
order:124 voltageNow=4251570
order:125 fps=12
order:126 fpsJitters=122794860;;8372396;;8375521;;8448958;;16691667;;8357812;;8367188;;8364062;;8383855;;8514062;;8238542;;849062
order:127 refreshrate=120
order:128 memAvailable=6370048
order:129 memFree=5990136
order:130 memTotal=11530092
order:131 pss=78217
order:132 arktsHeapPss=13586
order:133 gpuPss=280
order:134 graphicPss=0
order:135 heapAlloc=48156
order:136 heapFree=2648
order:137 heapSize=50780
order:138 nativeHeapPss=41877
order:139 privateClean=67404
order:140 privateDirty=2920
order:141 sharedClean=76224
order:142 sharedDirty=12848
order:143 stackPss=1096
order:144 swap=0
order:145 swapPss=0
order:146 ddrFrequency=1531000000
order:147 networkDown=0
order:148 networkUp=0
order:149 capture=data/local/tmp/capture/screenCap_1711190738589.png

command exec finished!
#
----------------------------------Print END--------------------------------------
```
3）执行完毕后会在data/local/tmp生成data.csv文件，每次执行命令覆盖写入，可导出到本地查看。
```bash
c:\Users\xxx>hdc file recv data/local/tmp/data.csv D:\
[I][2024-03-20 18:27:07] HdcFile::TransferSummary success
FileTransfer finish, Size:7306, File count = 1, time:377ms rate:19.38kB/s
---

## 发布版本

**3.2.0.0版本发布内容：预制SP_daemon bin文件，支持以下功能：**<br>
1. 支持RK3568、Hi3516。<br>
2. 支持Shell启动。<br>
3. 支持采集整机CPU、GPU、POWER、TEMPERATURE、应用的FPS、RAM、CPU等