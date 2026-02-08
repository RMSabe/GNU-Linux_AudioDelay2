/*
 * Real-Time Audio Delay 2 for GNU-Linux systems.
 * Version 3.1.1 (Interop Java version 1.1)
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

	/*pProcCtx: pointer to the C++ Process Context Object.*/
	private static long pProcCtx = 0L;

	private static String errMsg = "";

	private static native long _procCtxAlloc();
	private static native boolean _procCtxFree();

	private static native boolean _init();
	private static native void _deinit();

	private static native String _getLastErrorMessage();
	private static native boolean _setFileInDir(String fileDir);
	private static native boolean _setAudioDevDesc(String devDesc);

	private static native boolean _playbackInit();
	private static native boolean _playbackRun();

	private static native void _stopPlayback();

	private static native float _getDryInputAmplitude();
	private static native boolean _setDryInputAmplitude(float amp);
	private static native float _getOutputAmplitude();
	private static native boolean _setOutputAmplitude(float amp);

	private static native int _getFFDelay(int nFX);
	private static native boolean _setFFDelay(int nFX, int delay);
	private static native float _getFFAmplitude(int nFX);
	private static native boolean _setFFAmplitude(int nFX, float amp);

	private static native int _getFBDelay(int nFX);
	private static native boolean _setFBDelay(int nFX, int delay);
	private static native float _getFBAmplitude(int nFX);
	private static native boolean _setFBAmplitude(int nFX, float amp);

	private static native boolean _resetFFParams();
	private static native boolean _resetFBParams();

	private static boolean validateProcCtx()
	{
		if(pProcCtx == 0L)
		{
			errMsg = "CPPCore: Error: resource is not initialized.";
			return false;
		}

		return true;
	}

	public static boolean init()
	{
		if(pProcCtx != 0L) deinit();

		pProcCtx = _procCtxAlloc();
		if(pProcCtx == 0L)
		{
			errMsg = "CPPCore.init: Error: failed to allocate process context object.";
			return false;
		}

		if(!_init())
		{
			errMsg = _getLastErrorMessage();
			deinit();
			return false;
		}

		return true;
	}

	public static void deinit()
	{
		if(pProcCtx == 0L) return;

		_deinit();
		_procCtxFree();
		pProcCtx = 0L;
	}

	public static String getLastErrorMessage()
	{
		return errMsg;
	}

	public static boolean setFileInDir(String fileDir)
	{
		if(!validateProcCtx()) return false;

		if(!_setFileInDir(fileDir))
		{
			errMsg = _getLastErrorMessage();
			return false;
		}

		return true;
	}

	public static boolean setAudioDevDesc(String devDesc)
	{
		if(!validateProcCtx()) return false;

		if(!_setAudioDevDesc(devDesc))
		{
			errMsg = _getLastErrorMessage();
			return false;
		}

		return true;
	}

	public static boolean playbackInit()
	{
		if(!validateProcCtx()) return false;

		if(!_playbackInit())
		{
			errMsg = _getLastErrorMessage();
			return false;
		}

		return true;
	}

	public static boolean playbackRun()
	{
		if(!validateProcCtx()) return false;

		if(!_playbackRun())
		{
			errMsg = _getLastErrorMessage();
			return false;
		}

		return true;
	}

	public static void stopPlayback()
	{
		_stopPlayback();
	}

	public static float getDryInputAmplitude()
	{
		return _getDryInputAmplitude();
	}

	public static boolean setDryInputAmplitude(float amp)
	{
		if(!validateProcCtx()) return false;

		if(!_setDryInputAmplitude(amp))
		{
			errMsg = _getLastErrorMessage();
			return false;
		}

		return true;
	}

	public static float getOutputAmplitude()
	{
		return _getOutputAmplitude();
	}

	public static boolean setOutputAmplitude(float amp)
	{
		if(!validateProcCtx()) return false;

		if(!_setOutputAmplitude(amp))
		{
			errMsg = _getLastErrorMessage();
			return false;
		}

		return true;
	}

	public static int getFFDelay(int nFX)
	{
		int delay;

		if(!validateProcCtx()) return -1;

		delay = _getFFDelay(nFX);
		if(delay < 0) errMsg = _getLastErrorMessage();

		return delay;
	}

	public static boolean setFFDelay(int nFX, int delay)
	{
		if(!validateProcCtx()) return false;

		if(!_setFFDelay(nFX, delay))
		{
			errMsg = _getLastErrorMessage();
			return false;
		}

		return true;
	}

	public static float getFFAmplitude(int nFX)
	{
		return _getFFAmplitude(nFX);
	}

	public static boolean setFFAmplitude(int nFX, float amp)
	{
		if(!validateProcCtx()) return false;

		if(!_setFFAmplitude(nFX, amp))
		{
			errMsg = _getLastErrorMessage();
			return false;
		}

		return true;
	}

	public static int getFBDelay(int nFX)
	{
		int delay;

		if(!validateProcCtx()) return -1;

		delay = _getFBDelay(nFX);
		if(delay < 0) errMsg = _getLastErrorMessage();

		return delay;
	}

	public static boolean setFBDelay(int nFX, int delay)
	{
		if(!validateProcCtx()) return false;

		if(!_setFBDelay(nFX, delay))
		{
			errMsg = _getLastErrorMessage();
			return false;
		}

		return true;
	}

	public static float getFBAmplitude(int nFX)
	{
		return _getFBAmplitude(nFX);
	}

	public static boolean setFBAmplitude(int nFX, float amp)
	{
		if(!validateProcCtx()) return false;

		if(!_setFBAmplitude(nFX, amp))
		{
			errMsg = _getLastErrorMessage();
			return false;
		}

		return true;
	}

	public static boolean resetFFParams()
	{
		if(!validateProcCtx()) return false;

		if(!_resetFFParams())
		{
			errMsg = _getLastErrorMessage();
			return false;
		}

		return true;
	}

	public static boolean resetFBParams()
	{
		if(!validateProcCtx()) return false;

		if(!_resetFBParams())
		{
			errMsg = _getLastErrorMessage();
			return false;
		}

		return true;
	}
}

