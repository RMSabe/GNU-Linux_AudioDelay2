Real-Time Audio Delay 2 application for GNU-Linux systems.
Version 5.0.1 Audio Stream Version (Interop C++ Java version 1.0)

This is basically a variation of the Real-Time Audio Delay 2 version 5.0.1 Audio Stream Version

In this project, I tried integrating the Graphic UI resources from Java Runtime with the existing C++ codebase.
The C++ source code is almost the same as the regular version, but modified for integrating with Java Runtime.

Graphic UI was implemented in Java, everything else (file manipulation, audio device control, signal processing) is still implemented in C++.

THIS IS AN EXPERIMENTAL PROJECT. The project still needs quite some improvement done.

WARNING: Keep in mind that feedback delays are IIR (infinite impulse response) systems. Wrong settings can cause it to clip the audio signal endlessly,
causing a very loud and unpleasant sound. Be careful when setting the feedback delays. I am NOT responsible for hearing loss or damaged speakers!

INPUT: audio input device for capture.

OUTPUT: audio output device for playback.

HINT: For better performance, it is recommended that you choose the playback audio device directly from the audio device list instead of clicking "choose default device".

NOTICE:
This code supports streaming in 16 bit and 24 bit formats. Sample rate and number of channels depends on hardware compatibility.
However, some devices might not work well with both formats. I noticed some stream jamming when testing 24 bit on a specific audio device.
Also, the current buffer size settings might not be adequate for every use case. A few key settings to consider is the audio buffer size (AUDIOBUFFER_SIZE_FRAMES), the stream buffer segment size (STREAMBUFFER_SEGMENT_SIZE_FRAMES)
and the number of segments in the stream buffer (STREAMBUFFER_N_SEGMENTS). Usually, setting the AUDIOBUFFER_SIZE_FRAMES to about 1 second (AUDIOBUFFER_SIZE_FRAMES = approx SAMPLE_RATE) works well with most devices.
The stream buffer segment size and the number of segments defines how many times per second the audio device will be updated, and how far behind the playback is compared to the capture.
If those values are too big, you will experience more latency between input and output.
If those values are too small, you will likely experience jamming/interruption in the audio stream.
Try to find values that work well with your system.

Compilation notes:

Environment variables:
JDK_PATH: this path variable must be set to your Java Development Kit folder.
The makefile and/or shell scripts will use this path variable for compiling and running the code.

Either Makefile or the shell scripts may be used for building and running the project:
buildso : compile C++ code, build the shared object ".so" binary file (required for running the application).
buildj : compile the java code, generate the bytecode ".class" files (required for running the application).
run : run the application.

Both build.sh and Makefile will create the C++ binary object files in a folder called "cppobj". Make sure to create this folder if it doesn't exist.

This project requires the ALSA APIs. Install package libasound2-dev.
2 APIs must be explicitly linked: -lpthread and -lasound

Latest Update:
Some bug fixes
Code Optimization
New features

Author: Rafael Sabe
Email: rafaelmsabe@gmail.com

