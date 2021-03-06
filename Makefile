
#TOOL_BASE=
#STLINK=

ifndef TOOL_BASE
$(error TOOL_BASE is not set)
endif

ifndef STLINK
$(error STLINK is not set)
endif

TOOL_PATH=$(TOOL_BASE)/bin

CC      =$(TOOL_PATH)/arm-none-eabi-gcc
LD      =$(TOOL_PATH)/arm-none-eabi-ld
OBJCOPY =$(TOOL_PATH)/arm-none-eabi-objcopy
SIZE    =$(TOOL_PATH)/arm-none-eabi-size
GDB     =$(TOOL_PATH)/arm-none-eabi-gdb
OBJDUMP =$(TOOL_PATH)/arm-none-eabi-objdump

FLASH_START=0x8000000

C_FLAGS  = -g -O2 -Wall
C_FLAGS += -mlittle-endian -mthumb -mthumb-interwork
C_FLAGS += -mcpu=cortex-m4
C_FLAGS += -mfloat-abi=hard -mfpu=fpv4-sp-d16
C_FLAGS += -I./CMSIS/Device/ST/STM32F4xx/Include/
C_FLAGS += -I./CMSIS/Include/
C_FLAGS += -I.
C_FLAGS += -DSTM32F40_41xxx
C_FLAGS += -DSYSCLK_MHZ=168
#C_FLAGS += -DMACRO_REGS
#C_FLAGS += -save-temps
#C_FLAGS += -Wa,-ahlms=$(<:.c=.lst)			#problem for assembler files

LD_FLAGS = -Tlink.ld
LD_FLAGS += -Wl,-Map=out.map
LD_FLAGS += -lc -lg -lm -lgcc
LD_FLAGS += -nostartfiles
#LD_FLAGS += -nostdlib
#LD_FLAGS += -nodefaultlibs

# QUIET=1
ifdef QUIET
	CMD_PRINT=@
endif

all: version.inc my_first.bin

.PHONY: version.inc
version.inc:
	@ echo "char version_str[] =\"`date +%F-%H-%M-%S`\\\r\\\n\";" > version.inc
	@ touch usart_basic.c

C_SRCS=$(wildcard *.c)
AS_SRCS=$(wildcard *.s)
OBJS=$(C_SRCS:.c=.o)
OBJS+=$(AS_SRCS:.s=.o)

#dac_dma.o: dac_dma.c
#	$(CMD_PRINT) $(CC) -c -o $@ $(C_FLAGS) -Wa,-ahlms=dac_dma.lst $<

startup.o: startup.s
	$(CMD_PRINT) $(CC) -c -o $@ $(C_FLAGS) $<

%.o: %.c
	$(CMD_PRINT) $(CC) -c -o $@ $(C_FLAGS) $<

my_first.elf: $(OBJS)
	$(CMD_PRINT) $(CC) $^ $(LD_FLAGS) -o $@

my_first.bin: my_first.elf
	$(CMD_PRINT) $(OBJCOPY) -O binary $< $@

flash: my_first.bin
	$(STLINK)/st-flash write $< $(FLASH_START)

size: my_first.elf
	$(SIZE) --format=SysV $<

dis: my_first.elf
	$(OBJDUMP) -D $<

dis_bin: my_first.bin
	$(OBJDUMP) -D -b binary -m arm $<


todo:
	@ find -name "*.c" -exec grep -Hn "TODO" {} \;
	@ find -name "*.h" -exec grep -Hn "TODO" {} \;
	@ find -name "*.s" -exec grep -Hn "TODO" {} \;

info:
	$(CC) -v
	$(CC) -print-multi-lib

clean:
	$(CMD_PRINT) rm -f *.o *.elf *.hex *.bin *.map *.lst version.inc reg_gen 


