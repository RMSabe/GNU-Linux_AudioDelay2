buildall:
	g++ src/globldef.c -c -o bin/globldef.o
	g++ src/delay.c -c -o bin/delay.o
	g++ src/cstrdef.c -c -o bin/cstrdef.o
	g++ src/strdef.cpp -c -o bin/strdef.o
	g++ src/cppthread.cpp -c -o bin/cppthread.o
	g++ src/main.cpp -c -o bin/main.o
	g++ src/AudioDelay.cpp -c -o bin/AudioDelay.o
	g++ src/AudioStream.cpp -c -o bin/AudioStream.o
	g++ src/AudioStream_i16.cpp -c -o bin/AudioStream_i16.o
	g++ src/AudioStream_i24.cpp -c -o bin/AudioStream_i24.o
	g++ bin/*.o -lpthread -lasound -o delay.elf

clearall:
	rm bin/*.o
	rm *.elf

