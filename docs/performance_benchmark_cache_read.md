# 缓存文件读取性能基准测试方案

## 背景

`native_daemon` 的文件缓存读取函数（`TakeResultsFromCacheFile` 和 `ReadFromCacheFile`）正在进行性能优化，将逐条 `read()` 改为 4MB 大块缓冲区读取。本方案用于验证性能提升效果。

## 测试文件格式

缓存文件格式：
- 每条记录 = 4字节 `totalSize` (uint32_t, 小端序) + `totalSize` 字节数据
- 文件大小：约 100MB
- 记录数量：约 25000 条
- 平均记录大小：约 4KB

## 基准指标

### 优化前（原始版本）
- **read() 系统调用次数**：约 50,000 次（每条记录 2 次 read：1次读totalSize，1次读数据）
- **预期吞吐量**：约 900-1000 MB/s（Python环境，Windows）
- **预期耗时**：约 0.1-0.2 秒（解析 100MB 文件）

### 优化后（缓冲区版本）
- **read() 系统调用次数**：约 25 次（100MB / 4MB = 25）
- **预期吞吐量**：约 3000-4000 MB/s（Python环境，Windows）
- **预期耗时**：约 0.03-0.05 秒（解析 100MB 文件）

### 性能提升预期
- **吞吐量提升**：约 3-4 倍
- **read() 调用减少**：约 2000 倍
- **时间减少**：约 70-75%

## 测试环境准备

### 1. 生成测试缓存文件

使用 Python 脚本生成 100MB 测试缓存文件：

```bash
# 生成 100MB 测试文件
python3 generate_test_cache_file.py test_cache_100MB.bin 100

# 或生成其他大小的文件
python3 generate_test_cache_file.py test_cache_50MB.bin 50
```

脚本位置：`C:\Users\Administrator\.openclaw\workspace-developer-junior\generate_test_cache_file.py`

### 2. 验证测试文件

```bash
# 检查文件大小
ls -lh test_cache_100MB.bin

# 应该显示约 100MB
```

## 性能测试执行

### 方法一：使用 Python 基准测试脚本（推荐）

Python 脚本可以快速验证优化效果，适用于开发环境。

```bash
# 运行基准测试（默认4MB缓冲区）
python3 benchmark_cache_read.py test_cache_100MB.bin 4

# 使用其他缓冲区大小测试
python3 benchmark_cache_read.py test_cache_100MB.bin 8  # 8MB缓冲区
```

脚本位置：`C:\Users\Administrator\.openclaw\workspace-developer-junior\benchmark_cache_read.py`

**输出示例**：
```
============================================================
Testing ORIGINAL version (byte-by-byte read)
============================================================
Total records: 25338
Total bytes read: 104857925 (100.00 MB)
Duration: 0.1025 seconds
Throughput: 975.96 MB/s
Estimated read() calls: ~50676 (per-record)

============================================================
Testing OPTIMIZED version (buffered read)
============================================================
Total records: 25338
Total bytes read: 104857925 (100.00 MB)
Duration: 0.0300 seconds
Throughput: 3336.98 MB/s
Estimated read() calls: ~25 (buffered)

============================================================
COMPARISON
============================================================
Throughput improvement: 3.42x
Read() call reduction: 2027x
Time reduction: 70.8%
```

### 方法二：在目标设备上测试（Linux/Android）

在真实设备上测试，使用 `strace` 统计实际的 `read()` 系统调用次数。

#### 2.1 准备测试文件

将测试文件推送到设备：
```bash
adb push test_cache_100MB.bin /data/local/tmp/
```

#### 2.2 测试原始版本

使用 `strace` 统计 read() 调用次数：
```bash
# 需要先编译并运行原始版本的 native_daemon
# 然后使用 strace 统计系统调用
strace -c -e read -p <native_daemon_pid>
```

或使用简单的测试程序：
```bash
# 在设备上运行测试
strace -c -e read python3 /data/local/tmp/benchmark_cache_read.py /data/local/tmp/test_cache_100MB.bin
```

#### 2.3 测试优化版本

使用相同方法测试优化后的版本：
```bash
strace -c -e read -p <native_daemon_pid>
```

#### 2.4 对比结果

对比 `strace` 输出中的 `read` 系统调用次数：
- 原始版本：约 50,000 次
- 优化版本：约 25 次

### 方法三：使用 perf 工具（Linux）

使用 `perf` 工具分析系统调用和性能：

```bash
# 统计系统调用
perf stat -e syscalls:sys_enter_read -e syscalls:sys_exit_read <command>

# 或使用 perf record 进行详细分析
perf record -e syscalls:sys_enter_read -g <command>
perf report
```

## 实际集成测试

### 1. 编译优化后的 native_daemon

```bash
cd D:\workspace\developtools_profiler
# 使用优化后的代码编译
./build.sh --target device
```

### 2. 在设备上部署并测试

```bash
# 推送到设备
adb push out/bin/native_daemon /data/local/tmp/

# 启动 native_daemon（启用文件缓存模式）
adb shell /data/local/tmp/native_daemon --file-cache-mode

# 在另一个终端使用 strace 统计
adb shell strace -c -e read -p $(adb shell pidof native_daemon)
```

### 3. 生成测试数据并触发读取

使用 `native_hook` 生成测试数据，触发文件缓存读取：

```bash
# 启动目标应用并采集数据
hiperf -p <pid> -o /data/local/tmp/test.perf -d 10

# 触发 native_daemon 读取缓存文件
# 观察 strace 输出中的 read() 调用次数
```

## 验收标准

优化完成后，需要满足以下条件：

1. **read() 系统调用次数**：从约 50,000 次降低到约 25 次（减少 > 99.9%）
2. **吞吐量提升**：至少提升 2 倍（建议 > 3 倍）
3. **时间减少**：至少减少 50%（建议 > 70%）
4. **功能正确性**：解析结果与优化前完全一致（记录数量、数据内容）
5. **稳定性**：多次测试结果稳定，无内存泄漏或崩溃

## 已知测试结果

### Windows 开发环境（Python 模拟）

| 版本 | read() 调用 | 耗时 | 吞吐量 |
|------|--------------|------|--------|
| 原始版本 | ~50,676 | 0.1025s | 975.96 MB/s |
| 优化版本 | ~25 | 0.0300s | 3336.98 MB/s |
| **提升** | **2027x** | **70.8%** | **3.42x** |

## 测试脚本说明

### generate_test_cache_file.py

生成符合缓存文件格式的测试数据：
- 随机生成 100-8192 字节大小的记录
- 每条记录前加 4 字节 totalSize
- 支持自定义文件大小

### benchmark_cache_read.py

性能基准测试脚本：
- 模拟原始版本（逐条 read）
- 模拟优化版本（缓冲区 read）
- 对比性能指标
- 支持自定义缓冲区大小

## 注意事项

1. **文件系统缓存**：首次测试后，文件会被操作系统缓存，后续测试会更快。建议每次测试前清除缓存或多次测试取平均值。
2. **设备性能**：不同设备的性能差异较大，建议在同一设备上对比优化前后的结果。
3. **实际场景**：真实场景中的数据大小和分布可能与测试文件不同，建议根据实际使用情况调整测试文件。
4. **内存限制**：缓冲区大小不宜过大，建议不超过 8MB。

## 故障排查

### 问题：read() 调用次数未显著减少

**可能原因**：
- 缓冲区大小设置过小
- 代码未正确应用优化
- 文件大小小于缓冲区大小

**解决方法**：
- 检查缓冲区大小设置（建议 4MB）
- 确认代码已正确修改并重新编译
- 使用更大的测试文件

### 问题：性能提升不明显

**可能原因**：
- 文件系统缓存影响
- 测试文件过小
- 设备 I/O 性能瓶颈

**解决方法**：
- 多次测试取平均值
- 使用更大的测试文件（200MB+）
- 在真实设备上测试

## 联系人

如有问题，请联系开发团队或查看相关代码：
- `hook_manager.cpp` - `ReadFromCacheFile`
- `stack_preprocess.cpp` - `TakeResultsFromCacheFile`
- `cache_file_writer.h/cpp` - 缓存文件写入
