# 编译UT
本文档给了编译trace_streamer的UT代码的方法。  
直接运行./build.sh test 即可编译代码。

在ut代码编译完成之后，直接运行./test.sh，可以执行所有ut，显示正确与否。  
在ut执行之后，直接运行./lcov.sh，可以生成覆盖率报告。  
覆盖率报告位于out/test/html目录。  