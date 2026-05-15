# amalgame-ui-web

Webview-based GUI binding for the [Amalgame](https://github.com/amalgame-lang/Amalgame) language.

Wraps the [`webview/webview`](https://github.com/webview/webview) library
(MIT, by Serge Zaitsev and Steffen André Langnes) and exposes a small
Amalgame surface for driving HTML/CSS/JS interfaces rendered by the
OS-native web engine.

| Platform | Engine            | Bundled with OS? |
|----------|-------------------|------------------|
| Windows  | WebView2          | Yes on Win11; auto-installed on recent Win10 |
| macOS    | WKWebView         | Yes since 10.10  |
| Linux    | WebKitGTK         | No — install per distro |

## Why webview, not Tk / SDL / IUP?

This is the de-facto modern choice for productive desktop apps:
VS Code, Slack, Discord, 1Password (Tauri) all ship on top of an
OS webview. See [`docs/proposals/amalgame-ui-web.md`](../Amalgame/docs/proposals/amalgame-ui-web.md)
in the main Amalgame repo for the full design rationale (industry
survey, alternatives evaluated, v1→v2 migration guarantee).

## Scope

Single-window apps with HTML UI, JS injection, declarative IPC,
and an AM-side HTML builder with form-reading. Sufficient for
forms, dashboards, viewers, internal tools — roughly 80% of
"productive" desktop apps. See [`CHANGELOG.md`](CHANGELOG.md)
for the per-release detail.

| Capability                                  | Status |
|---------------------------------------------|--------|
| Native window with OS titlebar              | ✓ v0.0.1 |
| Configurable size, title                    | ✓ v0.0.1 |
| Load URL / HTML                             | ✓ v0.0.1 |
| Inject / Eval JS                            | ✓ v0.0.1 |
| DevTools (debug mode)                       | ✓ v0.0.1 |
| IPC: Amalgame ← JS (`Window.Bind`)          | ✓ v0.0.2 |
| HTML builder API (`Element` / `Page`)       | ✓ v0.0.3 |
| Form reading (auto-collect input state)     | ✓ v0.0.4 |
| OS theme auto + baseline CSS + overrides    | ✓ v0.0.4 |
| Declarative result routing (`OnResult`)     | ✓ v0.0.5 |
| Native menubar / dialogs / tray             | ✗ — v0.1+ |
| Multi-window (>1 simultaneous)              | Up to 4 slots, single-window recommended |
| Custom URL scheme (`am://`)                 | ✗ — v0.1+ |

## Install

```sh
amc package add ui-web
```

## Build prereqs

### Linux (Debian/Ubuntu)

```sh
sudo apt install libwebkit2gtk-4.1-dev pkg-config
```

### Linux (Fedora)

```sh
sudo dnf install webkit2gtk4.1-devel pkgconf
```

### Linux (Arch)

```sh
sudo pacman -S webkit2gtk-4.1 pkgconf
```

### macOS

Nothing to install — WKWebView ships with the OS (10.10+). Xcode
command-line tools provide the Cocoa/WebKit frameworks linked at
build time.

### Windows

- **Windows 11**: WebView2 is part of the OS.
- **Windows 10 ≥ 1803**: Microsoft auto-installs the evergreen
  WebView2 runtime.
- **Windows 10 < 1803**: ship the WebView2 bootstrapper in your
  installer. Add the following to your `amalgame.iss`:

  ```ini
  [Files]
  Source: "vendor\MicrosoftEdgeWebView2Setup.exe"; \
      DestDir: "{tmp}"; Flags: deleteafterinstall

  [Run]
  Filename: "{tmp}\MicrosoftEdgeWebView2Setup.exe"; \
      Parameters: "/silent /install"; \
      StatusMsg: "Installing WebView2 runtime..."
  ```

  Bootstrapper download: <https://developer.microsoft.com/microsoft-edge/webview2/>.

## Quick start

```amalgame
import Amalgame.UI.Web

class Program {
    public static void Main() {
        let w: Window = new Window("My App", 1024, 768, true)
        w.SetHtml("<h1>Hello, world</h1>")
        w.Run()
        w.Destroy()
    }
}
```

Or load a local file:

```amalgame
w.Navigate("file:///" + Cwd() + "/ui/index.html")
```

Smoke tests live in [`tests/`](tests/) — `spike.am` (window MVP),
`spike_bind.am` (IPC), `spike_builder.am` (Element/Page),
`spike_form.am` (form reading).

## API surface

```amalgame
class Window {
    Window(title: string, w: int, h: int, debug: bool)
    bool   IsValid()
    string GetTitle()  /  int GetWidth()  /  int GetHeight()
    void   SetTitle(t: string)
    void   SetSize(w: int, h: int, hint: int)
    void   Navigate(url: string)
    void   SetHtml(html: string)
    void   Init(js: string)             // injected before each page load
    void   Eval(js: string)             // run JS now (fire-and-forget)
    void   Run()                        // blocks on event loop
    void   Terminate()
    void   Destroy()
    void   Bind(name: string, handler: Closure)   // v0.0.2 — JS → AM
    void   Unbind(name: string)
}

class WindowHint {
    static int None() / Min() / Max() / Fixed()
}

// v0.0.3+ HTML builder
class Element {
    Element Attr(k, v) / Id(s) / Class(s) / Style(s)
    Element AddChild(c: Element) / SetText(t: string)
    Element OnClick(handler: Closure)
    Element OnResult(targetId: string)            // v0.0.5 — route return → #id
    Element Bind(name: string)                    // v0.0.4 — form key

    static Element Stack() / Row() / Div()
    static Element Heading(t) / Label(t) / Pre(t)
    static Element Button(t)
    static Element Input(name) / Textarea(name)   // v0.0.4
    static Element Select(name) / Option(value, label)  // v0.0.4
    static Element CheckBox(name)                 // v0.0.4
    static Element Radio(name, value)             // v0.0.4
}

class Page {
    static Page New()
    Page SetTitle(t: string) / SetBody(e: Element)
    Page SetStylesheet(url: string)               // v0.0.4 — replace baseline
    Page AddCss(url: string)                      // v0.0.4 — layer on top
    Page NoBaseline()                             // v0.0.4 — disable baseline
    Page SetTheme(mode: string)                   // v0.0.4 — "auto" | "light" | "dark"
    string EffectiveTheme()                       // v0.0.4 — resolved "light" or "dark"
    string Render()
    void   ApplyTo(win: Window)
    static string BaselineCss()                   // v0.0.4 — exposed for inspection
    static string DetectOSTheme()                 // v0.0.4 — "light" or "dark"
}
```

The API is designed forward-compatible with v0.1+ additions —
existing app code will *not* break when native menus, dialogs,
tray, multi-window, custom URL schemes, and typed IPC land. See
the proposal in the main Amalgame repo for the full v1 surface.

## Theming (v0.0.4)

`Page.Render` ships a baseline stylesheet by default. The baseline
exposes seven CSS variables you can override from your own
stylesheet without re-authoring the whole rule set:

| Variable          | Light default | Dark default | Used by                       |
|-------------------|---------------|--------------|-------------------------------|
| `--amc-bg`        | `#fff`        | `#1e1e1e`    | body background, inputs       |
| `--amc-fg`        | `#1a1a1a`     | `#e8e8e8`    | body text, all controls       |
| `--amc-muted`     | `#6a6a6a`     | `#9a9a9a`    | secondary text (reserved)     |
| `--amc-border`    | `#d0d0d0`     | `#404040`    | input / button borders        |
| `--amc-surface`   | `#f5f5f5`     | `#2a2a2a`    | button bg, pre bg             |
| `--amc-accent`    | `#0066cc`     | `#4a9eff`    | focus outline (reserved)      |
| `--amc-radius`    | `4px`         | `4px`        | input / button border-radius  |

The dark variant keys off `[data-theme=dark]` on `<html>`, which
`Page.Render` writes from the OS theme detection (`gsettings`
on Linux, `defaults` on macOS, registry on Windows). A secondary
`@media (prefers-color-scheme: dark)` fallback applies when
the caller bypasses `Render` and ships raw HTML.

On Linux specifically, dark mode also flips
`GtkSettings::gtk-application-prefer-dark-theme` at window
creation so the native `<select>` popup, scrollbars, and file
dialogs (rendered by GTK, not the webview) match the page theme.
Override with `Page.SetTheme("light"|"dark")` or by exporting
`AMALGAME_UI_THEME=dark` before launch.

Override an individual variable from a custom stylesheet:

```css
:root { --amc-accent: #ff6b00 }
```

Three ways to bring your own styles:

```amalgame
// Baseline + your overrides layered on top
Page.New().AddCss("file:///abs/app/overrides.css")

// Skip the baseline, ship your own complete stylesheet
Page.New().SetStylesheet("file:///abs/app/style.css")

// Skip the baseline, layer multiple stylesheets
Page.New().NoBaseline()
    .AddCss("https://unpkg.com/@picocss/pico@2/css/pico.min.css")
    .AddCss("file:///abs/app/app.css")
```

## Result routing (v0.0.5)

`Element.OnResult(targetId)` pairs with `OnClick` to push the
handler's return value into another element without a manual JS
bridge. Valid JSON returns are pretty-printed; plain strings pass
through verbatim.

```amalgame
Element.Stack()
    .AddChild(Element.Input("user"))
    .AddChild(
        Element.Button("Submit")
            .OnClick((req: string) => req)   // returns the form as JSON
            .OnResult("out")                  // → #out
    )
    .AddChild(Element.Pre("").Id("out"))
```

Behind the scenes `Page.Render` wraps the bound call in
`window.__amc_route(...)`, which awaits the promise from
`Window.Bind` and updates `document.getElementById(targetId)`.
Pass `""` to clear a previously-set target.

## Architecture

```
Amalgame app
    ↕ @c { Amalgame_UI_Web_* }
runtime/Amalgame_UI_Web.c    (slot table, thin wrappers)
    ↕
runtime/vendor/webview/webview.h    (vendored, MIT)
    ↕
WebView2 (Win) / WKWebView (mac) / WebKitGTK (Linux)
    ↕
HTML / CSS / JS frontend
```

`webview.h` is a header-only library; its C++ implementation is
instantiated exactly once via `runtime/vendor/webview/webview.cc`.
Both files are listed under `[stdlib].sources` in `amalgame.toml`
so amc compiles them as part of the package build.

## Vendored third-party

- `runtime/vendor/webview/webview.h` — webview/webview v0.12.0
  (MIT, © 2017 Serge Zaitsev, 2022 Steffen André Langnes).
  Upstream: <https://github.com/webview/webview>.

## Distributing apps

End users do **not** need the `-dev` packages — those only contain
build-time headers. They need the runtime shared library matching
the build version.

| OS                       | What end users need                                       |
|--------------------------|------------------------------------------------------------|
| Debian / Ubuntu          | `libwebkit2gtk-4.1-0 libgtk-3-0` (declare in `.deb` Depends) |
| Fedora                   | `webkit2gtk4.1 gtk3` (declare in `.rpm` Requires)          |
| Arch                     | `webkit2gtk-4.1 gtk3` (declare in `PKGBUILD` depends)      |
| macOS                    | nothing — WKWebView ships with the OS since 10.10          |
| Windows 11               | nothing — WebView2 runtime is part of the OS               |
| Windows 10 ≥ 1803        | nothing — Microsoft auto-installs the WebView2 evergreen   |
| Windows 10 < 1803        | bundle the WebView2 bootstrapper in `amalgame.iss`         |

On modern GNOME / KDE desktops WebKitGTK is usually pre-installed
(used by GNOME Web, Geary, evolution-mail, etc.). On minimal
server / non-graphical installs it isn't — declare the dependency
explicitly in your packaging.

**Three distribution strategies for Linux**, in order of recommendation
for v0.0.1:

1. **Native package with declared dependency** — let apt / dnf /
   pacman pull in WebKitGTK at install time. Smallest binary
   (~150 KB Amalgame app + a few MB facade.o / webview.o), best
   integration, what almost every Linux app does.
2. **AppImage** — bundle WebKitGTK and GTK3 into a single
   ~50 MB blob. Zero install friction, runs on any glibc-compatible
   distro. Best for distribution outside official channels.
3. **Flatpak** — bundle via the freedesktop runtime. Best
   sandboxing, but adds Flathub-publishing overhead.

`amc package build --bundle appimage` is on the roadmap for
v0.10+; until then, use one of the standard Linux packaging tools
(`fpm`, `dh-make`, `linuxdeploy`).

## License

Apache-2.0 — see [`LICENSE`](LICENSE).
The vendored `webview.h` retains its original MIT license.
