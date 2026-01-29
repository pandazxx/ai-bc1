#include "raylib.h"

int main(void)
{
    const int screenWidth = 960;
    const int screenHeight = 540;

    InitWindow(screenWidth, screenHeight, "Raylib Dino Runner");
    SetTargetFPS(60);

    const float groundY = screenHeight - 120.0f;
    Rectangle dino = {120.0f, groundY - 60.0f, 40.0f, 60.0f};
    Rectangle obstacle = {screenWidth + 60.0f, groundY - 50.0f, 30.0f, 50.0f};
    float velocityY = 0.0f;
    bool grounded = true;
    float obstacleSpeed = 360.0f;

    while (!WindowShouldClose())
    {
        float deltaTime = GetFrameTime();

        if (grounded && (IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_UP)))
        {
            velocityY = -500.0f;
            grounded = false;
        }

        velocityY += 1200.0f * deltaTime;
        dino.y += velocityY * deltaTime;

        if (dino.y >= groundY - dino.height)
        {
            dino.y = groundY - dino.height;
            velocityY = 0.0f;
            grounded = true;
        }

        obstacle.x -= obstacleSpeed * deltaTime;
        if (obstacle.x + obstacle.width < 0.0f)
        {
            obstacle.x = screenWidth + 120.0f;
        }

        if (CheckCollisionRecs(dino, obstacle))
        {
            obstacle.x = screenWidth + 120.0f;
        }

        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawLine(0, (int)groundY, screenWidth, (int)groundY, DARKGRAY);
        DrawRectangleRec(dino, BLACK);
        DrawRectangleRec(obstacle, DARKGRAY);
        DrawText("Press SPACE or UP to jump", 20, 20, 20, GRAY);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
