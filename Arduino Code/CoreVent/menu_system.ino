//////////////////////////////////////////////////////////////////////////////
//																		                                    	//
//     menus_system.ino ==> menu system routines CoreVent		            		//
//															                                    				//
//////////////////////////////////////////////////////////////////////////////


//
//    FILE: menu_system.ino
//  AUTHOR: Jon Longtin
//    DATE: 04-Apr-2020
//
// PUPROSE: menu functions for CoreVent system
//
#define MENU_TIME_OUT  15000L			// menu timeout (ms)
#define UPDATE_DELAY 50									// update time in ms

#define VT100_H

// http://ascii-table.com/ansi-escape-sequences-vt-100.php

#define OFF         "\033[0m"
#define BOLD        "\033[1m"
#define LOWINTENS   "\033[2m"  //does not work
#define UNDERLINE   "\033[4m"
#define BLINK       "\033[5m"  // does not work
#define REVERSE     "\033[7m"

#define GOTOXY( x,y) "\033[x;yH"   // Esc[Line;ColumnH //works with hard numbers, not with arguments

//#define CLS          "\033[2J"     // Esc[2J Clear entire screen
#define CLS          "\033[H\033[J"     // Esc[2J Clear entire screen


float getNumInput(float origValue) {
	const unsigned long menu_timeout = 3000;  // allowed time to make an entry
	unsigned long t_entry = millis();       // time that request to enter value started
	String inData = "";
	do  {
		while (Serial.available() > 0)
		{
			char received = Serial.read();
			inData += received;
			Serial.print(received);   // Process message when new line character is recieved
			if (received == 27) return origValue;  // user aborted 
			else if ((received == 13))
			{
//				if (inData.toInt() == 0) {		// prevents entering zero values. Removed 4/7/20
//					return origValue;
//				}
//				else
//				{
					return inData.toInt();
//				}
			}
		}
	} while ((millis() - t_entry) < menu_timeout);
	return origValue;
}

void paintScreen(void){
	Serial.print(CLS);  // clear screen
	Serial.println(F("CoreVent Essential Ventilator")); 
	Serial.print(F("Version: "));
	Serial.print(VERSION);
	Serial.print(F(", built: "));
	Serial.print(__TIME__);
	Serial.print(F(" "));
	Serial.println(__DATE__);
	Serial.println(F("\n------------- SETTINGS ------------------"));
	Serial.print(  F("  Breaths Per Minute set         : "));
	Serial.println(BPM,0);
	Serial.print(  F("  Peak inspiratory Pressure (PIP): "));
	Serial.println(PIP_s,0);
	Serial.print(  F("  PEEP Valve setting             : "));
	Serial.println(PEEP_set, 0);
	Serial.print(  F("  Sensitivity                    : "));
	Serial.println(-P_draw, 0);
	Serial.println(F("\n------------- BREATH STATS --------------"));
  Serial.print(  F("  Breaths per minute (actual)    : "));
  Serial.println((int)(EMA_BPM(60000./(Ti+Te_act))));
	Serial.print(  F("  Inspiratory time, Ti (sec)     : "));
	Serial.println(Ti/1000.,1);
	Serial.print(  F("  Expiratory time,  Te (sec)     : "));
	Serial.println(Te_act/1000.,1);
    Serial.print(  F("  I/E ratio                      : 1:"));
    Serial.println((float) Te/Ti,1); 
    Serial.print(  F("  PIP                            : "));
    Serial.println(PIP_m,1);
	Serial.print(  F("  PEEP (measured)                : "));
	Serial.println(PEEP_meas,1);
    Serial.println(F("-----------------------------------------"));
	Serial.println(F("Available commands:"));
	Serial.println(F("(B)PM, (P)ressure set, P(E)EP valve setting, (S)ensitivity:"));
}


void updateScreen(void)  //must make changes before patient stops 
{
	char cmd = 0;   // for incoming serial data
	paintScreen();
	if (Serial.available()) {							// user has pressed a key on Serial terminal, so go to menu
		cmd = Serial.read();  // read the incoming byte:
		// say what you got:
		    //Serial.print(F("I received: "));
		    //Serial.println(cmd, DEC);

		switch (cmd) { // convert to uppercase
			case 'B': case 'b': {
				Serial.print(F("\nEnter new BPM (1 ~ 40): "));
				float tBPM = getNumInput(BPM);
				//				Serial.println();
				if ((tBPM >= 1) && (tBPM <= 40)) {  // don't change BPM unless in safe range 
					BPM = tBPM;
				}
				break;
			}

			case 'P': case 'p': {		// enter new target pressure
				Serial.print(F("\nEnter new Pc (10 ~ 70): "));
				float tPc = getNumInput(PIP_s);
				if ((tPc >= 10) && (tPc <= 70)) {  // don't change Pc unless in safe range 
					if (tPc < (PEEP_set + minPIP_PEEP)) {
						Serial.print(F("\nError: PIP must be at least "));
						Serial.print(minPIP_PEEP, 0);
						Serial.println(F("cm H2O above PEEP!"));
					}
					else {
						PIP_s = tPc;
					}
				}
				break;
			}
			case 'E': case 'e': {		// enter new target pressure.  NOTE: This does NOT change PEEP!! 
				Serial.print(F("\nEnter new PEEP valve setting (0 ~ 40): "));
				float tPEEP = getNumInput(PEEP_set);
				if ((PEEP_set >= 0) && (PEEP_set <= 40)) {  // don't change PEEP unless in safe range 
					if (tPEEP > (PIP_s - minPIP_PEEP)) {
						Serial.print(F("\nError: PEEP must be at least "));
						Serial.print(minPIP_PEEP, 0);
						Serial.println(F("cm H2O below PEEP!"));
					}
					else {
						PEEP_set = tPEEP;
					}
				}
				break;
			}

			case 'S': case 's': {		// enter new target pressure.  NOTE: This does NOT change PEEP!! 
				Serial.print(F("Enter new sensitivty (0 ~ 6): "));
				float tP_draw = getNumInput(P_draw);  // make positive in case user enters negative value
			    if (tP_draw < 0) {
					tP_draw = -tP_draw;
				}
				if ((tP_draw >= 0) && (tP_draw <= 6)) {  // don't change PEEP unless in safe range 
					P_draw = tP_draw;
				}
				break;
			}
		}
	}
}
