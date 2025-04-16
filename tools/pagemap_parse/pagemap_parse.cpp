
/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include <iostream>
#include <fstream>
#include <sstream>
#include <unistd.h>
#include <fcntl.h>
#include <vector>
#include <charconv>

using namespace std;

constexpr uint64_t PFN_MASK = ((1ULL << 55) - 1);
constexpr uint64_t PAGE_SIZE = 1024 * 4;
constexpr int ARG_MINIMUM = 2;
constexpr int IN_RAM_OFFSET = 63;
constexpr int IN_SWAP_OFFSET = 54;
constexpr int SHARED_OFFSET = 53;
constexpr int EXCLUSIVE_OFFSET = 52;
constexpr int SOFTDIRTY_OFFSET = 51;
struct MapInfo {
    uint64_t startAddr; // 起始地址
    uint64_t endAddr;   // 结束地址
    char read;
    char write;
    char execute;
    char shared;
    uint64_t offset;    // 文件偏移量
    string dev;       // 设备号
    string inode;     // inode 号
    std::string pathname;  // 文件路径
};

struct PageInfo {
    unsigned int inRam;
    unsigned int inSwap;
    unsigned int shared;
    unsigned int exclusive;
    unsigned int softdirty;
    unsigned long pfn;
    uint64_t address;
};

namespace {
void PrintUsage(const string& program)
{
    cout << "Usage: " << program << " pid" <<endl;
}

int ParseMapsLine(const string& line, MapInfo& mapping)
{
    std::istringstream iss(line);
    uint64_t start, end;

    // 读取起始地址和结束地址
    if (!(iss >> hex >> start)) {
        return -1;
    }
    iss.ignore(1); // 忽略 '-'
    if (!(iss >> hex >> end))  {
        return -1;
    }
    mapping.startAddr = start;
    mapping.endAddr = end;

    // 读取权限并转换为整数
    iss >> mapping.read;
    iss >> mapping.write;
    iss >> mapping.execute;
    iss >> mapping.shared;
    // 读取偏移量
    if (!(iss >> mapping.offset))  {
        return -1;
    }
    // 读取设备号
    if (!(iss >> mapping.dev))  {
        return -1;
    }
    // 读取 inode 号
    if (!(iss >> mapping.inode))  {
        return -1;
    }
    // 读取文件路径
    if (!getline(iss, mapping.pathname)) {
        mapping.pathname = "[anno]";
    };
    return 0;
}

void ParsePagemap(uint64_t entry, PageInfo & pginfo)
{
    pginfo.inRam    = (entry >> IN_RAM_OFFSET) & 0x1;
    pginfo.inSwap   = (entry >> IN_SWAP_OFFSET) & 0x1;
    pginfo.shared    = (entry >> SHARED_OFFSET) & 0x1;
    pginfo.exclusive = (entry >> EXCLUSIVE_OFFSET) & 0x1;
    pginfo.softdirty = (entry >> SOFTDIRTY_OFFSET) & 0x1;
    pginfo.pfn       = entry & PFN_MASK;
}

void PrintPage(const MapInfo& mapping, const PageInfo& page)
{
    cout << hex << page.address << '-' << hex << (page.address + PAGE_SIZE) << " ";
    cout << mapping.read << mapping.write << mapping.execute << mapping.shared << " ";
    if (page.inRam) {
        cout << hex << page.pfn;
    } else if (page.inSwap) {
        cout << "[in swap]";
    } else {
        cout << "[not present]";
    }
    cout<< " " << mapping.pathname << endl;
}

bool IsValidPid(const string& pid_str)
{
    if (pid_str.empty()) {
        return false;
    }
    bool ret = all_of(pid_str.begin(), pid_str.end(), [](char c) {
        return isdigit(c);
    });
    return ret;
}
} // namespace

int main(int argc, char* argv[])
{
    if (argc != ARG_MINIMUM) {
        PrintUsage(argv[0]);
        return -1;
    }
    string pid_str = argv[1];
    if (!IsValidPid(pid_str)) {
        PrintUsage(argv[0]);
        return -1;
    }
    int pid = -1;
    auto result = std::from_chars(pid_str.data(), pid_str.data() + pid_str.size(), pid);
    if (result.ec != std::errc()) {
        PrintUsage(argv[0]);
        return -1;
    }
    string mapsPath = "/proc/" + to_string(pid) + "/maps";
    ifstream maps_file(mapsPath, ios::binary);
    if (!maps_file) {
        cerr << "Failed to open maps file" << endl;
        return -1;
    }

    string pagemapPath = "/proc/" + to_string(pid) + "/pagemap";
    int pagemapFd = open(pagemapPath.c_str(), O_RDONLY);
    if (pagemapFd == -1) {
        perror("Error opening file");
        return -1;
    }
    cout << "Address Range\t" << "Permissions\t" << "PFN\t" << "Path" << endl;
    string line;
    while (getline(maps_file, line)) {
        MapInfo mapping;
        bool ret = ParseMapsLine(line, mapping);
        if (ret != 0) {
            close(pagemapFd);
            return ret;
        }
        for (uint64_t tmpAddr = mapping.startAddr; tmpAddr < mapping.endAddr; tmpAddr += PAGE_SIZE) {
            // 计算文件中要读取的偏移量
            uint64_t offset = (tmpAddr / PAGE_SIZE) * sizeof(unsigned long long);
            uint64_t entry;
            if (pread(pagemapFd, &entry, sizeof(entry), offset) != sizeof(entry)) {
                perror("pread");
                break;
            }
            PageInfo page;
            ParsePagemap(entry, page);
            page.address = tmpAddr;
            PrintPage(mapping, page);
        }
    }
    maps_file.close();
    close(pagemapFd);
    return 0;
}