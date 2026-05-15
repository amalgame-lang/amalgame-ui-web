# Changelog

All notable changes to `amalgame-ui-web` are recorded here.

## [Unreleased] — v0.0.5-dev

### Added — Declarative result routing

- **`Element.OnResult(targetId)`** — pair with `OnClick` to push
  the handler's return value into another element by id, without
  a manual click-listener bridge. The Render-time `onclick` wraps
  the bound call in `window.__amc_route(...)`, which awaits the
  promise from `Window.Bind` and writes the result into
  `document.getElementById(targetId)`. Valid JSON returns are
  pretty-printed (two-space indent); plain strings pass through
  verbatim, so handlers returning either shape both render
  usefully. No-op when the target id doesn't resolve at click
  time. Pass `""` to clear a previously-set target.
- **`window.__amc_route(promise, id)`** auto-injected by
  `Page.ApplyTo` alongside `window.__amc_collect`. Exposed for
  app code that wants to call it directly from raw JS rather
  than via the `Element.OnResult` builder.

### Added — Reactive `OnChange` event

- **`Element.OnChange(handler: Closure)`** — fire on `change`
  for inputs/selects and on both `change` + `input` for text
  inputs/textareas (live update on every keystroke). Same JSON
  form payload as `OnClick`, so handlers can read every named
  field — not just the one that changed. Composes with
  `OnResult` exactly like `OnClick` does.

### Added — Layout sugar

- **`Element.Grid(rows: int, cols: int, gap: int)`** — CSS Grid
  container builder. Equivalent to ui-forms' `GridLayout`.
  Children flow left-to-right, top-to-bottom into `rows × cols`
  cells; pass `rows = 0` for implicit row count.
- **`Element.AbsoluteContainer()`** — `position:relative` div
  that anchors absolutely-positioned children. Equivalent to
  ui-forms' `AbsoluteLayout`.
- **`Element.Position(x: int, y: int)`** — inline sugar for
  `position:absolute;left:Xpx;top:Ypx`. Pair with `Size(w, h)`
  for full x/y/w/h placement.

### Added — WinForms-aligned widgets

The webview-side equivalents of common .NET / WinForms toolbox
controls — pure HTML5, leveraging the OS form widgets for native
pickers and accessibility:

- **`Element.Password(name)`** → `<input type=password>` —
  WinForms `TextBox` with `PasswordChar`.
- **`Element.Number(name, min, max, step)`** →
  `<input type=number>` — `NumericUpDown`.
- **`Element.Slider(name, min, max, step)`** →
  `<input type=range>` — `TrackBar`.
- **`Element.DatePicker(name)`** → `<input type=date>` —
  `DateTimePicker` (date-only).
- **`Element.TimePicker(name)`** → `<input type=time>`.
- **`Element.ColorPicker(name)`** → `<input type=color>` —
  inline `ColorDialog`.
- **`Element.ProgressBar(value, max)`** → `<progress>` —
  `ProgressBar`. Pass `value < 0` for an indeterminate bar.
- **`Element.Image(src)`** → `<img>` — accepts `file://`,
  `https://`, or `data:image/...` URLs.
- **`Element.Link(text, url)`** → `<a href>` — `LinkLabel`.
- **`Element.ListBox(name, size)`** → `<select multiple>` —
  `ListBox`. Auto-collect doesn't currently report multi-select
  values; attach `OnChange` and read `.selectedOptions` via
  `Window.Eval` until v0.0.6.

### Changed

- `tests/spike_form.am` drops its in-page click-listener bridge
  (the `v0.0.5 may add a declarative .OnResult` TODO from v0.0.4)
  in favor of the new builder. The spike is now four lines shorter
  and matches what `amc new --template ui-web-form` will scaffold
  once amc v0.8.14 ships.

### Added — Event-table refactor + WinForms property surface

- **Generic event binder `Element.On(eventName, handler)`** —
  attach any DOM event ("click", "change", "wheel", "drop", custom
  names). Internal bookkeeping is a parallel
  `(event, handler, resultTarget)` list, so multiple events can
  coexist on a single Element without per-event boilerplate.
- **WinForms-aligned event setters** as sugar over `.On(...)` —
  `.OnClick`, `.OnDblClick`, `.OnChange`, `.OnFocus`, `.OnBlur`,
  `.OnMouseEnter`, `.OnMouseLeave`, `.OnKeyDown`, `.OnKeyUp`.
  `OnChange` registers both `change` + `input` for text inputs and
  textareas (live update on every keystroke).
- **WinForms-style property setters** on every Element:
  `.Visible(bool)`, `.Enabled(bool)`, `.Tooltip(text)`,
  `.TabIndex(n)`, `.ForeColor(css)`, `.BackColor(css)`,
  `.Font(family, sizePx)`, `.DataTag(payload)`.
- **`Element.Raw(html)`** — escape hatch for one-off raw HTML
  injection (SVG fragments, JS-lib mount markup, etc.).

### Added — Completing the WinForms Common Controls

Widgets ticking off the remaining 🟢 rows in
[`docs/winforms-mapping.md`](docs/winforms-mapping.md):

- **`Element.PictureBox(src)`** — alias for `Image(src)`.
- **`Element.Panel()`** — alias for `Div()`.
- **`Element.GroupBox(title)`** — `<fieldset><legend>`.
- **`Element.Flow(direction)`** — flex with wrap (FlowLayoutPanel).
- **`Element.ToolStrip()`** — themed button row.
- **`Element.StatusStrip()`** — fixed-bottom status footer.
- **`Element.Iframe(url)`** — embed (WebBrowser).
- **`Element.MaskedTextBox(name, pattern, inputmode)`** —
  HTML5 `pattern` + `inputmode` (phone, zip, decimal…).
- **`Element.CheckedListBox(name)`** + **`Element.CheckedItem(name, value, label)`** —
  multi-select with checkboxes.
- **`Element.ListView(headers)`** + **`Element.ListViewRow(values)`** —
  table-mode ListView with header row.
- **`Element.TabControl(groupName)`** + **`Element.Tab(group, id, label, body)`** —
  pure-CSS tabs (radio-button sibling-selector pattern, no JS).

### Added — Baseline CSS themed rules

The baseline stylesheet grew rules for `fieldset.amc-groupbox`,
`footer.amc-statusstrip`, `.amc-toolstrip`, `iframe.amc-iframe`,
`table.amc-listview`, `ul.amc-checkedlistbox`, and `.amc-tabs`.
All colors reference `--amc-*` variables, so light/dark flip
remains automatic. Confirmed on Linux (WebKitGTK 4.1) — macOS /
Windows parity assumed but not yet smoke-tested.

### Added — Architecture documentation

Two reference docs under `docs/`:
- [`winforms-mapping.md`](docs/winforms-mapping.md) — full
  WinForms toolbox cross-reference with shipping status per
  release.
- [`architecture.md`](docs/architecture.md) — three-level model
  (Element / Component / Form), event-table model, CSS/JS
  injection layering, and four canonical extension scenarios.

### Known issues

- **amc chain-length quadratic** — building a single fluent
  `.AddChild(...)`-chain with ~24+ links sends amc's type
  inference into a non-terminating recursion. Workaround: split
  the body into `let block: Element = Element.Stack()...`
  intermediates (see `tests/dump_html.am`). Tracked as an amc fix
  (memoize `InferTypeFromExpr` by node identity).
- **List literals (`["a", "b"]`)** are not yet parsed by amc;
  pass `List<string>` built via `new List<string>(); list.Add(…)`.
  Tracked as an amc parser feature.

## [v0.0.4] — 2026-05-15

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
