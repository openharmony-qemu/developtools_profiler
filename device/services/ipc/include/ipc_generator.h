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

#ifndef IPC_GENERATOR_H
#define IPC_GENERATOR_H

#include <map>
#include <string>
#include <vector>
#include "google/protobuf/compiler/code_generator.h"
#include "google/protobuf/compiler/plugin.h"
#include "google/protobuf/descriptor.h"
#include "google/protobuf/io/coded_stream.h"
#include "google/protobuf/io/zero_copy_stream.h"

class IpcGeneratorImpl;

class IpcGenerator : public google::protobuf::compiler::CodeGenerator {
public:
    IpcGenerator();
    ~IpcGenerator();
    bool Generate(const google::protobuf::FileDescriptor* file,
                  const std::string& parameter,
                  google::protobuf::compiler::GeneratorContext* context,
                  std::string* error) const override;
};

#endif