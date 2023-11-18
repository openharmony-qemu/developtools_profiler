# TraceStreamer工具说明
TraceStreamer是一个trace数据解析程序，可以将一个trace文本文件或者基于proto序列化的二进制文件转换成为sqlite数据库的形式。 TraceStreamer使用C++实现，支持在ohos, linux, mac等系统上使用，具有良好的跨平台特性。  
![GitHub Logo](./figures/trace_streamer_stream.png)   
## 关于TraceStreamer的使用说明
TraceStreamer工具可以2种方式使用： 
1. 可以将系统离线trace文件解析并转为db，此工具支持基于文本的trace和基于proto的trace。  
2. TraceStreamer工具还可以WebAssembly的方式在浏览器中运行，需提供相关接口给js文件。  

### 导出db模式
在导出db模式下，通过下面的指令：
```
./trace_streamer trace文件路径名 -e 导出db路径名.db
```
可以将trace文件转为db文件。  
本应用支持在ohos, linux, mac使用。   
在数据导出之后，会在本地目录下生成一个trace_streamer.log文件，在导出db的目录下生成一个数据库文件同名，.db.ohos.ts后缀的文件。  
文件内容如下：  
```
当前时间戳:执行结果（数字）  
应用运行时间  
```
执行结果解释如下：
```
0 代表执行成功 1 表示输入文件不匹配， 2 表示解析错误， 3其他错误。
``` 
#### __关于db文件的说明__  
可以使用sqliteexport或DB Browser for SQLite工具加载生成的db，通过查看stat表，可以浏览当前数据一共有多少类数据，各类数据都收到多少条，数据是否正常等情况。在meta表会记录数据库导出时的一些系统信息，比如导入和导出的文件全路径，解析时间等信息。  
meta表可以选择不导出（有些情况下会暴露系统敏感信息），在导出时添加 -nm选项即可。  
更多db文件的介绍，可以参考[doc/des_tables.md](./doc/des_tables.md)。

### 内置浏览器模式
TraceStreamer可以WebAssembly方式在浏览器中运行，相关接口在wasm模式下生成的trace_streamer_builtin.js文件中，js可以使用的接口如下使用如下接口访问trace_streamer:
```
extern "C" {
/* 初始化wasm，在JS中注册回调函数，并返回一段可复用的内存空间，由JS调用
 * 
 * @ replyFunction: 回调函数
*  @ reqBufferSize: 返回的内存长度
* return: 返回一段内存地址给JS
*/
EMSCRIPTEN_KEEPALIVE uint8_t* Initialize(ReplyFunction replyFunction, uint32_t reqBufferSize)

/* 更新起始结束时间,由JS调用
 * 
 * @ len: 起始和结束时间组成的字符串长度
* return: 成功返回0。
*/
EMSCRIPTEN_KEEPALIVE int UpdateTraceTime(int len)

/* 设置TraceStreamer和第三方wasm通信的回调函数,并返回一段内存，由JS调用
 * 
 * @ sendDataCallBack:与第三方wasm通信的回调函数
* @ reqBufferSize: 返回的内存长度
* return: 成功返回0
*/
EMSCRIPTEN_KEEPALIVE uint8_t* TraceStreamer_Set_ThirdParty_DataDealer(SendDataCallBack sendDataCallBack, uint32_t reqBufferSize)

/* TraceStreamer的数据解析接口，由JS调用
 * 
* @ dataLen: 需要解析的数据源长度
* return: 成功返回0，失败返回-1
*/
EMSCRIPTEN_KEEPALIVE int TraceStreamerParseDataEx(int dataLen, bool isFinish)

/* TraceStreamer停止解析数据，由JS调用
 * 
* return: 成功返回0，失败返回-1
*/
EMSCRIPTEN_KEEPALIVE int TraceStreamerParseDataOver()

/* 数据库操作接口，由JS调用
 * 
* @ sqlLen: 需要执行的操作类sql语句长度
* return: 成功返回0，失败返回-1
*/
EMSCRIPTEN_KEEPALIVE int TraceStreamerSqlOperateEx(int sqlLen)

/*清空wasm内存中的内容，由JS调用
 * 
* return: 成功返回0，失败返回-1
*/
EMSCRIPTEN_KEEPALIVE int TraceStreamerReset()

/*执行查询类sql语句，由JS调用
 * 
* @ sqlLen: 需要执行的查询类sql语句长度
* return: 成功返回0，失败返回-1
*/
EMSCRIPTEN_KEEPALIVE int TraceStreamerSqlQueryEx(int sqlLen)

/*取消sql查询，由JS调用。
 * 
* return: 成功返回0。
*/
EMSCRIPTEN_KEEPALIVE int TraceStreamerCancel()

/*发送数据给第三方wasm解析，由TraceStreamer调用
 * 
* @ pluginData: 第三方插件的数据源
* @ len: 数据源长度
* @ componentName: 第三方插件名称
* return: 成功返回0
*/
int TraceStreamer_Plugin_Out_SendData(const char* pluginData, int len, const std::string componentName)

/* 初始化配置接口，由JS调用
 * 
* @ dataLen: 配置字符串的长度
* return: 成功返回0
*/
EMSCRIPTEN_KEEPALIVE int TraceStreamer_Init_ThirdParty_Config(int dataLen)

} // extern "C"
```

### 可以执行如下命令查看应用帮助
```./trace_streamer --help```

### TraceStreamer支持解析的事件列表
通过```./trace_streamer -i```
查看支持的事件列表。  
支持的事件列表参见[SupportEventList.md](./doc/des_support_event.md)。
## TraceStreamer重要概念介绍
### 进程和线程标识符
```
在通用操作系统中，进程号（pid/tgid）和线程号（tid）可能会被重复用于标识不同的进程或者线程。所以在trace数据源中，进程号（pid）和线程号（tid）也可能被重用。
TraceStreamer在解析数据过程中，使用ipid(internal pid)唯一标识进程， itid(internal tid)唯一标识线程。
```
### 计量器
用来记录系统中各种随时间连续变化的数值。例如： CPU的频率， 内存的使用量， 界面刷新频率。
#### 举例
CPU频率：  
![GitHub Logo](./figures/cpu_frequency.png)  
内存占用：  
![GitHub Logo](./figures/mem_usage.png)

### 过滤器
TraceStreamer设计过程中使用了流式处理的思想，数据从入口进入以后，就像进入一条河流，从上游流向下游，在河道中央有很多过滤器，每种过滤器会将流过的数据中自己关注的内容吸附捕捉到。最终，每个过滤器都拥有了大量同类型的数据，而且这些数据都是按时间序列排列的。TraceStreamer使用filterid来标识同一种用途的数据，可以方便在UI中绘制。 
![GitHub Logo](./figures/filters.png)

## Stat表设计
具体内容参见 [des_stat](./doc/des_stat.md)。
## TraceStreamer开发环境搭建和编译运行指引

本应用使用gn作为构建工具。
### 开发环境
可以在ubuntu、mac、windows下执行开发和编译，建议使用vscode开发工具。  
在windows平台上，需使用支持c++17标准的clang编译器。  
# 对外部的依赖
本应用依赖与sqlite、protobuf(htrace解析部分依赖)、nlohmann_json。  
本应用同时依赖于src/protos/protogen.sh目录下文件来生成相关pb.h，pb.cc文件。
_____ 

### 编译linux和Mac应用
在目录下有build.sh脚本，在不同的平台上会判断系统版本，编译相应系统的应用。
```
./build.sh linux
./build.sh macx
```

### 编译wasm
```
./build.sh wasm
```

### 开始编译
本工具建议独立编译，通过部署第三方依赖库，emsdk，可编译出支持不同平台的应用。   
具体方法可参考[compile_trace_streamer](./doc/compile_trace_streamer.md)。