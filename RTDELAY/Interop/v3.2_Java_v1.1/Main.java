/*
 * Real-Time Audio Delay 2 for GNU-Linux systems.
 * Version 3.2 (Interop Java version 1.1)
 *
 * Author: Rafael Sabe
 * Email: rafaelmsabe@gmail.com
 */

import java.io.File;
import java.awt.event.WindowEvent;
import java.awt.event.WindowAdapter;
import javax.swing.JFrame;
import javax.swing.JOptionPane;

public class Main
{
	public static JFrame mainwnd = null;
	public static MyScreen screen = null;
	public static AudioThread audiothread = null;
	public static File fileobj = null;

	public static WindowAdapter windowAdapter = new WindowAdapter() {
		@Override
		public void windowClosed(WindowEvent event)
		{
			appDeinit();
		}
	};

	public static void main(String[] args)
	{
		if(!appInit()) return;

		screen = new ChooseFileScreen(mainwnd);
		mainwnd.add(screen);
		mainwnd.setVisible(true);
	}

	public static boolean appInit()
	{
		if(!CPPCore.initialize())
		{
			JOptionPane.showMessageDialog(new JFrame(), CPPCore.getLastErrorMessage(), "INIT ERROR", JOptionPane.ERROR_MESSAGE);
			System.out.println(CPPCore.getLastErrorMessage());
			return false;
		}

		mainwnd = new JFrame();
		mainwnd.setTitle(Definitions.MAINWND_CAPTION);
		mainwnd.setSize(Definitions.MAINWND_INITSIZE);
		mainwnd.addWindowListener(windowAdapter);
		mainwnd.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);

		return true;
	}

	public static void appDeinit()
	{
		CPPCore.deinitialize();
	}

	public static void appExit(int exitCode)
	{
		appDeinit();

		JOptionPane.showMessageDialog(new JFrame(), CPPCore.getLastErrorMessage(), "PROCESS EXIT CALLED", JOptionPane.ERROR_MESSAGE);
		System.exit(exitCode);
	}

	public static void procLoadFileCreateAudioObject()
	{
		if(!CPPCore.loadFile_createAudioObject())
		{
			JOptionPane.showMessageDialog(new JFrame(), CPPCore.getLastErrorMessage(), "ERROR", JOptionPane.ERROR_MESSAGE);
			return;
		}

		switchToChooseAudioDeviceScreen();
	}

	public static void procInitAudioObject()
	{
		if(!CPPCore.initializeAudioObject())
		{
			JOptionPane.showMessageDialog(new JFrame(), CPPCore.getLastErrorMessage(), "ERROR", JOptionPane.ERROR_MESSAGE);
			return;
		}

		audiothread = new AudioThread();
		audiothread.start();

		switchToPlaybackRunningScreen();
	}

	public static void switchToChooseFileScreen()
	{
		screen.deinit();
		mainwnd.remove(screen);
		screen = new ChooseFileScreen(mainwnd);
		mainwnd.add(screen);
		mainwnd.revalidate();
	}

	public static void switchToChooseAudioDeviceScreen()
	{
		screen.deinit();
		mainwnd.remove(screen);
		screen = new ChooseAudioDeviceScreen(mainwnd);
		mainwnd.add(screen);
		mainwnd.revalidate();
	}

	public static void switchToPlaybackRunningScreen()
	{
		screen.deinit();
		mainwnd.remove(screen);
		screen = new PlaybackRunningScreen(mainwnd);
		mainwnd.add(screen);
		mainwnd.revalidate();
	}

	public static void switchToPlaybackFinishedScreen()
	{
		screen.deinit();
		mainwnd.remove(screen);
		screen = new PlaybackFinishedScreen(mainwnd);
		mainwnd.add(screen);
		mainwnd.revalidate();
	}

	public static void audiothreadProc()
	{
		CPPCore.runPlayback();
		switchToPlaybackFinishedScreen();
	}
}

