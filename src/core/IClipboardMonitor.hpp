// =============================================================================
// IClipboardMonitor.hpp
//
// "Porta" para observar a área de transferência do sistema. Cada ambiente
// (X11, Wayland, extensão do GNOME Shell) tem sua própria implementação na
// camada `platform`; o core só precisa saber que, quando um texto for
// copiado, o callback será chamado.
// =============================================================================
#pragma once

#include <functional>
#include <string>
#include <string_view>

namespace copynator::core {

class IClipboardMonitor {
public:
    // Chamado com o texto copiado (UTF-8).
    using TextCallback = std::function<void(const std::string& text)>;

    virtual ~IClipboardMonitor() = default;

    // Começa a observar. Chamadas repetidas substituem o callback.
    virtual void start(TextCallback onTextCopied) = 0;

    // Para de observar.
    virtual void stop() = 0;

    // Descrição curta do mecanismo, exibida em `copynator --status`.
    [[nodiscard]] virtual std::string_view description() const = 0;
};

} // namespace copynator::core
