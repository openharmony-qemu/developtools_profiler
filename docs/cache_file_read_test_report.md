# 文件缓存读取性能优化单元测试报告

## 测试概述

为 `TakeResultsFromCacheFile`（stack_preprocess.cpp）和 `ReadFromCacheFile`（hook_manager.cpp）重写后的 4MB 大块缓冲区读取逻辑编写单元测试。

## 测试文件

- **文件路径**: `device/plugins/native_daemon/test/unittest/common/native/cache_file_read_test.cpp`
- **已添加到构建系统**: 是（BUILD.gn 已更新）

## 测试场景覆盖

### 1. ParseRecordsFromBuffer 测试（StackPreprocess）

| 测试用例 | 描述 | 预期结果 |
|---------|------|---------|
| ParseRecordsFromBufferNormalRecords | 正常多条记录连续排列，一次缓冲区可容纳 | 正确解析所有记录，offset 和 readfileSize 正确 |
| ParseRecordsFromBufferCrossBoundary | 记录恰好跨缓冲区边界（残余 + 续读拼接） | 解析完整记录，保留未完成部分 |
| ParseRecordsFromBufferIncompleteRecord | 最后一条记录不完整（文件截断） | 只解析完整记录，保留未完成部分 |
| ParseRecordsFromBufferEmptyBuffer | 空文件 | 返回 true，offset 和 readfileSize 为 0 |
| ParseRecordsFromBufferInvalidSizeZero | totalSize 为 0 | 返回 false（非法值） |
| ParseRecordsFromBufferInvalidSizeTooLarge | totalSize 超过上限（BUF_SIZE - sizeof(uint32_t)） | 返回 false（非法值） |
| ParseRecordsFromBufferOnlySizeHeader | 只有 size header，没有数据 | 返回 true，offset 为 0，等待更多数据 |

### 2. ParseCacheFileRecords 测试（HookManager）

| 测试用例 | 描述 | 预期结果 |
|---------|------|---------|
| ParseCacheRecordsNormalRecords | 正常多条记录连续排列，一次缓冲区可容纳 | 正确解析所有记录，offset 和 readfileSize 正确 |
| ParseCacheRecordsCrossBoundary | 记录恰好跨缓冲区边界（残余 + 续读拼接） | 解析完整记录，保留未完成部分 |
| ParseCacheRecordsIncompleteRecord | 最后一条记录不完整（文件截断） | 只解析完整记录，保留未完成部分 |
| ParseCacheRecordsEmptyBuffer | 空文件 | 返回 true，offset 和 readfileSize 为 0 |
| ParseCacheRecordsInvalidSizeZero | totalSize 为 0 | 返回 false（非法值） |
| ParseCacheRecordsInvalidSizeTooLarge | totalSize 超过上限（BUF_SIZE - sizeof(uint32_t)） | 返回 false（非法值） |
| ParseCacheRecordsOnlySizeHeader | 只有 size header，没有数据 | 返回 true，offset 为 0，等待更多数据 |

### 3. TakeResultsFromCacheFile 测试（StackPreprocess）

| 测试用例 | 描述 | 预期结果 |
|---------|------|---------|
| TakeResultsFromCacheFileEmptyFile | 空文件 | 正常处理，不崩溃 |
| TakeResultsFromCacheFileMultipleRecords | 多条记录（小记录） | 正确读取并处理所有记录 |
| TakeResultsFromCacheFileLargeRecords | 大记录（1KB、2KB、4KB） | 正确读取并处理所有记录 |
| TakeResultsFromCacheFileInvalidFd | 无效文件描述符 | 正常处理，不崩溃 |

### 4. ReadFromCacheFile 测试（HookManager）

| 测试用例 | 描述 | 预期结果 |
|---------|------|---------|
| ReadFromCacheFileEmptyFile | 空文件 | 正常处理，不崩溃 |
| ReadFromCacheFileMultipleRecords | 多条记录（小记录） | 正确读取并处理所有记录 |
| ReadFromCacheFileLargeRecords | 大记录（1KB、2KB)KB、4KB） | 正确读取并处理所有记录 |
| ReadFromCacheFileInvalidFd | 无效文件描述符 | 正常处理，不崩溃 |

## 测试统计

- **总测试用例数**: 20
- **覆盖的函数**: 4
  - `StackPreprocess::ParseRecordsFromBuffer`
  - `StackPreprocess::TakeResultsFromCacheFile`
  - `HookManager::ParseCacheRecords`
  - `HookManager::ReadFromCacheFile`

## 关键测试逻辑

### 缓冲区边界测试

```cpp
// 测试记录跨缓冲区边界的情况
// 模拟场景：缓冲区末尾只包含部分记录
bool ret = preprocess.ParseRecordsFromBuffer(buffer.data(), buffer.size() - 50,
                                        bufferOffset, readfileSize);
EXPECT_TRUE(ret);
EXPECT_GT(bufferOffset, 0);  // 应该处理了部分数据
EXPECT_LT(bufferOffset, buffer.size());  // 但没有处理完所有数据
```

### 非法值检测测试

```cpp
// 测试 totalSize = 0 的情况
uint32_t size = 0;
memcpy(buffer.data(), &size, sizeof(uint32_t));
bool ret = preprocess.ParseRecordsFromBuffer(buffer.data(), buffer.size(),
                                        bufferOffset, readfileSize);
EXPECT_FALSE(ret);  // 应该返回 false
```

```cpp
// 测试 totalSize 超过上限的情况
uint32_t size = static_cast<uint32_t>(MAX_RECORD_SIZE) + 1;
memcpy(buffer.data(), &size, sizeof(uint32_t));
bool ret = preprocess.ParseRecordsFromBuffer(buffer.data(), buffer.size(),
                                        bufferOffset, readfileSize);
EXPECT_FALSE(ret);  // 应该返回 false
```

### 不完整记录处理测试

```cpp
// 测试文件截断导致最后一条记录不完整的情况
size_t partialSize = sizeof(uint32_t) + record1.size() + sizeof(uint32_t) + 50;
// 只写入 50 字节的第二条记录数据（完整应该是 200 字节）
bool ret = preprocess.ParseRecordsFromBuffer(buffer.data(), buffer.size(),
                                        bufferOffset, readfileSize);
EXPECT_TRUE(ret);
EXPECT_EQ(bufferOffset, sizeof(uint32_t) + record1.size() + sizeof(uint32_t));
// offset 应该停在第二条记录的 size 字段之后，等待更多数据
```

## 编译和运行说明

### 编译

```bash
# 在项目根目录执行
./build.sh --test
# 或使用 GN/Ninja 直接编译
gn gen out/Default
ninja -C out/Default native_daemon_ut
```

### 运行测试

```bash
# 方法1：使用构建系统运行
./build/run.sh ./out/Default/native_daemon_ut

# 方法2：直接运行可执行文件
./out/Default/native_daemon_ut

# 方法3：运行特定测试用例
./out/Default/native_daemon_ut --gtest_filter=CacheFileReadTest.*
```

### 代码覆盖率（如果启用）

```bash
# 编译时启用覆盖率
./build.sh --test --coverage

# 运行测试后生成覆盖率报告
./build/lcov.sh
```

## 测试注意事项

1. **编译参数**: 测试使用 `-Dprivate=public` 编译参数直接访问私有方法
2. **测试框架**: 使用 Google Test (gtest) 框架，遵循项目现有风格
3. **断言风格**: 使用 `HWTEST_F` 和 `EXPECT_*` 宏，避免 `EXPECT_TRUE(true)` 等无效断言
4. **文件清理**: 测试完成后自动删除临时文件
5. **边界条件**: 重点测试缓冲区边界、空文件、非法值等边界情况

## 代码质量检查

- ✅ 无永远正确的断言
- ✅ 测试命名清晰，符合 `测试场景_测试描述` 格式
- ✅ 使用项目现有的测试工具函数（如 `MakeRawStack`）
- ✅ 测试用例独立，可单独运行
- ✅ 测试覆盖正常路径和异常路径
- ✅ 测试覆盖边界条件和极端情况

## 后续建议

1. **集成测试**: 在真实环境中测试大文件（GB 级别）的读取性能
2. **性能测试**: 对比重写前后的性能指标（读取时间、内存占用）
3. **压力测试**: 并发读取多个缓存文件，测试线程安全性
4. **内存泄漏检测**: 使用 Valgrind 或 AddressSanitizer 检测内存泄漏

## 测试状态

- ✅ 测试代码编写完成
- ✅ 测试文件已添加到构建系统
- ⏳ 等待编译和执行
- ⏳ 等待测试结果验证

## 修改的文件

1. **新增**: `device/plugins/native_daemon/test/unittest/common/native/cache_file_read_test.cpp`
2. **修改**: `device/plugins/native_daemon/test/BUILD.gn`（添加测试文件到 sources_base）
