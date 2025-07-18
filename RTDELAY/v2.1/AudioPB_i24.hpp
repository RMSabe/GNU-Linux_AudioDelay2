/*
 * Real-Time Audio Delay 2 for GNU-Linux systems.
 * Version 2.1
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

		size_t BYTEBUF_SIZE = 0u;

		bool audio_hw_init(void) override;
		bool buffer_alloc(void) override;
		void buffer_free(void) override;
		void buffer_load_in(void) override;
		void buffer_load_out(void) override;
};

#endif /*AUDIOPB_I24_HPP*/

