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
