/*
 * Real-Time Audio Delay 2 for GNU-Linux systems.
 * Version 3.1.1 (Interop Java version 1.0)
 *
 * Author: Rafael Sabe
 * Email: rafaelmsabe@gmail.com
 */

#include "libcore.hpp"
#include "CPPCore.h"

__attribute__((__aligned__(PTR_SIZE_BITS))) JavaVM *p_jvm = NULL;

void libjni_trigger_exitProcess(int exitCode)
{
	JNIEnv *p_jnienv = NULL;
	jclass _jcls;
	jmethodID _jmethodid;

	int n_ret;

	n_ret = p_jvm->AttachCurrentThread((void**) &p_jnienv, NULL);

	if(n_ret != JNI_OK) return;

	_jcls = p_jnienv->FindClass("Main");
	if(_jcls == NULL) return;

	_jmethodid = p_jnienv->GetMethodID(_jcls, "appExit", "(I)V");
	if(_jmethodid == NULL) return;

	p_jnienv->CallStaticVoidMethod(_jcls, _jmethodid, (jint) exitCode);

	p_jvm->DetachCurrentThread();
	return;
}

JNIEXPORT jboolean JNICALL Java_CPPCore_init(JNIEnv *p_env, jclass jcls)
{
	if(!core_init()) return JNI_FALSE;

	p_env->GetJavaVM(&p_jvm);
	return JNI_TRUE;
}

JNIEXPORT void JNICALL Java_CPPCore_deinit(JNIEnv *p_env, jclass jcls)
{
	core_deinit();
	return;
}

JNIEXPORT jstring JNICALL Java_CPPCore_getLastErrorMessage(JNIEnv *p_env, jclass jcls)
{
	return p_env->NewStringUTF(get_last_errmsg());
}

JNIEXPORT jboolean JNICALL Java_CPPCore_setFileInDir(JNIEnv *p_env, jclass jcls, jstring fileDir)
{
	const char *fdir = NULL;
	bool b_ret;

	fdir = p_env->GetStringUTFChars(fileDir, NULL);

	b_ret = set_filein_dir(fdir);

	p_env->ReleaseStringUTFChars(fileDir, fdir);

	return (jboolean) b_ret;
}

JNIEXPORT jboolean JNICALL Java_CPPCore_setAudioDevDesc(JNIEnv *p_env, jclass jcls, jstring devDesc)
{
	const char *desc = NULL;
	bool b_ret;

	desc = p_env->GetStringUTFChars(devDesc, NULL);

	b_ret = set_audiodev_desc(desc);

	p_env->ReleaseStringUTFChars(devDesc, desc);

	return (jboolean) b_ret;
}

JNIEXPORT jboolean JNICALL Java_CPPCore_playbackInit(JNIEnv *p_env, jclass jcls)
{
	return (jboolean) playback_init();
}

JNIEXPORT jboolean JNICALL Java_CPPCore_playbackRun(JNIEnv *p_env, jclass jcls)
{
	return (jboolean) playback_run();
}

JNIEXPORT void JNICALL Java_CPPCore_stopPlayback(JNIEnv *p_env, jclass jcls)
{
	stop_playback();
	return;
}

JNIEXPORT jfloat JNICALL Java_CPPCore_getDryInputAmplitude(JNIEnv *p_env, jclass jcls)
{
	return (jfloat) rtdelay_getDryInputAmplitude();
}

JNIEXPORT jboolean JNICALL Java_CPPCore_setDryInputAmplitude(JNIEnv *p_env, jclass jcls, jfloat amp)
{
	return (jboolean) rtdelay_setDryInputAmplitude((float) amp);
}

JNIEXPORT jfloat JNICALL Java_CPPCore_getOutputAmplitude(JNIEnv *p_env, jclass jcls)
{
	return (jfloat) rtdelay_getOutputAmplitude();
}

JNIEXPORT jboolean JNICALL Java_CPPCore_setOutputAmplitude(JNIEnv *p_env, jclass jcls, jfloat amp)
{
	return (jboolean) rtdelay_setOutputAmplitude((float) amp);
}

JNIEXPORT jint JNICALL Java_CPPCore_getFFDelay(JNIEnv *p_env, jclass jcls, jint nFX)
{
	return (jint) rtdelay_getFFDelay((size_t) nFX);
}

JNIEXPORT jboolean JNICALL Java_CPPCore_setFFDelay(JNIEnv *p_env, jclass jcls, jint nFX, jint delay)
{
	return (jboolean) rtdelay_setFFDelay((size_t) nFX, (uint32_t) delay);
}

JNIEXPORT jfloat JNICALL Java_CPPCore_getFFAmplitude(JNIEnv *p_env, jclass jcls, jint nFX)
{
	return (jfloat) rtdelay_getFFAmplitude((size_t) nFX);
}

JNIEXPORT jboolean JNICALL Java_CPPCore_setFFAmplitude(JNIEnv *p_env, jclass jcls, jint nFX, jfloat amp)
{
	return (jboolean) rtdelay_setFFAmplitude((size_t) nFX, (float) amp);
}

JNIEXPORT jint JNICALL Java_CPPCore_getFBDelay(JNIEnv *p_env, jclass jcls, jint nFX)
{
	return (jint) rtdelay_getFBDelay((size_t) nFX);
}

JNIEXPORT jboolean JNICALL Java_CPPCore_setFBDelay(JNIEnv *p_env, jclass jcls, jint nFX, jint delay)
{
	return (jboolean) rtdelay_setFBDelay((size_t) nFX, (uint32_t) delay);
}

JNIEXPORT jfloat JNICALL Java_CPPCore_getFBAmplitude(JNIEnv *p_env, jclass jcls, jint nFX)
{
	return (jfloat) rtdelay_getFBAmplitude((size_t) nFX);
}

JNIEXPORT jboolean JNICALL Java_CPPCore_setFBAmplitude(JNIEnv *p_env, jclass jcls, jint nFX, jfloat amp)
{
	return (jboolean) rtdelay_setFBAmplitude((size_t) nFX, (float) amp);
}

JNIEXPORT jboolean JNICALL Java_CPPCore_resetFFParams(JNIEnv *p_env, jclass jcls)
{
	return (jboolean) rtdelay_resetFFParams();
}

JNIEXPORT jboolean JNICALL Java_CPPCore_resetFBParams(JNIEnv *p_env, jclass jcls)
{
	return (jboolean) rtdelay_resetFBParams();
}

