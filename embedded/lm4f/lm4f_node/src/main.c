#include <stdlib.h>                     // atoi
#include <inc/hw_types.h>		        // tBoolean
#include <RASLib/inc/init.h>            // InitializeMCU
#include <RASLib/inc/uart.h>            // InitializeUART, Printf
#include <RASLib/inc/rasstring.h>       // SPrintf
#include <RASLib/inc/encoder.h>
#include <RASLib/inc/time.h>
#include <RASLib/inc/gpio.h>
#include <RASLib/inc/motor.h>
#include <RASLib/inc/json_protocol.h>
#include <RASLib/inc/servo.h>

#include <StellarisWare/inc/hw_memmap.h>
#include <StellarisWare/inc/hw_sysctl.h>
#include <StellarisWare/driverlib/sysctl.h>
#include <StellarisWare/inc/hw_watchdog.h>
#include <StellarisWare/driverlib/watchdog.h>

#include "luddef.h"
#include "pid.h"
#include "vel_control.h"
#include "handlers.h"

/**
*	Publisher array for IGVC 2014 LM4F uController.
*		Used in Main for initializations.
*
*	subArray:	SPLM - Set Power Left Motor=>int POWER
*				SPRM - Set Power Right Motor=>int POWER
*				SVLX - Set velocity of x axis(2x desired encoder ticks per message=>int VELOCITY
*		  		SVAZ - Set velocity of about Z (2x desired encoder ticks per message)=>int ANGVELOCITY
*			 	RSTE - Reset Encoder Values=>NONE
*
*	pubArray:	Publishes state information of uController to debug topic.
*/
tSub SPLM,SPRM,SVLX,SVAX,RSTE;
tPub SPLMdebug, SPRMdebug, SVLXdebug, SVAXdebug;
tSub* subArray[NUMSUB] = {&SPLM, &SPRM, &SVLX, &SVAX, &RSTE};
tPub* pubArray[NUMPUB] = {&SPLMdebug, &SPRMdebug, &SVLXdebug, &SVAXdebug};

/**
*	Array of jsonkey strings for initializations
*/
char* subKey[NUMSUB] = {"SPLM", "SPRM", "SVLX", "SVAX", "RSTE"};
char* pubKey[NUMPUB] = {"SPLMdebug", "SPRMdebug", "SVLXdebug", "SVAXdebug"};

tMotor *left_motor, *right_motor;
tEncoder *left_encoder, *right_encoder;
tBoolean LED=true;

tPID pidLeft = {0};
tPID pidRight = {0};

extern float leftPower, rightPower;

void blinkLED(void) {
    SetPin(PIN_F2, LED);
    LED = !LED;
}

void WatchdogHandler(void) {
    WatchdogIntClear(WATCHDOG_BASE);
    WatchdogResetDisable(WATCHDOG_BASE);

    SetMotor(left_motor, 0);
    SetMotor(right_motor, 0);
    SetPin(PIN_F1, true);
}

void WatchDog_Init(void) {
    SetPin(PIN_F1, false);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_WDOG);  // Watchdog timer
    WatchdogIntClear(WATCHDOG_BASE);
    //WatchdogStallEnable(WATCHDOG_BASE);   // Enable for breakpoint debugging
    if(WatchdogLockState(WATCHDOG_BASE) == true) {
        WatchdogUnlock(WATCHDOG_BASE);
    }

    WatchdogIntRegister(WATCHDOG_BASE, WatchdogHandler);

    //Motor control node communicates at 10Hz, if no command for 20Hz halts
    WatchdogReloadSet(WATCHDOG_BASE, 2500000);   

    WatchdogIntEnable(WATCHDOG_BASE);
    WatchdogResetDisable(WATCHDOG_BASE);   //Second interrupt doesn't result processor

    WatchdogEnable(WATCHDOG_BASE);
}

int main(void) {
    int i;
    InitializeMCU();

    left_motor = InitializeServoMotor(PIN_A6, false);
    right_motor = InitializeServoMotor(PIN_A7, false);

    right_encoder = InitializeEncoder(PIN_E4, PIN_E5, false);   
    left_encoder = InitializeEncoder(PIN_B0, PIN_B1, false);

    ResetEncoder(left_encoder);
    ResetEncoder(right_encoder);

    CallEvery(blinkLED,0,0.25f);
	// Initialize subscribers
	// subHandlers Array located in handlers.c
    for(i=0;i<NUMSUB;i++) {
	    InitializeSubscriber(subArray[i], subKey[i], 0, subHandlers[i]);
    }
  
	// Initialize publishers
    for(i=0;i<NUMPUB;i++) {
    	InitializePublisher(pubArray[i], pubKey[i], 0, pubHandlers[i]);
    }

    WatchDog_Init();
    
    // Initialize PID and velocity control 
    InitializePID(&pidLeft, .00015, 0.000, 0.0001, -0.15, 0.15);
    InitializePID(&pidRight, .00015, 0.000, 0.0001, -0.15, 0.15);
    
    BeginPublishing(.1);
    BeginSubscribing(.1); //while(1) contained within BeginSubscribing
    while(1);
}
