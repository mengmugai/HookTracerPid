//
// Created by Administrator on 2020-08-19.
//

#ifndef FENXIANG_HLUA_H
#define FENXIANG_HLUA_H



// art的 so
#define Load_SO      "/data/app/com.avalon.caveonline.cn.leiting-1/lib/arm/libcocos2dlua.so"
// dvm 的 so路径
#define DvmLoad_SO   "/data/data/com.lyh.q296488320/lib/libcocos2dlua.so"


struct _MonoImage {
    //data: 脚本内容
    //
    //data_len:脚本长度
    //
    //name:脚本名称
    int ref_count;
    void *raw_data_handle;
    char *raw_data;
    int raw_data_len;

} typedef MonoImage;


static int (*Origin_luaL_loadbuffer)(void *lua_state, char *buff, size_t size, char *name)= nullptr;


static MonoImage *(*Origin_mono_image_init_mod_t)(char *data,
                                                  size_t data_len,
                                                  int need_copy,
                                                  void *status, int refonly, char *name)= nullptr;
class HLua {


public:
    static void HookLua();

    static void HookU3d();


private:
    static int My_luaL_loadbuffer(void *lua_state, char *buff, size_t size, char *name);

    static MonoImage * My_mono_image_init_mod_t(char *data,size_t data_len,int need_copy,void *status,int refonly,char *name);


};


#endif //FENXIANG_HLUA_H
