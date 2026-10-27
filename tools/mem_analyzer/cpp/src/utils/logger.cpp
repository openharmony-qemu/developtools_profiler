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

std::atomic<Level> g_level{Level::Info};
std::atomic<unsigned> g_prefix{0};
std::mutex g_mutex;                          // 保护 g_sink 与单条日志的原子输出
std::shared_ptr<Sink> g_sink;                // null = 默认 console（stdout/stderr）

// 级别标签（定宽 7 字符，便于对齐）。
const char* tag(Level lv) {
    switch (lv) {
        case Level::Debug:   return "[DEBUG]";
        case Level::Info:    return "[ INFO]";
        case Level::Success: return "[  OK ]";
        case Level::Warn:    return "[ WARN]";
        case Level::Error:   return "[ERROR]";
    }
    return "[     ]";
}

// 正文着色（整条同色的正文部分）：正色 = 不染色。
std::string paint(Level lv, bool dim, const std::string& text) {
    switch (lv) {
        case Level::Debug:   return style::dim(text);
        case Level::Info:    return dim ? style::dim(text) : text;
        case Level::Success: return style::green(text);
        case Level::Warn:    return style::yellow(text);
        case Level::Error:   return style::red(text);
    }
    return text;
}

// 前缀着色：级别色 + 加粗（增强级别辨识），颜色与正文一致。
std::string paint_tag(Level lv, bool dim, const std::string& text) {
    switch (lv) {
        case Level::Debug:   return style::bold(style::dim(text));
        case Level::Info:    return dim ? style::bold(style::dim(text)) : style::bold(text);
        case Level::Success: return style::bold(style::green(text));
        case Level::Warn:    return style::bold(style::yellow(text));
        case Level::Error:   return style::bold(style::red(text));
    }
    return text;
}

std::string timestamp_now() {
    using namespace std::chrono;
    const auto now = system_clock::now();
    const auto ms = duration_cast<milliseconds>(now.time_since_epoch()) % 1000;
    const std::time_t t = system_clock::to_time_t(now);
    std::tm tm{};
#if defined(_WIN32)
    localtime_s(&tm, &t);
#else
    localtime_r(&t, &tm);
#endif
    char buf[16];
    std::strftime(buf, sizeof(buf), "%H:%M:%S", &tm);
    std::ostringstream oss;
    oss << buf << '.' << std::setfill('0') << std::setw(3) << ms.count();
    return oss.str();
}

unsigned thread_id_short() {
    std::ostringstream oss;
    oss << std::this_thread::get_id();
    std::hash<std::string> h;
    return static_cast<unsigned>(h(oss.str())) % 10000u;
}

} // namespace

void set_level(Level min) { g_level.store(min); }
Level level() { return g_level.load(); }
void set_prefix(Prefix flags) { g_prefix.store(static_cast<unsigned>(flags)); }

void set_sink(std::shared_ptr<Sink> sink) {
    std::lock_guard<std::mutex> g(g_mutex);
    g_sink = std::move(sink);
}

void write_line(Level lv, bool dim, const std::string& msg) {
    std::lock_guard<std::mutex> g(g_mutex);   // 单条日志原子输出，多线程不交错

    const unsigned prefix_flags = g_prefix.load();

    std::string prefix;
    if (prefix_flags & static_cast<unsigned>(Prefix::Timestamp)) {
        prefix += "[" + timestamp_now() + "]";
    }
    if (prefix_flags & static_cast<unsigned>(Prefix::ThreadId)) {
        std::ostringstream oss;
        oss << '[' << thread_id_short() << ']';
        prefix += oss.str();
    }

    const std::string line = prefix + paint_tag(lv, dim, tag(lv)) + " " +
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