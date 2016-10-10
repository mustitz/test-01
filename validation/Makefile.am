LOG_DRIVER = ./validation.sh

TESTS = run-outsider

check_PROGRAMS = outsider

if ENABLE_DEBUG
EXTRA_CXXFLAGS = -g3 -O0 -Wall -Werror
else
EXTRA_CXXFLAGS = -Ofast
endif
EXTRA_CXXFLAGS += -std=gnu++11 -I${srcdir}/../include

outsider_CXXFLAGS = $(EXTRA_CXXFLAGS)
outsider_SOURCES = outsider.cpp

.PHONY : run-outsider

EXTRA_DIST = validation.sh
