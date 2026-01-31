/*
 * Real-Time Audio Delay 2 for GNU-Linux systems.
 * Version 3.1.1 (Interop Java version 1.0)
 *
 * Author: Rafael Sabe
 * Email: rafaelmsabe@gmail.com
 */

import java.awt.event.ComponentEvent;
import java.awt.event.ComponentAdapter;
import java.awt.*;
import javax.swing.*;

public abstract class MyScreen extends JPanel
{
	protected JFrame parent = null;
	protected MyScreenLayout layout = new MyScreenLayout();

	protected ComponentAdapter componentAdapter = new ComponentAdapter() {

		@Override
		public void componentResized(ComponentEvent event)
		{
			align();
		}

	};

	public void init()
	{
		this.setLayout(this.layout);
		this.setBackground(Definitions.MYSCREEN_BACKGROUNDCOLOR);

		this.parent.addComponentListener(this.componentAdapter);
	}

	public void deinit()
	{
		this.parent.removeComponentListener(this.componentAdapter);
	}

	public abstract void align();
}

