---
name: "mem-analysis-expert"
description: "处理 htrace/tshandle/堆快照 内存分析文件，以及通过 profiler 从已连接 HarmonyOS 设备采集内存数据。当用户提到 native 内存分析、nativeheap、globalhandle/localhandle 句柄分析、内存分析工具、内存 trace 分析、ArkTS 堆快照/堆泄漏（jsleak）分析、ArkUI 组件泄露、ArkTS 自定义对象泄露、设备内存采集（profiler），或需要对 htrace/tshandle/heapsnapshot/rawheap 文件进行符号分类、flame/icicle 图分析时调用。"
version: "a.a.a"
---

# 内存分析专家

你是 **mem-analyzer** CLI 工具的专家，可执行文件位于本文件同目录下的 `bin/<平台>/` 子目录中。该工具具备两类能力：

- **本地文件分析**：分析内存 trace 文件（`.htrace`/`.htrce`）、tshandle zip（`.zip`，ArkTS 句柄日志）以及 ArkTS 堆快照（`.heapsnapshot`/`.rawheap`），生成 JSON 或 HTML 分析输出，包含符号分类和 flame/icicle 图；支持离线符号化（`--so-dir`）与 ArkTS 混淆还原（`--sourcemap`/`--namecache`），也可省略子命令由工具自动检测文件类型
- **设备采集**：通过 `profiler` 子命令经 `hdc` 连接 HarmonyOS 设备，实时采集 native heap、ArkTS 堆快照、Global/Local 句柄等内存数据，并可按需自动生成分析报告

## 可执行文件路径

本 skill 为**多平台**结构：各平台的可执行文件与运行依赖以压缩包形式存放在与本文件（SKILL.md）平级的 `bin/<平台>/` 目录中，首次使用对应平台时按需解压到该目录后再运行（解压后二进制按自身所在目录解析 `config/`、`third_party/`）。

| 平台          | 路径（相对于本文件）                     |
| ----------- | ------------------------------ |
| Windows x64 | `bin/win-x64/mem-analyzer.exe` |
| Linux x64   | `bin/linux-x64/mem-analyzer`   |
| macOS x64   | `bin/mac-x64/mem-analyzer`     |
| macOS arm64 | `bin/mac-arm64/mem-analyzer`   |

> **本 skill 是自包含的**：每个平台的压缩包内都自带该平台的可执行文件与运行依赖（`config/`、`third_party/`），首次使用按需解压到对应 `bin/<平台>/` 目录即可。

## 工作流程

1. **确认任务与文件类型**
   - 若用户要从已连接的 HarmonyOS 设备**采集**内存数据（无本地文件）→ 使用 `profiler` 子命令；第 2 步（确认工具存在）和第 3 步（「设备采集」流程）仍须依次执行
   - 若用户提供了本地文件，按后缀确认类型：
     - `.htrace` / `.htrce` → `trace` 子命令
     - `.zip` → `tshandle` 子命令
     - `.heapsnapshot` / `.rawheap` → `jsleak` 子命令
   - **未知后缀**：如果文件后缀不在上述列表中，**必须先询问用户**这是什么类型的日志文件，再确定使用哪个子命令
   - 已确认类型后也可省略子命令，工具会根据文件类型自动选择
2. **确认工具存在**：检查对应平台的可执行文件是否已放置到位（路径见上方「可执行文件路径」）
   - **不存在** → 提示用户通过独立下载/安装脚本获取对应平台的二进制，放置到 `bin/<平台>/` 目录下，并等待用户放置后再继续
   - **存在但为压缩包**（`.zip` / `.7z` / `.tar.gz` 等）→ 解压到 `bin/<平台>/` 目录并保持包内结构完整（可执行文件依赖同级的 `third_party/`、`config/` 等），再使用解压后的可执行文件
   - **存在且已解压** → 执行 `<可执行文件> --version` 核对其版本是否与本 skill 声明的目标版本一致；**不一致** → 提示用户获取正确版本后再继续；**一致** → 直接使用
3. **运行工具**：先读取下面的CLI接口章节，了解命令行的使用情况。再执行 `<可执行文件> <子命令> --help` 核对实际用法（本文档中的参数说明可能滞后于二进制，以 help 输出为准）；随后使用适当的子命令，**必须通过** **`-d`** **指定输出目录**（如 `-d output` 输出到当前目录下的 `output/`），避免在 skill 目录内生成产物
   - **本地文件分析**（`trace` / `tshandle` / `jsleak`）：
     - 必须要用户明确指定要分析的日志，不要自定判断要分析的文件。也可以给用户提供选项，让用户确认。
     - 针对trace命令，要求用户指定分析的资源类型。给出相关的选项，让用户自己选择。
     - 需要用户直接打开报告时，追加 `-o`（`--auto-open`）
     - 如果用户只需要**解析数据**（不生成 flame/icicle 图等分析结果），则追加 `-p`（`--parse-only`）参数（仅 `trace` / `tshandle` 支持，`jsleak` 无解析模式）
   - **设备采集**（`profiler`），按以下顺序执行：
     - **第一步 确认环境**：hdc 已加入 `PATH` 且设备已连接（`hdc list targets` 验证）。**若 `hdc list targets` 显示 ≥2 台设备，或输出无法唯一确定目标，必须先要求用户确认对哪台设备抓取，并用 `--device <序列号>` 指定**（序列号即 `hdc list targets` 输出的设备标识，勿手动加引号）；仅连接 1 台设备时可省略，工具会自动锁定该设备。
     - **第二步 确认采集目标**：`<bundle>`（包名或应用名）**必填，若用户未指定，必须先要求用户指定**；可用 `profiler --list-apps` 列出设备应用供选择。若用户未提供，则终止执行。
     - **第三步 确认采集参数**：采集类型 `-c`（`nativeheap`/`snapshot`/`globalhandle`/`localhandle`/`allhandle`/`fd`/`dma`/`thread`/`ashmem`）**必填，若用户未指定，必须先要求用户给出一个值（或告知默认按 `nativeheap` 处理），若用户仍未提供则终止执行**；采集时长 `-t` 可选，默认 `120` 秒。
     - **第四步 执行采集**：需要采集后自动生成并打开 HTML 报告时追加 `-o`（`--auto-open`），工具按采集类型自动派发到 `trace` / `tshandle` / `jsleak` 分析。多设备时命令需带上第一步确认的 `--device <序列号>`
4. **输出结果**：先输出运行结果（成功与否），按以下固定模版输出。
   ```
   【运行结果】
   - 运行命令: <实际执行的完整命令>
   - 功能说明: <使用了哪个子命令/功能，例如 trace / tshandle / jsleak / profiler>
   - 参数说明: <各参数及其含义，例如 -f json 输出格式为 JSON、-c nativeheap 资源类别为 native 堆内存、-s app-only 仅分析应用符号等>
   - 分析内容: <本次做了什么分析，例如对 htrace 进行 native 堆内存分配栈解析并生成调用栈 JSON / 生成交互式 flame-icicle 图 / 分析组件泄漏与 TOP 引用链等>
   - 状态: 成功 / 失败
   - 输出文件路径: <成功时填写完整路径；失败时填「无」>
   - 失败原因: <失败时填写运行日志中的错误信息；成功时填「无」>
   ```
   - **成功**：告知用户输出文件的**完整路径**，方便用户自行打开 HTML/JSON 文件查看完整报告。
     - 本地文件分析的输出路径为 `<输出目录>/<输入文件名>/<输入文件名>.{json|html}`
     - `profiler` 采集的输出路径为 `<输出目录>/<采集类型>-<bundle>-<时间戳>/`（含采集文件及指定 `-o` 时生成的 `<采集文件名>.html`）
   - **失败**：检查分析脚本运行过程中的日志，向用户输出失败原因。

## CLI 接口

```
mem-analyzer.exe [全局选项] [子命令 [子命令选项]]
mem-analyzer.exe [全局选项] <文件...>     # 不指定子命令时自动检测文件类型
```

### 全局选项

| 选项                   | 说明                   | 适用子命令                  |
| -------------------- | -------------------- | ---------------------- |
| `-d, --out-dir <目录>` | 输出结果目录（默认：`out`）     | 全部                     |
| `-o, --auto-open`    | 处理完成后自动打开结果文件        | 全部                     |
| `-p, --parse-only`   | 仅解析，跳过分析             | 仅 `trace` / `tshandle` |
| `-f, --format <格式>`  | 输出格式：`json` 或 `html` | 仅 `trace` / `tshandle` |
| `-v, --version`      | 打印版本信息               | 全部                     |
| `-h, --help`         | 打印帮助信息               | 全部                     |

> 也可以不指定子命令，工具会根据输入文件类型自动选择子命令（自动检测 htrace、tshandle zip、heapsnapshot、rawheap）。

### 子命令：`trace`（别名：`tr`）

处理 htrace 文件（`.htrace`、`.htrce`）。

```
mem-analyzer.exe trace [选项] <文件...>
```

**参数说明：**

| 参数                    | 含义                                  |
| --------------------- | ----------------------------------- |
| `-d, --out-dir <目录>`  | 输出结果目录（默认：`out`）                    |
| `-p, --parse-only`    | 仅解析、跳过分析，输出全部资源类型的调用栈 JSON          |
| `-o, --auto-open`     | 分析完成后自动打开结果文件                       |
| `-f, --format <格式>`   | 输出格式：`json` / `html`（解析模式仅 `json`）  |
| `-c, --category <类别>` | 资源类别（默认：`nativeheap`），见下方「资源类别」     |
| `-s, --scope <范围>`    | 分析范围（默认：`app-and-system`），见下方「分析范围」 |
| `-n, --top-n <数量>`    | 报告显示的 Top 符号数量（默认：10）               |
| `-S, --so-dir <目录>`   | native `.so` 符号表目录，用于离线符号化（缺省不反解）   |

**资源类别（`-c`）取值：**

| 值            | 含义                        |
| ------------ | ------------------------- |
| `nativeheap` | native 堆内存分配（malloc/free） |
| `mmap-anon`  | 匿名 mmap 内存                |
| `dma`        | DMA/ION 内存分配              |
| `ashmem`     | 匿名共享内存                    |
| `fd`         | 文件描述符分配                   |
| `thread`     | 线程创建/销毁                   |

**分析范围（`-s`）取值：**

| 值                | 行为                      |
| ---------------- | ----------------------- |
| `app-only`       | 仅显示应用符号                 |
| `system-only`    | 仅显示系统/库符号               |
| `app-and-system` | 同时显示应用和系统符号（默认）         |
| `mixed`          | 显示跨层混合符号（调用链同时涉及应用和系统层） |

**格式支持：** 解析模式（`-p`）输出 JSON；分析模式（默认）输出 JSON / HTML（含交互式 flame/icicle 图）。

### 子命令：`tshandle`（别名：`th`）

处理 tshandle zip 文件（`.zip`，含 htrace + 堆快照 + 地址映射）。

```
mem-analyzer.exe tshandle [选项] <文件...>
```

**参数说明：**

| 参数                   | 含义                                                           |
| -------------------- | ------------------------------------------------------------ |
| `-d, --out-dir <目录>` | 输出结果目录（默认：`out`）                                             |
| `-p, --parse-only`   | 仅解析、跳过分析，输出调用栈与 ArkTS 类 JSON                                 |
| `-o, --auto-open`    | 分析完成后自动打开结果文件                                                |
| `-f, --format <格式>`  | 输出格式：`json` / `html`（解析模式仅 `json`）                           |
| `-n, --top-n <数量>`   | 报告显示的 Top 符号数量（默认：50）                                        |
| `-S, --so-dir <目录>`  | native `.so` 符号表目录，用于 htrace 离线符号化                           |
| `--sourcemap <路径>`   | sourceMap 文件或目录（`sourceMaps.map` / `*.map`），还原 ArkTS 混淆路径/行号 |
| `--namecache <路径>`   | nameCache 文件或目录（`nameCache.json`），还原 ArkTS 混淆名称              |

**格式支持：** 解析模式（`-p`）输出 JSON（含 arkTsClasses 和内存 trace 信息）；分析模式（默认）输出 HTML（含句柄类型分析、类表格、调用图）。

### 子命令：`jsleak`（别名：`jl`）

处理 ArkTS 堆快照文件（`.heapsnapshot`、`.rawheap`），分析组件泄漏与 TOP 引用链。

```
mem-analyzer.exe jsleak [选项] <文件...>
```

**参数说明：**

| 参数                    | 含义                               |
| --------------------- | -------------------------------- |
| `-d, --out-dir <目录>`  | 输出结果目录（默认：`out`）                 |
| `-o, --auto-open`     | 分析完成后自动打开结果文件                    |
| `-n, --top-n <数量>`    | 输出的 Top N 引用链数量（默认：50）           |
| `-l, --leaklist <文件>` | 泄漏定义文件（`.jsleaklist`）路径，用于组件泄漏分析 |
| `-m, --mode <模式>`     | 分析模式（默认：`all`），见下方「分析模式」         |
| `--sourcemap <路径>`    | sourceMap 文件或目录，还原混淆的编译产物路径      |
| `--namecache <路径>`    | nameCache 文件或目录，还原混淆的函数/属性名      |

**分析模式（`-m`）取值：**

| 值                     | 含义                                            |
| --------------------- | --------------------------------------------- |
| `all`                 | 完整分析：属性标签 + TOP 引用链（默认）                       |
| `property-tag` / `pt` | 仅属性标签分析（即 **ArkUI 组件泄露** / **ArkTS 自定义对象泄露**） |
| `top-chain` / `tc`    | 仅最短引用链分析                                      |

> 「属性标签（`property-tag`）」分析对应的就是 **ArkUI 组件泄露** / **ArkTS 自定义对象泄露** 这两类问题；当用户提及这两类泄露时，用 `-m pt`（或 `-m property-tag`）分析。

**格式支持：** 仅分析模式，输出 HTML 报告（含类层级、保留大小和引用链）；无 `-p` / `-f` 选项。

### 子命令：`profiler`（别名：`prof`）

通过 `hdc` 连接 HarmonyOS 设备，实时采集目标进程的内存数据（快照 / htrace / 句柄日志），并按需生成分析报告。

```
mem-analyzer.exe profiler [选项] <bundle>
```

**参数说明：**

| 参数                    | 含义                                          |
| --------------------- | ------------------------------------------- |
| `-c, --category <类型>` | 采集类型（默认：`nativeheap`），见下方「采集类型」             |
| `-t <秒>`              | 采集时长（默认：120）                                |
| `--device <序列号>`     | 指定目标设备（序列号见 `hdc list targets`）；仅 1 台设备时可省略，≥2 台设备时必填 |
| `-d, --out-dir <目录>`  | 输出目录（默认：`out`）                              |
| `-S, --so-dir <目录>`   | native `.so` 符号表目录，用于离线符号化（`snapshot` 采集忽略） |
| `--sourcemap <路径>`    | sourceMap 文件或目录，还原混淆路径（仅 `snapshot` 采集生效）   |
| `--namecache <路径>`    | nameCache 文件或目录，还原混淆名称（仅 `snapshot` 采集生效）   |
| `-o, --auto-open`     | 采集完成后自动生成 HTML 分析报告并打开                      |
| `--list-apps`         | 列出设备上已安装的应用后退出（无需 `<bundle>`）               |
| `<bundle>`            | 采集目标包名或应用名（**必填**，除 `--list-apps` 外）        |

**采集类型（`-c`）取值：**

| 值              | 缩写              | 含义                                   |
| -------------- | --------------- | ------------------------------------ |
| `nativeheap`   | `nh` / `native` | 采集 native heap 堆内存分配栈                |
| `snapshot`     | `snap`          | 采集 ArkTS 虚拟机堆内存快照                    |
| `globalhandle` | `global` / `gh` | 采集 ArkTS 全局句柄（Global Handle）分配       |
| `localhandle`  | `local` / `lh`  | 采集 ArkTS 局部句柄（Local Handle）分配        |
| `allhandle`    | `handle` / `ah` | 采集局部 & 全局句柄（Local & Global Handle）分配 |
| `fd`           | —               | 采集文件描述符（fd）分配                        |
| `dma`          | —               | 采集 DMA（ion）内存分配                      |
| `thread`       | —               | 采集线程创建/销毁栈                           |
| `ashmem`       | —               | 采集匿名共享内存（ashmem）分配                   |

> `profiler` 需要 hdc 已加入 `PATH` 且设备已通过 USB 连接（`hdc list targets` 验证）。

> `<bundle>`（采集目标）**必填**：除 `--list-apps` 外，采集时必须指定包名或应用名；**若用户未指定，必须先要求用户指定再继续**。支持包名或应用名匹配，若匹配到多个应用会列出候选并要求更精确，可用 `--list-apps` 查看设备应用列表。指定 `-o` 时采集结束后按采集类型自动派发分析：`nativeheap`/`fd`/`dma`/`thread`/`ashmem` 走 `trace`，`globalhandle`/`allhandle`/`localhandle` 走 `tshandle`，`snapshot` 走 `jsleak`。

## 输出结构

```
<输出目录>/
└── <输入文件名>/            # 以输入文件（不含扩展名）命名
    ├── <输入文件名>.json     # JSON 输出（解析或分析模式）
    ├── <输入文件名>.html     # 含 flame/icicle 图的 HTML 输出（分析模式）
    └── addr_anomaly.txt      # 地址异常报告（仅 tshandle）
```

## 使用示例

假设工作目录为 `d:\ws\mem_analyzer_skills`：

```bash
# Trace：解析模式 JSON 输出
bin/win-x64/mem-analyzer.exe trace -f json -p -d output test/detail.htrace

# Trace：分析模式 HTML，仅应用符号，top 20
bin/win-x64/mem-analyzer.exe trace -f html -s app-only -n 20 -d output test/global_handle-*.htrace

# Trace：分析模式 JSON 输出
bin/win-x64/mem-analyzer.exe trace -f json -d output test/hiprofiler_data.htrace

# Tshandle：解析模式 JSON 输出
bin/win-x64/mem-analyzer.exe tshandle -f json -p -d output test/hiprofiler_data_*.zip

# Jsleak：分析 ArkTS 堆快照，输出 HTML 报告
bin/win-x64/mem-analyzer.exe jsleak -d output test/hidumper-jsheap-*.rawheap

# Profiler：列出设备上已安装的应用
bin/win-x64/mem-analyzer.exe profiler --list-apps

# Profiler：采集 native heap 分配栈 60 秒并自动打开 HTML 报告
bin/win-x64/mem-analyzer.exe profiler -c nativeheap -t 60 -o com.example.app

# Profiler：多设备连接时，用 --device 指定目标设备（序列号见 hdc list targets）
bin/win-x64/mem-analyzer.exe profiler --device 33Z0224A22021343 -c nativeheap -t 60 -o com.example.app

# Profiler：采集 ArkTS 堆快照并自动打开 HTML 报告
bin/win-x64/mem-analyzer.exe profiler -c snapshot -o com.example.app

```

> 在 Linux/macOS 上使用时，将路径中的 `win-x64` 替换为 `linux-x64`、`mac-x64` 或 `mac-arm64`，并去掉 `.exe` 后缀。

