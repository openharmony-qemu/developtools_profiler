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
#include <string>
#include <fstream>
#include <vector>
#include <stack>
#include <cstdint>

namespace mem_analyzer {

/**
 * Streaming JSON writer.
 * Writes JSON incrementally to a file without building the entire string in memory.
 * Supports nested objects and arrays with proper indentation.
 */
class JsonStreamWriter {
public:
    explicit JsonStreamWriter(const std::string& file_path);
    ~JsonStreamWriter();

    // Primitives
    void write_null();
    void write_boolean(bool val);
    void write_number(double val);
    void write_number(int64_t val);
    void write_number(uint64_t val);
    void write_string(const std::string& val);
    void write_string(const char* val);  // avoids implicit bool conversion

    // Array
    void begin_array();
    void end_array();
    void write_array(const std::vector<int>& vals);
    void write_array(const std::vector<std::string>& vals);

    // Object
    void begin_object();
    void end_object();
    void write_key(const std::string& key);
    void write_key_value(const std::string& key, const std::string& val);
    void write_key_value(const std::string& key, const char* val);  // avoids implicit bool conversion
    void write_key_value(const std::string& key, double val);
    void write_key_value(const std::string& key, int64_t val);
    void write_key_value(const std::string& key, bool val);

    // Write a JSON-serialized string as raw content.
    // Single-line strings are written as-is; multi-line strings are re-indented
    // (first line inline, subsequent lines get current depth indent).
    void write_raw(const std::string& raw);

    void close();
    void flush();

private:
    void write_indent();
    void write_separator();
    void write_escaped_string(const std::string& s);

    std::ofstream file_;
    int depth_ = 0;
    std::vector<bool> needs_comma_;   // per-depth tracking of whether a comma is needed
    bool just_wrote_key_ = false;     // true if the last write was a key (next value is inline)
    bool closed_ = false;
};

} // namespace mem_analyzer
