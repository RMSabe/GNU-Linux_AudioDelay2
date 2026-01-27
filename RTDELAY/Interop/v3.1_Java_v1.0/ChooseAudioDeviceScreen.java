/*
 * Real-Time Audio Delay 2 for GNU-Linux systems.
 * Version 3.1 (Interop Java version 1.0)
 *
 * Author: Rafael Sabe
 * Email: rafaelmsabe@gmail.com
 */

import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.*;
import javax.swing.*;

public class ChooseAudioDeviceScreen extends MyScreen
{
	private static final String ACTIONCOMMAND_RETURNCHOOSEFILE = "RETURNCHOOSEFILE";
	private static final String ACTIONCOMMAND_CHOOSEAUDIODEV = "CHOOSEAUDIODEV";

	private JLabel jlabel1 = new JLabel();
	private JTextField jtextfield1 = new JTextField();
	private JButton jbutton1 = new JButton();
	private JButton jbutton2 = new JButton();

	private ActionListener actionListener = new ActionListener() {

		@Override
		public void actionPerformed(ActionEvent event)
		{
			if(event.getActionCommand().equals(ACTIONCOMMAND_RETURNCHOOSEFILE))
			{
				Main.switchToChooseFileScreen();
				return;
			}

			if(event.getActionCommand().equals(ACTIONCOMMAND_CHOOSEAUDIODEV))
			{
				CPPCore.setAudioDevDesc(jtextfield1.getText());
				Main.procPlaybackInit();
			}
		}

	};

	public ChooseAudioDeviceScreen(JFrame parent)
	{
		this.parent = parent;
		this.init();
	}

	@Override
	public void init()
	{
		super.init();

		this.jlabel1.setFont(new Font(Definitions.FONT_NAME, Definitions.FONT_STYLE, Definitions.TITLE_FONTSIZE));
		this.jlabel1.setForeground(Definitions.TEXT_FOREGROUNDCOLOR);
		this.jlabel1.setLayout(this.layout);
		this.jlabel1.setHorizontalAlignment(JLabel.CENTER);
		this.jlabel1.setText("Enter Audio Device's System ID:");
		this.jlabel1.setVisible(true);

		this.jtextfield1.setFont(new Font(Definitions.FONT_NAME, Definitions.FONT_STYLE, Definitions.SUBTITLE_FONTSIZE));
		this.jtextfield1.setBackground(Definitions.TEXTBOX_BACKGROUNDCOLOR);
		this.jtextfield1.setForeground(Definitions.TEXTBOX_FOREGROUNDCOLOR);
		this.jtextfield1.setLayout(this.layout);
		this.jtextfield1.setHorizontalAlignment(JTextField.CENTER);
		this.jtextfield1.setEditable(true);
		this.jtextfield1.setFocusable(true);
		this.jtextfield1.setOpaque(true);
		this.jtextfield1.setText("");
		this.jtextfield1.setVisible(true);

		this.jbutton1.setBackground(Definitions.BUTTON_BACKGROUNDCOLOR);
		this.jbutton1.setForeground(Definitions.BUTTON_FOREGROUNDCOLOR);
		this.jbutton1.setLayout(this.layout);
		this.jbutton1.addActionListener(this.actionListener);
		this.jbutton1.setActionCommand(ACTIONCOMMAND_RETURNCHOOSEFILE);
		this.jbutton1.setFocusable(true);
		this.jbutton1.setText("Return");
		this.jbutton1.setVisible(true);

		this.jbutton2.setBackground(Definitions.BUTTON_BACKGROUNDCOLOR);
		this.jbutton2.setForeground(Definitions.BUTTON_FOREGROUNDCOLOR);
		this.jbutton2.setLayout(this.layout);
		this.jbutton2.addActionListener(this.actionListener);
		this.jbutton2.setActionCommand(ACTIONCOMMAND_CHOOSEAUDIODEV);
		this.jbutton2.setFocusable(true);
		this.jbutton2.setText("Proceed");
		this.jbutton2.setVisible(true);

		this.add(this.jlabel1);
		this.add(this.jtextfield1);
		this.add(this.jbutton1);
		this.add(this.jbutton2);

		this.align();
	}

	@Override
	public void deinit()
	{
		super.deinit();

		this.jbutton1.removeActionListener(this.actionListener);
		this.jbutton2.removeActionListener(this.actionListener);

		this.remove(this.jlabel1);
		this.remove(this.jtextfield1);
		this.remove(this.jbutton1);
		this.remove(this.jbutton2);
	}

	@Override
	public void align()
	{
		Dimension parentSize = this.parent.getSize();

		Point center = new Point();

		Point jlabel1_pos = new Point();
		Dimension jlabel1_size = new Dimension();

		Point jtextfield1_pos = new Point();
		Dimension jtextfield1_size = new Dimension();

		Point jbutton1_pos = new Point();
		Dimension jbutton1_size = new Dimension();

		Point jbutton2_pos = new Point();
		Dimension jbutton2_size = new Dimension();

		this.setSize(parentSize);

		center.x = parentSize.width/2;
		center.y = parentSize.height/2;

		jlabel1_pos.x = Definitions.TITLE_MARGINLEFT;
		jlabel1_pos.y = Definitions.TITLE_MARGINTOP;

		jlabel1_size.width = parentSize.width - 2*jlabel1_pos.x;
		jlabel1_size.height = Definitions.TITLE_FONTSIZE + Definitions.TITLE_FONTSIZEMARGIN;

		jtextfield1_pos.x = jlabel1_pos.x;
		jtextfield1_pos.y = jlabel1_pos.y + jlabel1_size.height + Definitions.INTERCOMPONENT_MARGIN;

		jtextfield1_size.width = jlabel1_size.width;
		jtextfield1_size.height = Definitions.SUBTITLE_FONTSIZE + Definitions.SUBTITLE_FONTSIZEMARGIN;

		jbutton1_size.width = 100;
		jbutton1_size.height = 20;

		jbutton2_size.width = 100;
		jbutton2_size.height = 20;

		jbutton1_pos.x = center.x - jbutton1_size.width - Definitions.INTERCOMPONENT_MARGIN/2;
		jbutton1_pos.y = parentSize.height - jbutton1_size.height - Definitions.BOTTOM_WINDOW_MARGIN;

		jbutton2_pos.x = center.x + Definitions.INTERCOMPONENT_MARGIN/2;
		jbutton2_pos.y = parentSize.height - jbutton2_size.height - Definitions.BOTTOM_WINDOW_MARGIN;

		this.jlabel1.setSize(jlabel1_size);
		this.jlabel1.setLocation(jlabel1_pos);

		this.jtextfield1.setSize(jtextfield1_size);
		this.jtextfield1.setLocation(jtextfield1_pos);

		this.jbutton1.setSize(jbutton1_size);
		this.jbutton1.setLocation(jbutton1_pos);

		this.jbutton2.setSize(jbutton2_size);
		this.jbutton2.setLocation(jbutton2_pos);
	}
}

