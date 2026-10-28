/**
 * @file  : main.c
 * @author: julia jiang
 * @brief : read acceleration and print raw values and Gs over UART
 */

 #include <stdint.h>
 #include <stdio.h>
 #include <string.h>
 #include "stm32l4xx_hal.h"
 
 #define ACCEL_ADDR       (0x19U << 1)
 #define CTRL_REG1_A      0x20U
 #define CTRL_REG4_A      0x23U
 #define OUT_X_L_A        0x28U
 
 I2C_HandleTypeDef hi2c2 = {0};
 UART_HandleTypeDef hlpuart1 = {0};
 
 /* use the internal MSI clock at 4 MHz */
 void init_clock(void)
 {
     RCC_OscInitTypeDef oscillator = {0};
     RCC_ClkInitTypeDef clock = {0};
     RCC_PeriphCLKInitTypeDef peripheral_clock = {0};
 
     oscillator.OscillatorType = RCC_OSCILLATORTYPE_MSI;
     oscillator.MSIState = RCC_MSI_ON;
     oscillator.MSICalibrationValue = RCC_MSICALIBRATION_DEFAULT;
     oscillator.MSIClockRange = RCC_MSIRANGE_6;
     oscillator.PLL.PLLState = RCC_PLL_NONE;
 
     HAL_RCC_OscConfig(&oscillator);
 
     clock.ClockType = RCC_CLOCKTYPE_SYSCLK |
                       RCC_CLOCKTYPE_HCLK |
                       RCC_CLOCKTYPE_PCLK1 |
                       RCC_CLOCKTYPE_PCLK2;
     clock.SYSCLKSource = RCC_SYSCLKSOURCE_MSI;
     clock.AHBCLKDivider = RCC_SYSCLK_DIV1;
     clock.APB1CLKDivider = RCC_HCLK_DIV1;
     clock.APB2CLKDivider = RCC_HCLK_DIV1;
 
     HAL_RCC_ClockConfig(&clock, FLASH_LATENCY_0);
 
     peripheral_clock.PeriphClockSelection = RCC_PERIPHCLK_I2C2 |
                                            RCC_PERIPHCLK_LPUART1;
     peripheral_clock.I2c2ClockSelection = RCC_I2C2CLKSOURCE_PCLK1;
     peripheral_clock.Lpuart1ClockSelection = RCC_LPUART1CLKSOURCE_PCLK1;
 
     HAL_RCCEx_PeriphCLKConfig(&peripheral_clock);
 }
 
 /* enable peripheral clocks */
 void enable_clock(void)
 {
     __HAL_RCC_GPIOF_CLK_ENABLE();
     __HAL_RCC_I2C2_CLK_ENABLE();
     __HAL_RCC_GPIOG_CLK_ENABLE();
     __HAL_RCC_LPUART1_CLK_ENABLE();
     __HAL_RCC_PWR_CLK_ENABLE();
 
     HAL_PWREx_EnableVddIO2();
 }
 
 /* PF0 = SDA, PF1 = SCL, PG7 = UART TX */
 void init_pins(void)
 {
     GPIO_InitTypeDef pins = {0};
 
     pins.Pin = GPIO_PIN_0 | GPIO_PIN_1;
     pins.Mode = GPIO_MODE_AF_OD;
     pins.Pull = GPIO_PULLUP;
     pins.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
     pins.Alternate = GPIO_AF4_I2C2;
 
     HAL_GPIO_Init(GPIOF, &pins);
 
     pins.Pin = GPIO_PIN_7;
     pins.Mode = GPIO_MODE_AF_PP;
     pins.Pull = GPIO_NOPULL;
     pins.Alternate = GPIO_AF8_LPUART1;
 
     HAL_GPIO_Init(GPIOG, &pins);
 }
 
 void init_i2c(void)
 {
     hi2c2.Instance = I2C2;
     hi2c2.Init.Timing = 0x00200F17U;
     hi2c2.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
 
     HAL_I2C_Init(&hi2c2);
     HAL_I2CEx_ConfigAnalogFilter(&hi2c2, I2C_ANALOGFILTER_ENABLE);
     HAL_I2CEx_ConfigDigitalFilter(&hi2c2, 0U);
 }
 
 /* 115200 baud, 8 data bits, no parity, 1 stop bit */
 void init_uart(void)
 {
     hlpuart1.Instance = LPUART1;
     hlpuart1.Init.BaudRate = 115200U;
     hlpuart1.Init.WordLength = UART_WORDLENGTH_8B;
     hlpuart1.Init.StopBits = UART_STOPBITS_1;
     hlpuart1.Init.Parity = UART_PARITY_NONE;
     hlpuart1.Init.Mode = UART_MODE_TX;
     hlpuart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
 
     HAL_UART_Init(&hlpuart1);
 }
 
 void uart_print(const char *text)
 {
     HAL_UART_Transmit(&hlpuart1, (uint8_t *)text, strlen(text), 100U);
 }
 
 /* high resolution, +/-2 G, 100 Hz, XYZ enabled */
 void init_accelerometer(void)
 {
     uint8_t ctrl4 = 0x88U;
     uint8_t ctrl1 = 0x57U;
 
     HAL_I2C_Mem_Write(&hi2c2, ACCEL_ADDR, CTRL_REG4_A,
                      I2C_MEMADD_SIZE_8BIT, &ctrl4, 1U, 100U);
 
     HAL_I2C_Mem_Write(&hi2c2, ACCEL_ADDR, CTRL_REG1_A,
                      I2C_MEMADD_SIZE_8BIT, &ctrl1, 1U, 100U);
 
     HAL_Delay(100U);
 }
 
 /* print without floating-point printf */
 void print_axis(char axis, int16_t raw)
 {
     char text[64];
     int mg = raw / 16;
     int magnitude = (mg < 0) ? -mg : mg;
 
     snprintf(text, sizeof(text), "%c raw:%6d  %c%d.%03d G   ",
              axis, (int)raw, (mg < 0) ? '-' : '+',
              magnitude / 1000, magnitude % 1000);
 
     uart_print(text);
 }
 
 void read_acceleration(void)
 {
     uint8_t data[6] = {0};
 
     HAL_I2C_Mem_Read(&hi2c2, ACCEL_ADDR, OUT_X_L_A | 0x80U,
                     I2C_MEMADD_SIZE_8BIT, data, 6U, 100U);
 
     int16_t x = (int16_t)((uint16_t)data[0] |
                          ((uint16_t)data[1] << 8));
     int16_t y = (int16_t)((uint16_t)data[2] |
                          ((uint16_t)data[3] << 8));
     int16_t z = (int16_t)((uint16_t)data[4] |
                          ((uint16_t)data[5] << 8));
 
     print_axis('X', x);
     print_axis('Y', y);
     print_axis('Z', z);
     uart_print("\r\n");
 }
 
 int main(void)
 {
     HAL_Init();
 
     init_clock();
     enable_clock();
     init_pins();
     init_i2c();
     init_uart();
     init_accelerometer();
 
     uart_print("raw signed 16-bit readings and acceleration in G\r\n");
 
     while (1) {
         read_acceleration();
         HAL_Delay(200U);
     }
 }