CXX  	    = $(CXX)
CXXFLAGS    = $(CXXFLAGS)
LINKFLAGS   = $(LINKFLAGS)
LIBFLAGS    = $(LIBFLAGS)
LIBDIR      = $(LIBDIR)

PROGRAM     = perseg

SRC         = ChType.cpp  Perceptron.cpp  PerSeg.cpp  Segmentor.cpp
OBJ         = ChType.o    Perceptron.o    PerSeg.o    Segmentor.o

.SUFFIXES:
.SUFFIXES: .o .c .cpp

.cpp.o:
	$(CXX) $(CXXFLAGS) -c $*.cpp

all: $(PROGRAM)

$(PROGRAM): $(OBJ) 
	$(CXX) $(LINKFLAGS) $(LIBFLAGS) -o $(PROGRAM) $(OBJ) $(ASM)

clean:
	-rm -f *.o


