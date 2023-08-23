# 准备测试的硬件环境
    1. 测试依赖hi3516DV300设备。
    2. 烧录OHOS_STD代码。
    3. 通过直连方式，使用USB线将设备连接到存储测试代码的主机。

# 准备测试的软件环境
    1. 设置UT环境变量
        在代码根目录执行：export BUILD_UT=true
    2. 连接设备
        在代码根目录执行： root
                        remount
    3. 准备UT依赖的动态库
        通过push 把测试依赖的libsqlite.z.so等动态库推到hi3516DV300设备的system/lib文件夹下。
    4. 准备UT依赖的资源文件
        将OHOS_STD/test/resource目录push到hi3516DV300设备的/data目录
        在代码根目录执行： push ./test/resource/ /data/
    5. 环境清理
        (a) 执行之前先清理OHOS_STD/out/xxx-arm-release/obj/developtools 临时文件。
                rm -rf ~/OHOS_STD/out/xxx-arm-release/obj/developtools
        (b) 清理hi3516DV300设备上生成的中间文件/home/XXX/OHOS_STD/out。
                rm -rf /home/ohos/OHOS_STD/out*
        (c) 清理UT环境残留的覆盖率报告。
                rm -rf ~/OHOS_STD/developtools/profiler/build/html

# 测试步骤
    1. 启动UT执行环境, 启动后根据提示信息输入hi3516DV300对应的设备编号。
        在代码根目录执行：./test/developertest/start.sh
    2. 编译并执行hiprofiler_ts_ut。
        在start.sh启动的交互式窗口执行：run -t ut -ss developtools -ts hiprofiler_ts_ut

# 生成测试报告
    1. pull设备上生成的gcda
        进入设备环境后: 执行cd /home/XXX/OHOS_STD
                        tar -cvf out.tar out
                        exit
        退出shell后：pull /home/XXX/OHOS_STD/out.tar ~/OHOS_STD/
                    tar -xvf out.tar
                    pull /home/ohos/OHOS_STD/out/* /home/ohos/OHOS_STD/out/
                    cd /home/ohos/OHOS_STD/developtools/profiler/build
    2. 生成UT覆盖率报告
        在代码根目录执行：./developtools/profiler/build/lcov.sh
                        pull /data/test/hiprofiler_ts_ut.xml ~/OHOS_STD/developtools/profiler/build/html/
        报告位置：~/OHOS_STD/developtools/profiler/build/html/index.html

# 可能遇到的问题
    1. gcno文件不存在问题
        gcno文件是在编译阶段生成，编译时如果out目录存在.o临时文件，则不会编译源码，需要删除out/ obj/developtools/目录的编译中间文件后重新编译
    2. 运行时报缺少依赖库libsqlite.z.so
        需要把libsqlite.z.so push到设备的system/lib目录，在developtools/profiler/device/ohos_test.xml文件增加push命令
    3. UT所有步骤执行完之后，html中没有生成覆盖率信息
        权限问题，从设备中pull出来gcov文件后，需要关注文件权限
    4. UT输出覆盖率世间点不是当前最新时间
        清理掉原来的html文件，重新执行lcov.sh，生成新的html覆盖率报告