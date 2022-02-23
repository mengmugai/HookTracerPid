// Copyright (c) 2016 avs333
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
//		of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
//		to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//		copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
//		The above copyright notice and this permission notice shall be included in all
//		copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// 		AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

//fork from https://github.com/avs333/Nougat_dlfunctions
//do some modify
//support all cpu abi such as x86, x86_64
//support filename search if filename is not start with '/'

#include "dlfcn_nougat.h"

#define TAG_NAME    "MuGai"

#define log_info(fmt, args...) __android_log_print(ANDROID_LOG_INFO, TAG_NAME, (const char *) fmt, ##args)
#define log_err(...) __android_log_print(ANDROID_LOG_ERROR, TAG_NAME, __VA_ARGS__)

#ifdef LOG_DBG
#define log_dbg log_info
#else
#define log_dbg(...)
#endif

#ifdef __LP64__
#define Elf_Ehdr Elf64_Ehdr
#define Elf_Shdr Elf64_Shdr
#define Elf_Sym  Elf64_Sym
#else
#define Elf_Ehdr Elf32_Ehdr
#define Elf_Shdr Elf32_Shdr
#define Elf_Sym  Elf32_Sym
#endif


struct ctx {
    void *load_addr;
    void *dynstr;
    void *dynsym;
    int nsyms;
    off_t bias;
};

//extern "C" {

int fake_dlclose(void *handle) {
    if (handle) {
        struct ctx *ctx = (struct ctx *) handle;
        if (ctx->dynsym) free(ctx->dynsym);    /* we're saving dynsym and dynstr */
        if (ctx->dynstr) free(ctx->dynstr);    /* from library file just in case */
        free(ctx);
    }
    return 0;
}

#define fatal(fmt, args...) do { log_err(fmt,##args); goto err_exit; } while(0)

/* flags are ignored */
void *fake_dlopen_with_path(const char *libpath, int flags) {
    log_err("fake_dlopen_with_path 路径 为 %s", libpath);
    FILE *maps;
    char buff[256];
    struct ctx *ctx = 0;
    off_t load_addr, size;
    int k, fd = -1, found = 0;
    char *shoff;
    Elf_Ehdr *elf = (Elf_Ehdr *) MAP_FAILED;


    maps = fopen("/proc/self/maps", "r");

    if (!maps) {
        log_err("打开 maps文件失败 ");
        if (fd >= 0) close(fd);
        if (elf != MAP_FAILED) munmap(elf, size);
        fake_dlclose(ctx);
    }

    const vector<string> &vector = parse::Split(libpath, "|");
    if (vector.size() == 2) {
        while (!found && fgets(buff, sizeof(buff), maps)) {
            if (strstr(buff, "r-xp") && vector.size() == 2) {
//                string *pString1 = new string(buff);

                log_err("%s",vector.at(0).c_str());
                log_err("%s",buff);
                //如果包含| 并且规则匹配
                if (strstr(buff, vector.at(0).c_str()) && strstr(buff, vector.at(1).c_str())) {
//c0940000-c09b7000 r-xp 00000000 fc:08 1229975                            /data/app/com.kejian.one-Ym849UWJxpgSsC27ctZ8CA==/lib/arm/libTest.so
                    log_err("这句是 2");
                    auto *pString = new string(buff);

                    string path = pString->substr(pString->find("/data/app/"));
                    log_err("这句是 3");
                    //去掉回车 坑！！！！！
                    size_t n = path.find_last_not_of("\r\n\t");
                    if (n != string::npos){
                        path.erase(n + 1, path.size() - n);
                    }
                    log_err("这句是 4");
                    libpath = path.c_str();
                    found = 1;
                }
            }
        }
    } else {
        while (!found && fgets(buff, sizeof(buff), maps)) {
//          log_err("内存文件的和名字  %s  ", buff);
            if (strstr(buff, "r-xp") && strstr(buff, libpath)) {
                found = 1;
            }
        }
    }

    fclose(maps);

    if (!found) log_err("%s 没找到指定路径 ", libpath);


    if (sscanf(buff, "%lx", &load_addr) != 1) fatal("failed to read load address for %s", libpath);



    /* Now, mmap the same library once again */

    fd = open(libpath, O_RDONLY);
    if (fd < 0) fatal("打开文件失败 %s  %d", libpath, fd);
//    else
//        log_info("打开文件成功 %s", libpath);


    size = lseek(fd, 0, SEEK_END);
    if (size <= 0) fatal("lseek() failed for %s", libpath);



    elf = (Elf_Ehdr *) mmap(0, size, PROT_READ, MAP_SHARED, fd, 0);
    close(fd);
    fd = -1;

    if (elf == MAP_FAILED) fatal("mmap() failed for %s", libpath);


    ctx = (struct ctx *) calloc(1, sizeof(struct ctx));
    if (!ctx) fatal("no memory for %s", libpath);



    ctx->load_addr = (void *) load_addr;
    shoff = ((char *) elf) + elf->e_shoff;

    for (k = 0; k < elf->e_shnum; k++, shoff += elf->e_shentsize) {

        Elf_Shdr *sh = (Elf_Shdr *) shoff;
        log_dbg("%s: k=%d shdr=%p type=%x", __func__, k, sh, sh->sh_type);

        switch (sh->sh_type) {

            case SHT_DYNSYM:
                if (ctx->dynsym) fatal("%s: duplicate DYNSYM sections", libpath); /* .dynsym */
                ctx->dynsym = malloc(sh->sh_size);
                if (!ctx->dynsym) fatal("%s: no memory for .dynsym", libpath);
                memcpy(ctx->dynsym, ((char *) elf) + sh->sh_offset, sh->sh_size);
                ctx->nsyms = (sh->sh_size / sizeof(Elf_Sym));
                break;

            case SHT_STRTAB:
                if (ctx->dynstr) break;    /* .dynstr is guaranteed to be the first STRTAB */
                ctx->dynstr = malloc(sh->sh_size);
                if (!ctx->dynstr) fatal("%s: no memory for .dynstr", libpath);
                memcpy(ctx->dynstr, ((char *) elf) + sh->sh_offset, sh->sh_size);
                break;

            case SHT_PROGBITS:
                if (!ctx->dynstr || !ctx->dynsym) break;
                /* won't even bother checking against the section name */
                ctx->bias = (off_t) sh->sh_addr - (off_t) sh->sh_offset;
                k = elf->e_shnum;  /* exit for */
                break;
        }
    }

    munmap(elf, size);
    elf = 0;

    if (!ctx->dynstr || !ctx->dynsym) fatal("dynamic sections not found in %s", libpath);

#undef fatal

    log_dbg("%s: ok, dynsym = %p, dynstr = %p", libpath, ctx->dynsym, ctx->dynstr);

    return ctx;

    err_exit:
    if (fd >= 0) close(fd);
    if (elf != MAP_FAILED) munmap(elf, size);
    fake_dlclose(ctx);
    return 0;
}


#if defined(__LP64__)
static const char *const kSystemLibDir = "/system/lib64/";
static const char *const kOdmLibDir = "/odm/lib64/";
static const char *const kVendorLibDir = "/vendor/lib64/";
#else
static const char *const kSystemLibDir = "/system/lib/";
static const char *const kOdmLibDir = "/odm/lib/";
static const char *const kVendorLibDir = "/vendor/lib/";


#endif


void *fake_dlopen(const char *filename, int flags) {
    if ((strlen(filename) > 0 && filename[0] == '/') || (strstr(filename, "|") != nullptr)) {
        return fake_dlopen_with_path(filename, flags);
    } else {

        //尝试对系统So路径进行拼接
        char buf[512] = {0};
        void *handle = NULL;
        //sysmtem
        ///system/lib/com.kejian.one|libTest.so
        strcpy(buf, kSystemLibDir);
        strcat(buf, filename);
        //  log_err("路径变为 %s", buf);
        handle = fake_dlopen_with_path(buf, flags);
        if (handle) {
            return handle;
        }

        //odm
        memset(buf, 0, sizeof(buf));
        strcpy(buf, kOdmLibDir);
        strcat(buf, filename);
        handle = fake_dlopen_with_path(buf, flags);
        if (handle) {
            return handle;
        }

        //vendor
        memset(buf, 0, sizeof(buf));
        strcpy(buf, kVendorLibDir);
        strcat(buf, filename);
        handle = fake_dlopen_with_path(buf, flags);
        if (handle) {
            return handle;
        }

        return fake_dlopen_with_path(filename, flags);
    }


}

void *fake_dlsym(void *handle, const char *name) {
    log_err("开始 执行  fake_dlsym");

    int k;
    struct ctx *ctx = (struct ctx *) handle;
    Elf_Sym *sym = (Elf_Sym *) ctx->dynsym;
    char *strings = (char *) ctx->dynstr;

    for (k = 0; k < ctx->nsyms; k++, sym++)
        if (strcmp(strings + sym->st_name, name) == 0) {
            void *ret = (char *) ctx->load_addr + sym->st_value - ctx->bias;
            log_err("%s 找到对应函数 地址是 %p", name, ret);
            return ret;
        }
    return 0;

}


const char *fake_dlerror() {
    return NULL;
}
//}