/*
 * Real-Time Audio Delay 2 for GNU-Linux systems.
 * Version 3.1.1 (Interop Java version 1.0)
 *
 * Author: Rafael Sabe
 * Email: rafaelmsabe@gmail.com
 */

import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.*;
import javax.swing.*;

public class PlaybackRunningScreen extends MyScreen
{
	private static final String ACTIONCOMMAND_STOPPLAYBACK = "STOPPLAYBACK";
	private static final String ACTIONCOMMAND_RESETALLPARAMS = "RESETALLPARAMS";
	private static final String ACTIONCOMMAND_UPDATEDRYINAMP = "UPDATEDRYINAMP";
	private static final String ACTIONCOMMAND_UPDATEOUTAMP = "UPDATEOUTAMP";

	private static final String ACTIONCOMMAND_FFUPDATEDELAY = "FFUPDATEDELAY";
	private static final String ACTIONCOMMAND_FFUPDATEAMP = "FFUPDATEAMP";
	private static final String ACTIONCOMMAND_FFRESET = "FFRESET";
	private static final String ACTIONCOMMAND_FBUPDATEDELAY = "FBUPDATEDELAY";
	private static final String ACTIONCOMMAND_FBUPDATEAMP = "FBUPDATEAMP";
	private static final String ACTIONCOMMAND_FBRESET = "FBRESET";

	private static final int CONTAINERTEXT_FONTSIZE = 12;
	private static final int CONTAINERTEXT_FONTSIZEMARGIN = 16;
	private static final int CONTAINERTEXT_FONTSIZEMARGIN_TEXTAREA = 20;

	private static final int CONTAINER1_HEIGHT = 150;
	private static final int CONTAINER2_HEIGHT = 120;
	private static final int CONTAINER3_HEIGHT = 200;
	private static final int CONTAINER4_HEIGHT = 200;

	private static final int SUBCONTAINER_RADIOBUTTON_WIDTH = 150;

	private static final int ATTEMPTUPDATEPARAM_DRYINAMP = 1;
	private static final int ATTEMPTUPDATEPARAM_OUTAMP = 2;
	private static final int ATTEMPTUPDATEPARAM_FFDELAY = 3;
	private static final int ATTEMPTUPDATEPARAM_FFAMP = 4;
	private static final int ATTEMPTUPDATEPARAM_FBDELAY = 5;
	private static final int ATTEMPTUPDATEPARAM_FBAMP = 6;

	private JLabel jlabel_title = new JLabel();
	private JLabel jlabel_subtitle = new JLabel();
	private JButton jbutton_stoppb = new JButton();
	private JButton jbutton_resetall = new JButton();

	private JPanel jpanel_container1 = new JPanel();
	private JPanel jpanel_container2 = new JPanel();
	private JPanel jpanel_container3 = new JPanel();
	private JPanel jpanel_container4 = new JPanel();

	private JPanel jpanel_container3_subcontainer1 = new JPanel();
	private JPanel jpanel_container3_subcontainer2 = new JPanel();
	private JPanel jpanel_container3_subcontainer3 = new JPanel();

	private JPanel jpanel_container4_subcontainer1 = new JPanel();
	private JPanel jpanel_container4_subcontainer2 = new JPanel();
	private JPanel jpanel_container4_subcontainer3 = new JPanel();

	/*
	 * Container 1 (Current parameters):
	 *
	 * params: container label
	 *
	 * paramsin: dry input amplitude + ff delay params
	 *
	 * paramsout: output amplitude + fb delay params
	 */

	private JLabel jlabel_params = new JLabel();
	private JTextArea jtextarea_paramsin = new JTextArea();
	private JTextArea jtextarea_paramsout = new JTextArea();

	/*
	 * Container 2 (Set dry input amplitude / output amplitude)
	 *
	 * t_ : label
	 * tb_ : textbox (user input)
	 * b_ : button (update value)
	 */

	private JLabel jlabel_t_dryinupdate = new JLabel();
	private JLabel jlabel_t_outupdate = new JLabel();
	private JTextField jtextfield_tb_dryinupdate = new JTextField();
	private JTextField jtextfield_tb_outupdate = new JTextField();
	private JButton jbutton_b_dryinupdate = new JButton();
	private JButton jbutton_b_outupdate = new JButton();

	/*
	 * Container 3 (Set FF delay params)
	 *
	 * t_ : label
	 * tb_ : textbox (user input)
	 * b_ : button
	 * bg_ : button group
	 * rb_ : radio button
	 *
	 * ffsel: radiobuttons (FF delay channel selector)
	 *
	 * subcontainer1: ff sel
	 * subcontainer2: ff delay
	 * subcontainer3: ff amp
	 */

	private JLabel jlabel_t_ff = new JLabel();

	private ButtonGroup buttongroup_bg_ffsel = new ButtonGroup();
	private JRadioButton[] array_jradiobutton_rb_ffsel = new JRadioButton[CPPCore.RTDELAY_N_FF_DELAYS];
	private JLabel jlabel_t_ffsel = new JLabel();

	private JLabel jlabel_t_ffdupdate = new JLabel();
	private JTextField jtextfield_tb_ffdupdate = new JTextField();
	private JButton jbutton_b_ffdupdate = new JButton();

	private JLabel jlabel_t_ffaupdate = new JLabel();
	private JTextField jtextfield_tb_ffaupdate = new JTextField();
	private JButton jbutton_b_ffaupdate = new JButton();

	private JButton jbutton_b_ffreset = new JButton();

	/*
	 * Container 4 (Set FB delay params)
	 *
	 * t_ : label
	 * tb_ : textbox (user input)
	 * b_ : button
	 * bg_ : button group
	 * rb_ : radio button
	 *
	 * fbsel: radiobuttons (FB delay channel selector)
	 *
	 * subcontainer 1: fb sel
	 * subcontainer 2: fb delay
	 * subcontainer 3: fb amp
	 */

	private JLabel jlabel_t_fb = new JLabel();

	private ButtonGroup buttongroup_bg_fbsel = new ButtonGroup();
	private JRadioButton[] array_jradiobutton_rb_fbsel = new JRadioButton[CPPCore.RTDELAY_N_FB_DELAYS];
	private JLabel jlabel_t_fbsel = new JLabel();

	private JLabel jlabel_t_fbdupdate = new JLabel();
	private JTextField jtextfield_tb_fbdupdate = new JTextField();
	private JButton jbutton_b_fbdupdate = new JButton();

	private JLabel jlabel_t_fbaupdate = new JLabel();
	private JTextField jtextfield_tb_fbaupdate = new JTextField();
	private JButton jbutton_b_fbaupdate = new JButton();

	private JButton jbutton_b_fbreset = new JButton();

	private ActionListener actionListener = new ActionListener() {

		@Override
		public void actionPerformed(ActionEvent event)
		{
			String eventCmd = event.getActionCommand();

			if(eventCmd.equals(ACTIONCOMMAND_STOPPLAYBACK))
			{
				CPPCore.stopPlayback();
				return;
			}

			if(eventCmd.equals(ACTIONCOMMAND_RESETALLPARAMS))
			{
				CPPCore.resetFFParams();
				CPPCore.resetFBParams();
				CPPCore.setDryInputAmplitude(1.0f);
				CPPCore.setOutputAmplitude(1.0f);
				loadUpdateParams();

				return;
			}

			if(eventCmd.equals(ACTIONCOMMAND_UPDATEDRYINAMP))
			{
				attemptUpdateParam(ATTEMPTUPDATEPARAM_DRYINAMP, jtextfield_tb_dryinupdate);
				return;
			}

			if(eventCmd.equals(ACTIONCOMMAND_UPDATEOUTAMP))
			{
				attemptUpdateParam(ATTEMPTUPDATEPARAM_OUTAMP, jtextfield_tb_outupdate);
				return;
			}

			if(eventCmd.equals(ACTIONCOMMAND_FFRESET))
			{
				CPPCore.resetFFParams();
				loadUpdateParams();
				return;
			}

			if(eventCmd.equals(ACTIONCOMMAND_FFUPDATEDELAY))
			{
				attemptUpdateParam(ATTEMPTUPDATEPARAM_FFDELAY, jtextfield_tb_ffdupdate);
				return;
			}

			if(eventCmd.equals(ACTIONCOMMAND_FFUPDATEAMP))
			{
				attemptUpdateParam(ATTEMPTUPDATEPARAM_FFAMP, jtextfield_tb_ffaupdate);
				return;
			}

			if(eventCmd.equals(ACTIONCOMMAND_FBRESET))
			{
				CPPCore.resetFBParams();
				loadUpdateParams();
				return;
			}

			if(eventCmd.equals(ACTIONCOMMAND_FBUPDATEDELAY))
			{
				attemptUpdateParam(ATTEMPTUPDATEPARAM_FBDELAY, jtextfield_tb_fbdupdate);
				return;
			}

			if(eventCmd.equals(ACTIONCOMMAND_FBUPDATEAMP))
			{
				attemptUpdateParam(ATTEMPTUPDATEPARAM_FBAMP, jtextfield_tb_fbaupdate);
				return;
			}
		}

	};

	public PlaybackRunningScreen(JFrame parent)
	{
		this.parent = parent;
		this.init();
	}

	@Override
	public void init()
	{
		Font titleFont = new Font(Definitions.FONT_NAME, Definitions.FONT_STYLE, Definitions.TITLE_FONTSIZE);
		Font subtitleFont = new Font(Definitions.FONT_NAME, Definitions.FONT_STYLE, Definitions.SUBTITLE_FONTSIZE);
		Font containerFont = new Font(Definitions.FONT_NAME, Definitions.FONT_STYLE, CONTAINERTEXT_FONTSIZE);

		String rb_text = "";
		int rb_index = 0;
		int rb_length = 0;

		super.init();

		this.setupTextDefault(this.jlabel_title);
		this.jlabel_title.setFont(titleFont);
		this.jlabel_title.setText("Playback Running");

		this.setupTextDefault(this.jlabel_subtitle);
		this.jlabel_subtitle.setFont(subtitleFont);
		this.jlabel_subtitle.setText("WARNING: feedback delays are Infinite Impulse Response routes. Improper settings may cause it to clip endlessly.");

		this.setupButtonDefault(this.jbutton_stoppb);
		this.jbutton_stoppb.setActionCommand(ACTIONCOMMAND_STOPPLAYBACK);
		this.jbutton_stoppb.setText("Stop Playback");

		this.setupButtonDefault(this.jbutton_resetall);
		this.jbutton_resetall.setActionCommand(ACTIONCOMMAND_RESETALLPARAMS);
		this.jbutton_resetall.setText("Reset All Parameters");

		this.setupContainerDefault(this.jpanel_container1, true);
		this.jpanel_container1.setBackground(Definitions.CONTAINER_BACKGROUNDCOLOR);

		this.setupContainerDefault(this.jpanel_container2, true);
		this.jpanel_container2.setBackground(Definitions.CONTAINER_BACKGROUNDCOLOR);

		this.setupContainerDefault(this.jpanel_container3, true);
		this.jpanel_container3.setBackground(Definitions.CONTAINER_BACKGROUNDCOLOR);

		this.setupContainerDefault(this.jpanel_container4, true);
		this.jpanel_container4.setBackground(Definitions.CONTAINER_BACKGROUNDCOLOR);

		this.add(this.jlabel_title);
		this.add(this.jlabel_subtitle);
		this.add(this.jbutton_stoppb);
		this.add(this.jbutton_resetall);
		this.add(this.jpanel_container1);
		this.add(this.jpanel_container2);
		this.add(this.jpanel_container3);
		this.add(this.jpanel_container4);

		/*Container 1 Components*/

		this.setupTextDefault(this.jlabel_params);
		this.jlabel_params.setFont(containerFont);
		this.jlabel_params.setText("Current Parameters");

		this.setupTextDefault(this.jtextarea_paramsin);
		this.jtextarea_paramsin.setFont(containerFont);
		this.jtextarea_paramsin.setText("");

		this.setupTextDefault(this.jtextarea_paramsout);
		this.jtextarea_paramsout.setFont(containerFont);
		this.jtextarea_paramsout.setText("");

		this.jpanel_container1.add(this.jlabel_params);
		this.jpanel_container1.add(this.jtextarea_paramsin);
		this.jpanel_container1.add(this.jtextarea_paramsout);

		/*Container 2 Components*/

		this.setupTextDefault(this.jlabel_t_dryinupdate);
		this.jlabel_t_dryinupdate.setFont(containerFont);
		this.jlabel_t_dryinupdate.setText("Enter Dry Input Amplitude");

		this.setupTextDefault(this.jlabel_t_outupdate);
		this.jlabel_t_outupdate.setFont(containerFont);
		this.jlabel_t_outupdate.setText("Enter Output Amplitude");

		this.setupTextBoxDefault(this.jtextfield_tb_dryinupdate);
		this.jtextfield_tb_dryinupdate.setFont(containerFont);
		this.jtextfield_tb_dryinupdate.setText("");

		this.setupTextBoxDefault(this.jtextfield_tb_outupdate);
		this.jtextfield_tb_outupdate.setFont(containerFont);
		this.jtextfield_tb_outupdate.setText("");

		this.setupButtonDefault(this.jbutton_b_dryinupdate);
		this.jbutton_b_dryinupdate.setActionCommand(ACTIONCOMMAND_UPDATEDRYINAMP);
		this.jbutton_b_dryinupdate.setText("Update Dry Input Amplitude");

		this.setupButtonDefault(this.jbutton_b_outupdate);
		this.jbutton_b_outupdate.setActionCommand(ACTIONCOMMAND_UPDATEOUTAMP);
		this.jbutton_b_outupdate.setText("Update Output Amplitude");

		this.jpanel_container2.add(this.jlabel_t_dryinupdate);
		this.jpanel_container2.add(this.jlabel_t_outupdate);
		this.jpanel_container2.add(this.jtextfield_tb_dryinupdate);
		this.jpanel_container2.add(this.jtextfield_tb_outupdate);
		this.jpanel_container2.add(this.jbutton_b_dryinupdate);
		this.jpanel_container2.add(this.jbutton_b_outupdate);

		/*Container 3 Components*/

		this.setupTextDefault(this.jlabel_t_ff);
		this.jlabel_t_ff.setFont(containerFont);
		this.jlabel_t_ff.setText("Feedforward Delay");

		this.setupButtonDefault(this.jbutton_b_ffreset);
		this.jbutton_b_ffreset.setActionCommand(ACTIONCOMMAND_FFRESET);
		this.jbutton_b_ffreset.setText("Reset FF Parameters");

		this.setupContainerDefault(this.jpanel_container3_subcontainer1, false);
		this.setupContainerDefault(this.jpanel_container3_subcontainer2, false);
		this.setupContainerDefault(this.jpanel_container3_subcontainer3, false);

		this.jpanel_container3.add(this.jlabel_t_ff);
		this.jpanel_container3.add(this.jbutton_b_ffreset);
		this.jpanel_container3.add(this.jpanel_container3_subcontainer1);
		this.jpanel_container3.add(this.jpanel_container3_subcontainer2);
		this.jpanel_container3.add(this.jpanel_container3_subcontainer3);

		this.setupTextDefault(this.jlabel_t_ffdupdate);
		this.jlabel_t_ffdupdate.setFont(containerFont);
		this.jlabel_t_ffdupdate.setText("Enter FF Delay Time (# samples)");

		this.setupTextBoxDefault(this.jtextfield_tb_ffdupdate);
		this.jtextfield_tb_ffdupdate.setFont(containerFont);
		this.jtextfield_tb_ffdupdate.setText("");

		this.setupButtonDefault(this.jbutton_b_ffdupdate);
		this.jbutton_b_ffdupdate.setActionCommand(ACTIONCOMMAND_FFUPDATEDELAY);
		this.jbutton_b_ffdupdate.setText("Update FF Delay Time");

		this.jpanel_container3_subcontainer2.add(this.jlabel_t_ffdupdate);
		this.jpanel_container3_subcontainer2.add(this.jtextfield_tb_ffdupdate);
		this.jpanel_container3_subcontainer2.add(this.jbutton_b_ffdupdate);

		this.setupTextDefault(this.jlabel_t_ffaupdate);
		this.jlabel_t_ffaupdate.setFont(containerFont);
		this.jlabel_t_ffaupdate.setText("Enter FF Amplitude");

		this.setupTextBoxDefault(this.jtextfield_tb_ffaupdate);
		this.jtextfield_tb_ffaupdate.setFont(containerFont);
		this.jtextfield_tb_ffaupdate.setText("");

		this.setupButtonDefault(this.jbutton_b_ffaupdate);
		this.jbutton_b_ffaupdate.setActionCommand(ACTIONCOMMAND_FFUPDATEAMP);
		this.jbutton_b_ffaupdate.setText("Update FF Amplitude");

		this.jpanel_container3_subcontainer3.add(this.jlabel_t_ffaupdate);
		this.jpanel_container3_subcontainer3.add(this.jtextfield_tb_ffaupdate);
		this.jpanel_container3_subcontainer3.add(this.jbutton_b_ffaupdate);

		this.setupTextDefault(this.jlabel_t_ffsel);
		this.jlabel_t_ffsel.setFont(containerFont);
		this.jlabel_t_ffsel.setText("FF Channel Select");

		this.jpanel_container3_subcontainer1.add(this.jlabel_t_ffsel);

		rb_length = CPPCore.RTDELAY_N_FF_DELAYS;

		for(rb_index = 0; rb_index < rb_length; rb_index++)
		{
			this.array_jradiobutton_rb_ffsel[rb_index] = new JRadioButton();

			rb_text = "FF Channel ";
			rb_text += String.valueOf(rb_index + 1);

			this.array_jradiobutton_rb_ffsel[rb_index].setLayout(this.layout);
			this.array_jradiobutton_rb_ffsel[rb_index].setFocusable(true);
			this.array_jradiobutton_rb_ffsel[rb_index].setOpaque(false);
			this.array_jradiobutton_rb_ffsel[rb_index].setText(rb_text);
			this.array_jradiobutton_rb_ffsel[rb_index].setVisible(true);

			this.buttongroup_bg_ffsel.add(this.array_jradiobutton_rb_ffsel[rb_index]);
			this.jpanel_container3_subcontainer1.add(this.array_jradiobutton_rb_ffsel[rb_index]);
		}

		/*Container 4 Components*/

		this.setupTextDefault(this.jlabel_t_fb);
		this.jlabel_t_fb.setFont(containerFont);
		this.jlabel_t_fb.setText("Feedback Delay");

		this.setupButtonDefault(this.jbutton_b_fbreset);
		this.jbutton_b_fbreset.setActionCommand(ACTIONCOMMAND_FBRESET);
		this.jbutton_b_fbreset.setText("Reset FB Parameters");

		this.setupContainerDefault(this.jpanel_container4_subcontainer1, false);
		this.setupContainerDefault(this.jpanel_container4_subcontainer2, false);
		this.setupContainerDefault(this.jpanel_container4_subcontainer3, false);

		this.jpanel_container4.add(this.jlabel_t_fb);
		this.jpanel_container4.add(this.jbutton_b_fbreset);
		this.jpanel_container4.add(this.jpanel_container4_subcontainer1);
		this.jpanel_container4.add(this.jpanel_container4_subcontainer2);
		this.jpanel_container4.add(this.jpanel_container4_subcontainer3);

		this.setupTextDefault(this.jlabel_t_fbdupdate);
		this.jlabel_t_fbdupdate.setFont(containerFont);
		this.jlabel_t_fbdupdate.setText("Enter FB Delay Time (# samples)");

		this.setupTextBoxDefault(this.jtextfield_tb_fbdupdate);
		this.jtextfield_tb_fbdupdate.setFont(containerFont);
		this.jtextfield_tb_fbdupdate.setText("");

		this.setupButtonDefault(this.jbutton_b_fbdupdate);
		this.jbutton_b_fbdupdate.setActionCommand(ACTIONCOMMAND_FBUPDATEDELAY);
		this.jbutton_b_fbdupdate.setText("Update FB Delay Time");

		this.jpanel_container4_subcontainer2.add(this.jlabel_t_fbdupdate);
		this.jpanel_container4_subcontainer2.add(this.jtextfield_tb_fbdupdate);
		this.jpanel_container4_subcontainer2.add(this.jbutton_b_fbdupdate);

		this.setupTextDefault(this.jlabel_t_fbaupdate);
		this.jlabel_t_fbaupdate.setFont(containerFont);
		this.jlabel_t_fbaupdate.setText("Enter FB Amplitude");

		this.setupTextBoxDefault(this.jtextfield_tb_fbaupdate);
		this.jtextfield_tb_fbaupdate.setFont(containerFont);
		this.jtextfield_tb_fbaupdate.setText("");

		this.setupButtonDefault(this.jbutton_b_fbaupdate);
		this.jbutton_b_fbaupdate.setActionCommand(ACTIONCOMMAND_FBUPDATEAMP);
		this.jbutton_b_fbaupdate.setText("Update FB Amplitude");

		this.jpanel_container4_subcontainer3.add(this.jlabel_t_fbaupdate);
		this.jpanel_container4_subcontainer3.add(this.jtextfield_tb_fbaupdate);
		this.jpanel_container4_subcontainer3.add(this.jbutton_b_fbaupdate);

		this.setupTextDefault(this.jlabel_t_fbsel);
		this.jlabel_t_fbsel.setFont(containerFont);
		this.jlabel_t_fbsel.setText("FB Channel Select");

		this.jpanel_container4_subcontainer1.add(this.jlabel_t_fbsel);

		rb_length = CPPCore.RTDELAY_N_FB_DELAYS;

		for(rb_index = 0; rb_index < rb_length; rb_index++)
		{
			this.array_jradiobutton_rb_fbsel[rb_index] = new JRadioButton();

			rb_text = "FB Channel ";
			rb_text += String.valueOf(rb_index + 1);

			this.array_jradiobutton_rb_fbsel[rb_index].setLayout(this.layout);
			this.array_jradiobutton_rb_fbsel[rb_index].setFocusable(true);
			this.array_jradiobutton_rb_fbsel[rb_index].setOpaque(false);
			this.array_jradiobutton_rb_fbsel[rb_index].setText(rb_text);
			this.array_jradiobutton_rb_fbsel[rb_index].setVisible(true);

			this.buttongroup_bg_fbsel.add(this.array_jradiobutton_rb_fbsel[rb_index]);
			this.jpanel_container4_subcontainer1.add(this.array_jradiobutton_rb_fbsel[rb_index]);
		}

		this.align();
		this.loadUpdateParams();
	}

	@Override
	public void deinit()
	{
		int rb_length = 0;
		int rb_index = 0;

		super.deinit();

		this.jbutton_stoppb.removeActionListener(this.actionListener);
		this.jbutton_resetall.removeActionListener(this.actionListener);
		this.jbutton_b_dryinupdate.removeActionListener(this.actionListener);
		this.jbutton_b_outupdate.removeActionListener(this.actionListener);
		this.jbutton_b_ffreset.removeActionListener(this.actionListener);
		this.jbutton_b_ffdupdate.removeActionListener(this.actionListener);
		this.jbutton_b_ffaupdate.removeActionListener(this.actionListener);
		this.jbutton_b_fbreset.removeActionListener(this.actionListener);
		this.jbutton_b_fbdupdate.removeActionListener(this.actionListener);
		this.jbutton_b_fbaupdate.removeActionListener(this.actionListener);

		this.jpanel_container1.remove(this.jlabel_params);
		this.jpanel_container1.remove(this.jtextarea_paramsin);
		this.jpanel_container1.remove(this.jtextarea_paramsout);

		this.jpanel_container2.remove(this.jlabel_t_dryinupdate);
		this.jpanel_container2.remove(this.jtextfield_tb_dryinupdate);
		this.jpanel_container2.remove(this.jbutton_b_dryinupdate);
		this.jpanel_container2.remove(this.jlabel_t_outupdate);
		this.jpanel_container2.remove(this.jtextfield_tb_outupdate);
		this.jpanel_container2.remove(this.jbutton_b_outupdate);

		rb_length = CPPCore.RTDELAY_N_FF_DELAYS;

		for(rb_index = 0; rb_index < rb_length; rb_index++)
		{
			this.buttongroup_bg_ffsel.remove(this.array_jradiobutton_rb_ffsel[rb_index]);
			this.jpanel_container3_subcontainer1.remove(this.array_jradiobutton_rb_ffsel[rb_index]);
		}

		this.jpanel_container3_subcontainer1.remove(this.jlabel_t_ffsel);
		this.jpanel_container3_subcontainer2.remove(this.jlabel_t_ffdupdate);
		this.jpanel_container3_subcontainer2.remove(this.jtextfield_tb_ffdupdate);
		this.jpanel_container3_subcontainer2.remove(this.jbutton_b_ffdupdate);
		this.jpanel_container3_subcontainer3.remove(this.jlabel_t_ffaupdate);
		this.jpanel_container3_subcontainer3.remove(this.jtextfield_tb_ffaupdate);
		this.jpanel_container3_subcontainer3.remove(this.jbutton_b_ffaupdate);

		this.jpanel_container3.remove(this.jlabel_t_ff);
		this.jpanel_container3.remove(this.jbutton_b_ffreset);
		this.jpanel_container3.remove(this.jpanel_container3_subcontainer1);
		this.jpanel_container3.remove(this.jpanel_container3_subcontainer2);
		this.jpanel_container3.remove(this.jpanel_container3_subcontainer3);

		rb_length = CPPCore.RTDELAY_N_FB_DELAYS;

		for(rb_index = 0; rb_index < rb_length; rb_index++)
		{
			this.buttongroup_bg_fbsel.remove(this.array_jradiobutton_rb_fbsel[rb_index]);
			this.jpanel_container4_subcontainer1.remove(this.array_jradiobutton_rb_fbsel[rb_index]);
		}

		this.jpanel_container4_subcontainer1.remove(this.jlabel_t_fbsel);
		this.jpanel_container4_subcontainer2.remove(this.jlabel_t_fbdupdate);
		this.jpanel_container4_subcontainer2.remove(this.jtextfield_tb_fbdupdate);
		this.jpanel_container4_subcontainer2.remove(this.jbutton_b_fbdupdate);
		this.jpanel_container4_subcontainer3.remove(this.jlabel_t_fbaupdate);
		this.jpanel_container4_subcontainer3.remove(this.jtextfield_tb_fbaupdate);
		this.jpanel_container4_subcontainer3.remove(this.jbutton_b_fbaupdate);

		this.jpanel_container4.remove(this.jlabel_t_fb);
		this.jpanel_container4.remove(this.jbutton_b_fbreset);
		this.jpanel_container4.remove(this.jpanel_container4_subcontainer1);
		this.jpanel_container4.remove(this.jpanel_container4_subcontainer2);
		this.jpanel_container4.remove(this.jpanel_container4_subcontainer3);

		this.remove(this.jlabel_title);
		this.remove(this.jlabel_subtitle);
		this.remove(this.jbutton_stoppb);
		this.remove(this.jbutton_resetall);
		this.remove(this.jpanel_container1);
		this.remove(this.jpanel_container2);
		this.remove(this.jpanel_container3);
		this.remove(this.jpanel_container4);
	}

	@Override
	public void align()
	{
		Dimension parentSize = this.parent.getSize();

		Point center = new Point();

		Point title_pos = new Point();
		Dimension title_size = new Dimension();

		Point subtitle_pos = new Point();
		Dimension subtitle_size = new Dimension();

		Point b_stoppb_pos = new Point();
		Dimension b_stoppb_size = new Dimension();

		Point b_resetall_pos = new Point();
		Dimension b_resetall_size = new Dimension();

		Point container1_pos = new Point();
		Dimension container1_size = new Dimension();

		Point container2_pos = new Point();
		Dimension container2_size = new Dimension();

		Point container3_pos = new Point();
		Dimension container3_size = new Dimension();

		Point container4_pos = new Point();
		Dimension container4_size = new Dimension();

		this.setSize(parentSize);

		center.x = parentSize.width/2;
		center.y = parentSize.height/2;

		title_pos.x = Definitions.TITLE_MARGINLEFT;
		title_pos.y = Definitions.TITLE_MARGINTOP;

		title_size.width = parentSize.width - 2*title_pos.x;
		title_size.height = Definitions.TITLE_FONTSIZE + Definitions.TITLE_FONTSIZEMARGIN;

		subtitle_pos.x = title_pos.x;
		subtitle_pos.y = title_pos.y + title_size.height + Definitions.INTERCOMPONENT_MARGIN;

		subtitle_size.width = title_size.width;
		subtitle_size.height = Definitions.SUBTITLE_FONTSIZE + Definitions.SUBTITLE_FONTSIZEMARGIN;

		container1_size.width = title_size.width;
		container1_size.height = CONTAINER1_HEIGHT;

		container1_pos.x = title_pos.x;
		container1_pos.y = subtitle_pos.y + subtitle_size.height + Definitions.INTERCOMPONENT_MARGIN;

		container2_size.width = container1_size.width;
		container2_size.height = CONTAINER2_HEIGHT;

		container2_pos.x = container1_pos.x;
		container2_pos.y = container1_pos.y + container1_size.height + Definitions.INTERCOMPONENT_MARGIN;

		container3_size.width = container2_size.width;
		container3_size.height = CONTAINER3_HEIGHT;

		container3_pos.x = container2_pos.x;
		container3_pos.y = container2_pos.y + container2_size.height + Definitions.INTERCOMPONENT_MARGIN;

		container4_size.width = container3_size.width;
		container4_size.height = CONTAINER4_HEIGHT;

		container4_pos.x = container3_pos.x;
		container4_pos.y = container3_pos.y + container3_size.height + Definitions.INTERCOMPONENT_MARGIN;

		b_resetall_size.width = 200;
		b_resetall_size.height = 20;

		b_resetall_pos.x = center.x - b_resetall_size.width/2;
		b_resetall_pos.y = container4_pos.y + container4_size.height + Definitions.INTERCOMPONENT_MARGIN;

		b_stoppb_size.width = 200;
		b_stoppb_size.height = 20;

		b_stoppb_pos.x = center.x - b_stoppb_size.width/2;
		b_stoppb_pos.y = parentSize.height - b_stoppb_size.height - Definitions.BOTTOM_WINDOW_MARGIN;

		this.jlabel_title.setSize(title_size);
		this.jlabel_title.setLocation(title_pos);

		this.jlabel_subtitle.setSize(subtitle_size);
		this.jlabel_subtitle.setLocation(subtitle_pos);

		this.jbutton_stoppb.setSize(b_stoppb_size);
		this.jbutton_stoppb.setLocation(b_stoppb_pos);

		this.jbutton_resetall.setSize(b_resetall_size);
		this.jbutton_resetall.setLocation(b_resetall_pos);

		this.jpanel_container1.setSize(container1_size);
		this.jpanel_container1.setLocation(container1_pos);

		this.jpanel_container2.setSize(container2_size);
		this.jpanel_container2.setLocation(container2_pos);

		this.jpanel_container3.setSize(container3_size);
		this.jpanel_container3.setLocation(container3_pos);

		this.jpanel_container4.setSize(container4_size);
		this.jpanel_container4.setLocation(container4_pos);

		this.alignContainer1();
		this.alignContainer2();
		this.alignContainer3();
		this.alignContainer4();
	}

	private void alignContainer1()
	{
		Dimension parentSize = this.jpanel_container1.getSize();
		Point center = new Point();

		Point params_pos = new Point();
		Dimension params_size = new Dimension();

		Point paramsin_pos = new Point();
		Dimension paramsin_size = new Dimension();

		Point paramsout_pos = new Point();
		Dimension paramsout_size = new Dimension();

		center.x = parentSize.width/2;
		center.y = parentSize.height/2;

		params_pos.x = Definitions.INTERCOMPONENT_MARGIN;
		params_pos.y = Definitions.INTERCOMPONENT_MARGIN;

		params_size.width = parentSize.width - 2*params_pos.x;
		params_size.height = CONTAINERTEXT_FONTSIZE + CONTAINERTEXT_FONTSIZEMARGIN;

		paramsin_pos.x = params_pos.x;

		paramsin_size.width = center.x - paramsin_pos.x - Definitions.INTERCOMPONENT_MARGIN/2;
		paramsin_size.height = (2 + CPPCore.RTDELAY_N_FF_DELAYS)*CONTAINERTEXT_FONTSIZE + CONTAINERTEXT_FONTSIZEMARGIN_TEXTAREA;

		paramsin_pos.y = parentSize.height - paramsin_size.height - Definitions.INTERCOMPONENT_MARGIN;

		paramsout_pos.x = center.x + Definitions.INTERCOMPONENT_MARGIN/2;

		paramsout_size.width = paramsin_size.width;
		paramsout_size.height = (2 + CPPCore.RTDELAY_N_FB_DELAYS)*CONTAINERTEXT_FONTSIZE + CONTAINERTEXT_FONTSIZEMARGIN_TEXTAREA;

		paramsout_pos.y = parentSize.height - paramsout_size.height - Definitions.INTERCOMPONENT_MARGIN;

		this.jlabel_params.setSize(params_size);
		this.jlabel_params.setLocation(params_pos);

		this.jtextarea_paramsin.setSize(paramsin_size);
		this.jtextarea_paramsin.setLocation(paramsin_pos);

		this.jtextarea_paramsout.setSize(paramsout_size);
		this.jtextarea_paramsout.setLocation(paramsout_pos);
	}

	private void alignContainer2()
	{
		Dimension parentSize = this.jpanel_container2.getSize();
		Point center = new Point();

		Point t_dryin_pos = new Point();
		Dimension t_dryin_size = new Dimension();

		Point tb_dryin_pos = new Point();
		Dimension tb_dryin_size = new Dimension();

		Point b_dryin_pos = new Point();
		Dimension b_dryin_size = new Dimension();

		Point t_out_pos = new Point();
		Dimension t_out_size = new Dimension();

		Point tb_out_pos = new Point();
		Dimension tb_out_size = new Dimension();

		Point b_out_pos = new Point();
		Dimension b_out_size = new Dimension();

		center.x = parentSize.width/2;
		center.y = parentSize.height/2;

		t_dryin_pos.x = Definitions.INTERCOMPONENT_MARGIN;
		t_dryin_pos.y = Definitions.INTERCOMPONENT_MARGIN;

		t_dryin_size.width = center.x - t_dryin_pos.x - Definitions.INTERCOMPONENT_MARGIN/2;
		t_dryin_size.height = CONTAINERTEXT_FONTSIZE + CONTAINERTEXT_FONTSIZEMARGIN;

		t_out_pos.x = center.x + Definitions.INTERCOMPONENT_MARGIN/2;
		t_out_pos.y = t_dryin_pos.y;

		t_out_size.width = t_dryin_size.width;
		t_out_size.height = t_dryin_size.height;

		tb_dryin_pos.x = t_dryin_pos.x;
		tb_dryin_pos.y = t_dryin_pos.y + t_dryin_size.height + Definitions.INTERCOMPONENT_MARGIN;

		tb_dryin_size.width = t_dryin_size.width;
		tb_dryin_size.height = CONTAINERTEXT_FONTSIZE + CONTAINERTEXT_FONTSIZEMARGIN;

		tb_out_pos.x = t_out_pos.x;
		tb_out_pos.y = tb_dryin_pos.y;

		tb_out_size.width = t_out_size.width;
		tb_out_size.height = tb_dryin_size.height;

		b_dryin_size.width = t_dryin_size.width;
		b_dryin_size.height = 20;

		b_dryin_pos.x = t_dryin_pos.x;
		b_dryin_pos.y = tb_dryin_pos.y + tb_dryin_size.height + Definitions.INTERCOMPONENT_MARGIN;

		b_out_size.width = t_out_size.width;
		b_out_size.height = 20;

		b_out_pos.x = t_out_pos.x;
		b_out_pos.y = b_dryin_pos.y;

		this.jlabel_t_dryinupdate.setSize(t_dryin_size);
		this.jlabel_t_dryinupdate.setLocation(t_dryin_pos);

		this.jtextfield_tb_dryinupdate.setSize(tb_dryin_size);
		this.jtextfield_tb_dryinupdate.setLocation(tb_dryin_pos);

		this.jbutton_b_dryinupdate.setSize(b_dryin_size);
		this.jbutton_b_dryinupdate.setLocation(b_dryin_pos);

		this.jlabel_t_outupdate.setSize(t_out_size);
		this.jlabel_t_outupdate.setLocation(t_out_pos);

		this.jtextfield_tb_outupdate.setSize(tb_out_size);
		this.jtextfield_tb_outupdate.setLocation(tb_out_pos);

		this.jbutton_b_outupdate.setSize(b_out_size);
		this.jbutton_b_outupdate.setLocation(b_out_pos);
	}

	private void alignContainer3()
	{
		Dimension parentSize = this.jpanel_container3.getSize();

		Point center = new Point();

		Point subcontainer1_pos = new Point();
		Dimension subcontainer1_size = new Dimension();

		Point subcontainer2_pos = new Point();
		Dimension subcontainer2_size = new Dimension();

		Point subcontainer3_pos = new Point();
		Dimension subcontainer3_size = new Dimension();

		Point t_ff_pos = new Point();
		Dimension t_ff_size = new Dimension();

		Point b_ffreset_pos = new Point();
		Dimension b_ffreset_size = new Dimension();

		Point t_ffd_pos = new Point();
		Dimension t_ffd_size = new Dimension();

		Point tb_ffd_pos = new Point();
		Dimension tb_ffd_size = new Dimension();

		Point b_ffd_pos = new Point();
		Dimension b_ffd_size = new Dimension();

		Point t_ffa_pos = new Point();
		Dimension t_ffa_size = new Dimension();

		Point tb_ffa_pos = new Point();
		Dimension tb_ffa_size = new Dimension();

		Point b_ffa_pos = new Point();
		Dimension b_ffa_size = new Dimension();

		Point t_ffsel_pos = new Point();
		Dimension t_ffsel_size = new Dimension();

		Point[] rb_ffsel_pos = new Point[this.array_jradiobutton_rb_ffsel.length];
		Dimension[] rb_ffsel_size = new Dimension[this.array_jradiobutton_rb_ffsel.length];

		final int RB_LENGTH = this.array_jradiobutton_rb_ffsel.length;
		int rb_index = 0;

		for(rb_index = 0; rb_index < RB_LENGTH; rb_index++)
		{
			rb_ffsel_pos[rb_index] = new Point();
			rb_ffsel_size[rb_index] = new Dimension();
		}

		center.x = parentSize.width/2;
		center.y = parentSize.height/2;

		t_ff_pos.x = Definitions.INTERCOMPONENT_MARGIN;
		t_ff_pos.y = Definitions.INTERCOMPONENT_MARGIN;

		t_ff_size.width = parentSize.width - 2*t_ff_pos.x;
		t_ff_size.height = CONTAINERTEXT_FONTSIZE + CONTAINERTEXT_FONTSIZEMARGIN;

		b_ffreset_size.width = 200;
		b_ffreset_size.height = 20;

		b_ffreset_pos.x = center.x - b_ffreset_size.width/2;
		b_ffreset_pos.y = parentSize.height - b_ffreset_size.height - Definitions.INTERCOMPONENT_MARGIN;

		subcontainer1_pos.x = t_ff_pos.x;
		subcontainer1_pos.y = t_ff_pos.y + t_ff_size.height + Definitions.INTERCOMPONENT_MARGIN;

		subcontainer1_size.width = SUBCONTAINER_RADIOBUTTON_WIDTH;
		subcontainer1_size.height = b_ffreset_pos.y - subcontainer1_pos.y - Definitions.INTERCOMPONENT_MARGIN;

		subcontainer2_size.width = (parentSize.width - subcontainer1_size.width - 4*Definitions.INTERCOMPONENT_MARGIN)/2;
		subcontainer2_size.height = subcontainer1_size.height;

		subcontainer3_size.width = subcontainer2_size.width;
		subcontainer3_size.height = subcontainer2_size.height;

		subcontainer3_pos.x = parentSize.width - subcontainer3_size.width - Definitions.INTERCOMPONENT_MARGIN;
		subcontainer3_pos.y = subcontainer1_pos.y;

		subcontainer2_pos.x = subcontainer3_pos.x - subcontainer2_size.width - Definitions.INTERCOMPONENT_MARGIN;
		subcontainer2_pos.y = subcontainer1_pos.y;

		this.jlabel_t_ff.setSize(t_ff_size);
		this.jlabel_t_ff.setLocation(t_ff_pos);

		this.jbutton_b_ffreset.setSize(b_ffreset_size);
		this.jbutton_b_ffreset.setLocation(b_ffreset_pos);

		this.jpanel_container3_subcontainer1.setSize(subcontainer1_size);
		this.jpanel_container3_subcontainer1.setLocation(subcontainer1_pos);

		this.jpanel_container3_subcontainer2.setSize(subcontainer2_size);
		this.jpanel_container3_subcontainer2.setLocation(subcontainer2_pos);

		this.jpanel_container3_subcontainer3.setSize(subcontainer3_size);
		this.jpanel_container3_subcontainer3.setLocation(subcontainer3_pos);

		parentSize = subcontainer2_size;

		center.x = parentSize.width/2;
		center.y = parentSize.height/2;

		t_ffd_pos.x = 0;
		t_ffd_pos.y = 0;

		t_ffd_size.width = parentSize.width - 2*t_ffd_pos.x;
		t_ffd_size.height = CONTAINERTEXT_FONTSIZE + CONTAINERTEXT_FONTSIZEMARGIN;

		tb_ffd_pos.x = t_ffd_pos.x;
		tb_ffd_pos.y = t_ffd_pos.y + t_ffd_size.height + Definitions.INTERCOMPONENT_MARGIN;

		tb_ffd_size.width = t_ffd_size.width;
		tb_ffd_size.height = CONTAINERTEXT_FONTSIZE + CONTAINERTEXT_FONTSIZEMARGIN;

		b_ffd_size.width = t_ffd_size.width;
		b_ffd_size.height = 20;

		b_ffd_pos.x = t_ffd_pos.x;
		b_ffd_pos.y = tb_ffd_pos.y + tb_ffd_size.height + Definitions.INTERCOMPONENT_MARGIN;

		this.jlabel_t_ffdupdate.setSize(t_ffd_size);
		this.jlabel_t_ffdupdate.setLocation(t_ffd_pos);

		this.jtextfield_tb_ffdupdate.setSize(tb_ffd_size);
		this.jtextfield_tb_ffdupdate.setLocation(tb_ffd_pos);

		this.jbutton_b_ffdupdate.setSize(b_ffd_size);
		this.jbutton_b_ffdupdate.setLocation(b_ffd_pos);

		parentSize = subcontainer3_size;

		center.x = parentSize.width/2;
		center.y = parentSize.height/2;

		t_ffa_pos.x = 0;
		t_ffa_pos.y = 0;

		t_ffa_size.width = parentSize.width - 2*t_ffa_pos.x;
		t_ffa_size.height = CONTAINERTEXT_FONTSIZE + CONTAINERTEXT_FONTSIZEMARGIN;

		tb_ffa_pos.x = t_ffa_pos.x;
		tb_ffa_pos.y = t_ffa_pos.y + t_ffa_size.height + Definitions.INTERCOMPONENT_MARGIN;

		tb_ffa_size.width = t_ffa_size.width;
		tb_ffa_size.height = CONTAINERTEXT_FONTSIZE + CONTAINERTEXT_FONTSIZEMARGIN;

		b_ffa_size.width = t_ffa_size.width;
		b_ffa_size.height = 20;

		b_ffa_pos.x = t_ffa_pos.x;
		b_ffa_pos.y = tb_ffa_pos.y + tb_ffa_size.height + Definitions.INTERCOMPONENT_MARGIN;

		this.jlabel_t_ffaupdate.setSize(t_ffa_size);
		this.jlabel_t_ffaupdate.setLocation(t_ffa_pos);

		this.jtextfield_tb_ffaupdate.setSize(tb_ffa_size);
		this.jtextfield_tb_ffaupdate.setLocation(tb_ffa_pos);

		this.jbutton_b_ffaupdate.setSize(b_ffa_size);
		this.jbutton_b_ffaupdate.setLocation(b_ffa_pos);

		parentSize = subcontainer1_size;

		center.x = parentSize.width/2;
		center.y = parentSize.height/2;

		t_ffsel_pos.x = 0;
		t_ffsel_pos.y = 0;

		t_ffsel_size.width = parentSize.width - 2*t_ffsel_pos.x;
		t_ffsel_size.height = CONTAINERTEXT_FONTSIZE + CONTAINERTEXT_FONTSIZEMARGIN;

		this.jlabel_t_ffsel.setSize(t_ffsel_size);
		this.jlabel_t_ffsel.setLocation(t_ffsel_pos);

		rb_ffsel_size[RB_LENGTH - 1].width = t_ffsel_size.width;
		rb_ffsel_size[RB_LENGTH - 1].height = 20;

		rb_ffsel_pos[RB_LENGTH - 1].x = t_ffsel_pos.x;
		rb_ffsel_pos[RB_LENGTH - 1].y = parentSize.height - rb_ffsel_size[RB_LENGTH - 1].height;

		this.array_jradiobutton_rb_ffsel[RB_LENGTH - 1].setSize(rb_ffsel_size[RB_LENGTH - 1]);
		this.array_jradiobutton_rb_ffsel[RB_LENGTH - 1].setLocation(rb_ffsel_pos[RB_LENGTH - 1]);

		rb_index = RB_LENGTH - 2;
		while(rb_index >= 0)
		{
			rb_ffsel_size[rb_index].width = rb_ffsel_size[rb_index + 1].width;
			rb_ffsel_size[rb_index].height = rb_ffsel_size[rb_index + 1].height;

			rb_ffsel_pos[rb_index].x = rb_ffsel_pos[rb_index + 1].x;
			rb_ffsel_pos[rb_index].y = rb_ffsel_pos[rb_index + 1].y - rb_ffsel_size[rb_index].height;

			this.array_jradiobutton_rb_ffsel[rb_index].setSize(rb_ffsel_size[rb_index]);
			this.array_jradiobutton_rb_ffsel[rb_index].setLocation(rb_ffsel_pos[rb_index]);

			rb_index--;
		}
	}

	private void alignContainer4()
	{
		Dimension parentSize = this.jpanel_container4.getSize();

		Point center = new Point();

		Point subcontainer1_pos = new Point();
		Dimension subcontainer1_size = new Dimension();

		Point subcontainer2_pos = new Point();
		Dimension subcontainer2_size = new Dimension();

		Point subcontainer3_pos = new Point();
		Dimension subcontainer3_size = new Dimension();

		Point t_fb_pos = new Point();
		Dimension t_fb_size = new Dimension();

		Point b_fbreset_pos = new Point();
		Dimension b_fbreset_size = new Dimension();

		Point t_fbd_pos = new Point();
		Dimension t_fbd_size = new Dimension();

		Point tb_fbd_pos = new Point();
		Dimension tb_fbd_size = new Dimension();

		Point b_fbd_pos = new Point();
		Dimension b_fbd_size = new Dimension();

		Point t_fba_pos = new Point();
		Dimension t_fba_size = new Dimension();

		Point tb_fba_pos = new Point();
		Dimension tb_fba_size = new Dimension();

		Point b_fba_pos = new Point();
		Dimension b_fba_size = new Dimension();

		Point t_fbsel_pos = new Point();
		Dimension t_fbsel_size = new Dimension();

		Point[] rb_fbsel_pos = new Point[this.array_jradiobutton_rb_fbsel.length];
		Dimension[] rb_fbsel_size = new Dimension[this.array_jradiobutton_rb_fbsel.length];

		final int RB_LENGTH = this.array_jradiobutton_rb_fbsel.length;
		int rb_index = 0;

		for(rb_index = 0; rb_index < RB_LENGTH; rb_index++)
		{
			rb_fbsel_pos[rb_index] = new Point();
			rb_fbsel_size[rb_index] = new Dimension();
		}

		center.x = parentSize.width/2;
		center.y = parentSize.height/2;

		t_fb_pos.x = Definitions.INTERCOMPONENT_MARGIN;
		t_fb_pos.y = Definitions.INTERCOMPONENT_MARGIN;

		t_fb_size.width = parentSize.width - 2*t_fb_pos.x;
		t_fb_size.height = CONTAINERTEXT_FONTSIZE + CONTAINERTEXT_FONTSIZEMARGIN;

		b_fbreset_size.width = 200;
		b_fbreset_size.height = 20;

		b_fbreset_pos.x = center.x - b_fbreset_size.width/2;
		b_fbreset_pos.y = parentSize.height - b_fbreset_size.height - Definitions.INTERCOMPONENT_MARGIN;

		subcontainer1_pos.x = t_fb_pos.x;
		subcontainer1_pos.y = t_fb_pos.y + t_fb_size.height + Definitions.INTERCOMPONENT_MARGIN;

		subcontainer1_size.width = SUBCONTAINER_RADIOBUTTON_WIDTH;
		subcontainer1_size.height = b_fbreset_pos.y - subcontainer1_pos.y - Definitions.INTERCOMPONENT_MARGIN;

		subcontainer2_size.width = (parentSize.width - subcontainer1_size.width - 4*Definitions.INTERCOMPONENT_MARGIN)/2;
		subcontainer2_size.height = subcontainer1_size.height;

		subcontainer3_size.width = subcontainer2_size.width;
		subcontainer3_size.height = subcontainer2_size.height;

		subcontainer3_pos.x = parentSize.width - subcontainer3_size.width - Definitions.INTERCOMPONENT_MARGIN;
		subcontainer3_pos.y = subcontainer1_pos.y;

		subcontainer2_pos.x = subcontainer3_pos.x - subcontainer2_size.width - Definitions.INTERCOMPONENT_MARGIN;
		subcontainer2_pos.y = subcontainer1_pos.y;

		this.jlabel_t_fb.setSize(t_fb_size);
		this.jlabel_t_fb.setLocation(t_fb_pos);

		this.jbutton_b_fbreset.setSize(b_fbreset_size);
		this.jbutton_b_fbreset.setLocation(b_fbreset_pos);

		this.jpanel_container4_subcontainer1.setSize(subcontainer1_size);
		this.jpanel_container4_subcontainer1.setLocation(subcontainer1_pos);

		this.jpanel_container4_subcontainer2.setSize(subcontainer2_size);
		this.jpanel_container4_subcontainer2.setLocation(subcontainer2_pos);

		this.jpanel_container4_subcontainer3.setSize(subcontainer3_size);
		this.jpanel_container4_subcontainer3.setLocation(subcontainer3_pos);

		parentSize = subcontainer2_size;

		center.x = parentSize.width/2;
		center.y = parentSize.height/2;

		t_fbd_pos.x = 0;
		t_fbd_pos.y = 0;

		t_fbd_size.width = parentSize.width - 2*t_fbd_pos.x;
		t_fbd_size.height = CONTAINERTEXT_FONTSIZE + CONTAINERTEXT_FONTSIZEMARGIN;

		tb_fbd_pos.x = t_fbd_pos.x;
		tb_fbd_pos.y = t_fbd_pos.y + t_fbd_size.height + Definitions.INTERCOMPONENT_MARGIN;

		tb_fbd_size.width = t_fbd_size.width;
		tb_fbd_size.height = CONTAINERTEXT_FONTSIZE + CONTAINERTEXT_FONTSIZEMARGIN;

		b_fbd_size.width = t_fbd_size.width;
		b_fbd_size.height = 20;

		b_fbd_pos.x = t_fbd_pos.x;
		b_fbd_pos.y = tb_fbd_pos.y + tb_fbd_size.height + Definitions.INTERCOMPONENT_MARGIN;

		this.jlabel_t_fbdupdate.setSize(t_fbd_size);
		this.jlabel_t_fbdupdate.setLocation(t_fbd_pos);

		this.jtextfield_tb_fbdupdate.setSize(tb_fbd_size);
		this.jtextfield_tb_fbdupdate.setLocation(tb_fbd_pos);

		this.jbutton_b_fbdupdate.setSize(b_fbd_size);
		this.jbutton_b_fbdupdate.setLocation(b_fbd_pos);

		parentSize = subcontainer3_size;

		center.x = parentSize.width/2;
		center.y = parentSize.height/2;

		t_fba_pos.x = 0;
		t_fba_pos.y = 0;

		t_fba_size.width = parentSize.width - 2*t_fba_pos.x;
		t_fba_size.height = CONTAINERTEXT_FONTSIZE + CONTAINERTEXT_FONTSIZEMARGIN;

		tb_fba_pos.x = t_fba_pos.x;
		tb_fba_pos.y = t_fba_pos.y + t_fba_size.height + Definitions.INTERCOMPONENT_MARGIN;

		tb_fba_size.width = t_fba_size.width;
		tb_fba_size.height = CONTAINERTEXT_FONTSIZE + CONTAINERTEXT_FONTSIZEMARGIN;

		b_fba_size.width = t_fba_size.width;
		b_fba_size.height = 20;

		b_fba_pos.x = t_fba_pos.x;
		b_fba_pos.y = tb_fba_pos.y + tb_fba_size.height + Definitions.INTERCOMPONENT_MARGIN;

		this.jlabel_t_fbaupdate.setSize(t_fba_size);
		this.jlabel_t_fbaupdate.setLocation(t_fba_pos);

		this.jtextfield_tb_fbaupdate.setSize(tb_fba_size);
		this.jtextfield_tb_fbaupdate.setLocation(tb_fba_pos);

		this.jbutton_b_fbaupdate.setSize(b_fba_size);
		this.jbutton_b_fbaupdate.setLocation(b_fba_pos);

		parentSize = subcontainer1_size;

		center.x = parentSize.width/2;
		center.y = parentSize.height/2;

		t_fbsel_pos.x = 0;
		t_fbsel_pos.y = 0;

		t_fbsel_size.width = parentSize.width - 2*t_fbsel_pos.x;
		t_fbsel_size.height = CONTAINERTEXT_FONTSIZE + CONTAINERTEXT_FONTSIZEMARGIN;

		this.jlabel_t_fbsel.setSize(t_fbsel_size);
		this.jlabel_t_fbsel.setLocation(t_fbsel_pos);

		rb_fbsel_size[RB_LENGTH - 1].width = t_fbsel_size.width;
		rb_fbsel_size[RB_LENGTH - 1].height = 20;

		rb_fbsel_pos[RB_LENGTH - 1].x = t_fbsel_pos.x;
		rb_fbsel_pos[RB_LENGTH - 1].y = parentSize.height - rb_fbsel_size[RB_LENGTH - 1].height;

		this.array_jradiobutton_rb_fbsel[RB_LENGTH - 1].setSize(rb_fbsel_size[RB_LENGTH - 1]);
		this.array_jradiobutton_rb_fbsel[RB_LENGTH - 1].setLocation(rb_fbsel_pos[RB_LENGTH - 1]);

		rb_index = RB_LENGTH - 2;
		while(rb_index >= 0)
		{
			rb_fbsel_size[rb_index].width = rb_fbsel_size[rb_index + 1].width;
			rb_fbsel_size[rb_index].height = rb_fbsel_size[rb_index + 1].height;

			rb_fbsel_pos[rb_index].x = rb_fbsel_pos[rb_index + 1].x;
			rb_fbsel_pos[rb_index].y = rb_fbsel_pos[rb_index + 1].y - rb_fbsel_size[rb_index].height;

			this.array_jradiobutton_rb_fbsel[rb_index].setSize(rb_fbsel_size[rb_index]);
			this.array_jradiobutton_rb_fbsel[rb_index].setLocation(rb_fbsel_pos[rb_index]);

			rb_index--;
		}
	}

	private void setupTextDefault(JLabel text)
	{
		text.setForeground(Definitions.TEXT_FOREGROUNDCOLOR);
		text.setLayout(this.layout);
		text.setHorizontalAlignment(JLabel.CENTER);
	}

	private void setupTextDefault(JTextArea text)
	{
		text.setForeground(Definitions.TEXT_FOREGROUNDCOLOR);
		text.setLayout(this.layout);
		text.setEditable(false);
		text.setFocusable(false);
		text.setOpaque(false);
		text.setCursor(null);
		text.setVisible(true);
	}

	private void setupTextBoxDefault(JTextField textbox)
	{
		textbox.setBackground(Definitions.TEXTBOX_BACKGROUNDCOLOR);
		textbox.setForeground(Definitions.TEXTBOX_FOREGROUNDCOLOR);
		textbox.setLayout(this.layout);
		textbox.setHorizontalAlignment(JTextField.CENTER);
		textbox.setEditable(true);
		textbox.setFocusable(true);
		textbox.setOpaque(true);
		textbox.setVisible(true);
	}

	private void setupButtonDefault(JButton button)
	{
		button.setBackground(Definitions.BUTTON_BACKGROUNDCOLOR);
		button.setForeground(Definitions.BUTTON_FOREGROUNDCOLOR);
		button.setLayout(this.layout);
		button.addActionListener(this.actionListener);
		button.setFocusable(true);
		button.setVisible(true);
	}

	private void setupContainerDefault(JPanel container, boolean opaque)
	{
		container.setLayout(this.layout);
		container.setOpaque(opaque);
		container.setVisible(true);
	}

	private void loadUpdateParams()
	{
		int nfx = 0;

		String paramsin = "";
		String paramsout = "";

		paramsin = "Dry Input Amplitude: ";
		paramsin += String.valueOf(CPPCore.getDryInputAmplitude());
		paramsin += "\n";

		for(nfx = 0; nfx < CPPCore.RTDELAY_N_FF_DELAYS; nfx++)
		{
			paramsin += "\nFF Channel ";
			paramsin += String.valueOf(nfx + 1);
			paramsin += ":\tDelay Time: ";
			paramsin += String.valueOf(CPPCore.getFFDelay(nfx));
			paramsin += " samples\tAmplitude: ";
			paramsin += String.valueOf(CPPCore.getFFAmplitude(nfx));
		}

		paramsout = "Output Amplitude: ";
		paramsout += String.valueOf(CPPCore.getOutputAmplitude());
		paramsout += "\n";

		for(nfx = 0; nfx < CPPCore.RTDELAY_N_FB_DELAYS; nfx++)
		{
			paramsout += "\nFB Channel ";
			paramsout += String.valueOf(nfx + 1);
			paramsout += ":\tDelay Time: ";
			paramsout += String.valueOf(CPPCore.getFBDelay(nfx));
			paramsout += " samples\tAmplitude: ";
			paramsout += String.valueOf(CPPCore.getFBAmplitude(nfx));
		}

		this.jtextarea_paramsin.setText(paramsin);
		this.jtextarea_paramsout.setText(paramsout);
	}

	private void attemptUpdateParam(int param, JTextField textbox)
	{
		float fval = 0.0f;
		int ival = 0;
		int nFX = 0;

		boolean b_ret = false;

		switch(param)
		{
			case ATTEMPTUPDATEPARAM_FFDELAY:
			case ATTEMPTUPDATEPARAM_FFAMP:

				nFX = this.getFFSel();
				if(nFX < 0)
				{
					JOptionPane.showMessageDialog(new JFrame(), "Error: no feedforward channel selected", "ERROR", JOptionPane.ERROR_MESSAGE);
					return;
				}

				break;

			case ATTEMPTUPDATEPARAM_FBDELAY:
			case ATTEMPTUPDATEPARAM_FBAMP:

				nFX = this.getFBSel();
				if(nFX < 0)
				{
					JOptionPane.showMessageDialog(new JFrame(), "Error: no feedback channel selected", "ERROR", JOptionPane.ERROR_MESSAGE);
					return;
				}

				break;
		}

		switch(param)
		{
			case ATTEMPTUPDATEPARAM_DRYINAMP:
			case ATTEMPTUPDATEPARAM_OUTAMP:
			case ATTEMPTUPDATEPARAM_FFAMP:
			case ATTEMPTUPDATEPARAM_FBAMP:

				try
				{
					fval = Float.parseFloat(textbox.getText());
				}
				catch(Exception e)
				{
					JOptionPane.showMessageDialog(new JFrame(), "Error: invalid amplitude value entered", "ERROR", JOptionPane.ERROR_MESSAGE);
					return;
				}

				break;

			case ATTEMPTUPDATEPARAM_FFDELAY:
			case ATTEMPTUPDATEPARAM_FBDELAY:

				try
				{
					ival = Integer.parseInt(textbox.getText());
				}
				catch(Exception e)
				{
					JOptionPane.showMessageDialog(new JFrame(), "Error: invalid delay value entered", "ERROR", JOptionPane.ERROR_MESSAGE);
					return;
				}

				if(ival < 0)
				{
					JOptionPane.showMessageDialog(new JFrame(), "Error: invalid delay value entered", "ERROR", JOptionPane.ERROR_MESSAGE);
					return;
				}

				break;
		}

		switch(param)
		{
			case ATTEMPTUPDATEPARAM_DRYINAMP:
				b_ret = CPPCore.setDryInputAmplitude(fval);
				break;

			case ATTEMPTUPDATEPARAM_OUTAMP:
				b_ret = CPPCore.setOutputAmplitude(fval);
				break;

			case ATTEMPTUPDATEPARAM_FFDELAY:
				b_ret = CPPCore.setFFDelay(nFX, ival);
				break;

			case ATTEMPTUPDATEPARAM_FFAMP:
				b_ret = CPPCore.setFFAmplitude(nFX, fval);
				break;

			case ATTEMPTUPDATEPARAM_FBDELAY:
				b_ret = CPPCore.setFBDelay(nFX, ival);
				break;

			case ATTEMPTUPDATEPARAM_FBAMP:
				b_ret = CPPCore.setFBAmplitude(nFX, fval);
				break;
		}

		if(b_ret) this.loadUpdateParams();
		else JOptionPane.showMessageDialog(new JFrame(), CPPCore.getLastErrorMessage(), "ERROR", JOptionPane.ERROR_MESSAGE);
	}

	private int getFFSel()
	{
		final int RB_LENGTH = this.array_jradiobutton_rb_ffsel.length;
		int rb_index = 0;

		for(rb_index = 0; rb_index < RB_LENGTH; rb_index++)
			if(this.array_jradiobutton_rb_ffsel[rb_index].isSelected()) 
				return rb_index;

		return -1;
	}

	private int getFBSel()
	{
		final int RB_LENGTH = this.array_jradiobutton_rb_fbsel.length;
		int rb_index = 0;

		for(rb_index = 0; rb_index < RB_LENGTH; rb_index++)
			if(this.array_jradiobutton_rb_fbsel[rb_index].isSelected())
				return rb_index;

		return -1;
	}
}

