#ifndef constants_h
#define constants_h

#define Message_Initiate 'I'
#define Command_ReadPosition 'P'
#define Response_Position 'P'
#define Message_End 'E'

#define Speed_Zero 0
#define Speed_Half 1
#define Speed_Full 2



#define TX HIGH
#define RX LOW

#define Relay_ON LOW
#define Relay_OFF HIGH

#define Direction_Increasing true
#define Direction_Decreasing false

#define HalfSpeedPositionThreshold 5

#define MinPositionDegrees 90
#define MaxPositionDegrees 270

#endif