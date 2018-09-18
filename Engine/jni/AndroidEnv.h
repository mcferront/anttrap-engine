#pragma once

#include "EngineGlobal.h"

class AndroidEnv
{
private:
    static JavaVM   *s_pJavaVM;

public:
    static void SetJavaVM(JavaVM *pJavaVM)
    {
        s_pJavaVM = pJavaVM;
    }

    static JavaVM *GetJavaVM()
    {
        return s_pJavaVM;
    }

    static JNIEnv *GetJNIEnv()
    {
        Debug::Assert(Condition(s_pJavaVM != NULL), "JavaVM has not been initialized" );

        JNIEnv *pEnv = NULL;
        jint result = s_pJavaVM->GetEnv((void**)&pEnv, JNI_VERSION_1_4);
    
        Debug::Assert(Condition(result == JNI_OK), "Required JNI version 1.4 is unsupported" );

        return pEnv;
    }
};
