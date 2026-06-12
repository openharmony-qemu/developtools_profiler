## 修改说明

- 在 `symbols_file.cpp` 中新增对 `[anon:ArkTS Code:/` 开头的匿名 ArkTS 代码路径的支持，使其能够被正确识别为 HAP/ABC 类型并进行符号解析。

## 验证

- native_daemon_ut 测试通过
