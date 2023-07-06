# 如何编译TraceStreamer
TraceStreamer可以编译为命令行下的可执行程序，或者WebAssembly程序。

## 快速编译

可以按下面的方法快速编译TraceStreamer。

#### 准备工作

在码云上添加ssh公钥。  
本工具工程组织方式是gn。编译需要对应的编译器，编译前请自行配置本地编译器。  
所需编译器和版本如下表所示：
| 系统 | 工具 | 版本号|
| ---  | --- | --- |
|linux |clang/clang++| (Ubuntu 7.5.0-3ubuntu1~18.04) 7.5.0|
| macx |clang/clang++| 14.0.3 |
|windows |gcc.exe/g++.exe| (i686-posix-dwarf-rev0, Built by MinGW-W64 project) 8.1.0|
|wasm(linux下编译) |emcc/em++| 3.1.12|

对于wasm环境（目前只支持linux下编译），build.sh会自行配置环境。
#### 快速编译命令行下的可执行程序

在linux/windows/macx平台进入代码根目录下执行：

```
./build.sh
```
本脚本会下载和准备本地依赖的环境，并编译目标。  
在不同的平台上编译后的目标在不同的文件夹，如下所示：
| 系统 | release版本 | debug版本| 生成二进制名称
| ---  | --- | --- | --
|linux |out/linux| out/linux_debug| trace_streamer
| macx |out/macx| out/macx_debug |trace_streamer
|windows |out/windows| out/windows_debug|trace_streamer
|wasm(linux下编译) |-|out/wasm|trace_streamer_builtin.wasm和trace_streamer_builtin.js

#### 快速编译WebAssembly版本

如要编译WebAssembly版本，只需在代码根目录下执行：

```
./build.sh wasm
```
emsdk请使用3.1.12版本。

WebAssembly版本目前只支持在linux平台下编译。
