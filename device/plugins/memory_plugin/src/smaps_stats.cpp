/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021. All rights reserved.
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
#include "smaps_stats.h"

#include "securec.h"

namespace {
const int PERCENT = 100;

bool MatchHead(const std::string& name, const char* str)
{
    return strncmp(name.c_str(), str, strlen(str)) == 0;
}

bool MatchTail(const std::string& name, const char* str)
{
    CHECK_TRUE(name.size() >= strlen(str), false, "str is larger than name");
    int index = name.size() - strlen(str);
    if (index < 0) {
        return false;
    }
    return (name.substr(index) == str);
}
} // namespace
std::string SmapsStats::ParseCategory(const SmapsHeadInfo& smapsHeadInfo)
{
    std::string category;
    if (GetCategoryFromMap(smapsHeadInfo.path, category, endMap_, &MatchTail) ||
        GetCategoryFromMap(smapsHeadInfo.path, category, beginMap_, &MatchHead)) {
        return category;
    }
    category = smapsHeadInfo.iNode > 0 ? FILE_PAGE_TAG : ANON_PAGE_TAG;
    return category;
}

bool SmapsStats::GetCategoryFromMap(const std::string &name, std::string &category,
                                    const std::map<std::string, std::string> &map, MatchFunc func)
{
    for (const auto &p : map) {
        if (func(name, p.first.c_str())) {
            category = p.second;
            return true;
        }
    }
    return false;
}

bool SmapsStats::ParseMaps(int pid, ProcessMemoryInfo& processinfo, bool isReportApp, bool isReportSmaps)
{
    std::string smaps_path = std::string("/proc/") + std::to_string(pid) + std::string("/smaps");
    if (testpath_.size() > 0) {
        smaps_path = testpath_ + std::to_string(pid) + std::string("/smaps");
    }
    ReadVmemareasFile(smaps_path, processinfo, isReportApp, isReportSmaps);
    if (isReportApp) {
        ReviseStatsData();
    }
    return true;
}

bool SmapsStats::ReadVmemareasFile(const std::string& path, ProcessMemoryInfo& processinfo, bool isReportApp,
                                   bool isReportSmaps)
{
    SmapsInfo* smapsInfo = nullptr;
    bool findMapHead = false;
    MapPiecesInfo mappic = {0};
    MemUsageInfo memusage = {0};
    SmapsHeadInfo smapsHeadInfo = {};
    uint64_t prevEnd = 0;
    int prevHeap = 0;
    std::ifstream input(path, std::ios::in);
    CHECK_TRUE(!input.fail(), false, "%s:open %s failed, errno = %d", __func__, path.c_str(), errno);
    do {
        if (!input.good()) {
            return false;
        }
        std::string line;
        getline(input, line);
        line += '\n';
        if (!findMapHead) {
            // 00400000-00409000 r-xp 00000000 fc:00 426998  /usr/lib/gvfs/gvfsd-http
            ParseMapHead(line, mappic, smapsHeadInfo);
            findMapHead = true;
            if (isReportSmaps) {
                smapsInfo = processinfo.add_smapinfo();
                smapsInfo->set_start_addr(smapsHeadInfo.startAddrStr);
                smapsInfo->set_end_addr(smapsHeadInfo.endAddrStr);
                smapsInfo->set_permission(smapsHeadInfo.permission);
                smapsInfo->set_path(smapsHeadInfo.path);
                smapsInfo->set_category(ParseCategory(smapsHeadInfo));
            }
            continue;
        }
        if (findMapHead && GetMemUsageField(line, memusage)) {
            if (!lastline_) {
                continue;
            }
            if (isReportSmaps) {
                smapsInfo->set_size(memusage.vss);
                smapsInfo->set_rss(memusage.rss);
                smapsInfo->set_pss(memusage.pss);
                smapsInfo->set_dirty(memusage.privateDirty + memusage.sharedDirty);
                smapsInfo->set_swapper(memusage.swap + memusage.swapPss);
                smapsInfo->set_reside(static_cast<double>(memusage.rss) / memusage.vss * PERCENT);
                smapsInfo->set_private_clean(memusage.privateClean);
                smapsInfo->set_private_dirty(memusage.privateDirty);
                smapsInfo->set_shared_clean(memusage.sharedClean);
                smapsInfo->set_shared_dirty(memusage.sharedDirty);
                smapsInfo->set_swap(memusage.swap);
                smapsInfo->set_swap_pss(memusage.swapPss);
            }
        }
        if (isReportApp) {
            CollectVmemAreasData(mappic, memusage, prevEnd, prevHeap);
        }
        findMapHead = false;
        lastline_ = false;
    } while (!input.eof());
    input.close();
    return true;
}

bool SmapsStats::GetVMAStuId(int ops,
                             std::string name,
                             const VmeminfoAreaMapping vma[],
                             int count,
                             int32_t heapIndex[2],
                             bool& swappable)
{
    for (int i = 0; i < count; i++) {
        if (ops == OPS_START) {
            if (MatchHead(name, vma[i].heapstr)) {
                heapIndex[0] = vma[i].heapid[0];
                heapIndex[1] = vma[i].heapid[1];
                swappable = false;
                return true;
            }
        } else if (ops == OPS_END) {
            if (MatchTail(name, vma[i].heapstr)) {
                if (vma[i].heapid[1] == VMHEAP_NEEDFIX) {
                    HeapIndexFix(name, vma[i].heapstr, heapIndex);
                } else {
                    heapIndex[0] = vma[i].heapid[0];
                    heapIndex[1] = vma[i].heapid[1];
                }
                swappable = true;
                return true;
            }
        }
    }
    return false;
}

bool SmapsStats::GetVmaIndex(std::string name, uint32_t namesz, int32_t heapIndex[2], bool& swappable)
{
    switch (name[0]) {
        case '[':
            if (MatchHead(name, "[heap]") || MatchHead(name, "[stack")) {
                return GetVMAStuId(OPS_START, name, g_vmaMemHeap, sizeof(g_vmaMemHeap) /
                                    sizeof(g_vmaMemHeap[0]), heapIndex, swappable);
            } else if (MatchHead(name, "[anon:")) {
                if (MatchHead(name, "[anon:sensitive_vm-") &&
                    GetVMAStuId(OPS_END, name, g_vmaMemSuffix, sizeof(g_vmaMemSuffix) /
                    sizeof(g_vmaMemSuffix[0]), heapIndex, swappable)) {
                        return true;
                }
                return GetVMAStuId(OPS_START, name, g_vmaMemAnon, sizeof(g_vmaMemAnon) /
                                    sizeof(g_vmaMemAnon[0]), heapIndex, swappable);
            }
            break;
        case '/':
            if (MatchHead(name, "/memfd:")) {
                return GetVMAStuId(OPS_START, name, g_vmaMemFd, sizeof(g_vmaMemFd) /
                                    sizeof(g_vmaMemFd[0]), heapIndex, swappable);
            } else if (MatchHead(name, "/dev/")) {
                return GetVMAStuId(OPS_START, name, g_vmaMemDev, sizeof(g_vmaMemDev) /
                                    sizeof(g_vmaMemDev[0]), heapIndex, swappable);
            } else {
                return GetVMAStuId(OPS_END, name, g_vmaMemSuffix, sizeof(g_vmaMemSuffix) /
                                    sizeof(g_vmaMemSuffix[0]), heapIndex, swappable);
            }
            break;
        default:
            return GetVMAStuId(OPS_END, name, g_vmaMemSuffix, sizeof(g_vmaMemSuffix) /
                                sizeof(g_vmaMemSuffix[0]), heapIndex, swappable);
            break;
    }
    if (namesz > strlen(".sensitive_jvbin") && strstr(name.c_str(), ".sensitive_jvbin") != nullptr) {
        heapIndex[0] = VMHEAP_SENSITIVE_JVBIN;
        heapIndex[1] = VMHEAP_SENSITIVE_JVBIN_APP_SENSITIVE_JVBIN;
        swappable = true;
        return true;
    }
    return false;
}

void SmapsStats::CollectVmemAreasData(const MapPiecesInfo& mempic,
                                      const MemUsageInfo& memusage,
                                      uint64_t& prevEnd,
                                      int& prevHeap)
{
    std::string name;
    int32_t heapIndex[2] = {VMHEAP_UNKNOWN, VMHEAP_NULL};
    bool swappable = false;
    uint64_t swapablePss = 0;

    if (MatchTail(mempic.name, " (deleted)")) {
        name = mempic.name.substr(0, mempic.name.size() - strlen(" (deleted)"));
    } else {
        name = mempic.name;
    }
    uint32_t namesz = name.size();
    if (!GetVmaIndex(name, namesz, heapIndex, swappable)) {
        if (namesz > 0) {
            heapIndex[0] = VMHEAP_UNKNOWN_MAP;
        } else if (mempic.startAddr == prevEnd && prevHeap == VMHEAP_SO) {
            // bss section of a shared library
            heapIndex[0] = VMHEAP_SO;
        }
    }
    prevEnd = mempic.endAddr;
    prevHeap = heapIndex[0];
    swapablePss = GetSwapablepssValue(memusage, swappable);
    SetVmemAreasData(heapIndex[0], swapablePss, memusage);
    if ((heapIndex[1] != VMHEAP_NULL) && (heapIndex[1] != VMHEAP_NEEDFIX)) {
        SetVmemAreasData(heapIndex[1], swapablePss, memusage);
    }
}

void SmapsStats::ReviseStatsData()
{
    // Summary data to VMHEAP_UNKNOWN
    for (int i = VMHEAP_NUM_CORE_HEAP; i < VMHEAP_NUM_EXCLUSIVE_HEAP; i++) {
        stats_[VMHEAP_UNKNOWN] += stats_[i];
    }
}

bool SmapsStats::SetMapAddrInfo(std::string& line, MapPiecesInfo& head)
{
    const char* pStr = line.c_str();
    char* end = nullptr;
    head.startAddr = strtoull(pStr, &end, HEX_BASE);
    CHECK_TRUE(!(end == pStr || *end != '-'), false, "SetMapAddrInfo fail");
    pStr = end + 1;
    head.endAddr = strtoull(pStr, &end, HEX_BASE);
    CHECK_TRUE(end != pStr, false, "end == pStr");
    return true;
}

bool SmapsStats::ParseMapHead(std::string& line, MapPiecesInfo& head, SmapsHeadInfo& smapsHeadInfo)
{
    std::string newline = line;
    for (int i = 0; i < FIFTH_FIELD; i++) {
        std::string word = newline;
        size_t wordsz = word.find(" ");
        CHECK_TRUE(wordsz != std::string::npos, false, "wordsz == std::string::npos");
        word = newline.substr(0, wordsz);
        if (i == 0) {
            size_t pos = word.find("-");
            if (pos != std::string::npos) {
                smapsHeadInfo.startAddrStr = word.substr(0, pos);
                smapsHeadInfo.endAddrStr = word.substr(pos + 1, word.size());
                head.startAddr = strtoull(smapsHeadInfo.startAddrStr.c_str(), nullptr, HEX_BASE);
                head.endAddr = strtoull(smapsHeadInfo.endAddrStr.c_str(), nullptr, HEX_BASE);
            }
        } else if (i == 1) {
            smapsHeadInfo.permission = word.substr(0, word.size() - 1);
        } else if (i == 4) { // 4: iNode index
            smapsHeadInfo.iNode = strtoll(word.substr(0, word.size()).c_str(), nullptr, DEC_BASE);
        }
        size_t newlineops = newline.find_first_not_of(" ", wordsz);
        newline = newline.substr(newlineops);
    }
    head.name = newline.substr(0, newline.size() - 1);
    smapsHeadInfo.path = head.name;
    return true;
}

bool SmapsStats::GetMemUsageField(std::string& line, MemUsageInfo& memusage)
{
    char field[64];
    int len = 0;
    const char* pLine = line.c_str();

    int ret = sscanf_s(pLine, "%63s %n", field, sizeof(field), &len);
    if (ret == 1 && *field && field[strlen(field) - 1] == ':') {
        const char* c = pLine + len;
        std::string strfield(field);
        switch (field[0]) {
            case 'P':
                if (MatchHead(strfield, "Pss:")) {
                    memusage.pss = strtoull(c, nullptr, DEC_BASE);
                } else if (MatchHead(strfield, "Private_Clean:")) {
                    uint64_t prcl = strtoull(c, nullptr, DEC_BASE);
                    memusage.privateClean = prcl;
                    memusage.uss += prcl;
                } else if (MatchHead(strfield, "Private_Dirty:")) {
                    uint64_t prdi = strtoull(c, nullptr, DEC_BASE);
                    memusage.privateDirty = prdi;
                    memusage.uss += prdi;
                }
                break;
            case 'S':
                if (MatchHead(strfield, "Size:")) {
                    memusage.vss = strtoull(c, nullptr, DEC_BASE);
                } else if (MatchHead(strfield, "Shared_Clean:")) {
                    memusage.sharedClean = strtoull(c, nullptr, DEC_BASE);
                } else if (MatchHead(strfield, "Shared_Dirty:")) {
                    memusage.sharedDirty = strtoull(c, nullptr, DEC_BASE);
                } else if (MatchHead(strfield, "Swap:")) {
                    memusage.swap = strtoull(c, nullptr, DEC_BASE);
                } else if (MatchHead(strfield, "SwapPss:")) {
                    memusage.swapPss = strtoull(c, nullptr, DEC_BASE);
                }
                break;
            case 'R':
                if (MatchHead(strfield, "Rss:")) {
                    memusage.rss = strtoull(c, nullptr, DEC_BASE);
                }
                break;
            case 'V':
                if (MatchHead(strfield, "VmFlags:")) {
                    lastline_ = true;
                }
                break;
            default:
                break;
        }
        return true;
    }

    return false;
}

uint64_t SmapsStats::GetSwapablepssValue(const MemUsageInfo& memusage, bool swappable)
{
    if (!swappable || (memusage.pss == 0)) {
        return 0;
    }

    if ((memusage.sharedClean == 0) && (memusage.sharedDirty == 0)) {
        return memusage.privateClean;
    }
    int proportion = (memusage.pss - memusage.uss) / (memusage.sharedClean + memusage.sharedDirty);

    return (proportion * memusage.sharedClean) + memusage.privateClean;
}

void SmapsStats::SetVmemAreasData(int index, uint64_t swapablePss, const MemUsageInfo& usage)
{
    StatsInfo oobj(swapablePss, usage);
    stats_[index] += oobj;
}

void SmapsStats::HeapIndexFix(std::string name, const char* key, int32_t heapIndex[2])
{
    if (!strncmp(key, ".vdex", sizeof(".vdex"))) {
        if ((strstr(name.c_str(), "@boot") != nullptr) || (strstr(name.c_str(), "/boot") != nullptr) ||
            (strstr(name.c_str(), "/apex") != nullptr)) {
            heapIndex[0] = VMHEAP_SENSITIVE_JVBIN;
            heapIndex[1] = VMHEAP_SENSITIVE_JVBIN_BOOT_VDEX;
        } else {
            heapIndex[0] = VMHEAP_SENSITIVE_JVBIN;
            heapIndex[1] = VMHEAP_SENSITIVE_JVBIN_APP_VDEX;
        }
    } else if (!strncmp(key, ".hrt", sizeof(".hrt")) || !strncmp(key, ".hrt]", sizeof(".hrt]"))) {
        if ((strstr(name.c_str(), "@boot") != nullptr) || (strstr(name.c_str(), "/boot") != nullptr) ||
            (strstr(name.c_str(), "/apex") != nullptr)) {
            heapIndex[0] = VMHEAP_HRT;
            heapIndex[1] = VMHEAP_HRT_BOOT;
        } else {
            heapIndex[0] = VMHEAP_HRT;
            heapIndex[1] = VMHEAP_HRT_APP;
        }
    }
}

int SmapsStats::GetProcessJavaHeap()
{
    return stats_[VMHEAP_SENSITIVE_VM].privateDirty_ + GetPrivate(VMHEAP_HRT);
}

int SmapsStats::GetProcessNativeHeap()
{
    return stats_[VMHEAP_NATIVE].privateDirty_;
}

int SmapsStats::GetProcessCode()
{
    return GetPrivate(VMHEAP_SO) + GetPrivate(VMHEAP_JAR) + GetPrivate(VMHEAP_TTF) +
           GetPrivate(VMHEAP_SENSITIVE_JVBIN) + GetPrivate(VMHEAP_OAT) +
           GetPrivate(VMHEAP_SENSITIVE_VM_OTHER_ZYGOTE_CODE_CACHE) +
           GetPrivate(VMHEAP_SENSITIVE_VM_OTHER_APP_CODE_CACHE);
}

int SmapsStats::GetProcessStack()
{
    return stats_[VMHEAP_STACK].privateDirty_;
}

int SmapsStats::GetProcessGraphics()
{
    return GetPrivate(VMHEAP_GL_DEV) + GetPrivate(VMHEAP_GRAPHICS) + GetPrivate(VMHEAP_GL);
}

int SmapsStats::GetProcessPrivateOther()
{
    return GetTotalPrivateClean() + GetTotalPrivateDirty() - GetProcessJavaHeap() - GetProcessNativeHeap() -
           GetProcessCode() - GetProcessStack() - GetProcessGraphics();
}

int SmapsStats::GetProcessSystem()
{
    return GetTotalPss() - GetTotalPrivateClean() - GetTotalPrivateDirty();
}

int SmapsStats::GetTotalPrivateClean()
{
    return stats_[VMHEAP_UNKNOWN].privateClean_ + stats_[VMHEAP_NATIVE].privateClean_ +
           stats_[VMHEAP_SENSITIVE_VM].privateClean_;
}

int SmapsStats::GetTotalPrivateDirty()
{
    return stats_[VMHEAP_UNKNOWN].privateDirty_ + stats_[VMHEAP_NATIVE].privateDirty_ +
           stats_[VMHEAP_SENSITIVE_VM].privateDirty_;
}

int SmapsStats::GetPrivate(int type)
{
    return stats_[type].privateDirty_ + stats_[type].privateClean_;
}

int SmapsStats::GetTotalPss()
{
    return stats_[VMHEAP_UNKNOWN].pss_ + stats_[VMHEAP_NATIVE].pss_ + stats_[VMHEAP_SENSITIVE_VM].pss_
            + GetTotalSwappedOutPss();
}

int SmapsStats::GetTotalSwappedOutPss()
{
    return stats_[VMHEAP_UNKNOWN].swappedOutPss_ + stats_[VMHEAP_NATIVE].swappedOutPss_ +
           stats_[VMHEAP_SENSITIVE_VM].swappedOutPss_;
}
