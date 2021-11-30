//////////////////////////////////////////////////////////////////////////////
//																			                                    //
//          support_functions.ino ==> support functions for CoreVent      	//
//																		                                    	//
//////////////////////////////////////////////////////////////////////////////


float readCircPress(void) {
	const float calibOffset = 0.20;   // cm of water offset when sensor exposed to ambient

	// GA100-001PD Sensor (0-1 psi range, 0-70.307 cm H2O))
	// sensor span is 4.0 V for 1.0 psi = 70.307 cm H2O, or 70.307/4.0 = 17.577 [cm H2O/volt]
	// ADC yields 1024 bits over 0-5V, so converting ADC_RDG to voltage is:
	//    Vs = ADC_RDG * (5.0/1024)
	// then pressure is P = (Vs - 0.5)*17.577 [cmH2O/V]  (note 0.5 V offset!)
	// Apply pressure to port A (port closest to pins)
	// Description: black, 2 pressure ports, 3 pins, 4 legs
	// Orientation: pressure ports up, connector pins facing you
	// left to right => pin 1 to pin 3
	// pin 1: PWR SPLY (Vin = 5V, allowable range = 4.75-5.25V) (tap from Arduino pin 
	// pin 2: GND
	// pin 3: Vout (range = 0.5 - 4.5V; connect to Analog Input A0 on arduino)

	// leaving multiplication and division in: 2135 rdgs/sec (Uno R3)
	// folding multiplication and divisions  : 2280 rdgs/sec (Uno R3) = 

	float Vs = analogRead(A0) * 4.883e-3;			// sensor pin 3 to Arduino pin A0 
	return (Vs*17.5767 - 8.78835 + calibOffset);	// return pressure in cm H2O
}

//ISR to reset low-pressure alarm 
void resetLowP_alarm(void){
	digitalWrite(lowPressLED, LOW);			// clear low pressure light 
}

//ISR to silence the low-pressure alarm
void silenceLowP_alarm(void) {
	if ((millis() - t_silence) > 60000) {	// start 60 second timer AND do not allow multiple resets!
		digitalWrite(alrmBuzzer, HIGH);		// turn the damned buzzer off for 60 seconds 
		t_silence = millis();				// grab current time
	}
}

void raiseAlarm(alarmType alm) {
	switch (alm) {

	case ALARM_Ps_HIGH:
		digitalWrite(highPressLED, HIGH);		// turn on low-pressure light
		digitalWrite(alrmBuzzer,LOW);			// sound alarm
//		Serial.println(F("ALARM: pressure too high. P>P_highlmt"));
		break;

	case ALARM_Ps_LOW:
		digitalWrite(lowPressLED,HIGH);			// turn on low-pressure light.  Button must be pressed to clear
		if ((millis() - t_silence) > 60000) {	// it has been >60 s, so sound alarm
			digitalWrite(alrmBuzzer, LOW);		// sound alarm. Button MUST be pressed to clear this
 		}
//		Serial.println(F("ALARM: pressure too low. P<P_lowlmt"));
		break;

	case ALARM_T_TL:
		digitalWrite(inspTimeOut, HIGH);		// turn on inspiration time out LED
//		Serial.println(F("ALARM: inpiration time out (>T_TL)"));
		break;
	}
}

void clearAlarm(alarmType alm) {
	switch (alm) {

	case ALARM_Ps_HIGH:
		digitalWrite(highPressLED, LOW);		// turn off high-pressure alarm
		break;

	case ALARM_Ps_LOW:
		digitalWrite(alrmBuzzer, HIGH);			// turn off buzzer
		break;

	case ALARM_T_TL:
		digitalWrite(inspTimeOut, LOW);
		break;
	}
}

// Exponential Moving Average filter for breaths per minute...
float EMA_BPM(float p)
{
	const float FILTER_TC = 3.0;       // time in samples to reach ~ 63% of a steady value, Rise time of a signal to > 90% is approx 2.5 times this value;
	const float FILTER_WEIGHT(FILTER_TC / (FILTER_TC + 1.0));
	static float avg = BPM;

	avg = (FILTER_WEIGHT*avg) + (1.0 - FILTER_WEIGHT)*p;
	return avg;
}
