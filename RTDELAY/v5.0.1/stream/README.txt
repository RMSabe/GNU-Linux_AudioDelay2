Real-Time Audio Delay 2 application for GNU-Linux systems.
Version 5.0.1 (Audio Stream Version)

This is a real time audio delay, similar to the other real time audio delay project, but this one works using different logic.
It has 4 parallel feedforward delays and 4 parallel feedback delays, along with the dry signal.
User may set delay time (in number of samples) and amplitude (floating point) for any of the delays.

WARNING: Keep in mind that feedback delays are IIR (infinite impulse response) systems. Wrong settings can cause it to clip the audio signal endlessly,
causing a very loud and unpleasant sound. Be careful when setting the feedback delays. I am NOT responsible for hearing loss or damaged speakers!

INPUT: audio input device for capture.

OUTPUT: audio output device for playback.

HINT: The application will ask for the playback device ID. If you're using the system's default audio output, avoid using the ID "default". Use the device ID instead "plughw<x>:<y>" (x = card | y = device).
For some reason, when the ID "default" is used, the system adds a huge amount of latency between the application and the audio device.

NOTICE:
This code supports streaming in 16 bit and 24 bit formats. Sample rate and number of channels depends on hardware compatibility.
However, some devices might not work well with both formats. I noticed some stream jamming when testing 24 bit on a specific audio device.
Also, the current buffer size settings might not be adequate for every use case. A few key settings to consider is the audio buffer size (AUDIOBUFFER_SIZE_FRAMES), the stream buffer segment size (STREAMBUFFER_SEGMENT_SIZE_FRAMES)
and the number of segments in the stream buffer (STREAMBUFFER_N_SEGMENTS). Usually, setting the AUDIOBUFFER_SIZE_FRAMES to about 1 second (AUDIOBUFFER_SIZE_FRAMES = approx SAMPLE_RATE) works well with most devices.
The stream buffer segment size and the number of segments defines how many times per second the audio device will be updated, and how far behind the playback is compared to the capture.
If those values are too big, you will experience more latency between input and output.
If those values are too small, you will likely experience jamming/interruption in the audio stream.
Try to find values that work well with your system.

Use "arecord -l" to list audio capture device IDs.
Use "aplay -l" to list audio playback device IDs.

Compilation notes:
This project requires the ALSA APIs. Install package libasound2-dev.
2 APIs must be explicitly linked: -lpthread and -lasound

Latest Update:
Code Optimization

Author: Rafael Sabe
Email: rafaelmsabe@gmail.com

