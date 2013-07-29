ENABLE_DEBUG=-D DEBUG_ON
CCFLAGS=-Wall $(ENABLE_DEBUG)

LIBS=-L/usr/lib -lboost_thread -lboost_system
INCLUDE=-I/usr/include

OBJ=node.o
EXE=node


node: node.o
	g++ -o $(EXE) $(LIBS) $(OBJ)

%.o : %.cpp
	g++ -c $(CCFLAGS) $(INCLUDE) $<