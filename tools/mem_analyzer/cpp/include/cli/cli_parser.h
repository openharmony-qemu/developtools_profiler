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
#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <sstream>
#include <functional>

namespace mem_analyzer {

/**
 * Exception thrown by CliParser on parse errors or --help request.
 */
class CliError : public std::runtime_error {
public:
    bool isHelp;
    explicit CliError(const std::string& msg, bool help = false) : std::runtime_error(msg), isHelp(help) {}
};

/**
 * Simple command-line argument parser (replacement for CLI11).
 *
 * Supports:
 *   - String options:   -o <val>, --output-path <val>
 *   - Integer options:  -t <num>, --top-n <num>
 *   - Switch (bool):   -p, --parse-only
 *   - Positional args:  single or vector<string>
 *   - Subcommands:      trace, tshandle, jsleak
 *   - Subcommand aliases:  tr, th, lc
 *   - --help (auto-generated usage text)
 *
 * All registration methods (add_option, add_switch, add_positional,
 * add_subcommand) return *this for fluent chaining.
 *
 * Subcommands share variable references with the parent parser
 * (the same add_option calls are repeated on the subcommand).
 */
class CliParser {
public:
    explicit CliParser(const std::string& description);

    // ── Option registration (return *this for chaining) ──────
    /**
     * `provided`（可选）：命中该选项时被置为 true，用于区分「用户显式传入」与
     * 「使用默认值」，无需在解析后再反查 parser。
     */
    CliParser& AddOption(const std::string& names, std::string& target, const std::string& desc,
        bool* provided = nullptr);
    CliParser& AddSwitch(const std::string& names, bool& target, const std::string& desc);
    CliParser& AddOption(const std::string& names, int& target, const std::string& desc, bool* provided = nullptr);

    /**
     * Group options registered afterwards under `group` for help output.
     * An empty group restores the default "OPTIONS" section. Options are
     * rendered in registration order within each group; groups appear in
     * the order their first option was registered.
     */
    CliParser& SetOptionGroup(const std::string& group);

    /**
     * Override the argument part of the USAGE line (e.g.
     * "-p <process_name> <capture_type> [options]"). Rendered as
     * "<program> <args>". Empty (default) keeps the auto-generated
     * "[subcommand] [options] <files>" form.
     */
    CliParser& SetUsage(const std::string& args);

    // ── Positional args ──────────────────────────────────────
    CliParser& AddPositional(const std::string& name, std::vector<std::string>& target, const std::string& desc);
    CliParser& AddPositional(const std::string& name, std::string& target, const std::string& desc);

    // ── Subcommands ──────────────────────────────────────────
    /**
     * Add subcommand + alias, return sub-parser reference.
     * `hidden=true` 时该子命令仍可被解析与分发，但不出现在 --help 的
     * USAGE / SUBCOMMANDS 中。
     */
    CliParser& AddSubcommand(const std::string& name, const std::string& desc,
        const std::string& alias = "", bool hidden = false);
    /**
     * Add subcommand with setup callback, return parent for chaining
     */
    template<typename Fn>
    CliParser& AddSubcommand(const std::string& name, const std::string& desc,
        const std::string& alias, bool hidden, Fn setup)
    {
        auto& sub = AddSubcommand(name, desc, alias, hidden);
        setup(sub);
        return *this;
    }

    // ── Parse ────────────────────────────────────────────────
    void Parse(int argc, char** argv);

    // ── Query ────────────────────────────────────────────────
    bool IsSubcommand(const std::string& name) const;
    /**
     * 本次解析命中的子命令主名（别名已归一化为主名）；未命中子命令时为空串。
     */
    const std::string& MatchedSubcommand() const { return matchedSubcommand_; }

    // ── Error/help handling (mimics CLI::App::exit) ──────────
    int Exit(const std::exception& e) const;

    // ── Print help ───────────────────────────────────────────
    void PrintHelp(std::ostream& os = std::cout) const;

    // ── Usage args accessor ───────────────────────────────────
    const std::string& GetUsageArgs() const { return usageArgs_; }

    // ── Version ──────────────────────────────────────────────
    void SetVersion(const std::string& ver)
    {
        version_ = ver;
    }

private:
    struct Option {
        std::string shortName;   // "-o"（可为空：纯长名选项如 "--sourcemap"）
        std::string longName;    // "--output-path"
        std::string description;
        std::string defaultStr;
        std::string group;        // help section; empty => "OPTIONS"
        enum Type { STRING, INT, BOOL };
        Type type;
        void* target;             // pointer to bound variable
        bool* provided = nullptr; // 可选：命中该选项时置 true（见 add_option）
    };

    struct Positional {
        std::string name;
        std::string description;
        bool isString;           // true: targets std::string; false: targets std::vector<std::string>
        void* target;
    };

    struct SubcommandEntry {
        std::unique_ptr<CliParser> parser;
        std::string alias;
        bool hidden = false;      // hidden subcommands are omitted from --help
    };

    std::string description_;
    std::vector<Option> options_;
    std::vector<Positional> positionals_;
    std::string optionGroup_;  // group tag for options registered next
    std::string usageArgs_;    // custom USAGE argument part; empty => auto
    std::vector<std::pair<std::string, SubcommandEntry>> subcommands_;
    std::string programName_;
    std::string matchedSubcommand_;  // set after parse
    std::string version_;

    // ── Internal helpers ─────────────────────────────────────
    Option* FindOptionByName(const std::string& name);
    void ConsumeArgs(const std::vector<std::string>& args, size_t start);
    void ConsumeOption(const std::vector<std::string>& args, size_t& i);
    void ConsumeVectorArgs(const std::vector<std::string>& args, size_t& i,
        std::vector<std::string>* vecTarget);
    void PrintUsageSection(std::ostream& os) const;
    void PrintOptionsSection(std::ostream& os) const;
    void PrintSubcommandsSection(std::ostream& os) const;
    void ApplyDefaults();
    /**
     * Resolve alias → primary subcommand name, or return name unchanged
     */
    std::string ResolveSubcommand(const std::string& name) const;
    /**
     * Find subcommand entry by name or alias, returns nullptr if not found
     */
    const SubcommandEntry* FindSubcommand(const std::string& name) const;
};

} // namespace mem_analyzer
