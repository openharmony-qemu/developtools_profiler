/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#pragma once
#include <memory>
#include <sstream>
#include <string>
#include <utility>

namespace mem_analyzer {
namespace log {

// 日志级别（数值即输出阈值，低于当前 level 的日志被丢弃）。
enum class Level { LOG_DEBUG = 0, LOG_INFO = 1, LOG_SUCCESS = 2, LOG_WARN = 3, LOG_ERROR = 4 };

// 可选前缀组件（位掩码）。
enum class Prefix : unsigned {
    NONE      = 0,
    TIMESTAMP = 1u << 0,   // [HH:MM:SS.mmm]
    THREAD_ID  = 1u << 1,   // [tid]
};

// ── 全局状态 ──
void SetLevel(Level min);          // 低于 min 的日志被丢弃；默认 Info
Level GetLevel();
void SetPrefix(Prefix flags);      // 默认 None

// ── 后端抽象（默认 console：stderr）──
class Sink {
public:
    virtual ~Sink() = default;
    virtual void Write(Level lv, const std::string& line) = 0;
};
void SetSink(std::shared_ptr<Sink> sink);

// 内部：拼前缀 + 整条着色 + 落盘（logger.cpp 实现）。dim 仅 Info 使用。
void WriteLine(Level lv, bool dim, const std::string& msg);

// ── 六个等级入口（整条同色）──
template <typename... Args> void Logd(Args&&... a)
{
    Emit(Level::LOG_DEBUG, true, std::forward<Args>(a)...);  // 灰
}
template <typename... Args> void Logi(Args&&... a)
{
    Emit(Level::LOG_INFO, false, std::forward<Args>(a)...);  // 正色
}
template <typename... Args> void LogiMinor(Args&&... a)
{
    Emit(Level::LOG_INFO, true, std::forward<Args>(a)...);  // 灰（次要提示）
}
template <typename... Args> void Logok(Args&&... a)
{
    Emit(Level::LOG_SUCCESS, false, std::forward<Args>(a)...);  // 绿
}
template <typename... Args> void Logw(Args&&... a)
{
    Emit(Level::LOG_WARN, false, std::forward<Args>(a)...);  // 黄
}
template <typename... Args> void Loge(Args&&... a)
{
    Emit(Level::LOG_ERROR, false, std::forward<Args>(a)...);  // 红
}

// ── 核心实现（inline）──
template <typename... Args>
void Emit(Level lv, bool dim, Args&&... args)
{
    if (lv < GetLevel()) { // ① 级别过滤（先于字符串构造）
        return;
    }
    std::ostringstream oss;
    (oss << ... << std::forward<Args>(args));       // ② C++17 fold 拼接
    WriteLine(lv, dim, oss.str());                 // ③ 前缀 + 着色 + 落盘
}

} // namespace log
} // namespace mem_analyzer
