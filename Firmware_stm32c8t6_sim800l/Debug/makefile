################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include Startup/subdir.mk
-include Src/subdir.mk
-include Middlewares/ST/STM32_USB_Device_Library/Core/Src/subdir.mk
-include Middlewares/ST/STM32_USB_Device_Library/Class/CustomHID/Src/subdir.mk
-include Drivers/STM32F1xx_HAL_Driver/Src/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
EXECUTABLES += \
SIM800L_stm32f103c8t6.elf \

SIZE_OUTPUT += \
default.size.stdout \

OBJDUMP_LIST += \
SIM800L_stm32f103c8t6.list \

OBJCOPY_HEX += \
SIM800L_stm32f103c8t6.hex \

OBJCOPY_BIN += \
SIM800L_stm32f103c8t6.bin \


# All Target
all: SIM800L_stm32f103c8t6.elf secondary-outputs

# Tool invocations
SIM800L_stm32f103c8t6.elf: $(OBJS) $(USER_OBJS) D:\GitHub_MyRep\SIM800L_STM32\Firmware_stm32c8t6_sim800l\STM32F103C8TX_FLASH.ld
	arm-none-eabi-gcc -o "SIM800L_stm32f103c8t6.elf" @"objects.list" $(USER_OBJS) $(LIBS) -mcpu=cortex-m3 -T"D:\GitHub_MyRep\SIM800L_STM32\Firmware_stm32c8t6_sim800l\STM32F103C8TX_FLASH.ld" --specs=nosys.specs -Wl,-Map="SIM800L_stm32f103c8t6.map" -Wl,--gc-sections -static --specs=nano.specs -mfloat-abi=soft -mthumb -Wl,--start-group -lc -lm -Wl,--end-group
	@echo 'Finished building target: $@'
	@echo ' '

default.size.stdout: $(EXECUTABLES)
	arm-none-eabi-size  $(EXECUTABLES)
	@echo 'Finished building: $@'
	@echo ' '

SIM800L_stm32f103c8t6.list: $(EXECUTABLES)
	arm-none-eabi-objdump -h -S $(EXECUTABLES) > "SIM800L_stm32f103c8t6.list"
	@echo 'Finished building: $@'
	@echo ' '

SIM800L_stm32f103c8t6.hex: $(EXECUTABLES)
	arm-none-eabi-objcopy  -O ihex $(EXECUTABLES) "SIM800L_stm32f103c8t6.hex"
	@echo 'Finished building: $@'
	@echo ' '

SIM800L_stm32f103c8t6.bin: $(EXECUTABLES)
	arm-none-eabi-objcopy  -O binary $(EXECUTABLES) "SIM800L_stm32f103c8t6.bin"
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) *
	-@echo ' '

secondary-outputs: $(SIZE_OUTPUT) $(OBJDUMP_LIST) $(OBJCOPY_HEX) $(OBJCOPY_BIN)

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets
