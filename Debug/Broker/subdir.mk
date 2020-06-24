################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Broker/broker.c \
../Broker/collections.c \
../Broker/memory.c \
../Broker/utils.c 

OBJS += \
./Broker/broker.o \
./Broker/collections.o \
./Broker/memory.o \
./Broker/utils.o 

C_DEPS += \
./Broker/broker.d \
./Broker/collections.d \
./Broker/memory.d \
./Broker/utils.d 


# Each subdirectory must supply rules for building sources it contributes
Broker/%.o: ../Broker/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


