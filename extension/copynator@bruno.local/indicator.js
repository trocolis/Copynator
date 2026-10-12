// =============================================================================
// indicator.js
//
// Ícone do Copynator na barra superior, junto aos indicadores do sistema
// (Wi-Fi, volume, bateria).
//   - Clique esquerdo: abre/fecha o histórico no centro da tela;
//   - Clique direito: menu rápido (abrir, limpar, configurar atalho).
// =============================================================================
import Clutter from 'gi://Clutter';
import GObject from 'gi://GObject';
import St from 'gi://St';

import * as PanelMenu from 'resource:///org/gnome/shell/ui/panelMenu.js';
import * as PopupMenu from 'resource:///org/gnome/shell/ui/popupMenu.js';

export const CopynatorIndicator = GObject.registerClass(
class CopynatorIndicator extends PanelMenu.Button {
    /**
     * @param {object} actions
     * @param {() => void} actions.toggleWindow
     * @param {() => void} actions.clearHistory
     * @param {() => void} actions.setupShortcut
     */
    _init(actions) {
        super._init(0.5, 'Copynator', false);
        this._actions = actions;

        this.add_child(new St.Icon({
            icon_name: 'edit-paste-symbolic',
            style_class: 'system-status-icon',
        }));

        this._buildMenu();
    }

    _buildMenu() {
        const openItem = new PopupMenu.PopupMenuItem('Abrir histórico');
        openItem.connect('activate', () => this._actions.toggleWindow());
        this.menu.addMenuItem(openItem);

        const clearItem = new PopupMenu.PopupMenuItem('Limpar histórico (exceto fixados)');
        clearItem.connect('activate', () => this._actions.clearHistory());
        this.menu.addMenuItem(clearItem);

        this.menu.addMenuItem(new PopupMenu.PopupSeparatorMenuItem());

        const shortcutItem = new PopupMenu.PopupMenuItem('Configurar atalho Shift+Super+V');
        shortcutItem.connect('activate', () => this._actions.setupShortcut());
        this.menu.addMenuItem(shortcutItem);
    }

    // Por padrão, qualquer clique abre o menu do PanelMenu.Button. Aqui o
    // clique esquerdo (ou toque) alterna a janela e só o direito abre o menu.
    vfunc_event(event) {
        const type = event.type();
        const isPress = type === Clutter.EventType.BUTTON_PRESS;
        const isTouch = type === Clutter.EventType.TOUCH_BEGIN;

        if (isPress && event.get_button() === Clutter.BUTTON_SECONDARY) {
            this.menu.toggle();
            return Clutter.EVENT_STOP;
        }
        if (isPress || isTouch) {
            this.menu.close();
            this._actions.toggleWindow();
            return Clutter.EVENT_STOP;
        }
        return Clutter.EVENT_PROPAGATE;
    }
});
