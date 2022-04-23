AUTOMAKE_OPTIONS = foreign

noinst_PROGRAMS = simplest ffavi ffrgb

AM_CPPFLAGS=-I$(top_srcdir)/libcommons
LIBS= 

ffavi_SOURCES = ffavi.c
ffavi_CPPFLAGS = -I/usr/include/ffmpeg
ffavi_LDFLAGS = -lavformat -lavcodec -lavutil -lavdevice -lavfilter

simplest_SOURCES = simplest_ffmpeg_streamer.c
simplest_CPPFLAGS = -I/usr/include/ffmpeg
simplest_LDFLAGS = -lavformat -lavcodec -lavutil -lavdevice -lavfilter

ffrgb_SOURCES = scaling_video.c
ffrgb_CPPFLAGS = -I/usr/include/ffmpeg
ffrgb_LDFLAGS = -lavformat -lavcodec -lavutil -lavdevice -lavfilter -lavutil -lswscale
#simplest_LDFLAGS = -static
