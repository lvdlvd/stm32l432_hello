#include "arm_cm4.h"
#include "stm32l4xx.h"

#include <assert.h>
#include <string.h>

#include "clock.h"

void main(void) __attribute__((noreturn));
extern void main(void);                             // in main.c
extern void (*const vector_table[])(void);          // in vector.c
extern uint32_t _imgsrc, _imgdst, _imgsize, _bssstart, _bsssize;        // provided by linker script as addresses of the named variables

// how many clock cycles to wait before deciding not to use HSE (CK_IN)
enum { HSE_RDY_TIMEOUT = 40000 };

// This is the first thing that runs after the CPU comes out of reset.
// It loads the data segment, clears the Blank Stuff Segment,
// sets up the interrupt vector table to load from flash,
// initializes fault handling and FPU access,
// sets up the system clock to run at 80MHz, starts the system
// timer and calls main().
void Reset_Handler(void) __attribute__((noreturn));
void Reset_Handler(void) {

    // Copy text and data from FLASH to SRAM12
    memcpy(&_imgdst, &_imgsrc, (size_t)&_imgsize);

    // clear BSS segment (see stm32XXXX.ld)
    memset(&_bssstart, 0, (size_t)&_bsssize);

    // PM0214 sec 4.4.4 Vector Table Relocation
    // NOTE: without this vector_table may not be referenced, and will only be included
    // if the linker is invoked with flag --unedefined=vector_table or
    // EXTERN(vector_table) in the linker script.
    // SCB.VTOR = (uintptr_t)&vector_table;  // provided in vectors.c

    RCC.APB2ENR |= RCC_APB2ENR_SYSCFGEN;
    syscfg_memrmp_set_mem_mode(3); //  SRAM1 mapped at 0x00000000

    // PM0214 sec 4.4.7
    SCB.CCR |= SCB_CCR_DIV_0_TRP;  // division by zero causes trap
    // enable usage/bus/mem fault separate handlers (in fault.c)
    // PM0214 sec 4.4.9
    SCB.SHCSR |= SCB_SHCSR_USGFAULTENA | SCB_SHCSR_BUSFAULTENA | SCB_SHCSR_MEMFAULTENA;

    // section 4.6.1 CP10/CP11 (FPU) Full Access
    fpu_cpacr_cpacr_set_cp(0xf);

	// Disable all interrupts and clear pending bits
    RCC.CIER = 0;
    RCC.CICR = RCC.CIFR;

    // See RM0394 Section 6 for details on configuration of the clock tree.

    // feeding all peripherals to run at 64Mhz
    rcc_cfgr_set_hpre(0);   // AHB HCLK = SYSCLK  =  64MHz
    rcc_cfgr_set_ppre1(0);  // APB1 PCLK = AHB HCLK
    rcc_cfgr_set_ppre2(0);  // APB2 PCLK = AHB HCLK

    // set system clock to PLL 64 MHz, fed by HSI at 16MHz
    RCC.CR |= RCC_CR_HSION;
    // Wait till HSI is ready
    while ((RCC.CR & RCC_CR_HSIRDY) == 0) {
        __NOP();
    }

    rcc_pllcfgr_set_pllsrc(2);  		// select 2:HSI16 source (16MHz)
    rcc_pllcfgr_set_pllm(3);    		// 0..15 : vco_in = HSI / (1+m)  4..16MHz        16/4 = 4MHz
    rcc_pllcfgr_set_plln(32);           // 8...86 : vco_out = vco_in * n = 64...344MHz    4 * 32 = 128MHz
    rcc_pllcfgr_set_pllr(0);            // 0,1,2,3 -> p=2,4,6,8  : sysclk = vco_out / p <= 170MHz  4 * 32 / 2 = 64MHz
    RCC.PLLCFGR |= RCC_PLLCFGR_PLLREN;  // emable R output (system clock)
    RCC.CR |= RCC_CR_PLLON;             // switch on the PLL

    // prepare the flash, RM0394 section 3.3.3
    // prefetch and caching only when running from FLASH, not when running from SRAM1
    //  FLASH.ACR |= FLASH_ACR_PRFTEN | FLASH_ACR_ICEN | FLASH_ACR_DCEN;
    flash_acr_set_latency(3);  // 3 wait states (4 cycles) cf 3.3.3 p79 table 9
    while (flash_acr_get_latency() != 3) {
//		__NOP();
    }

    // Wait till PLL is ready
    while ((RCC.CR & RCC_CR_PLLRDY) == 0) {
//		__NOP();
    }

    // Select PLL as system clock source and wait until it takes effect
    rcc_cfgr_set_sw(3);
    while (rcc_cfgr_get_sws() != 3) {
//		__NOP();
    }

	// Prepare the Cortex system timer
	stk_load_set_reload(STK_LOAD_RELOAD);  // maximum value
	stk_val_set_current(STK_LOAD_RELOAD);
	STK.CTRL |= STK_CTRL_CLKSOURCE | STK_CTRL_ENABLE;

	main();

	for (;;) {
//		__NOP();  // hang
	}
}


static volatile uint32_t clockticks = 1;	// overflows after 2^56/(64Mhz) = 35 years.
static volatile uint32_t clockprev_val = 0;	// STK.VAL last time we called cycleCount

uint64_t cycleCount(void) {
	uint32_t primsk = __get_PRIMASK();
	__disable_irq();
	uint32_t c = clockticks;
	uint32_t v = stk_val_get_current();
	if (v > clockprev_val) {
		++c;
		clockticks = c;
	}
	clockprev_val = v;
	__DMB();
	__set_PRIMASK(primsk);
	return (((uint64_t)c)<<24) - v;
}

void delay(uint32_t usec) {
	uint64_t then = cycleCount() + C_US * usec;
	while (cycleCount() < then) {
//		__NOP();
	}
}