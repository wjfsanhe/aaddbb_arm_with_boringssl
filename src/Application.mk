APP_STL := gnustl_static 
#https://blog.csdn.net/ccxiaop/article/details/7850089
#    system --> 系统默认的最小支持的C++运行时库
#    stlport_static --> 以静态链接的方式使用stlport版本的STL
#    stlport_shared --> 以动态链接的方式使用stlport版本的STL
#    gnustl_static  --> 以静态链接的方式使用gnu版本的STL
APP_PLATFORM := android-24
iAPP_ABI := armeabi-v7a
APP_CFLAGS += -fext-numeric-literals
