#pragma once

/* DO NOT EDIT THIS FILE - it is machine generated */
#include <jni.h>
/* Header for class ru_yandex_passport_tvmauth_DynamicClient */

#ifndef _Included_ru_yandex_passport_tvmauth_DynamicClient
    #define _Included_ru_yandex_passport_tvmauth_DynamicClient
    #ifdef __cplusplus
extern "C" {
    #endif
    /*
     * Class:     ru_yandex_passport_tvmauth_DynamicClient
     * Method:    factoryDynamicClientNative
     * Signature: (JJ)Lru/yandex/passport/tvmauth/DynamicClient/NativeHandles;
     */
    JNIEXPORT jobject JNICALL Java_ru_yandex_passport_tvmauth_DynamicClient_factoryDynamicClientNative(JNIEnv*, jclass, jlong, jlong);

    /*
     * Class:     ru_yandex_passport_tvmauth_DynamicClient
     * Method:    addDstsNative
     * Signature: (JLjava/lang/String;)V
     */
    JNIEXPORT void JNICALL Java_ru_yandex_passport_tvmauth_DynamicClient_addDstsNative(JNIEnv*, jclass, jlong, jstring);

    /*
     * Class:     ru_yandex_passport_tvmauth_DynamicClient
     * Method:    getOptionalServiceTicketForTvmIdNative
     * Signature: (JI)Ljava/lang/String;
     */
    JNIEXPORT jstring JNICALL Java_ru_yandex_passport_tvmauth_DynamicClient_getOptionalServiceTicketForTvmIdNative(JNIEnv*, jclass, jlong, jint);

    #ifdef __cplusplus
}
    #endif
#endif