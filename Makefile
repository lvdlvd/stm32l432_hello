# Tool names
PREFIX=arm-none-eabi-
CC          := $(PREFIX)gcc
SIZE        := $(PREFIX)size
OBJCOPY     := $(PREFIX)objcopy

REVISION := $(shell git log -1 --format="%h" || echo "0000000")

ARCH_FLAGS       = -mthumb -mcpu=cortex-m4 -mfloat-abi=hard -mfpu=fpv4-sp-d16
# preventively disable -flto, which causes problems on the compinator
# OPT_FLAGS      = -O2 -flto -fuse-linker-plugin -ffunction-sections -fdata-sections -fverbose-asm -ffat-lto-objects -fno-exceptions -fno-unwind-tables
OPT_FLAGS        = -O2 -fuse-linker-plugin -ffunction-sections -fdata-sections -fverbose-asm -ffat-lto-objects -fno-exceptions -fno-unwind-tables
WARN_FLAGS   = -Werror -Wfatal-errors -Wall -Wextra -Wunsafe-loop-optimizations -Wdouble-promotion -Wundef  -Wno-pedantic -Wno-enum-conversion -Wno-deprecated
DEBUG_FLAGS      = -ggdb3 -D__REVISION__='0x$(REVISION)'
#DEBUG_FLAGS  += -fcallgraph-info=su
CFLAGS           = -std=gnu17 $(ARCH_FLAGS) $(OPT_FLAGS) $(WARN_FLAGS) $(DEBUG_FLAGS)
LDFLAGS          = -nostartfiles -lnosys -static $(ARCH_FLAGS) $(OPT_FLAGS) $(WARN_FLAGS) $(DEBUG_FLAGS) -Wl,-gc-sections,-Map,main.map -Wl,--cref

.DEFAULT_GOAL := main.elf

OBJS = \
	vectors.o \
	boot.o \
	fault.o \
	gpio2.o \
	tprintf.o \
	main.o \

$(OBJS): Makefile

%.o: %.c
	$(CC) -c -o $@ $(CFLAGS) $<

%.elf: stm32l432kc.ld
main.elf: $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS) -Tstm32l432kc.ld
	@$(SIZE) $@ | awk 'BEGIN {printf("%6s %6s %6s  %6s %6s %s\n", "text", "data", "bss", "flash", "ram", "file")} $$1*1 == $$1 {printf("%6d %6d %6d  %6d %6d %s\n", $$1, $$2, $$3-$$1, $$1+$$2, $$2+$$3, $$6)}'

disthex:main.elf
	$(OBJCOPY) -O ihex main.elf hello-$(REVISION).hex

flash: main.elf
	openocd \
		-f interface/stlink.cfg \
		-f target/stm32l4x.cfg \
		-c init \
		-c halt \
		-c "flash probe 0" \
		-c "stm32l4x mass_erase 0" \
		-c "sleep 100" \
		-c "program $< verify" \
		-c reset \
		-c shutdown	

clean:
	rm -f *~ *.o *.hex *.bin *.elf *.map

depend:
	makedepend -Y. -w150 *.c

# DO NOT DELETE

boot.o: arm_cm4.h stm32l4xx.h clock.h
fault.o: arm_cm4.h stm32l4xx.h
gpio2.o: gpio2.h stm32l4xx.h
main.o: arm_cm4.h stm32l4xx.h clock.h gpio2.h nvic.h tprintf.h usart.h ringbuffer.h
tprintf.o: tprintf.h
vectors.o: arm_cm4.h stm32l4xx.h
