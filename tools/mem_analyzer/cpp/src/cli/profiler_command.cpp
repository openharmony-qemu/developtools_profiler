/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
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
#include "cli/command.h"
#include "profiler/profiler_cmd.h"

namespace mem_analyzer {
namespace {

struct ProfilerOptions {
    std::string bundle;
    int duration = defaults::PROF_DURATION;
    bool durationSpecified = false;  // -t 是否被用户显式指定
    std::string category = defaults::HTRACE_TYPE;
    bool listApps = false;
    std::string soDir;
    std::string sourcemapDir;
    std::string nameCacheDir;
    std::string connectKey;           // --device value: device serial number
    bool connectKeyProvided = false; // whether --device was given explicitly
};

class ProfilerCommand final : public ICommand {
    ProfilerOptions opt_;

public:
    std::string Name() const override { return "profiler"; }
    std::string Alias() const override { return "prof"; }
    std::string Description() const override
    {
        return "Collect snapshot, htrace, Global/Local handle log";
    }
    int Order() const override { return 40; }

    void Bind(CliParser& sub, GlobalOptions& g) override
    {
        sub.SetUsage("[options] [<bundle>]")
            .AddPositional("bundle", opt_.bundle,
                "Target bundle name (required for capture, not needed with --list-apps)")
            .AddOption("-c,--category", opt_.category,
                "Capture type: nativeheap/nh/native, snapshot/snap, globalhandle/global/gh, "
                "allhandle/handle/ah, nativeleaks/nl, fd, dma, thread, ashmem")
            .AddOption("-t", opt_.duration, "Capture duration in seconds", &opt_.durationSpecified)
            .AddOption("--device", opt_.connectKey,
                "Target device: device serial number (see 'hdc list targets'); optional "
                "with 1 device, but required when 2 or more devices are connected or the target cannot be determined",
            &opt_.connectKeyProvided)
            .AddOption("-d,--out-dir", g.outDir, "Output directory")
            .AddOption("-S,--so-dir", opt_.soDir,
                "Directory of native .so symbol files for offline symbolization (ignored for snapshot capture)")
            .AddOption("--sourcemap", opt_.sourcemapDir,
                "Source map file or directory (sourceMaps.map / *.map) to deobfuscate heapsnapshot paths")
            .AddOption("--namecache", opt_.nameCacheDir,
                "nameCache file or directory (nameCache.json) to deobfuscate heapsnapshot names")
            .AddSwitch("-o,--auto-open", g.autoOpen, "Generate HTML analysis report and open it in browser")
            .AddSwitch("--list-apps", opt_.listApps, "List installed apps on device and exit");
        BindDebugOptions(sub, g);
    }

    int Run(const GlobalOptions& g) override
    {
        ProfilerCliOptions profOpt;
        profOpt.processName = opt_.bundle;
        profOpt.duration = opt_.duration;
        profOpt.durationSpecified = opt_.durationSpecified;
        profOpt.outputDir = g.outDir;
        profOpt.category = opt_.category;
        profOpt.flagReport = g.autoOpen;
        profOpt.flagListApps = opt_.listApps;
        profOpt.sourcemapDir = opt_.sourcemapDir;
        profOpt.nameObfuscation = opt_.nameCacheDir;
        profOpt.soDir = opt_.soDir;
        // 仅在显式传了 --device 时才下发 key：区分“未传”（由门禁按设备数自动判定）
        // 与“显式传空串”（--device ""，属参数错误，须报错）。
        if (opt_.connectKeyProvided) {
            profOpt.connectKeyProvided = true;
            profOpt.connectKey = opt_.connectKey;
        }
        return RunProfilerCmd(profOpt);
    }
};

const bool REGISTERED = []() {
    CommandRegistry::Add(std::make_shared<ProfilerCommand>());
    return true;
}();

} // namespace
} // namespace mem_analyzer
