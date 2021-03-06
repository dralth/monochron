/* ***************************************************************************
// anim.c - the main animation and drawing code for MONOCHRON
// This code is distributed under the GNU Public License
//		which can be found at http://www.gnu.org/licenses/gpl.txt
//
**************************************************************************** */

#include <avr/io.h>      // this contains all the IO port definitions
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>
#include <avr/wdt.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
 
#include "util.h"
#include "ratt.h"
#include "ks0108.h"
#include "ks0108conf.h"
#include "glcd.h"

#ifdef INTRUDERCHRON
//#include "font5x7.h"

// 2010-03-03 First Version
//            Requires: Patch to glcd.c routine glcdWriteCharGr(u08 grCharIdx)
//                      Font data in fontgr.h
//
// 2010-08-06 Version 2 - Integration into MultiChron

#define InvaderTimer 5                    // Smaller to make them move faster

//Routines called by dispatcher
void initanim_int(void);                  // Initialzes Animation
void initdisplay_int(uint8_t);            // Intializes Display
void step_int(void);                      // Moves Invaders
void drawdisplay_int(uint8_t);            // Draws Invaders
void setscore_int(uint8_t);               // Updates Time

//Local Routines
void WriteInvaders_int(uint8_t);          // Displays Invaders
void WriteBases_int(uint8_t);             // Displays Bases
void WriteTime_int(uint8_t);              // Displays Time             
void WriteDigits_int(uint8_t, uint8_t);   // Displays a Set of Digits


uint8_t pInvaders=1;                  // Invader's X Position
uint8_t pInvadersPrevious=0;          // Previous Invader's Position
int8_t  pInvadersDirection=1;         // Invader's Direction 1=Right, -1=Left
uint8_t Frame=0;                      // Current Animation Frame 
uint8_t Timer = InvaderTimer;         // Count down timer so they don't move rediculously fast
uint8_t left_score, right_score;      // Store For score
uint8_t left_score2, right_score2;    // Storage for player2 score

extern volatile uint8_t time_s, time_m, time_h;
extern volatile uint8_t old_m, old_h;
extern volatile uint8_t date_m, date_d, date_y;
extern volatile uint8_t alarming, alarm_h, alarm_m;
extern volatile uint8_t time_format;
extern volatile uint8_t region;
extern volatile uint8_t score_mode;
extern volatile uint8_t second_changed, minute_changed, hour_changed;

uint8_t digitsmutex_int = 0;
uint8_t last_score_mode2 = 0;
uint8_t wasalarming = 0; // flag to indicate resetting bases from reverse video is required
void initanim_int(void) {
#ifdef DEBUGF
  DEBUG(putstring("screen width: "));
  DEBUG(uart_putw_dec(GLCD_XPIXELS));
  DEBUG(putstring("\n\rscreen height: "));
  DEBUG(uart_putw_dec(GLCD_YPIXELS));
  DEBUG(putstring_nl(""));
#endif
  pInvaders = 1;
  pInvadersPrevious=0;
  initdisplay_int(0);
 }

void initdisplay_int(uint8_t inverted) {
  // clear screen
  glcdFillRectangle(0, 0, GLCD_XPIXELS, GLCD_YPIXELS, inverted);
  // get time & display
  last_score_mode2 = 99;
  setscore_int(inverted);
  WriteTime_int(inverted);
  // display players 
  WriteInvaders_int(inverted);
  // Show the bases, 1 time only
  WriteBases_int(inverted);
}

void step_int(void) {
 if (--Timer==0) 
  {
  Timer=InvaderTimer;
  pInvadersPrevious = pInvaders;
  pInvaders += pInvadersDirection;
  if (pInvaders > 31) {pInvadersDirection=-1;}
  if (pInvaders < 1) {pInvadersDirection=1;}
  Frame = !Frame;
  }
}

void drawdisplay_int(uint8_t inverted) {
    WriteInvaders_int(inverted);
    if (alarming || wasalarming) {
     WriteBases_int(inverted);
     wasalarming=alarming;
    }
    setscore_int(inverted);
    return;
}

void setscore_int(uint8_t inverted) {
   if (minute_changed || hour_changed || last_score_mode2 != score_mode) {
   	   minute_changed = hour_changed = 0;
   if (! digitsmutex_int) {
    digitsmutex_int++;
    last_score_mode2 = score_mode;
    left_score = hours(time_h);
    left_score2 = time_m;
    if (score_mode == SCORE_MODE_ALARM) {
     right_score = hours(alarm_h);
     right_score2 = alarm_m;
    } 
    else if (score_mode == SCORE_MODE_DATE) {
      right_score = 20;
      right_score2 = date_y;
    } 
    else if (region == REGION_US) {
     right_score = date_m;
     right_score2 = date_d;
    } 
    else {
     right_score = date_d;
     right_score2 = date_m;
    }
    digitsmutex_int--;
    WriteTime_int(inverted);
   }
  }
}

void WriteInvaders_int(uint8_t inverted) {
  uint8_t j;
  uint8_t i;
  // Clear Previous
  if (pInvadersPrevious > pInvaders) {i=pInvaders+96;}
  else {i=pInvaders-1;}
  glcdFillRectangle(i, 8, 1, 48, inverted);
  // Draw Current
  for (i=0;i<6;i++){
   for (j=0;j<6;j++){
    glcdSetAddress(pInvaders + (j*16), 1+i);
	glcdWriteCharGr(FontGr_INTRUDER_TRIANGLE_UP+(i/2)+(Frame*3),inverted);
   }
  }
}

void WriteBases_int(uint8_t inverted) {
  for (uint8_t i=0;i<4;i++) {
    glcdSetAddress(20 + (i*24), 7);
    glcdWriteCharGr(FontGr_INTRUDER_BASE,inverted);
   }
}

void WriteTime_int(uint8_t inverted) {
 	 glcdSetAddress(0,0);
	 printnumber(left_score,inverted);
     printnumber(left_score2,inverted);
     glcdSetAddress(102,0);
	 printnumber(right_score,inverted);
	 printnumber(right_score2,inverted);
}

#endif
