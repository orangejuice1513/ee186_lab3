/**
 * @file  : main.c
 * @author: julia jiang
 * @brief : read the accelerometer and magnetometer IDs using I2C
 */

 #include <stdint.h>
 #include "stm32l4xx_hal.h"
 
 /* device addresses and register sub-addresses */
 #define ACCEL_ADDR       (0x19 << 1)
 #define MAG_ADDR         (0x1E << 1)
 #define WHO_AM_I_A       0x0F
 #define WHO_AM_I_M       0x4F
 
 I2C_HandleTypeDef hi2c2 = {0};
 
 
 /* select PCLK1 as the I2C2 clock source */
 void init_clock(void)
 {
     RCC_PeriphCLKInitTypeDef peripheral_clock = {0};
 
     peripheral_clock.PeriphClockSelection = RCC_PERIPHCLK_I2C2;
     peripheral_clock.I2c2ClockSelection = RCC_I2C2CLKSOURCE_PCLK1;
 
     HAL_RCCEx_PeriphCLKConfig(&peripheral_clock);
 }
 
 /* enable GPIO F and I2C2 clocks */
 void enable_clock(void)
 {
     __HAL_RCC_GPIOF_CLK_ENABLE();
     __HAL_RCC_I2C2_CLK_ENABLE();
 }
 
 /* PF0 = SDA, PF1 = SCL */
 void init_pins(void)
 {
     GPIO_InitTypeDef pins = {0};
 
     pins.Pin = GPIO_PIN_0 | GPIO_PIN_1;
     pins.Mode = GPIO_MODE_AF_OD;
     pins.Pull = GPIO_PULLUP;
     pins.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
     pins.Alternate = GPIO_AF4_I2C2;
 
     HAL_GPIO_Init(GPIOF, &pins);
 }
 
 /* approximately 100 kHz with a 4 MHz peripheral clock */
 void init_i2c(void)
 {
     hi2c2.Instance = I2C2;
     hi2c2.Init.Timing = 0x00200F17;
     hi2c2.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
 
     HAL_I2C_Init(&hi2c2);
     HAL_I2CEx_ConfigAnalogFilter(&hi2c2, I2C_ANALOGFILTER_ENABLE);
     HAL_I2CEx_ConfigDigitalFilter(&hi2c2, 0);
 }
 
 /* read the fixed identification values */
 void read_sensor_ids(void)
{
    uint8_t accel_id = 0; // receives accelerometer id
    uint8_t mag_id = 0; // receivers magnetometer id 

    HAL_I2C_Mem_Read(&hi2c2, ACCEL_ADDR, WHO_AM_I_A,
                    I2C_MEMADD_SIZE_8BIT, &accel_id, 1, 100);

    HAL_I2C_Mem_Read(&hi2c2, MAG_ADDR, WHO_AM_I_M,
                    I2C_MEMADD_SIZE_8BIT, &mag_id, 1, 100);
}
 
 int main(void)
 {
     HAL_Init();
     init_clock();
     enable_clock();
     init_pins();
     init_i2c();
     read_sensor_ids();
 
     while (1) {
     }
 }