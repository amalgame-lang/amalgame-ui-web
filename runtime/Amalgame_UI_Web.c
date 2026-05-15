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
