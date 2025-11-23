#include "arm_cm4.h"
#include "stm32l4xx.h"

#include "clock.h"
#include "gpio2.h"
#include "nvic.h"
#include "tprintf.h"
#include "usart.h"

#define printf tprintf

/* clang-format off */
enum {
	USART2_TX_PIN  = PA2,
    LED_PIN        = PB3,    
};

static const struct gpio_config_t {
    enum GPIO_Pin  pins;
    enum GPIO_Conf mode;
} pin_cfgs[] = {
    {USART2_TX_PIN, GPIO_AF7_USART123|GPIO_HIGH},
    {LED_PIN, GPIO_OUTPUT},
    {0, 0}, // sentinel
};

 // prio[7:6] : 4 groups,  prio[5:4] : 4 subgroups
enum { IRQ_PRIORITY_GROUPING_2_2 = 5 };
#define PRIO(grp, sub) (((grp)<<6)|((sub)<<4))
struct {
    enum IRQn_Type irq;
    uint8_t        prio;
} irqprios[] = {
    {USART2_IRQn,   PRIO(2,0)},
    {None_IRQn, 0xff},
};
#undef PRIO

// USART2 is the console, for debug messages, it runs IRQ driven.
static struct Ringbuffer usart2tx;

void _putchar(char character) {
	if (!ringbuffer_full(&usart2tx)) {
		ringbuffer_put_head(&usart2tx, character);
	} else {
		ringbuffer_clear(&usart2tx);
		for (const char* p = "!OVFL!"; *p != 0; ++p) {
			ringbuffer_put_head(&usart2tx, *p);
		}
	}
	uart_start(&USART2);
	return;
}

void USART2_Handler(void) { uart_irq_handler(&USART2, &usart2tx); }

void hexdump(size_t len, const uint8_t* ptr) {
    static const char* hexchar = "01234567890abcdef";
    for (size_t i = 0; i<len; ++i) {
        _putchar(' ');
        _putchar(hexchar[ptr[i]>>4]);
        _putchar(hexchar[ptr[i]&0xf]);
    }
}

static uint64_t last = 0;

// Timer 6: 1Hz status
void TIM6_DACUNDER_Handler(void) {
    uint64_t now = cycleCount();

    if ((TIM6.SR & TIM1_SR_UIF) == 0)
        return;
    TIM6.SR &= ~TIM1_SR_UIF;

//    printf("\nuptime %.6f\n", (double)now/64E6);

    // now /= C_US; // microseconds
    // uint64_t sec = now / 1000000;
    // now %= 1000000;
    // printf("uptime %llu.%06llu\n", sec, now);
    printf("uptime %llu\n", now - last);
    last = now;
    digitalToggle(LED_PIN);
 }


// some device specific variables defined in .ld file
extern uint32_t UNIQUE_DEVICE_ID[3];            // Ref man Section 47.1
extern uint16_t TS_CAL1, TS_CAL2, VREFINT;      // Datasheet 3.18.1 and 3.18.2


static const char *clksrcstr[]  = {"MSI", "HSI16", "HSE", "PLL"};
static const char *pplsrcstr[]  = {"NONE", "MSI", "HSI16", "HSE"};
static const char *memmodestr[] = {
                "FLASH",  // 000: Main Flash memory mapped at 0x00000000.
                "SYS",    // 001: System Flash memory mapped at 0x00000000.
                "<2>",    // 010: Reserved
                "SRAM1",  // 011: SRAM1 mapped at 0x00000000.
                "<4>",    // 100: Reserved
                "<5>",    // 101: Reserved
                "QSPI",   // 110: QUADSPI memory mapped at 0x00000000.
                "<7>",    // 111: Reserved
};


void main(void) {
	uint8_t rf = (RCC.CSR >> 24) & 0xfc;
	RCC.CSR |= RCC_CSR_RMVF; // Set RMVF bit to clear the reset flags

	NVIC_SetPriorityGrouping(IRQ_PRIORITY_GROUPING_2_2);
    for (int i = 0; irqprios[i].irq != None_IRQn; i++) {
        NVIC_SetPriority(irqprios[i].irq, irqprios[i].prio);
    }

    // Enable all the devices we are going to need
	RCC.AHB2ENR  |= RCC_AHB2ENR_GPIOAEN|RCC_AHB2ENR_GPIOBEN;
	RCC.APB1ENR1 |= RCC_APB1ENR1_USART2EN | RCC_APB1ENR1_TIM6EN;

	for (const struct gpio_config_t* p = pin_cfgs; p->pins; ++p) {
		gpioConfig(p->pins, p->mode);
	}

	gpioLock(PAAll);
	gpioLock(PBAll);

    // prepare USART2 for console and debug messages
	ringbuffer_clear(&usart2tx);
	uart_init(&USART2, 115200);
	NVIC_EnableIRQ(USART2_IRQn);

    printf("SWREV:%x\n", __REVISION__);
    printf("CPUID:%08lx\n", SCB.CPUID);
    printf("IDCODE:%08lx\n", DBGMCU.IDCODE);
    printf("DEVID:%08lx:%08lx:%08lx\n", UNIQUE_DEVICE_ID[2], UNIQUE_DEVICE_ID[1], UNIQUE_DEVICE_ID[0]);
    printf("RESET:%02x%s%s%s%s%s%s%s%s\n", rf, rf & 0x80 ? " LPWR" : "", rf & 0x40 ? " WWDG" : "", rf & 0x20 ? " IWDG" : "",
    rf & 0x10 ? " SFT" : "", rf & 0x08 ? " POR" : "", rf & 0x04 ? " PIN" : "", rf & 0x02 ? " OBL" : "", rf & 0x02 ? "  FW" : "");
    if (rcc_cfgr_get_sws() == 3) {
        printf("PPLSRC: %s%s\n", pplsrcstr[rcc_pllcfgr_get_pllsrc()],
            ((rcc_pllcfgr_get_pllsrc() == 3) && (RCC.CR & RCC_CR_HSEBYP)) ? " (CK_IN)" : "");
    } else {
        printf("CLKSRC:%s%s\n", clksrcstr[rcc_cfgr_get_sws()],
            ((rcc_cfgr_get_sws() == 2) && (RCC.CR & RCC_CR_HSEBYP)) ? " (CK_IN)" : "");
    }
    printf("MEM: %s vtbl:%08lx\n", memmodestr[syscfg_memrmp_get_mem_mode()], SCB.VTOR);
    printf("ADCCAL: TEMP %u %u VREF %u\n", TS_CAL1, TS_CAL2, VREFINT);
	uart_wait(&USART2);

    // set up TIM6 for a 1Hz hearbeat
	TIM6.DIER |= TIM6_DIER_UIE;
    TIM6.PSC = (CLOCKSPEED_HZ/10000) - 1;
    TIM6.ARR = 10000 - 1; // 10KHz/10000 = 1Hz
    TIM6.CR1 |= TIM6_CR1_CEN;
    NVIC_EnableIRQ(TIM6_DACUNDER_IRQn);

    // Initialize the independent watchdog
    // IWDG.KR = 0x5555;  // enable watchdog config
    // IWDG.PR = 0;       // prescaler /4 -> 10khz
    // IWDG.RLR = 3200;   // count to 3200 -> 320ms timeout
    // IWDG.KR = 0xcccc;  // start watchdog countdown
    // //  IWDG.KR = 0xAAAA;  // pet the watchdog TODO check all subsystems

    for(;;) {
    	__WFI();
    }
}
