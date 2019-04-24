/*********************************************************************
 *               analog example for 8X2A
 *	Analog capture on connectors K2, K3, K4 and K5. 
 *********************************************************************/

#define BOARD 8X2A

#include <fruit.h>
#include <analog.h>
#include <softpwm.h>

t_delay mainDelay;
unsigned char pulseCount[6];
unsigned char pinState[6];
unsigned char pinDebounceCount[6];

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
	pinModeDigitalOut(MBEN); 	// enable B motor driver
	digitalSet(MBEN);
	pinModeDigitalOut(MAEN); 	// enable A motor driver
	digitalSet(MAEN);

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
		p##num = ptmp; if(ptmp == 1) pulseCount##num ++; } \
} while(0)
unsigned char readPulsePin(unsigned char num)
{
	switch(num) {
		case 0: return digitalRead(PULSE1); break; 
		case 1: return digitalRead(PULSE2); break; 
		case 2: return digitalRead(PULSE3); break; 
		case 3: return digitalRead(PULSE4); break; 
		case 4: return digitalRead(PULSE5); break; 
		case 5: return digitalRead(PULSE6); break;
		default: return 0;
	}
}
 
void pulseCountsProcess()
{
/*	static unsigned char ptmp, p1,p2,p3,p4,p5,p6;
	static unsigned char ptmp, p1,p2,p3,p4,p5,p6;

	INCPULSE(1);
	INCPULSE(2);
	INCPULSE(3);
	INCPULSE(4);
	INCPULSE(5);
	INCPULSE(6);
*/
	unsigned char i, ptmp;
#define DEBOUNCE_MAX 100
	for(i = 0 ; i < 6 ; i++) {
		if(readPulsePin(i)) {
			if(pinDebounceCount[i] < DEBOUNCE_MAX) pinDebounceCount[i]++;
		} else {
			if(pinDebounceCount[i] > 0) pinDebounceCount[i]--;
		}
		if(pinDebounceCount[i] > DEBOUNCE_MAX/2) {
			if(pinState[i] == 0) pulseCount[i]++;
			pinState[i] = 1;
		} else pinState[i] = 0;
	}	
}

void pulseCountsSend()
{
	static unsigned int sum, oldSum;
	static unsigned char len, buf[10] = {'B', 10};
	
	sum = pulseCount[0] + pulseCount[1] + pulseCount[2] + 
		pulseCount[3] + pulseCount[4] + pulseCount[5];
	if(0 || (sum != oldSum)) {
		oldSum = sum;
		len = 2;
		buf[len++] = pulseCount[0];
		buf[len++] = pulseCount[1];
		buf[len++] = pulseCount[2];
		buf[len++] = pulseCount[3];
		buf[len++] = pulseCount[4];
		buf[len++] = pulseCount[5];
		buf[len++] = '\n';
		fraiseSend(buf,len);
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
	if(c == 50) softpwmReceive(); // if first byte is 50, then call softpwm receive function.
	else if (c == 51) digitalWrite(RING1, fraiseGetChar() != 0);
	else if(c == 52) digitalWrite(RING2, fraiseGetChar() != 0);
}

