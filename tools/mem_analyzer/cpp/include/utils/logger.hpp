#pragma once
#include <memory>
#include <sstream>
#include <string>
#include <utility>

namespace mem_analyzer {
namespace log {

// 日志级别（数值即输出阈值，低于当前 level 的日志被丢弃）。
enum class Level { Debug = 0, Info = 1, Success = 2, Warn = 3, Error = 4 };

// 可选前缀组件（位掩码）。
enum class Prefix : unsigned {
    None      = 0,
    Timestamp = 1u << 0,   // [HH:MM:SS.mmm]
    ThreadId  = 1u << 1,   // [tid]
};

// ── 全局状态 ──
void set_level(Level min);          // 低于 min 的日志被丢弃；默认 Info
Level level();
void set_prefix(Prefix flags);      // 默认 None

// ── 后端抽象（默认 console：stderr）──
class Sink {
public:
    virtual ~Sink() = default;
    virtual void write(Level lv, const std::string& line) = 0;
};
void set_sink(std::shared_ptr<Sink> sink);

// 内部：拼前缀 + 整条着色 + 落盘（logger.cpp 实现）。dim 仅 Info 使用。
void write_line(Level lv, bool dim, const std::string& msg);

// ── 六个等级入口（整条同色）──
template <typename... Args> void logd(Args&&... a)       { emit(Level::Debug,   true,  std::forward<Args>(a)...); }  // 灰
template <typename... Args> void logi(Args&&... a)       { emit(Level::Info,    false, std::forward<Args>(a)...); }  // 正色
template <typename... Args> void logi_minor(Args&&... a) { emit(Level::Info,    true,  std::forward<Args>(a)...); }  // 灰（次要提示）
template <typename... Args> void logok(Args&&... a)      { emit(Level::Success, false, std::forward<Args>(a)...); }  // 绿
template <typename... Args> void logw(Args&&... a)       { emit(Level::Warn,    false, std::forward<Args>(a)...); }  // 黄
template <typename... Args> void loge(Args&&... a)       { emit(Level::Error,   false, std::forward<Args>(a)...); }  // 红

// ── 核心实现（inline）──
template <typename... Args>
void emit(Level lv, bool dim, Args&&... args) {
    if (lv < level()) return;                       // ① 级别过滤（先于字符串构造）
    std::ostringstream oss;
    (oss << ... << std::forward<Args>(args));       // ② C++17 fold 拼接
    write_line(lv, dim, oss.str());                 // ③ 前缀 + 着色 + 落盘
}

} // namespace log
} // namespace mem_analyzer
