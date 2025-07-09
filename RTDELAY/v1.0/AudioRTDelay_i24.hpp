/*
 * Real-Time Audio Delay 2 for GNU-Linux systems.
 * Version 1.0
 *
 * Author: Rafael Sabe
 * Email: rafaelmsabe@gmail.com
 */

#ifndef AUDIORTDELAY_I24_HPP
#define AUDIORTDELAY_I24_HPP

#include "AudioRTDelay.hpp"

class AudioRTDelay_i24 : public AudioRTDelay {
	public:
		AudioRTDelay_i24(const audiortdelay_init_params_t *p_params);
		~AudioRTDelay_i24(void);

	protected:
		static constexpr int32_t SAMPLE_MAX_VALUE = 0x7fffff;
		static constexpr int32_t SAMPLE_MIN_VALUE = -0x800000;

		static constexpr float SAMPLE_MAX_VALUE_F = 8388607.0f;
		static constexpr float SAMPLE_MIN_VALUE_F = -8388608.0f;

		size_t BYTEBUF_SIZE = 0u;

		uint8_t *p_bytebuf = NULL;

		bool audio_hw_init(void) override;
		bool buffer_alloc(void) override;
		void buffer_free(void) override;
		void buffer_load(void) override;
		void run_dsp(void) override;
};

#endif /*AUDIORTDELAY_I24_HPP*/

