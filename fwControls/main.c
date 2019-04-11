/*********************************************************************
 *               Ground Noise Motor 
 * .board: StepGN (FraiseStep 1.2 + 16MHz quartz)
 *  Antoine Rousseau @ metalu.net - feb.2019
 *********************************************************************/

#define BOARD Step1.2

#include <fruit.h>
#include <analog.h>
#include <switch.h>
#include <eeparams.h>


t_delay mainDelay;


void setup(void) {	
//----------- Setup ----------------
	fruitInit();
			
	//pinModeDigitalOut(LED); 	// set the LED pin mode to digital out
	//digitalClear(LED);		// clear the LED
	delayStart(mainDelay, 5000); 	// init the mainDelay to 5 ms

//----------- Analog setup ----------------
	analogInit();		// init analog module
	analogSelect(0, POT_HARD);
	analogSelect(1, POT_AMBIANCE);
	analogSelect(2, POT_MIC);
	analogSelect(3, POT_FX);

//----------- Switch setup ----------------
	switchInit();		// init analog module
	switchSelect(0, SWITCH_START);
	switchSelect(1, SWITCH_STOP);
	switchSelect(2, SWITCH_REWIND);

//----------- power leds setup ----------------

	pinModeDigitalOut(MAEN); 	// enable motor driver A
	digitalSet(MAEN);
	pinModeDigitalOut(MBEN); 	// enable motor driver B
	digitalSet(MBEN);
// power leds
	pinModeAnalogOut(PWLED1);
	pinModeAnalogOut(PWLED2);
	pinModeAnalogOut(PWLED3);
	pinModeAnalogOut(PWLED4);
	analogWrite(PWLED1, 0);
	analogWrite(PWLED2, 0);
	analogWrite(PWLED3, 0);
	analogWrite(PWLED4, 0);

	// EEreadMain();
}



int loopCount = 0;


void loop() {
// ---------- Main loop ------------
	fraiseService();	// listen to Fraise events
	analogService();	// analog management routine
	switchService();	// switch management routine

	if(delayFinished(mainDelay)) // when mainDelay triggers :
	{
		delayStart(mainDelay, 10000); 	// re-init mainDelay
		if(!switchSend()) analogSend();		// send analog channels that changed
	}
}

// Receiving

void fraiseReceiveChar() // receive text
{
	unsigned char c;
	
	c=fraiseGetChar();
	if(c=='L'){		//switch LED on/off 
		c=fraiseGetChar();
		digitalWrite(LED, c!='0');		
	}
	else
	if(c=='E') { 	// echo text (send it back to host)
		printf("C");
		c = fraiseGetLen(); 			// get length of current packet
		while(c--) printf("%c",fraiseGetChar());// send each received byte
		putchar('\n');				// end of line
	}
	else if(c=='W') { 	// WRITE: save eeprom
		if((fraiseGetChar() == 'R') && (fraiseGetChar() == 'I') && (fraiseGetChar() == 'T') && (fraiseGetChar() == 'E'))
			EEwriteMain();
	}
}

void fraiseReceive() // receive raw
{
	unsigned char c;
	unsigned int i;
	
	c=fraiseGetChar();
	
	switch(c) {
		case 10 : i = fraiseGetInt(); analogWrite(PWLED1, i); break;
		case 11 : i = fraiseGetInt(); analogWrite(PWLED2, i); break;
		case 12 : i = fraiseGetInt(); analogWrite(PWLED3, i); break;
		case 13 : i = fraiseGetInt(); analogWrite(PWLED4, i); break;
	}
}

void EEdeclareMain()
{
}
