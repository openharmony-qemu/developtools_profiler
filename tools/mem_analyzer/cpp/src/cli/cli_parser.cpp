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
#include "cli/cli_parser.hpp"
#include "utils/logger.hpp"
#include "utils/terminal_style.hpp"
#include <cerrno>
#include <climits>
#include <cstdlib>

namespace mem_analyzer {

namespace {
constexpr size_t OPTION_PREFIX_LEN = 2;
constexpr size_t OPTION_VALUE_ARG_COUNT = 2;
constexpr size_t HELP_COLUMN_WIDTH = 32;
constexpr size_t HELP_WRAP_INDENT = 34;
constexpr size_t HELP_FLAG_PADDING = 20;
constexpr size_t NO_SUBCMD_PADDING = 15;
} // namespace

// ═══════════════════════════════════════════════════════════════
// Helpers
// ═══════════════════════════════════════════════════════════════

static std::string StripLeadingDashes(const std::string& s)
{
    if (s.size() >= OPTION_PREFIX_LEN && s[0] == '-' && s[1] == '-') {
        return s.substr(OPTION_PREFIX_LEN);
    }
    if (s.size() >= 1 && s[0] == '-') {
        return s.substr(1);
    }
    return s;
}

static std::string ExtractProgramName(const std::string& argv0)
{
    auto pos = argv0.find_last_of("/\\");
    return (pos == std::string::npos) ? argv0 : argv0.substr(pos + 1);
}

static int ParseInt(const std::string& s)
{
    char* end = nullptr;
    errno = 0;
    long val = std::strtol(s.c_str(), &end, 10);
    if (end == s.c_str() || *end != '\0') {
        throw CliError("Invalid integer value: " + s);
    }
    // 不校验范围会让 -n 3000000000 在 32 位 long 平台变 LONG_MAX、在 64 位平台
    // 被 static_cast 截断成负数后静默回落到默认值；显式报错更符合预期。
    if (errno == ERANGE || val < INT_MIN || val > INT_MAX) {
        throw CliError("Integer value out of range: " + s);
    }
    return static_cast<int>(val);
}

/// 解析逗号/空格分隔的选项名列表（如 "-d,--out-dir" 或 "--sourcemap"）：
/// 短名（-x）存入 short_name，长名（--xxx）存入 long_name；
/// 同名 token 重复出现时只保留第一个。实际注册最多一个短名加一个长名，
/// 因此无需额外的多别名（extra_names）与去重收集逻辑。
static void ParseOptionNames(const std::string& names, std::string& shortName, std::string& longName)
{
    size_t start = 0;
    while (start <= names.size()) {
        size_t end = names.find_first_of(", ", start);
        if (end == std::string::npos) {
            end = names.size();
        }
        const std::string tok = names.substr(start, end - start);
        if (tok.size() >= OPTION_PREFIX_LEN && tok[0] == '-' && tok[1] == '-') {
            if (longName.empty()) {
                longName = tok;
            }
        } else if (tok.size() >= 1 && tok[0] == '-') {
            if (shortName.empty()) {
                shortName = tok;
            }
        }
        if (end == names.size()) {
            break;
        }
        start = end + 1;
    }
}

// ═══════════════════════════════════════════════════════════════
// Constructor
// ═══════════════════════════════════════════════════════════════

CliParser::CliParser(const std::string& description)
    : description_(description) {}

// ═══════════════════════════════════════════════════════════════
// Option registration
// ═══════════════════════════════════════════════════════════════

CliParser& CliParser::AddOption(const std::string& names, std::string& target, const std::string& desc, bool* provided)
{
    Option opt;
    opt.shortName = "";
    opt.longName = "";
    opt.description = desc;
    opt.group = optionGroup_;
    opt.type = Option::STRING;
    opt.target = &target;
    opt.provided = provided;
    opt.defaultStr = target;

    // Parse comma/space-separated names
    // e.g., "-o,--output-path" → short="-o", long="--output-path"
    ParseOptionNames(names, opt.shortName, opt.longName);

    options_.push_back(std::move(opt));
    return *this;
}

CliParser& CliParser::AddSwitch(const std::string& names, bool& target, const std::string& desc)
{
    Option opt;
    opt.shortName = "";
    opt.longName = "";
    opt.description = desc;
    opt.group = optionGroup_;
    opt.type = Option::BOOL;
    opt.target = &target;
    opt.defaultStr = target ? "true" : "false";

    // Parse comma/space-separated names
    ParseOptionNames(names, opt.shortName, opt.longName);

    options_.push_back(std::move(opt));
    return *this;
}

CliParser& CliParser::AddOption(const std::string& names, int& target, const std::string& desc, bool* provided)
{
    Option opt;
    opt.shortName = "";
    opt.longName = "";
    opt.description = desc;
    opt.group = optionGroup_;
    opt.type = Option::INT;
    opt.target = &target;
    opt.provided = provided;
    opt.defaultStr = std::to_string(target);

    // Parse comma/space-separated names
    ParseOptionNames(names, opt.shortName, opt.longName);

    options_.push_back(std::move(opt));
    return *this;
}

CliParser& CliParser::SetOptionGroup(const std::string& group)
{
    optionGroup_ = group;
    return *this;
}

CliParser& CliParser::SetUsage(const std::string& args)
{
    usageArgs_ = args;
    return *this;
}

// ═══════════════════════════════════════════════════════════════
// Positionals
// ═══════════════════════════════════════════════════════════════

CliParser& CliParser::AddPositional(const std::string& name, std::vector<std::string>& target, const std::string& desc)
{
    Positional p;
    p.name = name;
    p.description = desc;
    p.isString = false;
    p.target = &target;
    positionals_.push_back(std::move(p));
    return *this;
}

CliParser& CliParser::AddPositional(const std::string& name, std::string& target, const std::string& desc)
{
    Positional p;
    p.name = name;
    p.description = desc;
    p.isString = true;
    p.target = &target;
    positionals_.push_back(std::move(p));
    return *this;
}

// ═══════════════════════════════════════════════════════════════
// Subcommands
// ═══════════════════════════════════════════════════════════════

CliParser& CliParser::AddSubcommand(const std::string& name, const std::string& desc,
    const std::string& alias, bool hidden)
{
    SubcommandEntry entry;
    entry.parser = std::make_unique<CliParser>(desc);
    entry.alias = alias;
    entry.hidden = hidden;
    auto& ref = *entry.parser;
    subcommands_.emplace_back(name, std::move(entry));
    return ref;
}

// ═══════════════════════════════════════════════════════════════
// Internal: find an option by name
// ═══════════════════════════════════════════════════════════════

CliParser::Option* CliParser::FindOptionByName(const std::string& name)
{
    for (auto& opt : options_) {
        if (opt.shortName == name || opt.longName == name) {
            return &opt;
        }
    }
    return nullptr;
}

// ═══════════════════════════════════════════════════════════════
// Internal: apply default values
// ═══════════════════════════════════════════════════════════════

void CliParser::ApplyDefaults()
{
    // Defaults are already set via initial values of the target variables.
    // Nothing to do here since we captured default_str at registration time
    // for documentation purposes only.
}

// ═══════════════════════════════════════════════════════════════
// Internal: consume remaining args (options + positionals)
// ═══════════════════════════════════════════════════════════════

void CliParser::ConsumeArgs(const std::vector<std::string>& args, size_t start)
{
    size_t i = start;
    size_t positionalIdx = 0;

    auto nextStringPos = [&]() -> std::string* {
        if (positionalIdx < positionals_.size() && positionals_[positionalIdx].isString) {
            return static_cast<std::string*>(positionals_[positionalIdx].target);
        }
        return nullptr;
    };

    auto nextVectorPos = [&]() -> std::vector<std::string>* {
        if (positionalIdx < positionals_.size() && !positionals_[positionalIdx].isString) {
            return static_cast<std::vector<std::string>*>(positionals_[positionalIdx].target);
        }
        return nullptr;
    };

    while (i < args.size()) {
        const std::string& arg = args[i];

        // --help / -h at any level
        if (arg == "--help" || arg == "-h" || arg == "-H") {
            throw CliError("", true);
        }

        // Option (starts with -)
        if (arg.size() > 1 && arg[0] == '-') {
            std::string optName = arg;
            // Check if this is a known option
            Option* opt = FindOptionByName(optName);
            if (!opt) {
                throw CliError("Unknown option: " + optName);
            }

            // 标记该选项被显式指定（如 -t），供上层区分「用户传入」与「默认值」。
            if (opt->provided) {
                *opt->provided = true;
            }

            if (opt->type == Option::BOOL) {
                *static_cast<bool*>(opt->target) = true;
                i++;
            } else {
                // STRING or INT: need a value argument
                if (i + 1 >= args.size()) {
                    throw CliError("Option " + optName + " requires a value");
                }
                const std::string& val = args[i + 1];
                if (opt->type == Option::STRING) {
                    *static_cast<std::string*>(opt->target) = val;
                } else {
                    *static_cast<int*>(opt->target) = ParseInt(val);
                }
                i += OPTION_VALUE_ARG_COUNT;
            }
            continue;
        }

        // Positional argument
        if (positionalIdx < positionals_.size()) {
            auto* strTarget = nextStringPos();
            if (strTarget) {
                *strTarget = arg;
                positionalIdx++;
                i++;
                continue;
            }
            auto* vecTarget = nextVectorPos();
            if (vecTarget) {
                // vector positionals consume remaining non-option args only
                while (i < args.size()) {
                    const std::string& a = args[i];
                    // Stop at option-like args (they'll be parsed on next iteration)
                    if (a.size() > 1 && a[0] == '-') {
                        break;
                    }
                    vecTarget->push_back(a);
                    i++;
                }
                positionalIdx++;
                continue;
            }
        }

        throw CliError("Unexpected argument: " + arg);
    }

    // Validate required positionals (vectors can be empty, strings must be non-empty)
    // A string positional that hasn't been filled = missing required arg
    // For vectors, empty is OK (they're treated as "not provided")
}

// ═══════════════════════════════════════════════════════════════
// parse()
// ═══════════════════════════════════════════════════════════════

void CliParser::Parse(int argc, char** argv)
{
    programName_ = (argc > 0) ? ExtractProgramName(argv[0]) : "mem-analyzer";

    // Collect args as strings (skip argv[0])
    std::vector<std::string> args;
    for (int i = 1; i < argc; i++) {
        args.push_back(argv[i]);
    }

    // Check for --help at top level
    if (!args.empty()) {
        if (args[0] == "--help" || args[0] == "-h") {
            throw CliError("", true);
        }
    }

    // If we have subcommands, check if the first non-option arg is a subcommand
    // Supports: bare name "trace" or alias "tr"
    if (!subcommands_.empty() && !args.empty() && args[0][0] != '-') {
        std::string subcmdName = ResolveSubcommand(args[0]);
        const SubcommandEntry* entry = FindSubcommand(subcmdName);
        if (entry) {
            matchedSubcommand_ = subcmdName;
            // Forward remaining args to subcommand's internal parse
            std::vector<std::string> subArgs(args.begin() + 1, args.end());
            entry->parser->programName_ = programName_ + " " + subcmdName;

            // Support --help on subcommand
            if (!subArgs.empty() && (subArgs[0] == "--help" || subArgs[0] == "-h")) {
                throw CliError("", true);
            }

            // Parse subcommand args
            entry->parser->ConsumeArgs(subArgs, 0);
            return;
        }
    }

    // No subcommand matched: parse as auto-detect / top-level
    ConsumeArgs(args, 0);
}

// ═══════════════════════════════════════════════════════════════
// is_subcommand()
// ═══════════════════════════════════════════════════════════════

bool CliParser::IsSubcommand(const std::string& name) const
{
    return matchedSubcommand_ == name;
}

// ═══════════════════════════════════════════════════════════════
// exit()
// ═══════════════════════════════════════════════════════════════

int CliParser::Exit(const std::exception& e) const
{
    const auto* ce = dynamic_cast<const CliError*>(&e);
    if (ce && ce->isHelp) {
        // For subcommand help, print subcommand help
        if (!matchedSubcommand_.empty()) {
            const SubcommandEntry* entry = FindSubcommand(matchedSubcommand_);
            if (entry) {
                entry->parser->PrintHelp(std::cout);
                return 0;
            }
        }
        PrintHelp(std::cout);
        return 0;
    }

    // Regular error
    log::Loge(e.what());
    log::Logw("Use '", programName_, " --help' for usage.");
    return 1;
}

// ═══════════════════════════════════════════════════════════════
// print_help()
// ═══════════════════════════════════════════════════════════════

void CliParser::PrintHelp(std::ostream& os) const
{
    os << description_;
    if (!version_.empty()) {
        os << "  version " << version_;
    }
    os << "\n\n";

    // Usage line
    os << "USAGE:\n";
    if (!usageArgs_.empty()) {
        os << "  " << programName_ << " " << usageArgs_ << "\n";
    } else {
        // Auto-detect form (no subcommand)
        os << "  " << programName_ << " [options]";
        if (!positionals_.empty()) {
            for (const auto& p : positionals_)
                os << " <" << p.name << ">";
        }
        os << "\n";
        // Subcommand forms
        bool hasVisibleSub = false;
        for (const auto& [name, entry] : subcommands_) {
            if (!entry.hidden) {
                hasVisibleSub = true;
                break;
            }
        }
        if (hasVisibleSub) {
            for (const auto& [name, entry] : subcommands_) {
                if (entry.hidden) { // hidden subcommands are omitted from help
                    continue;
                }
                os << "  " << programName_ << " " << name;
                // Use subcommand's own usage_args if set, else generate from positionals
                const auto& subUsage = entry.parser->GetUsageArgs();
                if (!subUsage.empty()) {
                    os << " " << subUsage << "\n";
                } else {
                    os << " [options]";
                    for (const auto& p : entry.parser->positionals_)
                        os << " <" << p.name << ">";
                    os << "\n";
                }
            }
        }
    }
    os << "\n";

    // Options, grouped by their `group` tag (preserving registration order;
    // empty group renders under the default "OPTIONS" section).
    if (!options_.empty()) {
        std::vector<std::pair<std::string, std::vector<const Option*>>> groups;
        for (const auto& opt : options_) {
            const std::string& g = opt.group.empty() ? "OPTIONS" : opt.group;
            if (groups.empty() || groups.back().first != g) {
                groups.emplace_back(g, std::vector<const Option*>{});
            }
            groups.back().second.push_back(&opt);
        }
        for (const auto& [title, opts] : groups) {
            os << title << ":\n";
            for (const Option* opt : opts) {
                // Compute display name
                std::string display;
                if (!opt->shortName.empty()) {
                    display = opt->shortName;
                }
                if (!opt->longName.empty()) {
                    if (!display.empty()) {
                        display += ", ";
                    }
                    display += opt->longName;
                }

                // Type indicator
                std::string typeStr;
                switch (opt->type) {
                    case Option::STRING:
                        typeStr = " TEXT";
                        break;
                    case Option::INT:
                        typeStr = " INT";
                        break;
                    case Option::BOOL:
                        typeStr = "";
                        break;
                }

                std::string optLine = "  " + display + typeStr;
                // Pad to 32 chars
                if (optLine.size() < HELP_COLUMN_WIDTH) {
                    optLine.append(HELP_COLUMN_WIDTH - optLine.size(), ' ');
                } else {
                    optLine += "\n" + std::string(HELP_WRAP_INDENT, ' ');
                }

                os << optLine;

                // Description + default
                if (!opt->description.empty()) {
                    os << opt->description;
                }
                if (!opt->defaultStr.empty() && opt->type != Option::BOOL) {
                    os << " (default: " << opt->defaultStr << ")";
                }
                os << "\n";
            }
        }
        os << "  -h, --help" << std::string(HELP_FLAG_PADDING, ' ') << "Print this help message\n";
        os << "\n";
    }

    // Positionals
    if (!positionals_.empty()) {
        os << "ARGUMENTS:\n";
        for (const auto& p : positionals_) {
            std::string line = "  " + p.name;
            if (line.size() < HELP_COLUMN_WIDTH) {
                line.append(HELP_COLUMN_WIDTH - line.size(), ' ');
            } else {
                line += "\n" + std::string(HELP_WRAP_INDENT, ' ');
            }
            line += p.description;
            os << line << "\n";
        }
        os << "\n";
    }

    // Subcommands
    bool hasVisibleSub = false;
    for (const auto& [name, entry] : subcommands_) {
        if (!entry.hidden) {
            hasVisibleSub = true;
            break;
        }
    }
    if (hasVisibleSub) {
        os << "SUBCOMMANDS:\n";
        os << "  (no subcommand)" << std::string(NO_SUBCMD_PADDING, ' ') << "Omit subcommand to auto-detect file type\n";
        for (const auto& [name, entry] : subcommands_) {
            if (entry.hidden) { // hidden subcommands are omitted from help
                continue;
            }
            std::string display = "  " + name;
            if (!entry.alias.empty()) {
                display += ", " + entry.alias;
            }
            if (display.size() < HELP_COLUMN_WIDTH) {
                display.append(HELP_COLUMN_WIDTH - display.size(), ' ');
            }
            os << display << entry.parser->description_ << "\n";
        }
        os << "\n";
        os << "Use \"" << programName_ << " <command> --help\" for more information about a specific command.\n";
    }
}

// ═══════════════════════════════════════════════════════════════
// resolve_subcommand()
// ═══════════════════════════════════════════════════════════════

std::string CliParser::ResolveSubcommand(const std::string& name) const
{
    // Direct match?
    if (FindSubcommand(name)) {
        return name;
    }
    // Check aliases
    for (const auto& [key, entry] : subcommands_) {
        if (entry.alias == name) {
            return key;
        }
    }
    return name;  // not found, return as-is
}

const CliParser::SubcommandEntry* CliParser::FindSubcommand(const std::string& name) const
{
    for (const auto& [key, entry] : subcommands_) {
        if (key == name) {
            return &entry;
        }
    }
    return nullptr;
}

} // namespace mem_analyzer
