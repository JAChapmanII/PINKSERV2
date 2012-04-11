# places to find and put files
SDIR=src
LDIR=lib
MDIR=modules
ODIR=obj
BDIR=.

# main project binaries
BINS=$(BDIR)/pbrane $(BDIR)/cm

# module related objects from lib/
MOBJS=$(ODIR)/brain.o $(ODIR)/modules.o $(ODIR)/util.o
# module object files from modules/
MOBJS+=$(ODIR)/core.o $(ODIR)/function.o $(ODIR)/markov.o $(ODIR)/math.o
MOBJS+=$(ODIR)/regex.o $(ODIR)/script.o $(ODIR)/simple.o $(ODIR)/todo.o

# object files required for main binary
OBJS=$(MOBJS) $(ODIR)/util.o $(ODIR)/global.o $(ODIR)/config.o

# object files required for cm
CMOBJS=$(ODIR)/ircsocket.o $(ODIR)/subprocess.o $(ODIR)/util.o

CXXFLAGS=-std=c++0x -I$(SDIR) -I$(LDIR) -I$(MDIR)
LDFLAGS=-lboost_regex

ifndef release
CXXFLAGS+=-g
else
CXXFLAGS+=-O3 -Os
endif

ifndef nowall
CXXFLAGS+=-Wall -Wextra -pedantic -Wmain -Weffc++ -Wswitch-default -Wswitch-enum
CXXFLAGS+=-Wmissing-include-dirs -Wmissing-declarations -Wunreachable-code
CXXFLAGS+=-Winline -Wfloat-equal -Wundef -Wcast-align -Wredundant-decls
CXXFLAGS+=-Winit-self -Wshadow
endif

ifdef profile
CXXFLAGS+=-pg
LDFLAGS+=-pg
endif

all: dir $(BINS)
dir:
	mkdir -p $(SDIR) $(ODIR) $(BDIR)

$(BDIR)/pbrane: $(ODIR)/pbrane.o $(OBJS)
	$(CXX) -o $@ $^ $(LDFLAGS)
$(BDIR)/cm: $(ODIR)/cm.o $(CMOBJS)
	$(CXX) -o $@ $^ $(LDFLAGS) -lsfml-network -lsfml-system

$(ODIR)/%.o: $(SDIR)/%.cpp
	$(CXX) -c -o $@ $^ $(CXXFLAGS)
$(ODIR)/%.o: $(LDIR)/%.cpp
	$(CXX) -c -o $@ $^ $(CXXFLAGS)
$(ODIR)/%.o: $(MDIR)/%.cpp
	$(CXX) -c -o $@ $^ $(CXXFLAGS)

clean:
	rm -rf $(ODIR)/*.o $(BINS)


