///////////////////////////////////////////////////////////////////////////////////
//                                                                               //
//                   CoreVent Controller Code - Setup and Main Loop              //
//                                                                               //
///////////////////////////////////////////////////////////////////////////////////
//                                                                               //
//    Name:       CoreVent.ino                                                   //
//    Created:	  3/29/2020 8:43:43 PM                                           //
//    Author:     Jon Longtin, Ph.D., P.E.                                       //
//                Department of Mechanical Engineering                           //
//                Stony Brook University, Stony Brook, NY 11794-2300             //
//                                                                               //
//                Arduino Platform: Arduino UNO or equivalent                    //
//                                                                               //
//                                                                               //
///////////////////////////////////////////////////////////////////////////////////

// TODO : Add watchdog timer

#include <stdio.h>

// constants
#define VERSION   "0.4.2 - BETA"

//digital pin definitions for alarm indicators and valve control.  
// NOTE: On UNO avoid pins 0 and 1, they are RX and TX fo Serial!
const int alrmSilenceBtn = 2;				// alarm silence button
const int alrmResetBtn = 3;                 // alarm reset button for low-pressure alarm
const int lowPressLED = 4;                  // (red) low-pressure alarm. On until cleared
const int inspTimeOut = 5;                  // (blue) patient has timed out breathing; clear on next breath
const int highPressLED = 6;                 // (yellow) high-pressure alarm
const int inspValve = 7;                    // relay for inspriation valve to start breath (pin 4=Relay #4)
const int nrmlBreathLED = 8;                      // inspiration-start courtesy LED (future feature)
const int asstBreathLED = 9;                      // expiratin-start courtesy LED (future feature)
const int inspLED = 10;                    // (green) self-check OK
const int alrmBuzzer = 11;      //11        // alarm buzzer for low pressure or other critical alarm. NOTE: LOW to sound alarm!!

//alarm conditions; set to false until triggers
unsigned volatile long t_silence = -60000;	// set to "-60000" to ensure buzzer works in first minute of program executionp20
unsigned long timeOutCount = 0;

char incomingByte = 0;                      // for incoming serial data

// ventilator parameters
const int Tdc = 700;                        // time (in ms) to wait before low-pressure alarm trigger
const float minPIP_PEEP = 10;				// cannot set delta between PIP and PEEP less than this
float BPM = 16;							    // breaths per minute (is always an integer)
unsigned int t_TL = 1400;                   // inspiratory timeout  (time cycled limit): if Ps <= Pcby T_TL, start exhale
float PIP_s = 30;                           // peak inspiratory pressure setpoint [cm H2O]
float P_lowlmt = 10;						// pressure les than this indicates a disconnect// make this P_lowlmt
float P_highlmt = 70;						// cutoff pressure (depends on whether 60 or 80 cm H2O popoff used)
float P_draw = 4;							// should be about 2 - 4 cm below PEEP setpoint
float PEEP_set=5;

float PIP_m;								// peak inspiratory pressure (measured, not set)
float PEEP_meas;							// measured PEEP

unsigned long int Ti = 0;                   // time, in ms, of last inspiratory event
unsigned long int Te = 0;                   // time, in ms, of last expitory event
unsigned long Te_act = 0;					// actual expiratory time, as measured				
unsigned long asstBreathCnt = 0;			// assisted breaths since start

// alarm conditins
typedef enum {
	ALARM_Ps_HIGH,                          // sensor pressure > Pc      LOW PRIORITY alarm
	ALARM_Ps_LOW,                           // sensor pressure too low.  HIGH PRIORITY alarm
	ALARM_T_TL                              // inspiration timeout.  Keep track of only; not a major issue
} alarmType;

void setup() {
	attachInterrupt(digitalPinToInterrupt(alrmResetBtn), resetLowP_alarm, FALLING);			// reset alarm button ISR
	attachInterrupt(digitalPinToInterrupt(alrmSilenceBtn), silenceLowP_alarm,FALLING);		// silence alarm button ISR
 
	// setup outputs
	pinMode(lowPressLED, OUTPUT);
	pinMode(inspTimeOut, OUTPUT);
	pinMode(highPressLED, OUTPUT);
	pinMode(inspValve, OUTPUT);
	pinMode(nrmlBreathLED, OUTPUT);
	pinMode(asstBreathLED, OUTPUT);
	pinMode(alrmSilenceBtn, INPUT_PULLUP);
	pinMode(alrmResetBtn, INPUT_PULLUP);
	pinMode(alrmBuzzer, OUTPUT);

	digitalWrite(inspValve, LOW);     // make sure valves are closed at program startup
	
	//LED check at startup
	digitalWrite(alrmBuzzer, LOW); digitalWrite(lowPressLED, HIGH); digitalWrite(inspLED, HIGH);
	digitalWrite(inspTimeOut, HIGH); digitalWrite(highPressLED, HIGH); digitalWrite(nrmlBreathLED, HIGH); digitalWrite(asstBreathLED, HIGH);
	delay(1000);
	digitalWrite(alrmBuzzer, HIGH);  digitalWrite(lowPressLED, LOW); digitalWrite(inspLED, LOW);
	digitalWrite(inspTimeOut, LOW); digitalWrite(highPressLED, LOW); digitalWrite(nrmlBreathLED, LOW); digitalWrite(asstBreathLED, LOW);

	digitalWrite(inspValve, LOW);     // make sure valve is closed at program startup
	
	Serial.begin(57600); // set data rate to 57600 bps
}

void loop() {
	unsigned long Te_start;

	Ti = startInspirStep();			// start inspiration step

	Te = (60. / BPM) * 1000 - Ti; 	// calculate target expitory time based on Ti
	Te_start = millis();			// record start time of expiration
	updateScreen();
	PEEP_meas = startExpStep(Te);  	// start expiration step, returning PEEP pressure (may be too low if patient draws breath)
	Te_act = millis() - Te_start;	// actual expiratory time (will be shorter if patient draws breath)

	
 }
