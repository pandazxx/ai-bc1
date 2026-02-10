#include <assert.h>
#include <stdio.h>
#include <math.h>

/* Test game constants and physics formulas directly.
   No Raylib dependency -- pure logic tests. */

/* Constants copied from main.c */
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
#define SCREEN_WIDTH 960
#define SCREEN_HEIGHT 540

static void test_jump_physics(void)
{
    /* Jump apex time: v / g = 500 / 1200 ≈ 0.417s */
    float apexTime = -JUMP_VELOCITY / GRAVITY;
    assert(apexTime > 0.4f && apexTime < 0.45f);

    /* Jump height: v^2 / (2g) = 250000 / 2400 ≈ 104px */
    float jumpHeight = (JUMP_VELOCITY * JUMP_VELOCITY) / (2.0f * GRAVITY);
    assert(jumpHeight > 100.0f && jumpHeight < 110.0f);

    /* Total airtime: 2 * apex time ≈ 0.834s */
    float airtime = 2.0f * apexTime;
    assert(airtime > 0.8f && airtime < 0.9f);

    printf("  jump physics: OK\n");
}

static void test_difficulty_ramp(void)
{
    /* At score 0, speed should be BASE_SPEED */
    float speed0 = BASE_SPEED + 0 * SPEED_INCREMENT;
    assert(speed0 == BASE_SPEED);

    /* At score 7200, speed should hit MAX_SPEED */
    float speed7200 = BASE_SPEED + 7200.0f * SPEED_INCREMENT;
    assert(speed7200 >= MAX_SPEED);

    /* Speed should be capped at MAX_SPEED */
    float speedCapped = BASE_SPEED + 99999.0f * SPEED_INCREMENT;
    if (speedCapped > MAX_SPEED) speedCapped = MAX_SPEED;
    assert(speedCapped == MAX_SPEED);

    printf("  difficulty ramp: OK\n");
}

static void test_obstacle_gaps(void)
{
    /* MIN_GAP must be large enough for player to survive at MAX_SPEED.
       Time to cross gap: MIN_GAP / MAX_SPEED = 300 / 720 ≈ 0.417s
       This should be less than total airtime (~0.834s) so jumping is viable. */
    float crossTime = MIN_GAP / MAX_SPEED;
    float airtime = 2.0f * (-JUMP_VELOCITY / GRAVITY);
    assert(crossTime < airtime);

    /* MAX_GAP > MIN_GAP */
    assert(MAX_GAP > MIN_GAP);

    printf("  obstacle gaps: OK\n");
}

static void test_hitbox_inset(void)
{
    /* Player visual: 40x60, hitbox should be smaller by 2*INSET in each dimension */
    float visualW = 40.0f;
    float visualH = 60.0f;
    float hitW = visualW - HITBOX_INSET * 2.0f;
    float hitH = visualH - HITBOX_INSET * 2.0f;

    assert(hitW > 0.0f && hitW < visualW);
    assert(hitH > 0.0f && hitH < visualH);

    printf("  hitbox inset: OK\n");
}

static void test_screen_constants(void)
{
    assert(SCREEN_WIDTH == 960);
    assert(SCREEN_HEIGHT == 540);

    /* Ground at screenHeight - 120 */
    float groundY = SCREEN_HEIGHT - 120.0f;
    assert(groundY == 420.0f);

    printf("  screen constants: OK\n");
}

int main(void)
{
    printf("Running tests...\n");
    test_jump_physics();
    test_difficulty_ramp();
    test_obstacle_gaps();
    test_hitbox_inset();
    test_screen_constants();
    printf("All tests passed.\n");
    return 0;
}
