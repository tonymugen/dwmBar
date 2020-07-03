
CXX ?= g++
INSTALLDIR = /usr/local
# dwmbar binary
DBOUT = dwmbar
DBOBJ = modules.o

CXXFLAGS = -O2 -march=native -std=c++11 -pthread -lX11

all : $(DBOUT)
.PHONY : all

install : $(DBOUT)
	-cp -v $(DBOUT) $(INSTALLDIR)/bin
.PHONY : install

$(DBOUT) : dwmbar.cpp $(DBOBJ) config.hpp
	$(CXX) dwmbar.cpp $(DBOBJ) -o $(DBOUT) $(CXXFLAGS)

modules.o : modules.cpp modules.hpp
	$(CXX) -c modules.cpp $(CXXFLAGS)

.PHONY : clean
clean :
	-rm -v *.o $(DBOUT)

