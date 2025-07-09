globldef.o: globldef.c
	g++ globldef.c -c -o globldef.o

delay.o: delay.c
	g++ delay.c -c -o delay.o

cstrdef.o: cstrdef.c
	g++ cstrdef.c -c -o cstrdef.o

strdef.o: strdef.cpp
	g++ strdef.cpp -c -o strdef.o

cppthread.o: cppthread.cpp
	g++ cppthread.cpp -c -o cppthread.o

lib_res: globldef.o delay.o cstrdef.o strdef.o cppthread.o

AudioRTDelay.o: AudioRTDelay.cpp
	g++ AudioRTDelay.cpp -c -o AudioRTDelay.o

AudioRTDelay_i16.o: AudioRTDelay_i16.cpp
	g++ AudioRTDelay_i16.cpp -c -o AudioRTDelay_i16.o

AudioRTDelay_i24.o: AudioRTDelay_i24.cpp
	g++ AudioRTDelay_i24.cpp -c -o AudioRTDelay_i24.o

audio_rtdelay: AudioRTDelay.o AudioRTDelay_i16.o AudioRTDelay_i24.o

main.o: main.cpp
	g++ main.cpp -c -o main.o

rtdelay.elf: main.o audio_rtdelay lib_res
	g++ *.o -lpthread -lasound -o rtdelay.elf

cleanup:
	rm *.o

all: rtdelay.elf cleanup


