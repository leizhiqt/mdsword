AUTOMAKE_OPTIONS=foreign

#bin_PROGRAMS=chklic4fuzz deepdoor
bin_PROGRAMS=deepdoor
noinst_PROGRAMS=example
#noinst_PROGRAMS=example excapture extest cvtest

#>>>>>>>>>
#编译.cpp文件时的选项
#AM_CPPFLAGS=-I$(top_srcdir)/libcommons

#编译.c文件时的选项
#AM_CFLAGS=-fPIC -shared

#链接时选项
#AM_LDFLAGS=-std=c99 -L$(top_srcdir)/libcommons

#链接时所需链接的库文件
#AM_LDADD=-lmauto

#当前目录全局链接库
LIBS=-lpthread
#<<<<<<<<<

#>>>>>>>>>
#编译源文件
#XX_SOURCES=test.c

#编译.cpp文件时的选项
#XX__CPPFLAGS=-I$(top_srcdir)/libcommons

#编译.c文件时的选项
#XX__CFLAGS=-I$(top_srcdir)/libcommons

#链接选项
#XX_LDFLAGS=-static -L$(top_srcdir)/libcommons

#链接库
#XX_LDADD=-lmauto
#<<<<<<<<<

#chklic4fuzz
#chklic4fuzz_SOURCES=chklic4fuzz.c dongle.c
#chklic4fuzz_CFLAGS=-I$(top_srcdir)/libcommons
#chklic4fuzz_LDFLAGS=-static -L$(top_srcdir)/libcommons
#chklic4fuzz_LDADD=-lmauto -lRockeyARM

#deepdoor
deepdoor_SOURCES=deepdoor.cpp exdlib.cpp
#编译.cpp文件时的选项
#deepdoor_CPPFLAGS=-DDEEPDOOR_DEBUG -I$(top_srcdir)/libcommons -I/usr/include/opencv -I/usr/local/src/dlib
deepdoor_CPPFLAGS=-I$(top_srcdir)/libcommons -I/usr/include/opencv -I/usr/local/src/dlib
#链接时选项
#deepdoor_LDFLAGS=-static -Wl,-dy -lopencv_ml -Wl,-dn -lmauto -Wl,--as-needed
#链接时所需链接的库文件
deepdoor_LDADD=$(top_srcdir)/libcommons/libmauto.a -lopencv_shape -lopencv_stitching -lopencv_superres -lopencv_videostab -lopencv_aruco -lopencv_bgsegm -lopencv_bioinspired -lopencv_ccalib -lopencv_cvv -lopencv_dpm -lopencv_face -lopencv_photo -lopencv_freetype -lopencv_fuzzy -lopencv_hdf -lopencv_hfs -lopencv_img_hash -lopencv_line_descriptor -lopencv_optflow -lopencv_reg -lopencv_rgbd -lopencv_saliency -lopencv_stereo -lopencv_structured_light -lopencv_phase_unwrapping -lopencv_surface_matching -lopencv_tracking -lopencv_datasets -lopencv_video -lopencv_plot -lopencv_ml -lopencv_ximgproc -lopencv_calib3d -lopencv_features2d -lopencv_highgui -lopencv_videoio -lopencv_flann -lopencv_xobjdetect -lopencv_imgcodecs -lopencv_objdetect -lopencv_xphoto -lopencv_imgproc -lopencv_core -ldlib

#extest
extest_SOURCES=extest.cpp exdlib.cpp
#编译.cpp文件时的选项
#extest_CPPFLAGS=-DDEEPDOOR_DEBUG -I$(top_srcdir)/libcommons -I/usr/include/opencv -I/usr/local/src/dlib
extest_CPPFLAGS=-DDLIB_JPEG_SUPPORT -I$(top_srcdir)/libcommons -I/usr/include/opencv -I/usr/local/src/dlib
#链接时选项
#extest_LDFLAGS=-static -Wl,-dy -lopencv_ml -Wl,-dn -lmauto -Wl,--as-needed
#链接时所需链接的库文件
extest_LDADD =$(top_srcdir)/libcommons/libmauto.a -lopencv_ml -lopencv_objdetect -lopencv_stitching -lopencv_superres -lopencv_videostab -lopencv_calib3d -lopencv_features2d -lopencv_highgui -lopencv_video -lopencv_photo -lopencv_imgproc -lopencv_flann -lopencv_core -ldlib

#cvtest
cvtest_SOURCES=cvtest.c exconf.c protocol.c  match.c move.c test.c
cvtest_CFLAGS=-I$(top_srcdir)/libcommons -I/usr/include/opencv
#cvtest_LDFLAGS=-static -std=c99 -L$(top_srcdir)/libcommons
cvtest_LDADD=$(top_srcdir)/libcommons/libmauto.a -lopencv_core -lopencv_highgui -lopencv_imgproc

#excapture
excapture_SOURCES=excapture.c exconf.c protocol.c  match.c move.c test.c
excapture_CFLAGS=-I$(top_srcdir)/libcommons -I/usr/include/opencv
#excapture_LDFLAGS=-static -std=c99 -L$(top_srcdir)/libcommons
excapture_LDADD=$(top_srcdir)/libcommons/libmauto.a -lopencv_core -lopencv_highgui -lopencv_imgproc -lopencv_video

#example
example_SOURCES=example.c
example_CPPFLAGS=-I$(top_srcdir)/libcommons
#example_LDFLAGS=-static -std=c99 -L$(top_srcdir)/libcommons
example_LDADD=$(top_srcdir)/libcommons/libmauto.a

##opencv3
#opencv3_SOURCES=opencv3.cpp
#opencv3_CPPFLAGS=-I$(top_srcdir)/libcommons -I/opt/opencv/include 
#opencv3_LDFLAGS=-L/opt/opencv/lib64
#opencv3_LDADD=$(top_srcdir)/libcommons/libmauto.a -lopencv_dnn -lopencv_ml -lopencv_objdetect -lopencv_shape -lopencv_stitching -lopencv_superres -lopencv_videostab -lopencv_calib3d -lopencv_features2d -lopencv_highgui -lopencv_videoio -lopencv_imgcodecs -lopencv_video -lopencv_photo -lopencv_imgproc -lopencv_flann -lopencv_core

EXTRA_DIST=*.h
