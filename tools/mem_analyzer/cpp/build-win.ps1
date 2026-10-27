param(
    [string]$CmakePath = "",   # optional: specify cmake.exe explicitly
    [string]$GppPath = ""      # optional: specify g++.exe explicitly
)

<#
.SYNOPSIS
    mem-analyzer Windows release packaging script.
.DESCRIPTION
    1. Builds mem-analyzer.exe in Release mode
    2. Creates a portable directory with exe + third_party dependencies
    3. Packages into a ZIP archive

    Version is auto-detected from CMakeLists.txt.

    Usage: .\release-win.ps1
           .\release-win.ps1 -CmakePath "D:\cmake\bin\cmake.exe"
           .\release-win.ps1 -CmakePath "..." -GppPath "D:\mingw64\bin\g++.exe"
#>

$ErrorActionPreference = "Stop"
$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$ProjectRoot = Resolve-Path "$ScriptDir\.."
$BuildDir = "$ScriptDir\build"

# ── Helper: find exe via where.exe → Get-Command ──
function Find-ExeInPath([string]$name) {
    try {
        $result = where.exe $name 2>&1 4>$null
        if ($LASTEXITCODE -eq 0 -and $result) {
            # where.exe may return multiple matches; take the first
            return ($result | Select-Object -First 1).Trim()
        }
    } catch {
        # where.exe not available
    }
    $fromGC = (Get-Command $name -ErrorAction SilentlyContinue).Source
    if ($fromGC) { return $fromGC }
    return $null
}

# ── Environment checks ─────────────────────
Write-Host "----------------------------------------"
Write-Host " Checking build environment..."
Write-Host "----------------------------------------"

# ── cmake ───────────────────────────────────
$cmake = $null

# 1) User explicitly specified
if (-not [string]::IsNullOrEmpty($CmakePath)) {
    if (Test-Path $CmakePath) {
        $cmake = $CmakePath
    } else {
        Write-Host "  [ERROR] Specified cmake path not found: $CmakePath"
        Write-Host "----------------------------------------"; exit 1
    }
}

# 2) where.exe / PATH
if (-not $cmake) { $cmake = Find-ExeInPath "cmake.exe" }

if (-not $cmake) {
    Write-Host ""
    Write-Host "  [ERROR] cmake not found!"
    Write-Host "  Please install CMake 3.16+ from: https://cmake.org/download/"
    Write-Host "  Or add cmake.exe to your PATH and re-run."
    Write-Host "----------------------------------------"; exit 1
}

# ── C++ compiler ────────────────────────────
$compilerFound = $false
$compilerHint = ""

# 1) User explicitly specified g++
if (-not [string]::IsNullOrEmpty($GppPath)) {
    if (Test-Path $GppPath) {
        $compilerFound = $true; $compilerHint = $GppPath
    } else {
        Write-Host "  [ERROR] Specified g++ path not found: $GppPath"
        Write-Host "----------------------------------------"; exit 1
    }
}

# 2) where.exe / PATH (g++ then cl)
if (-not $compilerFound) {
    $gpp = Find-ExeInPath "g++.exe"
    if ($gpp) { $compilerFound = $true; $compilerHint = $gpp }
}
if (-not $compilerFound) {
    $cl = Find-ExeInPath "cl.exe"
    if ($cl) { $compilerFound = $true; $compilerHint = $cl }
}

if (-not $compilerFound) {
    Write-Host ""
    Write-Host "  [ERROR] C++ compiler not found!"
    Write-Host "  Install MinGW-w64: https://www.mingw-w64.org/"
    Write-Host "    Or via MSYS2: pacman -S mingw-w64-x86_64-gcc"
    Write-Host "    Or install Visual Studio 2019+ (for MSVC cl.exe)"
    Write-Host "----------------------------------------"; exit 1
}

Write-Host "  cmake:       $cmake"
Write-Host "  Compiler:    $compilerHint"
Write-Host "----------------------------------------"
Write-Host ""

# ── Version (auto-detect from version.cpp) ──
$versionCpp = Get-Content "$ScriptDir\src\utils\version.cpp" -Raw
$match = [regex]::Match($versionCpp, 'return\s+"([^"]+)"')
$Version = if ($match.Success) { $match.Groups[1].Value } else { "1.0.0" }

$Platform = "win64"
$ReleaseDir = "mem-analyzer-$Version-$Platform"
$ZipFile = "$ReleaseDir.zip"

Write-Host "========================================"
Write-Host " mem-analyzer Windows release build"
Write-Host "========================================"
Write-Host "  Version:   $Version"
Write-Host "  Platform:  $Platform"
Write-Host "  Output:    $ZipFile"
Write-Host ""

# ── Step 0: Download dependencies ────────────
Write-Host "[0/4] Downloading dependencies (setup_deps.py)..."
& python "$ProjectRoot\build\setup_deps.py" --mode cache
if ($LASTEXITCODE -ne 0) {
    Write-Host "  [ERROR] Dependency download failed. Aborting."
    Write-Host "----------------------------------------"; exit 1
}
Write-Host "  Done."
Write-Host ""

# ── Verify third-party source files ──────────
$sqlite3 = "$ProjectRoot\third_party\src\third_party_sqlite\src\sqlite3.c"
$jsonInc = "$ProjectRoot\third_party\src\third_party_json\single_include"

$missingSrc = @()
if (-not (Test-Path $sqlite3))  { $missingSrc += "sqlite3.c (expected at: $sqlite3)" }
if (-not (Test-Path $jsonInc))  { $missingSrc += "nlohmann/json (expected at: $jsonInc)" }

if ($missingSrc.Count -gt 0) {
    Write-Host ""
    Write-Host "  [ERROR] Missing third-party source dependencies after setup_deps:"
    foreach ($m in $missingSrc) { Write-Host "    - $m" }
    Write-Host "----------------------------------------"; exit 1
}

Write-Host "  Dependencies: all OK"
Write-Host ""

# ── Step 1: Clean build ────────────────────
Write-Host "[1/4] Cleaning previous build..."

if (Test-Path $BuildDir) {
    Remove-Item -Recurse -Force $BuildDir
    Write-Host "  Removed existing build directory."
}
New-Item -ItemType Directory -Path $BuildDir -Force | Out-Null

Write-Host "  Building mem-analyzer (Release)..."

# Determine CMake generator based on compiler, and linker flags for MinGW
$cmakeGenerator = if ($compilerHint -match 'g\+\+\.exe$') { "-G", "MinGW Makefiles" } else { @() }
$cmakeExtraFlags = if ($compilerHint -match 'g\+\+\.exe$') { @("-DCMAKE_EXE_LINKER_FLAGS=-static") } else { @() }

& $cmake -B $BuildDir -S $ScriptDir -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=$ReleaseDir $cmakeGenerator $cmakeExtraFlags
if ($LASTEXITCODE -ne 0) { throw "cmake configure failed" }

& $cmake --build $BuildDir --config Release --parallel 4
if ($LASTEXITCODE -ne 0) { throw "cmake build failed" }

Write-Host "  Done."
Write-Host ""

# ── Step 2: Create release directory ───────
Write-Host "[2/4] Creating release directory structure..."

if (Test-Path $ReleaseDir) {
    Remove-Item -Recurse -Force $ReleaseDir
}

$dirs = @(
    "$ReleaseDir",
    "$ReleaseDir\config\profiler",
    "$ReleaseDir\config\htrace",
    "$ReleaseDir\config\jsleak",
    "$ReleaseDir\third_party\bin\trace_streamer_binary\config",
    "$ReleaseDir\third_party\bin\rawheaptranslator\windows-x64",
    "$ReleaseDir\third_party\licenses\json",
    "$ReleaseDir\third_party\licenses\sqlite",
    "$ReleaseDir\third_party\licenses\zlib"
)

foreach ($d in $dirs) {
    New-Item -ItemType Directory -Path $d -Force | Out-Null
}

Write-Host "  Done."
Write-Host ""

# ── Step 3: Copy files ─────────────────────
Write-Host "[3/4] Copying files..."

$fileMappings = @(
    @{ src = "$BuildDir\mem-analyzer.exe";           dst = "$ReleaseDir\mem-analyzer.exe" },
    @{ src = "$ProjectRoot\third_party\bin\trace_streamer_binary\trace_streamer_windows.exe";
       dst = "$ReleaseDir\third_party\bin\trace_streamer_binary\trace_streamer_windows.exe" },
    @{ src = "$ProjectRoot\third_party\bin\trace_streamer_binary\config\config.json";
       dst = "$ReleaseDir\third_party\bin\trace_streamer_binary\config\config.json" },
    @{ src = "$ProjectRoot\third_party\bin\rawheaptranslator\windows-x64\rawheap_translator.exe";
       dst = "$ReleaseDir\third_party\bin\rawheaptranslator\windows-x64\rawheap_translator.exe" },
    # OSS 开源信息（json/sqlite/zlib；trace_streamer/rawheap 待上游许可证确认后补充）
    @{ src = "$ProjectRoot\third_party\src\third_party_json\README.OpenSource";
       dst = "$ReleaseDir\third_party\licenses\json\README.OpenSource" },
    @{ src = "$ProjectRoot\third_party\src\third_party_json\LICENSE.MIT";
       dst = "$ReleaseDir\third_party\licenses\json\LICENSE.MIT" },
    @{ src = "$ProjectRoot\third_party\src\third_party_sqlite\README.OpenSource";
       dst = "$ReleaseDir\third_party\licenses\sqlite\README.OpenSource" },
    @{ src = "$ProjectRoot\third_party\src\third_party_sqlite\LICENSE";
       dst = "$ReleaseDir\third_party\licenses\sqlite\LICENSE" },
    @{ src = "$ProjectRoot\third_party\src\third_party_zlib\README.OpenSource";
       dst = "$ReleaseDir\third_party\licenses\zlib\README.OpenSource" },
    @{ src = "$ProjectRoot\third_party\src\third_party_zlib\LICENSE";
       dst = "$ReleaseDir\third_party\licenses\zlib\LICENSE" }
)

$copyErrors = 0
foreach ($m in $fileMappings) {
    $src = $m.src
    $dst = $m.dst
    if (Test-Path $src) {
        Copy-Item -Path $src -Destination $dst -Force
        $relPath = $dst.Substring($ReleaseDir.Length + 1)
        Write-Host "  + $relPath"
    } else {
        Write-Warning "  MISSING: $src"
        $copyErrors++
    }
}

# Profiler config 目录（完整拷贝，包含 json 和所有 cmd 模板 txt）
$profilerConfigSrc = "$ScriptDir\config\profiler"
if (Test-Path $profilerConfigSrc) {
    Copy-Item -Path "$profilerConfigSrc\*" -Destination "$ReleaseDir\config\profiler\" -Recurse -Force
    Write-Host "  + config\profiler\ (profiler config + cmd templates)"
} else {
    Write-Warning "  MISSING: $profilerConfigSrc\ (profiler config directory not found)"
    $copyErrors++
}

# Htrace config 目录
$htraceConfigSrc = "$ScriptDir\config\htrace"
if (Test-Path $htraceConfigSrc) {
    Copy-Item -Path "$htraceConfigSrc\*" -Destination "$ReleaseDir\config\htrace\" -Recurse -Force
    Write-Host "  + config\htrace\ (htrace analysis config)"
}

# Jsleak config 目录
$jsleakConfigSrc = "$ScriptDir\config\jsleak"
if (Test-Path $jsleakConfigSrc) {
    Copy-Item -Path "$jsleakConfigSrc\*" -Destination "$ReleaseDir\config\jsleak\" -Recurse -Force
    Write-Host "  + config\jsleak\ (jsleak property tag config)"
}

# usage.md：打包时将 webp 引用替换为同名 png（本地包体积小）；
# 线上 git 中的 docs/usage.md 与 webp 原样保留，保持动画效果。
# 源文件为 UTF-8 无 BOM，必须显式按 UTF-8 读写，否则 PowerShell 5 会按 ANSI 读取导致中文乱码。
# $ReleaseDir 是相对路径，而 .NET File API 按 Environment.CurrentDirectory 解析（可能与 $PWD 不同步），
# 先解析成绝对路径，避免 WriteAllText 报“未能找到路径”。
$releaseDirFull = (Resolve-Path -Path $ReleaseDir).Path
$usageSrc = "$ProjectRoot\docs\usage.md"
if (Test-Path $usageSrc) {
    $usageContent = [System.IO.File]::ReadAllText($usageSrc, [System.Text.Encoding]::UTF8)
    $usageContent = $usageContent -replace '\.webp', '.png'
    [System.IO.File]::WriteAllText((Join-Path $releaseDirFull "usage.md"), $usageContent, (New-Object System.Text.UTF8Encoding -ArgumentList $false))
    Write-Host "  + usage.md (webp refs -> png)"
} else {
    Write-Warning "  MISSING: $usageSrc"
    $copyErrors++
}

# Docs images 目录：本地包只带 png（与替换后的 usage.md 引用一致），webp 不进包
$imagesSrc = "$ProjectRoot\docs\images"
if (Test-Path $imagesSrc) {
    $pngFiles = @(Get-ChildItem -Path $imagesSrc -Filter *.png -File)
    if ($pngFiles.Count -gt 0) {
        New-Item -ItemType Directory -Path "$ReleaseDir\images" -Force | Out-Null
        Copy-Item -Path $pngFiles.FullName -Destination "$ReleaseDir\images\" -Force
        Write-Host "  + images\ (*.png only, webp excluded)"
    } else {
        Write-Warning "  No .png in docs/images, falling back to copying all images"
        New-Item -ItemType Directory -Path "$ReleaseDir\images" -Force | Out-Null
        Copy-Item -Path "$imagesSrc\*" -Destination "$ReleaseDir\images\" -Recurse -Force
        $copyErrors++
    }
} else {
    Write-Warning "  MISSING: $imagesSrc\ (images directory not found)"
    $copyErrors++
}

if ($copyErrors -gt 0) {
    Write-Warning "  $copyErrors file(s) were missing, continuing..."
}

# Generate aggregated OSS NOTICE (from third-party README.OpenSource + LICENSE).
# Failure aborts packaging to guarantee complete OSS compliance info.
Write-Host "  Generating NOTICE..."
& python "$ProjectRoot\build\gen_notice.py" --dest "$ReleaseDir\NOTICE" `
    "$ProjectRoot\third_party\src\third_party_json" `
    "$ProjectRoot\third_party\src\third_party_sqlite" `
    "$ProjectRoot\third_party\src\third_party_zlib"
if ($LASTEXITCODE -ne 0) {
    throw "NOTICE generation failed, aborting packaging"
}

Write-Host "  Done."
Write-Host ""

# ── Step 4: Package ZIP ────────────────────
Write-Host "[4/4] Creating ZIP archive..."

# Remove old zip if exists
if (Test-Path $ZipFile) {
    Remove-Item $ZipFile -Force
}

Compress-Archive -Path "$ReleaseDir\*" -DestinationPath $ZipFile -Force
Write-Host "  Created: $(Resolve-Path $ZipFile)"
Write-Host ""

# 计算 SHA-256 校验和，写 <zip>.sha256（sha256sum 兼容格式：hash<空格><空格>文件名）
$hash = (Get-FileHash -Path $ZipFile -Algorithm SHA256).Hash.ToLower()
$zipFull = (Resolve-Path -Path $ZipFile).Path
$checksumFile = "$zipFull.sha256"
[System.IO.File]::WriteAllText($checksumFile, "$hash  $(Split-Path -Leaf $zipFull)`n", (New-Object System.Text.ASCIIEncoding))
Write-Host "  SHA256:   $hash"
Write-Host "  Checksum: $checksumFile"

# ── Verify ─────────────────────────────────
$zipInfo = Get-Item $ZipFile
Write-Host "========================================"
Write-Host " Release package created successfully!"
Write-Host "  ZIP: $ZipFile"
Write-Host "  Size: $([math]::Round($zipInfo.Length / 1KB, 1)) KB ($($zipInfo.Length) bytes)"
Write-Host "========================================"
