#!/bin/bash

g++ cppsrc/globldef.c -c -fPIC -o cppobj/globldef.o
g++ cppsrc/delay.c -c -fPIC -o cppobj/delay.o
g++ cppsrc/strdef.cpp -c -fPIC -o cppobj/strdef.o
g++ cppsrc/cppthread.cpp -c -fPIC -o cppobj/cppthread.o

g++ cppsrc/AudioRTDelay.cpp -c -fPIC -o cppobj/AudioRTDelay.o
g++ cppsrc/AudioPB.cpp -c -fPIC -o cppobj/AudioPB.o
g++ cppsrc/AudioPB_i16.cpp -c -fPIC -o cppobj/AudioPB_i16.o
g++ cppsrc/AudioPB_i24.cpp -c -fPIC -o cppobj/AudioPB_i24.o

g++ cppsrc/libcore.cpp -c -fPIC -o cppobj/libcore.o
g++ cppsrc/libjni.cpp -c -fPIC -I${JDK_PATH}/include -I${JDK_PATH}/include/linux -o cppobj/libjni.o

g++ cppobj/libjni.o cppobj/libcore.o cppobj/globldef.o cppobj/delay.o cppobj/strdef.o cppobj/cppthread.o cppobj/AudioRTDelay.o cppobj/AudioPB.o cppobj/AudioPB_i16.o cppobj/AudioPB_i24.o -lpthread -lasound -shared -o libcore.so

