#ifndef __I2C_PCF8563_H__
#define __I2C_PCF8563_H__

#include "i2c_config.h"
#include "esp_log.h"

// 驱动代码移植参照，特此感谢：https://blog.csdn.net/weixin_41756255/article/details/84996464

#define PCF8563_TICKS_TO_WAIT           (100 / portTICK_RATE_MS)    // I2C读写的超时等待时间

#define PCF8563_I2C_BUS                 I2C_NUM_0 // PCF8563所在的I2C总线

#define PCF8563_SLAVE_ADDRESS           0x51    // PCF8563在I2C总线上的从机器件地址


#define PCF8563_Check_Data                       (uint8_t)0x55  //检测器件是否存在用，可用其他数值
//
//电源复位功能
//
#define PCF8563_PowerResetEnable                 (uint8_t)0x08
#define PCF8563_PowerResetDisable                (uint8_t)0x09
 
//
//世纪位操作定义
//
#define PCF_Century_SetBitC                      (uint8_t)0x80
 
#define PCF_Century_19xx                         (uint8_t)0x03
#define PCF_Century_20xx                         (uint8_t)0x04
 
//
//数据格式
//
#define PCF_Format_BIN                           (uint8_t)0x01
#define PCF_Format_BCD                           (uint8_t)0x02
 
//
//设置PCF8563模式用
//
#define PCF_Mode_Normal                          (uint8_t)0x05
#define PCF_Mode_EXT_CLK                         (uint8_t)0x06
 
#define PCF_Mode_INT_Alarm                       (uint8_t)0x07
#define PCF_Mode_INT_Timer                       (uint8_t)0x08
 
/******************************************************************************
                             参数寄存器地址宏定义
******************************************************************************/
 
#define PCF8563_Address_Control_Status_1         (uint8_t)0x00  //控制/状态寄存器1
#define PCF8563_Address_Control_Status_2         (uint8_t)0x01  //控制/状态寄存器2
 
#define PCF8563_Address_CLKOUT                   (uint8_t)0x0d  //CLKOUT频率寄存器
#define PCF8563_Address_Timer                    (uint8_t)0x0e  //定时器控制寄存器
#define PCF8563_Address_Timer_VAL                (uint8_t)0x0f  //定时器倒计数寄存器
 
#define PCF8563_Address_Years                    (uint8_t)0x08  //年
#define PCF8563_Address_Months                   (uint8_t)0x07  //月
#define PCF8563_Address_Days                     (uint8_t)0x05  //日
#define PCF8563_Address_WeekDays                 (uint8_t)0x06  //星期
#define PCF8563_Address_Hours                    (uint8_t)0x04  //小时
#define PCF8563_Address_Minutes                  (uint8_t)0x03  //分钟
#define PCF8563_Address_Seconds                  (uint8_t)0x02  //秒
 
#define PCF8563_Alarm_Minutes                    (uint8_t)0x09  //分钟报警
#define PCF8563_Alarm_Hours                      (uint8_t)0x0a  //小时报警
#define PCF8563_Alarm_Days                       (uint8_t)0x0b  //日报警
#define PCF8563_Alarm_WeekDays                   (uint8_t)0x0c  //星期报警
 
/******************************************************************************
                               参数屏蔽宏定义
******************************************************************************/
 
#define PCF8563_Shield_Control_Status_1          (uint8_t)0xa8
#define PCF8563_Shield_Control_Status_2          (uint8_t)0x1f
 
#define PCF8563_Shield_Seconds                   (uint8_t)0x7f
#define PCF8563_Shield_Minutes                   (uint8_t)0x7f
#define PCF8563_Shield_Hours                     (uint8_t)0x3f
 
#define PCF8563_Shield_Days                      (uint8_t)0x3f
#define PCF8563_Shield_WeekDays                  (uint8_t)0x07
#define PCF8563_Shield_Months_Century            (uint8_t)0x1f
#define PCF8563_Shield_Years                     (uint8_t)0xff
 
#define PCF8563_Shield_Minute_Alarm              (uint8_t)0x7f
#define PCF8563_Shield_Hour_Alarm                (uint8_t)0x3f
#define PCF8563_Shield_Day_Alarm                 (uint8_t)0x3f
#define PCF8563_Shield_WeekDays_Alarm            (uint8_t)0x07
 
#define PCF8563_Shield_CLKOUT_Frequency          (uint8_t)0x03
#define PCF8563_Shield_Timer_Control             (uint8_t)0x03
#define PCF8563_Shield_Timer_Countdown_Value     (uint8_t)0xff
 
/******************************************************************************
                              PCF8563寄存器结构定义                          
******************************************************************************/
 
/**
   ==================================================================
                               全部寄存器结构
   ==================================================================
**/
typedef struct
{
	uint8_t Control_Status_1;      //控制寄存器1
	uint8_t Control_Status_2;      //控制寄存器2
	uint8_t Seconds;               //秒寄存器
	uint8_t Minutes;               //分钟寄存器
	uint8_t Hours;                 //小时寄存器
	uint8_t Days;                  //日期寄存器
	uint8_t WeekDays;              //星期寄存器，数值范围：0 ~ 6
	uint8_t Months_Century;        //月份寄存器，bit7为世纪位，0：指定世纪数为20xx；1：指定世纪数为19xx
	uint8_t Years;                 //年寄存器
	uint8_t Minute_Alarm;          //分钟报警寄存器
	uint8_t Hour_Alarm;            //小时报警寄存器
	uint8_t Day_Alarm;             //日期报警寄存器
	uint8_t WeekDays_Alarm;        //星期报警寄存器，数值范围：0 ~ 6
	uint8_t CLKOUT_Frequency;      //频率管脚输出控制寄存器
	uint8_t Timer_Control;         //定时器控制寄存器
	uint8_t Timer_Countdown_Value; //定时器计数寄存器
}_PCF8563_Register_Typedef;
 
/**
   ==================================================================
                               时间信息结构
   ==================================================================
**/
typedef struct
{
	uint8_t RTC_Hours;    //小时
	uint8_t RTC_Minutes;  //分钟
	uint8_t RTC_Seconds;  //秒钟
	uint8_t Reseved;      //保留
}_PCF8563_Time_Typedef;
 
 
/**
   ==================================================================
                               日期信息结构
   ==================================================================
**/
typedef struct
{
	uint8_t RTC_Years;    //年份
	uint8_t RTC_Months;   //月份
	uint8_t RTC_Days;     //日期
	uint8_t RTC_WeekDays; //星期，数值范围：0 ~ 6
}_PCF8563_Date_Typedef;
 
 
 
/**
   ==================================================================
                               闹铃信息结构
   ==================================================================
**/
typedef struct
{
	unsigned short Reseved;             //保留
	uint8_t RTC_AlarmDays;        //日期闹铃
	uint8_t RTC_AlarmWeekDays;    //星期闹铃，数值范围：0 ~ 6
	uint8_t RTC_AlarmHours;       //小时闹铃
	uint8_t RTC_AlarmMinutes;     //分钟闹铃
	uint8_t RTC_AlarmNewState;    //闹铃开关
                                        //其值有RTC_AlarmNewState_Open、RTC_AlarmNewState_Close、RTC_AlarmNewState_Open_INT_Enable
                                        //只使用其中一个即可
	uint8_t RTC_AlarmType;        //报警类型，
                                        //其值有RTC_AlarmType_Minutes、RTC_AlarmType_Hours、RTC_AlarmType_Days、RTC_AlarmType_WeekDays
                                        //多个报警类型打开，请用或关系合并
}_PCF8563_Alarm_Typedef;
 
//
//闹铃开关
//
#define RTC_AlarmNewState_Open                   (u8)0x01
#define RTC_AlarmNewState_Close                  (u8)0x02  //闹铃关闭，并且关闭中断输出
#define RTC_AlarmNewState_Open_INT_Enable        (u8)0x04  //闹铃开启并开启中断输出
//
//定义闹铃类型
//
#define RTC_AlarmType_Minutes                    (uint8_t)0x01
#define RTC_AlarmType_Hours                      (uint8_t)0x02
#define RTC_AlarmType_Days                       (uint8_t)0x04
#define RTC_AlarmType_WeekDays                   (uint8_t)0x08
 
/**
   ==================================================================
                               频率输出信息结构
   ==================================================================
**/
typedef struct
{
	uint8_t CLKOUT_Frequency;  //输出频率选择
	                                 //PCF_CLKOUT_F32768 --> 输出32.768KHz
                                     //PCF_CLKOUT_F1024  --> 输出1024Hz
                                     //PCF_CLKOUT_F32    --> 输出32Hz
                                     //PCF_CLKOUT_F1     --> 输出1Hz
	uint8_t CLKOUT_NewState;   //输出状态
	                                 //PCF_CLKOUT_Close  --> 输出被禁止并设成高阻抗
	                                 //PCF_CLKOUT_Open   --> 输出有效
}_PCF8563_CLKOUT_Typedef;
 
//
//频率输出寄存器 --> 0x0d
//
#define PCF_CLKOUT_Open                          (uint8_t)(1<<7)     //CLKOUT输出有效
#define PCF_CLKOUT_Close                         (uint8_t)(~(1<<7))  //CLKOUT输出被禁止并设成高阻抗
 
#define PCF_CLKOUT_F32768                        (uint8_t)0x00  //输出32.768KHz
#define PCF_CLKOUT_F1024                         (uint8_t)0x01  //输出1024Hz
#define PCF_CLKOUT_F32                           (uint8_t)0x02  //输出32Hz
#define PCF_CLKOUT_F1                            (uint8_t)0x03  //输出1Hz
 
/**
   ==================================================================
                               定时器信息结构
   ==================================================================
**/
typedef struct
{
	uint8_t RTC_Timer_Value;      //定时器计数器数值，设置定时时长，不需要的直接填0即可
	uint8_t RTC_Timer_Frequency;  //定时器工作频率
	                                    //PCF_Timer_F4096 --> 定时器时钟频率为4096Hz
	                                    //PCF_Timer_F64   --> 定时器时钟频率为64Hz
	                                    //PCF_Timer_F1    --> 定时器时钟频率为1Hz
	                                    //PCF_Timer_F160  --> 定时器时钟频率为1/60Hz
	uint8_t RTC_Timer_NewState;   //开启状态
	                                    //PCF_Timer_Close  --> 关闭
	                                    //PCF_Timer_Open   --> 开启
	uint8_t RTC_Timer_Interrupt;  //是否设置中断输出
	                                    //PCF_Time_INT_Close --> 关闭
	                                    //PCF_Time_INT_Open  --> 开启
}_PCF8563_Timer_Typedef;
 
//
//定时器控制寄存器 --> 0x0e
//
#define PCF_Timer_Open                           (uint8_t)(1<<7)     //定时器有效
#define PCF_Timer_Close                          (uint8_t)(~(1<<7))  //定时器无效
 
#define PCF_Timer_F4096                          (uint8_t)0x00  //定时器时钟频率为4096Hz
#define PCF_Timer_F64                            (uint8_t)0x01  //定时器时钟频率为64Hz
#define PCF_Timer_F1                             (uint8_t)0x02  //定时器时钟频率为1Hz
#define PCF_Timer_F160                           (uint8_t)0x03  //定时器时钟频率为1/60Hz
 
//
//中断输出开关
//
#define PCF_Time_INT_Open                        (uint8_t)(1<<0)     //定时器中断有效
#define PCF_Time_INT_Close                       (uint8_t)(~(1<<0))  //定时器中断无效
 
/******************************************************************************
                                 参数宏定义
******************************************************************************/
 
//
//控制/状态寄存器1 --> 0x00
//
#define PCF_Control_Status_NormalMode            (uint8_t)(~(1<<7))  //普通模式
#define PCF_Control_Status_EXT_CLKMode           (uint8_t)(1<<7)     //EXT_CLK测试模式
#define PCF_Control_ChipRuns                     (uint8_t)(~(1<<5))  //芯片运行
#define PCF_Control_ChipStop                     (uint8_t)(1<<5)     //芯片停止运行，所有芯片分频器异步置逻辑0
#define PCF_Control_TestcClose                   (uint8_t)(~(1<<3))  //电源复位功能失效（普通模式时置逻辑0）
#define PCF_Control_TestcOpen                    (uint8_t)(1<<3)     //电源复位功能有效
 
//
//控制/状态寄存器2 --> 0x01
//
#define PCF_Control_TI_TF1                       (uint8_t)(~(1<<4))  //当TF有效时INT有效，（取决于TIE的状态）
#define PCF_Control_TI_TF2                       (uint8_t)(1<<4)     //INT脉冲有效，（取决于TIE的状态）
                                                                           //注意：若AF和AIE有有效时，则INT一直有效
#define PCF_Control_ClearAF                      (uint8_t)(~(1<<3))  //清除报警
#define PCF_Control_ClearTF                      (uint8_t)(~(1<<2))
                                                                           //当报警发生时，AF被值逻辑1；在定时器倒计数结束时，
                                                                           //TF被值逻辑1，他们在被软件重写前一直保持原有值，
                                                                           //若定时器和报警中断都请求时，中断源有AF和TF决定，
                                                                           //若要使清除一个标志位而防止另一标志位被重写，应运用逻辑
                                                                           //指令AND
#define PCF_Alarm_INT_Open                       (uint8_t)(1<<1)     //报警中断有效
#define PCF_Alarm_INT_Close                      (uint8_t)(~(1<<1))  //报警中断无效
 
//
//秒寄存器 --> 0x02
//
#define PCF_Accuracy_ClockNo                     (uint8_t)(1<<7)     //不保证准确的时钟/日历数据
#define PCF_Accuracy_ClockYes                    (uint8_t)(~(1<<7))  //保证准确的时钟/日历数据
 
//
//分钟闹铃寄存器 --> 0x09
//
#define PCF_Alarm_MinutesOpen                    (uint8_t)(~(1<<7))  //分钟报警有效
#define PCF_Alarm_MinutesClose                   (uint8_t)(1<<7)     //分钟报警无效
 
//
//小时闹铃寄存器 --> 0x0a
//
#define PCF_Alarm_HoursOpen                      (uint8_t)(~(1<<7))  //小时报警有效
#define PCF_Alarm_HoursClose                     (uint8_t)(1<<7)     //小时报警无效
 
//
//日期闹铃寄存器 --> 0x0b
//
#define PCF_Alarm_DaysOpen                       (uint8_t)(~(1<<7))  //日报警有效
#define PCF_Alarm_DaysClose                      (uint8_t)(1<<7)     //日报警无效
 
//
//星期闹铃寄存器 --> 0x0c
//
#define PCF_Alarm_WeekDaysOpen                   (uint8_t)(~(1<<7))  //星期报警有效
#define PCF_Alarm_WeekDaysClose                  (uint8_t)(1<<7)     //星期报警无效


/******************************************************************************
                                  外部功能函数                      
******************************************************************************/

/**
 * @brief  检测 PCF8563（总线上是否存在 PCF8563 器件？芯片是否是坏片？）
 *      - 写入 PCF8563_Address_Timer_VAL 一个固定值，回读是否一致，判断芯片是否存在。
 * 
 * @param  void
 * 
 * @return
 *     - 0：成功。可继续初始化配置。
 *     - 1：错误，总线上不存在 PCF8563 器件，或已孙环。
 */
uint8_t PCF8563_Check(void);

/**
 * @brief  启动 PCF8563
 * 
 * @param  void
 * 
 * @return
 *     - none
 */
void PCF8563_Start(void);

/**
 * @brief  停止 PCF8563
 * 
 * @param  void
 * 
 * @return
 *     - none
 */
void PCF8563_Stop(void);

/**
 * @brief  设置 PCF8563 运行模式
 * 
 * @param  Mode  PCF8563 运行模式
 * 
 * @return
 *     - none
 */
void PCF8563_SetMode(uint8_t Mode);

/**
 * @brief  设置 PCF8563 电源复位功能开启与关闭
 * 
 * @param  NewState  PCF8563 电源复位功能开启与关闭
 * 
 * @return
 *     - none
 */
void PCF8563_SetPowerReset(uint8_t NewState);

/**
 * @brief  设置 PCF8563 CLK输出频率
 * 
 * @param  PCF_CLKOUTStruct  PCF8563 频率输出信息结构
 * 
 * @return
 *     - none
 */
void PCF8563_SetCLKOUT(_PCF8563_CLKOUT_Typedef* PCF_CLKOUTStruct);

/**
 * @brief  设置 PCF8563 定时器
 * 
 * @param  PCF_TimerStruct  PCF8563 定时器信息结构
 * 
 * @return
 *     - none
 */
void PCF8563_SetTimer(_PCF8563_Timer_Typedef* PCF_TimerStruct);

/**
 * @brief  设置 PCF8563 时间，主要用于后台调用，或者初始化时间用（年、月、日、星期、时、分）
 * 
 * @param  PCF_Format   PCF8563 日期格式
 * @param  PCF_Century  世纪 值
 * @param  Year         年 值
 * 
 * @return
 *     - none
 */
void PCF8563_Set_Times(uint8_t PCF_Format,\
                       uint8_t PCF_Century,\
	                   uint8_t Year, uint8_t Month, uint8_t Date, uint8_t Week,\
                       uint8_t Hour, uint8_t Minute);


/**
 * @brief  设置 PCF8563 时间信息。（时、分、秒）
 * 
 * @param  PCF_Format   PCF8563 格式
 * @param  PCF_DataStruct  PCF8563 时间信息结构
 * 
 * @return
 *     - none
 */
void PCF8563_SetTime(uint8_t PCF_Format, _PCF8563_Time_Typedef* PCF_DataStruct);

/**
 * @brief  读取 PCF8563 时间信息
 * 
 * @param  PCF_Format   PCF8563 格式
 * @param  PCF_DataStruct  PCF8563 时间信息结构
 * 
 * @return
 *     - none
 */
void PCF8563_GetTime(uint8_t PCF_Format, _PCF8563_Time_Typedef* PCF_DataStruct);

/**
 * @brief  写入 PCF8563 日期信息
 * 
 * @param  PCF_Format   PCF8563 格式
 * @param  PCF_Century  世纪 值
 * @param  PCF_DataStruct  PCF8563 时间信息结构
 * 
 * @return
 *     - none
 */
void PCF8563_SetDate(uint8_t PCF_Format, uint8_t PCF_Century, _PCF8563_Date_Typedef* PCF_DataStruct);

/**
 * @brief  读取 PCF8563 日期信息
 * 
 * @param  PCF_Format   PCF8563 格式
 * @param  PCF_Century  世纪 值指针
 * @param  PCF_DataStruct  PCF8563 时间信息结构
 * 
 * @return
 *     - none
 */
void PCF8563_GetDate(uint8_t PCF_Format, uint8_t *PCF_Century, _PCF8563_Date_Typedef* PCF_DataStruct);

#endif
