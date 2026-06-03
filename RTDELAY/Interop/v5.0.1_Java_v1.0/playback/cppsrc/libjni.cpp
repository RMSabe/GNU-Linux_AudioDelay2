/*
 * Real-Time Audio Delay 2 for GNU-Linux systems.
 * Version 5.0.1 (File Playback Version. Interop Java version 1.0)
 *
 * Author: Rafael Sabe
 * Email: rafaelmsabe@gmail.com
 */

#include "libcore.hpp"
#include <jni.h>

#ifdef __TEXTFORMAT_USE_WCHAR
#if __SIZEOF_WCHAR_T__ == 4
static __attribute__((__aligned__(PTR_SIZE_BITS))) uint16_t _jchar_textbuf[TEXTBUF_SIZE_CHARS];
#endif
#endif

#define __CPPCORE_AUDIO_STREAMBUFFER_N_SEGMENTS_FIELDNAME "AUDIO_STREAMBUFFER_N_SEGMENTS"
#define __CPPCORE_AUDIO_DELAY_BUFFER_SIZE_FRAMES_FIELDNAME "AUDIO_DELAY_BUFFER_SIZE_FRAMES"
#define __CPPCORE_AUDIO_DELAY_N_FFCH_FIELDNAME "AUDIO_DELAY_N_FFCH"
#define __CPPCORE_AUDIO_DELAY_N_FBCH_FIELDNAME "AUDIO_DELAY_N_FBCH"

__EXTERNC__ JNIEXPORT jboolean JNICALL Java_CPPCore_initialize(JNIEnv *p_jnienv, jclass jcls);
__EXTERNC__ JNIEXPORT void JNICALL Java_CPPCore_deinitialize(JNIEnv *p_jnienv, jclass jcls);
__EXTERNC__ JNIEXPORT jstring JNICALL Java_CPPCore_getLastErrorMessage(JNIEnv *p_jnienv, jclass jcls);
__EXTERNC__ JNIEXPORT jboolean JNICALL Java_CPPCore_setFileInDirectory(JNIEnv *p_jnienv, jclass jcls, jstring fileDirectory);
__EXTERNC__ JNIEXPORT jboolean JNICALL Java_CPPCore_loadFile_1createAudioObject(JNIEnv *p_jnienv, jclass jcls);
__EXTERNC__ JNIEXPORT jboolean JNICALL Java_CPPCore_loadAudioDeviceList(JNIEnv *p_jnienv, jclass jcls);
__EXTERNC__ JNIEXPORT jint JNICALL Java_CPPCore_getAudioDeviceListEntryCount(JNIEnv *p_jnienv, jclass jcls);
__EXTERNC__ JNIEXPORT jstring JNICALL Java_CPPCore_getAudioDeviceListEntry_1friendlyName(JNIEnv *p_jnienv, jclass jcls, jint index);
__EXTERNC__ JNIEXPORT jboolean JNICALL Java_CPPCore_chooseDevice(JNIEnv *p_jnienv, jclass jcls, jint index);
__EXTERNC__ JNIEXPORT jboolean JNICALL Java_CPPCore_chooseDefaultDevice(JNIEnv *p_jnienv, jclass jcls);
__EXTERNC__ JNIEXPORT jboolean JNICALL Java_CPPCore_initializeAudioObject(JNIEnv *p_jnienv, jclass jcls);
__EXTERNC__ JNIEXPORT jint JNICALL Java_CPPCore_getStatus(JNIEnv *p_jnienv, jclass jcls);
__EXTERNC__ JNIEXPORT jboolean JNICALL Java_CPPCore_runPlayback(JNIEnv *p_jnienv, jclass jcls);
__EXTERNC__ JNIEXPORT void JNICALL Java_CPPCore_pausePlayback(JNIEnv *p_jnienv, jclass jcls);
__EXTERNC__ JNIEXPORT void JNICALL Java_CPPCore_resumePlayback(JNIEnv *p_jnienv, jclass jcls);
__EXTERNC__ JNIEXPORT void JNICALL Java_CPPCore_stopPlayback(JNIEnv *p_jnienv, jclass jcls);
__EXTERNC__ JNIEXPORT jlong JNICALL Java_CPPCore_getAudioDataSizeFrames(JNIEnv *p_jnienv, jclass jcls);
__EXTERNC__ JNIEXPORT jlong JNICALL Java_CPPCore_getAudioDataPositionFrames(JNIEnv *p_jnienv, jclass jcls);
__EXTERNC__ JNIEXPORT jboolean JNICALL Java_CPPCore_setAudioDataPositionFrames(JNIEnv *p_jnienv, jclass jcls, jlong position);
__EXTERNC__ JNIEXPORT jfloat JNICALL Java_CPPCore_getDryInputAmplitude(JNIEnv *p_jnienv, jclass jcls);
__EXTERNC__ JNIEXPORT jboolean JNICALL Java_CPPCore_setDryInputAmplitude(JNIEnv *p_jnienv, jclass jcls, jfloat amp);
__EXTERNC__ JNIEXPORT jfloat JNICALL Java_CPPCore_getOutputAmplitude(JNIEnv *p_jnienv, jclass jcls);
__EXTERNC__ JNIEXPORT jboolean JNICALL Java_CPPCore_setOutputAmplitude(JNIEnv *p_jnienv, jclass jcls, jfloat amp);
__EXTERNC__ JNIEXPORT jint JNICALL Java_CPPCore_getFFDelay(JNIEnv *p_jnienv, jclass jcls, jint nFX);
__EXTERNC__ JNIEXPORT jboolean JNICALL Java_CPPCore_setFFDelay(JNIEnv *p_jnienv, jclass jcls, jint nFX, jint delay);
__EXTERNC__ JNIEXPORT jfloat JNICALL Java_CPPCore_getFFAmplitude(JNIEnv *p_jnienv, jclass jcls, jint nFX);
__EXTERNC__ JNIEXPORT jboolean JNICALL Java_CPPCore_setFFAmplitude(JNIEnv *p_jnienv, jclass jcls, jint nFX, jfloat amp);
__EXTERNC__ JNIEXPORT jint JNICALL Java_CPPCore_getFBDelay(JNIEnv *p_jnienv, jclass jcls, jint nFX);
__EXTERNC__ JNIEXPORT jboolean JNICALL Java_CPPCore_setFBDelay(JNIEnv *p_jnienv, jclass jcls, jint nFX, jint delay);
__EXTERNC__ JNIEXPORT jfloat JNICALL Java_CPPCore_getFBAmplitude(JNIEnv *p_jnienv, jclass jcls, jint nFX);
__EXTERNC__ JNIEXPORT jboolean JNICALL Java_CPPCore_setFBAmplitude(JNIEnv *p_jnienv, jclass jcls, jint nFX, jfloat amp);
__EXTERNC__ JNIEXPORT jboolean JNICALL Java_CPPCore_resetFFParams(JNIEnv *p_jnienv, jclass jcls);
__EXTERNC__ JNIEXPORT jboolean JNICALL Java_CPPCore_resetFBParams(JNIEnv *p_jnienv, jclass jcls);

static jstring libjni_tstr_to_jstr(JNIEnv *p_jnienv, const __tchar_t *tstr);

JNIEXPORT jboolean JNICALL Java_CPPCore_initialize(JNIEnv *p_jnienv, jclass jcls)
{
	jfieldID _jfieldid;

	_jfieldid = p_jnienv->GetStaticFieldID(jcls, __CPPCORE_AUDIO_STREAMBUFFER_N_SEGMENTS_FIELDNAME, "I");
	if(_jfieldid == NULL) goto _l_Java_CPPCore_initialize_error;

	__AUDIO_STREAMBUFFER_N_SEGMENTS = (size_t) p_jnienv->GetStaticIntField(jcls, _jfieldid);

	_jfieldid = p_jnienv->GetStaticFieldID(jcls, __CPPCORE_AUDIO_DELAY_BUFFER_SIZE_FRAMES_FIELDNAME, "I");
	if(_jfieldid == NULL) goto _l_Java_CPPCore_initialize_error;

	__AUDIO_DELAY_BUFFER_SIZE_FRAMES = (size_t) p_jnienv->GetStaticIntField(jcls, _jfieldid);

	_jfieldid = p_jnienv->GetStaticFieldID(jcls, __CPPCORE_AUDIO_DELAY_N_FFCH_FIELDNAME, "I");
	if(_jfieldid == NULL) goto _l_Java_CPPCore_initialize_error;

	__AUDIO_DELAY_N_FFCH = (size_t) p_jnienv->GetStaticIntField(jcls, _jfieldid);

	_jfieldid = p_jnienv->GetStaticFieldID(jcls, __CPPCORE_AUDIO_DELAY_N_FBCH_FIELDNAME, "I");
	if(_jfieldid == NULL) goto _l_Java_CPPCore_initialize_error;

	__AUDIO_DELAY_N_FBCH = (size_t) p_jnienv->GetStaticIntField(jcls, _jfieldid);

	return (jboolean) core_init();

_l_Java_CPPCore_initialize_error:

	err_msg = __TEXT("Java_CPPCore_initialize: Error: failed to load CPPCore class parameters.");
	return JNI_FALSE;
}

JNIEXPORT void JNICALL Java_CPPCore_deinitialize(JNIEnv *p_jnienv, jclass jcls)
{
	core_deinit();
	return;
}

JNIEXPORT jstring JNICALL Java_CPPCore_getLastErrorMessage(JNIEnv *p_jnienv, jclass jcls)
{
	return libjni_tstr_to_jstr(p_jnienv, err_msg.c_str());
}

JNIEXPORT jboolean JNICALL Java_CPPCore_setFileInDirectory(JNIEnv *p_jnienv, jclass jcls, jstring fileDirectory)
{
	const char *fdir = NULL;
	bool b_ret;

	fdir = p_jnienv->GetStringUTFChars(fileDirectory, NULL);

	b_ret = set_filein_dir(fdir);

	p_jnienv->ReleaseStringUTFChars(fileDirectory, fdir);

	return (jboolean) b_ret;
}

JNIEXPORT jboolean JNICALL Java_CPPCore_loadFile_1createAudioObject(JNIEnv *p_jnienv, jclass jcls)
{
	return (jboolean) loadfile_createaudioobject();
}

JNIEXPORT jboolean JNICALL Java_CPPCore_loadAudioDeviceList(JNIEnv *p_jnienv, jclass jcls)
{
	return (jboolean) load_audiodevicelist();
}

JNIEXPORT jint JNICALL Java_CPPCore_getAudioDeviceListEntryCount(JNIEnv *p_jnienv, jclass jcls)
{
	return (jint) audiodevicelist_get_entry_count();
}

JNIEXPORT jstring JNICALL Java_CPPCore_getAudioDeviceListEntry_1friendlyName(JNIEnv *p_jnienv, jclass jcls, jint index)
{
	const char *textinfo = NULL;

	textinfo = audiodevicelist_get_entry_info((size_t) index, true);

	if(textinfo == NULL) return p_jnienv->NewStringUTF("");

	return p_jnienv->NewStringUTF(textinfo);
}

JNIEXPORT jboolean JNICALL Java_CPPCore_chooseDevice(JNIEnv *p_jnienv, jclass jcls, jint index)
{
	return (jboolean) choose_device((size_t) index, false);
}

JNIEXPORT jboolean JNICALL Java_CPPCore_chooseDefaultDevice(JNIEnv *p_jnienv, jclass jcls)
{
	return (jboolean) choose_device(0u, true);
}

JNIEXPORT jboolean JNICALL Java_CPPCore_initializeAudioObject(JNIEnv *p_jnienv, jclass jcls)
{
	return (jboolean) audioobject_init();
}

JNIEXPORT jint JNICALL Java_CPPCore_getStatus(JNIEnv *p_jnienv, jclass jcls)
{
	return (jint) audioobject_getstatus();
}

JNIEXPORT jboolean JNICALL Java_CPPCore_runPlayback(JNIEnv *p_jnienv, jclass jcls)
{
	return (jboolean) run_playback();
}

JNIEXPORT void JNICALL Java_CPPCore_pausePlayback(JNIEnv *p_jnienv, jclass jcls)
{
	pause_playback();
	return;
}

JNIEXPORT void JNICALL Java_CPPCore_resumePlayback(JNIEnv *p_jnienv, jclass jcls)
{
	resume_playback();
	return;
}

JNIEXPORT void JNICALL Java_CPPCore_stopPlayback(JNIEnv *p_jnienv, jclass jcls)
{
	stop_playback();
	return;
}

JNIEXPORT jlong JNICALL Java_CPPCore_getAudioDataSizeFrames(JNIEnv *p_jnienv, jclass jcls)
{
	return (jlong) audiodata_get_size_frames();
}

JNIEXPORT jlong JNICALL Java_CPPCore_getAudioDataPositionFrames(JNIEnv *p_jnienv, jclass jcls)
{
	return (jlong) audiodata_get_position_frames();
}

JNIEXPORT jboolean JNICALL Java_CPPCore_setAudioDataPositionFrames(JNIEnv *p_jnienv, jclass jcls, jlong position)
{
	return (jboolean) audiodata_set_position_frames((__offset_t) position);
}

JNIEXPORT jfloat JNICALL Java_CPPCore_getDryInputAmplitude(JNIEnv *p_jnienv, jclass jcls)
{
	return (jfloat) delay_getDryInputAmplitude();
}

JNIEXPORT jboolean JNICALL Java_CPPCore_setDryInputAmplitude(JNIEnv *p_jnienv, jclass jcls, jfloat amp)
{
	return (jboolean) delay_setDryInputAmplitude((float) amp);
}

JNIEXPORT jfloat JNICALL Java_CPPCore_getOutputAmplitude(JNIEnv *p_jnienv, jclass jcls)
{
	return (jfloat) delay_getOutputAmplitude();
}

JNIEXPORT jboolean JNICALL Java_CPPCore_setOutputAmplitude(JNIEnv *p_jnienv, jclass jcls, jfloat amp)
{
	return (jboolean) delay_setOutputAmplitude((float) amp);
}

JNIEXPORT jint JNICALL Java_CPPCore_getFFDelay(JNIEnv *p_jnienv, jclass jcls, jint nFX)
{
	return (jint) delay_getFFDelay((size_t) nFX);
}

JNIEXPORT jboolean JNICALL Java_CPPCore_setFFDelay(JNIEnv *p_jnienv, jclass jcls, jint nFX, jint delay)
{
	return (jboolean) delay_setFFDelay((size_t) nFX, (uint32_t) delay);
}

JNIEXPORT jfloat JNICALL Java_CPPCore_getFFAmplitude(JNIEnv *p_jnienv, jclass jcls, jint nFX)
{
	return (jfloat) delay_getFFAmplitude((size_t) nFX);
}

JNIEXPORT jboolean JNICALL Java_CPPCore_setFFAmplitude(JNIEnv *p_jnienv, jclass jcls, jint nFX, jfloat amp)
{
	return (jboolean) delay_setFFAmplitude((size_t) nFX, (float) amp);
}

JNIEXPORT jint JNICALL Java_CPPCore_getFBDelay(JNIEnv *p_jnienv, jclass jcls, jint nFX)
{
	return (jint) delay_getFBDelay((size_t) nFX);
}

JNIEXPORT jboolean JNICALL Java_CPPCore_setFBDelay(JNIEnv *p_jnienv, jclass jcls, jint nFX, jint delay)
{
	return (jboolean) delay_setFBDelay((size_t) nFX, (uint32_t) delay);
}

JNIEXPORT jfloat JNICALL Java_CPPCore_getFBAmplitude(JNIEnv *p_jnienv, jclass jcls, jint nFX)
{
	return (jfloat) delay_getFBAmplitude((size_t) nFX);
}

JNIEXPORT jboolean JNICALL Java_CPPCore_setFBAmplitude(JNIEnv *p_jnienv, jclass jcls, jint nFX, jfloat amp)
{
	return (jboolean) delay_setFBAmplitude((size_t) nFX, (float) amp);
}

JNIEXPORT jboolean JNICALL Java_CPPCore_resetFFParams(JNIEnv *p_jnienv, jclass jcls)
{
	return (jboolean) delay_resetFFParams();
}

JNIEXPORT jboolean JNICALL Java_CPPCore_resetFBParams(JNIEnv *p_jnienv, jclass jcls)
{
	return (jboolean) delay_resetFBParams();
}

static jstring libjni_tstr_to_jstr(JNIEnv *p_jnienv, const __tchar_t *tstr)
{
	size_t _len;

	if(p_jnienv == NULL) return (jstring) NULL;

	if(tstr == NULL) return p_jnienv->NewStringUTF("");

#ifdef __TEXTFORMAT_USE_WCHAR

#if __SIZEOF_WCHAR_T__ == 4
	cstr_copy_text32_to_text16((const uint32_t*) tstr, _jchar_textbuf, TEXTBUF_SIZE_CHARS);

	_len = 0u;
	while(_jchar_textbuf[_len] != '\0') _len++;

	return p_jnienv->NewString((const jchar*) _jchar_textbuf, (jsize) _len);
#endif

#if __SIZEOF_WCHAR_T__ == 2
	_len = (size_t) cstr_getlength(tstr);

	return p_jnienv->NewString((const jchar*) tstr, (jsize) _len);
#endif

#if __SIZEOF_WCHAR_T__ == 1
	return p_jnienv->NewStringUTF((const char*) tstr);
#endif

#else
	return p_jnienv->NewStringUTF((const char*) tstr);
#endif
}

