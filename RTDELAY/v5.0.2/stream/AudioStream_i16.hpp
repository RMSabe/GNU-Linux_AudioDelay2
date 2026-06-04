/*
 * Real-Time Audio Delay 2 for GNU-Linux systems.
 * Version 5.0.2
 *
 * Author: Rafael Sabe
 * Email: rafaelmsabe@gmail.com
 */

#ifndef AUDIOSTREAM_I16_HPP
#define AUDIOSTREAM_I16_HPP

#include "AudioStream.hpp"

class AudioStream_i16 : public AudioStream {
	public:
		AudioStream_i16(const audiostream_params_t *p_params);
		~AudioStream_i16(void);

	private:
		static constexpr float SAMPLE_FACTOR = 32768.0f;

		void delaybuffer_loadin(void) override;
		void delaybuffer_loadout(void) override;
};

#endif /*AUDIOSTREAM_I16_HPP*/

