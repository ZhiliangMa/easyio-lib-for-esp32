#ifndef __EASYIO_MCPWM_CONFIG__
#define __EASYIO_MCPWM_CONFIG__

#include "driver/mcpwm.h"
#include "soc/mcpwm_periph.h"
#include "soc/rtc.h"

//MCPWM的信号可映射到任意GPIO输出管脚
//0-19, 21-23, 25-27, 32-39。GPIO 34-39 仅用作输入管脚
//乐鑫SDK是从14开始的
#define GPIO_MC0_PWM0A_OUT 15   //Set GPIO 15 as MC0 PWM0A
#define GPIO_MC0_PWM0B_OUT 16   //Set GPIO 16 as MC0 PWM0B
#define GPIO_MC0_PWM1A_OUT 17   //Set GPIO 17 as MC0 PWM1A
#define GPIO_MC0_PWM1B_OUT 18   //Set GPIO 18 as MC0 PWM1B
#define GPIO_MC0_PWM2A_OUT 19   //Set GPIO 19 as MC0 PWM2A
#define GPIO_MC0_PWM2B_OUT 21   //Set GPIO 21 as MC0 PWM2B

#define GPIO_MC0_SYNC0_IN   2   //Set GPIO 02 as MC0 SYNC0

#define GPIO_MC0_CAP0_IN   23   //Set GPIO 23 as  CAP0
#define GPIO_MC0_CAP1_IN   25   //Set GPIO 25 as  CAP1
#define GPIO_MC0_CAP2_IN   26   //Set GPIO 26 as  CAP2
#define CAP_SIG_NUM 3   //Three capture signals
#define CAP0_INT_EN BIT(27)  //Capture 0 interrupt bit
#define CAP1_INT_EN BIT(28)  //Capture 1 interrupt bit
#define CAP2_INT_EN BIT(29)  //Capture 2 interrupt bit

//You can get these value from the datasheet of servo you use, in general pulse width varies between 1000 to 2000 mocrosecond
#define SERVO_MIN_PULSEWIDTH 1000 //Minimum pulse width in microsecond
#define SERVO_MAX_PULSEWIDTH 2000 //Maximum pulse width in microsecond
#define SERVO_MAX_DEGREE 90 //Maximum angle in degree upto which servo can rotate





//DC直流有刷电机，包含两路同频PWM。如同时使用多路DC电机驱动，请开启同步功能，并将同步引脚接在同一信号上。（一定要接，不能空置，否则复位后会同步失败）
#define DC_MOTOR_0_2_MCPWM          MCPWM_UNIT_0 //默认DC直流有刷电机0~2，使用MCPWM0，如有其他功能占用冲突，请修改这里。
#define DC_MOTOR_3_5_MCPWM          MCPWM_UNIT_1 //默认DC直流有刷电机3~5，使用MCPWM1，如有其他功能占用冲突，请修改这里。

#define HALF_BRIDGE_0_2_MCPWM       MCPWM_UNIT_0 //默认half_bridge0~2，使用MCPWM0，如有其他功能占用冲突，请修改这里。
#define HALF_BRIDGE_3_5_MCPWM       MCPWM_UNIT_1 //默认half_bridge3~5，使用MCPWM1，如有其他功能占用冲突，请修改这里。

#define SERVO_0_5_MCPWM             MCPWM_UNIT_0 //默认舵机Servo0~5，使用MCPWM0，如有其他功能占用冲突，请修改这里。
#define SERVO_6_11_MCPWM            MCPWM_UNIT_1 //默认舵机Servo6~11，使用MCPWM1，如有其他功能占用冲突，请修改这里。

/**
 * @brief DC MOTOR默认引脚定义
 * 
 */
#define GPIO_MOTOR0_PWMA_OUT        25 //15   //Set GPIO 15 as MOTOR0 PWMA
#define GPIO_MOTOR0_PWMB_OUT        26 //16   //Set GPIO 16 as MOTOR0 PWMB
#define GPIO_MOTOR1_PWMA_OUT        17   //Set GPIO 17 as MOTOR1 PWMA
#define GPIO_MOTOR1_PWMB_OUT        18   //Set GPIO 18 as MOTOR1 PWMB
#define GPIO_MOTOR2_PWMA_OUT        19   //Set GPIO 19 as MOTOR2 PWMA
#define GPIO_MOTOR2_PWMB_OUT        21   //Set GPIO 21 as MOTOR2 PWMB

#define GPIO_MOTOR3_PWMA_OUT        7    //Set GPIO 7 as MOTOR3 PWMA
#define GPIO_MOTOR3_PWMB_OUT        8    //Set GPIO 8 as MOTOR3 PWMB
#define GPIO_MOTOR4_PWMA_OUT        9    //Set GPIO 9 as MOTOR4 PWMA
#define GPIO_MOTOR4_PWMB_OUT        10   //Set GPIO 10 as MOTOR4 PWMB
#define GPIO_MOTOR5_PWMA_OUT        11   //Set GPIO 11 as MOTOR5 PWMA
#define GPIO_MOTOR5_PWMB_OUT        12   //Set GPIO 12 as MOTOR5 PWMB

/**
 * @brief HALF BRIDGE默认引脚定义
 * 
 */
#define GPIO_HALF_BRIDGE0_PWMA_OUT      25 //15   //Set GPIO 15 as HALF_BRIDGE0 PWMA
#define GPIO_HALF_BRIDGE0_PWMB_OUT      26 //16   //Set GPIO 16 as HALF_BRIDGE0 PWMB
#define GPIO_HALF_BRIDGE1_PWMA_OUT      17   //Set GPIO 17 as HALF_BRIDGE1 PWMA
#define GPIO_HALF_BRIDGE1_PWMB_OUT      18   //Set GPIO 18 as HALF_BRIDGE1 PWMB
#define GPIO_HALF_BRIDGE2_PWMA_OUT      19   //Set GPIO 19 as HALF_BRIDGE2 PWMA
#define GPIO_HALF_BRIDGE2_PWMB_OUT      21   //Set GPIO 21 as HALF_BRIDGE2 PWMB

#define GPIO_HALF_BRIDGE3_PWMA_OUT      7    //Set GPIO 7 as HALF_BRIDGE3 PWMA
#define GPIO_HALF_BRIDGE3_PWMB_OUT      8    //Set GPIO 8 as HALF_BRIDGE3 PWMB
#define GPIO_HALF_BRIDGE4_PWMA_OUT      9    //Set GPIO 9 as HALF_BRIDGE4 PWMA
#define GPIO_HALF_BRIDGE4_PWMB_OUT      10   //Set GPIO 10 as HALF_BRIDGE4 PWMB
#define GPIO_HALF_BRIDGE5_PWMA_OUT      11   //Set GPIO 11 as HALF_BRIDGE5 PWMA
#define GPIO_HALF_BRIDGE5_PWMB_OUT      12   //Set GPIO 12 as HALF_BRIDGE5 PWMB


/**
 * @brief SERVO默认引脚定义
 * 
 */
#define GPIO_SERVO0_OUT      25 //15   //Set GPIO 15 as SERVO0 PWMA
#define GPIO_SERVO1_OUT      26 //16   //Set GPIO 16 as SERVO1 PWMB
#define GPIO_SERVO2_OUT      17   //Set GPIO 17 as SERVO2 PWMA
#define GPIO_SERVO3_OUT      18   //Set GPIO 18 as SERVO3 PWMB
#define GPIO_SERVO4_OUT      19   //Set GPIO 19 as SERVO4 PWMA
#define GPIO_SERVO5_OUT      21   //Set GPIO 21 as SERVO5 PWMB

#define GPIO_SERVO6_OUT      7    //Set GPIO 7 as SERVO6 PWMA
#define GPIO_SERVO7_OUT      8    //Set GPIO 8 as SERVO7 PWMB
#define GPIO_SERVO8_OUT      9    //Set GPIO 9 as SERVO8 PWMA
#define GPIO_SERVO9_OUT      10   //Set GPIO 10 as SERVO9 PWMB
#define GPIO_SERVO10_OUT     11   //Set GPIO 11 as SERVO10 PWMA
#define GPIO_SERVO11_OUT     12   //Set GPIO 12 as SERVO11 PWMB


/**
 * @brief 输入捕获默认引脚定义
 * 
 */
#define GPIO_CAPTURE0_IN   27   //Set GPIO 27 as  CAP0
#define GPIO_CAPTURE1_IN   25   //Set GPIO 25 as  CAP1
#define GPIO_CAPTURE2_IN   26   //Set GPIO 26 as  CAP2
#define GPIO_CAPTURE3_IN   32   //Set GPIO 32 as  CAP3
#define GPIO_CAPTURE4_IN   34   //Set GPIO 34 as  CAP4
#define GPIO_CAPTURE5_IN   35   //Set GPIO 35 as  CAP5

#endif
