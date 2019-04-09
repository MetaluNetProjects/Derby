#ifndef _CONFIG_H_
#define _CONFIG_H_

#define ANALOG_FILTER 3 
#define RAMP_UINCPOW 10 // 1 increment = 1024 milli-increments (mincs) = 1024x1024 micro-increments (uincs)
#define RAMP_VPOW 10
//#define RAMP_TO_POS_POW RAMP_UINCPOW-4
#define RAMP_MAXERROR 1 //  

// rotation motor:
#define MOTA_END K2
#define MOTA_ENDLEVEL 1
#define MOTA_A K1
#define MOTA_B KZ1

#define TRANS_LOSW K2
#define TRANS_HISW K3
#define TRANS_SWLEVEL 1

#define SERVO1 K4

#define PWLED1 MB1
#define PWLED2 MB2

#endif // _CONFIG_H_

