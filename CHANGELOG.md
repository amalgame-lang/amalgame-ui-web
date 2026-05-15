# Changelog

All notable changes to `amalgame-ui-web` are recorded here.

## [Unreleased] — v0.0.4-dev

### Added — Form reading

- **Form-field builders on `Element`** — `Textarea(name)`,
  `Select(name)`, `Option(value, label)`, `CheckBox(name)`, and
  `Radio(name, value)` join the existing `Input(name)` builder.
  Multiple `Radio` elements sharing a `name` form a group; the
  form payload reports the selected one's `value`.
- **`Element.Bind(name)`** — declarative sucre for
  `.Attr("name", name)`. Reads more clearly at call sites that
  build inputs from raw `new Element(...)`.
- **`Page.ApplyTo` injects a form-collect bridge** —
  `window.__amc_collect()` walks every `input[name]`,
  `select[name]`, and `textarea[name]` and returns a plain
  object. The injection happens via `Window.Init` so it
  survives navigation.

### Added — OS theming

- **Baseline stylesheet auto-injected by `Page.Render`** — a tiny
  modern reset + system fonts + form-widget polish, themed via
  CSS variables (`--amc-bg`, `--amc-fg`, `--amc-muted`,
  `--amc-border`, `--amc-surface`, `--amc-accent`,
  `--amc-radius`). The dark variant flips under
  `@media (prefers-color-scheme: dark)`, which all modern
  webviews (WebView2 / WKWebView / WebKitGTK) honor natively
  from the OS theme setting.
- **`Page.SetStylesheet(url)`** — use a single external
  stylesheet *instead of* the baseline (e.g.
  `file:///abs/path/style.css`, an `https://…` URL, or a
  `data:text/css,…` URL). Disables the baseline implicitly.
- **`Page.AddCss(url)`** — stack additional stylesheets on top
  of whatever is already there (baseline or custom). Multiple
  calls layer in declaration order.
- **`Page.NoBaseline()`** — disable the baseline entirely. Use
  when shipping a CSS reset / framework (Tailwind, Pico) that
  starts from a blank slate.
- `<meta name="viewport" content="width=device-width,initial-scale=1">`
  added to every rendered page so HiDPI scales correctly inside
  the webview.

### Changed

- **`OnClick` handlers now receive form state** — the generated
  `onclick` calls `window._amc_N(JSON.stringify(window.__amc_collect()))`,
  so the `req: string` parameter is a JSON object (e.g.
  `{"user":"alice","newsletter":true}`) rather than the previous
  JS argument array `"[]"`. Checkboxes emit booleans, radio
  groups emit the selected `value` (key absent if none checked).
- `Element.Input` now uses the new `Bind(name)` helper internally
  for consistency with the other form-field builders.

### Fixed

- **`Page.RenderElement` no longer emits closing tags for HTML
  void elements** (`input`, `br`, `hr`, `img`, `meta`, `link`).
  Browsers tolerated `<input></input>` but it parsed as a stray
  end-tag and bloated the rendered document.

### Breaking

- The shape of `req` passed to `OnClick` handlers changed from a
  JSON array of JS call arguments to a JSON object of form-field
  state. Handlers that ignored `req` (e.g.
  `(req: string) => "true"`) are unaffected. Handlers that
  echoed `req` directly back to the JS side now return a JSON
  object string instead of an array string.

## [v0.0.3] — 2026-05-15

### Added

- HTML builder API: `Element` with `Stack`, `Row`, `Label`,
  `Heading`, `Button`, `Input`, `Pre`, `Div` static builders and
  fluent `Attr / Id / Class / Style / AddChild / SetText / OnClick`
  chaining.
- `Page` with `SetTitle`, `SetBody`, `Render`, and `ApplyTo`. The
  latter walks the tree, allocates `_amc_<n>` names for each
  `OnClick`, and auto-registers them via `Window.Bind`.
- `Json.EncodeString` helper for safe handler return values.
- `Html.Escape` for text/attribute escaping inside `Page.Render`.

Requires `amc >= 0.8.12` (relies on cross-package chained method
calls and inline-lambda-as-arg fixes).

## [v0.0.2] — 2026-05-15

### Added

- `Window.Bind(name, handler: Closure)` and `Window.Unbind(name)`
  for JS → Amalgame IPC. The C trampoline dispatches every
  `window.<name>(...)` call from the webview into
  `AmalgameClosure_call2(req, NULL)` and forwards the returned
  `code_string` to `webview_return`. 64-slot per-window binding
  registry.
- `Closure` type accepted in facade method signatures (requires
  `amc >= 0.8.11`).

## [v0.0.1] — 2026-05-15

Initial release. Single-window MVP:

- `Window.New(title, w, h, debug)` + `IsValid`, `SetTitle`,
  `SetSize(w, h, hint)`, `Navigate(url)`, `SetHtml(html)`,
  `Init(js)`, `Eval(js)`, `Run()`, `Terminate()`, `Destroy()`.
- `WindowHint.None / Min / Max / Fixed` mirror webview_hint_t.
- Vendored webview/webview v0.12.0 (MIT) under
  `runtime/vendor/webview/`.
