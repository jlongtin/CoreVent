//////////////////////////////////////////////////////////////////////////////
//																		                                    	//
//     breathing_functions.ino ==> Breathing functions for CoreVent   			//
//																			                                    //
//////////////////////////////////////////////////////////////////////////////


//turn inspiration valve on to start inspiration phase
void turnInspValveON(void) {
	digitalWrite(inspValve, HIGH);			// turn valve on
	delay(10);
	digitalWrite(inspValve, HIGH);			// make sure valve is on 
	delay(10);
	digitalWrite(inspValve, HIGH);			// make damned sure valve is ON
	delay(10);
	digitalWrite(inspLED, HIGH);
}


//turn inspiration valve off to stop inspiration and start expiration
// note that no exhaust valve control is required.  (Breathing circuit has mechanically actuated valve for expiration).
void turnInspValveOFF(void) {
	digitalWrite(inspValve, LOW);			// turn valve on
	delay(10);
	digitalWrite(inspValve, LOW);			// make sure valve is off
	delay(10);
	digitalWrite(inspValve, LOW);			// make damned sure valve is OFF
	delay(10);
	digitalWrite(inspLED,LOW);				// turn off inspiration LED
}


unsigned long startInspirStep(void) {
	float Ps;
	unsigned long t;
	unsigned long Ts = millis();			// capture start of inspiration time
	clearAlarm(ALARM_Ps_HIGH);				// clear any previous high-pressure alarm
	turnInspValveON();						//  open valve
	digitalWrite(inspLED, HIGH);			// turn on inspiration LED

	do {										// fill patient until either P=Pc OR time >= t_TL
		Ps = readCircPress();				// read current pressure
		t = millis();						// current time
		if (Ps > P_highlmt) {				// check for high alarm
			raiseAlarm(ALARM_Ps_HIGH);		
		}
		if ((t - Ts) > t_TL) {				// timeout 
			timeOutCount++;
			raiseAlarm(ALARM_T_TL);
			turnInspValveOFF();
  			break;
		}
		else
		{
			clearAlarm(ALARM_T_TL);
		}
	} while ((Ps <= PIP_s) || (t-Ts) < 250);						// continue to inflate patient until Pc reached

//	if (((t - Ts) > Tdc) && (Ps <= P_lowlmt)) {  	// low pressure alarm (but wait until end of breath first
	if (Ps <= P_lowlmt) {  					// low pressure alarm (but wait until end of breath first
		raiseAlarm(ALARM_Ps_LOW);
	} 
	else
	{
		clearAlarm(ALARM_Ps_LOW);			// clear lowPressLED
	}

	turnInspValveOFF();						// close inspiration valve
	digitalWrite(inspLED, LOW);				// turn off inspiration LED
	PIP_m = readCircPress();					// read measured PIP pressure
	return (t - Ts);						// return inspiration time
}

float startExpStep(unsigned long Te) {		// NOTE: expiratory valve opens automatically when inspiratory valve shuts
	unsigned long Ts = millis();			// capture start of inspiration time
	float Ps;								// read pressure
	do {
		Ps = readCircPress();
		if (Ps < (PEEP_set - P_draw)) {		// patient is drawing for air, so fill immediatley
      asstBreathCnt++;   				// increment 
			digitalWrite(asstBreathLED,HIGH);
			digitalWrite(nrmlBreathLED,LOW);  
			return (Ps);							// terminate expiration immediately, and start inspiration
		}
	} while ((millis() - Ts) < Te);
	digitalWrite(asstBreathLED, LOW);
	digitalWrite(nrmlBreathLED, HIGH);  

	return (Ps);
}
