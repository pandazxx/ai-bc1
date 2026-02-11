#include "raylib.h"
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

/* --- Constants --- */

#define SCREEN_WIDTH 960
#define SCREEN_HEIGHT 540

#define GRAVITY 1200.0f
#define JUMP_VELOCITY -500.0f

#define BASE_SPEED 360.0f
#define SPEED_INCREMENT 0.05f
#define MAX_SPEED 720.0f

#define MIN_GAP 300.0f
#define MAX_GAP 600.0f
#define TRANSITION_GAP 500.0f

#define MAX_OBSTACLES 3

#define RESTART_COOLDOWN 0.3f

#define HITBOX_INSET 3.0f

#define FLYING_SCORE_THRESHOLD 1000
#define FLYING_CHANCE 0.3f

#define WIGGLE_INTERVAL 0.15f
#define LANDING_FRAMES 4

/* --- Game States --- */

#define STATE_PLAYING 0
#define STATE_DEAD 1

/* --- Obstacle Types --- */

#define OBS_GROUND 0
#define OBS_FLYING 1

/* --- Player Animation States --- */

#define ANIM_RUNNING 0
#define ANIM_JUMPING 1
#define ANIM_FALLING 2
#define ANIM_LANDING 3
#define ANIM_DUCKING 4
#define ANIM_DEAD 5

/* --- Data Structures --- */

typedef struct {
    Rectangle rect;
    float velocityY;
    bool grounded;
    bool ducking;
    int animState;
    float animTimer;
    int animFrame;
    int landingCounter;
} Player;

typedef struct {
    Rectangle rect;
    bool active;
    int type;
} Obstacle;

typedef struct {
    int state;
    int score;
    int highScore;
    float deadTimer;
    float obstacleSpeed;
    Obstacle obstacles[MAX_OBSTACLES];
    int obstacleHead;
    int lastObstacleType;
} Game;

/* --- Globals (needed for Emscripten main loop) --- */

static Player player;
static Game game;
static float groundY;

/* --- High Score --- */

static void LoadHighScore(void)
{
#ifdef __EMSCRIPTEN__
    game.highScore = EM_ASM_INT({
        var val = localStorage.getItem('dinoHighScore');
        return val ? parseInt(val) : 0;
    });
#else
    game.highScore = 0;
    FILE *f = fopen("highscore.dat", "r");
    if (f)
    {
        if (fscanf(f, "%d", &game.highScore) != 1)
        {
            game.highScore = 0;
        }
        fclose(f);
    }
#endif
}

static void SaveHighScore(void)
{
#ifdef __EMSCRIPTEN__
    EM_ASM({
        localStorage.setItem('dinoHighScore', $0.toString());
    }, game.highScore);
#else
    FILE *f = fopen("highscore.dat", "w");
    if (f)
    {
        fprintf(f, "%d", game.highScore);
        fclose(f);
    }
#endif
}

/* --- Obstacle Management --- */

static float RandomGap(int prevType, int nextType)
{
    float minGap = (prevType != nextType) ? TRANSITION_GAP : MIN_GAP;
    return minGap + (float)rand() / (float)RAND_MAX * (MAX_GAP - minGap);
}

static int ChooseObstacleType(void)
{
    if (game.score < FLYING_SCORE_THRESHOLD)
    {
        return OBS_GROUND;
    }
    float roll = (float)rand() / (float)RAND_MAX;
    return (roll < FLYING_CHANCE) ? OBS_FLYING : OBS_GROUND;
}

static void SpawnObstacle(Obstacle *obs, float startX, int type)
{
    obs->type = type;
    obs->rect.x = startX;
    obs->active = true;

    if (type == OBS_GROUND)
    {
        obs->rect.width = 30.0f;
        obs->rect.height = 50.0f;
        obs->rect.y = groundY - obs->rect.height;
    }
    else /* OBS_FLYING */
    {
        obs->rect.width = 40.0f;
        obs->rect.height = 145.0f;
        obs->rect.y = groundY - 180.0f;
    }
}

static void InitObstacles(void)
{
    int prevType = OBS_GROUND;
    float x = SCREEN_WIDTH + RandomGap(prevType, OBS_GROUND);
    for (int i = 0; i < MAX_OBSTACLES; i++)
    {
        int type = OBS_GROUND; /* All ground at start */
        SpawnObstacle(&game.obstacles[i], x, type);
        prevType = type;
        int nextType = OBS_GROUND;
        x += RandomGap(prevType, nextType);
    }
    game.obstacleHead = 0;
    game.lastObstacleType = OBS_GROUND;
}

/* --- Animation --- */

static void UpdatePlayerVisual(void)
{
    float baseW, baseH;

    switch (player.animState)
    {
    case ANIM_RUNNING:
        if (player.animFrame == 0)
        {
            baseW = 40.0f; baseH = 60.0f;
        }
        else
        {
            baseW = 42.0f; baseH = 58.0f;
        }
        break;
    case ANIM_JUMPING:
        baseW = 36.0f; baseH = 66.0f;
        break;
    case ANIM_FALLING:
        baseW = 44.0f; baseH = 54.0f;
        break;
    case ANIM_LANDING:
        baseW = 46.0f; baseH = 50.0f;
        break;
    case ANIM_DUCKING:
        if (player.animFrame == 0)
        {
            baseW = 60.0f; baseH = 30.0f;
        }
        else
        {
            baseW = 62.0f; baseH = 28.0f;
        }
        break;
    case ANIM_DEAD:
        baseW = 44.0f; baseH = 54.0f;
        break;
    default:
        baseW = 40.0f; baseH = 60.0f;
        break;
    }

    /* Anchor bottom edge to current Y position */
    float bottom = player.rect.y + player.rect.height;
    player.rect.width = baseW;
    player.rect.height = baseH;
    player.rect.y = bottom - baseH;
}

/* --- Game Reset --- */

static void ResetGame(void)
{
    game.state = STATE_PLAYING;
    game.score = 0;
    game.deadTimer = 0.0f;
    game.obstacleSpeed = BASE_SPEED;

    player.rect.x = 80.0f;
    player.rect.y = groundY - 60.0f;
    player.rect.width = 40.0f;
    player.rect.height = 60.0f;
    player.velocityY = 0.0f;
    player.grounded = true;
    player.ducking = false;
    player.animState = ANIM_RUNNING;
    player.animTimer = 0.0f;
    player.animFrame = 0;
    player.landingCounter = 0;

    InitObstacles();
}

/* --- Collision with hitbox forgiveness --- */

static bool CheckCollision(void)
{
    Rectangle playerHitbox = {
        player.rect.x + HITBOX_INSET,
        player.rect.y + HITBOX_INSET,
        player.rect.width - HITBOX_INSET * 2.0f,
        player.rect.height - HITBOX_INSET * 2.0f
    };

    for (int i = 0; i < MAX_OBSTACLES; i++)
    {
        if (game.obstacles[i].active && CheckCollisionRecs(playerHitbox, game.obstacles[i].rect))
        {
            return true;
        }
    }
    return false;
}

/* --- Frame --- */

static void GameFrame(void)
{
    float deltaTime = GetFrameTime();

    /* --- Input --- */
    if (game.state == STATE_PLAYING)
    {
        /* Ducking: hold Down Arrow while grounded */
        if (player.grounded)
        {
            if (IsKeyDown(KEY_DOWN))
            {
                player.ducking = true;
            }
            else
            {
                player.ducking = false;
            }
        }

        /* Jump: only when grounded and not ducking */
        if (player.grounded && !player.ducking &&
            (IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_UP)))
        {
            player.velocityY = JUMP_VELOCITY;
            player.grounded = false;
            player.ducking = false;
        }
    }
    else /* STATE_DEAD */
    {
        if (game.deadTimer >= RESTART_COOLDOWN &&
            (IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_UP)))
        {
            ResetGame();
        }
    }

    /* --- Update --- */
    if (game.state == STATE_PLAYING)
    {
        /* Player physics */
        if (!player.grounded)
        {
            player.velocityY += GRAVITY * deltaTime;
            player.rect.y += player.velocityY * deltaTime;

            if (player.rect.y >= groundY - player.rect.height)
            {
                player.rect.y = groundY - player.rect.height;
                player.velocityY = 0.0f;
                player.grounded = true;
                player.landingCounter = LANDING_FRAMES;
            }
        }

        /* Animation state machine */
        if (game.state == STATE_PLAYING)
        {
            if (!player.grounded)
            {
                player.animState = (player.velocityY < 0.0f) ? ANIM_JUMPING : ANIM_FALLING;
                player.animFrame = 0;
            }
            else if (player.landingCounter > 0)
            {
                player.animState = ANIM_LANDING;
                player.landingCounter--;
                if (player.landingCounter == 0)
                {
                    player.animState = player.ducking ? ANIM_DUCKING : ANIM_RUNNING;
                }
            }
            else if (player.ducking)
            {
                player.animState = ANIM_DUCKING;
            }
            else
            {
                player.animState = ANIM_RUNNING;
            }

            /* Wiggle timer for running and ducking */
            if (player.animState == ANIM_RUNNING || player.animState == ANIM_DUCKING)
            {
                player.animTimer += deltaTime;
                if (player.animTimer >= WIGGLE_INTERVAL)
                {
                    player.animTimer -= WIGGLE_INTERVAL;
                    player.animFrame = 1 - player.animFrame;
                }
            }
        }

        UpdatePlayerVisual();

        /* Move obstacles */
        for (int i = 0; i < MAX_OBSTACLES; i++)
        {
            if (game.obstacles[i].active)
            {
                game.obstacles[i].rect.x -= game.obstacleSpeed * deltaTime;

                /* Recycle when off-screen left */
                if (game.obstacles[i].rect.x + game.obstacles[i].rect.width < 0.0f)
                {
                    /* Find rightmost obstacle to place after it */
                    float rightmost = 0.0f;
                    int rightmostType = game.lastObstacleType;
                    for (int j = 0; j < MAX_OBSTACLES; j++)
                    {
                        float right = game.obstacles[j].rect.x + game.obstacles[j].rect.width;
                        if (right > rightmost)
                        {
                            rightmost = right;
                            rightmostType = game.obstacles[j].type;
                        }
                    }

                    int newType = ChooseObstacleType();
                    float gap = RandomGap(rightmostType, newType);
                    SpawnObstacle(&game.obstacles[i], rightmost + gap, newType);
                    game.lastObstacleType = newType;
                }
            }
        }

        /* Score */
        game.score++;

        /* Difficulty ramp */
        game.obstacleSpeed = BASE_SPEED + (float)game.score * SPEED_INCREMENT;
        if (game.obstacleSpeed > MAX_SPEED)
        {
            game.obstacleSpeed = MAX_SPEED;
        }

        /* Collision */
        if (CheckCollision())
        {
            game.state = STATE_DEAD;
            game.deadTimer = 0.0f;
            player.animState = ANIM_DEAD;
            UpdatePlayerVisual();
            if (game.score > game.highScore)
            {
                game.highScore = game.score;
                SaveHighScore();
            }
        }
    }
    else /* STATE_DEAD */
    {
        game.deadTimer += deltaTime;
    }

    /* --- Render --- */
    BeginDrawing();
    ClearBackground(RAYWHITE);

    /* Ground */
    DrawLine(0, (int)groundY, SCREEN_WIDTH, (int)groundY, DARKGRAY);

    /* Obstacles */
    for (int i = 0; i < MAX_OBSTACLES; i++)
    {
        if (game.obstacles[i].active)
        {
            Color obsColor = (game.obstacles[i].type == OBS_FLYING) ? MAROON : DARKGRAY;
            DrawRectangleRec(game.obstacles[i].rect, obsColor);
        }
    }

    /* Player */
    DrawRectangleRec(player.rect, BLACK);

    /* Score (top-right) */
    const char *scoreText = TextFormat("Score: %d", game.score);
    int scoreWidth = MeasureText(scoreText, 20);
    DrawText(scoreText, SCREEN_WIDTH - scoreWidth - 20, 20, 20, DARKGRAY);

    /* Dead overlay */
    if (game.state == STATE_DEAD)
    {
        const char *gameOverText = "Game Over";
        int goWidth = MeasureText(gameOverText, 40);
        DrawText(gameOverText, (SCREEN_WIDTH - goWidth) / 2, SCREEN_HEIGHT / 2 - 60, 40, BLACK);

        const char *hiText = TextFormat("HI: %d", game.highScore);
        int hiWidth = MeasureText(hiText, 20);
        DrawText(hiText, (SCREEN_WIDTH - hiWidth) / 2, SCREEN_HEIGHT / 2 - 10, 20, DARKGRAY);

        if (game.deadTimer >= RESTART_COOLDOWN)
        {
            const char *restartText = "Press Space to Restart";
            int restartWidth = MeasureText(restartText, 20);
            DrawText(restartText, (SCREEN_WIDTH - restartWidth) / 2, SCREEN_HEIGHT / 2 + 20, 20, GRAY);
        }
    }

    EndDrawing();
}

/* --- Main --- */

int main(void)
{
    srand((unsigned int)time(NULL));

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Raylib Dino Runner");
    SetTargetFPS(60);

    groundY = SCREEN_HEIGHT - 120.0f;

    LoadHighScore();
    ResetGame();

#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(GameFrame, 60, 1);
#else
    while (!WindowShouldClose())
    {
        GameFrame();
    }
#endif

    CloseWindow();
    return 0;
}
