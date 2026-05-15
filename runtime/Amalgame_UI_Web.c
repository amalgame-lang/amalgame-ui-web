/*
 * Amalgame.UI.Web — C glue layer.
 *
 * Translates the Amalgame_UI_Web_* int-handle API into webview_*
 * pointer-handle calls. The pointer table is kept here in this
 * single TU so every consumer .o that links against the package
 * shares the same handle space — same lesson as ui-tk's interp
 * pointer, learned the hard way.
 *
 * Include this .c file (or its compiled .o) once in the final
 * link. amc handles that via the [stdlib].sources field in
 * amalgame.toml.
 */

#include "Amalgame_UI_Web.h"
#include "vendor/webview/webview.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#ifdef _WIN32
#include <windows.h>
#include <shellapi.h>
#else
#include <unistd.h>
#include <sys/wait.h>
#endif

/* On Linux/BSD we want to flip the host GTK app to dark mode when the
 * OS is in dark mode — WebKitGTK renders <select> popups, native file
 * dialogs, and scrollbars through GTK, and these widgets ignore the
 * CSS `color-scheme: dark` declaration. The CSS path handles the
 * webview content; this handles the chrome around it. */
#if defined(__linux__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)
#include <gtk/gtk.h>
#define _AMALGAME_UI_WEB_HAVE_GTK 1
#endif

/* Slot table. NULL = free slot. Indexed by the int handle exposed
 * to Amalgame. v0.0.1 supports up to AMALGAME_UI_WEB_MAX_WINDOWS
 * simultaneous webviews — usually one is enough for productive
 * apps. Grow to a heap-allocated array if multi-window orchestration
 * lands. */
static webview_t _amalgame_uiweb_slots[AMALGAME_UI_WEB_MAX_WINDOWS] = {0};

static webview_t _slot_get(int slot) {
    if (slot < 0 || slot >= AMALGAME_UI_WEB_MAX_WINDOWS) return NULL;
    return _amalgame_uiweb_slots[slot];
}

int Amalgame_UI_Web_Create(int debug) {
    int slot;
    for (slot = 0; slot < AMALGAME_UI_WEB_MAX_WINDOWS; slot++) {
        if (_amalgame_uiweb_slots[slot] == NULL) break;
    }
    if (slot == AMALGAME_UI_WEB_MAX_WINDOWS) return -1;
    webview_t w = webview_create(debug ? 1 : 0, NULL);
    if (!w) return -1;
    _amalgame_uiweb_slots[slot] = w;
#ifdef _AMALGAME_UI_WEB_HAVE_GTK
    /* Flip GtkSettings::gtk-application-prefer-dark-theme when the OS
     * is in dark mode so <select> popups and other GTK-rendered chrome
     * match. Safe to call after webview_create — webview_create has
     * already invoked gtk_init internally. We only touch this setting
     * for the first window created; subsequent webview_create calls
     * inherit the same GtkSettings singleton. */
    if (slot == 0) {
        const char* theme = Amalgame_UI_Web_DetectOSTheme();
        if (theme && strcmp(theme, "dark") == 0) {
            GtkSettings* s = gtk_settings_get_default();
            if (s) {
                g_object_set(s, "gtk-application-prefer-dark-theme",
                             (gboolean)TRUE, NULL);
            }
        }
    }
#endif
    return slot;
}

int Amalgame_UI_Web_Destroy(int slot) {
    webview_t w = _slot_get(slot);
    if (!w) return 1;
    webview_destroy(w);
    _amalgame_uiweb_slots[slot] = NULL;
    return 0;
}

int Amalgame_UI_Web_SetTitle(int slot, const char* title) {
    webview_t w = _slot_get(slot);
    if (!w) return 1;
    return webview_set_title(w, title ? title : "");
}

int Amalgame_UI_Web_SetSize(int slot, int width, int height, int hint) {
    webview_t w = _slot_get(slot);
    if (!w) return 1;
    return webview_set_size(w, width, height, (webview_hint_t)hint);
}

int Amalgame_UI_Web_Navigate(int slot, const char* url) {
    webview_t w = _slot_get(slot);
    if (!w) return 1;
    return webview_navigate(w, url ? url : "about:blank");
}

int Amalgame_UI_Web_SetHtml(int slot, const char* html) {
    webview_t w = _slot_get(slot);
    if (!w) return 1;
    return webview_set_html(w, html ? html : "");
}

int Amalgame_UI_Web_Init(int slot, const char* js) {
    webview_t w = _slot_get(slot);
    if (!w) return 1;
    return webview_init(w, js ? js : "");
}

int Amalgame_UI_Web_Eval(int slot, const char* js) {
    webview_t w = _slot_get(slot);
    if (!w) return 1;
    return webview_eval(w, js ? js : "");
}

int Amalgame_UI_Web_Run(int slot) {
    webview_t w = _slot_get(slot);
    if (!w) return 1;
    return webview_run(w);
}

int Amalgame_UI_Web_Terminate(int slot) {
    webview_t w = _slot_get(slot);
    if (!w) return 1;
    return webview_terminate(w);
}

/* ─── v0.0.2 — Bind / Unbind / Return ───────────────────
 *
 * Each Bind entry holds the slot, the JS name, and the AmalgameClosure
 * pointer captured at registration. A single fixed trampoline
 * `_amalgame_uiweb_trampoline` is what webview actually invokes — it
 * looks up the entry from the `arg` opaque, calls the closure via
 * AmalgameClosure_call2(req, NULL), and forwards the returned
 * code_string to webview_return.
 *
 * Single-arg closure shape because the AM-side surface today is
 * `fn(req: string) -> string`. The second `call2` slot is reserved
 * for a future v0.0.3 surface where the seq id is exposed so
 * handlers can defer the reply (`call2(req, seq)`).
 */

typedef struct {
    int   slot;
    char* name;      /* heap-copied (strdup) so the AM-side string is
                        free to be GC'd or rebound */
    void* closure;   /* AmalgameClosure* */
} _binding_entry;

static _binding_entry _amalgame_uiweb_bindings[AMALGAME_UI_WEB_MAX_BINDINGS] = {0};

static int _find_binding(int slot, const char* name) {
    int i;
    for (i = 0; i < AMALGAME_UI_WEB_MAX_BINDINGS; i++) {
        _binding_entry* b = &_amalgame_uiweb_bindings[i];
        if (b->slot == slot && b->name && name && strcmp(b->name, name) == 0) {
            return i;
        }
    }
    return -1;
}

static int _free_binding_slot(void) {
    int i;
    for (i = 0; i < AMALGAME_UI_WEB_MAX_BINDINGS; i++) {
        if (_amalgame_uiweb_bindings[i].name == NULL) return i;
    }
    return -1;
}

static void _amalgame_uiweb_trampoline(const char* seq, const char* req, void* arg) {
    _binding_entry* b = (_binding_entry*)arg;
    if (!b || !b->closure) {
        /* No closure attached — return null to JS so the await resolves */
        webview_t w = _slot_get(b ? b->slot : -1);
        if (w) webview_return(w, seq, 0, "null");
        return;
    }
    /* Call AM closure: fn(req: string) -> string.
     * The C ABI is `void* fn(void* env, void* arg)` — `req` is the only
     * AM-visible argument; `seq` is reserved (passed as NULL) for a
     * future deferred-reply variant. */
    void* result = AmalgameClosure_call2(
        (AmalgameClosure*)b->closure,
        (void*)req,
        NULL
    );
    const char* res = result ? (const char*)result : "";
    webview_t w = _slot_get(b->slot);
    if (w) webview_return(w, seq, 0, res);
}

int Amalgame_UI_Web_Bind(int slot, const char* name, void* closure) {
    webview_t w = _slot_get(slot);
    if (!w || !name) return 1;
    int existing = _find_binding(slot, name);
    if (existing >= 0) {
        /* Rebinding — replace closure in-place, no need to re-call
         * webview_bind (the trampoline arg is already pointing at this
         * registry entry). */
        _amalgame_uiweb_bindings[existing].closure = closure;
        return 0;
    }
    int idx = _free_binding_slot();
    if (idx < 0) return 2;
    /* Manual strdup so the name survives the AM-side string's lifetime. */
    size_t n = strlen(name);
    char* copy = (char*)malloc(n + 1);
    if (!copy) return 3;
    memcpy(copy, name, n + 1);
    _amalgame_uiweb_bindings[idx].slot    = slot;
    _amalgame_uiweb_bindings[idx].name    = copy;
    _amalgame_uiweb_bindings[idx].closure = closure;
    return webview_bind(w, name, _amalgame_uiweb_trampoline,
                        &_amalgame_uiweb_bindings[idx]);
}

int Amalgame_UI_Web_Unbind(int slot, const char* name) {
    webview_t w = _slot_get(slot);
    if (!w || !name) return 1;
    int idx = _find_binding(slot, name);
    if (idx < 0) return 0;  /* idempotent — already gone */
    webview_unbind(w, name);
    free(_amalgame_uiweb_bindings[idx].name);
    _amalgame_uiweb_bindings[idx].slot    = 0;
    _amalgame_uiweb_bindings[idx].name    = NULL;
    _amalgame_uiweb_bindings[idx].closure = NULL;
    return 0;
}

int Amalgame_UI_Web_Return(int slot, const char* seq, int status,
                            const char* result) {
    webview_t w = _slot_get(slot);
    if (!w || !seq) return 1;
    return webview_return(w, seq, status, result ? result : "");
}

/* ─── v0.0.4 — OS theme detection ─────────────────────────
 *
 * Why this lives here rather than relying on the CSS
 * `@media (prefers-color-scheme: dark)` query: WebKitGTK does
 * not propagate the GNOME/KDE color-scheme to the embedded
 * page automatically. So we read the OS preference here and
 * AM emits `<html data-theme="dark">` on render, which the
 * baseline stylesheet keys off via attribute selectors.
 *
 * WebView2 and WKWebView do honor prefers-color-scheme — but
 * the data-theme path stays the canonical control surface
 * across all three platforms for consistency. */

const char* Amalgame_UI_Web_DetectOSTheme(void) {
    /* 0. Explicit env override wins. */
    const char* env = getenv("AMALGAME_UI_THEME");
    if (env) {
        if (strcmp(env, "dark") == 0)  return "dark";
        if (strcmp(env, "light") == 0) return "light";
    }

#if defined(__APPLE__)
    /* macOS: `defaults read -g AppleInterfaceStyle` prints `Dark`
     * (with a trailing newline) when dark mode is on; errors and
     * prints nothing otherwise. */
    FILE* p = popen("defaults read -g AppleInterfaceStyle 2>/dev/null", "r");
    if (p) {
        char buf[32] = {0};
        char* got = fgets(buf, sizeof(buf), p);
        pclose(p);
        if (got && strncmp(buf, "Dark", 4) == 0) return "dark";
    }
    return "light";
#elif defined(_WIN32)
    /* Windows: HKCU\Software\Microsoft\Windows\CurrentVersion\Themes\
     *          Personalize\AppsUseLightTheme → DWORD (0 = dark, 1 = light). */
    HKEY key;
    DWORD val = 1;
    DWORD size = sizeof(val);
    if (RegOpenKeyExA(HKEY_CURRENT_USER,
                      "Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
                      0, KEY_READ, &key) == ERROR_SUCCESS) {
        RegQueryValueExA(key, "AppsUseLightTheme", NULL, NULL,
                         (LPBYTE)&val, &size);
        RegCloseKey(key);
    }
    return (val == 0) ? "dark" : "light";
#else
    /* Linux: prefer `gsettings get org.gnome.desktop.interface
     * color-scheme` (GNOME 42+) — value is one of `'default'`,
     * `'prefer-dark'`, `'prefer-light'` (single-quoted). Falls
     * back to GTK_THEME containing `:dark`. */
    FILE* p = popen("gsettings get org.gnome.desktop.interface color-scheme 2>/dev/null", "r");
    if (p) {
        char buf[64] = {0};
        char* got = fgets(buf, sizeof(buf), p);
        pclose(p);
        if (got) {
            if (strstr(buf, "prefer-dark"))  return "dark";
            if (strstr(buf, "prefer-light")) return "light";
        }
    }
    const char* gtk = getenv("GTK_THEME");
    if (gtk && strstr(gtk, ":dark")) return "dark";
    /* Older GNOME / KDE / minimal WMs — default to light. */
    return "light";
#endif
}

/* v0.0.5 — open an external URL in the OS default browser.
 *
 * The scheme check is the only sanity gate we want — once we've
 * established the URL is one of http://, https://, file://, we
 * pass it as a single argv element to the platform launcher
 * (execvp on Unix, ShellExecuteA on Windows). No shell, so no
 * quoting / metacharacter concerns.
 *
 * Returns 0 on success, non-zero on bad scheme / launcher failure. */
int Amalgame_UI_Web_OpenUrl(const char* url) {
    if (!url) return 1;
    if (strncmp(url, "http://",  7) != 0 &&
        strncmp(url, "https://", 8) != 0 &&
        strncmp(url, "file://",  7) != 0) {
        return 2;  /* unsupported scheme — don't leak to a launcher */
    }
#if defined(_WIN32)
    HINSTANCE r = ShellExecuteA(NULL, "open", url, NULL, NULL, SW_SHOWNORMAL);
    return ((INT_PTR)r > 32) ? 0 : 3;
#elif defined(__APPLE__)
    pid_t pid = fork();
    if (pid < 0) return 4;
    if (pid == 0) {
        execlp("open", "open", url, (char*)NULL);
        _exit(127);  /* execlp failed — child exits, parent unaffected */
    }
    return 0;
#else
    pid_t pid = fork();
    if (pid < 0) return 4;
    if (pid == 0) {
        execlp("xdg-open", "xdg-open", url, (char*)NULL);
        _exit(127);
    }
    return 0;
#endif
}
