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
| **M1** | Depth | Ducking + flying obstacles, 2 obstacle types |
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

## Decision Log

### 2026-02-09 -- Project approach
- Adopted **progressive evolution** strategy: ship the smallest complete experience, iterate on real feedback.
- Tuning and feel prioritized over feature count at every milestone.

### 2026-02-09 -- M0 scope adjustments
- **WebGL build moved to M0** (was M5). Rationale: instant browser playtesting enables the feedback loop that drives the whole approach.
- **High score save moved to M0** (was M1). Rationale: gives players a reason to retry; cheap to implement.
- Single obstacle type only in M0 -- gameplay depth comes in M1 with ducking and flying obstacles.

### 2026-02-09 -- Documentation structure
- **CLAUDE.md** for repo conventions, build commands, technical context (how to work here).
- **DESIGN.md** for game design, milestones, specs, decisions (what we're building and why).
- **CONVERSATION_HISTORY.md** for raw session logs per agent requirements.
