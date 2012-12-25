// MARS WATCH
// December 2012
// Version 2.0
// BY: RYAN PIERCE, rdpierce@pobox.com
//
// TO DO:
// 1. Optimize Mars time algo for smaller code size. It currently takes up over 13k! We're right at the limit of 
//    available flash, and future enhancements will probably depend upon this.
// 2. The Set function currently doesn't know the upper bounds of days in the current month and allows entry of
//    invalid dates.
// 3. Alarm function is likely broken. Ideally it should be set in local time and translated into UTC.
// 4. Entry of leap second info in Set function.
// 5. Consider using EEPROM to store config info like 12/24 (because the RTC is always in 24 hour mode),
//    TZ offset, and leap seconds (when implemented.)
// 6. Possibly consider automatic adjustment for DST and/or a DST mode.
//
// Changes:
// 1. Removed Worm and Text modes.
// 2. Added support for setting and displaying the year.
// 3. RTC now stores time in UTC mode. Time is set in UTC mode. Time is converted to local time zone and possibly
// changed to 12 hour if necessary for display.
// 4. Added modes and changed overall menu flow to: Time (local), UTC Time, Mars Time, Stop, Alarm, Set.
// 5. Mars time displays MTC and number of MSD Sols, and the mission clocks for MSL (Curiousity) and MER-B (Opportunity).
// 6. Enhanced Set mode to add support for UTC offset.
// 7. Removed EEPROM library and associated code to save sketch size.
// 8. Changed Set mode to zero Seconds in the RTC whenever the Minute is set. This allows for accurate time setting.
//    Note, however, that we only check the RTC every 2000 updates, so rollover might have some delay.
// 9. Removed annoying "Nite" text and delay when watch goes to sleep.
// 10. Removed code that resets the time each time the Arduino is rebooted. I'm not sure what function it
//     ever really served, and it makes development more difficult.
// 11. Changed UTC mode so day of week isn't displayed. (Note: The RTC still stores this, and it is important
//     to read it from the RTC, since this gets converted to the local day of the week.)
//
//    DEMO MODE NOTES
//
//    Uncommenting the definition of DEMO at the top of file ST2_Routines will place the watch in a limited
//    functionality mode with a modified Time display (shows MTC time but adjusts for 12 hour and PM flag) that displays
//    local date as sub-modes, Stopwatch, Alarm, and Set modes. The idea is that you can tweak UTC so that MTC is
//    coincident to local time, give it to someone as a gift, and the clock will start to drift as MTC and UTC diverge. 
//    This assumes the recipient won't try to set the time and discover the weird UTC time, and won't use the Alarm 
//    function. (I considered disabling it, but a perspicacious recipient may see the watch's speaker and wonder why
//    it doesn't have an alarm.) Also, the Time mode displays local date; it is entirely possible the date might roll 
//    weirdly; it might help to set the UTC offset so that bogus UTC + offset corresponds within an hour to real local time; 
//    that will minimize the window when the wrong local date is displayed, which might give away the ruse.    

//  *****************************************************************************************************************
//  *                                                                                                               *
//  *                                         SpikenzieLabs.com                                                     *
//  *                                                                                                               *
//  *                                                ST2                                                            *
//  *                                                                                                               *
//  *****************************************************************************************************************
//
// BY: MARK DEMERS 
// May 2012
// VERSION 1.1
//
// Brief:
// Sketch used in the Solder: Time II Watch Kit, more info and build instructions at http://www.spikenzielabs.com/st2
// 
//
// LEGAL:
// This code is provided as is. No guaranties or warranties are given in any form. It is your responsibilty to 
// determine this codes suitability for your application.
// 
// Changes:
// July 23 2012
// Reduced fliker in some lighting conditions.
// 1. Moved Time and Date request during time and date display to top of Display Time routine.
// 2. Reduced frequency that the Time and Date is requested from the RTC (once every 2000 loops).
// 
// Improved brightness consistancy on all columns.
// 1. Added delay to "on time" of all the pixels in the first 16 columns that are driven by the Demultiplexer.
// 2. Adjusted delays to for Scrolling Text and Worm animation to balance slower overall refresh from added brightness delay.
//
// Fixed bug introduced by Arduino 1.0.1
// 1. Added conditional compile in Setup for the button pins. Arduino > 1.0 uses INPUT_PULLUP vs Digitalwrite HIGH.
// 
//

#include <Wire.h>
//#include <EEPROM.h>

#include <TimerOne.h>

#include <avr/sleep.h>
#include <avr/power.h>
#include <avr/wdt.h>

// Worm animation
int c =0;
int y = 3;
int target = 3;
int targdist =0;
boolean targdir = true;
int wormlenght = 15;
boolean soundeffect = false;

// int  i  =0;
// int  i2  =0;
// int  vite = 2;
// uint8_t incro = 0;
// uint8_t column = 0;
uint8_t TEXT = 65;
uint8_t i2cData = 0;
// int nextcounter = 0;

int STATE = 0;
int SUBSTATE = 0;
int MAXSTATE = 6;
boolean NextStateRequest = false;
boolean NextSUBStateRequest = false;
boolean JustWokeUpFlag = false;
boolean JustWokeUpFlag2= false;
boolean OptionModeFlag = false;

int ROWBITINDEX = 0;
int scrollCounter =0;
int ScrollLoops = 3;
int scrollSpeed = 100;                                    // was 1200
int blinkCounter = 0;
boolean blinkFlag = false;
boolean blinkON = true;
boolean blinkHour = false;
boolean blinkMin = false;
#define blinkTime  500                                   // was 1000

boolean displayFLAG = true;

unsigned long SleepTimer;
unsigned long currentMillis;
unsigned long SleepLimit = 6000;
boolean SleepEnable = true;
int UpdateTime = 0;

#define BUTTON1 2
#define MODEBUTTON 2
#define BUTTON2 3
#define SETBUTTON 3
boolean  bval = false;

//char Str1[] = "Hi";
char IncomingMessage[24];
char MessageRead;
//uint8_t INBYTE;
uint8_t Message[275];
int IncomingIndex = 0;
int IncomingMessIndex =0;
int IncomingMax = 0;
int MessagePointer = 0;
int StartWindow = 0;
int IncomingLoaded =0;


char days[7][4] = {
  "Sun","Mon","Tue","Wed","Thr","Fri","Sat"};
char months[12][4] = {
  "Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};

// Time Variables
uint8_t LocalHourTens = 1;
uint8_t LocalHourOnes = 2;
uint8_t MinTens = 0;
uint8_t MinOnes = 0;
uint8_t SecTens = 0;
uint8_t SecOnes = 0;

uint8_t LocalDays =1;
uint8_t LocalDateOnes = 1;
uint8_t LocalDateTens =0;
//uint8_t LocalMonthOnes =1;
//uint8_t LocalMonthTens = 1;
uint8_t LocalYearsOnes = 2;
uint8_t LocalYearsTens = 1;

uint8_t LocalMonthCode =1;

uint8_t UTCHourTens = 1;
uint8_t UTCHourOnes = 2;

uint8_t UTCDays =1;
uint8_t UTCDateOnes = 1;
uint8_t UTCDateTens =0;
uint8_t UTCYearsOnes = 2;
uint8_t UTCYearsTens = 1;

uint8_t UTCMonthCode =1;

boolean Display12Hour = true;
int8_t UTCOffset = -6;

uint8_t AMPMALARMDOTS = 0;

// Alarm
uint8_t AHourTens  = 1;
uint8_t AHourOnes = 2;
uint8_t AMinTens = 0;
uint8_t AMinOnes = 0;

boolean A_TH_Not24_flag = true;
boolean A_PM_NotAM_flag = false;

// StopWatch
  int OldTime = 0;
  int CurrentTime = 0;
  int TotalTime = 0;

uint8_t SWDigit4 = 0;  
uint8_t SWDigit3 = 0;   
uint8_t SWDigit2 = 0; 
uint8_t SWDigit1 = 0; 

int SWMINUTES = 0;
int SWSECONDS = 0;

int dayIndex = 0;

//uint8_t SetDigit  = 4;
//boolean MODEOVERRIDEFLAG = false;
boolean NextStateFlag = false;
boolean SetTimeFlag = false;
boolean ALARM1FLAG = false;
boolean ALARMON = false;

boolean scrollDirFlag = false;


//
volatile uint8_t Mcolumn = 0;
//volatile uint8_t Mrow = 0;
volatile uint8_t LEDMAT[20];

volatile boolean MINUP = false;
volatile boolean MINDOWN = false;
volatile boolean TFH = false;

const int digitoffset = 95;                // 95             // was 16


// Constants
// DS1337+ Address locations
#define		RTCDS1337	B01101000               // was B11010000  
#define		RTCCONT		B00001110		//; Control
#define		RTCSTATUS       B00001111		//; Status

//#define		RTC_HSEC	B00000001		//; Hundredth of a secound
#define		RTC_SEC		B00000000		//; Seconds
#define		RTC_MIN		B00000001		//; Minuites
#define		RTC_HOUR	B00000010		//; Hours

#define		RTC_DAY  	B00000011		//; Day
#define		RTC_DATE	B00000100		//; Date
#define		RTC_MONTH	B00000101		//; Month
#define		RTC_YEAR	B00000110		//; Year

#define		RTC_ALARM1SEC	B00000111		//; Seconds
#define		RTC_ALARM1MIN	B00001000		//; Minuites
#define		RTC_ALARM1HOUR	B00001001		//; Hours
#define         RTC_ALARM1DATE  B00001010               //; Date


// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Bit Map Letter - data array
// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// const byte LETTERS[95][5] = {
const byte LETTERS[106][5] = {
  0,0,0,0,0,            // Space
  0,0,95,0,0,           // !
  0,3,0,3,0,            // "
  18,63,18,63,18,       // #
  4,42,127,42,16,       // $
  34,21,42,84,34,       // %
  54,73,81,34,80,       // &
  0,0,3,0,0,            // '
  0,28,34,65,0,         // (
  0,65,34,28,0,         // (
  4,20,15,20,4,         // *
  8,8,62,8,8,           // +
  0,0,2,1,0,            // ' (not sure)
  8,8,8,8,8,            // -
  0,0,64,0,0,           // .
  32,16,8,4,2,          // /

  62,65,65,65,62,        // 0
  64,66,127,64,64,       // 1
  98,81,73,73,70,        // 2
  34,65,73,73,54,        // 3
  7,8,8,127,8,           // 4
  47,73,73,73,49,        // 5
  62,73,73,73,50,        // 6
  65,33,17,9,7,          // 7
  54,73,73,73,54,        // 8
  6,9,9,9,126,           // 9

  0,0,54,0,0,            // :
  0,64,54,0,0,           // ;
  8,20,34,65,0,          // <
  20,20,20,20,20,        // =
  0,65,34,20,8,          // >
  6,1,81,9,6,            // ?
  62,65,73,85,10,        // @

  124,10,9,10,124,       // A
  127,73,73,73,54,       // B
  62,65,65,65,34,        // C
  127,65,65,65,62,       // D
  127,73,73,73,65,       // E
  127,9,9,9,1,           // F
  62,65,73,73,50,        // G
  127,8,8,8,127,         // H
  0,65,127,65,0,         // I  was   65,65,127,65,65, 
  32,64,65,63,1,         // J
  127,8,20,34,65,        // K
  127,64,64,64,64,       // L
  127,2,4,2,127,         // M
  127,4,8,16,127,        // N
  62,65,65,65,62,        // O
  127,9,9,9,6,           // P
  62,65,81,33,94,        // Q
  127,9,25,41,70,        // R
  38,73,73,73,50,        // S
  1,1,127,1,1,           // T
  63,64,64,64,63,        // U
  31,32,64,32,31,        // V
  63,64,48,64,63,        // W
  99,20,8,20,99,         // X
  3,4,120,4,3,           // Y
  97,81,73,69,67,        // Z

  0,127,65,65,0,         // [
  2,4,8,16,32,           // back slash
  0,65,65,127,0,         // ]
  0,2,1,2,0,             // ^
  128,128,128,128,128,   // _
  0,0,1,2,0,             // `

  112,72,72,40,120,       // a
  126,48,72,72,48,        // b
  48,72,72,72,72,         // c
  48,72,72,48,126,        // d
  56,84,84,84,88,         // e
  0,8,124,10,0,           // f
  36,74,74,74,60,         // g
  126,8,8,8,112,          // h
  0,0,122,0,0,            // i
  32,64,66,62,2,          // j
  124,16,16,40,68,        // k
  0,124,64,64,0,          // l
  120,4,120,4,120,        // m
  124,8,4,4,120,          // n
  48,72,72,72,48,         // o
  126,18,18,18,12,        // p
  12,18,18,18,124,        // q
  124,8,4,4,8,            // r
  72,84,84,84,36,         // s
  4,4,126,4,4,            // t
  60,64,64,64,60,         // u
  28,32,64,32,28,         // v
  124,32,16,32,124,       // w
  68,40,16,40,68,         // x
  4,8,112,8,4,            // y
  68,100,84,76,68,        // z

  0,8,54,65,0,            // {
  0,0,127,0,0,            // |
  0,65,54,8,0,            // }
  16,8,24,16,8,           // ~

  // Small Numbers (for adding colon in clock applications)
  0,62,34,62,0,            // 0
  0,0,62,0,0,              // 1
  0,58,42,46,0,            // 2
  0,42,42,62,0,            // 3
  0,14,8,62,0,             // 4
  0,46,42,58,0,            // 5
  0,62,42,58,0,            // 6
  0,2,2,62,0,              // 7
  0,62,42,62,0,            // 8
  0,14,10,62,0,            // 9
  0,0,20,0,0,              // :



};

const byte GRAPHIC[5][5] = {
  0,0,0,0,0,
  0,28,62,127,0,           // Speaker cone
  34,28,65,34,28,          // Sound wave
  16,32,16,8,4,            // Check mark
  34,20,8,20,34,           // "X"
};

























