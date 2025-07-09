#!/bin/bash

g++ globldef.c -c -o globldef.o
g++ delay.c -c -o delay.o
g++ cstrdef.c -c -o cstrdef.o
g++ strdef.cpp -c -o strdef.o
g++ cppthread.cpp -c -o cppthread.o
g++ main.cpp -c -o main.o
g++ AudioRTDelay.cpp -c -o AudioRTDelay.o
g++ AudioRTDelay_i16.cpp -c -o AudioRTDelay_i16.o
g++ AudioRTDelay_i24.cpp -c -o AudioRTDelay_i24.o

g++ *.o -lpthread -lasound -o rtdelay.elf

rm *.o

