#!/usr/bin/env bash
set -euo pipefail

OUTPUT_DIR=${1:-build/webgl}
mkdir -p "$OUTPUT_DIR"

if ! command -v emcc >/dev/null 2>&1; then
  echo "emscripten not found, generating placeholder"
  cat > "$OUTPUT_DIR/index.html" <<'HTML'
<!DOCTYPE html>
<html lang="en">
  <head>
    <meta charset="UTF-8" />
    <title>Raylib Dino Runner</title>
  </head>
  <body>
    <p>WebGL build placeholder. Install emscripten to generate a real build.</p>
  </body>
</html>
HTML
  exit 0
fi

# Build Raylib for web if not already cached
RAYLIB_SRC=${RAYLIB_SRC:-/tmp/raylib}
RAYLIB_LIB="$RAYLIB_SRC/src/libraylib.a"

if [ ! -f "$RAYLIB_LIB" ]; then
  echo "Building Raylib for web platform..."
  git clone --depth 1 https://github.com/raysan5/raylib.git "$RAYLIB_SRC"
  cd "$RAYLIB_SRC/src"
  make PLATFORM=PLATFORM_WEB -j$(nproc 2>/dev/null || echo 2)
  cd - > /dev/null
fi

echo "Building WebGL package..."
emcc main.c -o "$OUTPUT_DIR/index.html" \
  -I"$RAYLIB_SRC/src" \
  -L"$RAYLIB_SRC/src" \
  -s USE_GLFW=3 \
  -s ASYNCIFY \
  -lraylib -lm \
  --shell-file "$RAYLIB_SRC/src/minshell.html" 2>/dev/null \
  || emcc main.c -o "$OUTPUT_DIR/index.html" \
    -I"$RAYLIB_SRC/src" \
    -L"$RAYLIB_SRC/src" \
    -s USE_GLFW=3 \
    -s ASYNCIFY \
    -lraylib -lm

echo "WebGL build complete: $OUTPUT_DIR/"
