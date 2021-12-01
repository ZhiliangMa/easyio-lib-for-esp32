
#include "easyio.h"

#define LED 33
#define KEY 0

// 任务句柄，包含创建任务的所有状态，对任务的操作都通过操作任务句柄实现
TaskHandle_t led_task_Handler = NULL;

// led_task 任务，控制LED闪烁
void led_task(void* arg)
{
    // 配置LED为推挽输出，设置初始电平为0
    led_init(LED, 0);
    while(1) {
        // LED状态闪烁
        led_blink(LED);
        vTaskDelay(200 / portTICK_PERIOD_MS);
    }
}

// i2c_mpu6050_task 任务。读取 MPU6050的 XYZ轴加速度计、XYZ轴陀螺仪 数据。IMU兼容 MPU6050/9250、ICM-20602/6/8
void i2c_mpu6050_task(void* arg)
{
    int16_t accel[3],gyro[3];

    // 配置I2C0-主机模式，400K，指定 SCL-14，SDA-4
    i2c_master_init(I2C_NUM_0, 400000, GPIO_NUM_14, GPIO_NUM_4);
    // 初始化 MPU6050/9250。检测 MPU6050/9250。并通过I2Cx总线写寄存器，来对功能进行配置。
    i2c_mpu6050_init(I2C_NUM_0);

    while(1) {
        // 读取 MPU6050/9250 的 XYZ轴加速度计、XYZ轴陀螺仪 数据
        i2c_mpu6050_get_accel_gyro(I2C_NUM_0, accel, gyro);
        printf("Accel: x:%d\ty:%d\tz:%d\t||\tGyro: x:%d\ty:%d\tz:%d\n", accel[0], accel[1], accel[2], gyro[0], gyro[1], gyro[2]);
        vTaskDelay(200 / portTICK_PERIOD_MS);
    }
}

void app_main(void)
{
    // 创建 led_task 任务，任务栈空间大小为 2048，任务优先级为3（configMINIMAL_STACK_SIZE 不能用 printf，会导致ESP32反复重启）
    xTaskCreate(led_task, "led_task", 2048, NULL, 3, &led_task_Handler);

    // 创建 i2c_mpu6050_task 任务。
    xTaskCreate(i2c_mpu6050_task, "i2c_mpu6050_task", 2048, NULL, 3, NULL);
}
