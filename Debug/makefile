################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include Team/Debug/subdir.mk
-include Team/subdir.mk
-include GameCard/Debug/subdir.mk
-include GameCard/subdir.mk
-include GameBoy/Debug/subdir.mk
-include GameBoy/subdir.mk
-include Broker/Debug/subdir.mk
-include Broker/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: tp-2020-1c-CoronaLinux

# Tool invocations
tp-2020-1c-CoronaLinux: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: GCC C Linker'
	gcc  -o "tp-2020-1c-CoronaLinux" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(EXECUTABLES)$(OBJS)$(C_DEPS) tp-2020-1c-CoronaLinux
	-@echo ' '

.PHONY: all clean dependents

-include ../makefile.targets
