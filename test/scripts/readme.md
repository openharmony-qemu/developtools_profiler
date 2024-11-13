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
windows环境下执行测试用例：
进入scripts目录
### 方式一：

    ```
    python main.py
    ```
执行参数在pytest.main中配置

### 方式二：
执行所有用例
    ```
    pytest ./
    ```
执行指定测试文件
    ```
    pytest ./testRoot/test_nativehook.py
    ```
    ```

## 测试报告
执行python main.py后，会在reports目录下生成测试报告