#!/usr/bin/env bash
set -euo pipefail

OUTPUT_DIR=${1:-build/webgl}
mkdir -p "$OUTPUT_DIR"

if command -v emcc >/dev/null 2>&1; then
  emcc main.c -o "$OUTPUT_DIR/index.html" -s USE_GLFW=3 -s ASYNCIFY
else
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
fi
