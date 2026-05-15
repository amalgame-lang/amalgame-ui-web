# WinForms → `amalgame-ui-web` mapping

Cross-reference between the Visual Studio WinForms Toolbox and the
`amalgame-ui-web` builder API. Drives the scope of `v0.0.x` releases
and documents which widgets are deferred or out of scope.

> **OS theming guarantee.** Every shipped widget participates in the
> baseline CSS variables (`--amc-bg`, `--amc-fg`, `--amc-border`,
> `--amc-surface`, `--amc-accent`, `--amc-muted`, `--amc-radius`) and
> flips automatically with the OS color scheme via
> `[data-theme=dark]`. Custom widgets only need to reference the
> variables — no per-theme stylesheet needed.

Legend
- ✅ Shipped
- 🟢 Planned for **v0.0.5** (this release)
- 🟡 Planned for **v0.0.6 / v0.0.7** (next releases)
- 🔵 Deferred to **v0.1.0+** (needs native bindings or a heavy JS lib)
- ❌ Not relevant in a webview (out of scope)

---

## 1. Common Controls

| WinForms          | `Element.*`              | Status   | Notes |
|-------------------|--------------------------|----------|-------|
| `Button`          | `Button(text)`           | ✅ v0.0.3 | |
| `CheckBox`        | `CheckBox(name)`         | ✅ v0.0.4 | |
| `CheckedListBox`  | `CheckedListBox(name)`   | 🟢       | `<ul>` of `<input type=checkbox>` items |
| `ComboBox`        | `Select(name)`           | ✅ v0.0.4 | |
| `DateTimePicker`  | `DatePicker(name)` + `TimePicker(name)` | ✅ v0.0.5 | HTML5 splits date/time |
| `Label`           | `Label(text)`            | ✅ v0.0.3 | |
| `LinkLabel`       | `Link(text, url)`        | ✅ v0.0.5 | |
| `ListBox`         | `ListBox(name, size)`    | ✅ v0.0.5 | multi-select |
| `ListView`        | `ListView(headers)`      | 🟢       | `<table>` with header row |
| `MaskedTextBox`   | `MaskedTextBox(name, pattern, mode)` | 🟢 | HTML5 `pattern` + `inputmode` |
| `MonthCalendar`   | —                        | 🔵       | No HTML5 inline equivalent; needs JS+CSS (~1-2 days). `DatePicker` covers 90%. |
| `NotifyIcon`      | —                        | 🔵       | System tray = OS native, **v0.2.x** |
| `NumericUpDown`   | `Number(name, min, max, step)` | ✅ v0.0.5 | |
| `PictureBox`      | `PictureBox(src)` (alias of `Image`) | 🟢 | |
| `ProgressBar`     | `ProgressBar(value, max)` | ✅ v0.0.5 | |
| `RadioButton`     | `Radio(name, value)`     | ✅ v0.0.4 | |
| `RichTextBox`     | —                        | 🟡       | `contenteditable` div, needs API design — v0.0.7 |
| `TextBox`         | `Input(name)` / `Textarea(name)` | ✅ v0.0.4 | |
| `ToolTip`         | `.Tooltip(text)`         | 🟢       | Instance method, sets HTML `title` |
| `TreeView`        | —                        | 🟡       | `<details>` recursion, needs expand/select model — v0.0.6 |
| `WebBrowser`      | `Iframe(url)`            | 🟢       | Embed page in page; full webview IS what ui-web is |

---

## 2. Containers

| WinForms            | `Element.*`              | Status   | Notes |
|---------------------|--------------------------|----------|-------|
| `FlowLayoutPanel`   | `Flow(direction)`        | 🟢       | flex with wrap |
| `GroupBox`          | `GroupBox(title)`        | 🟢       | `<fieldset><legend>` native |
| `Panel`             | `Panel()` (alias of `Div`) | 🟢      | |
| `SplitContainer`    | —                        | 🟡       | CSS `resize` partial; full split with JS — v0.0.6 |
| `TabControl`        | `TabControl(idPrefix)` + `Tab(title, body)` | 🟢 | Pure-CSS radio pattern, no JS needed |
| `TableLayoutPanel`  | `Grid(rows, cols, gap)`  | ✅ v0.0.5 | |

---

## 3. Menus & Toolbars

| WinForms             | `Element.*`              | Status   | Notes |
|----------------------|--------------------------|----------|-------|
| `ContextMenuStrip`   | —                        | 🟡       | JS `oncontextmenu` + dropdown — v0.0.6 |
| `MenuStrip`          | —                        | 🔵       | App menubar, **native v0.1.0** (Win32 + NSMenu + GtkMenuBar) |
| `StatusStrip`        | `StatusStrip()`          | 🟢       | `<footer>` fixed bottom |
| `ToolStrip`          | `ToolStrip()` (themed `Row()`) | 🟢 | |
| `ToolStripContainer` | —                        | ❌       | WinForms-specific docking, not webview-meaningful |

---

## 4. Data

| WinForms             | `Element.*`              | Status   | Notes |
|----------------------|--------------------------|----------|-------|
| `BindingNavigator`   | —                        | 🟡       | Pagination toolbar, façade around `ToolStrip` — v0.0.7 |
| `BindingSource`      | —                        | ❌       | .NET-runtime data binding, Amalgame handles differently |
| `DataGridView`       | —                        | 🔵       | Editable typed-column grid. Heavy (~1 week); likely wrap AG-Grid / Tabulator JS — v0.0.7+ |
| `DataSet`            | —                        | ❌       | .NET DataTable concept, out of scope |

---

## 5. Non-visual components

These belong to other Amalgame packages or core stdlib — not to a UI
binding.

| WinForms             | Alternative              | Status   |
|----------------------|--------------------------|----------|
| `BackgroundWorker`   | `Process.Run` / `Service` package | ✅ existing |
| `FileSystemWatcher`  | `amalgame-io-filewatcher` package | ✅ existing |
| `Timer`              | Amalgame event loop / `setTimeout` from JS side | ✅ |
| `Process`            | `Process` stdlib         | ✅       |
| `SoundPlayer`        | HTML5 `<audio>` via `Element.Audio` | 🟡 v0.0.7 |
| `SerialPort`         | `@c` block / native bindings | external |
| `ErrorProvider`      | Inline `<span class=error>` next to input | 🟢 design idiom |
| `ImageList`          | Plain `List<string>` of URLs | ✅ trivial |
| Others (AD / Help / Perf) | Not GUI; ignore here | ❌ |

---

## 6. Printing

| WinForms                 | `Element.*` / `Window.*`  | Status   |
|--------------------------|---------------------------|----------|
| `PrintDialog`            | `window.print()` JS       | 🟡 v0.1.0 |
| `PrintPreviewDialog`     | CSS `@media print`        | 🟡 v0.1.0 |
| `PageSetupDialog`        | —                         | 🔵 |
| `PrintDocument`          | —                         | 🔵 |
| `PrintPreviewControl`    | —                         | 🔵 |

---

## 7. Dialogs

| WinForms                 | `Element.*` / `Window.*`  | Status   |
|--------------------------|---------------------------|----------|
| `ColorDialog`            | `Element.ColorPicker(name)` (inline) | ✅ v0.0.5 |
| `FontDialog`             | —                         | 🔵 — no HTML standard, low priority |
| `OpenFileDialog`         | `<input type=file>` or `window.showOpenFilePicker()` | 🟡 v0.0.6 |
| `SaveFileDialog`         | `<a download>` or showSaveFilePicker | 🟡 v0.0.6 |
| `FolderBrowserDialog`    | `webkitdirectory` (partial) or native | 🔵 v0.1.0 — needs C glue |

---

## 8. Reporting / WPF Interop / Shapes

| WinForms                 | Decision  | Notes |
|--------------------------|-----------|-------|
| `ReportViewer`           | ❌        | Proprietary MS (SSRS/RDLC) |
| `ElementHost`            | ❌        | WPF interop, irrelevant |
| `LineShape` / `OvalShape` / `RectangleShape` / `ShapeContainer` | 🟡 | HTML5 SVG — design `Element.Svg(...)` if requested — v0.0.7+ |

---

## v0.0.5 shipping target (all-greens)

The widgets shipped in v0.0.5 cover **all `✅` + `🟢` rows above**:

- Already in facade (✅): Button, CheckBox, ComboBox, DateTimePicker
  (= DatePicker + TimePicker), Label, LinkLabel, ListBox, NumericUpDown,
  ProgressBar, RadioButton, TextBox (Input + Textarea), ColorDialog
  (ColorPicker), TableLayoutPanel (Grid), Slider, Password, Image,
  AbsoluteLayout, StackLayout.

- Added in v0.0.5 (🟢): CheckedListBox, ListView, MaskedTextBox,
  PictureBox (Image alias), Tooltip, Iframe (WebBrowser),
  FlowLayoutPanel (Flow), GroupBox, Panel (Div alias), TabControl,
  StatusStrip, ToolStrip, ErrorProvider idiom.

After v0.0.5 ships, an app port from `amalgame-ui-forms` only loses
access to `MenuStrip`, `NotifyIcon` (system tray), `MonthCalendar`
inline, and `DataGridView`. Three of those are explicitly planned
for v0.1.x.

---

## Theming responsibility

Every new widget in v0.0.5 must:

1. Reference baseline CSS variables (`--amc-*`) for any color it
   defines — never hardcode `#hex`.
2. Inherit `color` and `background` from the body when possible.
3. Add a `.amc-<kind>` class to its root if it needs class-targeted
   styles; the baseline declares those alongside the existing
   `button`/`input`/`select` rules.
4. Pass the `dump_html` smoke test under both `data-theme=light`
   and `data-theme=dark`.

The baseline stylesheet grew the following rule blocks in v0.0.5
to cover the new widgets:

```css
fieldset { border, padding, --amc-border }
legend   { color: --amc-muted }
footer.amc-statusstrip { background: --amc-surface; border-top: --amc-border }
table.amc-listview { border-collapse, th/td borders --amc-border }
.amc-tabs > input[type=radio]:checked + label { background: --amc-accent }
details.amc-treenode > summary { cursor, --amc-fg }
iframe { border: 1px solid --amc-border; background: --amc-bg }
```
