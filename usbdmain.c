/*----------------------------------------------------------------------------
 * Name:    usbmain.c
 * Purpose: MP3 Player 
 *----------------------------------------------------------------------------*/

#include "LPC17xx.h"
#include "type.h"
#include "GLCD.h"
#include "KBD.h" 
#include "LED.h"
#include "usb.h"
#include "usbcfg.h"
#include "usbhw.h"
#include "usbcore.h"
#include "usbaudio.h"
#include <stdlib.h>
#include <stdio.h>

#define __FI        1  // Font Index 1 = 16x24 pixels

extern void SystemClockUpdate(void);
extern uint32_t SystemFrequency;  
uint8_t  Mute;
uint32_t Volume;

#if USB_DMA
uint32_t *InfoBuf = (uint32_t *)(DMA_BUF_ADR);
short *DataBuf = (short *)(DMA_BUF_ADR + 4*P_C);
#else
uint32_t InfoBuf[P_C];
short DataBuf[B_S];
#endif

uint16_t  DataOut;
uint16_t  DataIn;
uint8_t   DataRun;
uint16_t  PotVal;
uint32_t  VUM;
uint32_t  Tick;

// --- LED VOLUME BAR ---
void update_volume_leds(uint16_t current_vol) {
    int i;
    int num_leds_on;
    
    // Map 0-255 PotVal to 0-8 LEDs
    if (current_vol <= 10) num_leds_on = 0; 
    else {
        num_leds_on = (current_vol * 8) / 240; 
        if (num_leds_on > 8) num_leds_on = 8;
    }

    // LEDs 0 to 7
    for (i = 0; i < 8; i++) {
        if (i < num_leds_on) LED_On(i); 
        else                 LED_Off(i);
    }
}

void get_potval(void) {
    uint32_t val;
    LPC_ADC->CR |= 0x01000000;
    do {
        val = LPC_ADC->GDR;
    } while ((val & 0x80000000) == 0);
    LPC_ADC->CR &= ~0x01000000;
    
    PotVal = ((val >> 8) & 0xF8) + ((val >> 7) & 0x08);
    update_volume_leds(PotVal);
}

// --- GRAPHICS HELPERS ---
// Draws a hollow circle
void draw_circle_hollow(int x0, int y0, int radius, unsigned short color) {
    int x = radius;
    int y = 0;
    int err = 0;
    GLCD_SetTextColor(color);
    while (x >= y) {
        GLCD_PutPixel(x0 + x, y0 + y); GLCD_PutPixel(x0 + y, y0 + x);
        GLCD_PutPixel(x0 - y, y0 + x); GLCD_PutPixel(x0 - x, y0 + y);
        GLCD_PutPixel(x0 - x, y0 - y); GLCD_PutPixel(x0 - y, y0 - x);
        GLCD_PutPixel(x0 + y, y0 - x); GLCD_PutPixel(x0 + x, y0 - y);
        if (err <= 0) { y += 1; err += 2*y + 1; }
        if (err > 0) { x -= 1; err -= 2*x + 1; }
    }
}

// Draws a "Disc" by drawing multiple circles
void draw_disc(int x0, int y0, int radius, unsigned short color) {
    int r;
    // Draw from radius 15 up to 'radius' (Creates a hole in the middle)
    for (r = 15; r <= radius; r++) { 
        draw_circle_hollow(x0, y0, r, color);
    }
}

// --- AUDIO INTERRUPT HANDLER ---
void TIMER0_IRQHandler(void) {
    long val;
    uint32_t cnt;
    
    // *** EXIT STRATEGY ***
    // Check P1.25 (Joystick Down) directly.
    // If LOW (0), reset the board.
    if ((LPC_GPIO1->FIOPIN & (1 << 25)) == 0) {
        NVIC_SystemReset(); 
    }

    if (DataRun) {
        val = DataBuf[DataOut];
        cnt = (DataIn - DataOut) & (B_S - 1);
        if (cnt == (B_S - P_C*P_S)) DataOut++;
        if (cnt > (P_C*P_S)) DataOut++;
        DataOut &= B_S - 1;
        if (val < 0) VUM -= val;
        else         VUM += val;
        val  *= Volume;
        val >>= 16;
        val  += 0x8000;
        val  &= 0xFFFF;
    } else { val = 0x8000; }

    if (Mute) val = 0x8000;
    LPC_DAC->CR = val & 0xFFC0;

    if ((Tick++ & 0x03FF) == 0) {
        get_potval();
        if (VolCur == 0x8000) Volume = 0;
        else                  Volume = VolCur * PotVal;
        val = VUM >> 20;
        VUM = 0;
        if (val > 7) val = 7;
    }
    LPC_TIM0->IR = 1;
}

// --- MAIN MP3 TASK ---
int mp3(void) {
    volatile uint32_t pclkdiv, pclk;
    char vol_text[20];
    int vol_percent;
    volatile int d; 
    int loop_count = 0;
    
    KBD_Init(); 
    LED_Init(); 
    GLCD_Init();
    GLCD_Clear(Black);
    
    // UI Setup
    GLCD_SetBackColor(Blue);
    GLCD_SetTextColor(White);
    GLCD_DisplayString(0, 0, __FI, "   MP3 PLAYER      ");
    
    GLCD_SetBackColor(Black);
    GLCD_SetTextColor(Red);
    GLCD_DisplayString(9, 0, __FI, " DOWN: EXIT ");
    
    // Draw Static Border
    GLCD_SetTextColor(White);
    for(d=10; d<310; d++) { GLCD_PutPixel(d, 40); GLCD_PutPixel(d, 190); }
    for(d=40; d<190; d++) { GLCD_PutPixel(10, d); GLCD_PutPixel(310, d); }
    
    SystemClockUpdate();
    LPC_PINCON->PINSEL1 &= ~((0x03<<18)|(0x03<<20));
    LPC_PINCON->PINSEL1 |= ((0x01<<18)|(0x02<<20));
    LPC_SC->PCONP |= (1 << 12);
    LPC_ADC->CR = 0x00200E04;
    LPC_DAC->CR = 0x00008000;
    
    pclkdiv = (LPC_SC->PCLKSEL0 >> 2) & 0x03;
    switch (pclkdiv) {
        case 0x00: default: pclk = SystemFrequency/4; break;
        case 0x01: pclk = SystemFrequency; break;
        case 0x02: pclk = SystemFrequency/2; break;
        case 0x03: pclk = SystemFrequency/8; break;
    }
    
    LPC_TIM0->MR0 = pclk/DATA_FREQ - 1;
    LPC_TIM0->MCR = 3;
    LPC_TIM0->TCR = 1;
    NVIC_EnableIRQ(TIMER0_IRQn);
    
    USB_Init();
    NVIC_EnableIRQ(USB_IRQn);
    USB_Reset();
    USB_SetAddress(0);
    USB_Connect(TRUE);
    
    // Draw Static Disc ONCE
    draw_disc(160, 115, 50, White);
    
    while(1) {
        // Minimal UI Updates (Throttled)
        if (loop_count++ > 20) {
            loop_count = 0;
            vol_percent = (PotVal * 100) / 255;
            
            GLCD_SetBackColor(Black);
            GLCD_SetTextColor(Yellow);
            sprintf(vol_text, "Vol: %3d%% ", vol_percent);
            GLCD_DisplayString(6, 6, __FI, (unsigned char *)vol_text);
            
            if (DataRun) {
                GLCD_SetTextColor(Green);
                GLCD_DisplayString(7, 6, __FI, "PLAYING ");
            } else {
                GLCD_SetTextColor(LightGrey);
                GLCD_DisplayString(7, 6, __FI, "WAITING ");
            }
        }
        
        // Small delay to prevent 100% CPU usage in main loop
        for (d = 0; d < 20000; d++);
    }
	
	return 0; // Should never reach here, but keeps compiler happy
}