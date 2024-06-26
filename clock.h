#pragma once
// This firmware uses the Cortex M4 builtin 'Systick' as a timer that counts in
// clock cycles. See PM0214 Section 4.5
#include "stm32l4xx.h"
#include <stdint.h>

enum { CLOCKSPEED_HZ = 64000000, C_US = CLOCKSPEED_HZ / 1000000 };

// For HCLK 64Mhz:  4: 64MHz/32 = 2Mhz, 3: 64MHz/16 = 4MHz.
enum spi_clock_div_t { SPI_8MHz = 2, SPI_4MHz = 3, SPI_2MHz = 4, SPI_1MHz = 5 };

static inline uint32_t usart_brr(uint32_t baud) {
  return (CLOCKSPEED_HZ + baud / 2) / baud;
}

// in boot.c, updated by SysTick irq handler
extern volatile uint64_t clockticks;

// return the current time as measured in cpu cycles since initCycleCount was
// called.
static inline uint64_t cycleCount(void) {
  return clockticks - stk_val_get_current();
}

// spinlock using the systick
// usec = 10...1000000 (.01 ms ..  1s)
static inline void delay(uint32_t usec) {
  uint64_t then = cycleCount() + C_US * usec;
  while (cycleCount() < then) {
    ;
  }
}
