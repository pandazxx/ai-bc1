# Game Design Document

## Vision

A Chrome offline dinosaur game clone built with Raylib -- simple, tight, and fun. Rectangle-based graphics. Playable in browser via WebGL.

## Design Principles

- **Progressive evolution** -- start with the smallest complete experience, ship it, gather feedback, iterate. Each milestone is a feedback checkpoint, not a deadline.
- **Tuning over features** -- a polished core loop beats a feature-rich game with sloppy feel. Nail the jump arc and difficulty curve before adding content.
- **Ship to learn** -- WebGL build from day one so anyone can playtest from a browser link. Real player feedback drives the roadmap.

## Milestones

| Milestone | Theme | Scope |
|---|---|---|
| **M0** | The Loop | Core run/jump/die/retry, score, high score save, difficulty ramp, WebGL build |
| **M1** | Depth | Ducking + flying obstacles, 2 obstacle types, squash/stretch + wiggle animation |
| **M2** | Juice | Particles, screen shake, day/night cycle, sound effects |
| **M3** | Progression | Milestones, combo scoring, death stats screen |
| **M4** | Modes | Pause menu, challenge mode or time trial |
| **M5** | Polish | Leaderboard, final tuning pass |

Milestones are a guide, not a contract. Feedback after each one determines what actually ships next.

## M0 Spec -- "The Loop"

**Goal:** A player picks it up, immediately understands it, wants to beat their last score, and can share a browser link for others to try.

### Features

| Element | Detail |
|---|---|
| **Player** | Single rectangle, fixed X position, gravity + jump |
| **Ground** | Horizontal line, always visible |
| **Obstacles** | Single type (ground-level boxes), random spacing |
| **Collision** | Instant death on hit |
| **Score** | Distance-based counter displayed on screen |
| **High score** | Persisted (file on native, localStorage on WebGL) |
| **Difficulty ramp** | Obstacle speed increases linearly with score |
| **Death & restart** | Freeze frame, show final score + best score, Space to retry |
| **Input** | Space or Up Arrow to jump -- nothing else |
| **WebGL build** | Playable in browser via Emscripten, deployed to S3 via CI/CD |

### Explicitly out of M0

- No ducking, no double jump, no power-ups
- No sound, no particles, no animations
- No menus, no pause
- No multiple obstacle types

### Tuning variables (to nail during M0)

| Variable | Notes |
|---|---|
| **Jump arc** | Height, hang time, landing speed. Should feel snappy, not floaty |
| **Obstacle spacing** | Minimum gap between obstacles so it's always survivable |
| **Difficulty curve** | How fast speed increases, whether it plateaus or keeps climbing |
| **Hitbox forgiveness** | Collision box slightly smaller than visual box -- near-misses should feel fair |

### Implementation plan

1. Restore source from git history (main.c, Makefile, tests, scripts)
2. Add score system -- distance counter, display on screen
3. Add high score -- save/load from file; WebGL uses localStorage via Emscripten API
4. Add difficulty ramp -- tie obstacle speed to current score
5. Add death & restart flow -- game states (playing/dead), freeze on collision, show scores, Space to restart
6. Tuning pass -- jump arc, obstacle gaps, difficulty curve
7. WebGL build -- ensure build_webgl.sh produces a working browser build
8. Tests -- basic assertions for game logic

## M0 Low-Level Design

### 1. Game States & Flow

Two states only. Game starts directly in PLAYING -- no title screen.

```
PLAYING ──(collision)──► DEAD ──(Space/Up after cooldown)──► PLAYING
```

| State | Behavior |
|---|---|
| **PLAYING** | Player runs, obstacles scroll, score counts up, jump input accepted |
| **DEAD** | Everything frozen in place. Show final score + best score. Accept restart after 0.3s cooldown |

No title screen in M0. Player is in the action immediately on launch.

### 2. User Interaction

**PLAYING state:**

| Input | Action |
|---|---|
| Space / Up Arrow (press) | Jump, only when grounded |

**DEAD state:**

| Input | Action |
|---|---|
| Space / Up Arrow (press) | Reset and restart, only after 0.3s cooldown |

Rules:
- **Fixed jump** -- every jump is identical. No variable height based on hold duration. Revisit for M1 if obstacle variety demands it.
- **Fresh press only** -- holding the key does not repeat jump. Must release and press again.
- **No input buffering** -- pressing jump before landing does not queue the jump.
- **Restart cooldown** -- 0.3s delay in DEAD state before restart input is accepted. Prevents accidental restart from mashing jump.

### 3. Physics Engine

**Integration method:** Euler integration (sufficient for simple platformer physics).

**Per-frame update:**
```
velocityY += gravity * deltaTime
playerY   += velocityY * deltaTime
if (playerY >= groundY) → clamp to ground, set grounded = true
```

All physics uses `deltaTime` (`GetFrameTime()`) for frame-rate independence.

**Constants (starting values, subject to tuning):**

| Constant | Value | Notes |
|---|---|---|
| **gravity** | 1200 u/s² | Downward acceleration |
| **jumpVelocity** | -500 u/s | Instant upward velocity on jump |

**Derived characteristics:**

| Property | Value |
|---|---|
| Time to apex | ~0.42s |
| Jump height | ~104 px |
| Total airtime | ~0.84s |

These are starting points. Final feel is determined during the tuning pass.

### 4. Data Structures

**Language:** C with structs, written for easy C++ migration later (no `typedef` tricks, use C99/C11 features that overlap with C++).

```c
typedef struct {
    Rectangle rect;      // position & size for drawing + collision
    float velocityY;     // current vertical velocity
    bool grounded;       // on the ground?
} Player;

typedef struct {
    Rectangle rect;      // position & size
    bool active;         // currently on screen?
} Obstacle;

#define MAX_OBSTACLES 3

typedef struct {
    int state;           // STATE_PLAYING or STATE_DEAD
    int score;           // current run score (int, distance-based)
    int highScore;       // best score, persisted
    float deadTimer;     // time spent in DEAD state (for restart cooldown)
    float obstacleSpeed; // current speed, increases with score
    Obstacle obstacles[MAX_OBSTACLES]; // fixed-size circular buffer
    int obstacleHead;    // index of next obstacle to spawn/recycle
} Game;
```

**Obstacle buffer:** Fixed-size circular array (`MAX_OBSTACLES = 3`). When an obstacle scrolls off the left edge, recycle it to the right with new random spacing. `obstacleHead` tracks which slot to reuse next. No dynamic allocation.

### 5. Frame Loop

Update-then-render. Player sees the result of this frame's input immediately.

```
Every frame:
  1. deltaTime = GetFrameTime()
  2. Input  (state-dependent)
  3. Update (state-dependent)
  4. Render (always runs)
```

**PLAYING frame:**

| Phase | Actions |
|---|---|
| **Input** | Check Space/Up press → set `velocityY = jumpVelocity` if grounded |
| **Update** | Apply gravity to player, move obstacles left, recycle off-screen obstacles, increment score, ramp `obstacleSpeed`, check collision → transition to DEAD |
| **Render** | Draw ground, player rect, obstacle rects, score text |

**DEAD frame:**

| Phase | Actions |
|---|---|
| **Input** | Check Space/Up press → restart if `deadTimer >= 0.3s` |
| **Update** | Increment `deadTimer` only. Everything else frozen. |
| **Render** | Draw ground, player rect, obstacle rects (frozen), "Game Over" text, final score, high score |

### 6. Score & Difficulty

**Score:** Increments each frame by `1` while in PLAYING state. At 60 FPS, roughly 60 points per second. Simple, deterministic, no float math.

**Difficulty ramp:**

```
obstacleSpeed = BASE_SPEED + (score * SPEED_INCREMENT)
```

| Constant | Starting value | Notes |
|---|---|---|
| `BASE_SPEED` | 360 u/s | Initial obstacle scroll speed |
| `SPEED_INCREMENT` | 0.05 u/s per point | Speed gain per score point |
| `MAX_SPEED` | 720 u/s | Hard cap, 2x base speed |

At these values: max speed reached at score ~7200 (roughly 2 minutes of play). Subject to tuning.

**Obstacle spacing:** When recycling an obstacle to the right edge, add a random gap between `MIN_GAP` and `MAX_GAP` (in pixels) beyond the screen width. Ensures the game is always survivable.

| Constant | Starting value |
|---|---|
| `MIN_GAP` | 300 px |
| `MAX_GAP` | 600 px |

### 7. High Score Storage

**Native (desktop):** Plain text file `highscore.dat` in working directory. Contains a single integer. Read on startup, write on death if score > highScore.

```c
// Save
FILE *f = fopen("highscore.dat", "w");
fprintf(f, "%d", highScore);
fclose(f);

// Load
FILE *f = fopen("highscore.dat", "r");
if (f) { fscanf(f, "%d", &highScore); fclose(f); }
```

**WebGL (Emscripten):** Use `emscripten_run_script()` to call `localStorage.setItem()` / `localStorage.getItem()`. Wrapped behind `#ifdef __EMSCRIPTEN__` preprocessor guard.

**Error handling:** If file doesn't exist or read fails, high score defaults to 0. No crash, no error message. Silent fallback.

### 8. Rendering & Layout

**Window:** 960x540 pixels, 60 FPS.

**Draw order** (back to front):
1. Clear background (`RAYWHITE`)
2. Ground line at `y = screenHeight - 120`
3. Obstacle rectangles (`DARKGRAY`)
4. Player rectangle (`DARKGRAY`)
5. Score text (top-right)
6. Game Over overlay (DEAD state only, centered)

**Player position:** Fixed X at `80px` from left edge. Y determined by physics.

**Player size:** 40x60 px (width x height). Collision rect inset by ~2-4px for hitbox forgiveness.

**Obstacle size:** 30x50 px (width x height). Sitting on the ground line.

**Text layout:**

| Text | Position | When |
|---|---|---|
| Score: `NNN` | Top-right, 20px padding | Always (PLAYING + DEAD) |
| `Game Over` | Center screen | DEAD only |
| `HI: NNN` | Below game over text | DEAD only |
| `Press Space to Restart` | Below high score | DEAD only |

All text uses Raylib default font. No custom fonts in M0.

### 9. WebGL Considerations

**Main loop:** Emscripten requires replacing the `while` loop with `emscripten_set_main_loop()`. Use `#ifdef __EMSCRIPTEN__` to switch between native and WebGL loop styles.

```c
#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(GameFrame, 60, 1);
#else
    while (!WindowShouldClose()) { GameFrame(); }
#endif
```

This means game logic must be refactored into a single `GameFrame()` function that runs one frame, rather than a `while` loop in `main()`.

**Build command:** `em++ main.c -o index.html -s USE_GLFW=3 -s ASYNCIFY -lraylib`

**High score:** Uses `localStorage` via `emscripten_run_script()` behind `#ifdef __EMSCRIPTEN__` guard (see section 7).

**Output:** `build/webgl/` directory containing `index.html`, `index.js`, `index.wasm`. Deployable to S3 as static files.

## M1 Spec -- "Depth"

**Goal:** Transform the one-trick timing game into a read-and-react game. The player now makes decisions: jump or duck.

### Features

| Element | Detail |
|---|---|
| **Ducking** | Down Arrow held = duck (wider/shorter rect). Release = stand. Grounded only. |
| **Flying obstacle** | New obstacle type that must be ducked under, can't jump over |
| **Obstacle types** | Ground (jump over) and flying (duck under) |
| **Spawning rules** | Type-aware gaps to prevent impossible patterns |
| **Animation** | Squash/stretch on jump/land, wiggle while running, dead pose |

### Explicitly out of M1

- No variable jump height (fixed jump + duck covers the two obstacle types)
- No speed bonus for ducking (deferred to later milestone)
- No sound, no particles

### 1. Ducking Mechanic

- **Input:** Down Arrow held = duck. Release = stand up.
- **Player rect:** Standing 40x60 → Ducking 60x30 (wider, shorter).
- **Hitbox:** Follows visual rect minus inset.
- **Can't jump while ducking.** Must release Down first, then press Space/Up.
- **Grounded only.** Can't duck mid-air.

### 2. Flying Obstacle

| Property | Ground obstacle | Flying obstacle |
|---|---|---|
| Size (WxH) | 30x50 | 40x145 |
| Y position | On ground (`groundY - 50`) | Top at `groundY - 180`, bottom at `groundY - 35` |
| Avoid by | Jumping | Ducking |
| Can jump over? | Yes | No -- extends above jump apex (player top at apex ~`groundY - 170`) |

Flying obstacle extends from `groundY - 180` (above jump apex) down to `groundY - 35` (35px above ground). Ducking player (30px tall) fits under the 35px gap. Standing or jumping player always collides.

### 3. Obstacle Spawning

**Type selection:**
- **Score < 1000:** Ground obstacles only. Player learns basics first.
- **Score >= 1000:** Random mix -- 70% ground, 30% flying. Ground stays the default; flying is the curveball.

**Type-aware minimum gap rules (prevents impossible patterns):**

| Transition | Minimum gap | Reasoning |
|---|---|---|
| Ground → Ground | `MIN_GAP` (300px) | Same as M0 |
| Flying → Flying | `MIN_GAP` (300px) | Stay ducking |
| Ground → Flying | `TRANSITION_GAP` (500px) | Must land from jump + start duck |
| Flying → Ground | `TRANSITION_GAP` (500px) | Must stand up + react + jump |

At max speed (720 u/s), 500px = ~0.7s. Player airtime is ~0.84s, so landing happens with ~0.14s before the next obstacle. Tight but survivable.

**Rule:** Never spawn ground and flying obstacles at overlapping X positions.

`TRANSITION_GAP` is a tuning variable -- adjust based on playtesting.

### 4. Animation (Squash & Stretch + Wiggle)

All animation is rectangle deformation only. Bottom of rect stays anchored to ground (adjust `rect.y` when height changes).

| State | Width | Height | Effect |
|---|---|---|---|
| **Running A** | 40 | 60 | Normal pose |
| **Running B** | 42 | 58 | Wiggle pose (alternates with A every ~0.15s) |
| **Jumping up** | 36 | 66 | Vertical stretch |
| **Falling** | 44 | 54 | Horizontal squash |
| **Landing** | 46 | 50 | Impact squash (3-4 frames, then back to running) |
| **Ducking A** | 60 | 30 | Ducking normal |
| **Ducking B** | 62 | 28 | Ducking wiggle |
| **Dead** | 44 | 54 | Squashed, frozen |

### 5. Data Structure Changes (M1 deltas from M0)

```c
#define OBS_GROUND 0
#define OBS_FLYING 1

typedef struct {
    Rectangle rect;
    bool active;
    int type;           // OBS_GROUND or OBS_FLYING
} Obstacle;

typedef struct {
    Rectangle rect;
    float velocityY;
    bool grounded;
    bool ducking;       // currently holding down?
    float animTimer;    // wiggle cycle timer
    int animFrame;      // 0 or 1 (pose A/B)
} Player;

typedef struct {
    // ... existing fields ...
    int lastObstacleType;  // track previous type for gap rules
} Game;
```

### 6. Variable Jump

**Not in M1.** Fixed jump + duck covers the two obstacle types cleanly. Binary decision: see ground obstacle → jump, see flying obstacle → duck. Revisit if mid-height obstacles are added later.

## Decision Log

### 2026-02-09 -- Project approach
- Adopted **progressive evolution** strategy: ship the smallest complete experience, iterate on real feedback.
- Tuning and feel prioritized over feature count at every milestone.

### 2026-02-09 -- M0 scope adjustments
- **WebGL build moved to M0** (was M5). Rationale: instant browser playtesting enables the feedback loop that drives the whole approach.
- **High score save moved to M0** (was M1). Rationale: gives players a reason to retry; cheap to implement.
- Single obstacle type only in M0 -- gameplay depth comes in M1 with ducking and flying obstacles.

### 2026-02-09 -- Game states
- Two states only: PLAYING and DEAD. No title screen.
- Game starts directly in PLAYING on launch (like Chrome dino).

### 2026-02-09 -- User interaction
- Fresh press only, no hold-to-repeat jump.
- No input buffering (no queued jumps before landing).
- 0.3s cooldown in DEAD state before restart input accepted.

### 2026-02-09 -- Physics and jump model
- **Fixed jump** for M0. Every jump has the same arc. Variable jump (hold for height) deferred to M1 if needed.
- **Euler integration** for physics. Simple, sufficient for this game.
- Gravity 1200 u/s², jump velocity -500 u/s as starting values, subject to tuning.
- All physics frame-rate independent via `deltaTime`.

### 2026-02-10 -- Low-level design (topics 5-9)
- **Frame loop:** Update-then-render order. State-dependent input and update, render always runs.
- **Score:** Int, +1 per frame. Difficulty ramp: linear speed increase capped at 2x base speed.
- **Obstacle spacing:** Random gap between MIN_GAP (300px) and MAX_GAP (600px).
- **High score:** File I/O on native, localStorage on WebGL. Silent fallback to 0 on error.
- **Rendering:** Back-to-front draw order. Default font. Player at fixed X=80px. Hitbox inset for forgiveness.
- **WebGL:** `emscripten_set_main_loop()` with `GameFrame()` function. `#ifdef __EMSCRIPTEN__` guards.

### 2026-02-10 -- Data structures
- Structs for Player, Obstacle, Game.
- **Circular buffer** (fixed-size array, `MAX_OBSTACLES = 3`) for obstacles. No dynamic allocation.
- **Int score** -- simple to display, no float-to-int casting needed.

### 2026-02-10 -- Language choice
- **C with structs** for M0. Maximise compatibility for future C++ migration.
- C++ migration rules when the time comes: rename `.c` → `.cpp`, `cc` → `g++`/`em++`, add methods to structs.
- C++ discipline: no inheritance hierarchies, no templates (except `std::vector` etc.), no exceptions, no smart pointers.

### 2026-02-10 -- M1 design decisions
- **Ducking:** Down Arrow held = wider/shorter rect (60x30). Grounded only. Can't jump while ducking.
- **Flying obstacle:** 40x35, bottom edge ~35px above ground. Must duck, can't jump over.
- **Spawning:** Ground only until score 1000, then 70/30 ground/flying mix.
- **Type-aware gaps:** `TRANSITION_GAP` (500px) between different types to prevent impossible patterns. Same-type uses `MIN_GAP` (300px).
- **Animation:** Squash/stretch + wiggle via rectangle deformation. No sprites, no new assets.
- **No variable jump in M1.** Fixed jump + duck is sufficient for two obstacle types.
- **No ducking speed bonus in M1.** Deferred to later milestone.

### 2026-02-09 -- Documentation structure
- **CLAUDE.md** for repo conventions, build commands, technical context (how to work here).
- **DESIGN.md** for game design, milestones, specs, decisions (what we're building and why).
- **CONVERSATION_HISTORY.md** for raw session logs per agent requirements.
