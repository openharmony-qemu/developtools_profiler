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
#include <chrono>
#include <cinttypes>
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <memory>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "codec_cov.h"
#include "file.h"
#include "filter/slice_filter.h"
#include "http_server.h"
#include "log.h"
#include "parser/bytrace_parser/bytrace_event_parser.h"
#include "parser/bytrace_parser/bytrace_parser.h"
#include "parting_string.h"
#include "rpc_server.h"
#include "string_help.h"

#include "thread_state_flag.h"
#include "trace_streamer/trace_streamer_selector.h"
#include "trace_streamer_filters.h"
#include "version.h"
using namespace SysTuning::TraceStreamer;
using namespace SysTuning;
namespace SysTuning {
namespace TraceStreamer {
using namespace SysTuning::TraceStreamer;
using namespace SysTuning::base;
constexpr size_t G_CHUNK_SIZE = 1024 * 1024;
constexpr int G_MIN_PARAM_NUM = 2;
constexpr size_t G_FILE_PERMISSION = 664;
// set version info in meta.cpp please
void ExportStatusToLog(const std::string& dbPath, TraceParserStatus status)
{
    std::string path = dbPath + ".ohos.ts";
    std::ofstream out(path, std::ios_base::trunc);
    out << (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()))
               .count()
        << ":" << status << std::endl;
    using std::chrono::system_clock;

    system_clock::time_point today = system_clock::now();

    std::time_t tt = system_clock::to_time_t(today);
    out << "last running  time  is: " << ctime(&tt);
    out << "last running status is: " << status;
    out.close();
}
void ShowHelpInfo(const char* argv)
{
    TS_LOGI(
        "trace analyze tool, it can transfer a bytrace/htrace file into a "
        "SQLite database and save result to a local file trace_streamer.log.\n"
        "Usage: %s FILE -e sqlite_out.pb\n"
        " or    %s FILE -c\n"
        "Options:\n"
        " -e    transfer a bytrace file into a SQLiteBased DB. with -nm to except meta table\n"
        " -c    command line mode.\n"
        " -h    start HTTP server.\n"
        " -s    separate arkts-plugin data, and save it in current dir with default filename.\n"
        " -p    Specify the port of HTTP server, default is 9001.\n"
        " -i    show information.\n"
        " -v    show version.",
        argv, argv);
}
void PrintInformation()
{
    TraceStreamerConfig cfg;
    cfg.PrintInfo();
}
void PrintVersion()
{
    fprintf(stderr, "version %s\n", g_traceStreamerVersion.c_str());
}

void LoadQueryFile(const std::string& sqlOperator, std::vector<std::string>& sqlStrings)
{
    auto fd = fopen(sqlOperator.c_str(), "r");
    if (!fd) {
        TS_LOGE("open file failed!");
        return;
    }
    char buffer[G_CHUNK_SIZE];
    while (!feof(fd)) {
        std::string sqlString;
        while (fgets(buffer, sizeof(buffer), fd)) {
            std::string line = buffer;
            if (line == "\n" || line == "\r\n") {
                break;
            }
            sqlString.append(buffer);

            if (EndWith(line, ";") || EndWith(line, ";\r\n")) {
                break;
            }
        }

        if (sqlString.empty()) {
            continue;
        }
        sqlStrings.push_back(sqlString);
    }
    fclose(fd);
    fd = nullptr;
}

void ReadSqlFileAndPrintResult(TraceStreamerSelector& ts, const std::string& sqlOperator)
{
    std::vector<std::string> sqlStrings;
    LoadQueryFile(sqlOperator, sqlStrings);
    for (const auto& str : sqlStrings) {
        ts.SearchDatabase(str, true);
    }
}

bool ReadAndParser(SysTuning::TraceStreamer::TraceStreamerSelector& ta, int fd)
{
    auto startTime =
        (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()))
            .count();
    g_loadSize = 0;
    while (true) {
        std::unique_ptr<uint8_t[]> buf = std::make_unique<uint8_t[]>(G_CHUNK_SIZE);
        auto rsize = Read(fd, buf.get(), G_CHUNK_SIZE);
        if (rsize == 0) {
            break;
        }

        if (rsize < 0) {
            TS_LOGE("Reading trace file failed (errno: %d, %s)", errno, strerror(errno));
            return false;
        }
        g_loadSize += rsize;
        if (!ta.ParseTraceDataSegment(std::move(buf), static_cast<size_t>(rsize))) {
            return false;
        };
        printf("\rLoadingFile:\t%.2f MB\r", static_cast<double>(g_loadSize) / 1E6);
    }
    ta.WaitForParserEnd();
    auto endTime =
        (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()))
            .count();
    (void)fprintf(stdout, "\nParserDuration:\t%u ms\n", static_cast<unsigned int>(endTime - startTime));
    (void)fprintf(stdout, "ParserSpeed:\t%.2f MB/s\n", (g_loadSize / (endTime - startTime) / 1E3));
    return true;
}
int OpenAndParserFile(TraceStreamerSelector& ts, const std::string& traceFilePath)
{
    int fd(OpenFile(traceFilePath, O_RDONLY, G_FILE_PERMISSION));
    if (fd < 0) {
        TS_LOGE("%s does not exist", traceFilePath.c_str());
        SetAnalysisResult(TRACE_PARSER_ABNORMAL);
        return 1;
    }
    if (!ReadAndParser(ts, fd)) {
        close(fd);
        SetAnalysisResult(TRACE_PARSER_ABNORMAL);
        return 1;
    }
    MetaData* metaData = ts.GetMetaData();

    std::string fileNameTmp = traceFilePath;
#ifdef _WIN32
    if (!base::GetCoding(reinterpret_cast<const uint8_t*>(fileNameTmp.c_str()), fileNameTmp.length())) {
        fileNameTmp = base::GbkToUtf8(fileNameTmp.c_str());
    }
#endif
    metaData->SetSourceFileName(fileNameTmp);
    metaData->SetTraceType((ts.DataType() == TRACE_FILETYPE_H_TRACE) ? "proto-based-trace" : "txt-based-trace");

    close(fd);
    return 0;
}
int ExportDatabase(TraceStreamerSelector& ts, const std::string& sqliteFilePath)
{
    auto startTime =
        (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()))
            .count();
    if (!sqliteFilePath.empty()) {
        MetaData* metaData = ts.GetMetaData();
        std::string fileNameTmp = sqliteFilePath;
#ifdef _WIN32
        if (!base::GetCoding(reinterpret_cast<const uint8_t*>(fileNameTmp.c_str()), fileNameTmp.length())) {
            fileNameTmp = base::GbkToUtf8(fileNameTmp.c_str());
        }
#endif
        metaData->SetOutputFileName(fileNameTmp);
        metaData->SetParserToolVersion(g_traceStreamerVersion);
        metaData->SetParserToolPublishDateTime(g_traceStreamerPublishVersion);
        metaData->SetTraceDataSize(g_loadSize);
        fprintf(stdout, "ExportDatabase begin...\n");
        if (ts.ExportDatabase(sqliteFilePath)) {
            fprintf(stdout, "ExportDatabase failed\n");
            ExportStatusToLog(sqliteFilePath, TRACE_PARSER_ABNORMAL);
            return 1;
        }
        fprintf(stdout, "ExportDatabase end\n");
    }
    auto endTime =
        (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()))
            .count();
    endTime += 1; // for any exception of endTime == startTime
    fprintf(stdout, "ExportDuration:\t%u ms\n", static_cast<unsigned int>(endTime - startTime));
    fprintf(stdout, "ExportSpeed:\t%.2f MB/s\n", (g_loadSize / (endTime - startTime)) / 1E3);
    return 0;
}

struct TraceExportOption {
    std::string traceFilePath;
    std::string sqliteFilePath;
    std::string sqlOperatorFilePath;
    bool interactiveState = false;
    bool exportMetaTable = true;
    bool separateFile = false;
};
struct HttpOption {
    bool enable = false;
    int port = 9001;
};
int CheckFinal(char** argv, TraceExportOption& traceExportOption, HttpOption& httpOption)
{
    if ((traceExportOption.traceFilePath.empty() ||
         (!traceExportOption.interactiveState && traceExportOption.sqliteFilePath.empty())) &&
        !httpOption.enable && !traceExportOption.separateFile && traceExportOption.sqlOperatorFilePath.empty()) {
        ShowHelpInfo(argv[0]);
        return 1;
    }
    return 0;
}
int CheckArgs(int argc, char** argv, TraceExportOption& traceExportOption, HttpOption& httpOption)
{
    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "-e")) {
            i++;
            if (i == argc) {
                ShowHelpInfo(argv[0]);
                return 1;
            }
            traceExportOption.sqliteFilePath = std::string(argv[i]);
            continue;
        } else if (!strcmp(argv[i], "-c") || !strcmp(argv[i], "--command")) {
            traceExportOption.interactiveState = true;
            continue;
        } else if (!strcmp(argv[i], "-q") || !strcmp(argv[i], "--query-file")) {
            i++;
            if (i == argc) {
                ShowHelpInfo(argv[0]);
                return 1;
            }
            traceExportOption.sqlOperatorFilePath = std::string(argv[i]);
            continue;
        } else if (!strcmp(argv[i], "-i") || !strcmp(argv[i], "--info")) {
            PrintInformation();
        } else if (!strcmp(argv[i], "-s") || !strcmp(argv[i], "--s")) {
            traceExportOption.separateFile = true;
            continue;
        } else if (!strcmp(argv[i], "-nm") || !strcmp(argv[i], "--nometa")) {
            traceExportOption.exportMetaTable = false;
            continue;
        } else if (!strcmp(argv[i], "-v") || !strcmp(argv[i], "--v") || !strcmp(argv[i], "-version") ||
                   !strcmp(argv[i], "--version")) {
            PrintVersion();
            return 1;
        } else if (!strcmp(argv[i], "-h")) {
            httpOption.enable = true;
            continue;
        } else if (!strcmp(argv[i], "-p")) {
            if (++i == argc) {
                ShowHelpInfo(argv[0]);
                return 1;
            }
            httpOption.port = std::stoi(argv[i]);
            continue;
        }
        traceExportOption.traceFilePath = std::string(argv[i]);
    }
    return CheckFinal(argv, traceExportOption, httpOption);
}
} // namespace TraceStreamer
} // namespace SysTuning
int main(int argc, char** argv)
{
    if (argc < G_MIN_PARAM_NUM) {
        ShowHelpInfo(argv[0]);
        return 1;
    }
    TraceExportOption tsOption;
    HttpOption httpOption;
    int ret = CheckArgs(argc, argv, tsOption, httpOption);
    if (ret) {
        if (!tsOption.sqliteFilePath.empty()) {
            ExportStatusToLog(tsOption.sqliteFilePath, GetAnalysisResult());
        }
        return 0;
    }
    if (httpOption.enable) {
        RpcServer rpcServer;
        HttpServer httpServer;
        httpServer.RegisterRpcFunction(&rpcServer);
        httpServer.Run(httpOption.port);
        return 0;
    }
    TraceStreamerSelector ts;
    ts.EnableMetaTable(tsOption.exportMetaTable);
    ts.EnableFileSave(tsOption.separateFile);
    if (OpenAndParserFile(ts, tsOption.traceFilePath)) {
        if (!tsOption.sqliteFilePath.empty()) {
            ExportStatusToLog(tsOption.sqliteFilePath, GetAnalysisResult());
        }
        return 1;
    }
    if (tsOption.interactiveState) {
        MetaData* metaData = ts.GetMetaData();
        metaData->SetOutputFileName("command line mode");
        metaData->SetParserToolVersion(g_traceStreamerVersion.c_str());
        metaData->SetParserToolPublishDateTime(g_traceStreamerPublishVersion.c_str());
        metaData->SetTraceDataSize(g_loadSize);
        while (1) {
            auto values = ts.SearchData();
            std::string symbolsPath = "default";
            if (!values.empty()) {
                ts.ReloadSymbolFiles(symbolsPath, values);
            } else {
                return 0;
            }
        }
    }
    if (!tsOption.sqliteFilePath.empty()) {
        if (ExportDatabase(ts, tsOption.sqliteFilePath)) {
            ExportStatusToLog(tsOption.sqliteFilePath, GetAnalysisResult());
            return 1;
        }
        if (!tsOption.sqliteFilePath.empty()) {
            ExportStatusToLog(tsOption.sqliteFilePath, GetAnalysisResult());
        }
    }
    if (!tsOption.sqlOperatorFilePath.empty()) {
        ReadSqlFileAndPrintResult(ts, tsOption.sqlOperatorFilePath);
    }
    return 0;
}
