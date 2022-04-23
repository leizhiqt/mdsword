AUTOMAKE_OPTIONS = foreign

noinst_PROGRAMS = testudp testcase

AM_CPPFLAGS=-I$(top_srcdir)/libcommons
LIBS= ../../libcommons/libmauto.a -lpthread

testudp_SOURCES = testudp.c
testudp_CPPFLAGS=-I$(top_srcdir)/libcommons
testudp_LDADD = $(top_srcdir)/libcommons/libmauto.a

testcase_SOURCES = testcase.c
testcase_CPPFLAGS=-I$(top_srcdir)/libcommons
testcase_LDADD = $(top_srcdir)/libcommons/libmauto.a
