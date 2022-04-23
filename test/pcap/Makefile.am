AUTOMAKE_OPTIONS = foreign

noinst_PROGRAMS = ulibpcap

AM_CPPFLAGS=-I$(top_srcdir)/libcommons
LIBS= 

ulibpcap_SOURCES = ulibpcap.c fpcap.c
ulibpcap_CPPFLAGS = -I/usr/include/pcap -I$(top_srcdir)/libcommons
ulibpcap_LDADD = $(top_srcdir)/libcommons/libmauto.a -lpcap
ulibpcap_LDFLAGS = -lpcap
#simplest_LDFLAGS = -static

EXTRA_DIST = *.h
