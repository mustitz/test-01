bin_PROGRAMS = test
noinst_LIBRARIES = libtestimg.a

if ENABLE_DEBUG
EXTRA_CXXFLAGS = -g3 -O0 -Wall -Werror
else
EXTRA_CXXFLAGS = -Ofast
endif
EXTRA_CXXFLAGS += -std=gnu++11 -I../include/

test_CXXFLAGS = $(EXTRA_CXXFLAGS)
test_LDADD = libtestimg.a
test_SOURCES = main.cpp

libtestimg_a_CXXFLAGS = $(EXTRA_CXXFLAGS)
libtestimg_a_SOURCES = lib.cpp
