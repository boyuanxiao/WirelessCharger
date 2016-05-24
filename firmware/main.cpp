#include "mbed.h"

#include "config.h"
//define SINE
#define SQUARE

#ifdef SINE //set up if you want to generate a sinusoidal PWM 
static FastPWM fastPWMB = FastPWM(PWMB);
Timer t;
static int idA=0;
static int idB = 0;

void updateDutyCycleA() {
    //sets the duty cycle based on sine value in the array, go to zero if we reach the end of the array.
    fastPWMA.pulsewidth_ticks(ticksA[idA++]);
    idA%=sineSteps;
}

void updateDutyCycleB() {
    //sets the duty cycle based on sine value in the array, go to zero if we reach the end of the array.
    fastPWMB.pulsewidth_ticks(ticksB[idB++]);
    idB%=sineSteps;
}

void timeInterrupt() {
    int i;
    t.start();
    for(i = 0; i<100; ++i) {
        updateDutyCycleA();
    }
    t.stop();
    int duration = (t.read_us())/100;
    printf("Each interrupt takes on average %d us \n", duration);
    t.reset();
}            

void writeDutyCycles() {
    int i;
    for (i = 0; i<sineSteps; ++i) {
        sineDutyA[i] =  (double) (0.5 * (1.0 + sin(i*sineStepsRadians)));
    }
    //write the second array to be such that it is offset by half the number of sineSteps
    int half = sineSteps / 2;
    for (i = 0; i<sineSteps; ++i) {
        #ifdef UNIPOLAR
        sineDutyB[i] = sineDutyA[(i+half)%sineSteps];
        #else
        sineDutyB[i] = sineDutyA[i];
        #endif
    }
}

void writeTicks() {
    int i;
    for(i = 0; i<sineSteps; ++i) {
        ticksA[i] = (uint32_t)(sineDutyA[i] * sysClk/carrierFrequency);
        ticksB[i] = (uint32_t)(sineDutyA[i] * sysClk/carrierFrequency);
    }
}         

void generateSinPWM(PinName *pin, double outputFrequency) {
    if(*pin == PWMA) {
        fastPWMA.period(1.0 / carrierFrequency);
        pwm_tickerA.attach(&updateDutyCycleA, 1.0/(float)(sineSteps * outputFrequency));
    } else {
        fastPWMB.period(1.0 / carrierFrequency);
        pwm_tickerB.attach(&updateDutyCycleB, 1.0/(float)(sineSteps * outputFrequency));

    } 
}
#endif

#ifdef SQUARE
InterruptIn pwmA(p29);
void turnOff() {
    pwm2 = 0;
}

void turnOn() {
    pwm2 = 1;
}
#endif

int main() {
    while (1) {
        //generate a 10kHz wave 'sine' wave if we cannot get any sensible arguments
        char bfr[17];
        char bfrWaveform[5];
        char bfrFrequency[10];       
        //expect either '-sin' or '-sqr', four chars, one char for space, then at least five for the frequency in Hz, plus null
        pc.gets(bfr, 15);
        strncpy(bfrWaveform, bfr, 4);
        bfrWaveform[4] = '\0';
        strncpy(bfrFrequency, bfr+5, 9);
        bfrFrequency[9] = '\0';        
        #ifdef SINE
            //turn on the gate driver
            enable = 1;
            outputFrequency = (double)atof(bfrFrequency);
            carrierFrequency = sineSteps * outputFrequency;
            #ifdef DEBUG
            pc.printf("the frequency as a double is %g\n", outputFrequency);
            #endif
            if(outputFrequency != 0.0) {
                writeDutyCycles();
                writeTicks();
                generateSinPWM(&PWMA, outputFrequency);
                generateSinPWM(&PWMB, outputFrequency);
            } else {
                pc.printf("cannot read frequency value\n");
            }
        #endif
        #ifdef SQUARE
            //turn on the gate driver
            enable = 1;
            outputFrequency = (double)atof(bfrFrequency);
            #ifdef DEBUG
            pc.printf("the frequency as a double is %g\n", outputFrequency);
            #endif
            if(outputFrequency != 0.0) {
                fastPWMA.period(1.0 / outputFrequency);
                fastPWMA.write(0.5);
                pwmA.rise(&turnOff);
                pwmA.fall(&turnOn);
            } else {
                pc.printf("cannot read frequency value\n");
            }
        #endif
    }
}