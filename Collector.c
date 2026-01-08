#include "LPC17xx.h"
#include "GLCD.h"
#include "KBD.h"
#include "LED.h"
#include <stdlib.h>
#include <stdio.h> 

// --- GAME CONSTANTS ---
#define SCREEN_WIDTH  320
#define SCREEN_HEIGHT 240
#define HEADER_HEIGHT 40 
#define GAME_TOP      (HEADER_HEIGHT + 4) // Coin spawns below this
#define LANE_WIDTH    (SCREEN_WIDTH / 3)

// Colors
#define COL_PLAYER    Blue
#define COL_COIN      Black  
#define COL_BG        White
#define COL_HEADER    Black 
#define COL_TEXT      Yellow

// Settings
#define PLAYER_W      40     
#define PLAYER_H      20
#define PLAYER_Y      (SCREEN_HEIGHT - 30) 
#define COIN_SIZE     20     
#define FALL_SPEED    12 // Fast!

// --- GLOBAL VARIABLES ---
int player_lane = 1; 
int score = 0;
int lives = 3;
int coin_lane = 0;
int coin_y = -20;
int coin_active = 0;

// --- DRAWING HELPERS ---

void draw_rect_fill(int x, int y, int w, int h, unsigned short color) {
    int i, j;
    GLCD_SetTextColor(color);
    for (i = 0; i < w; i++) {
        for (j = 0; j < h; j++) {
            if(y+j < SCREEN_HEIGHT) 
                GLCD_PutPixel(x + i, y + j);
        }
    }
}

void draw_vertical_line_segment(int x, int y_start, int length, unsigned short color) {
    int j;
    GLCD_SetTextColor(color);
    for (j = 0; j < length; j++) {
        if ((y_start + j) >= GAME_TOP && (y_start + j) < SCREEN_HEIGHT) {
            GLCD_PutPixel(x, y_start + j);
        }
    }
}

// Only draws the static layout ONCE
void draw_interface_static(void) {
    // Header
    draw_rect_fill(0, 0, SCREEN_WIDTH, HEADER_HEIGHT, COL_HEADER);
    GLCD_SetBackColor(COL_HEADER);
    GLCD_SetTextColor(White);
    GLCD_DisplayString(0, 3, 1, "Coin Catcher");
    
    // Background Lines (Full Length)
    draw_vertical_line_segment(LANE_WIDTH, GAME_TOP, SCREEN_HEIGHT-GAME_TOP, Black);
    draw_vertical_line_segment(LANE_WIDTH*2, GAME_TOP, SCREEN_HEIGHT-GAME_TOP, Black);
}

void update_score_display(void) {
    char buf[30];
    // Updates text area only
    GLCD_SetBackColor(COL_HEADER);
    GLCD_SetTextColor(COL_TEXT);
    sprintf(buf, "Lives: %d   Score: %d", lives, score);
    GLCD_DisplayString(1, 1, 1, (unsigned char *)buf);
}

void draw_player(int lane, unsigned short color) {
    int center_x = (lane * LANE_WIDTH) + (LANE_WIDTH / 2);
    int start_x = center_x - (PLAYER_W / 2);
    draw_rect_fill(start_x, PLAYER_Y, PLAYER_W, PLAYER_H, color);
}

void draw_coin(int lane, int y, unsigned short color) {
    // Safety Clip: Never draw above GAME_TOP
    if (y < GAME_TOP) return;
    
    int center_x = (lane * LANE_WIDTH) + (LANE_WIDTH / 2);
    int start_x = center_x - (COIN_SIZE / 2);
    draw_rect_fill(start_x, y, COIN_SIZE, COIN_SIZE, color);
}

// --- MAIN GAME ---

void collector_game(void) {
    int d;
    int game_running = 1;
    uint32_t last_btn = 0;
    int old_coin_y = GAME_TOP + 2; 
    
    // 1. Initialize
    player_lane = 1;
    score = 0;
    lives = 3;
    coin_active = 0;
    
    GLCD_Clear(COL_BG);
    draw_interface_static(); 
    update_score_display();  
    draw_player(player_lane, COL_PLAYER);
    
    while (game_running) {
        
        // --- A. SPAWN COIN ---
        if (!coin_active) {
            int new_lane = rand() % 3;
            if (new_lane == coin_lane) new_lane = (new_lane + 1) % 3;
            
            coin_lane = new_lane;
            coin_y = GAME_TOP + 2; // Start strictly below header
            old_coin_y = coin_y;
            coin_active = 1;
            
            draw_coin(coin_lane, coin_y, COL_COIN);
        }
        
        // --- B. INPUT ---
        uint32_t btn = get_button();
        
        if (btn == KBD_LEFT && last_btn != KBD_LEFT) {
            draw_player(player_lane, COL_BG); 
            player_lane--;
            if (player_lane < 0) player_lane = 0;
            draw_player(player_lane, COL_PLAYER); 
        }
        else if (btn == KBD_RIGHT && last_btn != KBD_RIGHT) {
            draw_player(player_lane, COL_BG); 
            player_lane++;
            if (player_lane > 2) player_lane = 2;
            draw_player(player_lane, COL_PLAYER); 
        }
        else if (btn == KBD_SELECT) {
            return;
        }
        last_btn = btn; 
        
        // --- C. PHYSICS & RENDER ---
        if (coin_active) {
            // 1. Erase Old Coin
            draw_coin(coin_lane, old_coin_y, COL_BG);
            
            // 2. Repair Lines (The Secret to Smoothness)
            // Since the coin is white now, it erased the black divider lines.
            // We just redraw the segment of the line that was "behind" the coin.
            // Center lane overlaps both lines, Left overlaps line 1, Right overlaps line 2.
            
            // Check overlap with Line 1 (LANE_WIDTH)
            int coin_left = (coin_lane * LANE_WIDTH) + (LANE_WIDTH/2) - (COIN_SIZE/2);
            int coin_right = coin_left + COIN_SIZE;
            
            if (coin_left <= LANE_WIDTH && coin_right >= LANE_WIDTH) {
                draw_vertical_line_segment(LANE_WIDTH, old_coin_y, COIN_SIZE, Black);
            }
            
            // Check overlap with Line 2 (LANE_WIDTH * 2)
            if (coin_left <= (LANE_WIDTH*2) && coin_right >= (LANE_WIDTH*2)) {
                draw_vertical_line_segment(LANE_WIDTH*2, old_coin_y, COIN_SIZE, Black);
            }
            
            // 3. Move & Draw New
            old_coin_y = coin_y;
            coin_y += FALL_SPEED;
            draw_coin(coin_lane, coin_y, COL_COIN);
        }
        
        // --- D. COLLISION ---
        int hit = 0, miss = 0;

        if (coin_y + COIN_SIZE >= PLAYER_Y) {
            if (coin_lane == player_lane) hit = 1;
            else if (coin_y > SCREEN_HEIGHT) miss = 1;
        }
        
        if (hit) {
            score++;
            coin_active = 0;
            draw_coin(coin_lane, coin_y, COL_BG); // Erase coin immediately
            update_score_display();
            
            // Visual Feedack: Player flashes Green
            draw_player(player_lane, Green);
            for(d=0;d<5000;d++);
            draw_player(player_lane, COL_PLAYER);
        }
        else if (miss) {
            lives--;
            coin_active = 0;
            draw_coin(coin_lane, coin_y, COL_BG); // Erase coin immediately
            update_score_display();
            
            // Visual Feedback: Player flashes Red
            // NO SCREEN CLEARING - Keeps header safe and game fast
            draw_player(player_lane, Red);
            for(d=0;d<5000;d++);
            draw_player(player_lane, COL_PLAYER);
        }

        if (lives <= 0) game_running = 0;
        
        // Game Speed Loop
        for(d=0; d<15000; d++);
    }
    
    // --- GAME OVER ---
    GLCD_Clear(Black);
    GLCD_SetBackColor(Black);
    GLCD_SetTextColor(Red);
    GLCD_DisplayString(4, 6, 1, "GAME OVER");
    
    char score_msg[20];
    sprintf(score_msg, "Final Score: %d", score);
    GLCD_SetTextColor(White);
    GLCD_DisplayString(6, 4, 1, (unsigned char *)score_msg);
    GLCD_DisplayString(8, 3, 1, "SELECT to Exit");
    
    while (get_button() != KBD_SELECT) {
        for(d=0; d<50000; d++);
    }
    GLCD_Clear(White);
}