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
 * IPC (webview_bind callback) is deferred to v0.0.2 — the C
 * callback shape doesn't yet have a clean Amalgame closure ABI
 * across all targets. Workaround: drive the JS side via Eval() and
 * have the page POST to a local http server hosted from Amalgame.
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

#ifdef __cplusplus
}
#endif

#endif /* AMALGAME_UI_WEB_H */
