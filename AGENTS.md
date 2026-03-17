# AGENTS.md - OpenHarmony SmartPerf Profiler

## Build Commands

### Building the Project
```bash
# Build entire project
./build.sh --product-name <product> --build-target hiprofiler_targets

# Build with tests enabled
BUILD_UT=true ./build.sh --product-name <product> --build-target hiprofiler_targets

# Build specific module
./build.sh --product-name <product> --build-target //developtools/profiler/device/plugins/memory_plugin:memdataplugin
```

### Running Tests
```bash
# Run all unit tests
./run_test.sh unittest

# Run specific test suite
./run_test.sh memdataplugin_ut

# Run specific test case
./run_test.sh memdataplugin_ut --gtest_filter=MemoryDataPluginTest.TestName_001

# Run with coverage
BUILD_UT=true ./build.sh --product-name <product> --build-target //developtools/profiler/device:unittest
```

### Running a Single Test
```bash
# Find test binary location: out/<product>/tests/unittest/developtools/profiler/device/<test_name>
./out/<product>/tests/unittest/developtools/profiler/device/memdataplugin_ut --gtest_filter=MemoryDataPluginTest.TestName_001
```

## Code Style Guidelines

### File Headers
All source files must include Apache 2.0 copyright header:
```cpp
/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
```

### Naming Conventions
- **Classes**: PascalCase (e.g., `MemoryDataPlugin`, `SpProfilerFactory`)
- **Functions**: PascalCase for public methods (e.g., `GetProfilerItem`)
- **Variables**: camelCase (e.g., `instance_`, `bufferSize`)
- **Constants**: UPPER_SNAKE_CASE (e.g., `MAX_BUFFER_SIZE`, `PAGE_SIZE`)
- **Files**: snake_case.cpp (e.g., `memory_data_plugin.cpp`)

### Namespace Organization
```cpp
namespace OHOS {
namespace SmartPerf {
    // Code here
} // namespace SmartPerf
} // namespace OHOS
```

### Smart Pointers
- Prefer `std::shared_ptr` and `std::unique_ptr` over raw pointers
- For OpenHarmony-specific code, use `sptr` and `wptr`
- Always initialize smart pointers in constructors
- Use `.reset()` in destructors to clear references

### Error Handling
- Return error codes (int) with 0 for success, -1 for failure
- Use `HILOG_ERROR` for logging errors
- Check return values from system calls
- Validate pointers before dereferencing with `ASSERT_NE(ptr, nullptr)`

### Memory Management
- Use smart pointers exclusively for dynamic memory
- Follow RAII principles for resource management
- Use `securec` functions for string operations
- Set pointers to nullptr after deletion

### Testing Conventions
- Use `HWTEST_F` macro for test cases
- Test naming: `MethodName_001` (normal), `_002` (exception), `_003` (boundary)
- Minimum 75% code coverage required
- Use Mock objects for dependencies
- Implement `SetUp()` and `TearDown()` in test fixtures

### BUILD.gn Configuration
- Use `ohos_unittest` for test targets
- Include all source dependencies in `deps`
- Add `external_deps` for OpenHarmony components
- Use `defines = ["private=public"]` to access private members in tests
- Set `module_out_path` to appropriate test output directory

### Import Order
1. C standard library headers
2. C++ standard library headers
3. Third-party library headers
4. OpenHarmony system headers
5. Project internal headers

### Formatting
- 4-space indentation (no tabs)
- Max line length: 120 characters
- Place braces on new line for functions
- Keep braces on same line for control structures
- Add space after commas, before opening braces

### Security Requirements
- Validate all user inputs
- Use bounds checking functions
- Check array bounds before access
- Sanitize file paths
- Use secure string operations
- Follow Huawei security coding standards

### Protobuf Usage
- Use `.pb.h` and `.pb.cc` generated files
- Prefer protobuf_lite for embedded systems
- Serialize data before transmission
- Parse with error checking

### Plugin Development
- Implement `PluginModuleCallbacks` interface
- Define `g_pluginModule` global variable
- Use callback pattern for data collection
- Register with plugin service via dlopen
- Follow plugin lifecycle: start -> report -> stop

### Performance Constraints
- Power limit: 5mA for profiling operations
- ROM budget: 10KB for new features
- Minimize memory allocations during profiling
- Use efficient data structures
- Avoid blocking operations in critical paths

### Logging
- Use `HILOG_DEBUG` for debug information
- Use `HILOG_INFO` for normal operations
- Use `HILOG_WARN` for warnings
- Use `HILOG_ERROR` for errors
- Include relevant context in log messages

### Thread Safety
- Use mutexes for shared data protection
- Prefer atomic operations for simple counters
- Document thread safety guarantees
- Avoid deadlocks with consistent lock ordering
- Use thread-safe containers when needed
