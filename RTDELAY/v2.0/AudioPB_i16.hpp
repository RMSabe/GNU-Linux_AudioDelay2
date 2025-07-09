/*
 * Real-Time Audio Delay 2 for GNU-Linux systems.
 * Version 2.0
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

		bool audio_hw_init(void) override;
		bool buffer_alloc(void) override;
		void buffer_free(void) override;
		void buffer_load_in(void) override;
		void buffer_load_out(void) override;
};

#endif /*AUDIOPB_I16_HPP*/

