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
  seven CSS variables (`--amc-bg`, `--amc-fg`, `--amc-muted`,
  `--amc-border`, `--amc-surface`, `--amc-accent`,
  `--amc-radius`). The dark variant keys off `[data-theme=dark]`
  on `<html>` (set by Render from the OS theme); a secondary
  `@media (prefers-color-scheme: dark)` fallback applies when
  callers ship their own raw HTML and skip Render.
- **OS theme auto-detection** via the new C primitive
  `Amalgame_UI_Web_DetectOSTheme()`:
  - macOS: `defaults read -g AppleInterfaceStyle`
  - Windows: `AppsUseLightTheme` registry DWORD
  - Linux: `gsettings ... color-scheme` (GNOME 42+) + `GTK_THEME`
    `:dark` substring fallback
  - Environment override: `AMALGAME_UI_THEME=dark|light` wins.
- **GTK dark theme flip on Linux** —
  `Amalgame_UI_Web_Create` sets
  `GtkSettings::gtk-application-prefer-dark-theme = TRUE` when
  the OS is in dark mode. Necessary because WebKitGTK renders
  `<select>` popups, file dialogs, and scrollbars through native
  GTK widgets that ignore the CSS `color-scheme` declaration.
- `:root { color-scheme: light }` / `[data-theme=dark] { color-scheme: dark }`
  in the baseline so the browser chrome (scrollbars, focus
  rings, text-selection color) matches the page theme on
  WebView2 / WKWebView.
- **`Page.SetTheme(mode)`** — force `"light"` / `"dark"` /
  `"auto"` (default; defers to `DetectOSTheme()`).
- **`Page.DetectOSTheme()`** static — returns `"light"` or
  `"dark"`. Useful when an app needs to know the OS theme for
  reasons other than CSS (e.g. picking matching iconography).
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
- `<html data-theme="…">` written on every render so user CSS
  can theme off the same attribute as the baseline.

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
- **Webview chrome locked down by default** —
  `Page.ApplyTo` injects guards that swallow the right-click
  context menu and the reload hotkeys (`F5`, `Ctrl+R`/`Cmd+R`,
  plus the shifted hard-reload variants). The page is loaded
  via `SetHtml`, so reloading would land on `about:blank` and
  strand the user. Apps that navigate to real URLs and want the
  standard browser chrome opt back in with
  `Page.New().AllowBrowserDefaults()`. `Ctrl+Shift+I` (DevTools
  in debug mode) is intentionally left untouched.
- **Textarea `resize: none`** in the baseline — the drag-grip
  destabilizes declarative layouts. Re-enable per-element via
  `.Style("resize:vertical")` when needed.
- **Body `user-select: none`** in the baseline so labels,
  headings, and buttons can't be highlighted with a cursor drag
  — matches the default behavior of native desktop apps and
  Tauri/Electron. Selection is re-enabled on `input`, `select`,
  `textarea`, and `pre` so form fields stay editable and code
  / output panels stay copy-friendly. Re-enable on a specific
  element via `.Style("user-select:text")`.

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
