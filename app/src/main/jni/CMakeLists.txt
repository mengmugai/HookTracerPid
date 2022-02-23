
# Sets the minimum version of CMake required to build the native library.
#指定 需要CMAKE的 最小版本要求
cmake_minimum_required(VERSION 3.4.1)


# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.


#添加一个 远程链接库
add_library( # Sets the name of the library. (我们用C/C++ 文件生成的 动态库 so名字 )
        LVmp

        # Sets the library as a shared library.(表明用于共享的 动态库 so结尾)
        SHARED

        # Provides a relative path to your source file(s).(
        # 这里就是 我们的 C的 文件的 路径 可以修改 有多少个 文件
        # 就添加多个 文件 如果 需要 生成 多个 so就需要 添加多个 add_library)
        utils/parse.cpp



        Hook/HLua.cpp
        main.cpp

        #        Substrate
        Substrate/hde64.c
        Substrate/SubstrateDebug.cpp
        Substrate/SubstrateHook.cpp
        Substrate/SubstratePosixMemory.cpp

        #        inlineHook
        dlfc/dlfcn_nougat.cpp
        dlfc/dlfcn_compat.cpp
        inlineHook/inlineHook.cpp
        inlineHook/relocate.cpp


        )

# Searches for a specified prebuilt library and stores the path as a
# variable. Because CMake includes system libraries in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.

find_library( # Sets the name of the path variable.（调用系统的日志库）
        log-lib

        # Specifies the name of the NDK library that
        # you want CMake to locate.
        log)

# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.

target_link_libraries( # Specifies the target library. 链接 我们自己生成的库 名字要和add_library 一样
        LVmp

        # Links the target library to the log library
        # included in the NDK.
        ${log-lib})