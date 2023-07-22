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
├── Power.cpp                                   # 功耗采集代码文件
├── RAM.cpp                                     # 内存采集代码文件
├── smartperf_command.cpp                       # 程序执行文件
├── smartperf_main.cpp                          # 程序入口文件
├── sp_profiler_factory.cpp                     # 采集工厂文件
├── sp_server_socket.cpp                        # 与SmartPerf hap通讯代码文件
├── sp_utils.cpp                                # 工具类
├── Temperature.cpp                             # 温度采集代码文件
```

## 约束条件
   SmartPerf应用在3.2系统版本后开始预制使用。

## 功能特性

**1. 参数说明**

| 命令   | 功能                   |是否必选|
| :-----| :--------------------- |:-----|
| -N    | 设置采集次数。             |是|
| -PKG  | 设置包名。                |否|
| -PID  | 设置进程pid(对于ram适用)。 |否|
| -OUT  | 设置csv输出目录。          |否|
| -c    | 是否采集cpu。            |否|
| -g    | 是否采集gpu。             |否|
| -f    | 是否采集fps。             |否|
| -t    | 是否采集温度。             |否|
| -p    | 是否采集电流。             |否|
| -r    | 是否采集内存(需指定进程pid)。             |否|
| -snapshot    | 是否截图。            |否|

---

**2. 使用方式**<br>
1）目前命令行版本已系统预制，可以进入shell，执行SP_daemon --help查看。

```bash
C:\Users\test>hdc_std shell
# SP_daemon --help
usage: SP_daemon <options> <arguments>
--------------------------------------------------------------------
These are common commands list:
 -N             set num of profiler <must be non-null>
 -PKG           set pkgname of profiler
 -PID           set process id of profiler
 -OUT           set output path of CSV
 -c             get cpuFreq and cpuLoad
 -g             get gpuFreq and gpuLoad
 -f             get fps and fps jitters
 -t             get soc-temp gpu-temp ..
 -p             get current_now and voltage_now
 -r             get ram(pss)
 -snapshot      get screen capture
--------------------------------------------------------------------
Example: SP_daemon -N 20 -PKG ohos.samples.ecg -c -g -t -p -f
--------------------------------------------------------------------

command exec finished!
#
```
2）执行示例命令：SP_daemon -N 20 -PKG ohos.samples.ecg -c -g -t -p -f。
```
----------------------------------Print START------------------------------------
order:0 cpu0freq=1992000
order:1 cpu0load=23.469387
order:2 cpu1freq=1992000
order:3 cpu1load=26.262627
order:4 cpu2freq=1992000
order:5 cpu2load=19.000000
order:6 cpu3freq=1992000
order:7 cpu3load=74.747475
order:8 current_now=-1000.000000
order:9 gpu-thermal=48333.000000
order:10 gpufreq=200000000
order:11 gpuload=0.000000
order:12 soc-thermal=48888.000000
order:13 timestamp=1501925596847
order:14 voltage_now=4123456.000000
----------------------------------Print END--------------------------------------
----------------------------------Print START------------------------------------
order:0 cpu0freq=1992000
order:1 cpu0load=33.673470
order:2 cpu1freq=1992000
order:3 cpu1load=19.801979
order:4 cpu2freq=1992000
order:5 cpu2load=37.755100
order:6 cpu3freq=1992000
order:7 cpu3load=55.555553
order:8 current_now=-1000.000000
order:9 gpu-thermal=48333.000000
order:10 gpufreq=200000000
order:11 gpuload=0.000000
order:12 soc-thermal=48888.000000
order:13 timestamp=1501925597848
order:14 voltage_now=4123456.000000
----------------------------------Print END--------------------------------------
```
3）执行完毕后会在data/local/tmp生成data.csv文件，每次执行命令覆盖写入。
```bash
# cat /data/local/tmp/data.csv
cpu0freq,cpu0load,cpu1freq,cpu1load,cpu2freq,cpu2load,cpu3freq,cpu3load,current_now,gpu-thermal,gpufreq,gpuload,soc-thermal,timestamp,voltage_now
1992000,-1.000000,1992000,-1.000000,1992000,-1.000000,1992000,-1.000000,-1000.000000,48333.000000,200000000,0.000000,49444.000000,1501925677010,4123456.000000
1992000,16.326530,1992000,22.680412,1992000,62.626263,1992000,41.836735,-1000.000000,48333.000000,200000000,0.000000,48888.000000,1501925678011,4123456.000000
1992000,16.326530,1992000,35.353535,1992000,50.505051,1992000,42.857143,-1000.000000,48333.000000,200000000,0.000000,49444.000000,1501925679013,4123456.000000
```
---

## 发布版本

**3.2.0.0版本发布内容：预制SP_daemon bin文件，支持以下功能：**<br>
1. 支持RK3568、Hi3516。<br>
2. 支持Shell启动。<br>
3. 支持采集整机CPU、GPU、POWER、TEMPERATURE、应用的FPS、RAM。