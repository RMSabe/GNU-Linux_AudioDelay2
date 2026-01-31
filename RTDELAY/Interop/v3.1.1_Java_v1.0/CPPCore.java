/*
 * Real-Time Audio Delay 2 for GNU-Linux systems.
 * Version 3.1.1 (Interop Java version 1.0)
 *
 * Author: Rafael Sabe
 * Email: rafaelmsabe@gmail.com
 */

public class CPPCore
{
	static
	{
		System.loadLibrary("core");
	}

	public static final int RTDELAY_N_FF_DELAYS = 4;
	public static final int RTDELAY_N_FB_DELAYS = 4;

	public static native boolean init();
	public static native void deinit();

	public static native String getLastErrorMessage();
	public static native boolean setFileInDir(String fileDir);
	public static native boolean setAudioDevDesc(String devDesc);

	public static native boolean playbackInit();
	public static native boolean playbackRun();

	public static native void stopPlayback();

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
}

