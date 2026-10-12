// =============================================================================
// RelativeTime.cpp — formatação de tempo relativo
// =============================================================================
#include "core/RelativeTime.hpp"

#include <format>

namespace copynator::core {

std::string formatRelativeTime(Timestamp moment, Timestamp now) {
    using namespace std::chrono;

    const auto elapsed = now - moment;

    if (elapsed < 1min) {
        return "Agora";
    }
    if (elapsed < 1h) {
        return std::format("há {} min", duration_cast<minutes>(elapsed).count());
    }
    if (elapsed < 24h) {
        return std::format("há {} h", duration_cast<hours>(elapsed).count());
    }
    if (elapsed < 48h) {
        return "ontem";
    }
    if (elapsed < days{7}) {
        return std::format("há {} dias", duration_cast<days>(elapsed).count());
    }

    // Itens antigos exibem a data no fuso horário local do usuário.
    const zoned_time localTime{current_zone(), floor<seconds>(moment)};
    return std::format("{:%d/%m/%Y}", localTime);
}

} // namespace copynator::core
