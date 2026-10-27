#include "cli/cli_parser.hpp"
#include "utils/logger.hpp"
#include "utils/terminal_style.hpp"
#include <cerrno>
#include <climits>
#include <cstdlib>

namespace mem_analyzer {

// ═══════════════════════════════════════════════════════════════
// Helpers
// ═══════════════════════════════════════════════════════════════

static std::string strip_leading_dashes(const std::string& s) {
    if (s.size() >= 2 && s[0] == '-' && s[1] == '-') return s.substr(2);
    if (s.size() >= 1 && s[0] == '-') return s.substr(1);
    return s;
}

static std::string extract_program_name(const std::string& argv0) {
    auto pos = argv0.find_last_of("/\\");
    return (pos == std::string::npos) ? argv0 : argv0.substr(pos + 1);
}

static int parse_int(const std::string& s) {
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
static void parse_option_names(const std::string& names, std::string& short_name,
                               std::string& long_name) {
    size_t start = 0;
    while (start <= names.size()) {
        size_t end = names.find_first_of(", ", start);
        if (end == std::string::npos) end = names.size();
        const std::string tok = names.substr(start, end - start);
        if (tok.size() >= 2 && tok[0] == '-' && tok[1] == '-') {
            if (long_name.empty()) long_name = tok;
        } else if (tok.size() >= 1 && tok[0] == '-') {
            if (short_name.empty()) short_name = tok;
        }
        if (end == names.size()) break;
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

CliParser& CliParser::add_option(const std::string& names, std::string& target, const std::string& desc,
                                 bool* provided) {
    Option opt;
    opt.short_name = "";
    opt.long_name = "";
    opt.description = desc;
    opt.group = option_group_;
    opt.type = Option::STRING;
    opt.target = &target;
    opt.provided = provided;
    opt.default_str = target;

    // Parse comma/space-separated names
    // e.g., "-o,--output-path" → short="-o", long="--output-path"
    parse_option_names(names, opt.short_name, opt.long_name);

    options_.push_back(std::move(opt));
    return *this;
}

CliParser& CliParser::add_switch(const std::string& names, bool& target, const std::string& desc) {
    Option opt;
    opt.short_name = "";
    opt.long_name = "";
    opt.description = desc;
    opt.group = option_group_;
    opt.type = Option::BOOL;
    opt.target = &target;
    opt.default_str = target ? "true" : "false";

    // Parse comma/space-separated names
    parse_option_names(names, opt.short_name, opt.long_name);

    options_.push_back(std::move(opt));
    return *this;
}

CliParser& CliParser::add_option(const std::string& names, int& target, const std::string& desc,
                                 bool* provided) {
    Option opt;
    opt.short_name = "";
    opt.long_name = "";
    opt.description = desc;
    opt.group = option_group_;
    opt.type = Option::INT;
    opt.target = &target;
    opt.provided = provided;
    opt.default_str = std::to_string(target);

    // Parse comma/space-separated names
    parse_option_names(names, opt.short_name, opt.long_name);

    options_.push_back(std::move(opt));
    return *this;
}

CliParser& CliParser::set_option_group(const std::string& group) {
    option_group_ = group;
    return *this;
}

CliParser& CliParser::set_usage(const std::string& args) {
    usage_args_ = args;
    return *this;
}

// ═══════════════════════════════════════════════════════════════
// Positionals
// ═══════════════════════════════════════════════════════════════

CliParser& CliParser::add_positional(const std::string& name, std::vector<std::string>& target, const std::string& desc) {
    Positional p;
    p.name = name;
    p.description = desc;
    p.is_string = false;
    p.target = &target;
    positionals_.push_back(std::move(p));
    return *this;
}

CliParser& CliParser::add_positional(const std::string& name, std::string& target, const std::string& desc) {
    Positional p;
    p.name = name;
    p.description = desc;
    p.is_string = true;
    p.target = &target;
    positionals_.push_back(std::move(p));
    return *this;
}

// ═══════════════════════════════════════════════════════════════
// Subcommands
// ═══════════════════════════════════════════════════════════════

CliParser& CliParser::add_subcommand(const std::string& name, const std::string& desc,
                                     const std::string& alias, bool hidden) {
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

CliParser::Option* CliParser::find_option_by_name(const std::string& name) {
    for (auto& opt : options_) {
        if (opt.short_name == name || opt.long_name == name) return &opt;
    }
    return nullptr;
}

// ═══════════════════════════════════════════════════════════════
// Internal: apply default values
// ═══════════════════════════════════════════════════════════════

void CliParser::apply_defaults() {
    // Defaults are already set via initial values of the target variables.
    // Nothing to do here since we captured default_str at registration time
    // for documentation purposes only.
}

// ═══════════════════════════════════════════════════════════════
// Internal: consume remaining args (options + positionals)
// ═══════════════════════════════════════════════════════════════

void CliParser::consume_args(const std::vector<std::string>& args, size_t start) {
    size_t i = start;
    size_t positional_idx = 0;

    auto next_string_pos = [&]() -> std::string* {
        if (positional_idx < positionals_.size() && positionals_[positional_idx].is_string) {
            return static_cast<std::string*>(positionals_[positional_idx].target);
        }
        return nullptr;
    };

    auto next_vector_pos = [&]() -> std::vector<std::string>* {
        if (positional_idx < positionals_.size() && !positionals_[positional_idx].is_string) {
            return static_cast<std::vector<std::string>*>(positionals_[positional_idx].target);
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
            std::string opt_name = arg;
            // Check if this is a known option
            Option* opt = find_option_by_name(opt_name);
            if (!opt) {
                throw CliError("Unknown option: " + opt_name);
            }

            // 标记该选项被显式指定（如 -t），供上层区分「用户传入」与「默认值」。
            if (opt->provided) *opt->provided = true;

            if (opt->type == Option::BOOL) {
                *static_cast<bool*>(opt->target) = true;
                i++;
            } else {
                // STRING or INT: need a value argument
                if (i + 1 >= args.size()) {
                    throw CliError("Option " + opt_name + " requires a value");
                }
                const std::string& val = args[i + 1];
                if (opt->type == Option::STRING) {
                    *static_cast<std::string*>(opt->target) = val;
                } else {
                    *static_cast<int*>(opt->target) = parse_int(val);
                }
                i += 2;
            }
            continue;
        }

        // Positional argument
        if (positional_idx < positionals_.size()) {
            auto* str_target = next_string_pos();
            if (str_target) {
                *str_target = arg;
                positional_idx++;
                i++;
                continue;
            }
            auto* vec_target = next_vector_pos();
            if (vec_target) {
                // vector positionals consume remaining non-option args only
                while (i < args.size()) {
                    const std::string& a = args[i];
                    // Stop at option-like args (they'll be parsed on next iteration)
                    if (a.size() > 1 && a[0] == '-') break;
                    vec_target->push_back(a);
                    i++;
                }
                positional_idx++;
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

void CliParser::parse(int argc, char** argv) {
    program_name_ = (argc > 0) ? extract_program_name(argv[0]) : "mem-analyzer";

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
        std::string subcmd_name = resolve_subcommand(args[0]);
        const SubcommandEntry* entry = find_subcommand(subcmd_name);
        if (entry) {
            matched_subcommand_ = subcmd_name;
            // Forward remaining args to subcommand's internal parse
            std::vector<std::string> sub_args(args.begin() + 1, args.end());
            entry->parser->program_name_ = program_name_ + " " + subcmd_name;

            // Support --help on subcommand
            if (!sub_args.empty() && (sub_args[0] == "--help" || sub_args[0] == "-h")) {
                throw CliError("", true);
            }

            // Parse subcommand args
            entry->parser->consume_args(sub_args, 0);
            return;
        }
    }

    // No subcommand matched: parse as auto-detect / top-level
    consume_args(args, 0);
}

// ═══════════════════════════════════════════════════════════════
// is_subcommand()
// ═══════════════════════════════════════════════════════════════

bool CliParser::is_subcommand(const std::string& name) const {
    return matched_subcommand_ == name;
}

// ═══════════════════════════════════════════════════════════════
// exit()
// ═══════════════════════════════════════════════════════════════

int CliParser::exit(const std::exception& e) const {
    const auto* ce = dynamic_cast<const CliError*>(&e);
    if (ce && ce->is_help) {
        // For subcommand help, print subcommand help
        if (!matched_subcommand_.empty()) {
            const SubcommandEntry* entry = find_subcommand(matched_subcommand_);
            if (entry) {
                entry->parser->print_help(std::cout);
                return 0;
            }
        }
        print_help(std::cout);
        return 0;
    }

    // Regular error
    log::loge(e.what());
    log::logw("Use '", program_name_, " --help' for usage.");
    return 1;
}

// ═══════════════════════════════════════════════════════════════
// print_help()
// ═══════════════════════════════════════════════════════════════

void CliParser::print_help(std::ostream& os) const {
    os << description_;
    if (!version_.empty()) os << "  version " << version_;
    os << "\n\n";

    // Usage line
    os << "USAGE:\n";
    if (!usage_args_.empty()) {
        os << "  " << program_name_ << " " << usage_args_ << "\n";
    } else {
        // Auto-detect form (no subcommand)
        os << "  " << program_name_ << " [options]";
        if (!positionals_.empty()) {
            for (const auto& p : positionals_)
                os << " <" << p.name << ">";
        }
        os << "\n";
        // Subcommand forms
        bool has_visible_sub = false;
        for (const auto& [name, entry] : subcommands_) {
            if (!entry.hidden) { has_visible_sub = true; break; }
        }
        if (has_visible_sub) {
            for (const auto& [name, entry] : subcommands_) {
                if (entry.hidden) continue;  // hidden subcommands are omitted from help
                os << "  " << program_name_ << " " << name;
                // Use subcommand's own usage_args if set, else generate from positionals
                const auto& sub_usage = entry.parser->get_usage_args();
                if (!sub_usage.empty()) {
                    os << " " << sub_usage << "\n";
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
                if (!opt->short_name.empty()) display = opt->short_name;
                if (!opt->long_name.empty()) {
                    if (!display.empty()) display += ", ";
                    display += opt->long_name;
                }

                // Type indicator
                std::string type_str;
                switch (opt->type) {
                    case Option::STRING: type_str = " TEXT"; break;
                    case Option::INT:    type_str = " INT";  break;
                    case Option::BOOL:   type_str = "";      break;
                }

                std::string opt_line = "  " + display + type_str;
                // Pad to 32 chars
                if (opt_line.size() < 32) opt_line.append(32 - opt_line.size(), ' ');
                else opt_line += "\n" + std::string(34, ' ');

                os << opt_line;

                // Description + default
                if (!opt->description.empty()) os << opt->description;
                if (!opt->default_str.empty() && opt->type != Option::BOOL) {
                    os << " (default: " << opt->default_str << ")";
                }
                os << "\n";
            }
        }
        os << "  -h, --help" << std::string(20, ' ') << "Print this help message\n";
        os << "\n";
    }

    // Positionals
    if (!positionals_.empty()) {
        os << "ARGUMENTS:\n";
        for (const auto& p : positionals_) {
            std::string line = "  " + p.name;
            if (line.size() < 32) line.append(32 - line.size(), ' ');
            else line += "\n" + std::string(34, ' ');
            line += p.description;
            os << line << "\n";
        }
        os << "\n";
    }

    // Subcommands
    bool has_visible_sub = false;
    for (const auto& [name, entry] : subcommands_) {
        if (!entry.hidden) { has_visible_sub = true; break; }
    }
    if (has_visible_sub) {
        os << "SUBCOMMANDS:\n";
        os << "  (no subcommand)" << std::string(15, ' ') << "Omit subcommand to auto-detect file type\n";
        for (const auto& [name, entry] : subcommands_) {
            if (entry.hidden) continue;  // hidden subcommands are omitted from help
            std::string display = "  " + name;
            if (!entry.alias.empty()) display += ", " + entry.alias;
            if (display.size() < 32) display.append(32 - display.size(), ' ');
            os << display << entry.parser->description_ << "\n";
        }
        os << "\n";
        os << "Use \"" << program_name_ << " <command> --help\" for more information about a specific command.\n";
    }
}

// ═══════════════════════════════════════════════════════════════
// resolve_subcommand()
// ═══════════════════════════════════════════════════════════════

std::string CliParser::resolve_subcommand(const std::string& name) const {
    // Direct match?
    if (find_subcommand(name)) return name;
    // Check aliases
    for (const auto& [key, entry] : subcommands_) {
        if (entry.alias == name) return key;
    }
    return name;  // not found, return as-is
}

const CliParser::SubcommandEntry* CliParser::find_subcommand(const std::string& name) const {
    for (const auto& [key, entry] : subcommands_) {
        if (key == name) return &entry;
    }
    return nullptr;
}

} // namespace mem_analyzer
