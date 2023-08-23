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
#include "sqlite_ext_funcs.h"
#include <cmath>
#include <functional>
#include <memory>
#include "log.h"
#include "sqlite3.h"
#include "../base/string_help.h"
namespace SysTuning {
namespace base {
/*
** Return a stdev value
*/
static void sqliteExtStdevFinalize(sqlite3_context* context)
{
    StdevCtx* ptr = static_cast<StdevCtx*>(sqlite3_aggregate_context(context, 0));
    if (ptr && ptr->cntValue > 1) {
        sqlite3_result_double(context, sqrt(ptr->rSValue / (ptr->cntValue - 1)));
    } else {
        sqlite3_result_double(context, 0.0);
    }
}
/*
** called each value received during a calculation of stdev or variance
*/
static void sqliteExtStdevNextStep(sqlite3_context* context, int32_t argc, sqlite3_value** argv)
{
    TS_ASSERT(argc == 1);
    StdevCtx* ptr = static_cast<StdevCtx*>(sqlite3_aggregate_context(context, sizeof(StdevCtx)));
    if (SQLITE_NULL != sqlite3_value_numeric_type(argv[0])) {
        ptr->cntValue++;
        double x = sqlite3_value_double(argv[0]);
        double deltaValue = (x - ptr->rMValue);
        ptr->rMValue += deltaValue / ptr->cntValue;
        ptr->rSValue += deltaValue * (x - ptr->rMValue);
    }
}

enum Type {
    TS_NULL = 0,
    TS_LONG,
    TS_DOUBLE,
    TS_STRING,
    TS_BYTES,
};

struct TSSqlValue {
    TSSqlValue() = default;

    static TSSqlValue Long(int64_t v)
    {
        TSSqlValue value;
        value.longValue = v;
        value.type = Type::TS_LONG;
        return value;
    }

    static TSSqlValue Double(double v)
    {
        TSSqlValue value;
        value.doubleValue = v;
        value.type = Type::TS_DOUBLE;
        return value;
    }

    static TSSqlValue String(const char* v)
    {
        TSSqlValue value;
        value.stringValue = v;
        value.type = Type::TS_STRING;
        return value;
    }

    static TSSqlValue Bytes(const char* v, size_t size)
    {
        TSSqlValue value;
        value.bytesValue = v;
        value.bytesCount = size;
        value.type = Type::TS_BYTES;
        return value;
    }

    double GetDouble() const
    {
        return doubleValue;
    }
    int64_t GetLong() const
    {
        return longValue;
    }
    const char* GetString() const
    {
        return stringValue;
    }
    const void* GetBytes() const
    {
        return bytesValue;
    }

    bool IsNull() const
    {
        return type == Type::TS_NULL;
    }

    union {
        const char* stringValue;
        int64_t longValue;
        double doubleValue;
        const void* bytesValue;
    };
    size_t bytesCount = 0;
    Type type = TS_NULL;
};

TSSqlValue SqliteValueToTSSqlValue(sqlite3_value* value)
{
    TSSqlValue sqlValue;
    switch (sqlite3_value_type(value)) {
        case SQLITE_INTEGER:
            sqlValue.type = Type::TS_LONG;
            sqlValue.longValue = sqlite3_value_int64(value);
            break;
        case SQLITE_FLOAT:
            sqlValue.type = Type::TS_DOUBLE;
            sqlValue.doubleValue = sqlite3_value_double(value);
            break;
        case SQLITE_TEXT:
            sqlValue.type = Type::TS_STRING;
            sqlValue.stringValue = reinterpret_cast<const char*>(sqlite3_value_text(value));
            break;
        case SQLITE_BLOB:
            sqlValue.type = Type::TS_BYTES;
            sqlValue.bytesValue = sqlite3_value_blob(value);
            sqlValue.bytesCount = static_cast<size_t>(sqlite3_value_bytes(value));
            break;
    }
    return sqlValue;
}
struct JsonBuild {
    JsonBuild() = default;
    void AppendHead()
    {
        body += "{";
    }
    void AppendTail()
    {
        body += "}";
    }
    void AppendCommon()
    {
        body += ",";
    }
    bool AppendSqlValue(const std::string& field_name, const TSSqlValue& value)
    {
        body += "\"" + field_name + "\":";
        return AppendSqlValue(value);
    }
    bool AppendSqlValue(const TSSqlValue& value)
    {
        switch (value.type) {
            case TS_LONG:
                body += std::to_string(value.longValue) + ",";
                break;
            case TS_DOUBLE:
                body += std::to_string(value.doubleValue) + ",";
                break;
            case TS_STRING:
                body += "\"" + std::string(value.stringValue) + "\"" + ",";
                break;
            case TS_BYTES:
                body += "\"" + std::string(static_cast<const char*>(value.bytesValue), value.bytesCount) + "\"" + ",";
                break;
            case TS_NULL:
                body += std::to_string(0) + ",";
                break;
        }
        return true;
    }
    std::string body;
    bool poped = false;
    void PopLast()
    {
        body.pop_back();
    }
    const std::string& Body() const
    {
        return body;
    }
};

void BuildJson(sqlite3_context* ctx, int32_t argc, sqlite3_value** argv)
{
    const int32_t PAIR_ARGS_SIZE = 2;
    if (argc % PAIR_ARGS_SIZE != 0) {
        TS_LOGI("BuildJson arg number error");
        sqlite3_result_error(ctx, "BuildJson arg number error", -1);
        return;
    }

    JsonBuild builder;
    builder.AppendHead();
    for (int32_t i = 0; i < argc; i += PAIR_ARGS_SIZE) {
        if (sqlite3_value_type(argv[i]) != SQLITE_TEXT) {
            TS_LOGI("BuildJson: Invalid args argc:%d, %d", argc, sqlite3_value_type(argv[i]));
            sqlite3_result_error(ctx, "BuildJson: Invalid args", -1);
            return;
        }

        auto* key = reinterpret_cast<const char*>(sqlite3_value_text(argv[i]));
        auto value = SqliteValueToTSSqlValue(argv[i + 1]);
        auto status = builder.AppendSqlValue(key, value);
        if (!status) {
            TS_LOGI("AppendSqlValueError");
            sqlite3_result_error(ctx, "AppendSqlValueError", -1);
            return;
        }
    }
    builder.PopLast();
    builder.AppendTail();
    std::string raw = builder.Body();
    if (raw.empty()) {
        sqlite3_result_blob(ctx, "", 0, nullptr);
        return;
    }
    std::unique_ptr<uint8_t[]> data = std::make_unique<uint8_t[]>(raw.size());
    memcpy_s(data.get(), raw.size(), raw.data(), raw.size());
    sqlite3_result_blob(ctx, data.release(), static_cast<int32_t>(raw.size()), free);
}

void RepeatedJsonStep(sqlite3_context* ctx, int32_t argc, sqlite3_value** argv)
{
    const int32_t PAIR_ARGS_SIZE = 2;
    auto** jsonBuild = static_cast<JsonBuild**>(sqlite3_aggregate_context(ctx, sizeof(JsonBuild*)));

    if (*jsonBuild == nullptr) {
        *jsonBuild = new JsonBuild();
    }
    JsonBuild* builder = *jsonBuild;
    builder->AppendHead();
    for (int32_t i = 0; i < argc; i += PAIR_ARGS_SIZE) {
        if (sqlite3_value_type(argv[i]) != SQLITE_TEXT) {
            TS_LOGI("BuildJson: Invalid args argc:%d, %d", argc, sqlite3_value_type(argv[i]));
            sqlite3_result_error(ctx, "BuildJson: Invalid args", -1);
            return;
        }

        auto* key = reinterpret_cast<const char*>(sqlite3_value_text(argv[i]));
        auto value = SqliteValueToTSSqlValue(argv[i + 1]);
        auto status = builder->AppendSqlValue(key, value);
        if (!status) {
            TS_LOGI("AppendSqlValueError");
            sqlite3_result_error(ctx, "AppendSqlValueError", -1);
            return;
        }
    }
    builder->PopLast();
    builder->AppendTail();
    builder->AppendCommon();
}
void RepeatedFieldStep(sqlite3_context* ctx, int32_t argc, sqlite3_value** argv)
{
    if (argc != 1) {
        TS_LOGE(
            "RepeatedField only support one arg, you can use BuildJson or BuildRepeatedJson function for multi args");
        return;
    }
    auto** jsonBuild = static_cast<JsonBuild**>(sqlite3_aggregate_context(ctx, sizeof(JsonBuild*)));

    if (*jsonBuild == nullptr) {
        *jsonBuild = new JsonBuild();
    }
    JsonBuild* builder = *jsonBuild;
    for (int32_t i = 0; i < argc; i++) {
        auto value = SqliteValueToTSSqlValue(argv[i]);
        auto status = builder->AppendSqlValue(value);
        if (!status) {
            sqlite3_result_error(ctx, "error", -1);
        }
    }
}

void RepeatedFieldFinal(sqlite3_context* ctx)
{
    auto** jsonBuilder = static_cast<JsonBuild**>(sqlite3_aggregate_context(ctx, 0));

    if (jsonBuilder == nullptr) {
        sqlite3_result_null(ctx);
        return;
    }

    std::unique_ptr<JsonBuild> builder(*jsonBuilder);
    std::string raw = builder->Body();
    raw.pop_back();
    if (raw.empty()) {
        sqlite3_result_null(ctx);
        return;
    }

    std::unique_ptr<uint8_t[]> data = std::make_unique<uint8_t[]>(raw.size());
    memcpy_s(data.get(), raw.size(), raw.data(), raw.size());
    sqlite3_result_blob(ctx, data.release(), static_cast<int32_t>(raw.size()), free);
}

void RepeatedJsonFinal(sqlite3_context* ctx)
{
    auto** jsonBuilder = static_cast<JsonBuild**>(sqlite3_aggregate_context(ctx, 0));

    if (jsonBuilder == nullptr) {
        sqlite3_result_null(ctx);
        return;
    }

    std::unique_ptr<JsonBuild> builder(*jsonBuilder);
    builder->PopLast();
    std::string raw = builder->Body();
    if (raw.empty()) {
        sqlite3_result_null(ctx);
        return;
    }

    std::unique_ptr<uint8_t[]> data = std::make_unique<uint8_t[]>(raw.size());
    memcpy_s(data.get(), raw.size(), raw.data(), raw.size());
    sqlite3_result_blob(ctx, data.release(), static_cast<int32_t>(raw.size()), free);
}
void ts_create_extend_function(sqlite3* db)
{
    sqlite3_create_function(db, "stdev", -1, SQLITE_UTF8, nullptr, 0, sqliteExtStdevNextStep, sqliteExtStdevFinalize);
    auto ret = sqlite3_create_function_v2(db, "RepeatedField", 1, SQLITE_UTF8, nullptr, nullptr, RepeatedFieldStep,
                                          RepeatedFieldFinal, nullptr);
    if (ret) {
        TS_LOGF("Error while initializing RepeatedField");
    }
    ret = sqlite3_create_function_v2(db, "BuildRepeatedJson", -1, SQLITE_UTF8, nullptr, nullptr, RepeatedJsonStep,
                                     RepeatedJsonFinal, nullptr);
    if (ret) {
        TS_LOGF("Error while initializing BuildRepeatedJson");
    }
    std::unique_ptr<JsonBuild> ctx = std::make_unique<JsonBuild>();
    ret = sqlite3_create_function_v2(db, "BuildJson", -1, SQLITE_UTF8, ctx.release(), BuildJson, nullptr, nullptr,
                                     [](void* ptr) { delete static_cast<JsonBuild*>(ptr); });
    if (ret != SQLITE_OK) {
        TS_LOGF("Error while initializing BuildJson");
    }
}
} // namespace base
} // namespace SysTuning