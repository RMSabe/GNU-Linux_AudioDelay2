/*
 * Real-Time Audio Delay 2 for GNU-Linux systems.
 * Version 5.0.1 (Audio Stream Version. Interop Java version 1.0)
 *
 * Author: Rafael Sabe
 * Email: rafaelmsabe@gmail.com
 */

import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.*;
import javax.swing.*;

public class AudioSetupScreen extends MyScreen
{
	private static final String ACTIONCOMMAND_PROCEED = "PROCEED";

	private static final String ERRORMSG_INVALIDSAMPLERATE = "Error: invalid sampling rate. Sampling rate must be a positive non-zero integer.";
	private static final String ERRORMSG_INVALIDNCHANNELS = "Error: invalid number of channels. Number of channels must be a positive non-zero integer.";
	private static final String ERRORMSG_INVALIDBITDEPTH = "Error: invalid bit depth. Supported bit depth values are 16 and 24.";

	private JLabel jlabel1 = new JLabel();
	private JLabel jlabel2 = new JLabel();
	private JLabel jlabel3 = new JLabel();
	private JLabel jlabel4 = new JLabel();
	private JTextField jtextfield1 = new JTextField();
	private JTextField jtextfield2 = new JTextField();
	private JTextField jtextfield3 = new JTextField();
	private JButton jbutton1 = new JButton();

	private ActionListener actionListener = new ActionListener() {
		@Override
		public void actionPerformed(ActionEvent event)
		{
			if(event.getActionCommand().equals(ACTIONCOMMAND_PROCEED)) procValidateInputs();
		}
	};

	public AudioSetupScreen(JFrame parentWindow)
	{
		this.parentWindow = parentWindow;
		this.init();
	}

	@Override
	public void init()
	{
		super.init();

		this.jlabel1.setFont(Definitions.TITLE_FONT);
		this.jlabel1.setForeground(Definitions.TEXT_FOREGROUNDCOLOR);
		this.jlabel1.setHorizontalAlignment(JLabel.CENTER);
		this.jlabel1.setText("Enter Audio Parameters");
		this.jlabel1.setVisible(true);

		this.jlabel2.setFont(Definitions.SUBTITLE_FONT);
		this.jlabel2.setForeground(Definitions.TEXT_FOREGROUNDCOLOR);
		this.jlabel2.setHorizontalAlignment(JLabel.CENTER);
		this.jlabel2.setText("Sampling Rate (Recommended: 44100)");
		this.jlabel2.setVisible(true);

		this.jlabel3.setFont(Definitions.SUBTITLE_FONT);
		this.jlabel3.setForeground(Definitions.TEXT_FOREGROUNDCOLOR);
		this.jlabel3.setHorizontalAlignment(JLabel.CENTER);
		this.jlabel3.setText("Number Of Channels (Recommended: 2)");
		this.jlabel3.setVisible(true);

		this.jlabel4.setFont(Definitions.SUBTITLE_FONT);
		this.jlabel4.setForeground(Definitions.TEXT_FOREGROUNDCOLOR);
		this.jlabel4.setHorizontalAlignment(JLabel.CENTER);
		this.jlabel4.setText("Bit Depth (Recommended: 16)");
		this.jlabel4.setVisible(true);

		this.jtextfield1.setBackground(Definitions.TEXTBOX_BACKGROUNDCOLOR);
		this.jtextfield1.setForeground(Definitions.TEXTBOX_FOREGROUNDCOLOR);
		this.jtextfield1.setHorizontalAlignment(JTextField.CENTER);
		this.jtextfield1.setEditable(true);
		this.jtextfield1.setFocusable(true);
		this.jtextfield1.setOpaque(true);
		this.jtextfield1.setFont(Definitions.SUBTITLE_FONT);
		this.jtextfield1.setText("");
		this.jtextfield1.setVisible(true);

		this.jtextfield2.setBackground(Definitions.TEXTBOX_BACKGROUNDCOLOR);
		this.jtextfield2.setForeground(Definitions.TEXTBOX_FOREGROUNDCOLOR);
		this.jtextfield2.setHorizontalAlignment(JTextField.CENTER);
		this.jtextfield2.setEditable(true);
		this.jtextfield2.setFocusable(true);
		this.jtextfield2.setOpaque(true);
		this.jtextfield2.setFont(Definitions.SUBTITLE_FONT);
		this.jtextfield2.setText("");
		this.jtextfield2.setVisible(true);

		this.jtextfield3.setBackground(Definitions.TEXTBOX_BACKGROUNDCOLOR);
		this.jtextfield3.setForeground(Definitions.TEXTBOX_FOREGROUNDCOLOR);
		this.jtextfield3.setHorizontalAlignment(JTextField.CENTER);
		this.jtextfield3.setEditable(true);
		this.jtextfield3.setFocusable(true);
		this.jtextfield3.setOpaque(true);
		this.jtextfield3.setFont(Definitions.SUBTITLE_FONT);
		this.jtextfield3.setText("");
		this.jtextfield3.setVisible(true);

		this.jbutton1.setBackground(Definitions.BUTTON_BACKGROUNDCOLOR);
		this.jbutton1.setForeground(Definitions.BUTTON_FOREGROUNDCOLOR);
		this.jbutton1.addActionListener(this.actionListener);
		this.jbutton1.setActionCommand(ACTIONCOMMAND_PROCEED);
		this.jbutton1.setFocusable(true);
		this.jbutton1.setText("Proceed");
		this.jbutton1.setVisible(true);

		this.add(this.jlabel1);
		this.add(this.jlabel2);
		this.add(this.jlabel3);
		this.add(this.jlabel4);
		this.add(this.jtextfield1);
		this.add(this.jtextfield2);
		this.add(this.jtextfield3);
		this.add(this.jbutton1);

		this.align();
	}

	@Override
	public void deinit()
	{
		super.deinit();

		this.jbutton1.removeActionListener(this.actionListener);
		this.remove(this.jlabel1);
		this.remove(this.jlabel2);
		this.remove(this.jlabel3);
		this.remove(this.jlabel4);
		this.remove(this.jtextfield1);
		this.remove(this.jtextfield2);
		this.remove(this.jtextfield3);
		this.remove(this.jbutton1);
	}

	@Override
	public void align()
	{
		Dimension parentSize = this.parentWindow.getSize();

		Point center = new Point();

		Point jlabel1_pos = new Point();
		Dimension jlabel1_size = new Dimension();

		Point jlabel2_pos = new Point();
		Dimension jlabel2_size = new Dimension();

		Point jlabel3_pos = new Point();
		Dimension jlabel3_size = new Dimension();

		Point jlabel4_pos = new Point();
		Dimension jlabel4_size = new Dimension();

		Point jtextfield1_pos = new Point();
		Dimension jtextfield1_size = new Dimension();

		Point jtextfield2_pos = new Point();
		Dimension jtextfield2_size = new Dimension();

		Point jtextfield3_pos = new Point();
		Dimension jtextfield3_size = new Dimension();

		Point jbutton1_pos = new Point();
		Dimension jbutton1_size = new Dimension();

		this.setSize(parentSize);

		center.x = parentSize.width/2;
		center.y = parentSize.height/2;

		jlabel1_pos.x = Definitions.TITLE_MARGINLEFT;
		jlabel1_pos.y = Definitions.TITLE_MARGINTOP;

		jlabel1_size.width = parentSize.width - 2*jlabel1_pos.x;
		jlabel1_size.height = Definitions.TITLE_FONTSIZE + Definitions.TITLE_FONTSIZEMARGIN;

		jbutton1_size.width = 120;
		jbutton1_size.height = 20;

		jbutton1_pos.x = center.x - jbutton1_size.width/2;
		jbutton1_pos.y = parentSize.height - jbutton1_size.height - Definitions.BOTTOM_WINDOW_MARGIN;

		jlabel2_size.width = 500;
		jlabel2_size.height = Definitions.SUBTITLE_FONTSIZE + Definitions.SUBTITLE_FONTSIZEMARGIN;
		jlabel2_pos.x = center.x - jlabel2_size.width/2;

		jlabel3_size.width = jlabel2_size.width;
		jlabel3_size.height = jlabel2_size.height;
		jlabel3_pos.x = jlabel2_pos.x;

		jlabel4_size.width = jlabel2_size.width;
		jlabel4_size.height = jlabel2_size.height;
		jlabel4_pos.x = jlabel2_pos.x;

		jtextfield1_size.width = jlabel2_size.width;
		jtextfield1_size.height = jlabel2_size.height;
		jtextfield1_pos.x = jlabel2_pos.x;

		jtextfield2_size.width = jlabel2_size.width;
		jtextfield2_size.height = jlabel2_size.height;
		jtextfield2_pos.x = jlabel2_pos.x;

		jtextfield3_size.width = jlabel2_size.width;
		jtextfield3_size.height = jlabel2_size.height;
		jtextfield3_pos.x = jlabel2_pos.x;

		jlabel2_pos.y = jlabel1_pos.y + jlabel1_size.height + Definitions.INTERCOMPONENT_MARGIN;
		jtextfield1_pos.y = jlabel2_pos.y + jlabel2_size.height + Definitions.INTERCOMPONENT_MARGIN;
		jlabel3_pos.y = jtextfield1_pos.y + jtextfield1_size.height + Definitions.INTERCOMPONENT_MARGIN;
		jtextfield2_pos.y = jlabel3_pos.y + jlabel3_size.height + Definitions.INTERCOMPONENT_MARGIN;
		jlabel4_pos.y = jtextfield2_pos.y + jtextfield2_size.height + Definitions.INTERCOMPONENT_MARGIN;
		jtextfield3_pos.y = jlabel4_pos.y + jlabel4_size.height + Definitions.INTERCOMPONENT_MARGIN;

		this.jlabel1.setSize(jlabel1_size);
		this.jlabel1.setLocation(jlabel1_pos);

		this.jlabel2.setSize(jlabel2_size);
		this.jlabel2.setLocation(jlabel2_pos);

		this.jlabel3.setSize(jlabel3_size);
		this.jlabel3.setLocation(jlabel3_pos);

		this.jlabel4.setSize(jlabel4_size);
		this.jlabel4.setLocation(jlabel4_pos);

		this.jtextfield1.setSize(jtextfield1_size);
		this.jtextfield1.setLocation(jtextfield1_pos);

		this.jtextfield2.setSize(jtextfield2_size);
		this.jtextfield2.setLocation(jtextfield2_pos);

		this.jtextfield3.setSize(jtextfield3_size);
		this.jtextfield3.setLocation(jtextfield3_pos);

		this.jbutton1.setSize(jbutton1_size);
		this.jbutton1.setLocation(jbutton1_pos);
	}

	private void procValidateInputs()
	{
		int val = -1;

		try
		{
			val = Integer.parseInt(this.jtextfield1.getText());
			if(val <= 0) throw new Exception();
		}
		catch(Exception e)
		{
			JOptionPane.showMessageDialog(new JFrame(), ERRORMSG_INVALIDSAMPLERATE, "ERROR", JOptionPane.ERROR_MESSAGE);
			return;
		}

		CPPCore.setSampleRate(val);

		try
		{
			val = Integer.parseInt(this.jtextfield2.getText());
			if(val <= 0) throw new Exception();
		}
		catch(Exception e)
		{
			JOptionPane.showMessageDialog(new JFrame(), ERRORMSG_INVALIDNCHANNELS, "ERROR", JOptionPane.ERROR_MESSAGE);
			return;
		}

		CPPCore.setNumberOfChannels(val);

		try
		{
			val = Integer.parseInt(this.jtextfield3.getText());
			switch(val)
			{
				case 16:
					CPPCore.setAudioFormat(CPPCore.AUDIOFORMAT_I16);
					break;

				case 24:
					CPPCore.setAudioFormat(CPPCore.AUDIOFORMAT_I24);
					break;

				default:
					throw new Exception();
			}
		}
		catch(Exception e)
		{
			JOptionPane.showMessageDialog(new JFrame(), ERRORMSG_INVALIDBITDEPTH, "ERROR", JOptionPane.ERROR_MESSAGE);
			return;
		}

		Main.procCreateAudioObject();
	}
}

