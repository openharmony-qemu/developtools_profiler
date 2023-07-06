/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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
#ifndef PROTO_READER_PLUGIN_H
#define PROTO_READER_PLUGIN_H
#include <string>
#include <google/protobuf/compiler/code_generator.h>
#include <google/protobuf/compiler/plugin.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/descriptor.pb.h>
#include <google/protobuf/io/printer.h>
#include <google/protobuf/io/zero_copy_stream.h>

namespace SysTuning {
namespace ProtoReader {
using google::protobuf::Descriptor;
using google::protobuf::EnumDescriptor;
using google::protobuf::FieldDescriptor;
using google::protobuf::FileDescriptor;
using google::protobuf::compiler::GeneratorContext;
using google::protobuf::io::Printer;
using google::protobuf::io::ZeroCopyOutputStream;

inline char Uppercase(char c)
{
    return ('a' <= c && c <= 'z') ? static_cast<char>(c + ('A' - 'a')) : c;
}

inline std::string ToUppercase(const std::string& str)
{
    std::string string(str);
    auto end = string.end();
    for (auto c = string.begin(); c != end; ++c) {
        *c = Uppercase(*c);
    }
    return string;
}

class ProtoReaderGenerator {
public:
    ProtoReaderGenerator(const FileDescriptor* file, Printer* printer) : fileDescriptor_(file), codePrinter_(printer) {}
    bool WriteProtoReader();
    void ParserNamespace();
    void ParserDescriptors();
    void ParserDependencies();
    void GetPBReaderInfo();
    void WriteBegin();
    void WriteEnumDescriptor(const EnumDescriptor* enumeration);
    void WriteDecoder(const Descriptor* descriptor);
    void WriteEnum(const Descriptor* descriptor);
    void WriteEnd();
    const std::string& GetError() const
    {
        return error_;
    }

public:
    std::string wrapperNamespace_;

private:
    template <class T>
    inline std::string GetDescriptorName(const T* descriptor) const
    {
        if (!package_.empty()) {
            auto strTmp = package_ + ".";
            return descriptor->full_name().substr(descriptor->full_name().find(strTmp) + sizeof(strTmp));
        } else {
            return descriptor->full_name();
        }
    }
    template <class T>
    std::string GetDescriptorClass(const T* descriptor)
    {
        std::string name = GetDescriptorName(descriptor);
        size_t pos = name.find(".");
        if (pos != std::string::npos) {
            name = name.replace(name.find("."), 1, "_");
        }
        return name;
    }
    std::string GetFieldNumberConstant(const FieldDescriptor* field);

private:
    struct TypeDesc {
        std::string toFunc;
        std::string type;
        std::string packedBufferType;
    };
    std::map<FieldDescriptor::Type, TypeDesc> fieldTypeDesc_ = {
        {FieldDescriptor::TYPE_BOOL, {"ToBool", "bool", "kVarInt"}},
        {FieldDescriptor::TYPE_SFIXED32, {"ToInt32", "int32_t", "kFixed32"}},
        {FieldDescriptor::TYPE_SINT32, {"ToInt32", "int32_t", "kVarInt"}},
        {FieldDescriptor::TYPE_INT32, {"ToInt32", "int32_t", "kVarInt"}},
        {FieldDescriptor::TYPE_SFIXED64, {"ToInt64", "int64_t", "kFixed64"}},
        {FieldDescriptor::TYPE_SINT64, {"ToInt64", "int64_t", "kVarInt"}},
        {FieldDescriptor::TYPE_INT64, {"ToInt64", "int64_t", "kVarInt"}},
        {FieldDescriptor::TYPE_FIXED32, {"ToUint32", "uint32_t", "kFixed32"}},
        {FieldDescriptor::TYPE_UINT32, {"ToUint32", "uint32_t", "kVarInt"}},
        {FieldDescriptor::TYPE_FIXED64, {"ToUint64", "uint64_t", "kFixed64"}},
        {FieldDescriptor::TYPE_UINT64, {"ToUint64", "uint64_t", "kVarInt"}},
        {FieldDescriptor::TYPE_FLOAT, {"ToFloat", "float", "kFixed32"}},
        {FieldDescriptor::TYPE_DOUBLE, {"ToDouble", "double", "kFixed64"}},
        {FieldDescriptor::TYPE_ENUM, {"ToInt32", "int32_t", "kVarInt"}},
        {FieldDescriptor::TYPE_STRING, {"ToString", "CharsView", ""}},
        {FieldDescriptor::TYPE_MESSAGE, {"ToBytes", "BytesView", ""}},
        {FieldDescriptor::TYPE_BYTES, {"ToBytes", "BytesView", ""}},
    };
    const FileDescriptor* const fileDescriptor_;
    Printer* const codePrinter_;
    std::string error_;
    std::string fileDefinded_;
    std::string package_;
    std::vector<std::string> vNamespaces_;
    std::string fullNamespacePrefix_;
    std::vector<const Descriptor*> vDescriptor_;
    std::vector<const EnumDescriptor*> vEnumDescriptor_;
    std::set<const FileDescriptor*> publicImports_;
    std::set<const Descriptor*> referencedMessages_;
    std::set<const EnumDescriptor*> referencedEnums_;
};
class ProtoReaderPlugin : public ::google::protobuf::compiler::CodeGenerator {
public:
    explicit ProtoReaderPlugin(){};
    ~ProtoReaderPlugin() override{};
    bool Generate(const FileDescriptor* file,
                  const std::string& options,
                  GeneratorContext* context,
                  std::string* error) const override;
};
} // namespace ProtoReader
} // namespace SysTuning
#endif // PROTO_READER_PLUGIN_H
