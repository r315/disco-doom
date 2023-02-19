
TARGET =disco-doom
TARGET_DIR =disco
######################################
# building variables
######################################
# debug build?
DEBUG = 1

ifeq ($(DEBUG), 1)
DEBUG_FLAGS =-g -gdwarf-2
OPT =-O0
else
OPT =-O3
endif

#######################################
# paths
#######################################
DOOM_ROOT_PATH =../..
TARGET_PATH =target/$(TARGET_DIR)
BSP_PATH =$(DOOM_ROOT_PATH)/$(TARGET_PATH)
BUILD_DIR =$(DOOM_ROOT_PATH)/build/$(TARGET_PATH)
EXEC =$(BUILD_DIR)/$(TARGET).elf

REPO_F769 =$(BSP_PATH)

BSP_SOURCES_PATHS = \
$(DOOM_ROOT_PATH)/$(TARGET_PATH)/core \
$(REPO_F769)/Drivers/STM32F7xx_HAL_Driver/Src \
$(REPO_F769)/Drivers/BSP/STM32F769I-Discovery \
$(REPO_F769)/Drivers/BSP/Components/otm8009a \
$(REPO_F769)/Drivers/BSP/Components/ft6x06 \
$(REPO_F769)/Drivers/BSP/Components/wm8994 \
$(REPO_F769)/Middlewares/Third_Party/FatFs/src \

DOOM_SOURCES_PATHS =$(DOOM_ROOT_PATH)/src

#######################################
# Includes
#######################################
BSP_INCLUDE_PATHS = \
$(DOOM_ROOT_PATH)/$(TARGET_PATH)/core \
$(REPO_F769)/Drivers/STM32F7xx_HAL_Driver/Inc \
$(REPO_F769)/Drivers/BSP/STM32F769I-Discovery \
$(REPO_F769)/Drivers/BSP/Components/Common \
$(REPO_F769)/Drivers/BSP/Components/otm8009a \
$(REPO_F769)/Drivers/BSP/Components/wm8994 \
$(REPO_F769)/Drivers/CMSIS/Device/ST/STM32F7xx/Include \
$(REPO_F769)/Drivers/CMSIS/Include \
$(REPO_F769)/Middlewares/Third_Party/FatFs/src \

DOOM_INCLUDES_PATHS =$(DOOM_ROOT_PATH)/inc

AS_INCLUDES = 

######################################
# source files
######################################

BSP_C_SOURCES = \
$(BSP_PATH)/core/main_stm32.c \
$(BSP_PATH)/core/fifo.c \
$(BSP_PATH)/core/serial.c \
$(BSP_PATH)/core/it.c \
$(BSP_PATH)/core/hal_msp.c \
$(BSP_PATH)/core/system_stm32f7xx.c \
$(BSP_PATH)/core/sd_diskio.c \
$(BSP_PATH)/core/fatfs.c \
$(BSP_PATH)/core/syscalls.c \
$(BSP_PATH)/core/lis302.c \
$(BSP_PATH)/core/pcf8574.c \
$(BSP_PATH)/core/i2c.c \
$(BSP_PATH)/core/audio.c \
$(BSP_PATH)/core/debug.c \
$(REPO_F769)/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_cortex.c \
$(REPO_F769)/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_dma2d.c \
$(REPO_F769)/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_dsi.c \
$(REPO_F769)/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_ll_fmc.c \
$(REPO_F769)/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_sdram.c \
$(REPO_F769)/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_ltdc.c \
$(REPO_F769)/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_ltdc_ex.c \
$(REPO_F769)/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_qspi.c \
$(REPO_F769)/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_tim.c \
$(REPO_F769)/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_tim_ex.c \
$(REPO_F769)/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_rcc.c \
$(REPO_F769)/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_rcc_ex.c \
$(REPO_F769)/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_flash.c \
$(REPO_F769)/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_flash_ex.c \
$(REPO_F769)/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_gpio.c \
$(REPO_F769)/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_dma.c \
$(REPO_F769)/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_dma_ex.c \
$(REPO_F769)/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_pwr.c \
$(REPO_F769)/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_pwr_ex.c \
$(REPO_F769)/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal.c \
$(REPO_F769)/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_i2c.c \
$(REPO_F769)/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_i2c_ex.c \
$(REPO_F769)/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_exti.c \
$(REPO_F769)/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_uart.c \
$(REPO_F769)/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_sd.c \
$(REPO_F769)/Drivers/STM32F7xx_HAL_Driver/Src//stm32f7xx_hal_sai.c \
$(REPO_F769)/Drivers/STM32F7xx_HAL_Driver/Src//stm32f7xx_ll_sdmmc.c \
$(REPO_F769)/Drivers/BSP/STM32F769I-Discovery/stm32f769i_discovery.c \
$(REPO_F769)/Drivers/BSP/STM32F769I-Discovery/stm32f769i_discovery_lcd.c \
$(REPO_F769)/Drivers/BSP/STM32F769I-Discovery/stm32f769i_discovery_sdram.c \
$(REPO_F769)/Drivers/BSP/STM32F769I-Discovery/stm32f769i_discovery_sd.c \
$(REPO_F769)/Drivers/BSP/STM32F769I-Discovery/stm32f769i_discovery_ts.c \
$(REPO_F769)/Drivers/BSP/Components/otm8009a/otm8009a.c \
$(REPO_F769)/Drivers/BSP/Components/ft6x06/ft6x06.c \
$(REPO_F769)/Drivers/BSP/Components/wm8994/wm8994.c \
$(REPO_F769)/Middlewares/Third_Party/FatFs/src/diskio.c \
$(REPO_F769)/Middlewares/Third_Party/FatFs/src/ff.c \
$(REPO_F769)/Middlewares/Third_Party/FatFs/src/ff_gen_drv.c \

DOOM_SOURCES = \
$(wildcard $(DOOM_SOURCES_PATHS)/*.c) \
$(TARGET_PATH)/i_sound.c \
$(TARGET_PATH)/i_net.c \
$(TARGET_PATH)/i_video.c \
$(TARGET_PATH)/i_input.c \

# ASM sources
ASM_SOURCES = $(BSP_PATH)/core/startup_stm32f769xx.s

# link script
LDSCRIPT = $(BSP_PATH)/core/STM32F769NIHx_FLASH.ld

C_INCLUDES =$(BSP_INCLUDE_PATHS) $(DOOM_INCLUDES_PATHS)
C_SOURCES =$(BSP_C_SOURCES) $(DOOM_SOURCES)
#######################################
# binaries
#######################################
PREFIX = arm-none-eabi-
ifdef GCC_ARM
CC = $(GCC_PATH)/$(PREFIX)gcc
AS = $(GCC_PATH)/$(PREFIX)gcc -x assembler-with-cpp
CP = $(GCC_PATH)/$(PREFIX)objcopy
SZ = $(GCC_PATH)/$(PREFIX)size
else
CC = $(PREFIX)gcc
AS = $(PREFIX)gcc -x assembler-with-cpp
CP = $(PREFIX)objcopy
SZ = $(PREFIX)size
NM = $(PREFIX)nm
endif
HEX = $(CP) -O ihex
BIN = $(CP) -O binary -S
 
#######################################
# FLAGS
#######################################
# cpu
CPU =-mcpu=cortex-m7
# fpu
FPU =-mfpu=fpv5-d16
# float-abi
FLOAT-ABI =-mfloat-abi=hard
# mcu
MCU = $(CPU) -mthumb $(FPU) $(FLOAT-ABI)
# AS defines
AS_DEFS =
# C symbols
C_DEFS =  \
-DUSE_HAL_DRIVER \
-DSTM32F769xx
# C Include paths
C_INC =$(addprefix -I, $(C_INCLUDES))
# compile gcc flags
CFLAGS = $(MCU) $(DEBUG_FLAGS) $(OPT) -std=c11 -Wall -fdata-sections -ffunction-sections -Wno-unused-const-variable -fstack-usage
CFLAGS +=$(C_DEFS) $(C_INC)
ASFLAGS = $(MCU) $(AS_DEFS) $(AS_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections
# Generate dependency information
#CFLAGS += -MMD -MP -MF"$(@:%.o=%.d)"

# libraries
LIBS =#-lc -lm -lnosys
LIBDIR = 
LDFLAGS = $(MCU) -T$(LDSCRIPT) $(LIBDIR) $(LIBS) -Wl,--gc-sections

#######################################
# Rules
#######################################
# default action: build all
all: $(EXEC) #$(BUILD_DIR)/$(TARGET).hex $(BUILD_DIR)/$(TARGET).bin

program: all
	openocd -f "$(BSP_PATH)/core/stm32f769-disco.cfg" -c "program $(EXEC) verify reset exit"

aslist: $(BUILD_DIR)/$(TARGET).elf
	@$(OBJDUMP) -D $< > $(BUILD_DIR)/$(TARGET).lst
test:
	 @echo $(CFLAGS)

bss-usage: $(EXEC)
	$(VERBOSE)$(NM) $< > $(BUILD_DIR)/bss.bu -S --size-sort

stack-usage: $(EXEC)
	@echo "" > $(BUILD_DIR)/stack.su
	@for file in $(BUILD_DIR)/*.su ; do cat $${file} >> $(BUILD_DIR)/stack.su.tmp; done
	@sort $(BUILD_DIR)/stack.su.tmp -n -k2 | column -t > $(BUILD_DIR)/stack.su
	@rm -f $(BUILD_DIR)/stack.su.tmp
#######################################
# Compile objects
#######################################
# list of objects
OBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(C_SOURCES:.c=.o)))
#vpath %.c $(sort $(dir $(C_SOURCES)))
# list of ASM program objects
OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(ASM_SOURCES:.s=.o)))
#vpath %.s $(sort $(dir $(ASM_SOURCES)))
VPATH +=$(DOOM_SOURCES_PATHS) $(BSP_SOURCES_PATHS)

#######################################
# Compile rules
#######################################
$(BUILD_DIR)/%.o: %.c Makefile | $(BUILD_DIR)
	@echo "Compile " $<
	@$(CC) -c $(CFLAGS) "$<" -o "$@"

$(BUILD_DIR)/%.o: %.s Makefile | $(BUILD_DIR)
	@echo "Assemble " $<
	@$(AS) -c $(CFLAGS) "$<" -o "$@"

$(BUILD_DIR)/$(TARGET).elf: $(OBJECTS) Makefile
	@$(CC) $(OBJECTS) $(LDFLAGS) -o "$@"
	@$(SZ) $@ -A

$(BUILD_DIR)/%.hex: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(HEX) "$<" "$@"
	
$(BUILD_DIR)/%.bin: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(BIN) "$<" "$@"	
	
$(BUILD_DIR):
	mkdir -p "$@"		

#######################################
# clean up
#######################################
clean:
	-rm -fR $(BUILD_DIR)
  
#######################################
# misc
#######################################
ifeq ($(GCC_COLORS), )
export GCC_COLORS='error=01;31:warning=01;35:note=01;36:caret=01;32:locus=01:quote=01'
#unexport GCC_COLORS
endif
# *** EOF ***
