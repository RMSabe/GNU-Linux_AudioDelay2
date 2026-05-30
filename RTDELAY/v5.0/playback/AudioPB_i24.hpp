/*
 * Real-Time Audio Delay 2 for GNU-Linux systems.
 * Version 5.0
 *
 * Author: Rafael Sabe
 * Email: rafaelmsabe@gmail.com
 */

#ifndef AUDIOPB_I24_HPP
#define AUDIOPB_I24_HPP

#include "AudioPB.hpp"

class AudioPB_i24 : public AudioPB {
	public:
		AudioPB_i24(const audiopb_params_t *p_params);
		~AudioPB_i24(void);

	protected:
		static constexpr float SAMPLE_FACTOR = 8388608.0f;

		void delaybuffer_loadin(void) override;
		void delaybuffer_loadout(void) override;
};

#endif /*AUDIOPB_I24_HPP*/

