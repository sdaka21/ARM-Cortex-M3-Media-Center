#include <LPC17xx.H> /* LPC17xx definitions */
#include "Menu.h"
#include "GLCD.h"

extern unsigned char STAR_pixel_data[];

//to cycle through menu items
uint8_t getNextMenu(uint8_t current){

	current = current + 1;
	if (current > 2){
		current = 0;
	}
	
	return current;
}

uint8_t getPreviousMenu(uint8_t current){

	if (current == 0){
		current = 2;
	}
	
	else current--; 
		
	return current;
}


void removeIconsMenu(){
	
		GLCD_DisplayString(4, 2, 1, "  Photo Gallery");
	
		GLCD_DisplayString(6, 2, 1, "  MP3 Player");
	
		GLCD_DisplayString(8, 2, 1, "  Game");
	
}


//draw icon hsowing selection

void drawSelectedMenu(uint8_t current){
	
	if (current == 0){GLCD_Bitmap(32,96, 24,24, STAR_pixel_data);}
	
	if (current == 1){GLCD_Bitmap(32,144, 24,24, STAR_pixel_data);}
	
	if (current == 2){GLCD_Bitmap(32,192, 24,24, STAR_pixel_data);}
	
}






