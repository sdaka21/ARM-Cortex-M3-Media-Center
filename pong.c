#include "LPC17xx.h"
#include "GLCD.h"
#include "KBD.h"
#include "LED.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "game.h"
#include "pong.h"

// --- CONSTANTS from PONG.H ---
// MAX_BALLS is now 3, BALL_SIZE is 6, BALL_SPEED is 3
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240
#define BALL_SIZE 6 
#define BALL_SPEED 3 
#define MAX_BALLS 3 // Maximum number of active balls allowed

// --- GAME SETTINGS ---
#define PADDLE_WIDTH  5
#define PADDLE_HEIGHT 40
#define PADDLE_SPEED  8    // Faster paddles
#define AI_SPEED      6    // Faster AI to keep up
#define WINNING_SCORE 5
#define SCORE_BAR_HEIGHT 20

// --- SPEED SETTING (Applies to the new BALL_SPEED of 3) ---
#define BALL_SPEED_MULTIPLIER 3 

// Structure for Ball (Defined in pong.h, kept here for reference of type)
// typedef struct {
//     int x, y;
//     int dx, dy;
//     int active;
// } Ball;

// Game Variables
Ball balls[MAX_BALLS];
int num_active_balls = 0;
int player_y, ai_y;
int player_score = 0, ai_score = 0;
int game_over = 0;
extern unsigned char ClockLEDOn;

// --- DRAWING HELPERS ---

// Color Definitions (Assumed)
#define DarkGrey 0x52AA
#define White    0xFFFF
#define Black    0x0000
#define Yellow   0xFFE0
#define Green    0x07E0
#define Cyan     0x07FF
#define Red      0xF800

void pong_draw_rect(int x, int y, int w, int h, unsigned short color) {
    int i, j;
    GLCD_SetTextColor(color);
    for (i = 0; i < w; i++) {
        for (j = 0; j < h; j++) {
            GLCD_PutPixel(x + i, y + j);
        }
    }
}

// Dedicated Score Area (The Box)
void draw_score_interface(void) {
    char buf[30];
    // Draw Score Background Bar
    pong_draw_rect(0, 0, SCREEN_WIDTH, SCORE_BAR_HEIGHT, DarkGrey);
    // Draw Divider Line
    pong_draw_rect(0, SCORE_BAR_HEIGHT, SCREEN_WIDTH, 2, White);
    
    GLCD_SetBackColor(DarkGrey);
    GLCD_SetTextColor(White);
    sprintf(buf, "CPU: %d   YOU: %d", ai_score, player_score);
    GLCD_DisplayString(0, 3, 1, (unsigned char *)buf);
}

void pong_show_message(const char *msg) {
    // Clear center area for message
    pong_draw_rect(20, 100, 280, 40, Black);
    GLCD_SetBackColor(Black);
    GLCD_SetTextColor(Yellow);
    GLCD_DisplayString(5, 1, 1, (unsigned char *)msg);
}

// --- BALL LOGIC ---

void reset_ball(Ball *ball) {
    ball->x = SCREEN_WIDTH / 2;
    ball->y = (SCREEN_HEIGHT + SCORE_BAR_HEIGHT) / 2;
    
    // Randomize speed slightly for variety
    int speed_var = (rand() % 3);
    
    // *** Speed calculated based on the new BALL_SPEED (3 * 3 = 9) ***
    int initial_speed = (BALL_SPEED + speed_var) * BALL_SPEED_MULTIPLIER; 
    
    // Set initial direction
    ball->dx = (rand() % 2 == 0) ? initial_speed : -initial_speed;
    ball->dy = (rand() % 2 == 0) ? initial_speed : -initial_speed;

    ball->active = 1;
}

void initialize_balls(void) {
    int i;
    for (i = 0; i < MAX_BALLS; i++) balls[i].active = 0;
    
    // Start with 1 ball
    reset_ball(&balls[0]);
    num_active_balls = 1;
}

void add_new_ball(void) {
    int i;
    if (num_active_balls < MAX_BALLS) {
        for (i = 0; i < MAX_BALLS; i++) {
            if (!balls[i].active) {
                reset_ball(&balls[i]);
                num_active_balls++;
                return;
            }
        }
    }
}

void update_ball_position(Ball *ball) {
    if (!ball->active) return;

    // 1. Erase Old
    pong_draw_rect(ball->x, ball->y, BALL_SIZE, BALL_SIZE, Black);
    
    // 2. Move
    ball->x += ball->dx;
    ball->y += ball->dy;
    
    // 3. Wall Collision (Top bounds is SCORE_BAR_HEIGHT + 2)
    if (ball->y <= SCORE_BAR_HEIGHT + 2 || ball->y >= SCREEN_HEIGHT - BALL_SIZE) {
        ball->dy = -ball->dy;
    }
    
    // 4. Paddle Collision (AI / Left)
    if (ball->x <= 10 + PADDLE_WIDTH &&
        ball->y + BALL_SIZE >= ai_y &&
        ball->y <= ai_y + PADDLE_HEIGHT) {
        ball->dx = abs(ball->dx);
        ball->dy += (ball->y - (ai_y + PADDLE_HEIGHT/2)) / 4; // Add spin
    }

    // 5. Paddle Collision (Player / Right)
    if (ball->x + BALL_SIZE >= SCREEN_WIDTH - 15 &&
        ball->y + BALL_SIZE >= player_y &&
        ball->y <= player_y + PADDLE_HEIGHT) {
        ball->dx = -abs(ball->dx);
        ball->dy += (ball->y - (player_y + PADDLE_HEIGHT/2)) / 4; // Add spin
    }
    
    // 6. Scoring
    if (ball->x < 0) { // CPU Missed (Player Scores)
        player_score++;
        draw_score_interface();
        ball->active = 0;
        num_active_balls--;
        
        // Add one new ball if max is not reached (MAX_BALLS is now 3)
        if (num_active_balls < MAX_BALLS) {
            add_new_ball();
        } 
        // Safety net: if all balls were lost, ensure one is started
        if (num_active_balls == 0) {
            add_new_ball();
        }
        
    }
    else if (ball->x > SCREEN_WIDTH) { // Player Missed (CPU Scores)
        ai_score++;
        draw_score_interface();
        ball->active = 0;
        num_active_balls--;
        
        // Add one new ball if max is not reached (MAX_BALLS is now 3)
        if (num_active_balls < MAX_BALLS) {
            add_new_ball();
        } 
        // Safety net: if all balls were lost, ensure one is started
        if (num_active_balls == 0) {
            add_new_ball();
        }
    }

    // 7. Game Over Check
    if (player_score >= WINNING_SCORE || ai_score >= WINNING_SCORE) {
        game_over = 1;
    }

    // 8. Redraw
    if (ball->active) {
        pong_draw_rect(ball->x, ball->y, BALL_SIZE, BALL_SIZE, Red);
    }
}

// --- PADDLE LOGIC ---

void update_player(uint32_t btn) {
    pong_draw_rect(SCREEN_WIDTH - 15, player_y, PADDLE_WIDTH, PADDLE_HEIGHT, Black);

    if (btn == KBD_UP) player_y -= PADDLE_SPEED;
    if (btn == KBD_DOWN) player_y += PADDLE_SPEED;

    if (player_y < SCORE_BAR_HEIGHT + 5) player_y = SCORE_BAR_HEIGHT + 5;
    if (player_y > SCREEN_HEIGHT - PADDLE_HEIGHT) player_y = SCREEN_HEIGHT - PADDLE_HEIGHT;

    pong_draw_rect(SCREEN_WIDTH - 15, player_y, PADDLE_WIDTH, PADDLE_HEIGHT, Green);
}

void update_ai(void) {
    int i;
    int target_y = -1;
    int closest_dist = SCREEN_WIDTH;

    // Target the closest incoming ball
    for (i = 0; i < MAX_BALLS; i++) {
        if (balls[i].active && balls[i].dx < 0) {
            if (balls[i].x < closest_dist) {
                closest_dist = balls[i].x;
                target_y = balls[i].y;
            }
        }
    }
    
    // If no ball is incoming, aim for the center
    if (target_y == -1) target_y = (SCREEN_HEIGHT + SCORE_BAR_HEIGHT) / 2;

    pong_draw_rect(10, ai_y, PADDLE_WIDTH, PADDLE_HEIGHT, Black);

    // AI movement logic (chase target_y)
    if (target_y > ai_y + (PADDLE_HEIGHT/2)) ai_y += AI_SPEED;
    else if (target_y < ai_y + (PADDLE_HEIGHT/2)) ai_y -= AI_SPEED;

    // Boundary checks
    if (ai_y < SCORE_BAR_HEIGHT + 5) ai_y = SCORE_BAR_HEIGHT + 5;
    if (ai_y > SCREEN_HEIGHT - PADDLE_HEIGHT) ai_y = SCREEN_HEIGHT - PADDLE_HEIGHT;

    pong_draw_rect(10, ai_y, PADDLE_WIDTH, PADDLE_HEIGHT, Cyan);
}

// --- MAIN LOOP ---

void pong_game(void) {
    int i, d;
    
    // Reset Game
    player_score = 0; ai_score = 0; game_over = 0;
    player_y = 120; ai_y = 120;
    initialize_balls();

    GLCD_Init();
    GLCD_Clear(Black);
    draw_score_interface(); // Draw top bar
    
    // Initial Paddles
    pong_draw_rect(SCREEN_WIDTH - 15, player_y, PADDLE_WIDTH, PADDLE_HEIGHT, Green);
    pong_draw_rect(10, ai_y, PADDLE_WIDTH, PADDLE_HEIGHT, Cyan);
    
    while (!game_over) {
        if (ClockLEDOn) {
            ClockLEDOn = 0;
            
            if (get_button() == KBD_SELECT) {
                GLCD_Clear(Black);
                return;
            }
            
            update_player(get_button());
            update_ai();
            
            for (i = 0; i < MAX_BALLS; i++) {
                update_ball_position(&balls[i]);
            }
        }
    }
    
    // --- Game Over ---
    if (player_score >= WINNING_SCORE) pong_show_message(" YOU WIN! (SELECT) ");
    else                               pong_show_message(" CPU WINS! (SELECT)");

    // Wait for Exit
    while (get_button() != KBD_SELECT) {
        for(d=0; d<50000; d++); // Small delay loop
    }
    
    GLCD_Clear(Black);
}