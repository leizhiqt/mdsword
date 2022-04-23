AUTOMAKE_OPTIONS = foreign

#bin_PROGRAMS = 
noinst_PROGRAMS = tracking train_face draw_gl

#头文件
LIBS=-lpthread

tracking_SOURCES = tracking.cpp
tracking_CPPFLAGS=-I$(top_srcdir)/libcommons -I/usr/include/opencv -I/usr/local/src/dlib
tracking_LDADD = $(top_srcdir)/libcommons/libmauto.a -lopencv_shape -lopencv_stitching -lopencv_superres -lopencv_videostab -lopencv_aruco -lopencv_bgsegm -lopencv_bioinspired -lopencv_ccalib -lopencv_cvv -lopencv_dpm -lopencv_face -lopencv_photo -lopencv_freetype -lopencv_fuzzy -lopencv_hdf -lopencv_hfs -lopencv_img_hash -lopencv_line_descriptor -lopencv_optflow -lopencv_reg -lopencv_rgbd -lopencv_saliency -lopencv_stereo -lopencv_structured_light -lopencv_phase_unwrapping -lopencv_surface_matching -lopencv_tracking -lopencv_datasets -lopencv_video -lopencv_plot -lopencv_ml -lopencv_ximgproc -lopencv_calib3d -lopencv_features2d -lopencv_highgui -lopencv_videoio -lopencv_flann -lopencv_xobjdetect -lopencv_imgcodecs -lopencv_objdetect -lopencv_xphoto -lopencv_imgproc -lopencv_core -ldlib

train_face_SOURCES = train_face.cpp
train_face_CPPFLAGS=-I$(top_srcdir)/libcommons -I/usr/include/opencv -I/usr/local/src/dlib
train_face_LDADD = $(top_srcdir)/libcommons/libmauto.a -lopencv_shape -lopencv_stitching -lopencv_superres -lopencv_videostab -lopencv_aruco -lopencv_bgsegm -lopencv_bioinspired -lopencv_ccalib -lopencv_cvv -lopencv_dpm -lopencv_face -lopencv_photo -lopencv_freetype -lopencv_fuzzy -lopencv_hdf -lopencv_hfs -lopencv_img_hash -lopencv_line_descriptor -lopencv_optflow -lopencv_reg -lopencv_rgbd -lopencv_saliency -lopencv_stereo -lopencv_structured_light -lopencv_phase_unwrapping -lopencv_surface_matching -lopencv_tracking -lopencv_datasets -lopencv_video -lopencv_plot -lopencv_ml -lopencv_ximgproc -lopencv_calib3d -lopencv_features2d -lopencv_highgui -lopencv_videoio -lopencv_flann -lopencv_xobjdetect -lopencv_imgcodecs -lopencv_objdetect -lopencv_xphoto -lopencv_imgproc -lopencv_core -ldlib

draw_gl_SOURCES = draw_gl.cpp
draw_gl_CPPFLAGS=-I$(top_srcdir)/libcommons -I/usr/include/GL  -I/usr/include/gdal
draw_gl_LDADD = $(top_srcdir)/libcommons/libmauto.a -lpng -lglut -lGL -lgdal

EXTRA_DIST = *.h
