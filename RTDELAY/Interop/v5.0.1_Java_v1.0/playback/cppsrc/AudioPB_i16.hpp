/*
 * Real-Time Audio Delay 2 for GNU-Linux systems.
 * Version 5.0.1
 *
 * Author: Rafael Sabe
 * Email: rafaelmsabe@gmail.com
 */

#ifndef AUDIOPB_I16_HPP
#define AUDIOPB_I16_HPP

#include "AudioPB.hpp"

class AudioPB_i16 : public AudioPB {
	public:
		AudioPB_i16(const audiopb_params_t *p_params);
		~AudioPB_i16(void);

	protected:
		static constexpr float SAMPLE_FACTOR = 32768.0f;

		void delaybuffer_loadin(void) override;
		void delaybuffer_loadout(void) override;
};

#endif /*AUDIOPB_I16_HPP*/

