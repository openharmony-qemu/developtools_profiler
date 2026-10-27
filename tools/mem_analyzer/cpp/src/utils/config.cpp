#include "config.hpp"
#include <unordered_map>

namespace mem_analyzer {

static const std::unordered_map<std::string, Scope> str_to_scope = {
    {"app-only", Scope::AppOnly},
    {"system-only", Scope::SystemOnly},
    {"app-and-system", Scope::AppAndSystem},
    {"mixed", Scope::Mixed},
};

static const std::unordered_map<Scope, std::string> scope_to_str = {
    {Scope::AppOnly, "app-only"},
    {Scope::SystemOnly, "system-only"},
    {Scope::AppAndSystem, "app-and-system"},
    {Scope::Mixed, "mixed"},
};

Scope scope_from_string(const std::string& s) {
    auto it = str_to_scope.find(s);
    if (it != str_to_scope.end()) return it->second;
    return Scope::AppAndSystem;
}

std::string scope_to_string(Scope s) {
    auto it = scope_to_str.find(s);
    if (it != scope_to_str.end()) return it->second;
    return "app-and-system";
}

} // namespace mem_analyzer
