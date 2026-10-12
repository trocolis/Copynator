// =============================================================================
// ExecutablePath.cpp — localização do executável
// =============================================================================
#include "platform/ExecutablePath.hpp"

#include <filesystem>
#include <glibmm/miscutils.h>

namespace copynator::platform {

std::string resolveExecutablePath() {
    if (auto inPath = Glib::find_program_in_path("copynator"); !inPath.empty()) {
        return inPath;
    }
    // /proc/self/exe é um link simbólico para o binário do processo atual.
    std::error_code error;
    const auto self = std::filesystem::read_symlink("/proc/self/exe", error);
    return error ? std::string("copynator") : self.string();
}

} // namespace copynator::platform
