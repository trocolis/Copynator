// =============================================================================
// daemonClient.js
//
// Cliente D-Bus do daemon do Copynator (io.github.copynator.Daemon).
//
// - Encapsula o proxy e as chamadas assíncronas;
// - Se o daemon não estiver rodando, inicia o executável `copynator`
//   (que, por ser de instância única, passa a atender as próximas chamadas);
// - Repassa o sinal WindowShown para quem estiver interessado.
// =============================================================================
import Gio from 'gi://Gio';
import GLib from 'gi://GLib';

const BUS_NAME = 'io.github.copynator';
const OBJECT_PATH = '/io/github/copynator/Daemon';

// Somente a parte da interface usada pela extensão.
const DAEMON_INTERFACE_XML = `
<node>
  <interface name="io.github.copynator.Daemon">
    <method name="Toggle"><arg name="placement" type="s" direction="in"/></method>
    <method name="AddEntry"><arg name="content" type="s" direction="in"/></method>
    <method name="Clear"><arg name="removed" type="u" direction="out"/></method>
    <signal name="WindowShown"><arg name="placement" type="s"/></signal>
    <signal name="PasteRequested"/>
  </interface>
</node>`;

const DaemonProxy = Gio.DBusProxy.makeProxyWrapper(DAEMON_INTERFACE_XML);

export class DaemonClient {
    /**
     * @param {object} handlers
     * @param {(placement: string) => void} handlers.onWindowShown chamado
     *        quando o daemon mostra a janela ("cursor" ou "center").
     * @param {() => void} handlers.onPasteRequested chamado quando um item é
     *        escolhido e deve ser colado na janela anterior.
     */
    constructor({onWindowShown, onPasteRequested}) {
        this._onWindowShown = onWindowShown;
        this._onPasteRequested = onPasteRequested;
        this._signalIds = [];

        // DO_NOT_AUTO_START: o daemon é iniciado explicitamente por nós, com
        // o executável correto, e não pelo mecanismo de ativação do D-Bus.
        this._proxy = new DaemonProxy(
            Gio.DBus.session, BUS_NAME, OBJECT_PATH,
            (proxy, error) => {
                if (error) {
                    console.warn(`[Copynator] Falha ao criar proxy D-Bus: ${error.message}`);
                    return;
                }
                this._signalIds = [
                    proxy.connectSignal('WindowShown',
                        (_proxy, _sender, [placement]) => this._onWindowShown(placement)),
                    proxy.connectSignal('PasteRequested',
                        () => this._onPasteRequested()),
                ];
            },
            null,
            Gio.DBusProxyFlags.DO_NOT_AUTO_START);
    }

    /** O daemon está em execução (o nome tem dono no barramento)? */
    get isRunning() {
        return Boolean(this._proxy?.g_name_owner);
    }

    /** Garante que o daemon esteja rodando, iniciando-o em segundo plano. */
    ensureRunning() {
        if (!this.isRunning) {
            this.spawn(['--background']);
        }
    }

    /** Mostra/esconde a janela. Sem daemon, inicia-o já mostrando a janela. */
    async toggle(placement) {
        if (!this.isRunning) {
            this.spawn([placement === 'center' ? '--center' : '']);
            return;
        }
        await this._call(() => this._proxy.ToggleAsync(placement));
    }

    /** Envia um texto copiado para o histórico. */
    async addEntry(text) {
        if (this.isRunning) {
            await this._call(() => this._proxy.AddEntryAsync(text));
        }
    }

    /** Limpa o histórico não fixado. */
    async clear() {
        if (this.isRunning) {
            await this._call(() => this._proxy.ClearAsync());
        } else {
            this.spawn(['--clear']);
        }
    }

    /**
     * Executa o binário `copynator` com os argumentos informados.
     * O gnome-shell nem sempre tem ~/.local/bin no PATH, por isso esse local
     * também é verificado.
     */
    spawn(args) {
        const executable = GLib.find_program_in_path('copynator') ??
            GLib.build_filenamev([GLib.get_home_dir(), '.local', 'bin', 'copynator']);
        try {
            const argv = [executable, ...args.filter(arg => arg !== '')];
            Gio.Subprocess.new(argv, Gio.SubprocessFlags.NONE);
        } catch (error) {
            console.error(`[Copynator] Não foi possível executar '${executable}': ${error.message}`);
        }
    }

    destroy() {
        this._signalIds.forEach(id => this._proxy?.disconnectSignal(id));
        this._signalIds = [];
        this._proxy = null;
    }

    // Executa uma chamada D-Bus registrando (sem propagar) eventuais erros.
    async _call(invoke) {
        try {
            await invoke();
        } catch (error) {
            console.warn(`[Copynator] Chamada D-Bus falhou: ${error.message}`);
        }
    }
}
