#!/bin/bash

${JDK_PATH}/bin/javac -h cppsrc CPPCore.java
${JDK_PATH}/bin/javac Definitions.java MyScreenLayout.java MyScreen.java Main.java AudioThread.java ChooseFileScreen.java ChooseAudioDeviceScreen.java PlaybackRunningScreen.java PlaybackFinishedScreen.java

