# stm32l432_hello
basic environment for the stm32l432kc nucleo and related boards

## STM32L431kx UFQFPN32 Pin Assignments:

| Pin  | Function  | DIR | Config   | Connected to                    |
| ---- | --------- | --- | -------- | ------------------------------- |
| PA0  | CK_IN     | in  |          | 8Mhz external clock source      |
| PA1  |           |     |          |                                 |
| PA2  | USART2 TX | out | AF7 High | Nucleo 32 ST-Link VCTX          |
| PA3  |           |     |          |                                 |
| PA4  |           |     |          |                                 |
| PA5  |           |     |          |                                 |
| PA6  |           |     |          |                                 |
| PA7  |           |     |          |                                 |
| PA8  |           |     |          |                                 |
| PA9  |           |     |          |                                 |
| PA10 |           |     |          |                                 |
| PA11 |           |     |          | CANRX / USBDM                   |
| PA12 |           |     |          | CANTX / USBDP                   |
| PA13 | SWDIO     |     |          | Nucleo-32 ST-Link               |
| PA14 | SWCLK     |     |          | Nucleo-32 ST-Link               |
| PA15 | USART2 RX | in  | AF3      | Nucleo-32 ST-Link VCRX          |
| PB0  |           |     |          |                                 |
| PB1  |           |     |          |                                 |
| PB2  |           |     |          | (not exposed on 32-pin package) |
| PB3  |           |     |          | Nucleo-32 User LED              |
| PB4  |           |     |          |                                 |
| PB5  |           |     |          |                                 |
| PB6  |           |     |          |                                 |
| PB7  |           |     |          |                                 |

# References

- UM1956  [User manual STM32 Nucleo-32 boards (MB1180)](https://www.st.com/resource/en/user_manual/um1956-stm32-nucleo32-boards-mb1180-stmicroelectronics.pdf)
- RM0394  [Reference manual  STM324Lxx MCUs](https://www.st.com/resource/en/reference_manual/rm0394-stm32l41xxx42xxx43xxx44xxx45xxx46xxx-advanced-armbased-32bit-mcus-stmicroelectronics.pdf)
- PM0214  [STM32 Cortex®-M4 MCUs and MPUs programming manual](https://www.st.com/resource/en/programming_manual/pm0214-stm32-cortexm4-mcus-and-mpus-programming-manual-stmicroelectronics.pdf)
- DS11451 [STM32L432Kx Datasheet](https://www.st.com/resource/en/datasheet/stm32l432kc.pdf)