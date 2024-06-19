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
    {SysTick_IRQn,  PRIO(0,0)},
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
	usart_start(&USART2);
	return;
}

void USART2_Handler(void) { usart_irq_handler(&USART2, &usart2tx); }

void hexdump(size_t len, const uint8_t* ptr) {
    static const char* hexchar = "01234567890abcdef";
    for (size_t i = 0; i<len; ++i) {
        _putchar(' ');
        _putchar(hexchar[ptr[i]>>4]);
        _putchar(hexchar[ptr[i]&0xf]);
    }
}

// Timer 6: 1Hz status
void TIM6_DACUNDER_Handler(void) {
    uint64_t now = cycleCount();

    if ((TIM6.SR & TIM1_SR_UIF) == 0)
        return;
    TIM6.SR &= ~TIM1_SR_UIF;

    printf("\nuptime %.6f\n", (double)now/64E6);

    // now /= C_US; // microseconds
    // uint64_t sec = now / 1000000;
    // now %= 1000000;
    // printf("\nuptime %llu.%06llu\n", sec, now);
    digitalToggle(LED_PIN);
 }

extern uint32_t UNIQUE_DEVICE_ID[3]; // Section 47.1, defined in .ld file

static const char* pplsrcstr[] = { "NONE", "MSI", "HSI16", "HSE" };



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
	usart_init(&USART2, 115200);
	NVIC_EnableIRQ(USART2_IRQn);

#ifdef __REVISION__
	printf("SWREV:%s\n", __REVISION__);
#endif
	printf("CPUID:%08lx\n",  SCB.CPUID);
	printf("IDCODE:%08lx\n",  DBGMCU.IDCODE);
	printf("DEVID:%08lx:%08lx:%08lx\n", UNIQUE_DEVICE_ID[2], UNIQUE_DEVICE_ID[1], UNIQUE_DEVICE_ID[0]);
	printf("RESET:%02x%s%s%s%s%s%s\n", rf, rf & 0x80 ? " LPWR" : "", rf & 0x40 ? " WWDG" : "", rf & 0x20 ? " IWDG" : "",
	          rf & 0x10 ? " SFT" : "", rf & 0x08 ? " POR" : "", rf & 0x04 ? " PIN" : "");
    printf("PPLSRC: %s%s\n", pplsrcstr[rcc_pllcfgr_get_pllsrc()], 
        (RCC.CR & RCC_CR_HSEBYP) && (rcc_pllcfgr_get_pllsrc()==3) ? " (CK_IN)" :""
    );
	usart_wait(&USART2);

    // set up TIM6 for a 1Hz hearbeat
    // note: it pushes spiq messages so do not start this before the BMI/BME are configured.
	TIM6.DIER |= TIM6_DIER_UIE;
    TIM6.PSC = (CLOCKSPEED_HZ/10000) - 1;
    TIM6.ARR = 10000 - 1; // 10KHz/10000 = 1Hz
    TIM6.CR1 |= TIM6_CR1_CEN;
    NVIC_EnableIRQ(TIM6_DACUNDER_IRQn);


    // Initialize the independent watchdog
    IWDG.KR = 0x5555;  // enable watchdog config
    IWDG.PR = 0;       // prescaler /4 -> 10khz
    IWDG.RLR = 3200;   // count to 3200 -> 320ms timeout
    IWDG.KR = 0xcccc;  // start watchdog countdown

    for(;;) {
    	__WFI();

        IWDG.KR = 0xAAAA;  // pet the watchdog TODO check all subsystems

        uint64_t start = cycleCount();
        float f = 0;
        for (int i = 1; i < 100000; ++i) {
            float g = i;
            f += 1.0f/(g*g);
        }
        printf("f: %f  %lld us\r", (double)f, (cycleCount()-start)/C_US);

    }
}
