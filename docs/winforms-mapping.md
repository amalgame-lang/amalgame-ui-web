# WinForms → `amalgame-ui-web` mapping

Cross-reference between the Visual Studio WinForms Toolbox and the
`amalgame-ui-web` builder API. Reflects the state through **v0.0.10**.

> **OS theming guarantee.** Every shipped widget participates in the
> baseline CSS variables (`--amc-bg`, `--amc-fg`, `--amc-border`,
> `--amc-surface`, `--amc-accent`, `--amc-muted`, `--amc-radius`) and
> flips automatically with the OS color scheme via
> `[data-theme=dark]`. Live OS-theme tracking is opt-in via
> `Page.AutoTheme(true)`.

## Status legend

| Marker | Meaning |
|---|---|
| ✅ **Disponible (fait)** — `vX.Y.Z` | Shipped and stable from the listed release onward |
| 🟡 **À venir** — `vX.Y.Z+` | On the short-term backlog (1-2 releases out) |
| 🔵 **Reporté** — `v0.1.0+` / `v0.2.x` | Deferred: needs OS-native bindings or a heavy JS lib |
| ❌ **Hors scope** | Not relevant in a webview / WinForms-specific concept |

---

## 1. Common Controls (Toolbox)

| WinForms          | `Element.*`                                | Status |
|-------------------|--------------------------------------------|--------|
| `Button`          | `Button(text)`                             | ✅ **Disponible (fait)** — v0.0.3 |
| `CheckBox`        | `CheckBox(name)` + `CheckBoxLabel(name, caption)` | ✅ **Disponible (fait)** — v0.0.4 / .5 (wrapped variant) |
| `CheckedListBox`  | `CheckedListBox(name)` + `CheckedItem(name, value, label)` | ✅ **Disponible (fait)** — v0.0.5 |
| `ComboBox`        | `Select(name)` + `Option(value, label)`    | ✅ **Disponible (fait)** — v0.0.4 |
| `DateTimePicker`  | `DatePicker(name)` + `TimePicker(name)`    | ✅ **Disponible (fait)** — v0.0.5 (split per HTML5 type) |
| `Label`           | `Label(text)`                              | ✅ **Disponible (fait)** — v0.0.3 |
| `LinkLabel`       | `Link(text, url)`                          | ✅ **Disponible (fait)** — v0.0.5 (opens in OS browser) |
| `ListBox`         | `ListBox(name, size)`                      | ✅ **Disponible (fait)** — v0.0.5 (multi-select) |
| `ListView`        | `ListView(headers, bodyId)` + `ListViewRow(values)` | ✅ **Disponible (fait)** — v0.0.5 |
| `MaskedTextBox`   | `MaskedTextBox(name, pattern, mode)`       | ✅ **Disponible (fait)** — v0.0.5 |
| `MonthCalendar`   | `MonthCalendar(name, year, month)`         | ✅ **Disponible (fait)** — v0.0.9 (navigator added v0.0.10) |
| `NotifyIcon`      | —                                          | 🔵 **Reporté** — v0.2.x (system tray = OS native) |
| `NumericUpDown`   | `Number(name, min, max, step)`             | ✅ **Disponible (fait)** — v0.0.5 (composite ▲/▼ buttons) |
| `PictureBox`      | `PictureBox(src)` (alias of `Image`)       | ✅ **Disponible (fait)** — v0.0.5 |
| `ProgressBar`     | `ProgressBar(value, max)`                  | ✅ **Disponible (fait)** — v0.0.5 |
| `RadioButton`     | `Radio(name, value)` + `RadioLabel(name, value, caption)` | ✅ **Disponible (fait)** — v0.0.4 / .5 (wrapped variant) |
| `RichTextBox`     | `RichTextBox(name)`                        | ✅ **Disponible (fait)** — v0.0.9 |
| `TextBox`         | `Input(name)` / `Textarea(name)` / `Password(name)` | ✅ **Disponible (fait)** — v0.0.4 / .5 |
| `ToolTip`         | `.Tooltip(text)`                           | ✅ **Disponible (fait)** — v0.0.5 (instance method) |
| `TreeView`        | `TreeView()` + `TreeNode(label)` + `TreeLeaf(label)` | ✅ **Disponible (fait)** — v0.0.7 |
| `WebBrowser`      | `Iframe(url)`                              | ✅ **Disponible (fait)** — v0.0.5 |

**Bonus primitives (no direct WinForms counterpart):**

| `Element.*` | First release | Notes |
|---|---|---|
| `Image(src)` | v0.0.5 | `<img>` — accepts file://, https://, data:image/… |
| `Slider(name, min, max, step)` | v0.0.5 | `<input type=range>` |
| `ColorPicker(name)` | v0.0.5 | Inline color picker |
| `Heading(text)` | v0.0.3 | `<h1>` |
| `Pre(text)` | v0.0.3 | Monospace output panel |

---

## 2. Containers (Toolbox)

| WinForms            | `Element.*`                                       | Status |
|---------------------|---------------------------------------------------|--------|
| `FlowLayoutPanel`   | `Flow(direction)`                                 | ✅ **Disponible (fait)** — v0.0.5 |
| `GroupBox`          | `GroupBox(title)`                                 | ✅ **Disponible (fait)** — v0.0.5 |
| `Panel`             | `Panel()` (alias of `Div`)                        | ✅ **Disponible (fait)** — v0.0.5 |
| `SplitContainer`    | `SplitContainer(orientation, initialRatio)`       | ✅ **Disponible (fait)** — v0.0.8 (drag-resizable) |
| `TabControl`        | `TabControl(group)` + `Tab(group, id, label, body)` | ✅ **Disponible (fait)** — v0.0.5 |
| `TableLayoutPanel`  | `Grid(rows, cols, gap)`                           | ✅ **Disponible (fait)** — v0.0.5 |

**Bonus containers:**

| `Element.*` | First release | Notes |
|---|---|---|
| `Stack()` | v0.0.3 | Flex column with 8px gap |
| `Row()` | v0.0.3 | Flex row with 8px gap |
| `Div()` | v0.0.3 | Generic block container |
| `AbsoluteContainer()` | v0.0.5 | `position:relative` parent for `.Position(x, y)` children |

---

## 3. Menus & Toolbars (Toolbox)

| WinForms             | `Element.*`                                              | Status |
|----------------------|----------------------------------------------------------|--------|
| `ContextMenuStrip`   | `ContextMenu(forId)` + `MenuOption` / `MenuSeparator`     | ✅ **Disponible (fait)** — v0.0.8 (right-click anchored at cursor) |
| `MenuStrip` (HTML)   | `MenuBar()` + `MenuItem` + `MenuOption` + `MenuSeparator` | ✅ **Disponible (fait)** — v0.0.8 (HTML/CSS path) |
| `MenuStrip` (native) | `Element.MenuBar().UseNative(true)` (marker today)        | 🔵 **Reporté** — v0.1.0 (Win32 + NSMenu + GtkMenuBar bridge) |
| `StatusStrip`        | `StatusStrip()`                                           | ✅ **Disponible (fait)** — v0.0.5 (fixed-bottom `<footer>`) |
| `ToolStrip`          | `ToolStrip()`                                             | ✅ **Disponible (fait)** — v0.0.5 (themed button row) |
| `ToolStripContainer` | —                                                        | ❌ **Hors scope** — WinForms docking, not webview-meaningful |

---

## 4. Data (Toolbox)

| WinForms             | `Element.*`              | Status |
|----------------------|--------------------------|--------|
| `BindingNavigator`   | —                        | 🟡 **À venir** — façade around `ToolStrip` (pagination), low priority |
| `BindingSource`      | —                        | ❌ **Hors scope** — .NET-runtime data binding |
| `DataGridView`       | —                        | 🔵 **Reporté** — v0.0.11+ (heavy widget, likely wrap Tabulator / AG-Grid JS) |
| `DataSet`            | —                        | ❌ **Hors scope** — .NET DataTable concept |

---

## 5. Components (non-visual)

These belong to other Amalgame packages or core stdlib — not to a UI binding.

| WinForms             | Alternative                            | Status |
|----------------------|----------------------------------------|--------|
| `BackgroundWorker`   | `Process.Run` / `Service` package      | ✅ **Disponible (fait)** — existing stdlib / package |
| `DirectoryEntry`     | —                                      | ❌ **Hors scope** — Active Directory |
| `DirectorySearcher`  | —                                      | ❌ **Hors scope** — Active Directory |
| `ErrorProvider`      | Inline `<span class=error>` next to input | ✅ **Disponible (fait)** — design pattern |
| `FileSystemWatcher`  | `amalgame-io-filewatcher` package      | ✅ **Disponible (fait)** — existing package |
| `HelpProvider`       | —                                      | 🟡 **À venir** — could map to `Tooltip` plus a help URL |
| `ImageList`          | Plain `List<string>` of URLs           | ✅ **Disponible (fait)** — trivial |
| `PerformanceCounter` | —                                      | ❌ **Hors scope** — system metrics |
| `Process`            | `Process` stdlib                       | ✅ **Disponible (fait)** — stdlib |
| `SerialPort`         | `@c` block / native bindings           | external / **Hors scope** for ui-web |
| `ServiceController`  | —                                      | ❌ **Hors scope** — Windows services |
| `SoundPlayer`        | `Element.Raw("<audio>…</audio>")` today | 🟡 **À venir** — typed `Element.Audio(src)` builder |
| `Timer`              | Amalgame event loop / `setTimeout` from JS side | ✅ **Disponible (fait)** — patterns |

---

## 6. Printing (Toolbox)

| WinForms                 | `Element.*` / `Window.*`            | Status |
|--------------------------|--------------------------------------|--------|
| `PrintDialog`            | `window.print()` via `Window.Eval`   | 🟡 **À venir** — v0.1.0 (small builder around the JS print API) |
| `PrintPreviewDialog`     | CSS `@media print`                   | 🟡 **À venir** — v0.1.0 |
| `PageSetupDialog`        | —                                    | 🔵 **Reporté** |
| `PrintDocument`          | —                                    | 🔵 **Reporté** |
| `PrintPreviewControl`    | —                                    | 🔵 **Reporté** |

---

## 7. Dialogs (Toolbox)

| WinForms                 | API                                       | Status |
|--------------------------|-------------------------------------------|--------|
| `MessageBox.Show`        | `Dialog.Info` / `Warning` / `Error` / `Confirm` / `YesNoCancel` / `Show` | ✅ **Disponible (fait)** — v0.0.6 |
| `ColorDialog` (inline)   | `Element.ColorPicker(name)`               | ✅ **Disponible (fait)** — v0.0.5 |
| `OpenFileDialog`         | `Dialog.OpenFile(win, accept, handler)`   | ✅ **Disponible (fait)** — v0.0.7 |
| `OpenFileDialog` + read  | `Dialog.OpenFileContent(win, accept, binary, handler)` | ✅ **Disponible (fait)** — v0.0.8 (FileReader bridge) |
| `SaveFileDialog`         | `Dialog.SaveFile(win, filename, content, mime, handler)` | ✅ **Disponible (fait)** — v0.0.7 |
| `FontDialog`             | —                                         | 🔵 **Reporté** — no HTML standard, low priority |
| `FolderBrowserDialog`    | `webkitdirectory` (partial) or native     | 🔵 **Reporté** — v0.1.0 (needs C glue) |

---

## 8. Reporting (Toolbox)

| WinForms                 | Decision                | Notes |
|--------------------------|-------------------------|-------|
| `ReportViewer`           | ❌ **Hors scope**       | Proprietary MS (SSRS / RDLC) |

---

## 9. WPF Interoperability

| WinForms                 | Decision                | Notes |
|--------------------------|-------------------------|-------|
| `ElementHost`            | ❌ **Hors scope**       | WPF interop, irrelevant in a webview |

---

## 10. Visual Basic PowerPacks (Shapes)

| WinForms                 | API today                                       | Status |
|--------------------------|--------------------------------------------------|--------|
| `LineShape` / `OvalShape` / `RectangleShape` / `ShapeContainer` | `Element.Raw("<svg>…</svg>")` works today | 🟡 **À venir** — typed `Element.Svg(...)` builder, v0.0.11+ |

---

## Page-level features

| Method                            | First release | Effect |
|-----------------------------------|---------------|--------|
| `Page.FillViewport()`             | v0.0.5 (default since v0.0.5b) | Body becomes a 100vh flex column; first child grows |
| `Page.NaturalFlow()`              | v0.0.5b       | Opt-out of FillViewport |
| `Page.FullBleed()`                | v0.0.8        | `body { padding: 0 }` for IDE-shell layouts |
| `Page.AutoTheme(b)`               | v0.0.8        | matchMedia listener + 1.5 s polling fallback for live OS-theme tracking |
| `Page.OnThemeChange(handler)`     | v0.0.8        | AM callback when the OS theme flips |
| `Page.SetTheme(mode)`             | v0.0.4        | `"auto"` / `"light"` / `"dark"` |
| `Page.PatchInner(win, id, e)`     | v0.0.5        | Partial-DOM update + binding accumulation |
| `Page.AppendInner(win, id, e)`    | v0.0.5        | Append a rendered Element to `#id` |
| `Window.SetInnerHtml(id, html)`   | v0.0.5        | Raw replace |
| `Window.AppendHtml(id, html)`     | v0.0.5        | Raw append |
| `Window.RemoveElement(id)`        | v0.0.5        | Drop by id |
| `Form` + `Application.Run(form)`  | v0.0.7        | WinForms-style entry point — collapses `Window + Page + ApplyTo + Run + Destroy` |

---

## Element-level common properties

Every `Element` carries these (mirror of the WinForms Properties pane):

| Method                  | WinForms equivalent | Notes |
|-------------------------|---------------------|-------|
| `.SetText(t)`           | `.Text`             | Inner text content |
| `.Id(id)`               | `.Name`             | HTML id |
| `.Class(cls)`           | (CSS class)         | Multi-class via space-separated string; multi-call replaces (Attr dedupe since v0.0.8) |
| `.Style(css)`           | (inline style)      | Cumulative across calls |
| `.Attr(k, v)`           | (raw HTML attr)     | Replaces on duplicate key |
| `.Size(w, h)`           | `.Size`             | px; 0 = leave unset |
| `.Position(x, y)`       | `.Location`         | Absolute positioning |
| `.Visible(b)`           | `.Visible`          | `display:none` when false |
| `.Enabled(b)`           | `.Enabled`          | `disabled` attr + dimmed |
| `.Tooltip(text)`        | `.ToolTip`          | HTML `title` |
| `.TabIndex(n)`          | `.TabIndex`         | Keyboard focus order |
| `.ForeColor(css)`       | `.ForeColor`        | Shorthand for `color:` |
| `.BackColor(css)`       | `.BackColor`        | Shorthand for `background:` |
| `.Font(family, sizePx)` | `.Font`             | Shorthand for `font-family/size` |
| `.DataTag(payload)`     | `.Tag`              | `data-tag` round-trip |
| `.Fill()`               | (CSS flex grow)     | Mark this child as the flex-grow / scroll target |
| `.Bind(name)`           | (form key)          | Declarative `name` attr |
| `.Raw(html)`            | (escape hatch)      | Append raw HTML chunk |

---

## Event model (uniform `On(eventName, handler)`)

| Sugar setter         | DOM event       | WinForms equivalent |
|----------------------|-----------------|---------------------|
| `.OnClick(h)`        | `click`         | `Click` |
| `.OnDblClick(h)`     | `dblclick`      | `DoubleClick` |
| `.OnChange(h)`       | `change` + `input` | `TextChanged` / `CheckedChanged` / `SelectedIndexChanged` |
| `.OnFocus(h)`        | `focus`         | `GotFocus` / `Enter` |
| `.OnBlur(h)`         | `blur`          | `LostFocus` / `Leave` |
| `.OnMouseEnter(h)`   | `mouseenter`    | `MouseEnter` |
| `.OnMouseLeave(h)`   | `mouseleave`    | `MouseLeave` |
| `.OnKeyDown(h)`      | `keydown`       | `KeyDown` |
| `.OnKeyUp(h)`        | `keyup`         | `KeyUp` |
| `.On(name, h)`       | (any)           | Custom event name |
| `.OnResult(targetId)`| (result routing)| Route the handler's return into `#targetId` |

---

## What an app port from `amalgame-ui-forms` keeps / loses

**Keeps everything** in §1–§3 + §7 marked ✅ above (~25 widgets + the
common properties + the uniform event model + the form-state
auto-collect + the modal Dialog suite + the partial-DOM update API).

**Loses** only:

- `MenuStrip` **OS-native** look (the HTML version is in — opt in
  to `data-mode="native"` via `Element.MenuBar().UseNative(true)`
  to auto-switch when the v0.1.0 native bridge lands).
- `NotifyIcon` (system tray) — v0.2.x.
- `DataGridView` — v0.0.11+, will wrap a JS lib.
- `FontDialog` — no HTML standard, parked.
- `FolderBrowserDialog` — v0.1.0 (needs C glue).

Three of those (`MenuStrip` native, `NotifyIcon`, `FolderBrowserDialog`)
are explicitly tracked as v0.1.x / v0.2.x line items in the main
roadmap.

---

## Theming responsibility

Every shipped widget:

1. References `--amc-*` CSS variables only (never hardcodes a color).
2. Inherits `color` and `background` from the body when possible.
3. Carries an `.amc-<kind>` root class so the baseline can target it.
4. Renders correctly under both `[data-theme=light]` and `[data-theme=dark]`.

Override individual variables from your own stylesheet:

```css
:root { --amc-accent: #ff6b00; --amc-radius: 8px }
```

or via `Page.AddCss("file:///abs/path/overrides.css")`.
