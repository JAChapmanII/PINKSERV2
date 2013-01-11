# places to find and put files
SDIR=src
LDIR=lib
PDIR=pbrane
MDIR=modules
ODIR=obj
BDIR=bin

# main project binaries
BINS=${BDIR}/pbrane ${BDIR}/teval

MOBJS=
# module related objects from lib/
MOBJS+=${ODIR}/brain.o ${ODIR}/modules.o ${ODIR}/util.o ${ODIR}/dictionary.o
MOBJS+=${ODIR}/chatline.o ${ODIR}/markovmodel.o
# module object files from modules/
MOBJS+=${ODIR}/function.o ${ODIR}/markov.o ${ODIR}/regex.o

# object files required for main binary
OBJS=
OBJS+=${MOBJS} ${ODIR}/util.o ${ODIR}/global.o ${ODIR}/config.o
OBJS+=${ODIR}/tokenfragment.o ${ODIR}/expressiontree.o
OBJS+=${ODIR}/permission.o ${ODIR}/variable.o

CXXFLAGS=-std=c++0x -I${SDIR} -I${LDIR} -I${PDIR} -I${MDIR}
LDFLAGS=-lboost_regex
# -lgmp -lgmpxx

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

all: dir ${BINS}
dir:
	mkdir -p ${SDIR} ${ODIR} ${BDIR}

${BDIR}/pbrane: ${ODIR}/pbrane.o ${OBJS}
	${CXX} -o $@ $^ ${LDFLAGS}
${BDIR}/teval: ${ODIR}/teval.o ${OBJS}
	${CXX} -o $@ $^ ${LDFLAGS}

${ODIR}/%.o: ${SDIR}/%.cpp
	${CXX} -c -o $@ $^ ${CXXFLAGS}
${ODIR}/%.o: ${LDIR}/%.cpp
	${CXX} -c -o $@ $^ ${CXXFLAGS}
${ODIR}/%.o: ${PDIR}/%.cpp
	${CXX} -c -o $@ $^ ${CXXFLAGS}
${ODIR}/%.o: ${MDIR}/%.cpp
	${CXX} -c -o $@ $^ ${CXXFLAGS}

clean:
	rm -rf ${ODIR}/*.o ${BINS}


