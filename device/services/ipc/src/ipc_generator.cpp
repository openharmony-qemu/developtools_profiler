/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021. All rights reserved.
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

#include "ipc_generator.h"
#include "ipc_generator_impl.h"

IpcGenerator::IpcGenerator() {}

IpcGenerator::~IpcGenerator() {}

bool IpcGenerator::Generate(const google::protobuf::FileDescriptor* file,
                            const std::string& parameter,
                            google::protobuf::compiler::GeneratorContext* context,
                            std::string* error) const
{
    auto pcsp = std::make_shared<IpcGeneratorImpl>();
    std::string base_name = pcsp->SetNames(file->name(), file->package());

    for (int i = 0; i < file->service_count(); i++) {
        const google::protobuf::ServiceDescriptor* service = file->service(i);
        pcsp->AddService(service->name());
        for (int j = 0; j < service->method_count(); j++) {
            const google::protobuf::MethodDescriptor* method = service->method(j);
            const google::protobuf::Descriptor* inputType = method->input_type();
            const google::protobuf::Descriptor* outputType = method->output_type();
            pcsp->AddServiceMethod(service->name(), method->name(), inputType->name(), outputType->name());
        }
    }

    std::unique_ptr<::google::protobuf::io::ZeroCopyOutputStream> header_output(context->Open(base_name + ".ipc.h"));
    std::unique_ptr<::google::protobuf::io::ZeroCopyOutputStream> source_output(context->Open(base_name + ".ipc.cc"));

    ::google::protobuf::io::CodedOutputStream header_out(header_output.get());
    ::google::protobuf::io::CodedOutputStream source_out(source_output.get());

    std::string header_str = pcsp->GenHeader();
    header_out.WriteString(header_str);

    std::string source_str = pcsp->GenSource();
    source_out.WriteString(source_str);

    return true;
}