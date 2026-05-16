# Changelog

All notable changes to `amalgame-ui-web` are recorded here.

## [Unreleased] — v0.0.9-dev

### Added — `Element.RichTextBox(name)`

Editable multi-line area that accepts inline formatting — the
WinForms `RichTextBox` equivalent. Rendered as a `<div
contenteditable="true">` so the user can paste rich content from
another app and use the browser's built-in shortcuts
(`Ctrl-B` / `Ctrl-I` / `Ctrl-U`) for inline formatting.

```amalgame
Element.RichTextBox("notes")
    .Attr("placeholder-text", "Write notes here…")
    .Style("min-height:120px")
```

The form payload reports the rich-text content as the inner HTML
under the `name` key. `__amc_collect` was extended to walk
`[contenteditable][name]` elements alongside the regular form
fields.

A built-in toolbar (Bold / Italic / Underline / Heading buttons)
is a v0.0.10 candidate; for now, attach a custom toolbar via
`Window.Eval` if you need one.

### Added — `Element.MonthCalendar(name, year, month)`

Inline month grid — the WinForms `MonthCalendar` equivalent. The
HTML markup is computed at page load by a small bridge JS that
runs from `Page.ApplyTo`; the AM side just declares the widget
with a form `name` and an initial `(year, month)`.

```amalgame
Element.MonthCalendar("birthday", 2026, 5)
```

Clicking a day highlights it; the form payload reports the
selection as an ISO date `YYYY-MM-DD` under `name`. Today's
date is auto-outlined with the accent color.

No navigation buttons in v0.0.9 — re-render via `Page.PatchInner`
with a different `(year, month)` to switch the displayed month.
A built-in ◀/▶ navigator is a v0.0.10 candidate.

### Added — Component pattern (documentation)

Recommended pattern for reusable widgets is a regular Amalgame
class with a `Render(): Element` method:

```amalgame
public class LabeledInput {
    public Caption: string
    public Name:    string

    public LabeledInput(caption: string, name: string) {
        this.Caption = caption
        this.Name    = name
    }

    public Element Render() {
        return Element.Row()
            .AddChild(Element.Label(this.Caption).Size(120, 0))
            .AddChild(Element.Input(this.Name))
    }
}

// usage
let first: LabeledInput = new LabeledInput("First name:", "first")
form.AddChild(first.Render())
```

No abstract base / interface — Amalgame's static dispatch makes
virtual overrides on a parent unreliable, so the convention
keeps things flat and predictable. Ship reusable widgets as a
regular Amalgame package; consumers pull them via
`amc package add my-team-widgets` and instantiate them like any
class.

amc bug-of-the-day: `new LabeledInput(...).Render()` chained in
one expression doesn't lower correctly today — split via a
`let` intermediate. Tracked separately.

### Changed — `__amc_collect`

The form-state bridge now also collects from `contenteditable[name]`
(reports `.innerHTML`) and `.amc-monthcal[name]` (reports the
ISO date built from `data-year` + `data-month` +
`data-selected`). Existing collectors for `input` / `select` /
`textarea` are unchanged.

### spike_v009

A standalone `tests/spike_v009.am` exercises the two new widgets
and the Component pattern (LabeledInput class with three
instances). Click "Submit" to see all four form keys
(`notes` rich text, `birthday` ISO date, `first`/`last`/`email`
component-filled) come through as a JSON object.

## [v0.0.8] — 2026-05-16

### Added — `MenuBar` (HTML/CSS, common cross-OS)

App-level horizontal menu strip, same shape as WinForms'
`MenuStrip`. HTML/CSS-only — renders pixel-identical on every
OS through the `--amc-*` theme variables. The OS-native variant
(Win32 / NSMenu / GtkMenuBar) is planned as opt-in in v0.1.0;
the common path here is what VS Code Desktop / Slack / Discord
ship with.

```amalgame
Element.MenuBar()
    .AddChild(Element.MenuItem("File")
        .AddChild(Element.MenuOption("New",   "amc_new"))
        .AddChild(Element.MenuOption("Open…", "amc_open"))
        .AddChild(Element.MenuSeparator())
        .AddChild(Element.MenuOption("Quit",  "amc_quit")))
    .AddChild(Element.MenuItem("Edit")
        .AddChild(Element.MenuOption("Undo",  "amc_undo")))
```

Four builders: `MenuBar()` (the `<nav>` container), `MenuItem(label)`
(label + dropdown panel — internally a `<details>` whose
`<summary>` is the label and children form the popup),
`MenuOption(label, actionName)` (a `<button>` inside the popup
that calls `window.<actionName>('')` on click), and
`MenuSeparator()` (themed `<hr>`).

Bridge wiring: a global `click` listener closes any open menu
when the user clicks outside it; `Escape` closes every open
menu and any visible context menu.

### Added — `Element.ContextMenu` (right-click menu)

```amalgame
let cm = Element.ContextMenu("workspace")
    .AddChild(Element.MenuOption("Cut",   "amc_cut"))
    .AddChild(Element.MenuOption("Copy",  "amc_copy"))
    .AddChild(Element.MenuSeparator())
    .AddChild(Element.MenuOption("Paste", "amc_paste"))

Element.Div().Id("workspace").Class("amc-ctx-target")
    .AddChild(cm)
    .AddChild( … your actual content … )
```

The bridge listens for `contextmenu` events on `.amc-ctx-target`
ancestors, calls `e.preventDefault()`, and positions the menu
at the cursor via `position:fixed`. Click outside / Escape hides
it. Reuses `MenuOption` / `MenuSeparator` so the look and bind
shape match the MenuBar.

### Added — `Element.SplitContainer`

Two-pane resizable container — equivalent to WinForms'
`SplitContainer`. `orientation` is `"row"` (left/right with a
vertical divider) or `"column"` (top/bottom). `initialRatio` is
the first pane's percentage 5..95 (e.g. `30` → 30/70 split).

```amalgame
Element.SplitContainer("row", 30)
    .AddChild(Element.Stack().AddChild(Element.TreeView()…))  // left
    .AddChild(Element.Stack().AddChild(…))                     // right
```

Pure HTML/CSS structure (`<div class=amc-split>` with two pane
divs and a divider) plus a small JS bridge that handles the
pointer drag — `pointerdown` on `.amc-split-divider` →
`pointermove` adjusts the panes' flex grow values → `pointerup`
ends the drag. Min ratio is clamped 5..95 to avoid one pane
collapsing entirely.

### Added — `Dialog.OpenFileContent` (FileReader)

Same UX as `Dialog.OpenFile` but the handler receives the file's
contents alongside the name as a JSON object string:

```amalgame
Dialog.OpenFileContent(win, ".txt,.json", false, (payload: string) => {
    // payload = "" on cancel, otherwise:
    // {"name":"notes.txt","content":"Hello, file…"}
    …
})
```

`accept` is the input filter (extensions or MIME types). The
`binary` boolean switches the reader between `readAsText`
(strings) and `readAsDataURL` with the prefix stripped
(base64-encoded payload — decode with `String_FromBase64` or
similar in your handler).

### spike_v008

A standalone spike (`tests/spike_v008.am`) lays out a full
IDE-shell demo:
- top `MenuBar` (File / Edit / View)
- middle `SplitContainer` 30/70 (TreeView left, editor right)
- right pane wrapped in a `ContextMenu` for right-click
- File > Open uses `OpenFileContent` to load a file into the
  status label
- File > Save uses `SaveFile` to dump a sample text
- bottom `StatusStrip`

Run: `bash tests/build_spike.sh spike_v008 && ./build/spike_v008`.

## [v0.0.7] — 2026-05-16

### Added — `Form` + `Application.Run`

WinForms-style entry point. `Form` is a fluent value-holder that
bundles title + size + body + lifecycle; `Application.Run(form)`
expands it into the full `Window + Page + ApplyTo + Run + Destroy`
boilerplate.

```amalgame
let f: Form = new Form("My App", 800, 600)
f.SetBody(Element.Stack()
    .AddChild(Element.Heading("Welcome"))
    .AddChild(Element.Button("Quit").OnClick((req) => "")))
Application.Run(f)
```

Plain value rather than a base class to subclass — AM's static
dispatch makes virtual overrides in a parent class unreliable.
`OnLoad(handler)` registers a closure to fire after the window
renders (use for late `Window.Bind` calls); `SetTheme(mode)`
forwards to `Page.SetTheme`; `SetDebug(true)` opens the window
with DevTools enabled.

A `tests/spike_app_form.am` smoke test mirrors the v0.0.5
spike_form.am with the new shorter shape.

### Added — `TreeView` builder

Hierarchical tree of nodes built from HTML5 `<details>`/`<summary>`
primitives — native expand/collapse + keyboard navigation for free.

```amalgame
Element.TreeView()
    .AddChild(Element.TreeNode("src")
        .AddChild(Element.TreeNode("parser")
            .AddChild(Element.TreeLeaf("ast.am"))
            .AddChild(Element.TreeLeaf("parser.am")))
        .AddChild(Element.TreeLeaf("main.am")))
    .AddChild(Element.TreeLeaf("README.md"))
```

Three builders: `TreeView()` for the root, `TreeNode(caption)` for
folder-like expandable nodes, `TreeLeaf(caption)` for terminal
items. Pass `.Attr("open","open")` on a `TreeNode` to render it
expanded by default.

Baseline CSS themes the rotating ▶ caret, hover bg, and 18px
nested indent — all via `--amc-*` variables.

### Added — `Dialog.OpenFile` / `Dialog.SaveFile`

File pickers leveraging the browser's native UI:

- `Dialog.OpenFile(win, accept, handler)` — triggers a hidden
  `<input type=file>` click. The handler receives the chosen
  filename (or `""` on cancel). `accept` filters the dialog
  (e.g. `".png,.jpg"` or `"image/*"`).
- `Dialog.SaveFile(win, filename, content, mimeType, handler)` —
  builds a `Blob` from `content`, wires it to a download anchor,
  and triggers the OS Save-As dialog. The handler fires with
  `"ok"` once the download is offered.

Browser sandbox limits: `OpenFile` exposes only the filename,
not the full path or content (file content via FileReader is on
the roadmap for v0.0.8). `SaveFile` succeeds as soon as the
download is initiated — there's no way to know if the user
accepted the Save-As dialog or where the file landed.

### Gallery

Added a 6th tab "v0.0.7" to `spike_gallery` exercising the
TreeView (nested project layout) + the two file dialogs.

## [v0.0.6] — 2026-05-16

### Added — `Dialog` class (modal message boxes)

Equivalent to WinForms' `MessageBox.Show`. Five static entry
points cover the common combinations of urgency + buttons:

- `Dialog.Info(win, title, message, handler)` — single OK
- `Dialog.Warning(win, title, message, handler)` — single OK with
  warning-orange accent on the header
- `Dialog.Error(win, title, message, handler)` — single OK with
  red accent on the header
- `Dialog.Confirm(win, title, message, handler)` — OK / Cancel
- `Dialog.YesNoCancel(win, title, message, handler)` — Yes / No / Cancel

Plus the lower-level `Dialog.Show(win, kind, title, message,
buttons, handler)` if you want to mix-and-match (`kind` ∈
{info,warning,error}, `buttons` ∈ {ok,ok-cancel,yes-no,
yes-no-cancel}).

The handler receives the clicked button's id as the `req`
string — `"ok"`, `"cancel"`, `"yes"`, `"no"`, or `"cancel"` from
the native Esc-key dismissal. Single-OK dialogs deliver `"ok"`
on dismiss; pressing Esc on any dialog delivers `"cancel"`.

Implementation: each call appends a `<dialog id="amc-dialog">`
to the document body via `Window.Eval`, wires `onclick` handlers
on the buttons that close the dialog with their id and forward
to the bound `_amc_dialog_result` closure. The HTML `<dialog>`
element handles the focus trap + Esc + backdrop natively.
Single-slot for now — opening a new dialog while one is visible
replaces the previous one. Sufficient for v0.0.6; stackable
dialogs are a v0.0.7 candidate if real apps need them.

Baseline CSS adds themed rules for `dialog.amc-dialog` (border,
radius, shadow, body padding) + the three accent variants for
the header. The `::backdrop` scrim is darkened to make the
modal stand out. All colors reference `--amc-*` variables, so
light/dark flip is automatic.

`tests/spike_gallery.am` gains a "Dialogs" tab that opens each
of the five variants and echoes the result into a `<pre>` via
`Window.SetInnerHtml`.

## [v0.0.5] — 2026-05-15

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
