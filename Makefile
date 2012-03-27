SDIR=src
ODIR=obj
MDIR=modules
BDIR=.

BINS=$(BDIR)/pbrane

MOBJS=$(ODIR)/modules.o $(ODIR)/function.o $(ODIR)/brain.o
MOBJS+=$(ODIR)/markov.o $(ODIR)/math.o $(ODIR)/regex.o
MOBJS+=$(ODIR)/simple.o $(ODIR)/todo.o $(ODIR)/core.o

OBJS=$(MOBJS) $(ODIR)/util.o $(ODIR)/global.o $(ODIR)/config.o


CXXFLAGS=-std=c++0x -Imodules -Isrc
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

all: dir $(BINS) 
	# cm
dir:
	mkdir -p $(SDIR) $(ODIR) $(BDIR)

$(BDIR)/pbrane: $(ODIR)/pbrane.o $(OBJS)
	$(CXX) -o $@ $^ $(LDFLAGS)

$(BDIR)/cm: $(ODIR)/cm.o $(ODIR)/ircsocket.o
	$(CXX) -o $@ $^ $(LDFLAGS) -lsfml-network -lsfml-system

$(ODIR)/%.o: $(SDIR)/%.cpp
	$(CXX) -c -o $@ $^ $(CXXFLAGS)
$(ODIR)/%.o: $(MDIR)/%.cpp
	$(CXX) -c -o $@ $^ $(CXXFLAGS)

clean:
	rm -rf $(ODIR)/*.o $(BINS) cm


