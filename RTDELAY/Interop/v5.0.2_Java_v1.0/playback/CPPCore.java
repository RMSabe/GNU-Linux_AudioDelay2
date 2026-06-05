/*
 * Real-Time Audio Delay 2 for GNU-Linux systems.
 * Version 5.0.2 (File Playback Version. Interop Java version 1.0)
 *
 * Author: Rafael Sabe
 * Email: rafaelmsabe@gmail.com
 */

public class CPPCore
{
	static { System.loadLibrary("core"); }

	public static final int STATUS_ERROR_INVALIDPARAMS = -5;
	public static final int STATUS_ERROR_MEMORY = -4;
	public static final int STATUS_ERROR_AUDIOHW = -3;
	public static final int STATUS_ERROR_NOFILE = -2;
	public static final int STATUS_ERROR_GENERIC = -1;
	public static final int STATUS_UNINITIALIZED = 0;
	public static final int STATUS_READY = 1;
	public static final int STATUS_RUNNING = 2;
	public static final int STATUS_PAUSED = 3;
	public static final int STATUS_STOPPED = 4;

	public static final int AUDIO_STREAMBUFFER_N_SEGMENTS = 2;
	public static final int AUDIO_DELAY_BUFFER_SIZE_FRAMES = 65536;
	public static final int AUDIO_DELAY_N_FFCH = 4;
	public static final int AUDIO_DELAY_N_FBCH = 4;

	public static native boolean initialize();
	public static native void deinitialize();
	public static native String getLastErrorMessage();
	public static native boolean setFileInDirectory(String fileDirectory);
	public static native boolean loadFile_createAudioObject();
	public static native boolean loadAudioDeviceList();
	public static native int getAudioDeviceListEntryCount();
	public static native String getAudioDeviceListEntry_friendlyName(int index);
	public static native boolean chooseDevice(int index, boolean enableResampling);
	public static native boolean chooseDefaultDevice(boolean enableResampling);
	public static native boolean initializeAudioObject();
	public static native int getStatus();
	public static native boolean runPlayback();
	public static native void pausePlayback();
	public static native void resumePlayback();
	public static native void stopPlayback();
	public static native long getAudioDataSizeFrames();
	public static native long getAudioDataPositionFrames();
	public static native boolean setAudioDataPositionFrames(long position);
	public static native float getDryInputAmplitude();
	public static native boolean setDryInputAmplitude(float amp);
	public static native float getOutputAmplitude();
	public static native boolean setOutputAmplitude(float amp);
	public static native int getFFDelay(int nFX);
	public static native boolean setFFDelay(int nFX, int delay);
	public static native float getFFAmplitude(int nFX);
	public static native boolean setFFAmplitude(int nFX, float amp);
	public static native int getFBDelay(int nFX);
	public static native boolean setFBDelay(int nFX, int delay);
	public static native float getFBAmplitude(int nFX);
	public static native boolean setFBAmplitude(int nFX, float amp);
	public static native boolean resetFFParams();
	public static native boolean resetFBParams();

	public static String formatSystemText(String text, String newLineReplacement, boolean replaceNewLine)
	{
		char[] _input = null;
		String _output = "";
		String _newlinestr = "";
		int nChar;

		_input = text.toCharArray();

		if(replaceNewLine) _newlinestr = newLineReplacement;
		else _newlinestr = System.lineSeparator();

		nChar = 0;
		while(nChar < _input.length)
		{
			switch(_input[nChar])
			{
				case '\r':
					if((nChar + 1) < _input.length)
						if(_input[nChar + 1] == '\n')
							nChar++;

				case '\n':
					_output += _newlinestr;
					break;

				default:
					_output += _input[nChar];
					break;
			}

			nChar++;
		}

		return _output;
	}

	public static String formatSystemText(String text)
	{
		return formatSystemText(text, "", false);
	}
}

