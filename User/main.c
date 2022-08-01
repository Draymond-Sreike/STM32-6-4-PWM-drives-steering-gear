#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "OLED.h"
#include "PWM.h"
#include "Key.h"
#include "SteeringEngine.h"

int main()
{
	uint8_t keyNum;
	float angle = 0.0;
	OLED_Init();
	key_Init();
	steeringEngineInit();
	OLED_ShowString(1, 1, "Welcome to use:");
	OLED_ShowString(2, 1, "Steering Engine");
	OLED_ShowString(3, 1, "Angle:");
	setSteeringEngineAngle(angle);
	OLED_ShowNum(3, 7, angle, 3);
	
	while(1)
	{
//		keyNum = key_GetKeyNum();
//		if(1 == keyNum)
//		{
//			angle += 30;
//			if(angle > 180)
//				angle = 0;
//			setSteeringEngineAngle(angle);
//		}
//		else if(2 == keyNum)
//		{
//			angle -= 30;
//			if(angle < 0)
//				angle = 180;
//			setSteeringEngineAngle(angle);
//		}
//		OLED_ShowNum(3, 7, angle, 3);
	}
}
