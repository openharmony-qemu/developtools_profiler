# SmartPerf.hap
## 简介
- OpenHarmony性能测试工具 SmartPerf UI版本，可采集CPU、GPU、Temperature、Power、应用RAM、FPS等指标，可使用悬浮窗口控制并实时展示测试数据以及支持本地测试报告图表展示功能。

- 带屏幕的设备且性能较好的设备推荐使用UI版本，性能较差或无屏幕设备请使用[命令行版本](../device_command/README_zh.md)。

## 代码目录

```
/developtools/smartperf_host/smartperf_device/device_ui
├── AppScope                                    # SmartPerf的包名、应用名称等配置目录
├── entry                                       # SmartPerf主要代码文件
│   └── src
│       └── main
│           ├── ets                             # ets代码目录
│               ├──Application
│                   ├── abilityStage.ts         # 应用abilityStage.ts文件
│               ├── MainAbility
│                   ├── MainAbility.ts          # 应用MainAbility.ts文件
│               ├── common
│                   ├── constant                # 一些常量类文件
│                   ├── database                # 数据库操作类
│                   ├── entity                  # ServiceAbility方法
│                   ├── profiler                # 采集逻辑封装目录
│                   ├── ui                      # 嵌套子页面目录
│                   ├── uitls                   # 工具类目录
│               ├── pages
│                   ├── AppSelectPage.ets        # 应用选择页面
│                   ├── CPU0LineChartPage.ets    # CPU折线图页面
│                   ├── LightAdjust.ets          # 亮度调整页面
│                   ├── LoginPage.ets            # 登录首页面
│                   ├── MainPage.ets             # 主页面（包含主页、报告、我的三个子页面）
│                   ├── ReportDetail.ets         # 报告详情页页面
│                   ....                         # 其他一些页面
│               ├── workers
│                   ├── worker.js                # 负责socket通信的worker线程
│           ├── resources                        # 资源配置文件存放目录
|               ├── base                         # 默认图片资源，字体大小，颜色资源存放目录
├── signature
|   ├── openharmony_smartperf.p7b                # 应用签名文件
├── BUILD.gn                                     # 应用打包配置文件
├── README_zh.md                                 # readme文件
```

## 约束条件
**1. SmartPerf应用在3.2系统版本后开始预制使用。**<br>
**2. 使用设备必须是具备屏幕的设备。**<br>
## 功能特性
**1. 采集指标说明**<br>

| 采集项  | 说明                                                      |
| :----- | :---------------------                                   |
| FPS    | 1秒内游戏画面或者应用界面真实刷新次数。                          |
| CPU频率 | 每1秒读取一次设备节点下各CPU的频点信息。                        |
| CPU负载 | 每1秒读取一次设备节点下各CPU的负载信息。                        |
| GPU频率 | 每1秒读取一次设备节点下GPU的频点信息。                          |
| GPU负载 | 每1秒读取一次设备节点下GPU的负载信息。                          |
| DDR频点 | 每1秒读取一次设备节点下DDR的频点信息。                           |
| RAM     | 每1秒读取一次应用进程的实际物理内存。                           |
| 温度    | 每1秒读取一次读取一次设备节点下的soc温度等信息。               |
| 电流    | 每1秒读取一次设备节点下的电流信息。                             |
| 电压    | 每1秒读取一次设备节点下电池的电压信息。                          |
| 截图    |每1秒截取一张截图。                                            |
|trace采集|当帧绘制时间超过100ms以上会自动抓取trace，1min内只抓取1次。        |


**2. 计算指标说明**<br>

| 计算指标  | 说明                                                      |
| :----- | :---------------------                                   |
| 归一化电流 | 归一化电流=电压 * 电流 / 3.8。   |
| 平均帧率 | 有效时间内的帧率之和除以时间。                        |
| 最高帧率 | 测试数据中帧率的最大值。                        |
| 低帧率 | 根据测试数据帧率数据最大数据计算满帧，按照满帧对应低帧率基线，然后计算得到低帧率。                         |
| 抖动率 | 根据测试数据帧率数据的最大数据计算满帧，按照满帧对应抖动率基线，然后计算得到抖动率。                          |
| 卡顿次数 | 目前卡顿次数依赖于每帧的绘制时间，帧绘制时间>100ms的计算为卡顿。 |

**3. 原始数据说明**<br>

| 原始数据  | 说明                                                      |
| :----- | :---------------------  |
| timestamp | 当前时间戳，对应于采集时间。 |
| taskId  | 任务Id，对应于网站端报告的任务ID。|
| cpu0Frequency  | cpu0核心的频率，单位一般是HZ。|
| cpu0Load  |  cpu0核心的负载占比，计算得出 单位%。|
| gpuFrequency  | gpu频率。|
| gpuLoad  | gpu负载占比，单位%。|
| ddrFrequency  | ddr频率。 |
| socThermalTemp  | soc温度。|
| gpuTemp  | gpu温度。|
| batteryTemp  |电池温度。|
| currentNow  | 当前读到的电流值，单位一般是mA。|
| voltageNow  | 当前读到的电压值，单位一般是uV(微伏)。|
| pss  | 应用实际使用的物理内存，.单位一般是KB。|
| fps  | 帧率。|
| fpsJitters  | 每一帧绘制间隔，单位ns。|

**4. 使用操作流程**<br>
步骤1：进入shell中查看SP_daemon进程是否存在，没有则需启动SP_daemon进程(SP_daemon用于采集fps、ram等指标)， 然后打开桌面的Smartperf工具。（采集指标少的时候一般为SP_daemon未启动，或者被后台清理）<br>

查看SP_daemon进程是否存在
```
C:\Users\test>hdc_std shell
# ps -ef | grep SP_daemon
root          4707     1 0 09:41:12 ?     00:00:00 SP_daemon
root          4711  4704 25 09:41:19 pts/0 00:00:00 grep SP_daemon
#

```
如果SP_daemon进程不存在启动SP_daemon
```
C:\Users\test>hdc_std shell
# ps -ef | grep SP_daemon
root          4720  4718 6 09:48:43 pts/0 00:00:00 grep SP_daemon
# SP_daemon
# ps -ef | grep SP_daemon
root          4723     1 0 09:48:50 ?     00:00:00 SP_daemon
root          4727  4718 5 09:48:53 pts/0 00:00:00 grep SP_daemon
#
```
步骤2：点击桌面SmartPerf应用图标启动应用，然后点击登录，进入首页，点击开始测试页，选择被测应用，配置采集项（目前支持CPU、GPU、FPS、POWER、TEMP、RAM、截图能力、trace）。<br>

步骤3：选择应用后，点击开始测试、启动悬浮窗（左上角展示），会自动拉起应用，左上角悬浮窗展示start后点击即可开始，展示会变为计时状态，显示当前采集计时。（提示：单击暂停、继续任务，双击展开、关闭详情悬浮窗、点击详情悬浮窗后任意采集项会展开相应采集折线图）。<br>

步骤4：长按计时器，可以保存采集项数据，悬浮框消失。<br>

步骤5：报告列表页查看报告，原始数据可在概览页对应路径自行使用hdc命令pull出来。


**5. 其他一些说明**<br>
  1）截图目录默认在/data/local/tmp/capture下。<br>
  2）Trace抓取：当帧绘制时间超过100ms以上会自动抓取trace，1min内只抓取1次，目录在/data目录下，trace文件较大，建议定期清除（文件名称为：mynewtrace[当前时间戳].ftrace）。<br>
  3) fps采集不到如何解决<br>
    方法一：后台只能存在一个被测试应用，同应用不能运行多个后台（如果有多个，需清除所有应用，重新启动测试）。<br>
    方法二：执行hidumper -s 10如出现connect error等错误，尝试kill 掉 hidumper_servic进程，系统会自动再次拉起。


## 发布版本
**3.2.0.0版本布内容：预制smartperf应用，支持以下功能：**<br>
1. 支持采集CPU、GPU、Temperature、Power、应用RAM、FPS等指标数据采集。<br>
2. 悬浮窗控制采集及实时展示数据。<br>
3. 本地图表展示。<br>





