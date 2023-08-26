/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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
#ifndef SP_UTILS_H
#define SP_UTILS_H
#include <iostream>
#include <vector>
#include <map>
namespace OHOS {
namespace SmartPerf {
namespace SPUtils {
/**
 * @brief Check if the file has permission to access
 *
 * @param fileName
 * @return true
 * @return false
 */
bool FileAccess(const std::string &fileName);
/**
 * @brief Load content from file node
 *
 * @param filePath
 * @param content
 * @return true
 * @return false
 */
bool LoadFile(const std::string &filePath, std::string &content);
/**
 * @brief read command return result
 *
 * @param cmd
 * @param result
 * @return true
 * @return false
 */
bool LoadCmd(const std::string &cmd, std::string &result);
/**
 * @brief
 *
 * @param path
 * @return std::string
 */
std::string IncludePathDelimiter(const std::string &path);
/**
 * @brief
 * @param path
 * @param files
 */
void ForDirFiles(const std::string &path, std::vector<std::string> &files);

/**
 * @brief check if substr in parentstr
 *
 * @param str
 * @param sub
 * @return true
 * @return false
 */
bool IsSubString(const std::string &str, const std::string &sub);
/**
 * @brief split content by delimiter
 *
 * @param content
 * @param sp
 * @param out
 */
void StrSplit(const std::string &content, const std::string &sp, std::vector<std::string> &out);
/**
 * @brief extract number from str
 *
 * @param str
 * @return std::string
 */
std::string ExtractNumber(const std::string &str);
/**
 * @brief replace '' \r\n from str
 * @param res
 */
void ReplaceString(std::string &res);
/**
 * @brief get cur Time  longlong
 *
 */
long long GetCurTime();
/**
 * @brief get top pkg 
 *
 */
std::string GetTopPkgName();
std::string GetRadar();
std::map<std::string, std::string> GetDeviceInfo();
std::map<std::string, std::string> GetCpuInfo();
std::map<std::string, std::string> GetGpuInfo();
std::map<std::string, std::string> GetNetwork();
};
}
}

#endif // SP_UTILS_H
