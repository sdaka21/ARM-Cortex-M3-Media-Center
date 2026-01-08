#include "LPC17xx.h"
#include "GLCD.h"
#include "KBD.h"
#include "game.h" 

#define __FI 1

void game(void) {
    int selected_game = 0; // 0 = Pong, 1 = Collector
    int d;
    
    GLCD_Clear(White);
    GLCD_SetBackColor(White);
    GLCD_SetTextColor(Black);
    
    GLCD_DisplayString(1, 0, __FI, "   GAME CENTER    ");
    GLCD_DisplayString(8, 0, __FI, " SELECT: Play     ");
    GLCD_DisplayString(9, 0, __FI, " LEFT: Main Menu  ");

    while (1) {
        // Draw Selection UI
        if (selected_game == 0) {
            GLCD_SetBackColor(Blue);
            GLCD_SetTextColor(White);
            GLCD_DisplayString(4, 2, __FI, "  > PONG GAME <   ");
            
            GLCD_SetBackColor(White);
            GLCD_SetTextColor(Black);
            GLCD_DisplayString(5, 2, __FI, "    COLLECTOR     ");
        } else {
            GLCD_SetBackColor(White);
            GLCD_SetTextColor(Black);
            GLCD_DisplayString(4, 2, __FI, "    PONG GAME     ");
            
            GLCD_SetBackColor(Blue);
            GLCD_SetTextColor(White);
            GLCD_DisplayString(5, 2, __FI, "  > COLLECTOR <   ");
        }
        
        // Input Handling
        uint32_t btn = get_button();
        
        if (btn == KBD_DOWN || btn == KBD_UP) {
            selected_game = !selected_game; 
            for(d=0; d<50000; d++);
        }
        else if (btn == KBD_SELECT) {
            if (selected_game == 0) {
                pong_game(); 
            } else {
                // Replace placeholder with the new game
                collector_game();
            }
            
            // Redraw Menu on return
            GLCD_Clear(White);
            GLCD_SetBackColor(White);
            GLCD_SetTextColor(Black);
            GLCD_DisplayString(1, 0, __FI, "   GAME CENTER    ");
            GLCD_DisplayString(8, 0, __FI, " SELECT: Play     ");
            GLCD_DisplayString(9, 0, __FI, " LEFT: Main Menu  ");
        }
        else if (btn == KBD_LEFT) {
            return; 
        }
    }
}