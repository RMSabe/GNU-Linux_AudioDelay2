/*
 * Real-Time Audio Delay 2 for GNU-Linux systems.
 * Version 5.0.1 (Audio Stream Version. Interop Java version 1.0)
 *
 * Author: Rafael Sabe
 * Email: rafaelmsabe@gmail.com
 */

import java.awt.event.WindowEvent;
import java.awt.event.WindowAdapter;
import javax.swing.JFrame;
import javax.swing.JOptionPane;

public class Main
{
	private static class AudioThread extends Thread {
		@Override
		public void run() { Main.audiothreadProc(); }
	};

	public static JFrame mainwnd = null;
	public static MyScreen screen = null;
	public static AudioThread audiothread = null;

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

		screen = new AudioSetupScreen(mainwnd);
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

	public static void procCreateAudioObject()
	{
		if(!CPPCore.createAudioObject())
		{
			JOptionPane.showMessageDialog(new JFrame(), CPPCore.getLastErrorMessage(), "ERROR", JOptionPane.ERROR_MESSAGE);
			return;
		}

		switchToChooseAudioDeviceScreen(false);
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

		switchToStreamRunningScreen();
	}

	public static void switchToAudioSetupScreen()
	{
		screen.deinit();
		mainwnd.remove(screen);
		screen = new AudioSetupScreen(mainwnd);
		mainwnd.add(screen);
		mainwnd.revalidate();
	}

	public static void switchToChooseAudioDeviceScreen(boolean outputDevice)
	{
		screen.deinit();
		mainwnd.remove(screen);
		screen = new ChooseAudioDeviceScreen(mainwnd, outputDevice);
		mainwnd.add(screen);
		mainwnd.revalidate();
	}

	public static void switchToStreamRunningScreen()
	{
		screen.deinit();
		mainwnd.remove(screen);
		screen = new StreamRunningScreen(mainwnd);
		mainwnd.add(screen);
		mainwnd.revalidate();
	}

	public static void switchToStreamFinishedScreen()
	{
		screen.deinit();
		mainwnd.remove(screen);
		screen = new StreamFinishedScreen(mainwnd);
		mainwnd.add(screen);
		mainwnd.revalidate();
	}

	public static void audiothreadProc()
	{
		CPPCore.runStream();
		switchToStreamFinishedScreen();
	}
}

