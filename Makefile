###############################################################
#
# Makefile for sampling tools
#
###############################################################
CXX ?= g++
INSTALLDIR = /usr/local
# dwmbar binary
DBOUT = dwmbar

CXXFLAGS = -O3 -march=native -std=c++11 -lX11

all : $(DBOUT)
.PHONY : all

install : $(DBOUT)
	-cp -v $(DBOUT) $(INSTALLDIR)/bin
.PHONY : install

$(DBOUT) : dwmbar.cpp
	$(CXX) dwmbar.cpp -o $(DBOUT) $(CXXFLAGS)

.PHONY : clean
clean :
	-rm -v *.o $(DBOUT)

