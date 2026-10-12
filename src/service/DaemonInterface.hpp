// =============================================================================
// DaemonInterface.hpp
//
// Contrato do daemon do Copynator:
//   - `IDaemonController`: operações que o daemon oferece, implementadas pela
//     aplicação. A camada de serviço (D-Bus) depende apenas desta interface.
//   - Constantes e XML de introspecção da interface D-Bus, compartilhados pelo
//     servidor (daemon) e pelo cliente (linha de comando).
// =============================================================================
#pragma once

#include "core/Placement.hpp"

#include <cstddef>
#include <string>
#include <string_view>

namespace copynator::service {

// Estado do daemon informado por GetStatus.
struct DaemonStatus {
    std::string version;
    std::size_t itemCount = 0;
    bool windowVisible = false;
    std::string monitorDescription; // mecanismo de monitoramento em uso
};

// Operações expostas pelo daemon (implementadas pela camada app).
class IDaemonController {
public:
    virtual ~IDaemonController() = default;

    virtual void show(core::Placement placement) = 0;
    virtual void toggle(core::Placement placement) = 0;
    virtual void hide() = 0;
    virtual void addEntry(std::string_view content) = 0;
    virtual std::size_t clearHistory() = 0;
    [[nodiscard]] virtual DaemonStatus status() const = 0;
};

// -----------------------------------------------------------------------------
// Identificadores no barramento de sessão
// -----------------------------------------------------------------------------

// Nome do barramento (é o mesmo ID da aplicação GTK, que garante unicidade).
inline constexpr std::string_view kBusName = "io.github.copynator";

// Caminho do objeto. Usamos um subcaminho para não conflitar com as
// interfaces que o próprio GApplication registra em /io/github/copynator.
inline constexpr std::string_view kObjectPath = "/io/github/copynator/Daemon";

inline constexpr std::string_view kInterfaceName = "io.github.copynator.Daemon";

// XML de introspecção: descreve métodos e sinais para o D-Bus.
inline constexpr std::string_view kIntrospectionXml = R"xml(
<node>
  <interface name="io.github.copynator.Daemon">
    <!-- Mostra a janela. placement: "cursor" ou "center" -->
    <method name="Show">
      <arg name="placement" type="s" direction="in"/>
    </method>
    <!-- Mostra a janela se estiver escondida; caso contrário, esconde -->
    <method name="Toggle">
      <arg name="placement" type="s" direction="in"/>
    </method>
    <method name="Hide"/>
    <!-- Registra um texto copiado (usado pela extensão do GNOME Shell) -->
    <method name="AddEntry">
      <arg name="content" type="s" direction="in"/>
    </method>
    <!-- Limpa o histórico não fixado e devolve quantos itens saíram -->
    <method name="Clear">
      <arg name="removed" type="u" direction="out"/>
    </method>
    <method name="GetStatus">
      <arg name="version" type="s" direction="out"/>
      <arg name="item_count" type="u" direction="out"/>
      <arg name="window_visible" type="b" direction="out"/>
      <arg name="monitor" type="s" direction="out"/>
    </method>
    <!-- Emitido quando a janela é mostrada, para que a extensão a posicione -->
    <signal name="WindowShown">
      <arg name="placement" type="s"/>
    </signal>
  </interface>
</node>
)xml";

} // namespace copynator::service
