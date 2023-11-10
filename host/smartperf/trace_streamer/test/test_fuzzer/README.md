# Hi3516烧录OH2代码
    1. 连接串口线， USB和网线。
    2. 使用HiTool工具加载并烧写OH2代码编译镜像。
        镜像路径： OH2_TOOL\out\ohos-arm-release\packages\phone\images\Hi3516DV300-emmc.xml
    3. 在新烧录好的开发板配置网络信息。
        配置IP:         ifconfig eth0 xxx.xxx.xxx.xxx
        配置子网掩码：   ifconfig eth0 xxx.xxx.xxx.xxx netmask 255.255.255.0
        分配hdcd端口：   hdcd -t &
        查看端口：       netstat -nat

# 编译FUZZ测试二进制文件
    1. 修改OH2_TOOL/developtools/profiler/ohos.build
        在testlist中添加："//developtools/profiler/trace_analyzer/test:fuzztest"
    2. 启动测试shell。
        cd OH2_TOOL
        ./test/developertest/start.sh  根据输出提示选择 hi3516DV300对应的数字。
    3. 编译可执行程序。
        run -t FUZZ -ss developtools -ts hiprofiler_ts_bytrace_fuzz_test
        run -t FUZZ -ss developtools -ts hiprofiler_ts_htrace_fuzz_test
        run -t FUZZ -ss developtools -ts hiprofiler_ts_selector_fuzz_test
       生成可执行文件路径： OH2_TOOL/out/ohos-arm-release/packages/phone/tests/fuzztest/hiprofiler/ts_fuzz/

# 准备FUZZ测试环境
    1. 使用hdc工具将上一步生成的可执行程序上传到开发板指定目录。
        例如： hdc_std file send hiprofiler_ts_htrace_fuzz_test /data/local/tmp/FuzzTest
        添加执行权限 chmod +x hiprofiler_ts_htrace_fuzz_test
    2. 上传动态库。
        代码目录下查询以下动态库， 并上传到开发板/system/lib目录。
            libsqlite.z.so
            libcrypto.so
            libssl.z.so
            libcrypto.z.so
            libgrpc.z.so

# 执行FUZZ测试用例
    cd /data/local/tmp/FuzzTest
    ./hiprofiler_ts_bytrace_fuzz_test -max_total_time=20
    ./hiprofiler_ts_htrace_fuzz_test -max_total_time=20
    ./hiprofiler_ts_selector_fuzz_test -max_total_time=20

# 可能遇到的问题
    1. 开发板启动失败，重启开发板，进入uboot中配置启动参数。
        setenv bootargs 'mem=640M console=ttyAMA0,115200 mmz=anonymous,0,0xA8000000,384M clk_ignore_unused androidboot.selinux=permissive skip_initramfs rootdelay=10 init=/init root=/dev/mmcblk0p5 rootfstype=ext4 rw blkdevparts=mmcblk0:1M(boot),15M(kernel),20M(updater),1M(misc),3307M(system),256M(vendor),-(userdata)'
        setenv bootcmd "mmc read 0x0 0x80000000 0x800 0x4800; bootm 0x80000000";
        save
        reset
    2. 执行测试用例过程中报“cannot merge previous GCDA ”。
        在开发板上进入OH2_TOOL目录，执行以下命令：
        find . -name "*.gcda" -print0 | xargs -0 rm
