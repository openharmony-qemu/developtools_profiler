# pytest命令行参数
-s: 显示输出调试信息，包括print打印的信息
-v: 显示更详细的信息
-n：支持多线程运行脚本（需要保持用例彼此独立）
--reruns NUM：失败用例重跑次数
-x：表示只要有一个用例报错，那么测试停止
-k：模糊匹配字符串进行用例跑测


## 测试用例执行
前置条件：
1. 将onebox中的trace_streamer_nativehook.exe, process_resource_limit.json放入inputfiles文件夹
2. 打开并运行onebox中的profiler-test和network-profiler-test这两个IDE工程
3. 将所有的 "text=True"替换为"shell=True, text=True"


windows环境下执行测试用例
1. Root组用例：
设备进入root模式，进入scripts/testRoot目录，命令行执行 pytest ./

2. User组用例：
进入华为商城，下载QQ浏览器
设备进入User模式，进入scripts/testUser目录，命令行执行 pytest ./

3. 稳定性用例：
进入scripts/tesReliability目录，命令行执行 pytest ./

## 测试报告
执行用例后，会在reports目录下生成测试报告