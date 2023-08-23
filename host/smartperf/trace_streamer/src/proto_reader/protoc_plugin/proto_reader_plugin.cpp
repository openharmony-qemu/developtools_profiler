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

#include "proto_reader_plugin.h"
#include "string_help.h"
namespace SysTuning {
namespace ProtoReader {
const std::string SYS_NAMESPACE = "SysTuning";
const int32_t MIN_OPTIONS_SIZE = 2;

bool ProtoReaderPlugin::Generate(const FileDescriptor* file,
                                 const std::string& options,
                                 GeneratorContext* context,
                                 std::string* error) const
{
    std::string newFileName = file->name().substr(0, file->name().find("proto")) + "pbreader";
    const std::unique_ptr<ZeroCopyOutputStream> generateFile(context->Open(newFileName + ".h"));
    Printer generatePrinterHead(generateFile.get(), '$');
    ProtoReaderGenerator protoReaderGenerator(file, &generatePrinterHead);
    std::vector<std::string> option = SplitStringToVec(const_cast<std::string&>(options), "=");
    if (option.size() < MIN_OPTIONS_SIZE) {
        return false;
    }
    if (option[0] == "wrapper_namespace") {
        protoReaderGenerator.wrapperNamespace_ = option[1];
    }
    if (!protoReaderGenerator.WriteProtoReader()) {
        *error = protoReaderGenerator.GetError();
        return false;
    }
    return true;
}

bool ProtoReaderGenerator::WriteProtoReader()
{
    GetPBReaderInfo();
    WriteBegin();
    for (const EnumDescriptor* enumDescriptor : vEnumDescriptor_) {
        WriteEnumDescriptor(enumDescriptor);
    }

    for (const Descriptor* descriptor : vDescriptor_) {
        WriteDecoder(descriptor);
    }
    WriteEnd();
    return error_.empty();
}
void ProtoReaderGenerator::ParserDescriptors()
{
    std::vector<const Descriptor*> vDescriptor;
    vDescriptor.reserve(static_cast<size_t>(fileDescriptor_->message_type_count()));
    for (int32_t i = 0; i < fileDescriptor_->message_type_count(); ++i) {
        vDescriptor.push_back(fileDescriptor_->message_type(i));
    }

    while (!vDescriptor.empty()) {
        const Descriptor* descriptor = vDescriptor.back();
        vDescriptor.pop_back();
        vDescriptor_.push_back(descriptor);
        for (int32_t i = 0; i < descriptor->nested_type_count(); ++i) {
            vDescriptor.push_back(descriptor->nested_type(i));
        }
    }

    for (int32_t i = 0; i < fileDescriptor_->enum_type_count(); ++i) {
        vEnumDescriptor_.push_back(fileDescriptor_->enum_type(i));
    }

    for (const Descriptor* descriptor : vDescriptor_) {
        for (int32_t i = 0; i < descriptor->enum_type_count(); ++i) {
            vEnumDescriptor_.push_back(descriptor->enum_type(i));
        }
    }
}

void ProtoReaderGenerator::ParserDependencies()
{
    for (int32_t i = 0; i < fileDescriptor_->public_dependency_count(); ++i) {
        publicImports_.insert(fileDescriptor_->public_dependency(i));
    }

    std::vector<const FileDescriptor*> vFileDescriptor;
    for (int32_t i = 0; i < fileDescriptor_->dependency_count(); ++i) {
        const FileDescriptor* fileDescriptor = fileDescriptor_->dependency(i);
        vFileDescriptor.push_back(fileDescriptor);
    }

    for (const FileDescriptor* fileDescriptor : vFileDescriptor) {
        for (int32_t i = 0; i < fileDescriptor->public_dependency_count(); ++i) {
            vFileDescriptor.push_back(fileDescriptor->public_dependency(i));
        }
    }

    for (const Descriptor* descriptor : vDescriptor_) {
        for (int32_t i = 0; i < descriptor->field_count(); ++i) {

            if (descriptor->field(i)->type() == FieldDescriptor::TYPE_MESSAGE) {
                if (!publicImports_.count(descriptor->field(i)->message_type()->file())) {
                    referencedMessages_.insert(descriptor->field(i)->message_type());
                }
            } else if (descriptor->field(i)->type() == FieldDescriptor::TYPE_ENUM) {
                if (!publicImports_.count(descriptor->field(i)->enum_type()->file())) {
                    referencedEnums_.insert(descriptor->field(i)->enum_type());
                }
            }
        }
    }
}

void ProtoReaderGenerator::ParserNamespace()
{
    vNamespaces_.push_back(SYS_NAMESPACE);
    if (!wrapperNamespace_.empty()) {
        vNamespaces_.push_back(wrapperNamespace_);
    }

    fullNamespacePrefix_ = "::";
    for (const std::string& namespaces : vNamespaces_)
        fullNamespacePrefix_ += namespaces + "::";
}

void ProtoReaderGenerator::GetPBReaderInfo()
{
    ParserNamespace();
    ParserDescriptors();
    ParserDependencies();
}

void ProtoReaderGenerator::WriteBegin()
{
    std::string notify = "// Autogenerated by the ProtoReader plugin. DO NOT EDIT.\n";
    std::string fileDefinded = fileDescriptor_->name() + "_H";
    fileDefinded = ToUppercase(fileDefinded);
    fileDefinded_ = fileDefinded.replace(fileDefinded.find("."), 1, "_");

    codePrinter_->Print(
        "$notify$\n"
        "#ifndef $fileDefinded$\n"
        "#define $fileDefinded$\n\n"
        "#include <stddef.h>\n"
        "#include <stdint.h>\n\n"
        "#include \"proto_reader/include/proto_reader.h\"\n"
        "#include \"proto_reader/include/data_area.h\"\n",
        "notify", notify, "fileDefinded", fileDefinded_);
    codePrinter_->Print("\n");

    for (const std::string& tsNamespace : vNamespaces_) {
        codePrinter_->Print("namespace $namespace$ {\n", "namespace", tsNamespace);
    }
    codePrinter_->Print("\n");

    for (const Descriptor* descriptor : referencedMessages_) {
        codePrinter_->Print("class $class$;\n", "class", GetDescriptorClass(descriptor));
    }
    for (const EnumDescriptor* enumDescriptor : referencedEnums_) {
        codePrinter_->Print("enum $class$ : int32_t;\n", "class", GetDescriptorClass(enumDescriptor));
    }
    codePrinter_->Print("\n");
}

void ProtoReaderGenerator::WriteEnumDescriptor(const EnumDescriptor* enumDescriptor)
{
    codePrinter_->Print("enum $class$ : int32_t {\n", "class", GetDescriptorClass(enumDescriptor));
    codePrinter_->Indent();

    std::string minName = "";
    std::string maxName = "";
    int32_t minNum = std::numeric_limits<int32_t>::max();
    int32_t maxNum = -1;

    std::string containingName;
    if (enumDescriptor->containing_type() != nullptr) {
        containingName = GetDescriptorClass(enumDescriptor) + "_";
    }

    for (int32_t i = 0; i < enumDescriptor->value_count(); ++i) {
        codePrinter_->Print("$name$ = $number$,\n", "name", containingName + enumDescriptor->value(i)->name(), "number",
                            std::to_string(enumDescriptor->value(i)->number()));
        if (enumDescriptor->value(i)->number() < minNum) {
            minNum = enumDescriptor->value(i)->number();
            minName = containingName + enumDescriptor->value(i)->name();
        }
        if (enumDescriptor->value(i)->number() > maxNum) {
            maxNum = enumDescriptor->value(i)->number();
            maxName = containingName + enumDescriptor->value(i)->name();
        }
    }
    codePrinter_->Outdent();
    codePrinter_->Print("};\n\n");
    codePrinter_->Print("const $class$ $class$_MIN = $min$;\n", "class", GetDescriptorClass(enumDescriptor), "min",
                        minName);
    codePrinter_->Print("const $class$ $class$_MAX = $max$;\n", "class", GetDescriptorClass(enumDescriptor), "max",
                        maxName);
    codePrinter_->Print("\n");
}

void ProtoReaderGenerator::WriteDecoder(const Descriptor* descriptor)
{
    int32_t maxFieldID = 0;
    for (int32_t i = 0; i < descriptor->field_count(); ++i) {
        const FieldDescriptor* field = descriptor->field(i);
        maxFieldID = std::max(maxFieldID, field->number());
    }

    std::string className = GetDescriptorClass(descriptor) + "_Reader";
    codePrinter_->Print(
        "class $name$ : public "
        "TypedProtoReader<$maxDataAreaID$> {\n",
        "name", className, "maxDataAreaID", std::to_string(maxFieldID));
    codePrinter_->Print(" public:\n");
    maxFieldID = 1 + maxFieldID;

    WriteEnum(descriptor);
    codePrinter_->Indent();
    codePrinter_->Print(
        "$name$(const uint8_t* data, size_t len) "
        ": TypedProtoReader(data, len) {}\n",
        "name", className);
    codePrinter_->Print(
        "explicit $name$(const std::string& raw) : "
        "TypedProtoReader(reinterpret_cast<const uint8_t*>(raw.data()), "
        "raw.size()) {}\n",
        "name", className);
    codePrinter_->Print(
        "explicit $name$(const BytesView& raw) : "
        "TypedProtoReader(raw.data_, raw.size_) {}\n",
        "name", className);

    for (int32_t i = 0; i < descriptor->field_count(); ++i) {
        const FieldDescriptor* field = descriptor->field(i);
        if (field->number() > maxFieldID) {
            codePrinter_->Print("// dataArea $name$ exceeded the maximum\n", "name", field->name());
            continue;
        }
        TypeDesc typeDesc{};
        auto t = fieldTypeDesc_.find(field->type());
        if (t != fieldTypeDesc_.end()) {
            typeDesc = t->second;
        }
        std::string toFunc = typeDesc.toFunc;
        std::string type = typeDesc.type;
        codePrinter_->Print("bool has_$name$() const { return at<$id$>().DataAreaValid(); }\n", "name",
                            field->lowercase_name(), "id", std::to_string(field->number()));
        if (field->is_packed()) {
            TypeDesc fieldTypeDesc{};
            auto fieldType = fieldTypeDesc_.find(field->type());
            if (fieldType != fieldTypeDesc_.end()) {
                fieldTypeDesc = fieldType->second;
            }
            const char* protoReaderWireType = fieldTypeDesc.packedBufferType.c_str();
            codePrinter_->Print(
                "PackedRepeatedDataAreaIterator<ProtoWireType::$protoReaderWireType$, $type$> "
                "$name$(bool* parseErrorInfo) const { return "
                "GetPackedRepeated<ProtoWireType::$protoReaderWireType$, $type$>($id$, "
                "parseErrorInfo); }\n",
                "protoReaderWireType", protoReaderWireType, "type", type, "name", field->lowercase_name(), "id",
                std::to_string(field->number()));
        } else if (field->is_repeated()) {
            codePrinter_->Print(
                "RepeatedDataAreaIterator<$type$> $name$() const { "
                "return "
                "ProtoReaderBase::GetRepeated<$type$>($id$); }\n",
                "name", field->lowercase_name(), "type", type, "id", std::to_string(field->number()));
        } else {
            codePrinter_->Print("$type$ $name$() const { return at<$id$>().$toFunc$(); }\n", "name",
                                field->lowercase_name(), "id", std::to_string(field->number()), "type", type, "toFunc",
                                toFunc);
        }
    }
    codePrinter_->Outdent();
    codePrinter_->Print("};\n\n");
}

void ProtoReaderGenerator::WriteEnum(const Descriptor* descriptor)
{
    if (descriptor->field_count()) {
        codePrinter_->Print("enum : int32_t {\n");
        codePrinter_->Indent();

        for (int32_t i = 0; i < descriptor->field_count(); ++i) {
            const FieldDescriptor* field = descriptor->field(i);
            codePrinter_->Print("$name$ = $id$,\n", "name", GetFieldNumberConstant(field), "id",
                                std::to_string(field->number()));
        }
        codePrinter_->Outdent();
        codePrinter_->Print("};\n");
    }
}

void ProtoReaderGenerator::WriteEnd()
{
    for (uint32_t i = vNamespaces_.size(); i > 0; --i) {
        codePrinter_->Print("} // namespace $namespace$\n", "namespace", vNamespaces_.at(i - 1));
    }
    codePrinter_->Print("#endif // $fileDefinded$\n", "fileDefinded", fileDefinded_);
}

std::string ProtoReaderGenerator::GetFieldNumberConstant(const FieldDescriptor* field)
{
    std::string name = field->camelcase_name();
    if (!name.empty()) {
        name.at(0) = Uppercase(name.at(0));
        name = "k" + name + "DataAreaNumber";
        return name;
    }
    return "";
}
} // namespace ProtoReader
} // namespace SysTuning

int main(int argc, char* argv[])
{
    SysTuning::ProtoReader::ProtoReaderPlugin protoReaderGenerate;
    return google::protobuf::compiler::PluginMain(argc, argv, &protoReaderGenerate);
}
