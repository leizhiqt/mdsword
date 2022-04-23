AUTOMAKE_OPTIONS = foreign

noinst_PROGRAMS = arp tcp pipe

AM_CPPFLAGS=-I$(top_srcdir)/libcommons -I/usr/include/pcap
LIBS= ../../libcommons/libmauto.a -lpcap -lnet -lpthread

arp_SOURCES = arp.c
#arp_CPPFLAGS = -I/usr/include/pcap
#arp_LDFLAGS = -lpcap -lnet

tcp_SOURCES = tcp.c
#tcp_CPPFLAGS = -I/usr/include/pcap
#tcp_LDFLAGS = -lpcap -lnet

pipe_SOURCES = pipe.c
#pipe_CPPFLAGS = -I/usr/include/pcap
#pipe_LDFLAGS = -lpcap -lnet

#miniecho_SOURCES = miniecho.c
#pipe_CPPFLAGS = -I/usr/include/pcap
#pipe_LDFLAGS = -lpcap -lnet

#mpcap_SOURCES = mpcap.c fpcap.c 
#mpcap_CPPFLAGS = -I/usr/include/pcap -I$(top_srcdir)/libcommons
#mpcap_LDADD = $(top_srcdir)/libcommons/libmauto.a -lpcap

#simplest_LDFLAGS = -static

#EXTRA_DIST = *.h
