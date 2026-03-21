/*
 * Real-Time Audio Delay 2 for GNU-Linux systems.
 * Version 3.2 (Interop Java version 1.0)
 *
 * Author: Rafael Sabe
 * Email: rafaelmsabe@gmail.com
 */

#include "libcore.hpp"
#include "CPPCore.h"

extern procctx_t* libjni_getProcCtx(JNIEnv *p_env, jclass jcls);

JNIEXPORT jlong JNICALL Java_CPPCore__1procCtxAlloc(JNIEnv *p_env, jclass jcls)
{
	return (jlong) procctx_alloc();
}

JNIEXPORT jboolean JNICALL Java_CPPCore__1procCtxFree(JNIEnv *p_env, jclass jcls)
{
	return (jboolean) procctx_free(libjni_getProcCtx(p_env, jcls));
}

JNIEXPORT jboolean JNICALL Java_CPPCore__1init(JNIEnv *p_env, jclass jcls)
{
	return (jboolean) core_init(libjni_getProcCtx(p_env, jcls));
}

JNIEXPORT void JNICALL Java_CPPCore__1deinit(JNIEnv *p_env, jclass jcls)
{
	core_deinit(libjni_getProcCtx(p_env, jcls));
	return;
}

JNIEXPORT jstring JNICALL Java_CPPCore__1getLastErrorMessage(JNIEnv *p_env, jclass jcls)
{
	const __tchar_t *tstr = NULL;

	tstr = get_last_errmsg(libjni_getProcCtx(p_env, jcls));

	if(tstr == NULL) return p_env->NewStringUTF("");

	return p_env->NewStringUTF((const char*) tstr);
}

JNIEXPORT jboolean JNICALL Java_CPPCore__1setFileInDir(JNIEnv *p_env, jclass jcls, jstring fileDir)
{
	const char *fdir = NULL;
	bool b_ret;

	fdir = p_env->GetStringUTFChars(fileDir, NULL);

	b_ret = set_filein_dir(libjni_getProcCtx(p_env, jcls), fdir);

	p_env->ReleaseStringUTFChars(fileDir, fdir);

	return (jboolean) b_ret;
}

JNIEXPORT jboolean JNICALL Java_CPPCore__1loadFileCreateAudioObject(JNIEnv *p_env, jclass jcls)
{
	return (jboolean) loadfile_createaudioobject(libjni_getProcCtx(p_env, jcls));
}

JNIEXPORT jboolean JNICALL Java_CPPCore__1loadAudioDeviceList(JNIEnv *p_env, jclass jcls)
{
	return (jboolean) load_audiodevicelist(libjni_getProcCtx(p_env, jcls));
}

JNIEXPORT jint JNICALL Java_CPPCore__1getAudioDeviceListEntryCount(JNIEnv *p_env, jclass jcls)
{
	return (jint) audiodevicelist_get_entry_count(libjni_getProcCtx(p_env, jcls));
}

JNIEXPORT jstring JNICALL Java_CPPCore__1getAudioDeviceListEntryInfo(JNIEnv *p_env, jclass jcls, jint index, jboolean info)
{
	const char *textinfo = NULL;

	textinfo = audiodevicelist_get_entry_info(libjni_getProcCtx(p_env, jcls), (size_t) index, (bool) info);

	if(textinfo == NULL) return p_env->NewStringUTF("");

	return p_env->NewStringUTF(textinfo);
}

JNIEXPORT jboolean JNICALL Java_CPPCore__1chooseDevice(JNIEnv *p_env, jclass jcls, jint index, jboolean defaultDevice)
{
	return (jboolean) choose_device(libjni_getProcCtx(p_env, jcls), (size_t) index, (bool) defaultDevice);
}

JNIEXPORT jboolean JNICALL Java_CPPCore__1initializeAudioObject(JNIEnv *p_env, jclass jcls)
{
	return (jboolean) audioobject_init(libjni_getProcCtx(p_env, jcls));
}

JNIEXPORT jboolean JNICALL Java_CPPCore__1runPlayback(JNIEnv *p_env, jclass jcls)
{
	return (jboolean) run_playback(libjni_getProcCtx(p_env, jcls));
}

JNIEXPORT void JNICALL Java_CPPCore__1stopPlayback(JNIEnv *p_env, jclass jcls)
{
	stop_playback(libjni_getProcCtx(p_env, jcls));
	return;
}

JNIEXPORT jfloat JNICALL Java_CPPCore__1getDryInputAmplitude(JNIEnv *p_env, jclass jcls)
{
	return (jfloat) rtdelay_getDryInputAmplitude(libjni_getProcCtx(p_env, jcls));
}

JNIEXPORT jboolean JNICALL Java_CPPCore__1setDryInputAmplitude(JNIEnv *p_env, jclass jcls, jfloat amp)
{
	return (jboolean) rtdelay_setDryInputAmplitude(libjni_getProcCtx(p_env, jcls), (float) amp);
}

JNIEXPORT jfloat JNICALL Java_CPPCore__1getOutputAmplitude(JNIEnv *p_env, jclass jcls)
{
	return (jfloat) rtdelay_getOutputAmplitude(libjni_getProcCtx(p_env, jcls));
}

JNIEXPORT jboolean JNICALL Java_CPPCore__1setOutputAmplitude(JNIEnv *p_env, jclass jcls, jfloat amp)
{
	return (jboolean) rtdelay_setOutputAmplitude(libjni_getProcCtx(p_env, jcls), (float) amp);
}

JNIEXPORT jint JNICALL Java_CPPCore__1getFFDelay(JNIEnv *p_env, jclass jcls, jint nFX)
{
	return (jint) rtdelay_getFFDelay(libjni_getProcCtx(p_env, jcls), (size_t) nFX);
}

JNIEXPORT jboolean JNICALL Java_CPPCore__1setFFDelay(JNIEnv *p_env, jclass jcls, jint nFX, jint delay)
{
	return (jboolean) rtdelay_setFFDelay(libjni_getProcCtx(p_env, jcls), (size_t) nFX, (uint32_t) delay);
}

JNIEXPORT jfloat JNICALL Java_CPPCore__1getFFAmplitude(JNIEnv *p_env, jclass jcls, jint nFX)
{
	return (jfloat) rtdelay_getFFAmplitude(libjni_getProcCtx(p_env, jcls), (size_t) nFX);
}

JNIEXPORT jboolean JNICALL Java_CPPCore__1setFFAmplitude(JNIEnv *p_env, jclass jcls, jint nFX, jfloat amp)
{
	return (jboolean) rtdelay_setFFAmplitude(libjni_getProcCtx(p_env, jcls), (size_t) nFX, (float) amp);
}

JNIEXPORT jint JNICALL Java_CPPCore__1getFBDelay(JNIEnv *p_env, jclass jcls, jint nFX)
{
	return (jint) rtdelay_getFBDelay(libjni_getProcCtx(p_env, jcls), (size_t) nFX);
}

JNIEXPORT jboolean JNICALL Java_CPPCore__1setFBDelay(JNIEnv *p_env, jclass jcls, jint nFX, jint delay)
{
	return (jboolean) rtdelay_setFBDelay(libjni_getProcCtx(p_env, jcls), (size_t) nFX, (uint32_t) delay);
}

JNIEXPORT jfloat JNICALL Java_CPPCore__1getFBAmplitude(JNIEnv *p_env, jclass jcls, jint nFX)
{
	return (jfloat) rtdelay_getFBAmplitude(libjni_getProcCtx(p_env, jcls), (size_t) nFX);
}

JNIEXPORT jboolean JNICALL Java_CPPCore__1setFBAmplitude(JNIEnv *p_env, jclass jcls, jint nFX, jfloat amp)
{
	return (jboolean) rtdelay_setFBAmplitude(libjni_getProcCtx(p_env, jcls), (size_t) nFX, (float) amp);
}

JNIEXPORT jboolean JNICALL Java_CPPCore__1resetFFParams(JNIEnv *p_env, jclass jcls)
{
	return (jboolean) rtdelay_resetFFParams(libjni_getProcCtx(p_env, jcls));
}

JNIEXPORT jboolean JNICALL Java_CPPCore__1resetFBParams(JNIEnv *p_env, jclass jcls)
{
	return (jboolean) rtdelay_resetFBParams(libjni_getProcCtx(p_env, jcls));
}

procctx_t* libjni_getProcCtx(JNIEnv *p_env, jclass jcls)
{
	jfieldID _jfieldid;
	jlong _jlong;

	if(p_env == NULL) return NULL;

	_jfieldid = p_env->GetStaticFieldID(jcls, "pProcCtx", "J");
	if(_jfieldid == NULL) return NULL;

	_jlong = p_env->GetStaticLongField(jcls, _jfieldid);

	return (procctx_t*) _jlong;
}

