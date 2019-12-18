//
// Created by admin on 2019/10/23.
//
#include "CallJava.h"
CallJava::CallJava(_JavaVM *javaVM, JNIEnv *env, jobject *obj) {
    LOGE("=================CallJava");
    this->javaVM = javaVM;
    this->jniEnv = env;
    this->jobj = *obj;
    this->jobj = env->NewGlobalRef(jobj);
    jclass  jlz = jniEnv->GetObjectClass(jobj);
    if(!jlz)
    {
        if(LOG_DEBUG)
        {
            LOGE("get jclass wrong");
        }
        return;
    }

    jmid_parpared = env->GetMethodID(jlz, "onCallParpared", "()V");
    jmid_load = env->GetMethodID(jlz, "onCallLoad", "(Z)V");
    jmid_timeinfo = env->GetMethodID(jlz, "onCallTimeInfo", "(II)V");
    jmid_error = env->GetMethodID(jlz, "onCallError", "(ILjava/lang/String;)V");
    jmid_complete = env->GetMethodID(jlz, "onCallComplete", "()V");
    jmid_valumedb = env->GetMethodID(jlz, "onCallValumeDB", "(I)V");
    jmid_pcmtoaac = env->GetMethodID(jlz, "encodecPcmToAAc", "(I[B)V");
    jmid_pcminfo = env->GetMethodID(jlz, "onCallPcmInfo", "([BI)V");
    jmid_pcmrate = env->GetMethodID(jlz, "onCallPcmRate", "(I)V");

}

void CallJava::onCallParpared(int type) {

    if(type == MAIN_THREAD)
    {
        jniEnv->CallVoidMethod(jobj, jmid_parpared);
    }
    else if(type == CHILD_THREAD)
    {
        JNIEnv *jniEnv;
        if(javaVM->AttachCurrentThread(&jniEnv, 0) != JNI_OK)
        {
            if(LOG_DEBUG)
            {
                LOGE("get child thread jnienv worng");
            }
            return;
        }
        jniEnv->CallVoidMethod(jobj, jmid_parpared);
        javaVM->DetachCurrentThread();
    }

}

void CallJava::onCallLoad(int type, bool load) {

    if(type == MAIN_THREAD)
    {
        jniEnv->CallVoidMethod(jobj, jmid_load, load);
    }
    else if(type == CHILD_THREAD)
    {
        JNIEnv *jniEnv;
        if(javaVM->AttachCurrentThread(&jniEnv, 0) != JNI_OK)
        {
            if(LOG_DEBUG)
            {
                LOGE("call onCallLoad worng");
            }
            return;
        }
        jniEnv->CallVoidMethod(jobj, jmid_load, load);
        javaVM->DetachCurrentThread();
    }


}

void CallJava::onCallTimeInfo(int type, int curr, int total) {
    if(type == MAIN_THREAD)
    {
        jniEnv->CallVoidMethod(jobj, jmid_timeinfo, curr, total);
    }
    else if(type == CHILD_THREAD)
    {
        JNIEnv *jniEnv;
        if(javaVM->AttachCurrentThread(&jniEnv, 0) != JNI_OK)
        {
            if(LOG_DEBUG)
            {
                LOGE("call onCallTimeInfo worng");
            }
            return;
        }
        jniEnv->CallVoidMethod(jobj, jmid_timeinfo, curr, total);
        javaVM->DetachCurrentThread();
    }
}

CallJava::~CallJava() {

}

void CallJava::onCallError(int type, int code, char *msg) {
    if(type == MAIN_THREAD)
    {
        jstring jmsg = jniEnv->NewStringUTF(msg);
        jniEnv->CallVoidMethod(jobj, jmid_error, code, jmsg);
        jniEnv->DeleteLocalRef(jmsg);
    }
    else if(type == CHILD_THREAD)
    {
        JNIEnv *jniEnv;
        if(javaVM->AttachCurrentThread(&jniEnv, 0) != JNI_OK)
        {
            if(LOG_DEBUG)
            {
                LOGE("call onCallError worng");
            }
            return;
        }
        jstring jmsg = jniEnv->NewStringUTF(msg);
        jniEnv->CallVoidMethod(jobj, jmid_error, code, jmsg);
        jniEnv->DeleteLocalRef(jmsg);
        javaVM->DetachCurrentThread();
    }
}

void CallJava::onCallComplete(int type) {
    if(type == MAIN_THREAD)
    {
        jniEnv->CallVoidMethod(jobj, jmid_complete);
    }
    else if(type == CHILD_THREAD)
    {
        JNIEnv *jniEnv;
        if(javaVM->AttachCurrentThread(&jniEnv, 0) != JNI_OK)
        {
            if(LOG_DEBUG)
            {
                LOGE("call onCallComplete worng");
            }
            return;
        }
        jniEnv->CallVoidMethod(jobj, jmid_complete);
        javaVM->DetachCurrentThread();
    }
}

void CallJava::onCallValumeDB(int type, int db) {
    if(type == MAIN_THREAD)
    {
        jniEnv->CallVoidMethod(jobj, jmid_valumedb, db);
    }
    else if(type == CHILD_THREAD)
    {
        JNIEnv *jniEnv;
        if(javaVM->AttachCurrentThread(&jniEnv, 0) != JNI_OK)
        {
            if(LOG_DEBUG)
            {
                LOGE("call onCallComplete worng");
            }
            return;
        }
        jniEnv->CallVoidMethod(jobj, jmid_valumedb, db);
        javaVM->DetachCurrentThread();
    }
}

void CallJava::onCallPcmToAAc(int type, int size, void *buffer) {

    if(type == MAIN_THREAD)
    {
        jbyteArray jbuffer = jniEnv->NewByteArray(size);
        jniEnv->SetByteArrayRegion(jbuffer, 0, size, static_cast<const jbyte *>(buffer));

        jniEnv->CallVoidMethod(jobj, jmid_pcmtoaac, size, jbuffer);

        jniEnv->DeleteLocalRef(jbuffer);

    }
    else if(type == CHILD_THREAD)
    {
        JNIEnv *jniEnv;
        if(javaVM->AttachCurrentThread(&jniEnv, 0) != JNI_OK)
        {
            if(LOG_DEBUG)
            {
                LOGE("call onCallComplete worng");
            }
            return;
        }
        jbyteArray jbuffer = jniEnv->NewByteArray(size);
        jniEnv->SetByteArrayRegion(jbuffer, 0, size, static_cast<const jbyte *>(buffer));

        jniEnv->CallVoidMethod(jobj, jmid_pcmtoaac, size, jbuffer);

        jniEnv->DeleteLocalRef(jbuffer);

        javaVM->DetachCurrentThread();
    }

}

void CallJava::onCallPcmInfo(void *buffer, int size) {
    JNIEnv *jniEnv;
    if(javaVM->AttachCurrentThread(&jniEnv, 0) != JNI_OK)
    {
        if(LOG_DEBUG)
        {
            LOGE("call onCallComplete worng");
        }
        return;
    }
    jbyteArray jbuffer = jniEnv->NewByteArray(size);
    jniEnv->SetByteArrayRegion(jbuffer, 0, size, static_cast<const jbyte *>(buffer));

    jniEnv->CallVoidMethod(jobj, jmid_pcminfo, jbuffer, size);

    jniEnv->DeleteLocalRef(jbuffer);

    javaVM->DetachCurrentThread();
}

void CallJava::onCallPcmRate(int samplerate) {
    JNIEnv *jniEnv;
    if(javaVM->AttachCurrentThread(&jniEnv, 0) != JNI_OK)
    {
        if(LOG_DEBUG)
        {
            LOGE("call onCallComplete worng");
        }
        return;
    }
    jniEnv->CallVoidMethod(jobj, jmid_pcmrate, samplerate);

    javaVM->DetachCurrentThread();
}



