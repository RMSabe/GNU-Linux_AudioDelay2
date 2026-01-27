/*
 * Real-Time Audio Delay 2 for GNU-Linux systems.
 * Version 3.1 (Interop Java version 1.0)
 *
 * Author: Rafael Sabe
 * Email: rafaelmsabe@gmail.com
 */

public class AudioThread extends Thread
{
	@Override
	public void run()
	{
		Main.audiothreadProc();
	}
}

