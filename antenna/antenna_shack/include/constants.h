#ifndef constants_h
#define constants_h

#define Message_Initiate 'I'
#define Response_Position 'P'
#define Message_End 'E'

#define Run_ON LOW
#define Run_OFF HIGH

#define Slow_ON LOW
#define Slow_OFF HIGH

#define Direction_Increasing HIGH
#define Direction_Decreasing LOW

#define Speed_Stop 0
#define Speed_Half 1
#define Speed_Full 2

#define HalfSpeedPositionThreshold 50

#define MinPositionDegrees 90
#define MaxPositionDegrees 270


#define Mode_Run 0
#define Mode_Calibrate_Pos 1
#define Mode_Calibrate_Min 2
#define Mode_Calibrate_Max 3

#define Antenna_Stopped 0
#define Antenna_Travelling 1
#define Antenna_Damping 2

#endif