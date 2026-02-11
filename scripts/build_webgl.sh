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
  git clone --depth 1 --branch 5.0 https://github.com/raysan5/raylib.git "$RAYLIB_SRC"
  cd "$RAYLIB_SRC/src"
  emcc -c rcore.c -Os -Wall -DPLATFORM_WEB -DGRAPHICS_API_OPENGL_ES2
  emcc -c rshapes.c -Os -Wall -DPLATFORM_WEB -DGRAPHICS_API_OPENGL_ES2
  emcc -c rtextures.c -Os -Wall -DPLATFORM_WEB -DGRAPHICS_API_OPENGL_ES2
  emcc -c rtext.c -Os -Wall -DPLATFORM_WEB -DGRAPHICS_API_OPENGL_ES2
  emcc -c rmodels.c -Os -Wall -DPLATFORM_WEB -DGRAPHICS_API_OPENGL_ES2
  emcc -c utils.c -Os -Wall -DPLATFORM_WEB
  emcc -c raudio.c -Os -Wall -DPLATFORM_WEB
  emar rcs libraylib.a rcore.o rshapes.o rtextures.o rtext.o rmodels.o utils.o raudio.o
  cd - > /dev/null
  echo "Raylib built: $RAYLIB_LIB"
fi

echo "Building WebGL package..."
emcc main.c -o "$OUTPUT_DIR/index.html" \
  -I"$RAYLIB_SRC/src" \
  -L"$RAYLIB_SRC/src" \
  -Os -Wall \
  -s USE_GLFW=3 \
  -s ASYNCIFY \
  -s TOTAL_MEMORY=67108864 \
  -DPLATFORM_WEB \
  -lraylib

echo "WebGL build complete: $OUTPUT_DIR/"
