# CLAUDE.md

## Project Overview

**Raylib Dino Runner** -- a Chrome offline dinosaur game clone written in C using the Raylib graphics library. All game objects are rendered as simple rectangles. The player presses Space or Up Arrow to jump over obstacles.

**Current state:** Source code, Makefile, test files, and build scripts were deleted in recent commits. The CI/CD workflow and documentation remain. The full implementation is recoverable from git history (see commit `0a0ae48` and earlier).

## Repository Structure

```
ai-bc1/
├── .github/workflows/release.yml   # GitHub Actions release pipeline
├── AGENT_REQUIREMENTS.md            # AI agent behavior rules
├── CICD_REQUIREMENTS.md             # CI/CD specifications
├── CONVERSATION_HISTORY.md          # Log of past agent conversations
├── README.md                        # Project readme
├── CLAUDE.md                        # This file
└── .gitkeep
```

### Historical source layout (deleted, recoverable from git)

```
├── main.c                           # Game entry point (~60 lines)
├── Makefile                         # Build system (GNU Make)
├── tests/test_placeholder.c         # Unit test stub (assert-based)
└── scripts/build_webgl.sh           # Emscripten WebGL build script
```

## Language & Dependencies

| Dependency     | Purpose                              |
|----------------|--------------------------------------|
| C (C11)        | Primary language                     |
| Raylib         | 2D graphics and input handling       |
| libm           | Math library                         |
| GNU Make       | Build automation                     |
| Emscripten     | Optional, for WebGL builds via emcc  |
| AWS CLI        | CI/CD: upload WebGL builds to S3     |

## Build & Run Commands

```sh
make           # Compile dino_runner executable
make run       # Compile and run the game
make test      # Compile and run unit tests
make clean     # Remove build artifacts (dino_runner, test_placeholder)
```

### WebGL build

```sh
scripts/build_webgl.sh build/webgl   # Requires emscripten (emcc) for real build
```

## Compiler Flags & Conventions

- **Compiler:** `cc` (system default C compiler)
- **Standard:** C11 (`-std=c11`)
- **Warnings:** `-Wall -Wextra` (all warnings enabled)
- **Linker flags:** `-lraylib -lm`

### Code style

- `camelCase` for local variables (`velocityY`, `groundY`, `obstacleSpeed`, `deltaTime`)
- `PascalCase` for Raylib types and functions (`Rectangle`, `InitWindow`, `BeginDrawing`)
- Braces on their own line for function/control block bodies
- `const` for compile-time constants
- Single-file architecture (all game logic in `main.c`)
- No external test framework; tests use standard `<assert.h>`

## CI/CD

**Workflow:** `.github/workflows/release.yml`
**Trigger:** Push tags matching `release/*`

Pipeline steps:
1. Checkout code
2. `make test` -- run unit tests
3. `scripts/build_webgl.sh build/webgl` -- build WebGL package
4. Upload to S3 (conditional on `S3_BUCKET` secret being set)

### Required GitHub Secrets

| Secret                  | Description                         |
|-------------------------|-------------------------------------|
| `S3_BUCKET`             | Target S3 bucket name               |
| `AWS_REGION`            | AWS region (defaults to us-east-1)  |
| `AWS_ACCESS_KEY_ID`     | AWS access key                      |
| `AWS_SECRET_ACCESS_KEY` | AWS secret key                      |

## Agent Behavior Rules

From `AGENT_REQUIREMENTS.md`:
- Address the user as **"Moses"**
- Begin conversations with **"What's up Moses"**
- Record every conversation summary in `CONVERSATION_HISTORY.md`

## Key Technical Details

- **Window:** 960x540 pixels, 60 FPS target
- **Physics:** Gravity at 1200 units/s^2, jump velocity -500 units/s
- **Obstacle speed:** 360 units/s, resets to offscreen right when passing left edge
- **Collision:** Raylib `CheckCollisionRecs()` between player and obstacle rectangles
- **Ground line:** Drawn at `screenHeight - 120` pixels from top

## Working with This Repo

1. **Restoring deleted files:** All source files exist in git history. Use `git show <commit>:<path>` or `git checkout <commit> -- <path>` to restore them (e.g., `git checkout 0a0ae48 -- main.c Makefile tests/ scripts/`).
2. **Adding new features:** Keep the single-file approach for game logic unless complexity warrants splitting. Add tests in `tests/` using `<assert.h>`.
3. **Releases:** Tag with `release/<version>` to trigger the CI/CD pipeline.
4. **Conversation logging:** After each session, append a dated summary to `CONVERSATION_HISTORY.md`.
