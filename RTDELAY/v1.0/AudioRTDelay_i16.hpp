/*
 * Real-Time Audio Delay 2 for GNU-Linux systems.
 * Version 1.0
 *
 * Author: Rafael Sabe
 * Email: rafaelmsabe@gmail.com
 */

#ifndef AUDIORTDELAY_I16_HPP
#define AUDIORTDELAY_I16_HPP

#include "AudioRTDelay.hpp"

class AudioRTDelay_i16 : public AudioRTDelay {
	public:
		AudioRTDelay_i16(const audiortdelay_init_params_t *p_params);
		~AudioRTDelay_i16(void);

	protected:
		static constexpr int16_t SAMPLE_MAX_VALUE = 0x7fff;
		static constexpr int16_t SAMPLE_MIN_VALUE = -0x8000;

		static constexpr float SAMPLE_MAX_VALUE_F = 32767.0f;
		static constexpr float SAMPLE_MIN_VALUE_F = -32768.0f;

		bool audio_hw_init(void) override;
		bool buffer_alloc(void) override;
		void buffer_free(void) override;
		void buffer_load(void) override;
		void run_dsp(void) override;
};

#endif /*AUDIORTDELAY_I16_HPP*/

