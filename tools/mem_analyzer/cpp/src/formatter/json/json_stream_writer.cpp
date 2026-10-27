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
#include "formatter/json/json_stream_writer.hpp"
#include "utils/file_utils.hpp"
#include <iomanip>
#include <sstream>

namespace mem_analyzer {

// -----------------------------------------------------------------------------
// Construction / Destruction
// -----------------------------------------------------------------------------

JsonStreamWriter::JsonStreamWriter(const std::string& file_path)
{
    // Use wide-path aware open to handle Chinese paths on Windows
    auto ofs_path = path_from_utf8(file_path);
    file_.open(ofs_path);
    // If the file fails to open, the stream's failbit will be set.
}

JsonStreamWriter::~JsonStreamWriter() {
    close();
}

// -----------------------------------------------------------------------------
// Private helpers
// -----------------------------------------------------------------------------

void JsonStreamWriter::write_indent() {
    for (int i = 0; i < depth_; ++i) {
        file_ << "    ";
    }
}

void JsonStreamWriter::write_separator() {
    if (!needs_comma_.empty()) {
        if (needs_comma_.back()) {
            file_ << ",";
        }
        file_ << "\n";
    }
}

void JsonStreamWriter::write_escaped_string(const std::string& s) {
    file_ << "\"";
    for (std::size_t i = 0; i < s.size(); ++i) {
        unsigned char c = static_cast<unsigned char>(s[i]);
        switch (c) {
            case '"':  file_ << "\\\""; break;
            case '\\': file_ << "\\\\"; break;
            case '\n': file_ << "\\n";  break;
            case '\t': file_ << "\\t";  break;
            case '\r': file_ << "\\r";  break;
            case '\b': file_ << "\\b";  break;
            case '\f': file_ << "\\f";  break;
            default:
                if (c < 0x20) {
                    // Control character: write as \uXXXX
                    file_ << "\\u"
                          << std::hex << std::setw(4) << std::setfill('0')
                          << static_cast<int>(c)
                          << std::dec;
                } else {
                    file_ << static_cast<char>(c);
                }
                break;
        }
    }
    file_ << "\"";
}

// -----------------------------------------------------------------------------
// Primitives
// -----------------------------------------------------------------------------

void JsonStreamWriter::write_null() {
    if (just_wrote_key_) {
        just_wrote_key_ = false;
    } else {
        write_separator();
        write_indent();
    }
    file_ << "null";
    needs_comma_.back() = true;
}

void JsonStreamWriter::write_boolean(bool val) {
    if (just_wrote_key_) {
        just_wrote_key_ = false;
    } else {
        write_separator();
        write_indent();
    }
    file_ << (val ? "true" : "false");
    needs_comma_.back() = true;
}

void JsonStreamWriter::write_number(double val) {
    if (just_wrote_key_) {
        just_wrote_key_ = false;
    } else {
        write_separator();
        write_indent();
    }
    // Use default std::ostream formatting for double
    file_ << val;
    needs_comma_.back() = true;
}

void JsonStreamWriter::write_number(int64_t val) {
    if (just_wrote_key_) {
        just_wrote_key_ = false;
    } else {
        write_separator();
        write_indent();
    }
    file_ << val;
    needs_comma_.back() = true;
}

void JsonStreamWriter::write_number(uint64_t val) {
    if (just_wrote_key_) {
        just_wrote_key_ = false;
    } else {
        write_separator();
        write_indent();
    }
    file_ << val;
    needs_comma_.back() = true;
}

void JsonStreamWriter::write_string(const std::string& val) {
    if (just_wrote_key_) {
        just_wrote_key_ = false;
    } else {
        write_separator();
        write_indent();
    }
    write_escaped_string(val);
    needs_comma_.back() = true;
}

void JsonStreamWriter::write_string(const char* val) {
    write_string(std::string(val));
}

// -----------------------------------------------------------------------------
// Array
// -----------------------------------------------------------------------------

void JsonStreamWriter::begin_array() {
    if (!just_wrote_key_) {
        write_separator();
        write_indent();
    }
    just_wrote_key_ = false;
    file_ << "[";
    needs_comma_.push_back(false);
    depth_++;
}

void JsonStreamWriter::end_array() {
    depth_--;
    file_ << "\n";
    write_indent();
    file_ << "]";
    needs_comma_.pop_back();
    // Mark the parent level that we've written a complete element
    if (!needs_comma_.empty()) {
        needs_comma_.back() = true;
    }
}

void JsonStreamWriter::write_array(const std::vector<int>& vals) {
    begin_array();
    for (std::size_t i = 0; i < vals.size(); ++i) {
        write_number(static_cast<int64_t>(vals[i]));
    }
    end_array();
}

void JsonStreamWriter::write_array(const std::vector<std::string>& vals) {
    begin_array();
    for (std::size_t i = 0; i < vals.size(); ++i) {
        write_string(vals[i]);
    }
    end_array();
}

// -----------------------------------------------------------------------------
// Object
// -----------------------------------------------------------------------------

void JsonStreamWriter::begin_object() {
    if (!just_wrote_key_) {
        write_separator();
        write_indent();
    }
    just_wrote_key_ = false;
    file_ << "{";
    needs_comma_.push_back(false);
    depth_++;
}

void JsonStreamWriter::end_object() {
    depth_--;
    file_ << "\n";
    write_indent();
    file_ << "}";
    needs_comma_.pop_back();
    // Mark the parent level that we've written a complete element
    if (!needs_comma_.empty()) {
        needs_comma_.back() = true;
    }
}

void JsonStreamWriter::write_key(const std::string& key) {
    write_separator();
    write_indent();
    write_escaped_string(key);
    file_ << ": ";
    just_wrote_key_ = true;
}

void JsonStreamWriter::write_key_value(const std::string& key, const std::string& val) {
    write_key(key);
    write_string(val);
}

void JsonStreamWriter::write_key_value(const std::string& key, const char* val) {
    write_key_value(key, std::string(val));
}

void JsonStreamWriter::write_key_value(const std::string& key, double val) {
    write_key(key);
    write_number(val);
}

void JsonStreamWriter::write_key_value(const std::string& key, int64_t val) {
    write_key(key);
    write_number(val);
}

void JsonStreamWriter::write_key_value(const std::string& key, bool val) {
    write_key(key);
    write_boolean(val);
}

// -----------------------------------------------------------------------------
// Raw / Lifecycle
// -----------------------------------------------------------------------------

void JsonStreamWriter::write_raw(const std::string& raw) {
    if (just_wrote_key_) {
        just_wrote_key_ = false;
    } else {
        write_separator();
        write_indent();
    }
    // Auto-detect: single-line → write as-is; multi-line → re-indent
    if (raw.find('\n') == std::string::npos) {
        file_ << raw;
    } else {
        // Find the minimum indentation across all lines to preserve relative nesting
        std::istringstream iss(raw);
        std::string line;
        size_t min_indent = std::string::npos;
        while (std::getline(iss, line)) {
            size_t pos = line.find_first_not_of(" \t");
            if (pos != std::string::npos && pos < min_indent) {
                min_indent = pos;
            }
        }
        if (min_indent == std::string::npos) min_indent = 0;

        iss.clear();
        iss.seekg(0);
        bool first = true;
        while (std::getline(iss, line)) {
            if (!first) {
                file_ << "\n";
                write_indent();
            }
            size_t pos = line.find_first_not_of(" \t");
            if (pos != std::string::npos) {
                // Strip only the common indentation, preserve relative nesting
                size_t strip = (pos >= min_indent) ? (pos - min_indent) : 0;
                file_ << line.substr(pos - strip);
            }
            first = false;
        }
    }
    needs_comma_.back() = true;
}

void JsonStreamWriter::flush() {
    file_.flush();
}

void JsonStreamWriter::close() {
    if (closed_) return;
    closed_ = true;
    file_.flush();
    file_.close();
}

} // namespace mem_analyzer
