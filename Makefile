SRCDIR=src
BINS=pbrane
OBJS=

CXXFLAGS=-std=c++0x
LDFLAGS=-g -lboost_regex

ifndef nowall
CXXFLAGS+=-Wextra -pedantic -Wmain -Weffc++ -Wswitch-default -Wswitch-enum
CXXFLAGS+=-Wmissing-include-dirs -Wmissing-declarations -Wunreachable-code
CXXFLAGS+=-Winline -Wfloat-equal -Wundef -Wcast-align -Wredundant-decls
CXXFLAGS+=-Winit-self -Wshadow
endif

all: $(BINS)

pbrane: $(SRCDIR)/pbrane.o
	$(CXX) -o $@ $^ $(LDFLAGS)

clean:
	rm -rf *.o $(BINS)

