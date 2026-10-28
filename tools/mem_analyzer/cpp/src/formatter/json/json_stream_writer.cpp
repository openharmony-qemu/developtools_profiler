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

JsonStreamWriter::JsonStreamWriter(const std::string& filePath)
{
    // Use wide-path aware open to handle Chinese paths on Windows
    auto ofsPath = PathFromUtf8(filePath);
    file_.open(ofsPath);
    // If the file fails to open, the stream's failbit will be set.
}

JsonStreamWriter::~JsonStreamWriter()
{
    close();
}

// -----------------------------------------------------------------------------
// Private helpers
// -----------------------------------------------------------------------------

void JsonStreamWriter::WriteIndent()
{
    for (int i = 0; i < depth_; ++i) {
        file_ << "    ";
    }
}

void JsonStreamWriter::WriteSeparator()
{
    if (!needsComma_.empty()) {
        if (needsComma_.back()) {
            file_ << ",";
        }
        file_ << "\n";
    }
}

void JsonStreamWriter::WriteEscapedString(const std::string& s)
{
    file_ << "\"";
    for (std::size_t i = 0; i < s.size(); ++i) {
        unsigned char c = static_cast<unsigned char>(s[i]);
        switch (c) {
            case '"':
                file_ << "\\\"";
                break;
            case '\\':
                file_ << "\\\\";
                break;
            case '\n':
                file_ << "\\n";
                break;
            case '\t':
                file_ << "\\t";
                break;
            case '\r':
                file_ << "\\r";
                break;
            case '\b':
                file_ << "\\b";
                break;
            case '\f':
                file_ << "\\f";
                break;
            default:
                if (c < 0x20) {
                    // Control character: write as \uXXXX
                    file_ << "\\u" <<
                          std::hex << std::setw(4) << std::setfill('0') <<
                          static_cast<int>(c) <<
                          std::dec;
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

void JsonStreamWriter::WriteNull()
{
    if (justWroteKey_) {
        justWroteKey_ = false;
    } else {
        WriteSeparator();
        WriteIndent();
    }
    file_ << "null";
    needsComma_.back() = true;
}

void JsonStreamWriter::WriteBoolean(bool val)
{
    if (justWroteKey_) {
        justWroteKey_ = false;
    } else {
        WriteSeparator();
        WriteIndent();
    }
    file_ << (val ? "true" : "false");
    needsComma_.back() = true;
}

void JsonStreamWriter::WriteNumber(double val)
{
    if (justWroteKey_) {
        justWroteKey_ = false;
    } else {
        WriteSeparator();
        WriteIndent();
    }
    // Use default std::ostream formatting for double
    file_ << val;
    needsComma_.back() = true;
}

void JsonStreamWriter::WriteNumber(int64_t val)
{
    if (justWroteKey_) {
        justWroteKey_ = false;
    } else {
        WriteSeparator();
        WriteIndent();
    }
    file_ << val;
    needsComma_.back() = true;
}

void JsonStreamWriter::WriteNumber(uint64_t val)
{
    if (justWroteKey_) {
        justWroteKey_ = false;
    } else {
        WriteSeparator();
        WriteIndent();
    }
    file_ << val;
    needsComma_.back() = true;
}

void JsonStreamWriter::WriteString(const std::string& val)
{
    if (justWroteKey_) {
        justWroteKey_ = false;
    } else {
        WriteSeparator();
        WriteIndent();
    }
    WriteEscapedString(val);
    needsComma_.back() = true;
}

void JsonStreamWriter::WriteString(const char* val)
{
    WriteString(std::string(val));
}

// -----------------------------------------------------------------------------
// Array
// -----------------------------------------------------------------------------

void JsonStreamWriter::BeginArray()
{
    if (!justWroteKey_) {
        WriteSeparator();
        WriteIndent();
    }
    justWroteKey_ = false;
    file_ << "[";
    needsComma_.push_back(false);
    depth_++;
}

void JsonStreamWriter::EndArray()
{
    depth_--;
    file_ << "\n";
    WriteIndent();
    file_ << "]";
    needsComma_.pop_back();
    // Mark the parent level that we've written a complete element
    if (!needsComma_.empty()) {
        needsComma_.back() = true;
    }
}

void JsonStreamWriter::WriteArray(const std::vector<int>& vals)
{
    BeginArray();
    for (std::size_t i = 0; i < vals.size(); ++i) {
        WriteNumber(static_cast<int64_t>(vals[i]));
    }
    EndArray();
}

void JsonStreamWriter::WriteArray(const std::vector<std::string>& vals)
{
    BeginArray();
    for (std::size_t i = 0; i < vals.size(); ++i) {
        WriteString(vals[i]);
    }
    EndArray();
}

// -----------------------------------------------------------------------------
// Object
// -----------------------------------------------------------------------------

void JsonStreamWriter::BeginObject()
{
    if (!justWroteKey_) {
        WriteSeparator();
        WriteIndent();
    }
    justWroteKey_ = false;
    file_ << "{";
    needsComma_.push_back(false);
    depth_++;
}

void JsonStreamWriter::EndObject()
{
    depth_--;
    file_ << "\n";
    WriteIndent();
    file_ << "}";
    needsComma_.pop_back();
    // Mark the parent level that we've written a complete element
    if (!needsComma_.empty()) {
        needsComma_.back() = true;
    }
}

void JsonStreamWriter::WriteKey(const std::string& key)
{
    WriteSeparator();
    WriteIndent();
    WriteEscapedString(key);
    file_ << ": ";
    justWroteKey_ = true;
}

void JsonStreamWriter::WriteKeyValue(const std::string& key, const std::string& val)
{
    WriteKey(key);
    WriteString(val);
}

void JsonStreamWriter::WriteKeyValue(const std::string& key, const char* val)
{
    WriteKeyValue(key, std::string(val));
}

void JsonStreamWriter::WriteKeyValue(const std::string& key, double val)
{
    WriteKey(key);
    WriteNumber(val);
}

void JsonStreamWriter::WriteKeyValue(const std::string& key, int64_t val)
{
    WriteKey(key);
    WriteNumber(val);
}

void JsonStreamWriter::WriteKeyValue(const std::string& key, bool val)
{
    WriteKey(key);
    WriteBoolean(val);
}

// -----------------------------------------------------------------------------
// Raw / Lifecycle
// -----------------------------------------------------------------------------

void JsonStreamWriter::WriteRaw(const std::string& raw)
{
    if (justWroteKey_) {
        justWroteKey_ = false;
    } else {
        WriteSeparator();
        WriteIndent();
    }
    // Auto-detect: single-line → write as-is; multi-line → re-indent
    if (raw.find('\n') == std::string::npos) {
        file_ << raw;
    } else {
        // Find the minimum indentation across all lines to preserve relative nesting
        std::istringstream iss(raw);
        std::string line;
        size_t minIndent = std::string::npos;
        while (std::getline(iss, line)) {
            size_t pos = line.find_first_not_of(" \t");
            if (pos != std::string::npos && pos < minIndent) {
                minIndent = pos;
            }
        }
        if (minIndent == std::string::npos) {
            minIndent = 0;
        }

        iss.clear();
        iss.seekg(0);
        bool first = true;
        while (std::getline(iss, line)) {
            if (!first) {
                file_ << "\n";
                WriteIndent();
            }
            size_t pos = line.find_first_not_of(" \t");
            if (pos != std::string::npos) {
                // Strip only the common indentation, preserve relative nesting
                size_t strip = (pos >= minIndent) ? (pos - minIndent) : 0;
                file_ << line.substr(pos - strip);
            }
            first = false;
        }
    }
    needsComma_.back() = true;
}

void JsonStreamWriter::Flush()
{
    file_.flush();
}

void JsonStreamWriter::Close()
{
    if (closed_) {
        return;
    }
    closed_ = true;
    file_.flush();
    file_.close();
}

} // namespace mem_analyzer
