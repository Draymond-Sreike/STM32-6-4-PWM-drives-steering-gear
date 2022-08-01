#include "stm32f10x.h"                  // Device header

void PWM_Init()
{
	//该部分定时器我们选择定时器2，其是挂载在APB1总线上的，故这里选择的是APB1来开启RCC内部时钟
	//开启与定时器2相关的RCC_APB1外设时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
	
	//选择定时器的触发为内部的RCC时钟触发，并将该触发供到TIM2，即定时器2上
	//此配置将打通RCC与定时器2时基单元的连接
	TIM_InternalClockConfig(TIM2);
	
	/************配置定时器时基单元***************/
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
	//选择滤波的取样点相关参数进行配置，该项参数配置对该工程影响不打，随机取TIM_CKD_DIV1
	TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
	//选择计数器的计数模式为向上计数，即从0开始递增计数
	TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
	//配置自动重装载寄存器的参数，该参数配为：当计数器计数100次时进行重装，此值为PWM的”空“值，即占空比的分母
	//该操作使得占空比计算的分母为100，分辨率为1%
	TIM_TimeBaseInitStruct.TIM_Period = (200 - 1);//ARR自动重装寄存器
	//配置预分频器的参数，该参数配为分频7200倍，此时接入计数器的频率变为72MHz/(72000*0.2)=5000Hz
	//搭配ARR自动重装寄存器的配置，可使得PWM的频率为50Hz，以此来满足舵机对PWM频率的要求
	TIM_TimeBaseInitStruct.TIM_Prescaler = (7200 - 1);//PSC分频器
	//通用寄存器中没有重复次数计数器，该参数配0即可
	TIM_TimeBaseInitStruct.TIM_RepetitionCounter = 0;
	//TIM_TimeBaseStructInit(&TIM_TimeBaseInitStruct); //此处犯错，不是使用这个函数！！！
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseInitStruct); //应该用该函数！！！同时该函数执行完成后会有一个副作用：中断标志位会被置SET
	
	
	/************配置定时器输出比较单元***************/
	TIM_OCInitTypeDef TIM_OCInitStruct;
	//因为该结构体内的成员数据有些是高级定时器才有，使用通用定时器不需配置
	//但为了保证结构体成员数据的完整性，也为了保证后续执行程序时不因这些数据没有配置而出错
	//我们可以通过下面这个函数快速配置完这个结构体的所有成员数据
	//后续我们再根据通用定时器的配置需要，选择其中的部分成员数据进行配置即可
	TIM_OCStructInit(&TIM_OCInitStruct);
	//选择其中的部分成员数据进行配置
	//选择输出比较模式为PWM模式1，此时若定时器为向上计数模式
		//则当计数器（CNT） < 捕获/比较寄存器（CCR）时，REF置有效电平（高电平）
		//当计数器（CNT）>= 捕获/比较寄存器（CCR）时，REF置无效电平（低电平）
	TIM_OCInitStruct.TIM_OCMode = TIM_OCMode_PWM1;
	//配置极性选择，是REF对外输出的倒数第二关
	//若选择高电平输出有效电平，即TIM_OCPolarity_High
		//即REF传过来高电平，则输出使能电路对外输出有效电平（高电平）相当于此模式下REF是啥就对外输出啥
	//若选择低电平输出有效电平，即TIM_OCPolarity_Low
		//即REF传过来低电平，则输出使能电路对外输出有效电平（高电平）相当于此模式下将REF输出取反
	TIM_OCInitStruct.TIM_OCPolarity = TIM_OCPolarity_High;
	//输出电路使能,REF对外输出的最后一关
	TIM_OCInitStruct.TIM_OutputState = TIM_OutputState_Enable;
	//配置比较寄存器（CCR）的比较值，这个值将决定占空比，初始化之后我们先暂时让占空比为0，即恒为低电平
	//该项输入要求本来是从0x0000到0xffff但由于我们设置的定时器重装值为0-99，所以我们只填0-100到这个数里面
		//至于为什么重装值是0-99，但这里却可以填0-100，是因为，计数器（CNT）>= 捕获/比较寄存器（CCR）在”=“成立的瞬间才翻转电平输出
		//比如这个比较值我们填99，这时计数器从0-98（99个数）的过程中都是原电平状态，当等于99时翻转电平状态，计数器总共100个数（0-99）
		//可以得出我们填99，占空比就是99%，其他数字也一样
	//填入的这个比较值可以直接是10进制的0-100，不需要按照该数据定义的用十六进制
	TIM_OCInitStruct.TIM_Pulse = 0;//0x0032
	//选择通用定时中的OC2单元,因为我们此时用的是A1口，其连接CH2，而CH2连接到了OC2
	TIM_OC2Init(TIM2, &TIM_OCInitStruct);
	
	/************配置GPIO***************/
	//使特定的IO口连接定时器的OC（输出）单元
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	GPIO_InitTypeDef GPIO_InitStructure;
	//IO口设置为复用推挽输出模式
	//因为普通推挽输出模式下引脚的控制权是来自于输出数据寄存器的
	//要使得定时器对引脚有控制权，我们就需要”复用“
	//此模式下，输出数据寄存器将被断开，控制权完全交由其他片上外设（这里就是我们的定时器在起作用）
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	//参考引脚定义图，OC2连接的CH2通道，而CH2通道连接的是A1口，故配置A1口进行输出
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1; 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	//所有参数配置完成，开启定时器进行计数
	TIM_Cmd(TIM2, ENABLE);
	//现在PWM波形信号就会输出到A1了
	
}

//封装修改占空比（实际是修改CCR比较寄存器的值）的函数供外部使用
//compare可传递十进制值
//这里由于我们配置的定时器的自动重装值是99，我们输入的范围是0-100
//至于为什么是0-100见配置输出比较单元的解释
void PWM_SetCompare(uint16_t compare)
{
	//这个函数是在程序运行过程中更改（OC2所连接的）CCR比较寄存器的值，在这里体现为更改占空比
	//实际上占空比是要用CCR的值于ACC的值相除得到的，即CCR/ACC
	//这里是因为ACC的值为100，所以CCR配出来的值直接等于占空比 （实际上是99，我们在不至于混淆的情况下说是100）
	TIM_SetCompare2(TIM2, compare);
}