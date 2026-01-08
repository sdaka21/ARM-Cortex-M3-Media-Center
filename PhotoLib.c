#include <stdio.h>
#include "LPC17xx.h"                        
#include "GLCD.h"
#include "LED.h"
#include "KBD.h"

#define __FI         1                     

// External Image Declarations
// (These work because you successfully removed 'static' from the image files!)
extern const unsigned char COLEPALMER_pixel_data[];
extern const unsigned char BRONBRON_pixel_data[];
extern const unsigned char BABYMEME_pixel_data[];
extern const unsigned char COLESTARE_pixel_data[];

// Arrow Helpers
void animateRightArrow(void) {
    GLCD_SetBackColor(White);
    GLCD_SetTextColor(White);
    GLCD_DisplayString(5, 17, __FI, " > ");
}

void animateLeftArrow(void) {
    GLCD_SetBackColor(White);
    GLCD_SetTextColor(White);
    GLCD_DisplayString(5, 0, __FI, " < ");
}

// --- RENAMED FUNCTION ---
// Matches the call in main.c
void startPhotoGallery(void) {    
    uint32_t photoG_joystick = 0U;
    int currentImage = 0;
    
    unsigned char* images[] = {
        (unsigned char *)COLEPALMER_pixel_data,
        (unsigned char *)BRONBRON_pixel_data,
        (unsigned char *)BABYMEME_pixel_data,
        (unsigned char *)COLESTARE_pixel_data
    };

    int imageCount = sizeof(images) / sizeof(images[0]);

    LED_Init();
    GLCD_Init();
    GLCD_Clear(White);
            
    for (;;) {
        // UI Setup
        GLCD_SetBackColor(Black);
        GLCD_SetTextColor(Red);
        GLCD_DisplayString(0, 0, __FI, "  Scroll Through!  ");

        // Draw Image
        if (images[currentImage] != NULL) {
            GLCD_Bitmap(75, 50, 180, 180, images[currentImage]);
        }

        // Clear Arrows
        GLCD_SetBackColor(White);
        GLCD_SetTextColor(Blue);
        GLCD_DisplayString(5, 0, __FI, "  ");
        GLCD_DisplayString(5, 17, __FI, "  ");
        
        // Input Loop
        while (1) {
            photoG_joystick = get_button();
            
            if (photoG_joystick == KBD_RIGHT) { 
                animateRightArrow();    
                currentImage = (currentImage + 1) % imageCount;
                break; 
            } 
            else if (photoG_joystick == KBD_LEFT) { 
                animateLeftArrow();    
                currentImage = (currentImage + imageCount - 1) % imageCount;
                break; 
            } 
            else if (photoG_joystick == KBD_DOWN) {
                GLCD_Clear(White);
                return; // Return to Main Menu
            }
        }
        while (get_button() != 0) { }
    }
}