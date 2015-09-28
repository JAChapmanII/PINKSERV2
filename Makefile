# places to find and put files
SDIR=src
LDIR=lib
PDIR=pbrane
MDIR=modules
ODIR=obj
BDIR=bin

# main binary
BIN=pbrane

OBJS:= \
	$(patsubst ${SDIR}/%.cpp,${ODIR}/%.o,$(wildcard ${SDIR}/*.cpp)) \
	$(patsubst ${LDIR}/%.cpp,${ODIR}/%.o,$(wildcard ${LDIR}/*.cpp)) \
	$(patsubst ${PDIR}/%.cpp,${ODIR}/%.o,$(wildcard ${PDIR}/*.cpp)) \
	$(patsubst ${MDIR}/%.cpp,${ODIR}/%.o,$(wildcard ${MDIR}/*.cpp))

CXXFLAGS=-std=c++1y -I${SDIR} -I${LDIR} -I${PDIR} -I${MDIR}
LDFLAGS=-lboost_regex -lsqlite3
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

all: dir ${BDIR}/${BIN}
dir:
	mkdir -p ${SDIR} ${ODIR} ${BDIR}

${BDIR}/${BIN}: ${OBJS}
	${CXX} -o $@ $^ ${LDFLAGS}

${LDIR}/modules.cpp.gen: ${MDIR}/*.hpp
	${BDIR}/makemods
${ODIR}/modules.o: ${LDIR}/modules.cpp ${LDIR}/modules.cpp.gen
	${CXX} -c -o $@ $< ${CXXFLAGS}

${ODIR}/%.o: ${SDIR}/%.cpp
	${CXX} -c -o $@ $^ ${CXXFLAGS}
${ODIR}/%.o: ${LDIR}/%.cpp
	${CXX} -c -o $@ $^ ${CXXFLAGS}
${ODIR}/%.o: ${PDIR}/%.cpp
	${CXX} -c -o $@ $^ ${CXXFLAGS}
${ODIR}/%.o: ${MDIR}/%.cpp
	${CXX} -c -o $@ $^ ${CXXFLAGS}

clean:
	rm -rf ${ODIR}/*.o ${LDIR}/modules.cpp.gen ${BDIR}/${BIN}


