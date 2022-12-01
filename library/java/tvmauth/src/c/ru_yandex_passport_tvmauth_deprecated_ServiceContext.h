#pragma once

/* DO NOT EDIT THIS FILE - it is machine generated */
#include <jni.h>
/* Header for class ru_yandex_passport_tvmauth_deprecated_ServiceContext */

#ifndef _Included_ru_yandex_passport_tvmauth_deprecated_ServiceContext
    #define _Included_ru_yandex_passport_tvmauth_deprecated_ServiceContext
    #ifdef __cplusplus
extern "C" {
    #endif
    /*
     * Class:     ru_yandex_passport_tvmauth_deprecated_ServiceContext
     * Method:    factory
     * Signature: (ILjava/lang/String;Ljava/lang/String;)J
     */
    JNIEXPORT jlong JNICALL Java_ru_yandex_passport_tvmauth_deprecated_ServiceContext_factory(JNIEnv*, jclass, jint, jstring, jstring);

    /*
     * Class:     ru_yandex_passport_tvmauth_deprecated_ServiceContext
     * Method:    dispose
     * Signature: (J)V
     */
    JNIEXPORT void JNICALL Java_ru_yandex_passport_tvmauth_deprecated_ServiceContext_dispose(JNIEnv*, jclass, jlong);

    /*
     * Class:     ru_yandex_passport_tvmauth_deprecated_ServiceContext
     * Method:    checkNative
     * Signature: (JLjava/lang/String;)Lru/yandex/passport/tvmauth/CheckedServiceTicket;
     */
    JNIEXPORT jobject JNICALL Java_ru_yandex_passport_tvmauth_deprecated_ServiceContext_checkNative(JNIEnv*, jclass, jlong, jstring);

    /*
     * Class:     ru_yandex_passport_tvmauth_deprecated_ServiceContext
     * Method:    signCgiParamsForTvmNative
     * Signature: (JLjava/lang/String;Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;
     */
    JNIEXPORT jstring JNICALL Java_ru_yandex_passport_tvmauth_deprecated_ServiceContext_signCgiParamsForTvmNative(JNIEnv*, jclass, jlong, jstring, jstring, jstring);

    #ifdef __cplusplus
}
    #endif
#endif