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

## v0.0.1-dev scope

Single-window apps with HTML UI, JS injection, and event-loop
control. Sufficient for forms, dashboards, viewers, internal tools
— roughly 80% of "productive" desktop apps.

| Capability                                 | Status |
|---------------------------------------------|--------|
| Native window with OS titlebar              | ✓     |
| Configurable size, title                    | ✓     |
| Load URL (`http`, `https`, `file`, `data`)  | ✓     |
| Load HTML directly                          | ✓     |
| Inject JS before page load                  | ✓     |
| Eval JS at runtime                          | ✓     |
| DevTools (debug mode)                       | ✓     |
| IPC: Amalgame ← JS (`webview_bind`)         | ✗ — v0.0.2 |
| Native menubar / dialogs / tray             | ✗ — v0.10+ |
| Multi-window (>1 simultaneous)              | Up to 4 slots, single-window recommended |
| Custom URL scheme (`am://`)                 | ✗ — v0.10+ |

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

A complete smoke test lives in [`tests/spike.am`](tests/spike.am).

## API surface (v0.0.1)

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
}

class WindowHint {
    static int None() / Min() / Max() / Fixed()
}
```

The API was designed forward-compatible with v0.10+ additions —
existing app code will *not* break when native menus, dialogs,
tray, multi-window, custom URL schemes, and typed IPC land. See
the proposal in the main Amalgame repo for the full v2 surface.

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
