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
#include "utils/logger.hpp"
#include "utils/terminal_style.hpp"

#include <atomic>
#include <chrono>
#include <ctime>
#include <functional>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <sstream>
#include <thread>

namespace mem_analyzer {
namespace log {

namespace {

constexpr int MS_DIGIT_WIDTH = 3;
std::atomic<Level> g_level{Level::LOG_INFO};
std::atomic<unsigned> g_prefix{0};
std::mutex g_mutex;                          // 保护 g_sink 与单条日志的原子输出
std::shared_ptr<Sink> g_sink;                // null = 默认 console（stdout/stderr）

// 级别标签（定宽 7 字符，便于对齐）。
const char* tag(Level lv)
{
    switch (lv) {
        case Level::LOG_DEBUG:   return "[DEBUG]";
        case Level::LOG_INFO:    return "[ INFO]";
        case Level::LOG_SUCCESS: return "[  OK ]";
        case Level::LOG_WARN:    return "[ WARN]";
        case Level::LOG_ERROR:   return "[ERROR]";
    }
    return "[     ]";
}

// 正文着色（整条同色的正文部分）：正色 = 不染色。
std::string paint(Level lv, bool dim, const std::string& text)
{
    switch (lv) {
        case Level::LOG_DEBUG:   return style::Dim(text);
        case Level::LOG_INFO:    return dim ? style::Dim(text) : text;
        case Level::LOG_SUCCESS: return style::Green(text);
        case Level::LOG_WARN:    return style::Yellow(text);
        case Level::LOG_ERROR:   return style::Red(text);
    }
    return text;
}

// 前缀着色：级别色 + 加粗（增强级别辨识），颜色与正文一致。
std::string PaintTag(Level lv, bool dim, const std::string& text)
{
    switch (lv) {
        case Level::LOG_DEBUG:   return style::Bold(style::Dim(text));
        case Level::LOG_INFO:    return dim ? style::Bold(style::Dim(text)) : style::Bold(text);
        case Level::LOG_SUCCESS: return style::Bold(style::Green(text));
        case Level::LOG_WARN:    return style::Bold(style::Yellow(text));
        case Level::LOG_ERROR:   return style::Bold(style::Red(text));
    }
    return text;
}

std::string TimestampNow()
{
    using namespace std::chrono;
    const auto now = system_clock::now();
    const auto ms = duration_cast<milliseconds>(now.time_since_epoch()) % 1000;
    const std::time_t t = system_clock::to_time_t(now);
    std::tm tm{};
#if defined(_WIN32)
    (void)localtime_s(&tm, &t);
#else
    (void)localtime_r(&t, &tm);
#endif
    char buf[16];
    std::strftime(buf, sizeof(buf), "%H:%M:%S", &tm);
    std::ostringstream oss;
    oss << buf << '.' << std::setfill('0') << std::setw(MS_DIGIT_WIDTH) << ms.count();
    return oss.str();
}

unsigned ThreadIdShort()
{
    std::ostringstream oss;
    oss << std::this_thread::get_id();
    std::hash<std::string> h;
    return static_cast<unsigned>(h(oss.str())) % 10000u;
}

} // namespace

void SetLevel(Level min) { g_level.store(min); }
Level level() { return g_level.load(); }
void SetPrefix(Prefix flags) { g_prefix.store(static_cast<unsigned>(flags)); }

void SetSink(std::shared_ptr<Sink> sink)
{
    std::lock_guard<std::mutex> g(g_mutex);
    g_sink = std::move(sink);
}

void WriteLine(Level lv, bool dim, const std::string& msg)
{
    std::lock_guard<std::mutex> g(g_mutex);   // 单条日志原子输出，多线程不交错

    const unsigned prefixFlags = g_prefix.load();

    std::string prefix;
    if (prefixFlags & static_cast<unsigned>(Prefix::TIMESTAMP)) {
        prefix += "[" + TimestampNow() + "]";
    }
    if (prefixFlags & static_cast<unsigned>(Prefix::THREAD_ID)) {
        std::ostringstream oss;
        oss << '[' << ThreadIdShort() << ']';
        prefix += oss.str();
    }

    const std::string line = prefix + PaintTag(lv, dim, tag(lv)) + " " +
                             paint(lv, dim, msg) + "\n";

    if (g_sink) {
        g_sink->write(lv, line);
        return;
    }
    // 诊断统一走 stderr（无缓冲单流）：保证与 error 的严格输出顺序，并保持 stdout 干净
    std::cerr << line;
}

} // namespace log
} // namespace mem_analyzer