#include "FastPWM.h"
//comment out to remove printfs
#define DEBUG 

//macroprocessor: define how many steps to divide the sine wave into
#define sineSteps 5

//if this is defined, then both pins will be fired: will require the PCB where both PWM pins are used
#define UNIPOLAR

//number of divisions in the carrier wave (int)
double pi = 3.1415926535897;
//sineSteps in radians
double sineStepsRadians = 2.0 * pi / sineSteps;
//carrier wave frequency, must be greater than output frequency * sine steps;
double carrierFrequency;
//array carrying the value of of a normalised sine wave sampled at sineSteps points
double sineDutyA[sineSteps];
double sineDutyB[sineSteps];
double sysClk = 96000.0*1000.0; //mBed clock of 96MHz

uint32_t ticksA[sineSteps];
uint32_t ticksB[sineSteps];

uint32_t sqrTicksA[2];
uint32_t sqrTicksB[2];

 double outputFrequency, defaultOutputFrequency = 10000;
//ticker to trigger a change in the pwm duty cycle
Ticker pwm_tickerA;
Ticker pwm_tickerB;
Ticker unipolarDelay;

//PWM pin 1
PinName PWMA = p26;
//PWM pin 2
PinName PWMB = p27;
//Disable pin, set high to pull all outputs low
PinName ENB = p30;
DigitalOut pwm2(PWMB, 1);
//object for communicating with pc over serial
Serial pc(USBTX, USBRX);
 
//global PWM object pointer
static FastPWM fastPWMA = FastPWM(PWMA); 


//initialise the disable pin to high (outputs all low)
DigitalOut enable(ENB, 0);