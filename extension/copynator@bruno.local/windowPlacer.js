// =============================================================================
// windowPlacer.js
//
// Posiciona a janela do Copynator.
//
// No Wayland, aplicativos não podem escolher a posição das próprias janelas
// nem consultar a posição do cursor. O Shell pode fazer as duas coisas, então
// o fluxo é:
//   1. o daemon mostra a janela e emite WindowShown("cursor" | "center");
//   2. esta classe lê a posição do cursor, encontra a janela e a move,
//      respeitando a área útil do monitor (a janela nunca fica cortada);
//   3. a janela é ativada, recebendo o foco do teclado.
//
// Detalhes:
//   - a janela GTK escondida é destruída no compositor e recriada ao ser
//     mostrada, então o sinal pode chegar antes ou depois de a nova janela
//     existir;
//   - no 'window-created' o app-id ainda não foi informado pelo cliente e o
//     tamanho é 0; ambos só são confiáveis depois que a janela é exibida
//     (sinal 'shown'). Mover antes disso também seria desfeito pelo
//     posicionamento inicial do Mutter.
// =============================================================================
import GLib from 'gi://GLib';

import * as Main from 'resource:///org/gnome/shell/ui/main.js';

// Pedidos de posicionamento mais antigos que isso são descartados.
const PENDING_TIMEOUT_US = 2 * GLib.USEC_PER_SEC;

// Tempo máximo de espera pelo sinal 'shown' de uma janela nova.
const READY_TIMEOUT_MS = 500;

export class WindowPlacer {
    /** @param {string} appId ID da aplicação GTK (io.github.copynator) */
    constructor(appId) {
        this._appId = appId;
        this._pending = null; // pedido aguardando a janela ficar pronta
        this._windowsNotReady = new Set(); // janelas novas, ainda não exibidas

        this._windowCreatedId = global.display.connect('window-created',
            (_display, window) => this._onWindowCreated(window));
    }

    /** Pede que a janela seja posicionada ("cursor" ou "center"). */
    requestPlacement(placement) {
        // O cursor é lido AGORA: é a posição no momento do atalho/clique.
        const [pointerX, pointerY] = global.get_pointer();
        const request = {placement, pointerX, pointerY, time: GLib.get_monotonic_time()};

        const window = this._findWindow();
        if (window && !this._windowsNotReady.has(window)) {
            this._place(window, request); // janela já pronta
        } else {
            this._pending = request; // aguarda a janela ficar pronta
        }
    }

    destroy() {
        global.display.disconnect(this._windowCreatedId);
        this._windowsNotReady.clear();
        this._pending = null;
    }

    // -------------------------------------------------------------------------

    _isOurs(window) {
        return window.get_gtk_application_id?.() === this._appId ||
            window.get_wm_class() === this._appId;
    }

    _findWindow() {
        return global.get_window_actors()
            .map(actor => actor.get_meta_window())
            .find(window => window && this._isOurs(window)) ?? null;
    }

    _onWindowCreated(window) {
        // Ainda não dá para saber se a janela é nossa (ver cabeçalho): todas
        // as janelas novas são acompanhadas até ficarem prontas.
        this._windowsNotReady.add(window);

        const signalIds = [];
        let timeoutId = 0;
        const markReady = () => {
            if (!this._windowsNotReady.delete(window)) {
                return; // já tratada
            }
            signalIds.forEach(id => window.disconnect(id));
            if (timeoutId) {
                GLib.source_remove(timeoutId);
            }
            this._onWindowReady(window);
        };

        // 'shown' é emitido depois que o Mutter fez o posicionamento inicial.
        // O tempo limite cobre casos em que o sinal não chega.
        signalIds.push(window.connect('shown', markReady));
        signalIds.push(window.connect('unmanaged', () => {
            this._windowsNotReady.delete(window);
            signalIds.forEach(id => window.disconnect(id));
            if (timeoutId) {
                GLib.source_remove(timeoutId);
            }
        }));
        timeoutId = GLib.timeout_add(GLib.PRIORITY_DEFAULT, READY_TIMEOUT_MS, () => {
            timeoutId = 0;
            markReady();
            return GLib.SOURCE_REMOVE;
        });
    }

    _onWindowReady(window) {
        if (!this._pending || !this._isOurs(window)) {
            return;
        }
        const request = this._pending;
        this._pending = null;
        if (GLib.get_monotonic_time() - request.time < PENDING_TIMEOUT_US) {
            this._place(window, request);
        }
    }

    _place(window, request) {
        const frame = window.get_frame_rect();
        const monitorIndex = this._monitorIndexAt(request.pointerX, request.pointerY);
        const workArea = Main.layoutManager.getWorkAreaForMonitor(monitorIndex);

        let x, y;
        if (request.placement === 'center') {
            x = workArea.x + (workArea.width - frame.width) / 2;
            y = workArea.y + (workArea.height - frame.height) / 2;
        } else {
            // Canto superior esquerdo da janela no cursor, como no Win+V.
            x = request.pointerX;
            y = request.pointerY;
        }

        // Mantém a janela inteira dentro da área útil (sem barra do topo/dock).
        x = clamp(x, workArea.x, workArea.x + workArea.width - frame.width);
        y = clamp(y, workArea.y, workArea.y + workArea.height - frame.height);

        window.move_frame(true, Math.round(x), Math.round(y));
        Main.activateWindow(window);

        // Visível com G_MESSAGES_DEBUG=Gjs-Console (útil para diagnóstico).
        console.debug(`[Copynator] janela posicionada (${request.placement}) em ` +
            `${Math.round(x)},${Math.round(y)} — cursor em ${request.pointerX},${request.pointerY}, ` +
            `área útil ${workArea.x},${workArea.y} ${workArea.width}x${workArea.height}`);
    }

    // Índice do monitor que contém o ponto (ou o monitor principal).
    _monitorIndexAt(x, y) {
        const monitor = Main.layoutManager.monitors.find(m =>
            x >= m.x && x < m.x + m.width && y >= m.y && y < m.y + m.height);
        return monitor?.index ?? Main.layoutManager.primaryIndex;
    }
}

function clamp(value, min, max) {
    // Se a janela for maior que a área útil, prioriza o canto superior esquerdo.
    return Math.max(min, Math.min(value, max));
}
