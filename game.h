#ifndef __GAME_H
#define __GAME_H

// Main Entry Point (Called by main.c)
extern void game(void);

// Sub-Games
extern void pong_game(void);
extern void start_drawing_game(void);
extern void collector_game(void); // <--- Added this line

#endif