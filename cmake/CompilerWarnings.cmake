# =============================================================================
# CompilerWarnings.cmake
#
# Define a função `copynator_set_warnings(<alvo>)`, que aplica um conjunto
# rigoroso de avisos de compilação. Manter os avisos centralizados garante que
# todos os módulos sigam o mesmo padrão de qualidade.
# =============================================================================
function(copynator_set_warnings target)
    target_compile_options(${target} PRIVATE
        -Wall               # Avisos comuns
        -Wextra             # Avisos extras
        -Wpedantic          # Conformidade estrita com o padrão
        -Wshadow            # Variáveis que escondem outras
        -Wconversion        # Conversões implícitas com perda
        -Wsign-conversion   # Conversões entre signed/unsigned
        -Wnon-virtual-dtor  # Classes polimórficas sem destrutor virtual
        -Wold-style-cast    # Casts no estilo C
        -Woverloaded-virtual
        -Wnull-dereference
    )
endfunction()
