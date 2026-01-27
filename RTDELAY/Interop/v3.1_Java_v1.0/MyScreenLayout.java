/*
 * Real-Time Audio Delay 2 for GNU-Linux systems.
 * Version 3.1 (Interop Java version 1.0)
 *
 * Author: Rafael Sabe
 * Email: rafaelmsabe@gmail.com
 */

import java.awt.*;

/*
 * Empty implementation of LayoutManager.
 *
 * MyScreenLayout object is almost not used at all in the code.
 *
 * Its only purpose is to "exist", (to be a valid instance of a LayoutManager object),
 * so that "setLocation()" methods work properly
 */

public class MyScreenLayout implements LayoutManager
{
	@Override
	public void addLayoutComponent(String name, Component comp) {}

	@Override
	public void removeLayoutComponent(Component comp) {}

	@Override
	public Dimension preferredLayoutSize(Container parent)
	{
		return null;
	}

	@Override
	public Dimension minimumLayoutSize(Container parent)
	{
		return null;
	}

	@Override
	public void layoutContainer(Container parent) {}
}

