# `amalgame-ui-web` — architecture & extension model

How HTML / CSS / JS is organized in `amalgame-ui-web`, and how a
Amalgame developer extends the toolkit with custom widgets and
full-tuning escape hatches. Pairs with [`winforms-mapping.md`](winforms-mapping.md)
which inventories the widget catalogue.

---

## Three-level model

```
┌──────────────────────────────────────────────────────────────┐
│ LEVEL 3 — Form                                                │
│ Top-level window holder. Drives Window + Page underneath.     │
│ Lifecycle (OnLoad), title / size / theme / debug, body root.  │
│ Equivalent: WinForms `Form`. Status: ✅ shipped v0.0.7.        │
└─────────────────────────┬────────────────────────────────────┘
                          │ contains
┌─────────────────────────▼────────────────────────────────────┐
│ LEVEL 2 — Component  (composite, custom widget)               │
│ Plain Amalgame class with `Render(): Element`. No abstract    │
│ base — AM static dispatch makes parent-virtual overrides      │
│ unreliable, so a convention reads better and runs more        │
│ predictably. Encapsulates state + rendering for re-usable     │
│ widgets a dev ships in a personal package.                    │
│ Status: ✅ documented + canonical from v0.0.9.                 │
└─────────────────────────┬────────────────────────────────────┘
                          │ Render() returns
┌─────────────────────────▼────────────────────────────────────┐
│ LEVEL 1 — Element  (primitive HTML node)                      │
│ Tree of HTML elements with .Attr / .Style / .Class /          │
│ .On(event, h) / .AddChild(c). Static builders for every       │
│ widget in the WinForms toolbox (Button, Input, Grid, …).      │
│ Status: ✅ shipped from v0.0.3, extended in v0.0.4 / v0.0.5.  │
└──────────────────────────────────────────────────────────────┘
```

Most apps live in **Level 1**. Bigger apps use **Level 2** for
re-usable components. **Level 3** is sugar around `Window + Page`
that gives a WinForms-ish entry point for porting old code.

---

## Level 1 — `Element` primitive

The tree-of-HTML-nodes builder. Every WinForms widget has an
`Element.<Widget>(...)` static builder that returns a configured
`Element` tree. The builder is fluent (`.AddChild`, `.Style`,
`.On(...)` etc. all return `this`).

```amalgame
Element.Stack()
    .AddChild(Element.Heading("Title"))
    .AddChild(Element.Input("user").Tooltip("Your full name"))
    .AddChild(Element.Button("Save")
                  .OnClick((req) => save(req))
                  .OnResult("out"))
    .AddChild(Element.Pre("").Id("out"))
```

### Common properties (every Element accepts)

These methods mirror the *Properties* pane of the WinForms designer:

| `.Method()`             | WinForms equivalent | Notes |
|-------------------------|---------------------|-------|
| `.SetText(t)`           | `.Text`             | inner text content |
| `.Id(id)`               | `.Name`             | HTML id |
| `.Class(cls)`           | (CSS class)         | space-sep accepted |
| `.Style(css)`           | (inline style)      | cumulative |
| `.Attr(k, v)`           | (raw HTML attr)     | escape hatch |
| `.Size(w, h)`           | `.Size`             | px; 0 = leave unset |
| `.Position(x, y)`       | `.Location`         | absolute positioning |
| `.Visible(b)`           | `.Visible`          | toggles `display:none` |
| `.Enabled(b)`           | `.Enabled`          | `disabled` attr + dimmed |
| `.Tooltip(text)`        | `.ToolTip`          | sets HTML `title` |
| `.TabIndex(n)`          | `.TabIndex`         | keyboard focus order |
| `.ForeColor(css)`       | `.ForeColor`        | shorthand for `color:` |
| `.BackColor(css)`       | `.BackColor`        | shorthand for `background:` |
| `.Font(family, sizePx)` | `.Font`             | shorthand for `font-family/size` |
| `.Tag(data)`            | `.Tag`              | `data-tag` attr; round-trips through JS |

### Events (uniform `On(event, handler)` model)

Every WinForms event has an HTML DOM event equivalent. The same
JSON form-state payload is delivered to every handler (via the
auto-injected `window.__amc_collect` bridge — see Level 0 below).
`OnResult(targetId)` applies to all handlers on an Element.

| `.On*(handler)`         | DOM event       | WinForms equivalent |
|-------------------------|-----------------|---------------------|
| `.OnClick(h)`           | `click`         | `Click`             |
| `.OnDblClick(h)`        | `dblclick`      | `DoubleClick`       |
| `.OnChange(h)`          | `change`+`input`| `TextChanged` / `CheckedChanged` / `SelectedIndexChanged` |
| `.OnFocus(h)`           | `focus`         | `GotFocus` / `Enter` |
| `.OnBlur(h)`            | `blur`          | `LostFocus` / `Leave` / `Validated` |
| `.OnMouseEnter(h)`      | `mouseenter`    | `MouseEnter`        |
| `.OnMouseLeave(h)`      | `mouseleave`    | `MouseLeave`        |
| `.OnKeyDown(h)`         | `keydown`       | `KeyDown`           |
| `.OnKeyUp(h)`           | `keyup`         | `KeyUp`             |
| `.On(name, h)`          | (any)           | (custom event name) |

Internally, the bookkeeping is a single `List<(eventName, handler,
resultTargetId)>` per Element. The explicit setters are sugar over
`On(name, h)`. The custom form is the extension point — a Component
package can hook arbitrary DOM events (`mouseup`, `wheel`, `drop`,
…) without facade changes.

### Escape hatch: raw HTML / JS

For one-off needs that don't justify a builder:

```amalgame
Element.Div().Raw("<svg><circle r=20 cx=20 cy=20 fill=red/></svg>")
Element.Button("Go").On("custom-event", h)
Page.AddCss("file:///abs/styles/app.css")    // ✅ since v0.0.4
Window.Eval("document.body.classList.add('compact')")
```

A `Page.AddJs(url)` / `Page.RawHead(meta)` pair is planned for
v0.1+ — until then, drive external JS via `Window.Eval` (or a
`Window.Init` for pre-navigation injection).

---

## Level 2 — `Component` composite

When a widget tree repeats across screens (login form, settings panel,
data row), promote it to a reusable `Component`. Status: 🟡 v0.0.6.

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
let last:  LabeledInput = new LabeledInput("Last name:",  "last")

Element.Stack()
    .AddChild(first.Render())
    .AddChild(last.Render())
```

Components ship as **regular Amalgame packages**:

```
amalgame package add my-team-widgets
```

Their facade exposes plain classes with a `Render()` method. No
engine support needed — `Component` is a convention, not a base
class. Consumers compose `.Render()` like any other Element
source.

amc bug-of-the-day worth knowing about: chaining
`new LabeledInput(...).Render()` in one expression used to lower
incorrectly. Fixed in amc v0.8.16; on older compilers, split via
a `let x = new LabeledInput(...)` intermediate.

---

## Level 3 — `Form` (WinForms-style entrypoint)

`Form` is a value-style wrapper around `Window + Page` that reads
like WinForms boilerplate without forcing subclass inheritance.
Status: ✅ shipped v0.0.7. Plain class, public mutable fields,
optional `OnLoad` closure — no virtual overrides to defeat.

```amalgame
class Program {
    public static void Main() {
        let f: Form = new Form("My App", 800, 600)
        f.SetTheme("auto")
        f.SetDebug(false)
        f.OnLoad((req: string) => {
            // late `Window.Bind` registrations go here
            return ""
        })
        f.SetBody(
            Element.Stack()
                .AddChild(Element.MenuBar()
                    .AddChild(Element.MenuItem("File")
                        .AddChild(Element.MenuOption("Quit", "amc_quit"))))
                .AddChild(Element.Button("Hello").OnClick((req) => "\"hi\""))
        )
        Application.Run(f)
    }
}
```

`Form` is sugar — under the hood it's the existing
`Window + Page + ApplyTo + Run + Destroy` sequence. Theme,
lifecycle hooks, and MenuBar action bindings live around it.
A future `OnClosing` hook for "save before quitting" prompts is
planned alongside the native MenuBar opt-in (v0.1.0).

---

## Level 0 — Runtime glue (`Page.ApplyTo` injection)

What the engine wires into every page automatically. The dev never
calls these directly but can override or extend them.

### `window.__amc_collect()`
Returns a JSON object of every named form field's current value.
Booleans for checkboxes, selected value for radio groups,
`.innerHTML` for `[contenteditable][name]` (RichTextBox, v0.0.9),
ISO `YYYY-MM-DD` for `.amc-monthcal[name]` (MonthCalendar,
v0.0.9). Multi-select arrays for `<select multiple>` are still
not auto-collected — read `.selectedOptions` via `Window.Eval`.
Every event handler receives `JSON.stringify(__amc_collect())`
as `req`.

### `window.__amc_route(promise, targetId)`
Awaits a handler's return promise and writes the result into
`#targetId`. Pretty-prints JSON, falls through to raw string. Used
by `Element.OnResult(targetId)`.

### Chrome lockdown
Right-click context menu + reload hotkeys (F5 / Ctrl+R / Cmd+R)
are blocked by default since `Page.SetHtml` has no URL to reload
to. Opt out per Page with `.AllowBrowserDefaults()`.

### Baseline stylesheet
A 7-variable theme (`--amc-bg`, `--amc-fg`, `--amc-border`,
`--amc-surface`, `--amc-accent`, `--amc-muted`, `--amc-radius`)
flipped by `[data-theme=dark]` on `<html>`. The dark/light value
is written from OS detection (`gsettings` / Apple / Windows
registry). Per-widget styles in the baseline use *only* the
variables — no hardcoded colors. Override one or many:

```amalgame
Page.New().AddCss("data:text/css,:root{--amc-accent:%23ff6b00}")
```

---

## CSS / JS injection layering

The page is built in this order; later layers override earlier ones:

```
1. Baseline stylesheet   ← Page.BaselineCss(), themed via --amc-*
2. Page.SetStylesheet(url)  (replaces baseline; use NoBaseline()
                             then AddCss(url) for a stack)
3. Page.AddCss(url)*       ← layered after baseline / custom main
4. <style> inside  Element.Raw(...) / .Style(s)
5. Window.Init(js)         ← injected before each navigation
6. Window.Eval(js)         ← runs once, now
```

Inline `.Style(...)` on Elements always wins because it ends up as
an attribute on the element. CSS variables remain the recommended
way to re-theme without rewriting.

---

## Extension scenarios

### "I just want a button with a different color"
Use `.BackColor(...)` / `.ForeColor(...)` or `.Style("background:#abc")`.

### "I want a custom widget I'll reuse"
Make a `Component` subclass that returns an `Element` tree. Ship in a
package or keep in your project.

### "I want to use a JS library (Tabulator, Plotly, Codemirror)"
1. `Page.AddCss(url-to-lib-css)` for the lib's stylesheet.
2. `Element.Div().Id("plot").Class("amc-plot-host")` as the mount point.
3. `Window.Eval("Plotly.newPlot('plot', ...)")` to instantiate.
4. Optional: bind a `Window.Bind` callback for events back to Amalgame.

### "I want to add an OS-native thing webview can't do"
Drop into C glue: extend `runtime/Amalgame_UI_Web.c` with a new
primitive and expose it via `@c { … }` in `facade.am`. The vendored
`webview/webview` exposes `webview_get_window()` for the native
handle (HWND / NSWindow / GtkWindow).
