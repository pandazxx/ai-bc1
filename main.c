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

#define MAX_OBSTACLES 3

#define RESTART_COOLDOWN 0.3f

#define HITBOX_INSET 3.0f

/* --- Game States --- */

#define STATE_PLAYING 0
#define STATE_DEAD 1

/* --- Data Structures --- */

typedef struct {
    Rectangle rect;
    float velocityY;
    bool grounded;
} Player;

typedef struct {
    Rectangle rect;
    bool active;
} Obstacle;

typedef struct {
    int state;
    int score;
    int highScore;
    float deadTimer;
    float obstacleSpeed;
    Obstacle obstacles[MAX_OBSTACLES];
    int obstacleHead;
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

static float RandomGap(void)
{
    return MIN_GAP + (float)rand() / (float)RAND_MAX * (MAX_GAP - MIN_GAP);
}

static void SpawnObstacle(Obstacle *obs, float startX)
{
    obs->rect.x = startX;
    obs->rect.y = groundY - 50.0f;
    obs->rect.width = 30.0f;
    obs->rect.height = 50.0f;
    obs->active = true;
}

static void InitObstacles(void)
{
    float x = SCREEN_WIDTH + RandomGap();
    for (int i = 0; i < MAX_OBSTACLES; i++)
    {
        SpawnObstacle(&game.obstacles[i], x);
        x += RandomGap();
    }
    game.obstacleHead = 0;
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
        if (player.grounded && (IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_UP)))
        {
            player.velocityY = JUMP_VELOCITY;
            player.grounded = false;
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
        player.velocityY += GRAVITY * deltaTime;
        player.rect.y += player.velocityY * deltaTime;

        if (player.rect.y >= groundY - player.rect.height)
        {
            player.rect.y = groundY - player.rect.height;
            player.velocityY = 0.0f;
            player.grounded = true;
        }

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
                    for (int j = 0; j < MAX_OBSTACLES; j++)
                    {
                        float right = game.obstacles[j].rect.x + game.obstacles[j].rect.width;
                        if (right > rightmost)
                        {
                            rightmost = right;
                        }
                    }
                    SpawnObstacle(&game.obstacles[i], rightmost + RandomGap());
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
            DrawRectangleRec(game.obstacles[i].rect, DARKGRAY);
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
