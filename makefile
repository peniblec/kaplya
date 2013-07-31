ENABLE_DEBUG=-D DEBUG_ON
CCFLAGS=-Wall $(ENABLE_DEBUG)

LIBS=-L/usr/lib -lboost_thread -lboost_system
INCLUDE=-I/usr/include

OBJ=node.o Peer.o Network.o
EXE=node


$(EXE): $(OBJ)
	g++ -o $(EXE) $(LIBS) $(OBJ)

node.o : node.cpp Config.hpp
Peer.o : Peer.cpp Peer.hpp Config.hpp
Network.o : Network.cpp Network.hpp Config.hpp

%.o : %.cpp
	g++ -c $(CCFLAGS) $(INCLUDE) $<