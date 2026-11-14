// Standalone, non-Qt sanity check for the ArchTitan OS static registry API.
// Full runtime registration is daemon-owned and is NOT inspected here.
#include "protected_registry.hpp"
#include <iostream>
#include <string>

int main() {
    if (!thm::protected_names().contains("titan-task-manager")) return 1;
    if (!thm::protected_names().contains("titan-hwm")) return 2;
    if (!thm::browser_names().contains("firefox")) return 3;
    if (thm::protected_exe_prefixes().empty()) return 4;
    if (thm::browser_exe_prefixes().empty()) return 5;
    std::cout << "Canonical THM static registry names and prefixes are present.\n";
    return 0;
}
