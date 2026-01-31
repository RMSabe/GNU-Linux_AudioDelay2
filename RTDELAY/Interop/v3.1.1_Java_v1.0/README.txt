Real-Time Audio Delay 2 application for GNU-Linux systems.
Version 3.1.1 (Interop C++ Java version 1.0)

This is basically a variation of the Real-Time Audio Delay 2 version 3.1

In this project, I tried integrating the Graphic UI resources from Java Runtime with the existing C++ codebase.
The C++ source code is almost the same as the regular version 3.1, but modified for integrating with Java Runtime.

Graphic UI was implemented in Java, everything else (file manipulation, audio device control, signal processing) is still implemented in C++.

THIS IS AN EXPERIMENTAL PROJECT. The project still needs quite some improvement done.

WARNING: Keep in mind that feedback delays are IIR (infinite impulse response) systems. Wrong settings can cause it to clip the audio signal endlessly,
causing a very loud and unpleasant sound. Be careful when setting the feedback delays. I am NOT responsible for hearing loss or damaged speakers!

INPUT: WAVE ".wav" audio files, 16bit or 24bit PCM encoding. Number of channels and sample rate depends on your audio hardware.

OUTPUT: audio output device for playback.

HINT: The application will ask for the playback device ID. If you're using the system's default audio output, avoid using the ID "default". Use the device ID instead "plughw<x>:<y>" (x = card | y = device).
For some reason, when the ID "default" is used, the system adds a huge amount of latency between the application and the audio device.

Use "aplay -l" to list audio playback device IDs.

Compilation notes:

Environment variables:
JDK_PATH: this path variable must be set to your Java Development Kit folder.
The makefile and/or shell scripts will use this path variable for compiling and running the code.

Either Makefile or the shell scripts may be used for building and running the project:
buildso : compile C++ code, build the shared object ".so" binary file (required for running the application).
buildj : compile the java code, generate the bytecode ".class" files (required for running the application).
run : run the application.

This project requires the ALSA APIs. Install package libasound2-dev.
2 APIs must be explicitly linked: -lpthread and -lasound

Author: Rafael Sabe
Email: rafaelmsabe@gmail.com

