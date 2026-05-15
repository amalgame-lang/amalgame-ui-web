/*
 * webview.cc — single translation unit that instantiates the
 * webview/webview C++ implementation.
 *
 * webview.h is a header-only library: declarations are visible from
 * C, but the actual implementation is C++ and only emerges when the
 * header is compiled by a C++ compiler. Including the header here in
 * a .cc file gives us exactly one set of symbols, linked into the
 * final binary alongside the C glue (Amalgame_UI_Web.c) and the
 * facade.am output.
 *
 * Do NOT include webview.h from any other .cc TU in the project, or
 * you will get duplicate-symbol link errors. Include it from .c TUs
 * (declarations only) freely.
 *
 * Platform notes:
 *   - Linux   : requires pkg-config webkit2gtk-4.1
 *   - macOS   : requires -framework Cocoa -framework WebKit
 *   - Windows : requires WebView2Loader.dll and Windows SDK
 *
 * License: this file is Apache-2.0 (Amalgame package wrapper).
 * The vendored webview.h is MIT (upstream — see its own header).
 */

#include "webview.h"
