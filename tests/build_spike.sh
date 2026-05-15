#!/bin/sh
# Build the spike smoke test for amalgame-ui-web.
#
# Mirrors the ui-tk build flow:
#   1. Compile webview.cc with g++ (instantiates the C++ impl
#      of the vendored webview/webview library — exactly one TU).
#   2. Compile Amalgame_UI_Web.c with gcc (the thin C glue).
#   3. Run amc --lib on facade.am to produce a facade .c file +
#      compile it to an .o.
#   4. Compile tests/spike.am with amc → spike.c.
#   5. Link the binary with webkit2gtk + gtk + libamalgame.
#
# Requires:
#   - amc 0.8.10+
#   - g++ + gcc
#   - libwebkit2gtk-4.1-dev (Debian/Ubuntu) or webkit2gtk4.1-devel (Fedora)
#   - pkg-config

set -eu

# Resolve paths relative to this script's location so the script
# works whether invoked from package root or from tests/.
HERE="$(cd "$(dirname "$0")" && pwd)"
PKG_DIR="$(dirname "$HERE")"
AMC_DIR="${AMC_DIR:-/home/neitsab/Développement/Amalgame}"
AMC="${AMC:-$AMC_DIR/amc}"

cd "$PKG_DIR"

# --- 0. Detect WebKitGTK variant --------------------------------------
# 4.1 is the current upstream target (GTK 3 + libsoup3). 4.0 is the
# legacy variant (GTK 3 + libsoup2) still shipped by many distros.
# webview/webview supports both. We honour whichever .pc file is
# present, with 4.1 preferred.
if pkg-config --exists webkit2gtk-4.1; then
    WEBKIT_PC=webkit2gtk-4.1
elif pkg-config --exists webkit2gtk-4.0; then
    WEBKIT_PC=webkit2gtk-4.0
else
    echo "ERROR: neither webkit2gtk-4.1 nor webkit2gtk-4.0 is installed."
    echo "Install one of:"
    echo "  Debian/Ubuntu: sudo apt install libwebkit2gtk-4.1-dev"
    echo "                  (or libwebkit2gtk-4.0-dev on older releases)"
    echo "  Fedora:         sudo dnf install webkit2gtk4.1-devel"
    echo "  Arch:           sudo pacman -S webkit2gtk-4.1"
    exit 1
fi
echo "Using $WEBKIT_PC ($(pkg-config --modversion "$WEBKIT_PC"))"

# --- 1. webview C++ implementation TU ---------------------------------
# Define WEBVIEW_STATIC so WEBVIEW_API expands to plain `extern` (not
# `inline`). Without it the C++ compiler emits the functions as inline,
# no symbols land in the .o, and the final link fails on
# `undefined reference to webview_create` (etc).
if [ ! -f runtime/vendor/webview/webview.o ] || \
   [ runtime/vendor/webview/webview.h -nt runtime/vendor/webview/webview.o ] || \
   [ runtime/vendor/webview/webview.cc -nt runtime/vendor/webview/webview.o ]; then
    echo "Building webview C++ impl…"
    g++ -c -O2 -std=c++17 -DNDEBUG -Wno-unused-parameter \
        -DWEBVIEW_STATIC \
        -I runtime/vendor/webview \
        $(pkg-config --cflags "$WEBKIT_PC") \
        runtime/vendor/webview/webview.cc \
        -o runtime/vendor/webview/webview.o
fi

# --- 2. C glue TU ------------------------------------------------------
# Same WEBVIEW_STATIC requirement — keeps the C-side declarations in
# sync with the C++ TU's symbol visibility.
if [ ! -f runtime/Amalgame_UI_Web.o ] || \
   [ runtime/Amalgame_UI_Web.h -nt runtime/Amalgame_UI_Web.o ] || \
   [ runtime/Amalgame_UI_Web.c -nt runtime/Amalgame_UI_Web.o ]; then
    echo "Building C glue layer…"
    gcc -c -O2 -DWEBVIEW_STATIC \
        -I runtime \
        -I runtime/vendor/webview \
        -I "$AMC_DIR/runtime" \
        runtime/Amalgame_UI_Web.c \
        -o runtime/Amalgame_UI_Web.o
fi

# --- 3. facade.am → facade.o ------------------------------------------
if [ ! -f facade.o ] || \
   [ facade.am -nt facade.o ] || \
   [ runtime/Amalgame_UI_Web.h -nt facade.o ]; then
    echo "Building facade.am…"
    "$AMC" --lib facade.am --quiet
    gcc -c -O2 \
        -I runtime \
        -I "$AMC_DIR/runtime" \
        a.out.c \
        -o facade.o
fi

# --- 4. spike.am → spike.c --------------------------------------------
mkdir -p build
echo "Compiling tests/spike.am…"
"$AMC" -o build/spike tests/spike.am \
       --external facade.am --quiet

# --- 5. Link the spike binary -----------------------------------------
# Use gcc (not g++) for spike.c — Amalgame_Net.h passes string literals
# as `code_string` (non-const char*) which g++ rejects in C++ mode.
# Pull libstdc++ in explicitly so the C++ symbols from webview.o resolve.
echo "Linking build/spike…"
gcc -O2 \
    -I "$AMC_DIR/runtime" \
    -I runtime \
    -I runtime/vendor/webview \
    build/spike.c \
    facade.o \
    runtime/Amalgame_UI_Web.o \
    runtime/vendor/webview/webview.o \
    "$AMC_DIR/lib/libamalgame.a" \
    $(pkg-config --libs "$WEBKIT_PC") \
    -lstdc++ -lm -lgc -lcurl -lz -ldl -lpthread \
    -o build/spike

echo "Built build/spike — run it now."
