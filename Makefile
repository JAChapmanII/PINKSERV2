# places to find and put files
SDIR=src
LDIR=lib
MDIR=modules
ODIR=obj
BDIR=bin

# main binary
BIN=pbrane

OBJS:= \
	$(patsubst ${SDIR}/%.cpp,${ODIR}/%.o,$(wildcard ${SDIR}/*.cpp)) \
	$(patsubst ${LDIR}/%.cpp,${ODIR}/%.o,$(wildcard ${LDIR}/*.cpp)) \
	$(patsubst ${MDIR}/%.cpp,${ODIR}/%.o,$(wildcard ${MDIR}/*.cpp))

CXXFLAGS=-std=c++1y -I${SDIR} -I${LDIR} -I${MDIR}
LDFLAGS=-lboost_regex -lsqlite3 -lcurl -lpbrane -lsekisa
# -lgmp -lgmpxx

ifndef release
CXXFLAGS+=-g -rdynamic
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

all: dir ${LDIR}/modules.cpp.gen ${SDIR} ${BDIR}/${BIN}
dir:
	mkdir -p ${SDIR} ${ODIR} ${BDIR}

${BDIR}/${BIN}: ${OBJS}
	${CXX} -o $@ $^ ${LDFLAGS}

# TODO: fix dependency here, src/pbrane.cpp does not see it
${LDIR}/modules.cpp.gen: ${MDIR}/*.hpp
	~/src/pbrane/bin/makemods $@

${ODIR}/%.o: ${SDIR}/%.cpp
	${CXX} -c -o $@ $^ ${CXXFLAGS}
${ODIR}/%.o: ${LDIR}/%.cpp
	${CXX} -c -o $@ $^ ${CXXFLAGS}
${ODIR}/%.o: ${MDIR}/%.cpp
	${CXX} -c -o $@ $^ ${CXXFLAGS}

clean:
	rm -rf ${ODIR}/*.o ${LDIR}/modules.cpp.gen ${BDIR}/${BIN}


