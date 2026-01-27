Real-Time Audio Delay 2 application for GNU-Linux systems.
Version 3.1

This is a real time audio delay, similar to the other real time audio delay project, but this one works using different logic.
It has 4 parallel feedforward delays and 4 parallel feedback delays, along with the dry signal.
User may set delay time (in number of samples) and amplitude (floating point) for any of the delays.

WARNING: Keep in mind that feedback delays are IIR (infinite impulse response) systems. Wrong settings can cause it to clip the audio signal endlessly,
causing a very loud and unpleasant sound. Be careful when setting the feedback delays. I am NOT responsible for hearing loss or damaged speakers!

INPUT: WAVE ".wav" audio files, 16bit or 24bit PCM encoding. Number of channels and sample rate depends on your audio hardware.

OUTPUT: audio output device for playback.

HINT: The application will ask for the playback device ID. If you're using the system's default audio output, avoid using the ID "default". Use the device ID instead "plughw<x>:<y>" (x = card | y = device).
For some reason, when the ID "default" is used, the system adds a huge amount of latency between the application and the audio device.

Use "aplay -l" to list audio playback device IDs.

Compilation notes:
This project requires the ALSA APIs. Install package libasound2-dev.
2 APIs must be explicitly linked: -lpthread and -lasound

Latest Update:
Some bug fixes.
Code Optimizations.

Author: Rafael Sabe
Email: rafaelmsabe@gmail.com

