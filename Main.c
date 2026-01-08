/*----------------------------------------------------------------------------
 * Name:    Blinky.c
 * Purpose: LED Flasher and Graphic Demo
 * Note(s): 
 *----------------------------------------------------------------------------
 * This file is part of the uVision/ARM development tools.
 * This software may only be used under the terms of a valid, current,
 * end user licence from KEIL for a compatible version of KEIL software
 * development tools. Nothing else gives you the right to use this software.
 *
 * This software is supplied "AS IS" without warranties of any kind.
 *
 * Copyright (c) 2008-2011 Keil - An ARM Company. All rights reserved.
 *----------------------------------------------------------------------------*/
         
#include <stdio.h>         
#include <LPC17xx.H>
#include <stdint.h>/* NXP LPC17xx definitions            */
#include "string.h"
#include "GLCD.h"
#include "LED.h"
#include "KBD.h"
#include "Menu.h"
#include "PhotoLib.h"

//#include "usbdmain.h"
#include "game.h"
#define PHOTO_GAL 0
#define MP3 1
#define GAME 2

//#include "star.h"
extern int mp3();
extern unsigned char ClockLEDOn;
extern unsigned char ClockLEDOff;
extern unsigned char ClockANI;
extern unsigned char HEADINGG_pixel_data[];

//extern unsigned char Bg_16bpp_t[];
//extern unsigned char Bg_16bpp_l[];
//extern unsigned char Bg_16bpp_r[];
//extern unsigned char Bg_16bpp_b[];
//extern unsigned char ARM_Ani_16bpp[];
int i = 0;

extern unsigned char STAR_pixel_data[];

void reset(){

	//char s[] = "test";
  //LED_Init ();
 // GLCD_Init();
	//KBD_Init();
	GLCD_SetBackColor(White);
	GLCD_SetTextColor(Black);

  GLCD_Clear  (White);
	GLCD_Bitmap (  0,   0, 320,  69, HEADINGG_pixel_data); //top header 320x69
 // GLCD_Bitmap (  0,  69,   4, 102, Bg_16bpp_l+70);
 // GLCD_Bitmap (316,  69,   4, 102, Bg_16bpp_r+70);
  //GLCD_Bitmap (  0, 171, 320,  69, Bg_16bpp_b+70);

  SysTick_Config(SystemCoreClock/50);  /* Generate interrupt every 20 ms     */

	GLCD_DisplayString(4, 4, 1, "Photo Gallery");
	GLCD_DisplayString(6, 4, 1, "MP3 Player");
	GLCD_DisplayString(8, 4, 1, "Game");
	GLCD_Bitmap(32,96, 24,24, STAR_pixel_data);

}
//extern unsigned char IMG1[];

/*----------------------------------------------------------------------------
  Main Program
 *----------------------------------------------------------------------------*/
int main (void) {                       /* Main Program                       */
  int num     = -1; 
  int dir     =  1;
  int pic     =  0;
	//uint32_t j = 10;
	
	uint8_t menuSelect = 0; //0=photo, 1=mp3, 2=Game
	
	//char s[] = "test";
  LED_Init ();
  GLCD_Init();
	KBD_Init();

  GLCD_Clear  (White);
	GLCD_Bitmap (  0,   0, 320,  69, HEADINGG_pixel_data); //top header 320x69
 // GLCD_Bitmap (  0,  69,   4, 102, Bg_16bpp_l+70);
 // GLCD_Bitmap (316,  69,   4, 102, Bg_16bpp_r+70);
  //GLCD_Bitmap (  0, 171, 320,  69, Bg_16bpp_b+70);

  SysTick_Config(SystemCoreClock/50);  /* Generate interrupt every 20 ms     */

	GLCD_DisplayString(4, 4, 1, "Photo Gallery");
	GLCD_DisplayString(6, 4, 1, "MP3 Player");
	GLCD_DisplayString(8, 4, 1, "Game");
	GLCD_Bitmap(32,96, 24,24, STAR_pixel_data);
	//GLCD_DisplayString(7, 0, 1, s);
	
  for (;;) {                            /* Loop forever                       */
    if (ClockANI) {
      ClockANI = 0;
      if (pic++ > 8) pic = 0;
       // GLCD_Bitmap (99, 99, 120, 45, &ARM_Ani_16bpp[pic*(120*45*2)]);

			//320x260 -> turn into 160x130
			//make upscaler, allow put pixel values to 
			
    }

    if (ClockLEDOn) {    /* Blink LED every 1 second (for 0.5s)*/
      ClockLEDOn  = 0;

      /* Calculate 'num': 0,1,...,LED_NUM-1,LED_NUM-1,...,1,0,0,...           */
      num += dir;
      if (num == LED_NUM) { dir = -1; num =  LED_NUM-1; } 
      else if   (num < 0) { dir =  1; num =  0;         }
    
      LED_On (num);
			
		
		if (get_button() == KBD_DOWN){
			
			menuSelect = getNextMenu(menuSelect);
			removeIconsMenu();
			drawSelectedMenu(menuSelect);

		}
			
		if (get_button() == KBD_UP){
			
			menuSelect = getPreviousMenu(menuSelect);
			removeIconsMenu();
			drawSelectedMenu(menuSelect);

		}
		
		if (get_button() == KBD_SELECT){
		
			if (menuSelect == PHOTO_GAL){

			startPhotoGallery();
			menuSelect = 0;	
			reset();
			continue;

	
			}
				
			if (menuSelect == MP3){
			
			GLCD_Clear  (White);
			GLCD_DisplayString(6, 4, 1, "MP3 Player");	
			mp3();
			menuSelect = 0;	
			reset();
			continue;
			}
				
			if (menuSelect == GAME){
			game();
			menuSelect = 0;	
			reset();
			}
		
		}
		
		//	GLCD_Bitmap (  0,   0, 320,  69, Bg_16bpp_t+70);
		if (get_button() == KBD_LEFT){
		
		//	GLCD_SetTextColor(Blue);
			
		}
		
		//GLCD_PutPixel(i,i);
		//puts pixel on screen, changes colour for new pixels with set text colour
		//i++;
		
    }
    if (ClockLEDOff) {
      ClockLEDOff = 0;

      LED_Off(num);
    }
		
		


			
		
		
	}
	
	
}