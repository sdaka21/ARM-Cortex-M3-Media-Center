#ifndef __PONG_H
#define __PONG_H

#define MAX_BALLS 3          // Maximum number of balls allowed in play (Reduced from 5)
#define BALL_SIZE 6
#define BALL_SPEED 3
#define SCREEN_WIDTH  320
#define SCREEN_HEIGHT 240

// Structure for a single ball
typedef struct {
    int x, y;
    int dx, dy;
    int active; // 1 if active, 0 if inactive/scored
} Ball;

#endif