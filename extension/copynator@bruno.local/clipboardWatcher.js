// =============================================================================
// clipboardWatcher.js
//
// Observa a área de transferência a partir do próprio GNOME Shell.
//
// No Wayland, aplicativos comuns não podem ler a área de transferência em
// segundo plano; o Shell (compositor) pode. Por isso a extensão escuta o sinal
// `owner-changed` do Meta.Selection — disparado por evento, sem polling — e
// envia o texto ao daemon via D-Bus.
// =============================================================================
import Meta from 'gi://Meta';
import St from 'gi://St';

// Mesmo limite usado pelo daemon (HistorySettings::maxContentBytes). Evita
// trafegar conteúdos enormes pelo D-Bus apenas para serem descartados.
const MAX_TEXT_LENGTH = 1024 * 1024;

// Tipo MIME usado por gerenciadores de senha (KeePassXC, Bitwarden, etc.)
// para indicar conteúdo sensível que NÃO deve ir para históricos.
const PASSWORD_HINT_MIME = 'x-kde-passwordManagerHint';

// Tipos MIME que indicam que o conteúdo pode ser lido como texto.
const TEXT_MIME_TYPES = ['text/plain', 'text/plain;charset=utf-8', 'UTF8_STRING', 'STRING', 'TEXT'];

export class ClipboardWatcher {
    /** @param {(text: string) => void} onTextCopied */
    constructor(onTextCopied) {
        this._onTextCopied = onTextCopied;
        this._selection = global.display.get_selection();
        this._ownerChangedId = this._selection.connect('owner-changed',
            (_selection, selectionType, source) => this._onOwnerChanged(selectionType, source));
    }

    _onOwnerChanged(selectionType, source) {
        // Apenas a área de transferência "Ctrl+C" (não a seleção primária).
        if (selectionType !== Meta.SelectionType.SELECTION_CLIPBOARD || !source) {
            return;
        }

        const mimeTypes = source.get_mimetypes();
        if (mimeTypes.includes(PASSWORD_HINT_MIME)) {
            return; // respeita a privacidade de senhas copiadas
        }
        if (!mimeTypes.some(mime => TEXT_MIME_TYPES.includes(mime))) {
            return; // imagens, arquivos, etc.
        }

        St.Clipboard.get_default().get_text(St.ClipboardType.CLIPBOARD, (_clipboard, text) => {
            if (text && text.length <= MAX_TEXT_LENGTH) {
                this._onTextCopied(text);
            }
        });
    }

    destroy() {
        if (this._ownerChangedId) {
            this._selection.disconnect(this._ownerChangedId);
            this._ownerChangedId = 0;
        }
    }
}
