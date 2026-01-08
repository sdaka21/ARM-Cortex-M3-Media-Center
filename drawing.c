#include <math.h>
#include <stdio.h>        
#include <stdlib.h> 
#include <LPC17xx.H>
#include <stdint.h>
#include "string.h"
#include "GLCD.h"
#include "LED.h"
#include "KBD.h"
#include "Menu.h"

// --- Definitions ---
#define PIXEL_DRAW 2
#define LINE_DRAW 1
#define CIRCLE_DRAW 0
#define MENU_MODE 3

#define MENU_CIRCLE 0 
#define MENU_LINE  1 
#define MENU_PIXEL 2 
#define MENU_SAVE 3 

#define MENU_WIDTH 58
#define MENU_ICON_HEIGHT 52

// --- Globals ---
int MENU = MENU_CIRCLE;
int MODE = PIXEL_DRAW; 
int pX = 1, pY = 1; // Player Position

// External Assets
extern unsigned char DRAWMEN_pixel_data[];
extern unsigned char userPicture[6][48];
extern unsigned char ClockLEDOn;

// History Variables
int lastPressX1, lastPressY1;
int lastPressX2, lastPressY2;

// --- Helpers ---

void init_drawing(void){
    GLCD_Clear(LightGrey);
    pX= 1; pY= 1;
    GLCD_SetBackColor(Black);
    GLCD_SetTextColor(Black);
}

void setColor(int color){
    GLCD_SetBackColor(color);
    GLCD_SetTextColor(color);
}

void setVarMode(int mode){
    MODE = mode;
}

// --- Graphics Primitives ---

void draw1Pixel(unsigned int x, unsigned int y){
    GLCD_PutPixel(x, y);
}

void drawFilledRect(unsigned int x, unsigned int y, unsigned int width, unsigned int height) {
    int i, j;
    for (i = 0; i<width; i++){
        for (j = 0; j<height; j++){
            GLCD_PutPixel(x+i, y+j);
        }
    }
}

// Draws a scaled 5x5 pixel block
void draw5Pixel(unsigned int x, unsigned int y, int colour){
    setColor(colour);
    drawFilledRect(x*5 + 80, y*5, 5, 5);
}

void drawRect(unsigned int x, unsigned int y, unsigned int width, unsigned int height) {
    int i, j;
    for(i=0; i<width; i++) { GLCD_PutPixel(x+i, y); GLCD_PutPixel(x+i, y+height-1); }
    for(j=0; j<height; j++) { GLCD_PutPixel(x, y+j); GLCD_PutPixel(x+width-1, y+j); }
}

void drawPointer(unsigned int x, unsigned int y){
    setColor(Green);
    drawRect((x)*5 - 1 + 80,(y)*5 - 1,7,7);
}

void removePointer(unsigned int x, unsigned int y){
    setColor(White);
    drawRect((x)*5 - 1 + 80,(y)*5 - 1,7,7);
}

void movePointer(unsigned int x, unsigned int y){
    removePointer(pX, pY);
    drawPointer(x,y);
}

// --- Canvas Logic ---

void canvasUpdatePixel(unsigned int x, unsigned int y){
    unsigned int byteIndex = x / 8;
    int bitIndex = x % 8; 
    unsigned char byte = userPicture[byteIndex][y];
    int bitValue = (byte >> (7 - bitIndex) & 1);
    
    if (bitValue == 1) draw5Pixel(x,y, Black);
    else               draw5Pixel(x,y, White);
} 

void refreshMenu(void){
    // Ensure DRAWMEN_pixel_data is available before using
    // GLCD_Bitmap(0,0, 60,240, DRAWMEN_pixel_data);
}

void refreshCanvasStart(void){
    int col, row;
    for (col = 0; col < 48; col++) {
        for (row = 0; row < 48; row++) {
            canvasUpdatePixel(col,row);
        }
    }
}

void canvasDrawPixel(unsigned int x, unsigned int y, int colour){
    int byteIndex = x / 8;
    int bitIndex = x % 8;

    if (colour == Black) userPicture[byteIndex][y] |= (1 << (7-bitIndex));
    else                 userPicture[byteIndex][y] &= ~(1 << (7-bitIndex));
    
    draw5Pixel(x, y, colour);
}

// --- Menu Logic ---

int menuUp(void){
    int temp = MENU-1 ;
    if (temp < 0) temp = 3;    
    return temp;
}

int menuDown(void){
    int temp = MENU+1 ;    
    if (temp > 3) temp = 0;
    return temp;
}

void drawMenuPointer(int menuItem){
    setColor(Blue);
    drawRect(1,(menuItem) *60 ,MENU_WIDTH, MENU_ICON_HEIGHT);
    setColor(Black);
}

void drawLoadMenuPointer(int menuItem){
    setColor(Red);
    drawRect(1,(menuItem) *60 ,MENU_WIDTH, MENU_ICON_HEIGHT);
    setColor(Black);
}

void removeMenuPointer(int menuItem){
    setColor(White);    
    drawRect(1,(menuItem) *60 ,MENU_WIDTH, MENU_ICON_HEIGHT);
    setColor(Black);
}

void updateMenu(int menuItem){
    removeMenuPointer(MENU);
    drawMenuPointer(menuItem);
    MENU = menuItem;
}

void setMode(int mode){
    switch (mode) {
        case MENU_MODE :
            MODE = MENU_MODE;
            removePointer(pX,pY);
            drawMenuPointer(MENU);
            break;
        case PIXEL_DRAW :
            MODE = PIXEL_DRAW;
            removeMenuPointer(MENU);
            break;
        case CIRCLE_DRAW :
            MODE = CIRCLE_DRAW;
            removeMenuPointer(MENU);
            break;
        case LINE_DRAW :
            MODE = LINE_DRAW;
            removeMenuPointer(MENU);
            break;
    }
}

void refreshGap(int y){
    setColor(LightGrey);
    drawFilledRect(60, (y * 5) -1, 20, 13);
    setColor(Black);
}

// --- Input Logic ---

void updatePointer(void) {
    int input = get_button();
    
    if (input == KBD_UP) {
        if(MODE == MENU_MODE) updateMenu(menuUp());
        else {
            removePointer(pX, pY); canvasUpdatePixel(pX,pY);
            pY--; if (pY > 48 || pY < 1) pY = 1;
            drawPointer(pX,pY);
        }
    } 
    else if (input == KBD_DOWN) {
        if(MODE == MENU_MODE) updateMenu(menuDown());
        else {
            removePointer(pX, pY); canvasUpdatePixel(pX,pY);
            pY++; if (pY > 48 || pY < 1) pY = 1;
            drawPointer(pX,pY);
        }
    } 
    else if (input == KBD_LEFT && MODE != MENU_MODE) {
        removePointer(pX, pY); canvasUpdatePixel(pX,pY);
        if (pX < 1) { setMode(MENU_MODE); refreshGap(pY); } 
        else { pX--; drawPointer(pX,pY); }
    } 
    else if (input == KBD_RIGHT) {
        if(MODE != MENU_MODE) {
            removePointer(pX, pY); canvasUpdatePixel(pX,pY);
            if (pX > 48 || pX < 1)pX = 1; 
            pX++; drawPointer(pX,pY);
        } else{
            pX = 1; refreshGap(pY); setMode(PIXEL_DRAW); drawPointer(pX,pY);
        }
    }
    if (pX > 48 || pX < 0)pX = 1; 
    if (pY > 48 || pY < 0) pY = 1;
}

void canvasDrawLine(unsigned int x0, unsigned int y0,unsigned int x1, unsigned int y1, int colour){
    unsigned int x = x0; 
    unsigned int y = y0; 
    int dx = (x1 > x0) ? (x1 - x0) : (x0 - x1); 
    int dy = (y1 > y0) ? (y1 - y0) : (y0 - y1); 
    int sx = (x0 < x1) ? 1 : -1;                
    int sy = (y0 < y1) ? 1 : -1;                
    int err = dx - dy;                          

    while (1) {
        canvasDrawPixel(x, y, colour); 
        if (x == x1 && y == y1) break;          
        int e2 = 2 * err; 
        if (e2 > -dy) { err -= dy; x += sx; }
        if (e2 < dx) { err += dx; y += sy; }
    }
}

void canvasDrawCircle(int xc, int yc, int radius){
    int x = 0; int y = radius; int d = 3 - 2 * radius; 
    while (x <= y) {
        canvasDrawPixel(xc+x, yc+y, Black); canvasDrawPixel(xc-x, yc+y, Black);
        canvasDrawPixel(xc+x, yc-y, Black); canvasDrawPixel(xc-x, yc-y, Black);
        canvasDrawPixel(xc+y, yc+x, Black); canvasDrawPixel(xc-y, yc+x, Black);
        canvasDrawPixel(xc+y, yc-x, Black); canvasDrawPixel(xc-y, yc-x, Black);
        if (d <= 0) d = d + 4 * x + 6;
        else { d = d + 4 * (x - y) + 10; y--; }
        x++;
    }
}

// --- MAIN ENTRY POINT (Called by game.c menu) ---
void start_drawing_game(void){
    int histNum = 0; 
    int cHistNum = 0; 
    unsigned int lineframeCounter = 0;
    unsigned int circleframeCounter = 0;
    int rad = 0;
    
    init_drawing();
    refreshMenu();
    draw5Pixel(4,4, White);
    refreshCanvasStart();
    drawPointer(pX,pY);
    
    for(;;){
        if (ClockLEDOn) { ClockLEDOn = 0; updatePointer(); }
        
        // Pixel Draw
        if(get_button() == KBD_SELECT && MODE == PIXEL_DRAW){
            canvasDrawPixel(pX,pY, Black);
        }
        
        // Line Draw
        lineframeCounter++;
        if(get_button() == KBD_SELECT && MODE == LINE_DRAW){
            drawLoadMenuPointer(MENU);
            if (histNum == 0 && lineframeCounter > 30) {
                lastPressX1 = pX; lastPressY1 = pY;
                histNum = 1; lineframeCounter = 0;        
            }
            else if (histNum == 1 && lineframeCounter > 30) {
                lastPressX2 = pX; lastPressY2 = pY;
                canvasDrawLine(lastPressX1,lastPressY1, lastPressX2, lastPressY2, Black);
                histNum=0; lineframeCounter = 0;
            }
        }
        
        // Menu Save
        if(get_button() == KBD_SELECT && MODE == MENU_MODE){
            if (MENU == MENU_SAVE){
                drawLoadMenuPointer(MENU_SAVE);
                refreshCanvasStart();
                removeMenuPointer(MENU_SAVE);
            }
            else MODE = MENU; 
        }

        // Circle Draw
        circleframeCounter++;
        if(get_button() == KBD_SELECT && MODE == CIRCLE_DRAW){
            drawLoadMenuPointer(MENU);
            if (cHistNum == 0 && circleframeCounter > 30) {
                lastPressX1 = pX; lastPressY1 = pY;
                cHistNum = 1; circleframeCounter = 0;        
            }
            else if (cHistNum == 1 && circleframeCounter > 30) {
                lastPressX2 = pX; lastPressY2 = pY;
                rad = fmax( abs(lastPressX1 - pX)/2 , abs(lastPressX2 - pY)/2);
                canvasDrawCircle(lastPressX1,lastPressY1, rad);
                cHistNum=0; circleframeCounter = 0;
            }
        }
        
        // --- EXIT CONDITION ---
        if (get_button() == KBD_LEFT && MODE == MENU_MODE){
            return; // Return to Game Menu
        }
    }
}