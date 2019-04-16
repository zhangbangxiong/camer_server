#############################################################################

# $Id: Makefile,v 1.10 2011/05/31 10:00:00 Skal Exp $
# #
# #############################################################################
#
ifeq (${CC},"")
	CC=gcc
endif

AR = ar

OS=$(shell uname)

ifeq ($(findstring CYGWIN,$(OS)),CYGWIN)
	PLATFORM = CYGWIN
	INCLUDE = -I/usr/include -I/usr/local/include
endif

ifeq ($(findstring Linux,$(OS)),Linux)
	PLATFORM = LINUX
endif

LIBS += 
LDFLAGS += -L$(PWD)/libs -L/usr/lib64 -lzlog -lpthread -lm -lmysqlclient -ldl -lz -lcurl -lrtmp 
INCLUDE += -I$(PWD)/inc -I$(PWD)/librtmp
DEFINES = -D$(PLATFORM)

CXXFLAGS = $(DEFINES) -g -Wall -pipe -O3 -DMG_ENABLE_THREADS $(ARCHITECTURE) $(BUS) $(ENDIANNESS) $(FEATURES) $(SPECIFIC_CFLAGS)

TARGET = stream_to_rtmp
OBJECTS= main.o ezxml.o arm_xml.o util.o config.o cjson.o mongoose.o 

all:$(OBJECTS) 
	$(CC) -o $(TARGET) $(OBJECTS) $(LDFLAGS)
.cpp.o:
	$(CXX) -c $(CXXFLAGS) $(INCLUDE) $< -o $@

.c.o:
	$(CC) -c $(CXXFLAGS) $(INCLUDE)  $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGET)

rebuild:
	make clean
	make
