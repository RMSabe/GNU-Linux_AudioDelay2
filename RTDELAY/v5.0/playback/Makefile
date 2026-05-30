lib_res: globldef.c delay.c cstrdef.c strdef.cpp cppthread.cpp
	g++ globldef.c -c -o globldef.o
	g++ delay.c -c -o delay.o
	g++ cstrdef.c -c -o cstrdef.o
	g++ strdef.cpp -c -o strdef.o
	g++ cppthread.cpp -c -o cppthread.o

audio_res:
	g++ AudioDelay.cpp -c -o AudioDelay.o
	g++ AudioPB.cpp -c -o AudioPB.o
	g++ AudioPB_i16.cpp -c -o AudioPB_i16.o
	g++ AudioPB_i24.cpp -c -o AudioPB_i24.o

main.o: main.cpp
	g++ main.cpp -c -o main.o

rtdelay.elf: main.o audio_res lib_res
	g++ *.o -lpthread -lasound -o rtdelay.elf

build: rtdelay.elf

cleanup:
	rm *.o

all: build cleanup


