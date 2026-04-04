/*
 * Real-Time Audio Delay 2 for GNU-Linux systems.
 * Version 3.2 (Interop Java version 1.1)
 *
 * Author: Rafael Sabe
 * Email: rafaelmsabe@gmail.com
 */

#include "libcore.hpp"
#include <jni.h>

extern "C" JNIEXPORT jboolean JNICALL Java_CPPCore_initialize(JNIEnv *p_jnienv, jclass jcls);
extern "C" JNIEXPORT void JNICALL Java_CPPCore_deinitialize(JNIEnv *p_jnienv, jclass jcls);
extern "C" JNIEXPORT jstring JNICALL Java_CPPCore_getLastErrorMessage(JNIEnv *p_jnienv, jclass jcls);
extern "C" JNIEXPORT jboolean JNICALL Java_CPPCore_setFileInDirectory(JNIEnv *p_jnienv, jclass jcls, jstring fileDirectory);
extern "C" JNIEXPORT jboolean JNICALL Java_CPPCore_loadFile_1createAudioObject(JNIEnv *p_jnienv, jclass jcls);
extern "C" JNIEXPORT jboolean JNICALL Java_CPPCore_loadAudioDeviceList(JNIEnv *p_jnienv, jclass jcls);
extern "C" JNIEXPORT jint JNICALL Java_CPPCore_getAudioDeviceListEntryCount(JNIEnv *p_jnienv, jclass jcls);
extern "C" JNIEXPORT jstring JNICALL Java_CPPCore_getAudioDeviceListEntryName(JNIEnv *p_jnienv, jclass jcls, jint index);
extern "C" JNIEXPORT jstring JNICALL Java_CPPCore_getAudioDeviceListEntryDescription(JNIEnv *p_jnienv, jclass jcls, jint index);
extern "C" JNIEXPORT jboolean JNICALL Java_CPPCore_chooseDevice(JNIEnv *p_jnienv, jclass jcls, jint index);
extern "C" JNIEXPORT jboolean JNICALL Java_CPPCore_chooseDefaultDevice(JNIEnv *p_jnienv, jclass jcls);
extern "C" JNIEXPORT jboolean JNICALL Java_CPPCore_initializeAudioObject(JNIEnv *p_jnienv, jclass jcls);
extern "C" JNIEXPORT jboolean JNICALL Java_CPPCore_runPlayback(JNIEnv *p_jnienv, jclass jcls);
extern "C" JNIEXPORT void JNICALL Java_CPPCore_stopPlayback(JNIEnv *p_jnienv, jclass jcls);
extern "C" JNIEXPORT jfloat JNICALL Java_CPPCore_getDryInputAmplitude(JNIEnv *p_jnienv, jclass jcls);
extern "C" JNIEXPORT jboolean JNICALL Java_CPPCore_setDryInputAmplitude(JNIEnv *p_jnienv, jclass jcls, jfloat amp);
extern "C" JNIEXPORT jfloat JNICALL Java_CPPCore_getOutputAmplitude(JNIEnv *p_jnienv, jclass jcls);
extern "C" JNIEXPORT jboolean JNICALL Java_CPPCore_setOutputAmplitude(JNIEnv *p_jnienv, jclass jcls, jfloat amp);
extern "C" JNIEXPORT jint JNICALL Java_CPPCore_getFFDelay(JNIEnv *p_jnienv, jclass jcls, jint nFX);
extern "C" JNIEXPORT jboolean JNICALL Java_CPPCore_setFFDelay(JNIEnv *p_jnienv, jclass jcls, jint nFX, jint delay);
extern "C" JNIEXPORT jfloat JNICALL Java_CPPCore_getFFAmplitude(JNIEnv *p_jnienv, jclass jcls, jint nFX);
extern "C" JNIEXPORT jboolean JNICALL Java_CPPCore_setFFAmplitude(JNIEnv *p_jnienv, jclass jcls, jint nFX, jfloat amp);
extern "C" JNIEXPORT jint JNICALL Java_CPPCore_getFBDelay(JNIEnv *p_jnienv, jclass jcls, jint nFX);
extern "C" JNIEXPORT jboolean JNICALL Java_CPPCore_setFBDelay(JNIEnv *p_jnienv, jclass jcls, jint nFX, jint delay);
extern "C" JNIEXPORT jfloat JNICALL Java_CPPCore_getFBAmplitude(JNIEnv *p_jnienv, jclass jcls, jint nFX);
extern "C" JNIEXPORT jboolean JNICALL Java_CPPCore_setFBAmplitude(JNIEnv *p_jnienv, jclass jcls, jint nFX, jfloat amp);
extern "C" JNIEXPORT jboolean JNICALL Java_CPPCore_resetFFParams(JNIEnv *p_jnienv, jclass jcls);
extern "C" JNIEXPORT jboolean JNICALL Java_CPPCore_resetFBParams(JNIEnv *p_jnienv, jclass jcls);

JNIEXPORT jboolean JNICALL Java_CPPCore_initialize(JNIEnv *p_jnienv, jclass jcls)
{
	return (jboolean) core_init();
}

JNIEXPORT void JNICALL Java_CPPCore_deinitialize(JNIEnv *p_jnienv, jclass jcls)
{
	core_deinit();
	return;
}

JNIEXPORT jstring JNICALL Java_CPPCore_getLastErrorMessage(JNIEnv *p_jnienv, jclass jcls)
{
	const __tchar_t *tstr = NULL;

	tstr = get_last_errmsg();

	if(tstr == NULL) return p_jnienv->NewStringUTF("");

	return p_jnienv->NewStringUTF((const char*) tstr);
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

JNIEXPORT jstring JNICALL Java_CPPCore_getAudioDeviceListEntryName(JNIEnv *p_jnienv, jclass jcls, jint index)
{
	const char *textinfo = NULL;

	textinfo = audiodevicelist_get_entry_info((size_t) index, false);

	if(textinfo == NULL) return p_jnienv->NewStringUTF("");

	return p_jnienv->NewStringUTF(textinfo);
}

JNIEXPORT jstring JNICALL Java_CPPCore_getAudioDeviceListEntryDescription(JNIEnv *p_jnienv, jclass jcls, jint index)
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

JNIEXPORT jboolean JNICALL Java_CPPCore_runPlayback(JNIEnv *p_jnienv, jclass jcls)
{
	return (jboolean) run_playback();
}

JNIEXPORT void JNICALL Java_CPPCore_stopPlayback(JNIEnv *p_jnienv, jclass jcls)
{
	stop_playback();
	return;
}

JNIEXPORT jfloat JNICALL Java_CPPCore_getDryInputAmplitude(JNIEnv *p_jnienv, jclass jcls)
{
	return (jfloat) rtdelay_getDryInputAmplitude();
}

JNIEXPORT jboolean JNICALL Java_CPPCore_setDryInputAmplitude(JNIEnv *p_jnienv, jclass jcls, jfloat amp)
{
	return (jboolean) rtdelay_setDryInputAmplitude((float) amp);
}

JNIEXPORT jfloat JNICALL Java_CPPCore_getOutputAmplitude(JNIEnv *p_jnienv, jclass jcls)
{
	return (jfloat) rtdelay_getOutputAmplitude();
}

JNIEXPORT jboolean JNICALL Java_CPPCore_setOutputAmplitude(JNIEnv *p_jnienv, jclass jcls, jfloat amp)
{
	return (jboolean) rtdelay_setOutputAmplitude((float) amp);
}

JNIEXPORT jint JNICALL Java_CPPCore_getFFDelay(JNIEnv *p_jnienv, jclass jcls, jint nFX)
{
	return (jint) rtdelay_getFFDelay((size_t) nFX);
}

JNIEXPORT jboolean JNICALL Java_CPPCore_setFFDelay(JNIEnv *p_jnienv, jclass jcls, jint nFX, jint delay)
{
	return (jboolean) rtdelay_setFFDelay((size_t) nFX, (uint32_t) delay);
}

JNIEXPORT jfloat JNICALL Java_CPPCore_getFFAmplitude(JNIEnv *p_jnienv, jclass jcls, jint nFX)
{
	return (jfloat) rtdelay_getFFAmplitude((size_t) nFX);
}

JNIEXPORT jboolean JNICALL Java_CPPCore_setFFAmplitude(JNIEnv *p_jnienv, jclass jcls, jint nFX, jfloat amp)
{
	return (jboolean) rtdelay_setFFAmplitude((size_t) nFX, (float) amp);
}

JNIEXPORT jint JNICALL Java_CPPCore_getFBDelay(JNIEnv *p_jnienv, jclass jcls, jint nFX)
{
	return (jint) rtdelay_getFBDelay((size_t) nFX);
}

JNIEXPORT jboolean JNICALL Java_CPPCore_setFBDelay(JNIEnv *p_jnienv, jclass jcls, jint nFX, jint delay)
{
	return (jboolean) rtdelay_setFBDelay((size_t) nFX, (uint32_t) delay);
}

JNIEXPORT jfloat JNICALL Java_CPPCore_getFBAmplitude(JNIEnv *p_jnienv, jclass jcls, jint nFX)
{
	return (jfloat) rtdelay_getFBAmplitude((size_t) nFX);
}

JNIEXPORT jboolean JNICALL Java_CPPCore_setFBAmplitude(JNIEnv *p_jnienv, jclass jcls, jint nFX, jfloat amp)
{
	return (jboolean) rtdelay_setFBAmplitude((size_t) nFX, (float) amp);
}

JNIEXPORT jboolean JNICALL Java_CPPCore_resetFFParams(JNIEnv *p_jnienv, jclass jcls)
{
	return (jboolean) rtdelay_resetFFParams();
}

JNIEXPORT jboolean JNICALL Java_CPPCore_resetFBParams(JNIEnv *p_jnienv, jclass jcls)
{
	return (jboolean) rtdelay_resetFBParams();
}

