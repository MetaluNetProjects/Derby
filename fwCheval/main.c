/*********************************************************************
 *               Ground Noise Motor 
 * .board: StepGN (FraiseStep 1.2 + 16MHz quartz)
 *  Antoine Rousseau @ metalu.net - feb.2019
 *********************************************************************/

#define BOARD Step1.2

#include <fruit.h>
#include <analog.h>
#include <dcmotor.h>
#include <ramp.h>
#include <eeparams.h>
#include <servo.h>


//t_ramp posRamp;
t_delay mainDelay;

#ifdef _DCMOTOR_H_
#define USE_MOTORS
#endif

#ifdef USE_MOTORS
DCMOTOR_DECLARE(A);
DCMOTOR_DECLARE(B);
#endif


//-------------  Timer1 macros :  ---------------------------------------- 
//prescaler=PS fTMR1=FOSC/(4*PS) nbCycles=0xffff-TMR1init T=nbCycles/fTMR1=(0xffff-TMR1init)*4PS/FOSC
//TMR1init=0xffff-(T*FOSC/4PS) ; max=65536*4PS/FOSC : 
//ex: PS=8 : T=0.01s : TMR1init=0xffff-15000
//Maximum 1s !!
#define	TMR1init(T) (0xffff-((T*FOSC)/32000)) //ms ; maximum: 8MHz:262ms 48MHz:43ms 64MHz:32ms
#define	TMR1initUS(T) (0xffff-((T*FOSC)/32000000)) //us ; 
#define InitTimer(T) do{ TMR1H=TMR1init(T)/256 ; TMR1L=TMR1init(T)%256; PIR1bits.TMR1IF=0; }while(0)
#define InitTimerUS(T) do{ TMR1H=TMR1initUS(T)/256 ; TMR1L=TMR1initUS(T)%256; PIR1bits.TMR1IF=0; }while(0)
#define TimerOut() (PIR1bits.TMR1IF)

void highInterrupts()
{
	if(PIR1bits.TMR1IF) {
		DCMOTOR_CAPTURE_SERVICE_SINGLE(A);
		InitTimerUS(50UL);
	}
	servoHighInterrupt();
}

void setup(void) {	
//----------- Setup ----------------
	fruitInit();
			
	pinModeDigitalOut(LED); 	// set the LED pin mode to digital out
	digitalClear(LED);		// clear the LED
	delayStart(mainDelay, 5000); 	// init the mainDelay to 5 ms

//----------- Analog setup ----------------
	analogInit();		// init analog module
//	analogSelect(0, MAC);	// assign MotorA current sense to analog channel 0

//----------- Servo setup ----------------
	servoInit();        // init servo module
	servoSelect(0,SERVO1); // 

//----------- dcmotor setup ----------------

#ifdef USE_MOTORS
	dcmotorInit(A);
//	dcmotorInit(B);
	pinModeDigitalIn(TRANS_LOSW);
	pinModeDigitalIn(TRANS_HISW);
    
// power leds
	digitalSet(MBEN);
	pinModeDigitalOut(MBEN);
	
	pinModeAnalogOut(PWLED1);
	pinModeAnalogOut(PWLED2);
	analogWrite(PWLED1, 0);
	analogWrite(PWLED2, 0);
#endif
	
	
//#define HW_PARAMS	
	DCMOTOR(A).Setting.PosWindow = 1;
	DCMOTOR(A).Setting.PwmMin = 150;
#ifdef HW_PARAMS
	DCMOTOR(A).Setting.PosWindow = 6;
	DCMOTOR(A).Setting.PwmMin = 50;
	//DCMOTOR(D).Setting.PosErrorGain = 6;
	//DCMOTOR(D).Setting.onlyPositive = 0;
	
	DCMOTOR(A).PosRamp.maxSpeed = 1800;
	DCMOTOR(A).PosRamp.maxAccel = 2400;
	DCMOTOR(A).PosRamp.maxDecel = 2400;
	//rampSetPos(&DCMOTOR(C).PosRamp, 0);

	DCMOTOR(A).PosPID.GainP = 40;
	DCMOTOR(A).PosPID.GainI = 1;
	DCMOTOR(A).PosPID.GainD = 0;
	DCMOTOR(A).PosPID.MaxOut = 1023;

	//DCMOTOR(C).VolVars.homed = 0;
#else
	EEreadMain();
#endif
	
//	DCMOTOR(B).Setting.reversed = 1;

	T1CON=0b00110011;//src=fosc/4,ps=8,16bit r/w,on.
	PIE1bits.TMR1IE=1;  //1;
	IPR1bits.TMR1IP=1;
}


void testTransEnds()
{
	if((digitalRead(TRANS_HISW) == TRANS_SWLEVEL) && (DCMOTOR(A).Vars.PWMConsign > 0)) {
		DCMOTOR(A).Vars.PWMConsign = 0;
		DCMOTOR(A).Setting.Mode = 0;
	}
		
	if((digitalRead(TRANS_LOSW) == TRANS_SWLEVEL) && (DCMOTOR(A).Vars.PWMConsign < 0)) {
		DCMOTOR(A).Vars.PWMConsign = 0;
		DCMOTOR(A).Setting.Mode = 0;
	}

#if 0
	if((state == STATE_RUNNING) 
	&& (digitalRead(TRANS_HISW) == TRANS_SWLEVEL)
	&& (mode == MODE_AUTO)) {
		state = STATE_HOMING;
		DCMOTOR(A).Vars.PWMConsign = 0; // stop rotation
		DCMOTOR(A).Setting.Mode = 0;
		DCMOTOR(B).Vars.PWMConsign = transHomePWM;
		DCMOTOR(B).Setting.Mode = 0;
	}
		
	if((state == STATE_HOMING) 
	/*&& (DCMOTOR(A).VolVars.homed == 1) */
	&& (digitalRead(TRANS_LOSW) == TRANS_SWLEVEL)) {
		state = STATE_RUNNING;
		DCMOTOR(A).VolVars.Position = 0;
		DCMOTOR(B).VolVars.Position = 0;
		//rampSetPos(&(DCMOTOR(B).PosRamp), 0);
		DCMOTOR(A).VolVars.homed = 1;
		DCMOTOR(B).VolVars.homed = 1;
		/*DCMOTOR(A).Vars.SpeedConsign = speed>>SPEED_FILTER; 
		DCMOTOR(A).Setting.Mode = 1;*/
	}
#endif
}
	

void sendMotorState()
{
	static unsigned char buf[10] = { 'B', 10};
	static int ramppos;
	
	buf[2] = (unsigned char)DCMOTOR_GETPOS(A);
	buf[3] = digitalRead(TRANS_LOSW) == 0;
	buf[4] = digitalRead(TRANS_HISW) == 0;
	buf[5] = DCMOTOR(A).Vars.PWMConsign >> 8;
	buf[6] = DCMOTOR(A).Vars.PWMConsign & 255;
	ramppos = (int)rampGetPos(&(DCMOTOR(A).PosRamp));
	buf[7] = ramppos >> 8;
	buf[8] = ramppos & 255;
	buf[9] = '\n';
	fraiseSend(buf,10);
}

void loop() {
// ---------- Main loop ------------
	fraiseService();	// listen to Fraise events
	analogService();	// analog management routine
	servoService();	// servo management routine

	if(delayFinished(mainDelay)) // when mainDelay triggers :
	{
		delayStart(mainDelay, 10000); 	// re-init mainDelay
		analogSend();		// send analog channels that changed
		fraiseService();
#ifdef USE_MOTORS
		testTransEnds();
		DCMOTOR_COMPUTE_SINGLE(A,ASYM);
		sendMotorState();
#endif
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
		case 20 :  servoReceive(); break;
#ifdef USE_MOTORS
	    case 120 : DCMOTOR_INPUT(A) ; break;
#endif
	}
}

void EEdeclareMain()
{
	DCMOTOR_DECLARE_EE(A);
}
