

#include "main.h"


void into_TestSo();

jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {

    LOG(ERROR) << "插件 JNI_OnLoad 开始加载 ";
    //在 onload 改变 指定函数 函数地址 替换成自己的
    JNIEnv *env = nullptr;
    if (vm->GetEnv((void **) &env, JNI_VERSION_1_6) == JNI_OK) {

        into_TestSo();
        LOGE("%d",gettid());

        return JNI_VERSION_1_6;
    }

    return 0;
}

//获取 ArtSo基地址 函数
// 从 lib64和普通的lib里面去寻找
void *getAppSo() {
    //1,看对应的Map文件
    //2，MT管理器去 data/app 下面

    //so路径需注意

    auto *pLibart = dlopen_compat("com.mmg.detection|libnative-lib.so", RTLD_NOW);

//    auto *pLibart = dlopen_compat(
//            "/data/app/com.kejian.one-X5q5ERrm-bW9b98-rRoKvQ==/lib/arm/libTest.so", RTLD_NOW);

    if (pLibart != nullptr) {
        LOGE("拿到 TestSo.so");
        return pLibart;
    }else{
        LOGE("没有拿到 TestSo.so");
    }
    return nullptr;
}


//jstring My_Java_com_mmg_detection_utils_DebugUtils_getTracerPid(JNIEnv *env, jobject thiz, jstring str) {
//
//    LOG(ERROR) << "插件Hook成功 " << parse::jstring2str(env, str);
//
//    return Source_Java_com_mmg_detection_utils_DebugUtils_gettracerpid(env, thiz, str);
//}
int My_Java_com_mmg_detection_utils_DebugUtils_getTracerPid(JNIEnv *env, jobject thiz) {
    int ret_value = Source_Java_com_mmg_detection_utils_DebugUtils_gettracerpid(env, thiz);
    LOG(ERROR) << "返回值为:";
//    LOGE(ret_value)
    int my_ret_value=339;
    if (ret_value!=0){
        my_ret_value=0;
    }

    return my_ret_value;
}

void into_TestSo() {
    //先拿到对应的So文件句柄
    void *AppSoLib = getAppSo();
    if (AppSoLib != nullptr) {
        LOG(ERROR) << "拿到  TestSo 句柄";


        void *App_Fun = dlsym_compat(AppSoLib,
                                      "Java_com_mmg_detection_utils_DebugUtils_getTracerPid");
        if (App_Fun) {

            //1,我们拿到的原函数的地址
            //2,我们自己实现对应函数的函数地址
            //3,被Hook原函数地址
            if (ELE7EN_OK == registerInlineHook((uint32_t) App_Fun,
                                                (uint32_t) My_Java_com_mmg_detection_utils_DebugUtils_getTracerPid,
                                                (uint32_t **) &Source_Java_com_mmg_detection_utils_DebugUtils_gettracerpid)) {
                if (ELE7EN_OK == inlineHook((uint32_t) App_Fun)) {
                    LOGE("inlineHook Test_MD5 success");
                }
            }
        }
    }

}





