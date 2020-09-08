# Makefile.am -- Process this file with automake to produce Makefile.in
bin_PROGRAMS = sdatxtract

AM_CFLAGS = -Wall -Wno-long-long -Wconversion -pedantic

sdatxtract_SOURCES	=	src/main.cpp src/sdatxtract.cpp src/sdat.cpp \
				src/common.cpp src/decoder/cioutil.c src/decoder/libsmfc.c \
				src/decoder/libsmfcx.c src/decoder/nssamp.c src/decoder/nsstrm.c \
				src/decoder/nsswav.c src/decoder/sseq2mid.c src/swar.c

EXTRA_DIST = autogen.sh
