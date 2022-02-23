//
// Created by Administrator on 2020-08-19.
//

#include <stdint.h>
#include <dlfcn.h>
#include "HLua.h"
#include "../dlfc/dlfcn_compat.h"
#include "../utils/logging.h"
#include "../utils/Log.h"
#include "../inlineHook/inlineHook.h"





void HLua::HookLua() {

    void *handle=NULL;

    if(get_sdk_level() <= 19){
        // 4.4 直接 dlopen打开 就行 没有
        // dlopen限制 可以加载 任意路径的 so
        handle = dlopen(DvmLoad_SO, RTLD_NOW);
        LOGE("在 DVM 拿句柄 ");
    } else{
        handle = dlopen_compat(Load_SO, RTLD_NOW);
        LOGE("在 ART 拿句柄  ");
    }

    if (handle) {
        LOGE("拿到 so 地址 ");
        void *luabuffer = dlsym_compat(handle, "luaL_loadbuffer");

        if (luabuffer) {
            LOGE("拿到了 luaL_loadbuffer 地址 ");
            if (ELE7EN_OK == registerInlineHook((uint32_t) luabuffer,
                                                (uint32_t) My_luaL_loadbuffer,
                                                (uint32_t **) &Origin_luaL_loadbuffer)) {

                if (ELE7EN_OK == inlineHook((uint32_t) luabuffer)) {
                    LOGE("挂钩成功 luaL_loadbuffer ");
                }
            }
        } else{
            LOGE("没找到 Buffloader 地址 ")
        }
    }
}

int HLua::My_luaL_loadbuffer(void *lua_state, char *buff, size_t size, char *name) {
    LOGE("lua size: %d, name: %s", (uint32_t) size, name);
    if (name != NULL) {
        char *name_t = strdup(name);
        if (name_t != " " && name_t[0] != ' ') {
            FILE *file;
            char full_name[256];
            int name_len = strlen(name);
            if (8 < name_len <= 100) {
                char *base_dir = (char *) "/sdcard/hookLua/";
                int i = 0;
                while (i < name_len) {
                    if (name_t[i] == '/') {
                        name_t[i] = '.';
                    }
                    i++;
                }
                if (strstr(name_t, ".lua")) {
                    sprintf(full_name, "%s%s", base_dir, name_t);
                    file = fopen(full_name, "wb");
                    if (file != NULL) {
                        fwrite(buff, 1, size, file);
                        fclose(file);
                        free(name_t);
                    }



                    //lua脚本hook加载
                    /*file = fopen(full_name, "r");
                    if (file != NULL) {
                        LOGD("[Tencent]-------path-----%s", full_name);
                        fseek(file, 0, SEEK_END);
                        size_t new_size = ftell(file);
                        fseek(file, 0, SEEK_SET);
                        char *new_buff = (char *) alloca(new_size + 1);
                        fread(new_buff, new_size, 1, file);
                        fclose(file);
                        return origin_luaL_loadbuffer(lua_state, buff, size, name);
                    }*/
                }
            }
        }
    }

    return Origin_luaL_loadbuffer(lua_state, buff, size, name);
}

void HLua::HookU3d() {
    void *handle=nullptr;

    if(get_sdk_level() <= 19){
        // 4.4 直接 dlopen打开 就行 没有
        // dlopen限制 可以加载 任意路径的 so
        handle = dlopen(DvmLoad_SO, RTLD_NOW);
        LOGE("在 DVM 拿句柄 ");
    } else{
        handle = dlopen_compat(Load_SO, RTLD_NOW);
        LOGE("在 ART 拿句柄  ");
    }

    if (handle) {

        void *mono_image_open = dlsym_compat(handle, "mono_image_open_from_data_with_name");

        if (mono_image_open) {
            if (ELE7EN_OK == registerInlineHook((uint32_t) mono_image_open,
                                                (uint32_t) My_mono_image_init_mod_t,
                                                (uint32_t **) &Origin_mono_image_init_mod_t)) {

                if (ELE7EN_OK == inlineHook((uint32_t) mono_image_open)) {
                    LOGE("inlineHook mono_image_open_from_data_with_name success");
                }
            }
        }
    }
}


MonoImage * HLua::My_mono_image_init_mod_t(char *data,
                                   size_t data_len,
                                   int need_copy,
                                   void *status,
                                   int refonly,
                                   char *name) {


    if (name != NULL) {
        //strdup()在内部调用了malloc()为变量分配内存
        char *name_t = strdup(name);
        if (name_t != " " && name_t[0] != ' ') {
            FILE *file;
            char full_name[256];
            int name_len = strlen(name);
            //if (8 < name_len <= 100) {
            char *base_dir = (char *) "/sdcard/hookDll/";
            int i = 0;
            while (i < name_len) {
                //将 / 换成 .
                if (name_t[i] == '/') {
                    name_t[i] = '.';
                }
                i++;
            }
            //lua脚本保存
            file = fopen(full_name, "wb");
            if (file != NULL) {
                fwrite(data, 1, data_len, file);
                fclose(file);
                free(name_t);
            }

        }
    }
    return Origin_mono_image_init_mod_t(data,data_len,need_copy,status,refonly,name);

}