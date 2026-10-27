#pragma once
// ============================================================================
//  leak_file_reader.hpp
//
//  读取 Native Leaks 不可达内存泄漏文件（trace -l 输入）：
//    偏移 0-7 :  magic "NSNAP1.0"
//    偏移 8-15:  count (uint64 LE) 泄漏块数
//    偏移 16+ :  count x 16 字节（8B 地址 LE + 8B 大小 LE）
//  返回所有泄漏地址的集合。count=0 返回空集合。
// ============================================================================
#include <cstdint>
#include <string>
#include <unordered_set>

namespace mem_analyzer {

std::unordered_set<uint64_t> read_leak_file(const std::string& path);

} // namespace mem_analyzer
