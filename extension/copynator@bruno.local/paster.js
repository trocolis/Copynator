// =============================================================================
// paster.js
//
// Cola o item escolhido na janela que estava em foco antes do Copynator,
// como o Win+V do Windows 11.
//
// No Wayland, aplicativos não podem simular teclas; o GNOME Shell pode, por
// meio de um teclado virtual do Clutter. O fluxo é:
//   1. o daemon copia o item, esconde a janela e emite PasteRequested;
//   2. aguardamos o foco sair da janela do Copynator (o Mutter devolve o foco
//      à janela anterior);
//   3. enviamos Ctrl+V (ou Ctrl+Shift+V em terminais).
// =============================================================================
import Clutter from 'gi://Clutter';
import GLib from 'gi://GLib';

// Intervalo entre verificações do foco e tempo máximo de espera.
const FOCUS_POLL_MS = 20;
const FOCUS_TIMEOUT_MS = 600;

// Pequena pausa após a troca de foco, para a janela de destino estar pronta
// para receber teclas.
const SETTLE_DELAY_MS = 60;

// Terminais usam Ctrl+Shift+V para colar (Ctrl+V tem outro significado).
// Comparação por trecho do WM_CLASS / app-id, sem diferenciar maiúsculas.
const TERMINAL_HINTS = [
    'terminal', 'console', 'kgx', 'ptyxis', 'ghostty', 'kitty', 'alacritty',
    'wezterm', 'foot', 'tilix', 'konsole', 'xterm', 'urxvt', 'terminator', 'blackbox',
];

export class Paster {
    /** @param {string} appId ID da aplicação do Copynator */
    constructor(appId) {
        this._appId = appId;
        this._timeoutIds = new Set();

        // GNOME 46+ expõe o backend pelo contexto do palco; versões mais
        // antigas usavam Clutter.get_default_backend() (removido no 51).
        const backend = global.stage.context?.get_backend?.() ?? Clutter.get_default_backend();
        const seat = backend.get_default_seat();
        this._keyboard = seat.create_virtual_device(Clutter.InputDeviceType.KEYBOARD_DEVICE);
    }

    /** Cola na janela anterior assim que o foco voltar para ela. */
    requestPaste() {
        let waitedMs = 0;
        this._schedule(FOCUS_POLL_MS, () => {
            const target = global.display.get_focus_window();
            if (target && !this._isOurs(target)) {
                this._schedule(SETTLE_DELAY_MS, () => {
                    this._sendPaste(target);
                    return GLib.SOURCE_REMOVE;
                });
                return GLib.SOURCE_REMOVE;
            }
            waitedMs += FOCUS_POLL_MS;
            // Sem outra janela para receber o foco (ex.: área de trabalho
            // vazia), desistimos: o item continua na área de transferência.
            return waitedMs < FOCUS_TIMEOUT_MS ? GLib.SOURCE_CONTINUE : GLib.SOURCE_REMOVE;
        });
    }

    destroy() {
        this._timeoutIds.forEach(id => GLib.source_remove(id));
        this._timeoutIds.clear();
        this._keyboard?.run_dispose();
        this._keyboard = null;
    }

    // -------------------------------------------------------------------------

    _isOurs(window) {
        return window.get_gtk_application_id?.() === this._appId ||
            window.get_wm_class() === this._appId;
    }

    _isTerminal(window) {
        const ids = [window.get_wm_class(), window.get_gtk_application_id?.(), window.get_sandboxed_app_id?.()]
            .filter(Boolean)
            .map(id => id.toLowerCase());
        return ids.some(id => TERMINAL_HINTS.some(hint => id.includes(hint)));
    }

    _sendPaste(window) {
        const keys = this._isTerminal(window)
            ? [Clutter.KEY_Control_L, Clutter.KEY_Shift_L, Clutter.KEY_v]
            : [Clutter.KEY_Control_L, Clutter.KEY_v];

        // Pressiona na ordem e solta na ordem inversa, como um usuário faria.
        const time = GLib.get_monotonic_time();
        keys.forEach(key =>
            this._keyboard.notify_keyval(time, key, Clutter.KeyState.PRESSED));
        [...keys].reverse().forEach(key =>
            this._keyboard.notify_keyval(time, key, Clutter.KeyState.RELEASED));
    }

    // Agenda um temporizador que é cancelado automaticamente no destroy().
    _schedule(intervalMs, callback) {
        const id = GLib.timeout_add(GLib.PRIORITY_DEFAULT, intervalMs, () => {
            const result = callback();
            if (result === GLib.SOURCE_REMOVE) {
                this._timeoutIds.delete(id);
            }
            return result;
        });
        this._timeoutIds.add(id);
    }
}
