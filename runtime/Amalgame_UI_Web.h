/*
 * Amalgame Standard Library — Amalgame.UI.Web
 * Copyright (c) 2026 Bastien Mouget
 * https://github.com/amalgame-lang/Amalgame
 *
 * Thin C binding over webview/webview. Exposes the
 * `Amalgame_UI_Web_*` surface that facade.am @c blocks call into.
 *
 * Webview handles (`webview_t` upstream) are opaque pointers that
 * we propagate as `void*` here — Amalgame sees them as int handles
 * indirected through a static slot table so the AM-side type
 * stays simple. v0.0.1 caps the table at 4 simultaneous windows;
 * the cap moves to a heap-resizing array when multi-window lands
 * in v0.10+.
 *
 * Why webview? OS-native rendering with the smallest surface area
 * possible — Wails (Go), countless indie cross-platform tools, and
 * v1 Tauri (Rust) all rely on this exact library.
 *
 * v0.0.1 surface:
 *   - Amalgame_UI_Web_Create(debug)          → int slot, -1 on err
 *   - Amalgame_UI_Web_Destroy(slot)
 *   - Amalgame_UI_Web_SetTitle(slot, title)
 *   - Amalgame_UI_Web_SetSize(slot, w, h, hint)
 *   - Amalgame_UI_Web_Navigate(slot, url)
 *   - Amalgame_UI_Web_SetHtml(slot, html)
 *   - Amalgame_UI_Web_Init(slot, js)         (inject before page load)
 *   - Amalgame_UI_Web_Eval(slot, js)         (run JS now)
 *   - Amalgame_UI_Web_Run(slot)              (block on event loop)
 *   - Amalgame_UI_Web_Terminate(slot)
 *
 * v0.0.2 additions — bidirectional IPC:
 *   - Amalgame_UI_Web_Bind(slot, name, closure)   register JS handler
 *   - Amalgame_UI_Web_Unbind(slot, name)
 *   - Amalgame_UI_Web_Return(slot, seq, st, res)  reply from a handler
 *
 * The Bind call takes an AmalgameClosure* (cast to void*). The C
 * trampoline dispatches the JS request through AmalgameClosure_call2
 * with (req: code_string, NULL); the closure returns a code_string
 * that the trampoline forwards to webview_return automatically.
 *
 * Handler signature on the Amalgame side (single string arg):
 *
 *     fn(req: string) -> string
 *
 * `req` is the JSON-encoded JS argument array (so `window.foo(1, "x")`
 * gives `req == "[1,\"x\"]"`). Return any string — it's parsed back
 * as JSON on the JS side and becomes the awaited value.
 *
 * Linking:
 *   Linux  : -lwebkit2gtk-4.1 (pkg-config webkit2gtk-4.1)
 *   macOS  : -framework Cocoa -framework WebKit
 *   Windows: WebView2Loader.lib + WIL headers
 */

#ifndef AMALGAME_UI_WEB_H
#define AMALGAME_UI_WEB_H

#include "_runtime.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Size hints accepted by SetSize. Mirror webview_hint_t exactly so
 * the AM int passes through unchanged. */
#define AMALGAME_UI_WEB_HINT_NONE  0
#define AMALGAME_UI_WEB_HINT_MIN   1
#define AMALGAME_UI_WEB_HINT_MAX   2
#define AMALGAME_UI_WEB_HINT_FIXED 3

/* All functions return 0 on success, non-zero on error, except
 * Create which returns a slot id (-1 on error). Slot ids are
 * non-negative ints in [0, AMALGAME_UI_WEB_MAX_WINDOWS). */
#define AMALGAME_UI_WEB_MAX_WINDOWS 4

int  Amalgame_UI_Web_Create(int debug);
int  Amalgame_UI_Web_Destroy(int slot);
int  Amalgame_UI_Web_SetTitle(int slot, const char* title);
int  Amalgame_UI_Web_SetSize(int slot, int width, int height, int hint);
int  Amalgame_UI_Web_Navigate(int slot, const char* url);
int  Amalgame_UI_Web_SetHtml(int slot, const char* html);
int  Amalgame_UI_Web_Init(int slot, const char* js);
int  Amalgame_UI_Web_Eval(int slot, const char* js);
int  Amalgame_UI_Web_Run(int slot);
int  Amalgame_UI_Web_Terminate(int slot);

/* v0.0.2 — bidirectional IPC.
 *
 * Bind: register `closure` (an AmalgameClosure*, cast to void* at the
 * call boundary) as the handler for `window.<name>(...)` calls from
 * JS. Returns 0 on success, non-zero if the registry is full or the
 * slot is invalid.
 *
 * Unbind: drop the binding. Idempotent.
 *
 * Return: usually not called from Amalgame — the trampoline auto-
 * forwards the closure's return value. Exposed for advanced cases
 * where the handler needs to defer the response (e.g. async I/O). */
#define AMALGAME_UI_WEB_MAX_BINDINGS 64

int  Amalgame_UI_Web_Bind(int slot, const char* name, void* closure);
int  Amalgame_UI_Web_Unbind(int slot, const char* name);
int  Amalgame_UI_Web_Return(int slot, const char* seq, int status,
                            const char* result);

/* v0.0.4 — OS theme detection.
 *
 * Returns "dark" or "light" based on the running OS's preferences:
 *   - macOS  : `defaults read -g AppleInterfaceStyle` (Dark = dark).
 *   - Windows: registry AppsUseLightTheme (0 = dark, 1 = light).
 *   - Linux  : `gsettings get org.gnome.desktop.interface color-scheme`
 *              (`'prefer-dark'` → dark); falls back to GTK_THEME env
 *              substring `:dark`.
 *
 * Environment override: AMALGAME_UI_THEME=dark|light wins over OS
 * detection. Useful in tests and in apps that expose an in-app
 * theme switcher.
 *
 * The return pointer points into a static string literal — do not
 * free, and do not assume it persists across threads. */
const char* Amalgame_UI_Web_DetectOSTheme(void);

#ifdef __cplusplus
}
#endif

#endif /* AMALGAME_UI_WEB_H */
