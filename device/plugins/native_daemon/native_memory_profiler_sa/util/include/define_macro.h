/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef DEFINE_MACRO_H
#define DEFINE_MACRO_H

namespace OHOS::Developtools::NativeDaemon {
#ifndef RET_OK
    #define RET_OK (0)
#endif

#ifndef RET_ERR
    #define RET_ERR (-1)
#endif
} // namespace OHOS::Developtools::NativeDaemon

#define DEFRET_1(data, value, ...) (value)
#define DEFRET(...) DEFRET_1(__VA_ARGS__, false)

#define WRITEBOOL(parcel, data, ...) \
    do { \
        if (!(parcel).WriteBool(data)) { \
            PROFILER_LOG_ERROR(LOG_CORE, "WriteBool "#data" failed"); \
            return DEFRET(false, ##__VA_ARGS__); \
        } \
    } while (0)

#define WRITEINT32(parcel, data, ...) \
    do { \
        if (!(parcel).WriteInt32(data)) { \
            PROFILER_LOG_ERROR(LOG_CORE, "WriteInt32 "#data" failed"); \
            return DEFRET(false, ##__VA_ARGS__); \
        } \
    } while (0)

#define WRITEINT64(parcel, data, ...) \
    do { \
        if (!(parcel).WriteInt64(data)) { \
            PROFILER_LOG_ERROR(LOG_CORE, "WriteInt64 "#data" failed"); \
            return DEFRET(false, ##__VA_ARGS__); \
        } \
    } while (0)

#define WRITEUINT8(parcel, data, ...) \
    do { \
        if (!(parcel).WriteUint8(data)) { \
            PROFILER_LOG_ERROR(LOG_CORE, "WriteUint8 "#data" failed"); \
            return DEFRET(false, ##__VA_ARGS__); \
        } \
    } while (0)

#define WRITEUINT32(parcel, data, ...) \
    do { \
        if (!(parcel).WriteUint32(data)) { \
            PROFILER_LOG_ERROR(LOG_CORE, "WriteUint32 "#data" failed"); \
            return DEFRET(false, ##__VA_ARGS__); \
        } \
    } while (0)

#define WRITEUINT64(parcel, data, ...) \
    do { \
        if (!(parcel).WriteUint64(data)) { \
            PROFILER_LOG_ERROR(LOG_CORE, "WriteUint64 "#data" failed"); \
            return DEFRET(false, ##__VA_ARGS__); \
        } \
    } while (0)

#define WRITESTRING(parcel, data, ...) \
    do { \
        if (!(parcel).WriteString(data)) { \
            PROFILER_LOG_ERROR(LOG_CORE, "WriteString "#data" failed"); \
            return DEFRET(false, ##__VA_ARGS__); \
        } \
    } while (0)

#define READBOOL(parcel, data, ...) \
    do { \
        if (!(parcel).ReadBool(data)) { \
            PROFILER_LOG_ERROR(LOG_CORE, "ReadBool "#data" failed"); \
            return DEFRET(false, ##__VA_ARGS__); \
        } \
    } while (0)

#define READINT32(parcel, data, ...) \
    do { \
        if (!(parcel).ReadInt32(data)) { \
            PROFILER_LOG_ERROR(LOG_CORE, "ReadInt32 "#data" failed"); \
            return DEFRET(false, ##__VA_ARGS__); \
        } \
    } while (0)

#define READINT64(parcel, data, ...) \
    do { \
        if (!(parcel).ReadInt64(data)) { \
            PROFILER_LOG_ERROR(LOG_CORE, "ReadInt64 "#data" failed"); \
            return DEFRET(false, ##__VA_ARGS__); \
        } \
    } while (0)

#define READUINT8(parcel, data, ...) \
    do { \
        if (!(parcel).ReadUint8(data)) { \
            PROFILER_LOG_ERROR(LOG_CORE, "ReadUint8 "#data" failed"); \
            return DEFRET(false, ##__VA_ARGS__); \
        } \
    } while (0)

#define READUINT32(parcel, data, ...) \
    do { \
        if (!(parcel).ReadUint32(data)) { \
            PROFILER_LOG_ERROR(LOG_CORE, "ReadUint32 "#data" failed"); \
            return DEFRET(false, ##__VA_ARGS__); \
        } \
    } while (0)

#define READUINT64(parcel, data, ...) \
    do { \
        if (!(parcel).ReadUint64(data)) { \
            PROFILER_LOG_ERROR(LOG_CORE, "ReadUint64 "#data" failed"); \
            return DEFRET(false, ##__VA_ARGS__); \
        } \
    } while (0)

#define READSTRING(parcel, data, ...) \
    do { \
        if (!(parcel).ReadString(data)) { \
            PROFILER_LOG_ERROR(LOG_CORE, "ReadString "#data" failed"); \
            return DEFRET(false, ##__VA_ARGS__); \
        } \
    } while (0)

#endif // DEFINE_MACRO_H