#编译.cpp文件时的选项
#AM_CPPFLAGS=-fPIC -shared

#编译.c文件时的选项
AM_CFLAGS=-fPIC -shared
#链接时选项
AM_LDFLAGS=-std=c99 #-L/var/xxx/lib -L/opt/mysql/lib -Wl,R/var/xxx/lib -Wl,R/opt/mysql/lib
#当前目录全局链接库
LIBS=-lpthread

#可执行文件&动态库
#bin_PROGRAMS		编译&安装
#noinst_PROGRAMS	只编译不安装

#静态库
#noinst_LIBRARIES	只编译不安装
#lib_LIBRARIES		编译&安装
noinst_LIBRARIES = libmauto.a
libmauto_a_SOURCES = times.c log.c thpool.c websocket.c taskdata.c linklist.c sfline.c conf.c csocket.c

#动态库
libmautodir=$(libdir)
#libmauto_PROGRAMS = libmauto.so
noinst_PROGRAMS = libmauto.so
libmauto_so_SOURCES = times.c log.c thpool.c websocket.c taskdata.c linklist.c sfline.c conf.c csocket.c

#链接时选项
#libmauto_so_LDFLAGS = -fPIC -shared

#安装头文件
#include_HEADERS = *.h
EXTRA_DIST = *.h
