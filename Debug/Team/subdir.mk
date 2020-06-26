################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Team/appeared_pokemon.c \
../Team/auxiliar.c \
../Team/entrenador.c \
../Team/planificado.c \
../Team/posicion.c \
../Team/team.c \
../Team/utils.c 

OBJS += \
./Team/appeared_pokemon.o \
./Team/auxiliar.o \
./Team/entrenador.o \
./Team/planificado.o \
./Team/posicion.o \
./Team/team.o \
./Team/utils.o 

C_DEPS += \
./Team/appeared_pokemon.d \
./Team/auxiliar.d \
./Team/entrenador.d \
./Team/planificado.d \
./Team/posicion.d \
./Team/team.d \
./Team/utils.d 


# Each subdirectory must supply rules for building sources it contributes
Team/%.o: ../Team/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


