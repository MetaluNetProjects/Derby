/*********************************************************************
 *               analog example for 8X2A
 *	Analog capture on connectors K2, K3, K4 and K5. 
 *********************************************************************/

#define BOARD 8X2A

#include <fruit.h>
#include <analog.h>
#include <softpwm.h>

t_delay mainDelay;
unsigned char pulseCount1, pulseCount2, pulseCount3, pulseCount4, pulseCount5, pulseCount6;

void setup(void) {	
//----------- Setup ----------------
	fruitInit();
			
	//pinModeDigitalOut(LED); 	// set the LED pin mode to digital out
	//digitalClear(LED);		// clear the LED
	delayStart(mainDelay, 5000); 	// init the mainDelay to 5 ms

//----------- Analog setup ----------------
	analogInit();		// init analog module
//	analogSelect(0,K2);	// assign connector K1 to analog channel 0

//----------- softpwm setup ----------------
	softpwmInit();		// init softpwm module
	
	pinModeDigitalOut(MCEN); 	// enable C motor driver
	digitalSet(MCEN);
	pinModeDigitalOut(MDEN); 	// enable D motor driver
	digitalSet(MDEN);

	pinModeDigitalOut(RING1);
	digitalClear(RING1);
	pinModeDigitalOut(RING2);
	digitalClear(RING2);
	
	pinModeDigitalIn(PULSE1);
	pinModeDigitalIn(PULSE2);
	pinModeDigitalIn(PULSE3);
	pinModeDigitalIn(PULSE4);
	pinModeDigitalIn(PULSE5);
	pinModeDigitalIn(PULSE6);
}

// ---------- pulse counts process ---------

#define INCPULSE(num) do {\
	ptmp = digitalRead(PULSE##num); if(p##num != ptmp) {\
		p##num = ptmp; if(ptmp == 1) pulseCount##num ++;}\
} while(0)

void pulseCountsProcess()
{
	static unsigned char ptmp, p1,p2,p3,p4,p5,p6;

	INCPULSE(1);
	INCPULSE(2);
	INCPULSE(3);
	INCPULSE(4);
	INCPULSE(5);
	INCPULSE(6);
}

void pulseCountsSend()
{
	static unsigned int sum, oldSum;
	static unsigned char buf[10] = {'B', '200'};
	
	sum = pulseCount1 + pulseCount2 + pulseCount3 + pulseCount4 + pulseCount5;
	if(sum != oldSum) {
		oldSum = sum;
		buf[2] = pulseCount1;
		buf[3] = pulseCount2;
		buf[4] = pulseCount3;
		buf[5] = pulseCount4;
		buf[6] = pulseCount5;
		buf[7] = pulseCount6;
	}
}

// ---------- Interrupts ------------
void highInterrupts()
{
	softpwmHighInterrupt();
}
void lowInterrupts()
{
	softpwmLowInterrupt();
}

// ---------- Main loop ------------
void loop() {
	fraiseService();	// listen to Fraise events
	analogService();	// analog management routine
	pulseCountsProcess();
	
	if(delayFinished(mainDelay)) // when mainDelay triggers :
	{
		delayStart(mainDelay, 5000); 	// re-init mainDelay
		analogSend();		// send analog channels that changed
		pulseCountsSend();
	}
}

// Receiving

void fraiseReceiveChar() // receive text
{
	unsigned char c;
	
	c=fraiseGetChar();
	if(c=='L'){		//switch LED on/off 
		c=fraiseGetChar();
		//digitalWrite(LED, c!='0');		
	}
	else if(c=='E') { 	// echo text (send it back to host)
		printf("C");
		c = fraiseGetLen(); 			// get length of current packet
		while(c--) printf("%c",fraiseGetChar());// send each received byte
		putchar('\n');				// end of line
	}	
}

void fraiseReceive() // receive raw bytes
{
	unsigned char c=fraiseGetChar();
	if(c==50) softpwmReceive(); // if first byte is 50, then call softpwm receive function.
}

