# stm32l432_hello
Basic environment for the stm32l432kc nucleo and related boards

## STM32L431kx UFQFPN32 Pin Assignments:

Template table, see UM1256 and DS11451 Section 4 for details and more possibilities.

| Pin  | Function                           | DIR | CFG | Connected to           |
| ---- | ---------------------------------- | --- | --- | ---------------------- |
| PA0  | CK_IN / TIM2 CH1                   | in  |     | Nucleo 32 MCO 8MHz     |
| PA1  | TIM2 CH2                           |     |     |                        |
| PA2  | LPUART1 TX / USART2 TX / TIM2 CH3  |     |     | Nucleo 32 ST-Link VCTX |
| PA3  | LPUART1 RX / USART2 RX / TIM2 CH4  |     |     |                        |
| PA4  | SPI1 NSS / SPI3 NSS                |     |     |                        |
| PA5  | SPI1 SCK                           |     |     |                        |
| PA6  | SPI1 MISO                          |     |     |                        |
| PA7  | SPI1 MOSI                          |     |     |                        |
| PA8  | TIM1 CH1 / MCO                     |     |     |                        |
| PA9  | TIM1 CH2 / I2C1 SCL / USART1 TX    |     |     |                        |
| PA10 | TIM1 CH3 / I2C1 SDA / USART1 RX    |     |     |                        |
| PA11 | TIM1 CH4 / CANRX / USBDM           |     |     |                        |
| PA12 | CANTX / USBDP                      |     |     |                        |
| PA13 | SWDIO                              |     |     | Nucleo-32 ST-Link      |
| PA14 | SWCLK                              |     |     | Nucleo-32 ST-Link      |
| PA15 | USART2 RX / SPI1 NSS / SPI3 NSS    | in  |     | Nucleo-32 ST-Link VCRX |
| PB0  | SPI1 NSS                           |     |     |                        |
| PB1  | LPUART1 RTS_DE / LPTIM2 IN1        |     |     |                        |
| PB3  | SPI1 SCK  / SPI3 SCK               | out |     | Nucleo-32 User LED     |
| PB4  | SPI1 MISO / SPI3 MISO              |     |     |                        |
| PB5  | LPTIM1 IN1 / SPI1 MOSI / SPI3 MOSI |     |     |                        |
| PB6  | LPTIM1 ETR / USART1 TX             |     |     |                        |
| PB7  | LPTIM1 IN2 / USART1 TX             |     |     |                        |

Note:  PB2 is not exposed on 32-pin package.

Note: Analog inputs: PA0-7 = ADC1 IN5-12, PB0-1 = ADC1 IN 15-16

# References

- [UM1956](https://www.st.com/resource/en/user_manual/um1956-stm32-nucleo32-boards-mb1180-stmicroelectronics.pdf) User manual STM32 Nucleo-32 boards (MB1180)
- [RM0394](https://www.st.com/resource/en/reference_manual/rm0394-stm32l41xxx42xxx43xxx44xxx45xxx46xxx-advanced-armbased-32bit-mcus-stmicroelectronics.pdf) Reference manual STM324Lxx MCUs
- [PM0214](https://www.st.com/resource/en/programming_manual/pm0214-stm32-cortexm4-mcus-and-mpus-programming-manual-stmicroelectronics.pdf) Programming Manual STM32 Cortex®-M4 MCUs and MPUs
- [DS11451](https://www.st.com/resource/en/datasheet/stm32l432kc.pdf) Datasheet STM32L432Kx