#include "stm32f10x.h"                  // Device header
#include "PWM.h"

void steeringEngineInit()
{
	PWM_Init();
}

void setSteeringEngineAngle(float angle)	
{
	PWM_SetCompare((angle / 180) * 20 + 5);
}

//void setSteeringEngineAngle(float angle)
//{
//	PWM_SetCompare(angle / 180 * 2000 + 500);
//}
