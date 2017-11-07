################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
.../src/boundsCalculation.c \
.../src/db.c \
.../src/initialize.c \
.../src/interpolation.c \
.../src/list.c \
.../src/localSearch.c \
.../src/main.c \
.../src/objectiveFunction.c \
.../src/openMP.c \
.../src/predictor.c \
.../src/u_debug.c \
.../src/u_file.c \
.../src/u_misc.c \
.../src/u_string.c 

OBJS += \
../obj/boundsCalculation.o \
../obj/db.o \
../obj/initialize.o \
../obj/interpolation.o \
../obj/list.o \
../obj/localSearch.o \
../obj/main.o \
../obj/objectiveFunction.o \
../obj/openMP.o \
../obj/predictor.o \
../obj/u_debug.o \
../obj/u_file.o \
../obj/u_misc.o \
../obj/u_string.o 

C_DEPS += \
../obj/boundsCalculation.d \
../obj/db.d \
../obj/initialize.d \
../obj/interpolation.d \
../obj/list.d \
../obj/localSearch.d \
../obj/main.d \
../obj/objectiveFunction.d \
../obj/openMP.d \
../obj/predictor.d \
../obj/u_debug.d \
../obj/u_file.d \
../obj/u_misc.d \
../obj/u_string.d 


# Each subdirectory must supply rules for building sources it contributes
../obj/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	mpicc -I/usr/include/mysql -I/usr/local/include -O0 -g -pedantic -Wall -c -fmessage-length=0 -fopenmp -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


