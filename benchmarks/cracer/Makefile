# This MUST be before any includes
mkfile_path := $(abspath $(lastword $(MAKEFILE_LIST)))
mkfile_dir := $(patsubst %/,%,$(dir $(mkfile_path)))

# environment
COMPILER_HOME=$(mkfile_dir)/../../opencilk-project/llvm/build
RTS_HOME=$(mkfile_dir)/../../rts

RD_HOME=$(mkfile_dir)/../../cracer_rdtool
RD_INCLUDE=$(RD_HOME)/include
VPATH=$(mkfile_dir)/../src:$(RD_HOME)/om:$(RD_HOME)/

# compilation flags
CC=$(COMPILER_HOME)/bin/clang
CXX=$(COMPILER_HOME)/bin/clang++

RTS_INCLUDE=$(RTS_HOME)/include
RTS_STATIC_LIB=$(RTS_HOME)/lib/libcilkrts.a

RTS_DYNAMIC_LIB=-Wl,-rpath -Wl,$(RTS_HOME)/lib/x86_64/libcilkrts.so -lcilkrts

OPT=-O3
# OPT=-O0
# STATS=-DSTATS_MODE
STATS=
# TIMES=10
TIMES=1

TOOL_OBJ=drivercsan.o shadow_stack.o stat_util.o mem_access.o print_addr.o trace.o

TOOL_FLAGS=-g $(OPT) -fcilkplus -fno-omit-frame-pointer -std=c++11 -D__CILKRTS_ABI_VERSION=1 -DTIMING_COUNT=$(TIMES) $(STATS) -flto# -DDEBUG_MODE
BASIC_FLAGS=-g $(OPT) -fcilkplus -fno-omit-frame-pointer -std=c++11 -D__CILKRTS_ABI_VERSION=1 -fsanitize=cilk -DTIMING_COUNT=$(TIMES) $(STATS) -DEND_OF_COMPUTATION -flto #-mllvm -cilksan-loop-hoisting=false
LDLIBS=-lpthread -ldl -lrt -lnuma -flto

.PHONY: default all clean
default: all

ktiming.o: ktiming.c
	$(CXX) $(TOOL_FLAGS) -I$(RTS_INCLUDE) -I$(RD_INCLUDE) -c $< -o $@

%.o: %.cpp
	$(CXX) $(TOOL_FLAGS) -I$(RTS_INCLUDE) -I$(RD_INCLUDE) -c $< -o $@

### matmul
TARGETS = mm-ori
APPS = mm-ori

mm-ori.o: matmul.cpp
	$(CXX) $(TOOL_FLAGS) -I$(RTS_INCLUDE) -I$(RD_INCLUDE) -c $< -o $@

mm-ori: ktiming.o mm-ori.o getoptions.o
	$(CXX) $^ $(RTS_STATIC_LIB) $(LDLIBS) -rdynamic -o $@

TARGETS += mm-rd
APPS += mm-rd

mm-rd.o: matmul.cpp
	$(CXX) $(BASIC_FLAGS) -I$(RTS_INCLUDE) -I$(RD_INCLUDE) -c $< -o $@

mm-rd: ktiming.o mm-rd.o getoptions.o $(TOOL_OBJ)
	$(CXX) $^ $(RTS_STATIC_LIB) $(LDLIBS) -rdynamic -o $@

### matmul_z (matmul with z layout)
TARGETS += mmz-ori
APPS += mmz-ori

mmz-ori.o: matmul_z.cpp
	$(CXX) $(TOOL_FLAGS) -I$(RTS_INCLUDE) -I$(RD_INCLUDE) -c $< -o $@

mmz-ori: ktiming.o mmz-ori.o getoptions.o
	$(CXX) $^ $(RTS_STATIC_LIB) $(LDLIBS) -rdynamic -o $@

TARGETS += mmz-rd
APPS += mmz-rd

mmz-rd.o: matmul_z.cpp
	$(CXX) $(BASIC_FLAGS) -I$(RTS_INCLUDE) -I$(RD_INCLUDE) -c $< -o $@

mmz-rd: ktiming.o mmz-rd.o getoptions.o $(TOOL_OBJ)
	$(CXX) $^ $(RTS_STATIC_LIB) $(LDLIBS) -rdynamic -o $@

### fib
TARGETS += fib-ori
APPS += fib-ori

fib-ori.o: fib.cpp
	$(CXX) $(TOOL_FLAGS) -I$(RTS_INCLUDE) -I$(RD_INCLUDE) -c $< -o $@

fib-ori: ktiming.o fib-ori.o getoptions.o
	$(CXX) $^ $(RTS_STATIC_LIB) $(LDLIBS) -rdynamic -o $@

TARGETS += fib-rd
APPS += fib-rd

fib-rd.o: fib.cpp
	$(CXX) $(BASIC_FLAGS) -I$(RTS_INCLUDE) -I$(RD_INCLUDE) -c $< -o $@

fib-rd: ktiming.o fib-rd.o getoptions.o $(TOOL_OBJ)
	$(CXX) $^ $(RTS_STATIC_LIB) $(LDLIBS) -rdynamic -o $@

### fibx
TARGETS += fibx-ori
APPS += fibx-ori

fibx-ori.o: fibx.cpp
	$(CXX) $(TOOL_FLAGS) -I$(RTS_INCLUDE) -I$(RD_INCLUDE) -c $< -o $@

fibx-ori: ktiming.o fibx-ori.o getoptions.o
	$(CXX) $^ $(RTS_STATIC_LIB) $(LDLIBS) -rdynamic -o $@

TARGETS += fibx-rd
APPS += fibx-rd

fibx-rd.o: fibx.cpp
	$(CXX) $(BASIC_FLAGS) -I$(RTS_INCLUDE) -I$(RD_INCLUDE) -c $< -o $@

fibx-rd: ktiming.o fibx-rd.o getoptions.o $(TOOL_OBJ)
	$(CXX) $^ $(RTS_STATIC_LIB) $(LDLIBS) -rdynamic -o $@

### qsort
TARGETS += qs-ori
APPS += qs-ori

qs-ori.o: qsort.cpp
	$(CXX) $(TOOL_FLAGS) -I$(RTS_INCLUDE) -I$(RD_INCLUDE) -c $< -o $@

qs-ori: ktiming.o qs-ori.o getoptions.o
	$(CXX) $^ $(RTS_STATIC_LIB) $(LDLIBS) -rdynamic -o $@

TARGETS += qs-rd
APPS += qs-rd

qs-rd.o: qsort.cpp
	$(CXX) $(BASIC_FLAGS) -I$(RTS_INCLUDE) -I$(RD_INCLUDE) -c $< -o $@

qs-rd: ktiming.o qs-rd.o getoptions.o $(TOOL_OBJ)
	$(CXX) $^ $(RTS_STATIC_LIB) $(LDLIBS) -rdynamic -o $@

### nqueens
TARGETS += nq-ori
APPS += nq-ori

nq-ori.o: nqueens.cpp
	$(CXX) $(TOOL_FLAGS) -I$(RTS_INCLUDE) -I$(RD_INCLUDE) -c $< -o $@

nq-ori: ktiming.o nq-ori.o getoptions.o
	$(CXX) $^ $(RTS_STATIC_LIB) $(LDLIBS) -rdynamic -o $@

TARGETS += nq-rd
APPS += nq-rd

nq-rd.o: nqueens.cpp
	$(CXX) $(BASIC_FLAGS) -I$(RTS_INCLUDE) -I$(RD_INCLUDE) -c $< -o $@

nq-rd: ktiming.o nq-rd.o getoptions.o $(TOOL_OBJ)
	$(CXX) $^ $(RTS_STATIC_LIB) $(LDLIBS) -rdynamic -o $@

### heat
TARGETS += heat-ori
APPS += heat-ori

heat-ori.o: heat.cpp
	$(CXX) $(TOOL_FLAGS) -I$(RTS_INCLUDE) -I$(RD_INCLUDE) -c $< -o $@

heat-ori: ktiming.o heat-ori.o getoptions.o
	$(CXX) $^ $(RTS_STATIC_LIB) $(LDLIBS) -rdynamic -o $@

TARGETS += heat-rd
APPS += heat-rd

heat-rd.o: heat.cpp
	$(CXX) $(BASIC_FLAGS) -I$(RTS_INCLUDE) -I$(RD_INCLUDE) -c $< -o $@

heat-rd: ktiming.o heat-rd.o getoptions.o $(TOOL_OBJ)
	$(CXX) $^ $(RTS_STATIC_LIB) $(LDLIBS) -rdynamic -o $@

### fft
TARGETS += fft-ori
APPS += fft-ori

fft-ori.o: fft.cpp
	$(CXX) $(TOOL_FLAGS) -I$(RTS_INCLUDE) -I$(RD_INCLUDE) -c $< -o $@

fft-ori: ktiming.o fft-ori.o getoptions.o
	$(CXX) $^ $(RTS_STATIC_LIB) $(LDLIBS) -rdynamic -o $@

TARGETS += fft-rd
APPS += fft-rd

fft-rd.o: fft.cpp
	$(CXX) $(BASIC_FLAGS) -I$(RTS_INCLUDE) -I$(RD_INCLUDE) -c $< -o $@

fft-rd: ktiming.o fft-rd.o getoptions.o $(TOOL_OBJ)
	$(CXX) $^ $(RTS_STATIC_LIB) $(LDLIBS) -rdynamic -o $@


### cholesky
TARGETS += chol-ori
APPS += chol-ori

chol-ori.o: cholesky.cpp
	$(CXX) $(TOOL_FLAGS) -I$(RTS_INCLUDE) -I$(RD_INCLUDE) -c $< -o $@

chol-ori: ktiming.o chol-ori.o getoptions.o
	$(CXX) $^ $(RTS_STATIC_LIB) $(LDLIBS) -rdynamic -o $@

TARGETS += chol-rd
APPS += chol-rd

chol-rd.o: cholesky.cpp
	$(CXX) $(BASIC_FLAGS) -I$(RTS_INCLUDE) -I$(RD_INCLUDE) -c $< -o $@

chol-rd: ktiming.o chol-rd.o getoptions.o $(TOOL_OBJ)
	$(CXX) $^ $(RTS_STATIC_LIB) $(LDLIBS) -rdynamic -o $@

### cilksort
TARGETS += cs-ori
APPS += cs-ori

cs-ori.o: cilksort.cpp
	$(CXX) $(TOOL_FLAGS) -I$(RTS_INCLUDE) -I$(RD_INCLUDE) -c $< -o $@

cs-ori: ktiming.o cs-ori.o getoptions.o
	$(CXX) $^ $(RTS_STATIC_LIB) $(LDLIBS) -rdynamic -o $@

TARGETS += cs-rd
APPS += cs-rd

cs-rd.o: cilksort.cpp
	$(CXX) $(BASIC_FLAGS) -I$(RTS_INCLUDE) -I$(RD_INCLUDE) -c $< -o $@

cs-rd: ktiming.o cs-rd.o getoptions.o $(TOOL_OBJ)
	$(CXX) $^ $(RTS_STATIC_LIB) $(LDLIBS) -rdynamic -o $@

### strassen
TARGETS += stra-ori
APPS += stra-ori

stra-ori.o: strassen.cpp
	$(CXX) $(TOOL_FLAGS) -I$(RTS_INCLUDE) -I$(RD_INCLUDE) -c $< -o $@

stra-ori: ktiming.o stra-ori.o getoptions.o
	$(CXX) $^ $(RTS_STATIC_LIB) $(LDLIBS) -rdynamic -o $@

TARGETS += stra-rd
APPS += stra-rd

stra-rd.o: strassen.cpp
	$(CXX) $(BASIC_FLAGS) -I$(RTS_INCLUDE) -I$(RD_INCLUDE) -c $< -o $@

stra-rd: ktiming.o stra-rd.o getoptions.o $(TOOL_OBJ)
	$(CXX) $^ $(RTS_STATIC_LIB) $(LDLIBS) -rdynamic -o $@

### strassen_z (strassen with z_layout)
TARGETS += straz-ori
APPS += straz-ori

straz-ori.o: strassen_z.cpp
	$(CXX) $(TOOL_FLAGS) -I$(RTS_INCLUDE) -I$(RD_INCLUDE) -c $< -o $@

straz-ori: ktiming.o straz-ori.o getoptions.o
	$(CXX) $^ $(RTS_STATIC_LIB) $(LDLIBS) -rdynamic -o $@

TARGETS += straz-rd
APPS += straz-rd

straz-rd.o: strassen_z.cpp
	$(CXX) $(BASIC_FLAGS) -I$(RTS_INCLUDE) -I$(RD_INCLUDE) -c $< -o $@

straz-rd: ktiming.o straz-rd.o getoptions.o $(TOOL_OBJ)
	$(CXX) $^ $(RTS_STATIC_LIB) $(LDLIBS) -rdynamic -o $@

### knapsack
TARGETS += ks-ori
APPS += ks-ori

ks-ori.o: knapsack.cpp
	$(CXX) $(TOOL_FLAGS) -I$(RTS_INCLUDE) -I$(RD_INCLUDE) -c $< -o $@

ks-ori: ktiming.o ks-ori.o getoptions.o
	$(CXX) $^ $(RTS_STATIC_LIB) $(LDLIBS) -rdynamic -o $@

TARGETS += ks-rd
APPS += ks-rd

ks-rd.o: knapsack.cpp
	$(CXX) $(BASIC_FLAGS) -I$(RTS_INCLUDE) -I$(RD_INCLUDE) -c $< -o $@

ks-rd: ktiming.o ks-rd.o getoptions.o $(TOOL_OBJ)
	$(CXX) $^ $(RTS_STATIC_LIB) $(LDLIBS) -rdynamic -o $@

### lu
TARGETS += lu-ori
APPS += lu-ori

lu-ori.o: lu.cpp
	$(CXX) $(TOOL_FLAGS) -I$(RTS_INCLUDE) -I$(RD_INCLUDE) -c $< -o $@

lu-ori: ktiming.o lu-ori.o getoptions.o
	$(CXX) $^ $(RTS_STATIC_LIB) $(LDLIBS) -rdynamic -o $@

TARGETS += lu-rd
APPS += lu-rd

lu-rd.o: lu.cpp
	$(CXX) $(BASIC_FLAGS) -I$(RTS_INCLUDE) -I$(RD_INCLUDE) -c $< -o $@

lu-rd: ktiming.o lu-rd.o getoptions.o $(TOOL_OBJ)
	$(CXX) $^ $(RTS_STATIC_LIB) $(LDLIBS) -rdynamic -o $@

### rectmul
TARGETS += rmul-ori
APPS += rmul-ori

rmul-ori.o: rectmul.cpp
	$(CXX) $(TOOL_FLAGS) -I$(RTS_INCLUDE) -I$(RD_INCLUDE) -c $< -o $@

rmul-ori: ktiming.o rmul-ori.o getoptions.o
	$(CXX) $^ $(RTS_STATIC_LIB) $(LDLIBS) -rdynamic -o $@

TARGETS += rmul-rd
APPS += rmul-rd

rmul-rd.o: rectmul.cpp
	$(CXX) $(BASIC_FLAGS) -I$(RTS_INCLUDE) -I$(RD_INCLUDE) -c $< -o $@

rmul-rd: ktiming.o rmul-rd.o getoptions.o $(TOOL_OBJ)
	$(CXX) $^ $(RTS_STATIC_LIB) $(LDLIBS) -rdynamic -o $@

all: $(TARGETS)

clean: 
	rm -rf $(APPS) *.o *.s core* 
