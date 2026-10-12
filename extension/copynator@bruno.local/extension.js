// =============================================================================
// extension.js — ponto de entrada da extensão do Copynator para o GNOME Shell
//
// Compõe os módulos da extensão:
//   DaemonClient     -> conversa com o daemon via D-Bus
//   ClipboardWatcher -> envia ao daemon cada texto copiado (essencial no Wayland)
//   WindowPlacer     -> posiciona a janela no cursor ou no centro da tela
//   Indicator        -> ícone no painel com clique esquerdo/direito
// =============================================================================
import {Extension} from 'resource:///org/gnome/shell/extensions/extension.js';
import * as Main from 'resource:///org/gnome/shell/ui/main.js';

import {ClipboardWatcher} from './clipboardWatcher.js';
import {DaemonClient} from './daemonClient.js';
import {CopynatorIndicator} from './indicator.js';
import {WindowPlacer} from './windowPlacer.js';

const APP_ID = 'io.github.copynator';

export default class CopynatorExtension extends Extension {
    enable() {
        this._windowPlacer = new WindowPlacer(APP_ID);

        this._daemon = new DaemonClient(
            placement => this._windowPlacer.requestPlacement(placement));

        // Inicia o daemon em segundo plano para que nenhuma cópia se perca.
        this._daemon.ensureRunning();

        this._clipboardWatcher = new ClipboardWatcher(text => {
            this._daemon.addEntry(text);
        });

        this._indicator = new CopynatorIndicator({
            toggleWindow: () => this._daemon.toggle('center'),
            clearHistory: () => this._daemon.clear(),
            setupShortcut: () => this._daemon.spawn(['--setup-shortcut']),
        });
        // Posição 0 da caixa direita: imediatamente à esquerda do grupo de
        // indicadores do sistema (Wi-Fi, volume, bateria).
        Main.panel.addToStatusArea(this.uuid, this._indicator, 0, 'right');
    }

    disable() {
        // Libera tudo na ordem inversa da criação (exigência do GNOME Shell:
        // nenhuma referência ou sinal pode sobrar após desativar).
        this._indicator?.destroy();
        this._indicator = null;

        this._clipboardWatcher?.destroy();
        this._clipboardWatcher = null;

        this._daemon?.destroy();
        this._daemon = null;

        this._windowPlacer?.destroy();
        this._windowPlacer = null;
    }
}
