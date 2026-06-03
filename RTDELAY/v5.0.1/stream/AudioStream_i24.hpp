/*
 * Real-Time Audio Delay 2 for GNU-Linux systems.
 * Version 5.0.1
 *
 * Author: Rafael Sabe
 * Email: rafaelmsabe@gmail.com
 */

#ifndef AUDIOSTREAM_I24_HPP
#define AUDIOSTREAM_I24_HPP

#include "AudioStream.hpp"

class AudioStream_i24 : public AudioStream {
	public:
		AudioStream_i24(const audiostream_params_t *p_params);
		~AudioStream_i24(void);

	private:
		static constexpr float SAMPLE_FACTOR = 8388608.0f;

		void delaybuffer_loadin(void) override;
		void delaybuffer_loadout(void) override;
};

#endif /*AUDIOSTREAM_I24_HPP*/

